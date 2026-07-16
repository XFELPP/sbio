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

#ifndef SBIO_EXECUTION_MPI_THREADED_HH
#define SBIO_EXECUTION_MPI_THREADED_HH

#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/storage/host_buffer.hh"
#include "sbio/storage/mpi_shared.hh"
#include "sbio/storage/thread_local_buffer.hh"
#include "sbio/util/mpi.hh"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace sbio {
  /**
   * The MPIThreadedExecution policy is designed for a hybrid MPI/threaded approach.
   *
   * The main features of the policy are:
   * - Allocations for types of the `IndexRole` are made over shared MPI windows.
   * - Shared values are accordingly synchronized via the sync group mechanism on
   *   pre/post update hooks.
   * - As `IndexRole` buffers are shared, redundant INDEXING is avoided by only
   *   allowing that Broker state for rank 0.
   * - Steps are processed in a simple round-robin fashion with a modulo world-size
   *   increment for each rank.
   * - Exhaustion of available indexed capacity automatically retriggers INDEXING
   *   if applicable for the Brokers.
   * - Allocations for the types of `DataRole` and `TableRole` use ThreadLocalBuffer
   *   to allow a single rank to parallelize data across multiple threads.
   */
  class MPIThreadedExecution : public Execution<MPIThreadedExecution> {
  public:
    template <typename Descriptor>
    using BufferTypeFor = std::conditional_t<
      std::is_same_v<typename Descriptor::role, IndexRole> ||
      std::is_same_v<typename Descriptor::role, GroupRole> ||
      std::is_same_v<typename Descriptor::hint, Shareable>,
      MPISharedBuffer,
      std::conditional_t<
        std::is_same_v<typename Descriptor::role, DataRole> ||
        std::is_same_v<typename Descriptor::role, TableRole>,
        ThreadLocalBuffer,
        HostBuffer
      >
    >;

    template <IsTypeList Requirements, FormatTraits FTraits>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      if (m_shmem_comm == MPI_COMM_NULL) {
        MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &m_size);

        MPI_Comm_split_type(MPI_COMM_WORLD,
                            MPI_COMM_TYPE_SHARED,
                            0,
                            MPI_INFO_NULL,
                            &m_shmem_comm);
      }
      return allocate_impl_helper(Requirements{}, request);
    }

    template <typename... Descriptors, FormatTraits FTraits>
    static auto allocate_impl_helper(TypeList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      Storage<TypeList<Descriptors...>, MPIThreadedExecution> s;
      std::size_t i { 0 };

      auto make_window_or_local = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;
        std::size_t sz { std::max(Descriptor::min_size, request.size_requests[i++]) };
        auto& buf { s.template get<Descriptor>() };

        using BufRole = typename Descriptor::role;
        using BufHint = typename Descriptor::hint;

        if constexpr (std::is_same_v<BufRole, IndexRole> ||
                      std::is_same_v<BufRole, GroupRole> ||
                      std::is_same_v<BufHint, Shareable>) {
          buf.allocate(m_shmem_comm, sz);
        } else if constexpr (std::is_same_v<BufRole, DataRole> ||
                             std::is_same_v<BufRole, TableRole>) {
          buf.set_memory(nullptr, sz);
        } else {
          buf.set_memory(new char[sz], sz);
        }
      };

      ( (make_window_or_local(std::type_identity<Descriptors>{})), ... );
      return s;
    }

    /**
     * Before updates, fence shared memory.
     *
     * This policy establishes a shared memory window over `index` data and additional
     * roles which have been marked as `Shareable`.
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

      if constexpr (std::is_same_v<Role, IndexRole> ||
                    std::is_same_v<Role, GroupRole> ||
                    std::is_same_v<Hint, Shareable>) {
        auto fence_win = [](auto& buf) {
          if constexpr (requires { buf.window(); }) {
            if (buf.window() != MPI_WIN_NULL) {
              MPI_Win_fence(0, buf.window());
            }
          }
        };
        storage.template for_each_role<Role>(fence_win);
      }
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
     * After updates, synchronize shared resources.
     *
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

      if constexpr (std::is_same_v<Role, IndexRole> ||
                    std::is_same_v<Role, GroupRole> ||
                    std::is_same_v<Hint, Shareable>) {
        auto fence_win = [](auto& buf) {
          if constexpr (requires { buf.window(); }) {
            if (buf.window() != MPI_WIN_NULL) {
              MPI_Win_fence(0, buf.window());
            }
          }
        };
        storage.template for_each_role<Role>(fence_win);
      }

      auto broadcast_all_ranks = [](auto& var) {
        using VarT = decltype(var);
        MPI_Bcast(&var, 1, mpi::type_for<VarT>(), 0, MPI_COMM_WORLD);
      };
      sync_vars.for_each(broadcast_all_ranks);
    }

    /**
     * The MPIThreadedExecution policy splits BrokerGroup data fetch and resolution.
     *
     * When the BrokerGroup requests data of a specific kind for a specific index,
     * the process will be split into two stages. First, the IO fetch portion,
     * where the data is actually pulled from the stream into memory buffers,
     * is done behind a lock. This lock simplifies life on the Broker-side, as any
     * shared mutable state (counters, or internal tracking variables) does not need
     * to be protected while data is being read. It, of course, has a performance
     * penalty, however, as the fetch is serialized.
     *
     * After the fetch portion concludes, the data resolution stage can be done in
     * parallel.
     *
     * @tparam FTraits The data-format traits.
     * @tparam FetchCBType The type of the BrokerGroup callback for data fetching.
     * @tparam GetCBType The type of the BrokerGroup callback for data resolution.
     * @param[in] step_idx The step_idx for which data should be fetched and read.
     * @oaram[in] unit_fetcher A per-broker callback from the BrokerGroup to fetch data.
     * @param[in] num_fetches The number of fetches to perform. (Generally equal
     *            to the number of brokers)
     * @param[in] unit_get_data A per-broker callback from the BrokerGroup to resolve a
     *            piece of requested data inside the bytes just fetched.
     * @param[in] num_accesses The number of gets to perform. (Generally equal
     *            to the number of brokers)
     * @returns The IOStatus from the fetch and get procedure.
     */
    template <class FTraits, class FetchCBType, class GetCBType>
    static IOStatus get_data_impl(typename FTraits::StepIdxType step_idx,
                                  FetchCBType&& unit_fetcher,
                                  std::size_t num_fetches,
                                  GetCBType&& unit_get_data,
                                  std::size_t num_accesses) {
      IOStatus status { IOStatus::Success };
      {
        std::lock_guard<std::mutex> lock(m_io_mutex);
        for (std::size_t i = 0; i < num_fetches; ++i) {
          if (auto s = unit_fetcher(i); s != IOStatus::Success) {
            status = s;
            break;
          }
        }
      }

      if (status == IOStatus::Success) {
        for (std::size_t i = 0; i < num_accesses; ++i) {
          unit_get_data(i);
        }
      }
      return status;
    }


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
      storage =
        MPIThreadedExecution::template allocate_storage<Requirements, FTraits>(alloc_request);

      pre_update_impl<Role>(storage);
      if (should_index()) {
        commit_cb(storage.template get<Role>());
      }

      post_update_impl<Role>(storage, std::forward<SyncT>(sync_vars), IOStatus::Success);
    }

    /**
     * The MPIThreadedExecution policy generates step indices modulo MPI world size.
     *
     * This policy generates steps with consideration of both the MPI world, as well
     * as the threads in a single MPI rank. As the index generator is shared by all
     * threads in the process, it is generated on an atomic counter. This requires the
     * use of acquire/release semantics, and thus has some ammount of overhead compared
     * to a lock-free policy. Furthermore, the increment on the counter is done modulo
     * the size of the MPI world. This gives each rank a fixed offset for received
     * indices:
     * - Step 1: Rank 0 processes 0, Rank 1 processes 1
     * - Step 2: Rank 0 processes 2, Rank 1 processes 3 ... and so on.
     *
     * A lock is explicitly acquired in the event that the index capacity
     * has been reached, and thus a reindexing trigger must be called.
     *
     * @note This policy is intended to be used with indexable formats. This
     * function will work if it is not indexable; however, the callback trigger
     * must then be designed in a way to terminate the generation of indices
     * in some fashion, or it will continue forever.
     *
     * @tparam FTraits The data-format traits.
     * @tparam IndexTrigger The type of the reindex callback trigger.
     * @param[in] max_capacity The current max capacity (currently available indices).
     * @param[in] trigger The reindex callback routine.
     * @returns The next step_idx.
     */
    template <FormatTraits FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType
    next_impl(typename FTraits::StepIdxType& max_capacity, IndexTrigger&& trigger) {
      static std::atomic<typename FTraits::StepIdxType> local_idx { 0 };
      static std::mutex trigger_mutex;

      typename FTraits::StepIdxType idx { local_idx.load(std::memory_order_acquire) };
      typename FTraits::StepIdxType step { idx * m_size + m_rank };

      if (step >= max_capacity) {
        std::lock_guard<std::mutex> lock(trigger_mutex);

        idx = local_idx.load(std::memory_order_acquire);
        step = idx * m_size + m_rank;

        if (step >= max_capacity) {
          if (!trigger()) {
            return FTraits::ExhaustedSentinel;
          }

          step = idx * m_size + m_rank;
          if (step >= max_capacity) {
            return FTraits::ExhaustedSentinel;
          }
        }
      }

      while (step < max_capacity) {
        if (local_idx.compare_exchange_weak(idx, idx + 1, std::memory_order_acq_rel)) {
          return step;
        }
        step = idx * m_size + m_rank;
      }

      return FTraits::ExhaustedSentinel;
    }

  private:
    /**
     * Communicator used when generating shareable buffers.
     */
    static inline MPI_Comm m_shmem_comm { MPI_COMM_NULL };
    static inline int m_rank { -1 };     ///< This processes rank in the MPI world.
    static inline int m_size { -1 };     ///< The size of the MPI world.
    static inline std::mutex m_io_mutex; ///< Mutex for locking threaded IO fetches
  };
} // namespace sbio

#endif // SBIO_EXECUTION_MPI_THREADED_HH
