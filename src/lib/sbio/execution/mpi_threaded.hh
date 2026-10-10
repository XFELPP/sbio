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
#include "sbio/core/roles.hh"
#include "sbio/core/state_handle.hh"
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
#include <bitset>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <type_traits>
#include <vector>

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
    static constexpr std::size_t MaxInactiveRanks { 1024 };

    template <typename Descriptor>
    using BufferTypeFor = std::conditional_t<
      std::is_same_v<typename Descriptor::role, roles::Index> ||
      std::is_same_v<typename Descriptor::hint, Shareable>,
      MPISharedBuffer,
      std::conditional_t<
        std::is_same_v<typename Descriptor::role, roles::Data> ||
        std::is_same_v<typename Descriptor::role, roles::Table>,
        ThreadLocalBuffer,
        HostBuffer
      >
    >;

    struct Config {
      MPI_Comm communicator { MPI_COMM_WORLD };
      std::vector<int> active_ranks {};
      int main_rank { 0 };
      bool main_rank_loops { true };

      std::size_t num_threads { 0 };
      std::vector<int> cpu_affinities {};

      /**
       * Number of (active) ranks per node, mainly for testing.
       *
       * 0 (the default) detects the nodes with `MPI_COMM_TYPE_SHARED`. A
       * positive value splits the active ranks into consecutive groups of this
       * size instead, which must still be able to share memory (e.g. to emulate
       * several nodes on one host).
       */
      int ranks_per_node { 0 };
    };

    static constexpr std::bitset<
      static_cast<std::size_t>(ParallelizationMethods::NUM_METHODS)
    > ParallelSupport { 0x3 }; // 0b11 - MPI | THREADS

    static void configure_impl(const Config& config) {
      int initialized { 0 };
      MPI_Initialized(&initialized);

      if (!initialized) {
        int argc { 0 };
        char** argv { nullptr };

        int provided;
        MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
        if (provided < MPI_THREAD_MULTIPLE) {
          abort();
        }
      }

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
        MPI_Group_excl(world_group, m_num_inactive_ranks, m_inactive_ranks, &active_group);

        MPI_Comm_create_group(m_world_comm, active_group, 0, &m_active_comm);

        MPI_Group_free(&world_group);
        MPI_Group_free(&active_group);
#endif // _WIN32
      } else {
        MPI_Comm_dup(m_world_comm, &m_active_comm);
      }

      m_main_rank = config.main_rank;
      m_main_rank_loops = config.main_rank_loops;

      m_shmem_rank = -1;
      m_shmem_size = 0;

      m_node_indexer = -1;
      m_node_iterates.clear();

      if (m_active_comm != MPI_COMM_NULL) {
        MPI_Comm_rank(m_active_comm, &m_active_rank);
        MPI_Comm_size(m_active_comm, &m_active_size);

        if (config.ranks_per_node > 0) {
          MPI_Comm_split(m_active_comm,
                         m_active_rank / config.ranks_per_node,
                         m_active_rank,
                         &m_shmem_comm);
        } else {
          MPI_Comm_split_type(m_active_comm,
                              MPI_COMM_TYPE_SHARED,
                              0,
                              MPI_INFO_NULL,
                              &m_shmem_comm);
        }

        MPI_Comm_rank(m_shmem_comm, &m_shmem_rank);
        MPI_Comm_size(m_shmem_comm, &m_shmem_size);

        // Determine and setup which ranks on the node iterate (distribute step indices with next)
        // The `indexing rank` is the first rank of the node - it must reindex (even
        // if it exits) as long as anybody is still iterating
        int iterates { rank_iterates() ? 1 : 0 };
        m_node_iterates.assign(static_cast<std::size_t>(m_shmem_size), 0);
        MPI_Allgather(&iterates, 1, MPI_INT, m_node_iterates.data(), 1, MPI_INT, m_shmem_comm);

        m_node_indexer = 0; // Even if no rank on the node iterates, index for discovery
        for (int rank = 0; rank < m_shmem_size; ++rank) {
          if (m_node_iterates[static_cast<std::size_t>(rank)]) {
            m_node_indexer = rank;
            break;
          }
        }
      }
    }

    template <class T>
    struct SegmentState {
      SegmentState()
        : id(next_id.fetch_add(1, std::memory_order_relaxed))
      {}

      T& get() const {
        thread_local std::vector<std::pair<std::uint64_t, T>> registry;
        for (auto& [key, v] : registry) {
          if (key == id) {
            return v;
          }
        }

        return registry.emplace_back(id, T{}).second;
      }

      std::uint64_t id;
      static inline std::atomic<std::uint64_t> next_id { 1 };
    };

    class IterationState {
    public:
      IterationState()
        : m_tag(next_tag.fetch_add(1, std::memory_order_relaxed))
      {}

      // Make move for Python bindings, handling atomics
      IterationState(IterationState&& other) noexcept
        : m_shared_capacity(other.m_shared_capacity.load())
        , m_exhausted(other.m_exhausted.load())
        , m_in_flight(other.m_in_flight.load())
        , m_holding_step(other.m_holding_step)
        , m_batch_round(other.m_batch_round.load())
        , m_batch_round_base(other.m_batch_round_base.load())
        , m_batch_window_start(other.m_batch_window_start.load())
        , m_batch_blocks_before(other.m_batch_blocks_before.load())
        , m_window_version(other.m_window_version.load())
        , m_active_units(other.m_active_units.load())
        , m_participating(other.m_participating)
        , m_departed(other.m_departed.load())
        , m_node_departed(other.m_node_departed)
        , m_tag(other.m_tag)
      {}

      IterationState& operator=(IterationState&& other) noexcept {
        if (this != &other) {
          m_shared_capacity.store(other.m_shared_capacity.load());
          m_exhausted.store(other.m_exhausted.load());
          m_in_flight.store(other.m_in_flight.load());
          m_holding_step = other.m_holding_step;
          m_batch_round.store(other.m_batch_round.load());
          m_batch_round_base.store(other.m_batch_round_base.load());
          m_batch_window_start.store(other.m_batch_window_start.load());
          m_batch_blocks_before.store(other.m_batch_blocks_before.load());
          m_window_version.store(other.m_window_version.load());
          m_active_units.store(other.m_active_units.load());
          m_participating = other.m_participating;
          m_departed.store(other.m_departed.load());
          m_node_departed = other.m_node_departed;
          m_tag = other.m_tag;
        }

        return *this;
      }

    private:
      friend class MPIThreadedExecution;

      /**
       * Intra-rank capacity store for inter-thread synchronization of indices.
       */
      std::atomic<std::size_t> m_shared_capacity { 0 };
      /**
       * Latch for if the rank has exhausted all indices.
       */
      std::atomic<bool> m_exhausted { false };

      /**
       * The number of steps handed out by next() and not yet finished processing.
       *
       * A thread returns a step at the start of each next() call.
       */
      std::atomic<std::size_t> m_in_flight { 0 };

      /**
       * Mutex for thread synchronization on reindexing
       */
      std::mutex m_trigger_mutex;

      SegmentState<bool> m_holding_step; ///< Whether this thread is holding a step.

      /**
       * The round counter for the rank.
       */
      std::atomic<std::size_t> m_batch_round { 0 };
      /**
       * The round when the capacity window started (was refilled).
       */
      std::atomic<std::size_t> m_batch_round_base { 0 };
      /**
       * The first step of the window.
       */
      std::atomic<std::size_t> m_batch_window_start { 0 };
      /**
       * The number dealt in the window before the current one.
       */
      std::atomic<std::size_t> m_batch_blocks_before { 0 };
      /**
       * Odd while a thread changes the window above (with m_shared_capacity).
       */
      std::atomic<std::size_t> m_window_version { 0 };

      /**
       * The number of this rank's threads currently iterating with this state.
       */
      std::atomic<std::size_t> m_active_units { 0 };
      SegmentState<bool> m_participating; ///< Whether this thread is iterating.

      /**
       * Latch for if this rank left the iteration early (all its threads left).
       */
      std::atomic<bool> m_departed { false };

      /**
       * Which node ranks left the iteration early.
       *
       * @note For the node's indexing rank only
       */
      std::vector<char> m_node_departed {};

      /**
       * Message tag of this iteration (in the order states are created on each rank).
       */
      int m_tag { 0 };
      static inline std::atomic<int> next_tag { 0 };
    };

    template <IsRequirementsList Requirements, class IO, class FTraits>
    requires FormatTraits<FTraits, IO, MPIThreadedExecution>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::MPIThreadedExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::MPIThreadedExecution");
      } else {
        m_logger = logger;
      }

      return allocate_impl_helper(Requirements{}, request);
    }

    template <typename... Descriptors, class FTraits>
    static auto allocate_impl_helper(RequirementsList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      static std::atomic<int> next_win_tag(100); // Tag each MPI window to distinguish them
      Storage<RequirementsList<Descriptors...>, MPIThreadedExecution> s;
      std::size_t i { 0 };

      if (m_active_comm == MPI_COMM_NULL) {
        // Configuration was forgotten - force a default config now or will seg fault
        Config def_cfg {};
        configure(def_cfg);
      }

      auto make_window_or_local = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;
        std::size_t sz { std::max(Descriptor::min_size, request.size_requests[i++]) };
        auto& buf { s.template get<Descriptor>() };

        using BufRole = typename Descriptor::role;
        using BufHint = typename Descriptor::hint;

        if constexpr (std::is_same_v<BufRole, roles::Index> ||
                      std::is_same_v<BufHint, Shareable>) {
          int tag { next_win_tag.fetch_add(1) };

          if (m_shmem_comm == MPI_COMM_NULL) {
            // Inactive rank, so no node to share with
            buf.set_memory(new char[sz], sz);
          } else {
            // Reserve the catalog area after the buffer (see `catalog_area`)
            buf.allocate(m_shmem_comm, catalog_offset(sz) + CatalogBytes);
            buf.set_memory(buf.ptr(), sz);
            if (buf.window() != MPI_WIN_NULL) {
              MPI_Win_lock_all(MPI_MODE_NOCHECK, buf.window());
            }
          }
          buf.set_tag(tag);
        } else if constexpr (std::is_same_v<BufRole, roles::Data> ||
                             std::is_same_v<BufRole, roles::Table>) {
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

      // NOTE: This policy only implements synchronization on Index/Shareable.
      //       DataRole updates (per-step hot path) do NOT synchronize.
      if constexpr (std::is_same_v<Role, roles::Index> ||
                    std::is_same_v<Hint, Shareable>) {
        if (m_shmem_comm == MPI_COMM_NULL) {
          return;
        }

        auto fence_win = [](auto& buf) {
          if constexpr (requires { buf.window(); }) {
            if (buf.window() != MPI_WIN_NULL) {
              MPI_Win_sync(buf.window());
            }
          }
        };
        storage.template for_each_role<Role>(fence_win);
      }
    }

    /**
     * Check on if indexing should be done by the Broker.
     *
     * File indexing is performed by one rank on each individual node (machine/server).
     * The first rank of the node (rank 0, or lowest, of a node-wide communicator)
     * fills the indexing role.
     *
     * The EPolicy provides a shared-memory buffer for the index storage, so the other
     * ranks can read what the indexer puts into it.
     *
     * @note A rank without their own node-index, still then create and own a
     *       private storage buffer for the offsets.
     *
     * @returns `true` for the node's indexing rank (or an inactive rank), else `false`.
     */
    static bool should_index_impl() {
      return (m_shmem_comm == MPI_COMM_NULL) || is_node_indexer();
    }

    /**
     * After updates, synchronize shared resources.
     *
     * Paired with the pre_update hook, the post_update ensures the synchronization
     * of `IndexRole` storage is completed.
     *
     * Remaining `sync_vars` (including index catalog) get shared through the window
     * as well, via a special catalog area. The catalog area is setup by the indexer
     * for the node, and then it publishes a new sequence number. The other ranks can
     * read back after this. (No additional messages are passed, so communication
     * cannot end up causing a stall if a rank leaves early.)
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

      // NOTE: This policy only implements synchronization on Index/Shareable.
      //       DataRole updates (per-step hot path) do NOT synchronize.
      if constexpr (std::is_same_v<Role, roles::Index> ||
                    std::is_same_v<Hint, Shareable>) {
        if (m_shmem_comm == MPI_COMM_NULL) {
          return;
        }

        auto& buf = storage.template get<Role>();
        const std::uint64_t published { static_cast<std::uint64_t>(buf.fetch_next_seq()) + 1 };
        auto* area { catalog_area(buf) };
        std::atomic_ref<std::uint64_t> seq { *reinterpret_cast<std::uint64_t*>(area) };
        char* bytes { area + CatalogHeaderBytes };

        if (is_node_indexer()) {
          // Indexer updates the catalog
          std::size_t pos { 0 };
          auto copy_into_window = [&](auto& var) {
            if (pos + sizeof(var) > CatalogBytes - CatalogHeaderBytes) {
              m_logger->error("Catalog does not fit the reserved window area ({} bytes)",
                              CatalogBytes);
              MPI_Abort(m_world_comm, 1);
            }
            std::memcpy(bytes + pos, &var, sizeof(var));
            pos += sizeof(var);
          };
          sync_vars.for_each(copy_into_window);

          MPI_Win_sync(buf.window());
          seq.store(published, std::memory_order_release);
          MPI_Win_sync(buf.window());
        } else {
          MPI_Win_sync(buf.window());
          while (seq.load(std::memory_order_acquire) < published) {
            std::this_thread::yield();
            MPI_Win_sync(buf.window());
          }

          std::size_t pos { 0 };
          auto copy_from_window = [&](auto& var) {
            std::memcpy(&var, bytes + pos, sizeof(var));
            pos += sizeof(var);
          };
          sync_vars.for_each(copy_from_window);
        }
      }
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
     * The MPIThreadedExecution policy multi-contiguous step fetch and get.
     *
     * @tparam FTraits The data-format traits.
     * @tparam FetchCBType The type of the BrokerGroup callback for data fetching.
     * @tparam GetCBType The type of the BrokerGroup callback for data resolution.
     * @param[in] steps The steps to read { start, stop }. Currently adding a third step
     *            for strided access is not yet supported.
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
    static IOStatus get_data_steps_impl(const std::initializer_list<typename FTraits::StepIdxType> steps,
                                        FetchCBType&& unit_fetcher,
                                        std::size_t num_fetches,
                                        GetCBType&& unit_get_data,
                                        std::size_t num_accesses) {
      if (num_fetches == 0) {
        return IOStatus::Success;
      }
      IOStatus status { IOStatus::Success };
      for (std::size_t i = 0; i < num_fetches; ++i) {
        if (auto fetch_status = unit_fetcher(i); fetch_status != IOStatus::Success) {
          status = fetch_status;
          break;
        }
      }
      if (status == IOStatus::Success) {
        bool passed_step { steps.size() == 3 };
        typename FTraits::StepIdxType first { *steps.begin() };
        typename FTraits::StepIdxType last { passed_step ? *(steps.end() - 2) : *(steps.end() - 1) };
        std::size_t count { (last > first) ? static_cast<std::size_t>(last - first) : 1 };
        for (std::size_t cnt = 0; cnt < count; ++cnt) {
          for (std::size_t seg = 0; seg < num_accesses; ++seg) {
            unit_get_data(seg, cnt);
          }
        }
      }

      return status;
    }

    static bool is_current_rank_inactive() {
      for (std::size_t i = 0; i < m_num_inactive_ranks; ++i) {
        if (m_rank == m_inactive_ranks[i]) {
          return true;
        }
      }
      return false;
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
     * @param[in/out] state The iteration state held by the caller (i.e. DataSource)
     * @param[in] max_capacity The current max capacity (currently available indices).
     * @param[in] trigger The reindex callback routine.
     * @returns The next step_idx.
     */
    template <class FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType
    next_impl(IterationState& state,
              typename FTraits::StepIdxType& max_capacity,
              IndexTrigger&& trigger) {
      // A single step is just a batch of 1 - steps are round-robined within the
      // capacity window
      return next_batch_impl<FTraits>(state,
                                      1,
                                      max_capacity,
                                      std::forward<IndexTrigger>(trigger)).first;
    }

    /**
     * The MPIThreadedExecution policy generates contiguous batches of step indices.
     *
     * Each capacity window [window start, capacity) is cut into blocks of `batch_size`
     * steps, which are dealt round-robin over the iterating ranks. The dealing continues
     * across windows: the `g`-th block overall goes to the worker `g % worker_count`.
     * The threads of a rank share a round counter, and round `j` of the window gives
     * the rank its `j`-th block of the window. The final block of a window may be
     * shorter.
     *
     * Reindexing follows `next_impl`, under the trigger mutex, after the rank's
     * handed out batches are returned and the node released the window.
     *
     * @tparam FTraits The data-format traits.
     * @tparam IndexTrigger The type of the reindex callback trigger.
     * @param[in/out] state The iteration state held by the caller (i.e. DataSource)
     * @param[in] batch_size The maximum size of a batch.
     * @param[in] max_capacity The current max capacity (currently available indices).
     * @param[in] trigger The reindex callback routine.
     * @returns The next batch [first, last), or the Exhausted batch.
     */
    template <class FTraits, class IndexTrigger>
    static StepBatch<typename FTraits::StepIdxType>
    next_batch_impl(IterationState& state,
                    std::size_t batch_size,
                    typename FTraits::StepIdxType& max_capacity,
                    IndexTrigger&& trigger) {
      using StepIdx = typename FTraits::StepIdxType;
      constexpr StepBatch<StepIdx> Exhausted {
        FTraits::ExhaustedSentinel,
        FTraits::ExhaustedSentinel
      };

      if (!rank_iterates() || !join_iteration(state)) {
        return Exhausted;
      }

      release_step(state);

      const std::size_t size { batch_size > 0 ? batch_size : 1 };
      const std::size_t worker_count {
        static_cast<std::size_t>(m_main_rank_loops ? m_active_size : m_active_size - 1)
      };
      std::size_t worker_rank;
      if (m_main_rank_loops || m_active_rank <= m_main_rank) {
        worker_rank = static_cast<std::size_t>(m_active_rank);
      } else {
        worker_rank = static_cast<std::size_t>(m_active_rank - 1);
      }

      // The rank's first step for `round`, from a consistent view of the window.
      // Returns false if the window is being changed (retry).
      auto block_first = [&](std::size_t round, std::size_t& first, std::size_t& cap) -> bool {
        const std::size_t version { state.m_window_version.load(std::memory_order_acquire) };
        if (version & 1) {
          return false;
        }
        cap = state.m_shared_capacity.load(std::memory_order_relaxed);
        const std::size_t start { state.m_batch_window_start.load(std::memory_order_relaxed) };
        const std::size_t base { state.m_batch_round_base.load(std::memory_order_relaxed) };
        const std::size_t before { state.m_batch_blocks_before.load(std::memory_order_relaxed) };

        std::atomic_thread_fence(std::memory_order_acquire);

        if (state.m_window_version.load(std::memory_order_relaxed) != version ||
            round < base) {
          return false;
        }

        // The rank's first block in the window, then one block per round
        const std::size_t offset {
          (worker_rank + worker_count - before % worker_count) % worker_count
        };

        first = start + ((round - base) * worker_count + offset) * size;

        return true;
      };

      while (true) {
        if (state.m_exhausted.load(std::memory_order_acquire)) {
          return Exhausted;
        }

        std::size_t round { state.m_batch_round.load(std::memory_order_acquire) };
        std::size_t first;
        std::size_t current_cap;
        if (!block_first(round, first, current_cap)) {
          std::this_thread::yield();
          continue;
        }

        if (first >= current_cap) {
          std::lock_guard<std::mutex> lock(state.m_trigger_mutex);

          if (state.m_exhausted.load(std::memory_order_acquire)) {
            return Exhausted;
          }

          if (state.m_shared_capacity.load(std::memory_order_relaxed) != max_capacity) {
            // Capacity indexed before the iteration started: same window, larger
            // Capacity was indexed before the iteration started - so same window, but larger.
            auto update_capacity = [&]() {
              state.m_shared_capacity.store(max_capacity, std::memory_order_relaxed);
            };
            publish_window(state, update_capacity);
          }

          round = state.m_batch_round.load(std::memory_order_acquire);
          if (block_first(round, first, current_cap) && first >= current_cap) {
            while (state.m_in_flight.load(std::memory_order_acquire) != 0) {
              std::this_thread::yield(); // Wait for all handed out batches to be returned
            }

            // The node may only rewrite the window once all its ranks are done with it
            release_window(state);

            if (!trigger()) {
              state.m_exhausted.store(true, std::memory_order_release);
              m_logger->debug("[Rank {} - thread {}] Trigger returned exhausted: "
                              "max_cap = {}, round = {}",
                              m_rank,
                              std::hash<std::thread::id>{}(std::this_thread::get_id()),
                              max_capacity,
                              round);
              return Exhausted;
            }

            // Starting a new window over [current_cap, max_cap) at this round
            auto new_window_for_round = [&]() {
              const std::size_t start { state.m_batch_window_start.load(std::memory_order_relaxed) };
              const std::size_t blocks { (current_cap - start + size - 1) / size };
              state.m_batch_blocks_before.fetch_add(blocks, std::memory_order_relaxed);
              state.m_batch_window_start.store(current_cap, std::memory_order_relaxed);
              state.m_batch_round_base.store(round, std::memory_order_relaxed);
              state.m_shared_capacity.store(max_capacity, std::memory_order_relaxed);
            };
            publish_window(state, new_window_for_round);
          }

          continue;
        }

        // Claim before actually taking the batch, so triggers aren't missed
        state.m_in_flight.fetch_add(1);
        if (state.m_batch_round.compare_exchange_strong(round, round + 1,
                                                        std::memory_order_acq_rel)) {
          state.m_holding_step.get() = true;
          const std::size_t last { std::min(first + size, current_cap) };
          return { static_cast<StepIdx>(first), static_cast<StepIdx>(last) };
        }
        state.m_in_flight.fetch_sub(1); // Didn't get a batch
      }
    }

    /**
     * Exit the iteration with `state`, releasing any held steps.
     *
     * When the last iterating thread of the rank leaves, before data is exhuasted,
     * the rank is marked as departed and no longer iterates. Any calls on `next`
     * will just return exhausted - so the node doesn't wait for it during reindexing.
     * However, a node *INDEXER* will keep reindexing while any other rank on the node
     * is around, or until the data is fully exhausted.
     *
     * @tparam FTraits The data format type.
     * @tparam IndexTrigger The type of the reindex callback trigger.
     * @param[in/out] state The iteration state held by the caller (i.e. DataSource)
     * @param[in] max_capacity The current max capacity (of step indices).
     * @param[in] trigger The reindex callback trigger.
     */
    template <class FTraits, class IndexTrigger>
    static void end_iteration_impl(IterationState& state,
                                   typename FTraits::StepIdxType& max_capacity,
                                   IndexTrigger&& trigger) {
      (void)max_capacity;
      release_step(state);

      bool& participating { state.m_participating.get() };
      if (!participating) {
        return;
      }
      participating = false;

      // Proceeds under a mutex so that a thread cannot join while the rank departs
      // --> See join_iteration
      std::lock_guard<std::mutex> lock(state.m_trigger_mutex);
      if (--state.m_active_units != 0) {
        return; // Other threads of this rank are still iterating
      }

      if (state.m_exhausted.load(std::memory_order_acquire) ||
          state.m_departed.load(std::memory_order_acquire)) {
        return;
      }
      state.m_departed.store(true, std::memory_order_release);

      if (m_shmem_comm == MPI_COMM_NULL || m_shmem_size < 2) {
        return;
      }

      if (!is_node_indexer()) {
        int msg { NodeDeparted };
        MPI_Send(&msg, 1, MPI_INT, m_node_indexer, node_tag(state), m_shmem_comm);
        return;
      }

      // While others on the node are around, keep reindexing for them
      while (others_iterating(state)) {
        release_window(state);
        if (!others_iterating(state)) {
          break;
        }
        if (!trigger()) {
          state.m_exhausted.store(true, std::memory_order_release);
          break;
        }
      }
    }

  private:
    static constexpr std::size_t CatalogBytes { 4096 };     ///< Window area for sync_vars.
    static constexpr std::size_t CatalogHeaderBytes { 64 }; ///< Sequence number slot.

    /**
     * Offset of the catalog area after a shared buffer of `size` bytes.
     */
    static constexpr std::size_t catalog_offset(std::size_t size) {
      return (size + 63) & ~static_cast<std::size_t>(63);
    }

    /**
     * The catalog used by the processing ranks (inset into the rest of the buffer).
     */
    template <class BufT>
    static char* catalog_area(BufT& buf) {
      return static_cast<char*>(buf.ptr()) + catalog_offset(buf.size());
    }

    /**
     * Whether this rank takes steps (calls `next`) at all.
     */
    static bool rank_iterates() {
      return !is_current_rank_inactive() && !(!m_main_rank_loops && m_rank == m_main_rank);
    }

    /**
     * Change the capacity window of `state` (under the trigger mutex).
     *
     * Threads reading the window see either the old or the new one, never a mix.
     */
    template <class Update>
    static void publish_window(IterationState& state, Update&& update) {
      const std::size_t version { state.m_window_version.load(std::memory_order_relaxed) };
      state.m_window_version.store(version + 1, std::memory_order_relaxed);

      std::atomic_thread_fence(std::memory_order_release);

      update();
      state.m_window_version.store(version + 2, std::memory_order_release);
    }

    static bool is_node_indexer() {
      return m_shmem_rank == m_node_indexer;
    }

    static constexpr int NodeRelease { 1 };  ///< Done with the window - may be rewritten.
    static constexpr int NodeDeparted { 2 }; ///< Left the iteration early.

    /**
     * Message tag of the node messages of an iteration.
     *
     * @note Iteration states are expected to be created in the same order on each rank.
     */
    static int node_tag(const IterationState& state) {
      return 1000 + state.m_tag % 30000;
    }

    /**
     * Release the current index window of the node before it is rewritten.
     *
     * The other ranks of the node tell the node's indexer that they are done with
     * the window (or left the iteration entirely). The indexer waits for all of them.
     * Called after the rank drained its handed out steps.
     */
    static void release_window(IterationState& state) {
      if (m_shmem_comm == MPI_COMM_NULL || m_shmem_size < 2) {
        return;
      }

      const int tag { node_tag(state) };
      if (!is_node_indexer()) {
        int msg { NodeRelease };
        MPI_Send(&msg, 1, MPI_INT, m_node_indexer, tag, m_shmem_comm);
        return;
      }

      if (state.m_node_departed.size() != static_cast<std::size_t>(m_shmem_size)) {
        state.m_node_departed.assign(static_cast<std::size_t>(m_shmem_size), 0);
      }

      for (int rank = 0; rank < m_shmem_size; ++rank) {
        const auto u_rank { static_cast<std::size_t>(rank) };
        if (rank == m_shmem_rank || !m_node_iterates[u_rank] || state.m_node_departed[u_rank]) {
          continue;
        }

        int msg { 0 };
        MPI_Recv(&msg, 1, MPI_INT, rank, tag, m_shmem_comm, MPI_STATUS_IGNORE);
        if (msg == NodeDeparted) {
          state.m_node_departed[u_rank] = 1;
        }
      }
    }

    /**
     * Whether other ranks of the node still iterate.
     *
     * @note Used by the node's indexer rank only.
     */
    static bool others_iterating(const IterationState& state) {
      for (int rank = 0; rank < m_shmem_size; ++rank) {
        const auto u_rank { static_cast<std::size_t>(rank) };
        if (rank == m_shmem_rank || !m_node_iterates[u_rank]) {
          continue;
        }

        if (u_rank >= state.m_node_departed.size() || !state.m_node_departed[u_rank]) {
          return true;
        }
      }

      return false;
    }

    /**
     * Count this thread as iterating with `state` (once per iteration).
     *
     * @note A thread starting after all others of its rank left finds the rank departed.
     * @returns `false` if the rank departed the iteration.
     */
    static bool join_iteration(IterationState& state) {
      bool& participating { state.m_participating.get() };
      if (participating) {
        return true;
      }

      std::lock_guard<std::mutex> lock(state.m_trigger_mutex);
      if (state.m_departed.load(std::memory_order_acquire)) {
        return false;
      }

      participating = true;
      ++state.m_active_units;

      return true;
    }

    static void release_step(IterationState& state) {
      bool& holding { state.m_holding_step.get() };
      if (holding) {
        holding = false;
        state.m_in_flight.fetch_sub(1);
      }
    }

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

    static inline std::size_t m_num_threads { 0 };

    /**
     * Communicator used when generating shareable buffers.
     */
    static inline MPI_Comm m_shmem_comm { MPI_COMM_NULL };
    static inline int m_shmem_rank { -1 };
    static inline int m_shmem_size { 0 };
    static inline int m_node_indexer { -1 }; ///< The node rank which indexes for the node.
    /**
     * For each rank on the node, whether it iterates (calls `next`).
     *
     * @note This indicates whether the rank participates in collective reindexing.
     */
    static inline std::vector<int> m_node_iterates {};
    static inline int m_rank { -1 }; ///< This processes rank in the MPI world.
    static inline int m_size { -1 }; ///< The size of the MPI world.

    static inline std::shared_ptr<spdlog::logger> m_logger; ///< Execution policy logger
  };
} // namespace sbio

#endif // SBIO_EXECUTION_MPI_THREADED_HH
