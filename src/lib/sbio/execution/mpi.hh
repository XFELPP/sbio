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
#include "sbio/core/roles.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/sync.hh"
#include "sbio/storage/host_buffer.hh"
#include "sbio/storage/mpi_shared.hh"
#include "sbio/util/mpi.hh"

#include <mpi.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <bitset>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <thread>
#include <type_traits>
#include <vector>

namespace sbio {
  /**
   * A basic shared MPI execution policy.
   *
   * This policy implements some basic functionality for MPI-based workflows.
   * The main features are:
   * - Allocations for types of the `roles::Index` are made over shared MPI windows.
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
      std::is_same_v<typename Descriptor::role, roles::Index> ||
      std::is_same_v<typename Descriptor::hint, Shareable>,
      MPISharedBuffer,
      HostBuffer
    >;

    struct Config {
      MPI_Comm communicator { MPI_COMM_WORLD };
      std::vector<int> active_ranks {};
      int main_rank { 0 };
      bool main_rank_loops { true };

      int ranks_per_node { 0 };
    };

    static constexpr std::bitset<
      static_cast<std::size_t>(ParallelizationMethods::NUM_METHODS)
    > ParallelSupport { 0x2 }; // 0b10 - MPI

    static void configure_impl(const Config& config) {
      int initialized { 0 };
      MPI_Initialized(&initialized);

      if (!initialized) {
        int argc { 0 };
        char** argv { nullptr };
        MPI_Init(&argc, &argv);
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
        MPI_Allgather(&iterates,
                      1,
                      MPI_INT,
                      m_node_iterates.data(),
                      1,
                      MPI_INT,
                      m_shmem_comm);

        m_node_indexer = 0;
        for (int r = 0; r < m_shmem_size; ++r) {
          if (m_node_iterates[static_cast<std::size_t>(r)]) {
            m_node_indexer = r;
            break;
          }
        }
      }
    }

    template <class T>
    struct SegmentState {
      mutable T value {};
      T& get() const { return value; }
    };

    class IterationState {
    public:
      IterationState()
        : m_tag(next_tag++)
      {}

    private:
      friend class MPIExecution;

      /**
       * The rank-local event/step index counter for distribution.
       */
      std::size_t m_event_idx { 0 };

      /**
       * The first step of current capacity window (for batch distribution).
       */
      std::size_t m_batch_window_start { 0 };

      /**
       * The number of batches distributed for current winodw.
       */
      std::size_t m_batch_round { 0 };

      /**
       * The number of blocks dealt in the windows before the current one.
       */
      std::size_t m_batch_blocks_before { 0 };

      bool m_exhausted { false };     ///< Latch for if the trigger reported no more data.
      bool m_participating { false }; ///< Whether this rank is in a loop with this state.
      bool m_departed { false };      ///< Latch for if this rank left the iteration early.

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
      static inline int next_tag { 0 };
    };


    template <IsRequirementsList Requirements, class IO, class FTraits>
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
    static auto allocate_impl_helper(RequirementsList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      static int new_win_tag { 100 }; // Tag each MPI window to distinguish them
      Storage<RequirementsList<Descriptors...>, MPIExecution> s;
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
          int tag { new_win_tag++ };

          if (m_shmem_comm == MPI_COMM_NULL) {
            // Inactive rank, so no node to share with
            buf.set_memory(new char[sz], sz);
          } else {
            buf.allocate(m_shmem_comm, catalog_offset(sz) + CatalogBytes);
            buf.set_memory(buf.ptr(), sz);
            if (buf.window() != MPI_WIN_NULL) {
              MPI_Win_lock_all(MPI_MODE_NOCHECK, buf.window());
            }
          }
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

      // NOTE: This policy only implements synchronization on Index/Shareable.
      //       DataRole updates (per-step hot path) do NOT synchronize.
      // Check if Windows need synchronization
      if constexpr (std::is_same_v<Role, roles::Index> ||
                    std::is_same_v<Hint, Shareable>) {
        if (m_shmem_comm == MPI_COMM_NULL) {
          return;
        }
        auto fence_win = [](auto& buf) {
          // Check if the buffer type is of one supporting a Window.
          if constexpr (requires { buf.window(); }) {
            if (buf.window() != MPI_WIN_NULL) {
              // If its a Window and has valid memory backing, fence to synchronize.
              MPI_Win_sync(buf.window());
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
     * @param[in/out] state The iteration state held by the caller (i.e. DataSource)
     * @param[in] max_capacity The current max capacity (i.e., already indexed steps).
     * @param[in] trigger A callback to reindex (if appropriate) when capacity is exhausted.
     * @returns The next step to process using the fixed offset of the world size.
     */
    template <class FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType
    next_impl(IterationState& state,
              typename FTraits::StepIdxType& max_capacity,
              IndexTrigger&& trigger) {

      if (!rank_iterates() || !join_iteration(state) || state.m_exhausted) {
        return FTraits::ExhaustedSentinel;
      }

      int worker_count { m_main_rank_loops ? m_active_size : m_active_size - 1 };
      int worker_rank;
      if (m_main_rank_loops || m_active_rank <= m_main_rank) {
        worker_rank = m_active_rank;
      } else {
        worker_rank = m_active_rank - 1;
      }

      auto step = state.m_event_idx + worker_rank;
      state.m_event_idx += worker_count;

      while (step >= max_capacity) {
        // Node only rewrites the window once all the ranks finish with it
        release_window(state);

        if (!trigger()) {
          state.m_exhausted = true;
          m_logger->debug("[Rank {}] Trigger returned exhausted: "
                          "max_cap = {}",
                          m_rank,
                          max_capacity);
          return FTraits::ExhaustedSentinel;
        }
      }

      return step;
    }

    /**
     * Retrieve the next contiguous batch of steps to process.
     *
     * Batches are distributed per capacity window (the steps added by one reindexing),
     * as a batch cannot span a reindexing. Within a window, rank r receives the blocks
     * of `batch_size` steps starting at `window_start + (round * world_size + r) * batch_size`.
     * The last block of a window may be shorter. Once a rank has no block left in the
     * current window it reindexes, and the next window starts at the previous capacity.
     *
     * @note Batches and single steps (`next_impl`) should not be mixed on one
     *       iteration state.
     *
     * @tparam FTraits The FormatTraits for the data format.
     * @tparam IndexTrigger The type of the lambda callback to reindex as needed.
     * @param[in/out] state The iteration state held by the caller (i.e. DataSource)
     * @param[in] batch_size The maximum number of steps in the batch.
     * @param[in] max_capacity The current max capacity (i.e., already indexed steps).
     * @param[in] trigger A callback to reindex (if appropriate) when capacity is exhausted.
     * @returns The next batch of steps for this rank.
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

      if (!rank_iterates() || !join_iteration(state) || state.m_exhausted) {
        return Exhausted;
      }

      const std::size_t worker_count {
        static_cast<std::size_t>(m_main_rank_loops ? m_active_size : m_active_size - 1)
      };
      std::size_t worker_rank;
      if (m_main_rank_loops || m_active_rank <= m_main_rank) {
        worker_rank = static_cast<std::size_t>(m_active_rank);
      } else {
        worker_rank = static_cast<std::size_t>(m_active_rank - 1);
      }

      const std::size_t size { batch_size > 0 ? batch_size : 1 };

      // Blocks are distributed round-robin over the ranks, and continue across windows.
      // This is the rank's first block in the window, and then one block per round
      auto block_first = [&]() {
        const std::size_t offset {
          (worker_rank + worker_count - state.m_batch_blocks_before % worker_count) % worker_count
        };

        return state.m_batch_window_start + (state.m_batch_round * worker_count + offset) * size;
      };

      std::size_t first { block_first() };

      while (first >= static_cast<std::size_t>(max_capacity)) {
        // No block left for this rank in the current window so move to the next window
        std::size_t cap { static_cast<std::size_t>(max_capacity) };
        state.m_batch_blocks_before += (cap - state.m_batch_window_start + size - 1) / size;
        state.m_batch_window_start = cap;
        state.m_batch_round = 0;

        // Node only rewrites the window once all the ranks finish with it
        release_window(state);

        if (!trigger()) {
          state.m_exhausted = true;

          m_logger->debug("[Rank {}] Trigger returned exhausted: "
                          "max_cap = {}",
                          m_rank,
                          max_capacity);
          return Exhausted;
        }

        first = block_first();
      }
      state.m_batch_round++;

      const std::size_t want { first + size };
      const std::size_t last {
        want < static_cast<std::size_t>(max_capacity) ? want : static_cast<std::size_t>(max_capacity)
      };

      return { static_cast<StepIdx>(first), static_cast<StepIdx>(last) };
    }

    /**
     * Exit the iteration with `state`, releasing any held steps.
     *
     * @param[in/out] state The iteration state held by the caller (i.e. DataSource)
     */
    template <class FTraits, class IndexTrigger>
    static void end_iteration_impl(IterationState& state,
                                   typename FTraits::StepIdxType& max_capacity,
                                   IndexTrigger&& trigger) {
      (void)max_capacity;

      if (!state.m_participating) {
        return;
      }
      state.m_participating = false;

      if (state.m_exhausted || state.m_departed) {
        return;
      }
      state.m_departed = true;

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
          state.m_exhausted = true;
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
     * The other ranks of the node tell the node indexing rank that they are done with
     * the window (or left the iteration). The indexing rank waits for all of them.
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
          state.m_node_departed[ur] = 1;
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
      if (state.m_participating) {
        return true;
      }

      if (state.m_departed) {
        return false;
      }

      state.m_participating = true;

      return true;
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
    /**
     * Communicator used when generating shareable buffers.
     */
    static inline MPI_Comm m_shmem_comm { MPI_COMM_NULL };
    static inline int m_shmem_rank { -1 };   ///< This rank within its node (m_shmem_comm).
    static inline int m_shmem_size { 0 };    ///< The number of (active) ranks on this node.
    static inline int m_node_indexer { -1 }; ///< The node rank which indexes for the node.
    /**
     * For each rank on the node, whether it iterates (calls `next`).
     *
     * @note This indicates whether the rank participates in collective reindexing.
     */
    static inline std::vector<int> m_node_iterates {};
    static inline int m_rank { -1 };         ///< This processes rank in the MPI world.
    static inline int m_size { -1 };         ///< The size of the MPI world.

    static inline std::shared_ptr<spdlog::logger> m_logger;
  };
} // namespace sbio

#endif // SBIO_EXECUTION_MPI_HH
