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

#ifndef SBIO_EXECUTION_THREADED_HH
#define SBIO_EXECUTION_THREADED_HH

#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/roles.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/core/storage.hh"
#include "sbio/storage/host_buffer.hh"
#include "sbio/storage/thread_local_buffer.hh"
#include "sbio/formats/format_traits.hh"

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <bitset>
#include <concepts>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <type_traits>
#include <vector>

namespace sbio {
  /**
   * ThreadedExecution manages a single-process/multi-thread IO strategy.
   *
   * This execution policy the necessary index distribution, and data synchronization
   * when operating brokers in a single process/multi-threaded environment. It is
   * designed to allow data-buffers (`DataRole`), and broker group buffers (`TableRole`)
   * to use a ThreadLocalBuffer allowing for true thread-level parallelism. Other buffer
   * roles are intentionally shared using a simpler HostBuffer which makes them
   * available process wide.
   */
  class ThreadedExecution : public Execution<ThreadedExecution> {
  public:
    template <typename Descriptor>
    using BufferTypeFor = std::conditional_t<
      std::is_same_v<typename Descriptor::role, roles::Data> ||
      std::is_same_v<typename Descriptor::role, roles::Table>,
      ThreadLocalBuffer,
      HostBuffer
    >;

    struct Config {
      std::size_t num_threads { 0 };
      std::vector<int> cpu_affinities {};
    };

    static constexpr std::bitset<
      static_cast<std::size_t>(ParallelizationMethods::NUM_METHODS)
    > ParallelSupport { 0x1 }; // 0b01 - THREADS

    static void configure_impl(const Config& config) {
      m_num_threads = config.num_threads;
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

        return registry.emplace_back(id, T {}).second;
      }

      std::uint64_t id;
      static inline std::atomic<std::uint64_t> next_id { 1 };
    };

    class IterationState {
    public:
      IterationState() = default;

      // Make move for Python bindings, handling atomics
      IterationState(IterationState&& other) noexcept
        : m_event_idx(other.m_event_idx.load())
        , m_shared_capacity(other.m_shared_capacity.load())
        , m_exhausted(other.m_exhausted.load())
        , m_in_flight(other.m_in_flight.load())
        , m_holding_step(other.m_holding_step)
      {}

      IterationState& operator=(IterationState&& other) noexcept {
        if (this != &other) {
          m_event_idx.store(other.m_event_idx.load());
          m_shared_capacity.store(other.m_shared_capacity.load());
          m_exhausted.store(other.m_exhausted.load());
          m_in_flight.store(other.m_in_flight.load());
          m_holding_step = other.m_holding_step;
        }

        return *this;
      }

    private:
      friend class ThreadedExecution;

      /**
       * Thread-local index to index to distribute.
       */
      std::atomic<std::size_t> m_event_idx { 0 };
      /**
       * Capacity store for inter-thread synchronization of indices.
       */
      std::atomic<std::size_t> m_shared_capacity { 0 };
      /**
       * Latch for if exhausted all indices.
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
    };

    /**
     * Allocate ThreadLocalBuffer storage for Index/DataRole and HostBuffer otherwise.
     *
     * Beyond the role-based specialization, this execution policy does not perform
     * any complex calculation when serving allocation requests. If there is memory
     * available to serve the request, it will be fulfilled withou modification.
     *
     * @tparam Requirements The per-buffer-role requirement specifications.
     * @tparam FTraits The data-format traits.
     * @param[in] request The allocation request.
     * @returns Allocated storage per the request.
     */
    template <IsRequirementsList Requirements, class IO, class FTraits>
    requires FormatTraits<FTraits, IO, ThreadedExecution>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::ThreadedExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::ThreadedExecution");
      } else {
        m_logger = logger;
      }

      return allocate_impl_helper(Requirements{}, request);
    }

    template <typename... Descriptors, class FTraits>
    static auto allocate_impl_helper(RequirementsList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      Storage<RequirementsList<Descriptors...>, ThreadedExecution> s;

      std::size_t i { 0 };

      auto make_thread_local_data = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;

        std::size_t final_sz { std::max(Descriptor::min_size, request.size_requests[i++]) };
        auto& buf { s.template get<Descriptor>() };

        using BufRole = typename Descriptor::role;
        if constexpr (std::is_same_v<BufRole, roles::Data> ||
                      std::is_same_v<BufRole, roles::Table>) {
          buf.set_memory(nullptr, final_sz);
        } else {
          buf.set_memory(new char[final_sz], final_sz);
        }
      };
      ( (make_thread_local_data(std::type_identity<Descriptors> {})), ... );

      return s;
    }

    /**
     * The ThreadedExecution policy splits BrokerGroup data fetch and resolution.
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
     * The ThreadedExecution policy multi-contiguous step fetch and get.
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

    /**
     * The ThreadedExecution policy generates step indices in monotonically.
     *
     * As the index generator is shared by all threads in the process, it is
     * generated on an atomic counter. This requires the use of acquire/release
     * semantics, and thus has some ammount of overhead compared to a lock-free
     * policy.
     *
     * Likewise, a lock is explicitly acquired in the event that the index capacity
     * has been reached, and thus a reindexing trigger must be called.
     *
     * @note This policy is intended to be used with indexable formats. This
     * function will work if it is not indexable; however, the callback trigger
     * must then be designed in a way to terminate the generation of indices
     * in some fashion, or it will continue forever.
     *
     * @note A batch is provided with a maximum size - it may be smaller, as it will
     *       not extend beyond the current maximum capactiy.
     *
     * @tparam FTraits The data-format traits.
     * @tparam IndexTrigger The type of the reindex callback trigger.
     * @param[in/out] state The iteration state held by the caller (i.e. DataSource)
     * @param[in] batch_size The *maximum* number of steps in a batch.
     * @param[in] max_capacity The current max capacity (currently available indices).
     * @param[in] trigger The reindex callback routine.
     * @returns The next step_idx.
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

      const StepIdx max_count { static_cast<StepIdx>(batch_size > 0 ? batch_size : 1) };

      release_step(state);

      typename FTraits::StepIdxType current { state.m_event_idx.load(std::memory_order_acquire) };

      while (true) {
        if (state.m_exhausted.load(std::memory_order_acquire)) {
          m_logger->debug("[Thread {}] Trigger returned exhausted on separate thread: "
                          "shared_cap = {}, max_cap = {}, event_idx = {}",
                          std::hash<std::thread::id>{}(std::this_thread::get_id()),
                          state.m_shared_capacity.load(),
                          max_capacity,
                          state.m_event_idx.load());
          return Exhausted;
        }

        typename FTraits::StepIdxType current_cap =
          state.m_shared_capacity.load(std::memory_order_acquire);

        if (current >= current_cap) {
          std::lock_guard<std::mutex> lock(state.m_trigger_mutex);

          if (state.m_exhausted.load(std::memory_order_acquire)) {
            m_logger->debug("[Thread {}] Trigger returned exhausted on separate thread: "
                            "shared_cap = {}, max_cap = {}, event_idx = {}",
                            std::hash<std::thread::id>{}(std::this_thread::get_id()),
                            state.m_shared_capacity.load(),
                            max_capacity,
                            state.m_event_idx.load());
            return Exhausted;
          }

          if (state.m_shared_capacity.load(std::memory_order_relaxed) != max_capacity) {
            state.m_shared_capacity.store(max_capacity, std::memory_order_release);
          }

          current = state.m_event_idx.load(std::memory_order_acquire);
          current_cap = state.m_shared_capacity.load(std::memory_order_relaxed);

          if (current >= current_cap) {
            while (state.m_in_flight.load(std::memory_order_acquire) != 0) {
              std::this_thread::yield(); // Wait for all handed out steps to be returned
            }

            if (!trigger()) {
              state.m_exhausted.store(true, std::memory_order_release);
              m_logger->debug("[Thread {}] Trigger returned exhausted: "
                              "shared_cap = {}, max_cap = {}, event_idx = {}",
                              std::hash<std::thread::id>{}(std::this_thread::get_id()),
                              state.m_shared_capacity.load(),
                              max_capacity,
                              state.m_event_idx.load());
              return Exhausted;
            }

            state.m_shared_capacity.store(max_capacity, std::memory_order_release);
            current_cap = max_capacity;
          }
        }

        // Make sure to stake claim before taking a step to ensure no missed triggers
        state.m_in_flight.fetch_add(1);
        while (current < current_cap) {
          const StepIdx remaining { current_cap - current };
          const StepIdx count { max_count < remaining ? max_count : remaining };
          if (state.m_event_idx.compare_exchange_weak(current, current + count,
                                                      std::memory_order_acq_rel)) {
            state.m_holding_step.get() = true;
            return { current, current + count };
          }

          current_cap = state.m_shared_capacity.load(std::memory_order_acquire);
        }

        // Do NOT return exhausted here. The trigger must be entered in a coordinated
        // fashion
        state.m_in_flight.fetch_sub(1); // Didn't get a step
      }
    }

    /**
     * The ThreadedExecution policy generates step indices in monotonically.
     *
     * @note The ThreadedExecution policy uses the batch implementation with a
     *       batch size of 1.
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
      // Since this API expects a single step index, return just the first of the pair
      return next_batch_impl<FTraits>(state,
                                      1,
                                      max_capacity,
                                      std::forward<IndexTrigger>(trigger)).first;
    }

    /**
     * Exit the iteration with `state`, releasing any held steps.
     *
     * @param[in/out] state The iteration state held by the caller (i.e. DataSource)
     */
    static void end_iteration_impl(IterationState& state) {
      release_step(state);
    }

  private:
    static void release_step(IterationState& state) {
      bool& holding { state.m_holding_step.get() };
      if (holding) {
        holding = false;
        state.m_in_flight.fetch_sub(1);
      }
    }

    static inline std::size_t m_num_threads { 0 };

    static inline std::shared_ptr<spdlog::logger> m_logger;
  };

} // namespace sbio

#endif // SBIO_EXECUTION_THREADED_HH
