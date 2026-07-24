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
#include "sbio/core/storage.hh"
#include "sbio/storage/host_buffer.hh"
#include "sbio/storage/thread_local_buffer.hh"
#include "sbio/formats/format_traits.hh"

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <concepts>
#include <memory>
#include <mutex>
#include <type_traits>

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
      std::is_same_v<typename Descriptor::role, DataRole> ||
      std::is_same_v<typename Descriptor::role, TableRole>,
      ThreadLocalBuffer,
      HostBuffer
    >;

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
    template <IsTypeList Requirements, class IO, class FTraits>
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
    static auto allocate_impl_helper(TypeList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      Storage<TypeList<Descriptors...>, ThreadedExecution> s;

      std::size_t i { 0 };

      auto make_thread_local_data = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;

        std::size_t final_sz { std::max(Descriptor::min_size, request.size_requests[i++]) };
        auto& buf { s.template get<Descriptor>() };

        using BufRole = typename Descriptor::role;
        if constexpr (std::is_same_v<BufRole, DataRole> ||
                      std::is_same_v<BufRole, TableRole>) {
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
    static IOStatus get_data_steps_impl(std::initializer_list<typename FTraits::StepIdxType> steps,
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
     * @tparam FTraits The data-format traits.
     * @tparam IndexTrigger The type of the reindex callback trigger.
     * @param[in] max_capacity The current max capacity (currently available indices).
     * @param[in] trigger The reindex callback routine.
     * @returns The next step_idx.
     */
    template <class FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType
    next_impl(typename FTraits::StepIdxType& max_capacity, IndexTrigger&& trigger) {
      static std::atomic<typename FTraits::StepIdxType> event_idx { 0 };

      static std::mutex trigger_mutex;

      static std::atomic<typename FTraits::StepIdxType> shared_capacity { 0 };
      static std::atomic<bool> exhausted { false };

      typename FTraits::StepIdxType current { event_idx.load(std::memory_order_acquire) };

      while (true) {
        if (exhausted.load(std::memory_order_acquire)) {
          m_logger->debug("[Thread {}] Trigger returned exhausted on separate thread: "
                          "shared_cap = {}, max_cap = {}, event_idx = {}",
                          std::hash<std::thread::id>{}(std::this_thread::get_id()),
                          shared_capacity.load(),
                          max_capacity,
                          event_idx.load());
          return FTraits::ExhaustedSentinel;
        }

        typename FTraits::StepIdxType current_cap =
          shared_capacity.load(std::memory_order_acquire);

        if (current >= current_cap) {
          std::lock_guard<std::mutex> lock(trigger_mutex);

          if (exhausted.load(std::memory_order_acquire)) {
            m_logger->debug("[Thread {}] Trigger returned exhausted on separate thread: "
                            "shared_cap = {}, max_cap = {}, event_idx = {}",
                            std::hash<std::thread::id>{}(std::this_thread::get_id()),
                            shared_capacity.load(),
                            max_capacity,
                            event_idx.load());
            return FTraits::ExhaustedSentinel;
          }

          if (shared_capacity.load(std::memory_order_relaxed) != max_capacity) {
            shared_capacity.store(max_capacity, std::memory_order_release);
          }

          current = event_idx.load(std::memory_order_acquire);
          current_cap = shared_capacity.load(std::memory_order_relaxed);

          if (current >= current_cap) {
            if (!trigger()) {
              exhausted.store(true, std::memory_order_release);
              m_logger->debug("[Thread {}] Trigger returned exhausted: "
                              "shared_cap = {}, max_cap = {}, event_idx = {}",
                              std::hash<std::thread::id>{}(std::this_thread::get_id()),
                              shared_capacity.load(),
                              max_capacity,
                              event_idx.load());
              return FTraits::ExhaustedSentinel;
            }

            shared_capacity.store(max_capacity, std::memory_order_release);
            current_cap = max_capacity;
          }
        }

        while (current < current_cap) {
          if (event_idx.compare_exchange_weak(current, current + 1, std::memory_order_acq_rel)) {
            return current;
          }

          current_cap = shared_capacity.load(std::memory_order_acquire);
        }

        // Do NOT return exhausted here. The trigger must be entered in a coordinated
        // fashion
      }
    }

  private:
    /**
     * Set of mutexes to allow different brokers of a group from different threads
     * to fetch in parallel.
     */
    static inline std::mutex m_broker_mutexes[2048];

    static inline std::shared_ptr<spdlog::logger> m_logger;
  };

} // namespace sbio

#endif // SBIO_EXECUTION_THREADED_HH
