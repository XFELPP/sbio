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

#include <bitset>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <vector>

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
    static constexpr std::size_t MaxInactiveRanks { 1024 };

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

    struct Config {
      MPI_Comm communicator { MPI_COMM_WORLD };
      std::vector<int> active_ranks {};
      int main_rank { 0 };
      bool main_rank_loops { true };
    };

    static constexpr std::bitset<
      static_cast<std::size_t>(ParallelizationMethods::NUM_METHODS)
    > ParallelSupport { 0x2 }; // 0b10 - MPI

    static void configure_impl(const Config& config) {
      if (m_world_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&m_world_comm);
      }

      if (m_active_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&m_active_comm);
      }

      if (m_shmem_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&m_shmem_comm);
      }

      MPI_Comm_dup(config.communicator, &m_world_comm);

      MPI_Comm_rank(m_world_comm, &m_rank);
      MPI_Comm_size(m_world_comm, &m_size);

      m_num_inactive_ranks = 0;
      if (config.active_ranks.size() > 0) {
        int curr_rank { -1 };
        for (int rank : config.active_ranks) {
          curr_rank++;

          while (rank != curr_rank) {
            if (m_num_inactive_ranks < MaxInactiveRanks) {
              m_inactive_ranks[m_num_inactive_ranks++] = curr_rank;
            }
            curr_rank++;
          }
        }

        curr_rank++;
        while (curr_rank < m_size) {
          if (m_num_inactive_ranks < MaxInactiveRanks) {
            m_inactive_ranks[m_num_inactive_ranks++] = curr_rank;
          }
          curr_rank++;
        }
      }

      if (m_num_inactive_ranks > 0) {
#ifdef _WIN32
        // Ugh. MS-MPI doesn't seem to have the API below...
        int color { is_current_rank_inactive() ? MPI_UNDEFINED : 0 };
        MPI_Comm_split(m_world_comm, color, m_rank, &m_active_comm);
#else
        MPI_Group world_group;
        MPI_Comm_group(m_world_comm, &world_group);

        MPI_Group active_group;
        MPI_Group_excl(world_group,
                       m_num_inactive_ranks,
                       m_inactive_ranks,
                       &active_group);

        MPI_Comm_create_group(m_world_comm, active_group, 0, &m_active_comm);

        MPI_Group_free(&world_group);
        MPI_Group_free(&active_group);
#endif // _WIN32
      } else {
        MPI_Comm_dup(m_world_comm, &m_active_comm);
      }

      if (m_active_comm != MPI_COMM_NULL) {
        MPI_Comm_rank(m_active_comm, &m_active_rank);
        MPI_Comm_size(m_active_comm, &m_active_size);

        MPI_Comm_split_type(m_active_comm,
                            MPI_COMM_TYPE_SHARED,
                            0,
                            MPI_INFO_NULL,
                            &m_shmem_comm);
      }

      m_main_rank = config.main_rank;
      m_main_rank_loops = config.main_rank_loops;

      // Reset remaining state
      m_event_idx = 0;
    }

    template <IsTypeList Requirements, class IO, class FTraits>
    requires FormatTraits<FTraits, IO, MPIExecution>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::MPIExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::MPIExecution");
      } else {
        m_logger = logger;
      }

      return allocate_impl_helper(Requirements{}, request);
    }

    template <typename... Descriptors, class FTraits>
    static auto allocate_impl_helper(TypeList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      static int new_win_tag { 100 }; // Tag each MPI window to distinguish them
      Storage<TypeList<Descriptors...>, MPIExecution> s;
      std::size_t i { 0 };

      if (m_active_comm == MPI_COMM_NULL) {
        // Configuration was forgotten - force a default config now or will seg fault
        Config def_cfg {};
        configure(def_cfg);
      }

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
          if (buf.window() != MPI_WIN_NULL) {
            MPI_Win_lock_all(0, buf.window());
          }
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
      int seq { 0 };
      if constexpr (std::is_same_v<Role, IndexRole> ||
                    std::is_same_v<Role, GroupRole> ||
                    std::is_same_v<Hint, Shareable>) {
        auto& buf = storage.template get<Role>();
        tag = buf.tag();
        seq = buf.fetch_next_seq();

        auto fence_win = [](auto& buf) {
          // Check if the buffer type is of one supporting a Window.
          if constexpr (requires { buf.window(); }) {
            if (buf.window() != MPI_WIN_NULL) {
              // If its a Window and has valid memory backing, synchronize.
              MPI_Win_sync(buf.window());
            }
          }
        };

        storage.template for_each_role<Role>(fence_win);
      }

      // Broadcast all requested sync_vars
      // We tag our data to distinguish between brokers on the communicator
      // This requires point-to-point Send/Recv, but we recreate a binomial tree
      // distribution pattern like you would get from using a Bcast
      int tree_idx { 0 };
      auto broadcast_all_ranks = [tag, seq, &tree_idx](auto& var) {
        using VarT = decltype(var);

        auto mpi_type { mpi::type_for<VarT>() };

        int ub { mpi::tag_upper_bound() };
        int msg_tag { ((tag * 100 + (seq % 100)) * 10 + tree_idx) % ub };
        tree_idx++;

        if (m_rank == 0) {
          std::vector<MPI_Request> reqs(m_size - 1);
          for (int peer = 1; peer < m_size; ++peer) {
            m_logger->trace("[Rank {}] About to send sync vars to {}."
                            "(tag = {}, seq = {}, msg_tag = {})",
                            m_rank,
                            peer,
                            tag,
                            seq,
                            msg_tag);
            MPI_Isend(&var, 1, mpi_type, peer, msg_tag, m_active_comm, &reqs[peer - 1]);
          }

          if (!reqs.empty()) {
            MPI_Waitall(static_cast<int>(reqs.size()), reqs.data(), MPI_STATUSES_IGNORE);
            m_logger->trace("[Rank {}] Sync vars sent to all peers!"
                            "(tag = {}, seq = {}, msg_tag = {})",
                            m_rank,
                            tag,
                            seq,
                            msg_tag);
          }
        } else {
          MPI_Request req;
          m_logger->trace("[Rank {}] Waiting to receive sync vars from rank 0."
                          "(tag = {}, seq = {}, msg_tag = {})",
                          m_rank,
                          tag,
                          seq,
                          msg_tag);
          MPI_Irecv(&var, 1, mpi_type, 0, msg_tag, m_active_comm, &req);
          MPI_Wait(&req, MPI_STATUS_IGNORE);
          m_logger->trace("[Rank {}] Received sync vars from rank 0."
                          "(tag = {}, seq = {}, msg_tag = {})",
                          m_rank,
                          tag,
                          seq,
                          msg_tag);
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
    static bool should_index_impl() { return m_rank == m_main_rank; }

    static bool is_current_rank_inactive() {
      for (std::size_t i = 0; i < m_num_inactive_ranks; ++i) {
        if (m_rank == m_inactive_ranks[i]) {
          return true;
        }
      }
      return false;
    }

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
    template <class FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType
    next_impl(typename FTraits::StepIdxType& max_capacity, IndexTrigger&& trigger) {
      static typename FTraits::StepIdxType m_event_idx { 0 };

      if ((!m_main_rank_loops && m_rank == m_main_rank) || is_current_rank_inactive()) {
        return FTraits::ExhaustedSentinel;
      }

      int worker_count { m_main_rank_loops ? m_active_size : m_active_size - 1 };
      int worker_rank;
      if (m_main_rank_loops || m_active_rank <= m_main_rank) {
        worker_rank = m_active_rank;
      } else {
        worker_rank = m_active_rank - 1;
      }

      auto step = m_event_idx + worker_rank;
      m_event_idx += worker_count;

      while (step >= max_capacity) {
        if (!trigger()) {
          m_logger->debug("[Rank {}] Trigger returned exhausted: "
                          "max_cap = {}",
                          m_rank,
                          max_capacity);
          return FTraits::ExhaustedSentinel;
        }
      }

      return step;
    }

  private:
    /**
     * Communicator for synchronizing across the whole MPI world.
     */
    static inline MPI_Comm m_world_comm { MPI_COMM_NULL };
    static inline MPI_Comm m_active_comm { MPI_COMM_NULL };
    static inline int m_active_rank { -1 };
    static inline int m_active_size { -1 };
    static inline int m_inactive_ranks[MaxInactiveRanks] {};
    static inline std::size_t m_num_inactive_ranks { 0 };
    static inline int m_main_rank { 0 };
    static inline bool m_main_rank_loops { true };
    /**
     * Communicator used when generating shareable buffers.
     */
    static inline MPI_Comm m_shmem_comm { MPI_COMM_NULL };
    static inline int m_rank { -1 }; ///< This processes rank in the MPI world.
    static inline int m_size { -1 }; ///< The size of the MPI world.
    /**
     * Rank-local index within the MPI world's set of indices to distribute.
     */
    static inline std::size_t m_event_idx { 0 };

    static inline std::shared_ptr<spdlog::logger> m_logger;
  };
} // namespace sbio

#endif // SBIO_EXECUTION_MPI_HH
