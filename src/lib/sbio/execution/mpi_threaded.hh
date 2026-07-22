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
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
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
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::MPIThreadedExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::MPIThreadedExecution");
      } else {
        m_logger = logger;
      }

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
      static std::atomic<int> next_win_tag(100); // Tag each MPI window to distinguish them
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
          int tag { next_win_tag.fetch_add(1) };

          buf.allocate(m_shmem_comm, sz);
          buf.set_tag(tag);
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

      int tag { 0 };
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
      if (num_fetches == 0) {
        return IOStatus::Success;
      }

      IOStatus status { IOStatus::Success };
      for (std::size_t i = 0; i < num_fetches; ++i) {
        std::lock_guard<std::mutex> lock(m_broker_mutexes[i % 2048]);
        if (auto fetch_status = unit_fetcher(i); fetch_status != IOStatus::Success) {
          status = fetch_status;
          break;
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

      static std::atomic<typename FTraits::StepIdxType> shared_capacity { 0 };
      static std::atomic<bool> exhausted { false };

      while (true) {
        if (exhausted.load(std::memory_order_acquire)) {
          m_logger->debug("[Rank {} - thread {}] Trigger returned exhausted on separate thread: "
                          "shared_cap = {}, max_cap = {}, local_idx = {}",
                          m_rank,
                          std::hash<std::thread::id>{}(std::this_thread::get_id()),
                          shared_capacity.load(),
                          max_capacity,
                          local_idx.load());
          return FTraits::ExhaustedSentinel;
        }

        typename FTraits::StepIdxType idx { local_idx.load(std::memory_order_acquire) };
        typename FTraits::StepIdxType base_step { idx * m_size };
        typename FTraits::StepIdxType step { base_step + m_rank };

        typename FTraits::StepIdxType current_cap =
          shared_capacity.load(std::memory_order_acquire);

        if (base_step >= current_cap) {
          std::lock_guard<std::mutex> lock(trigger_mutex);

          if (exhausted.load(std::memory_order_acquire)) {
            m_logger->debug("[Rank {} - thread {}] Trigger returned exhausted on separate thread: "
                            "shared_cap = {}, max_cap = {}, local_idx = {}",
                            m_rank,
                            std::hash<std::thread::id>{}(std::this_thread::get_id()),
                            shared_capacity.load(),
                            max_capacity,
                            local_idx.load());
            return FTraits::ExhaustedSentinel;
          }

          if (shared_capacity.load(std::memory_order_relaxed) != max_capacity) {
            shared_capacity.store(max_capacity, std::memory_order_release);
          }

          idx = local_idx.load(std::memory_order_acquire);
          base_step = idx * m_size;
          step = base_step + m_rank;
          current_cap = shared_capacity.load(std::memory_order_relaxed);

          if (base_step >= current_cap) {
            m_logger->debug("[Rank {} - thread {}] Entering trigger: "
                            "shared_cap = {}, max_cap = {}, local_idx = {}",
                            m_rank,
                            std::hash<std::thread::id>{}(std::this_thread::get_id()),
                            shared_capacity.load(),
                            max_capacity,
                            local_idx.load());
            if (!trigger()) {
              exhausted.store(true, std::memory_order_release);
              m_logger->debug("[Rank {} - thread {}] Trigger returned exhausted: "
                              "shared_cap = {}, max_cap = {}, local_idx = {}",
                              m_rank,
                              std::hash<std::thread::id>{}(std::this_thread::get_id()),
                              shared_capacity.load(),
                              max_capacity,
                              local_idx.load());
              // Point-to-point binomial tree barrier to synchronize all ranks before exiting next()
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
            shared_capacity.store(max_capacity, std::memory_order_release);
            current_cap = max_capacity;
          }
        }

        while (base_step < current_cap) {
          if (step >= current_cap) {
            if (local_idx.compare_exchange_weak(idx, idx + 1, std::memory_order_acq_rel)) {
              break;
            }
          } else {
            if (local_idx.compare_exchange_weak(idx, idx + 1, std::memory_order_acq_rel)) {
              return step;
            }
          }

          base_step = idx * m_size;
          step = base_step + m_rank;
          current_cap = shared_capacity.load(std::memory_order_acquire);
          if (exhausted.load(std::memory_order_acquire)) {
            m_logger->debug("[Rank {} - thread {}] Returned exhausted after current cap check: "
                            "base_step = {}, step = {}, max_cap = {}, current_cap = {}",
                            m_rank,
                            std::hash<std::thread::id>{}(std::this_thread::get_id()),
                            base_step,
                            step,
                            max_capacity,
                            current_cap);
            return FTraits::ExhaustedSentinel;
          }
        }
      }
    }

  private:
    /**
     * Communicator used when generating shareable buffers.
     */
    static inline MPI_Comm m_shmem_comm { MPI_COMM_NULL };
    static inline int m_rank { -1 };     ///< This processes rank in the MPI world.
    static inline int m_size { -1 };     ///< The size of the MPI world.
    /**
     * Set of mutexes to allow different brokers of a group from different threads
     * to fetch in parallel.
     */
    static inline std::mutex m_broker_mutexes[2048];

    static inline std::shared_ptr<spdlog::logger> m_logger;
  };
} // namespace sbio

#endif // SBIO_EXECUTION_MPI_THREADED_HH
