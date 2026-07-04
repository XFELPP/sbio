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

#ifndef SBIO_MPI_EXECUTION_HH
#define SBIO_MPI_EXECUTION_HH

#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/sync.hh"
#include "sbio/mpi/storage.hh"

#include <mpi.h>

#include <type_traits>

namespace {
  /**
   * A static dispatcher for converting C++ types to MPI types.
   *
   * @tparam T The input data type.
   * @returns The MPI_Datatype signifier for T.
   */
  template <typename T>
  inline MPI_Datatype mpi_type_for() {
    using U = std::remove_cvref_t<T>;

    if constexpr (std::is_same_v<U, bool>) {
      return MPI_C_BOOL;
    } else if constexpr (std::is_same_v<U, int>) {
      return MPI_INT;
    } else if constexpr (std::is_same_v<U, unsigned int>) {
      return MPI_UNSIGNED;
    } else if constexpr (std::is_same_v<U, long>) {
      return MPI_LONG;
    } else if constexpr (std::is_same_v<U, unsigned long>) {
      return MPI_UNSIGNED_LONG;
    } else if constexpr (std::is_same_v<U, long long>) {
      return MPI_LONG_LONG;
    } else if constexpr (std::is_same_v<U, unsigned long long>) {
      return MPI_UNSIGNED_LONG_LONG;
    } else if constexpr (std::is_same_v<U, float>) {
      return MPI_FLOAT;
    } else if constexpr (std::is_same_v<U, double>) {
      return MPI_DOUBLE;
    } else if constexpr (std::is_same_v<U, char>) {
      return MPI_CHAR;
    } else if constexpr (std::is_same_v<U, std::uint8_t>) {
      return MPI_UINT8_T;
    } else if constexpr (std::is_same_v<U, std::uint32_t>) {
      return MPI_UINT32_T;
    } else if constexpr (std::is_same_v<U, std::uint64_t>) {
      return MPI_UINT64_T;
    } else if constexpr (std::is_same_v<U, std::int32_t>) {
      return MPI_INT32_T;
    } else if constexpr (std::is_same_v<U, std::int64_t>) {
      return MPI_INT64_T;
    }

    return MPI_DATATYPE_NULL;
  }
} // anonymous namespace

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
  class MPIExecution : public IExecution<MPIExecution> {
  public:
    /**
     * Map IndexRole to MpiSharedBuffer and all other roles to HostBuffer.
     */
    template <typename Descriptor>
    using BufferTypeFor =
      std::conditional_t<std::is_same_v<typename Descriptor::role, IndexRole>,
                         MpiSharedBuffer,
                         HostBuffer>;

    template <FormatTraits FTraits>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      MPI_Comm_rank(MPI_COMM_WORLD, &MPIExecution::m_rank);
      MPI_Comm_size(MPI_COMM_WORLD, &MPIExecution::m_size);

      MPI_Comm_split_type(MPI_COMM_WORLD,
                          MPI_COMM_TYPE_SHARED,
                          0,
                          MPI_INFO_NULL,
                          &MPIExecution::m_node_comm);

      using Requirements = typename FTraits::BufferRequirements;
      return allocate_impl_helper(Requirements{}, request);
    }

    template <typename... Descriptors, FormatTraits FTraits>
    static auto allocate_impl_helper(TypeList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
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
        if constexpr (std::is_same_v<BufRole, IndexRole>) {
          // For IndexRole buffers, make them shared
          MPI_Win win;
          MPI_Aint sz_out;
          int disp;
          void* baseptr;

          MPI_Win_allocate_shared(sz, 1, MPI_INFO_NULL, m_node_comm, &baseptr, &win);
          MPI_Win_shared_query(win, 0, &sz_out, &disp, &baseptr);

          // Now make the storage buffer point to window memory.
          // TODO: Error checks, size checks and so on...
          buf.set_memory(baseptr, sz);
          buf.set_window(win);
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
      // Check if Windows need synchronization - only for IndexRole buffers
      if constexpr (std::is_same_v<Role, IndexRole>) {
        auto fence_win = [](auto& buf) {
          // Check if the buffer type is of one supporting a Window.
          if constexpr (requires { buf.window(); }) {
            if (buf.window() != MPI_WIN_NULL) {
              // If its a Window and has valid memory backing, fence to synchronize.
              MPI_Win_fence(0, buf.window());
            }
          }
        };

        storage.template for_each_role<IndexRole>(fence_win);
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
      // Check if Windows need synchronization - only for IndexRole buffers
      if constexpr (std::is_same_v<Role, IndexRole>) {
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

        storage.template for_each_role<IndexRole>(fence_win);
      }

      // Broadcast all requested sync_vars
      auto broadcast_all_ranks = [](auto& var) {
        using VarT = decltype(var);

        auto mpi_type { mpi_type_for<VarT>() };

        // Currently only support the broadcast on primitives not arrays/vectors
        MPI_Bcast(&var, 1, mpi_type, 0, MPI_COMM_WORLD);
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
    static bool should_index_impl() {
      if (m_rank == 0) {
        // We'll rely on rank 0 to do this, it will distribute afterwards
        return true;
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
    template <FormatTraits FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType
    next_impl(typename FTraits::StepIdxType& max_capacity, IndexTrigger&& trigger) {
      static typename FTraits::StepIdxType event_idx { 0 };

      auto step = event_idx + m_rank;
      event_idx += m_size;

      if (step >= max_capacity) {
        if (!trigger()) {
          return FTraits::ExhaustedSentinel;
        }

        if (step >= max_capacity) {
          return FTraits::ExhaustedSentinel;
        }
      }

      return step;
    }

  private:
    static inline MPI_Comm m_node_comm { MPI_COMM_NULL };
    static inline int m_rank { -1 };
    static inline int m_size { -1 };
  };
} // namespace sbio

#endif // SBIO_MPI_EXECUTION_HH
