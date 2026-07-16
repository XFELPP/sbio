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
  class MPIExecution : public IExecution<MPIExecution> {
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
          buf.allocate(m_shmem_comm, sz);
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
      }

      // Broadcast all requested sync_vars
      auto broadcast_all_ranks = [](auto& var) {
        using VarT = decltype(var);

        auto mpi_type { mpi::type_for<VarT>() };

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
    static bool should_index_impl() { m_rank == 0; }

    /**
     * Prepare buffers used by the BrokerGroup.
     *
     * This policy uses shared buffers for the BrokerGroup (if applicable). When
     * requested, therefore, it will fence and build the buffers over an MPI
     * Window. In general, the BrokerGroup will use these buffers for shareable
     * metadata used in processing (constants and the like) - there is no
     * requirement for that, however.
     *
     * This routine relies on the BrokerGroup providing a set of lambda's to run
     * before and after the creation of the shared buffer.
     * 1. First, the `stage_cb` will run for rank 0.
     * 2. Any synchronization will then occur for a first time.
     * 3. The stage_cb must return a byte count for allocation, this is now used for
     *    creating an allocation for an MPI Window (if this is possible).
     * 4. The newly created Window is synchronized.
     * 5. The commit_cb is run.
     * 6. The newly created Window is resynchronized, completing the commit.
     *
     * @note It may be a common pattern that sync_vars includes the size, which is
     *       also broadcast internally. This may seem redundant; however, it allows
     *       returning this information to the caller as well. Furthermore, there
     *       is no restriction on sync_vars, which may also contain additional data.
     *
     * @tparam Role The `Role` for the storage used by the BrokerGroup.
     * @tparam FTraits The data format's format traits.
     * @tparam StorageT The complete type of the BrokerGroup's storage.
     * @tparam SyncT The type of any data to be synched.
     * @tparam StageCB The type of a callback to run for staging the buffer.
     * @tparam CommitCB The type of a callback to commit the buffer at the end.
     * @param[in] storage The BrokerGroup's storage.
     * @param[in] sync_vars The set of variables that must be synchronized for the
     *            buffer preparation to work.
     * @param[in] stage_cb The callback used to prepare the buffer. This is run on
     *            rank 0 only, and it MUST return the number of bytes for the Window.
     * @param[in] commit_cb The callback used after the Window is created to finalize
     *            its preparation (e.g. moving data into the Window).
     */
    template <
      typename Role,
      typename FTraits,
      typename StorageT,
      typename SyncT,
      typename StageCB,
      typename CommitCB
    >
    static void prepare_group_buffers_impl(StorageT& storage,
                                           SyncT& sync_vars,
                                           StageCB&& stage_cb,
                                           CommitCB&& commit_cb) {
      std::size_t size { 0 };
      if (should_index()) {
        size = stage_cb();
      }

      auto mpi_type { mpi::type_for<decltype(size)>() };

      MPI_Bcast(&size, 1, mpi_type, 0, MPI_COMM_WORLD);

      using Requirements = typename FTraits::GroupBufferRequirements;
      AllocationRequest<FTraits> alloc_request;
      alloc_request.size_requests[0] = size;
      storage = MPIExecution::template allocate_storage<Requirements, FTraits>(alloc_request);

      pre_update_impl<Role>(storage);
      if (should_index()) {
        commit_cb(storage.template get<Role>());
      }

      post_update_impl<Role>(storage, std::forward<SyncT>(sync_vars), IOStatus::Success);
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
    static inline MPI_Comm m_shmem_comm { MPI_COMM_NULL };
    static inline int m_rank { -1 };
    static inline int m_size { -1 };
  };
} // namespace sbio

#endif // SBIO_EXECUTION_MPI_HH
