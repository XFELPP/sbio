/*
 * sbio - Stream Broker IO
 *
 * Copyright (C) 2025-2026 Gabriel Dorlhiac
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SBIO_EXECUTION_MPI_HH
#define SBIO_EXECUTION_MPI_HH

#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/sync.hh"
#include "sbio/storage/host_buffer.hh"
#include "sbio/storage/mpi_shared.hh"
#include "sbio/util/mpi.hh"

#include <mpi.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <type_traits>

namespace sbio {
  /**
   * A basic shared MPI execution policy.
   *
   * This policy implements some basic functionality for MPI-based workflows.
   * The main features are:
   * - Allocations for types of the `IndexRole` are made over shared MPI windows.
   * - Shared values are accordingly synchronized via the sync group mechanism on
   *   pre/post update hooks.
   * - As `IndexRole` buffers are shared, redundant INDEXING is avoided by only
   *   allowing that Broker state for rank 0.
   * - Steps are processed in a simple round-robin fashion with a modulo world-size
   *   increment for each rank.
   * - Exhaustion of available indexed capacity automatically retriggers INDEXING
   *   if applicable for the Brokers.
   */
  class MPIExecution : public Execution<MPIExecution> {
  public:
    /**
     * Map IndexRole to MPISharedBuffer and all other roles to HostBuffer.
     */
    template <typename Descriptor>
    using BufferTypeFor = std::conditional_t<
      std::is_same_v<typename Descriptor::role, IndexRole> ||
      std::is_same_v<typename Descriptor::role, GroupRole> ||
      std::is_same_v<typename Descriptor::hint, Shareable>,
      MPISharedBuffer,
      HostBuffer
    >;

    template <IsTypeList Requirements, FormatTraits FTraits>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::MPIExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::MPIExecution");
      } else {
        m_logger = logger;
      }

      if (MPIExecution::m_shmem_comm == MPI_COMM_NULL) {
        MPI_Comm_rank(MPI_COMM_WORLD, &MPIExecution::m_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &MPIExecution::m_size);

        MPI_Comm_split_type(MPI_COMM_WORLD,
                            MPI_COMM_TYPE_SHARED,
                            0,
                            MPI_INFO_NULL,
                            &MPIExecution::m_shmem_comm);
      }

      return allocate_impl_helper(Requirements{}, request);
    }

    template <typename... Descriptors, FormatTraits FTraits>
    static auto allocate_impl_helper(TypeList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      static int new_win_tag { 100 }; // Tag each MPI window to distinguish them
      Storage<TypeList<Descriptors...>, MPIExecution> s;
      std::size_t i { 0 };

      // Move pack expansion into a lambda for legibility
      auto make_window_or_local = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;

        // Using a very dumb size selection right now, take the biggest of user
        // and the data formats minimum. May want to make this smarter...
        std::size_t sz { std::max(Descriptor::min_size, request.size_requests[i++]) };
        auto& buf { s.template get<Descriptor>() };

        using BufRole = typename Descriptor::role;
        using BufHint = typename Descriptor::hint;
        if constexpr (std::is_same_v<BufRole, IndexRole> ||
                      std::is_same_v<BufRole, GroupRole> ||
                      std::is_same_v<BufHint, Shareable>) {
          int tag { new_win_tag++ };
          buf.allocate(m_shmem_comm, sz);
          buf.set_tag(tag);
        } else {
          // Otherwise, just use a standard host buffer.
          buf.set_memory(new char[sz], sz);
        }
      };

      ( (make_window_or_local(std::type_identity<Descriptors>{})), ... );

      return s;
    }

    /**
     * This policy establishes a shared memory window over `index` data.
     * The pre_update hook includes a fence on that window so we can synchronize
     * updates to the memory across ranks.
     *
     * @tparam Role The role of the storage being looked at
     * @tparam StorageT The kind of the storage being looked (type of buffer)
     * @param storage The storage to synchronize.
     */
    template <class Role, class StorageT>
    static void pre_update_impl(StorageT& storage) {
      using List = ExtractDescriptorsListT<StorageT>;

      using Descriptor = typename FindDescriptor<Role, 0, List>::type;

      using Hint = typename GetHint<Descriptor>::type;

      // Check if Windows need synchronization
      if constexpr (std::is_same_v<Role, IndexRole> ||
                    std::is_same_v<Role, GroupRole> ||
                    std::is_same_v<Hint, Shareable>) {
        auto fence_win = [](auto& buf) {
          // Check if the buffer type is of one supporting a Window.
          if constexpr (requires { buf.window(); }) {
            if (buf.window() != MPI_WIN_NULL) {
              // If its a Window and has valid memory backing, fence to synchronize.
              MPI_Win_fence(0, buf.window());
            }
          }
        };

        storage.template for_each_role<Role>(fence_win);
      }
    }

    /**
     * Paired with the pre_update hook, the post_update ensures the synchronization
     * of `IndexRole` storage is completed.
     *
     * Any remaining `sync_vars` are synchronized via a simple MPI_Bcast.
     *
     * @tparam Role The role of the storage being looked at
     * @tparam StorageT The kind of the storage being looked (type of buffer)
     * @tparam SyncT The type of the attributes to be synchronized
     * @param storage The storage to synchronize.
     * @param sync_vars The attributes that require synchronization
     * @param status The IOStatus from the INDEXING state.
     */
    template <class Role, class StorageT, class SyncT>
    static void post_update_impl(StorageT& storage, SyncT&& sync_vars, IOStatus status) {
      using List = ExtractDescriptorsListT<StorageT>;

      using Descriptor = typename FindDescriptor<Role, 0, List>::type;

      using Hint = typename GetHint<Descriptor>::type;

      // Check if Windows need synchronization
      int tag { 0 };
      if constexpr (std::is_same_v<Role, IndexRole> ||
                    std::is_same_v<Role, GroupRole> ||
                    std::is_same_v<Hint, Shareable>) {
        // Only the IndexRole buffers use a Window
        auto fence_win = [](auto& buf) {
          // Check if the buffer type is of one supporting a Window.
          if constexpr (requires { buf.window(); }) {
            if (buf.window() != MPI_WIN_NULL) {
              // If its a Window and has valid memory backing, fence to synchronize.
              MPI_Win_fence(0, buf.window());
            }
          }
        };

        storage.template for_each_role<Role>(fence_win);
        tag = storage.template get<Role>().tag();
      }

      // Broadcast all requested sync_vars
      // We tag our data to distinguish between brokers on the communicator
      // This requires point-to-point Send/Recv, but we recreate a binomial tree
      // distribution pattern like you would get from using a Bcast
      int tree_idx { 0 };
      auto broadcast_all_ranks = [m_rank, m_size, tag, &tree_idx](auto& var) {
        using VarT = decltype(var);

        auto mpi_type { mpi::type_for<VarT>() };

        int msg_tag { tag * 100 + tree_idx };
        tree_idx++;

        int mask { 1 };
        while (mask < m_size) {
          if (m_rank < mask) {
            int peer { m_rank + mask };
            if (peer < m_size) {
              MPI_Send(&var, 1, mpi_type, peer, msg_tag, MPI_COMM_WORLD);
            }
          } else if (m_rank < 2 * mask) {
            int src { m_rank - mask };
            MPI_Recv(&var, 1, mpi_type, src, msg_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          }
          mask <<= 1;
        }
      };

      sync_vars.for_each(broadcast_all_ranks);
    }

    /**
     * Check on if indexing should be done by the Broker.
     *
     * File indexing is performed only by rank 0. This execution policy provides a
     * shared memory buffer for the index storage - synchronization of that buffer
     * ensures that other ranks will see any data rank 0 puts into it.
     *
     * @returns `true` for rank 0, else `false`.
     */
    static bool should_index_impl() { return m_rank == 0; }

    /**
     * Retrieve the next step index to process.
     *
     * Steps are processed using a fixed offset of the rank, with increments of
     * the world size. E.g. for a world size of 2:
     * - Step 1: Rank 0 processes 0, Rank 1 processes 1
     * - Step 2: Rank 0 processes 2, Rank 1 processes 3 ... and so on.
     *
     * @tparam FTraits The FormatTraits for the data format.
     * @tparam IndexTrigger The type of the lambda callback to reindex as needed.
     * @param[in] max_capacity The current max capacity (i.e., already indexed steps).
     * @param[in] trigger A callback to reindex (if appropriate) when capacity is
     *            exhausted.
     * @returns The next step to process using the fixed offset of the world size.
     */
    template <FormatTraits FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType
    next_impl(typename FTraits::StepIdxType& max_capacity, IndexTrigger&& trigger) {
      static typename FTraits::StepIdxType event_idx { 0 };

      auto step = event_idx + m_rank;
      event_idx += m_size;

      while (step >= max_capacity) {
        if (!trigger()) {
          m_logger->debug("[Rank {}] Trigger returned exhausted: "
                          "max_cap = {}",
                          m_rank,
                          max_capacity);
          int mask { 1 };
          while (mask < m_size) {
            if ((m_rank & mask) == 0) {
              int peer { m_rank | mask };
              if (peer < m_size) {
                int dummy { 0 };
                MPI_Recv(&dummy, 1, MPI_INT, peer, 9999, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              }
            } else {
              int peer { m_rank & ~mask };
              int dummy { 0 };
              MPI_Send(&dummy, 1, MPI_INT, peer, 9999, MPI_COMM_WORLD);
              break;
            }
            mask <<= 1;
          }

          mask >>= 1;
          while (mask > 0) {
            if ((m_rank & mask) == 0) {
              int peer { m_rank | mask };
              if (peer < m_size) {
                int dummy { 0 };
                MPI_Send(&dummy, 1, MPI_INT, peer, 9999, MPI_COMM_WORLD);
              }
            } else {
              int peer { m_rank & ~mask };
              int dummy { 0 };
              MPI_Recv(&dummy, 1, MPI_INT, peer, 9999, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            mask >>= 1;
          }

          return FTraits::ExhaustedSentinel;
        }
      }

      return step;
    }

    template <typename Buffer>
    static void* acquire_broker_view_impl(Buffer& buf) {
      if constexpr (requires { buf.host_ptr(); }) {
        if (buf.is_dirty()) {
          buf.set_dirty(false);
        }
        return buf.host_ptr();
      }

      return buf.ptr();
    }

    template <typename Buffer, typename Result>
    static Result release_broker_view_impl(Buffer& buf, void* broker_view, Result res) {
      if constexpr (requires { buf.host_ptr(); }) {
        // Map host-mirror offset to the GPU device address space
        std::size_t offset =
            reinterpret_cast<const char*>(res.data) - reinterpret_cast<const char*>(broker_view);
        res.data = reinterpret_cast<const char*>(buf.ptr()) + offset;
      }
      return res;
    }

  private:
    /**
     * Communicator used when generating shareable buffers.
     */
    static inline MPI_Comm m_shmem_comm { MPI_COMM_NULL };
    static inline int m_rank { -1 }; ///< This processes rank in the MPI world.
    static inline int m_size { -1 }; ///< The size of the MPI world.

    static inline std::shared_ptr<spdlog::logger> m_logger;
  };
} // namespace sbio

#endif // SBIO_EXECUTION_MPI_HH
