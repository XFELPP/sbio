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

#ifndef SBIO_EXECUTION_SERIAL_HH
#define SBIO_EXECUTION_SERIAL_HH

#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/core/storage.hh"
#include "sbio/execution/composed.hh"
#include "sbio/execution/host_memory.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/storage/host_buffer.hh"

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace sbio {
  /**
   * SerialDistribution is a simple distribution using a single process and thread.
   *
   * This distribution manager provides purely synchronous behaviour. Every buffer in
   * the manager is a shared buffer (there's only 1 parallel unit).
   *
   * The `SerialExecution` alias below composes this manager with host-memory.
   */
  class SerialDistribution {
  public:

    struct Config {};

    static constexpr std::bitset<
      static_cast<std::size_t>(ParallelizationMethods::NUM_METHODS)
    > ParallelSupport { 0x0 }; // 0b00 - NONE

    static void configure_impl(const Config&) {}

    class IterationState {
      friend class SerialDistribution;

      std::size_t m_event_idx { 0 }; ///< The event/step index counter for distribution.
    };

    template <class T>
    struct SegmentState {
      mutable T value {};
      T& get() const { return value; }
    };

    /**
     * Setup a spdlog logger.
     */
    static void init_logger() {
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::SerialExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::SerialExecution");
      } else {
        m_logger = logger;
      }
    }

    /**
     * The SerialExecution policy generates step indices in monotonically.
     *
     * The policy will continuously increment the step counter, returning the
     * next index until exhaustion. It will then use the callback trigger to
     * reindex.
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
      if (state.m_event_idx >= max_capacity) {
        if (!trigger()) {
          m_logger->debug("Trigger returned exhausted: "
                          "max_cap = {}, local_idx = {}",
                          max_capacity,
                          state.m_event_idx);
          return FTraits::ExhaustedSentinel;
        }

        if (state.m_event_idx >= max_capacity) {
          m_logger->debug("Index exceeded capacity: "
                          "max_cap = {}, local_idx = {}",
                          max_capacity,
                          state.m_event_idx);
          return FTraits::ExhaustedSentinel;
        }
      }

      return state.m_event_idx++;
    }

    /**
     * The SerialExecution policy hands out contiguous batches of steps.
     *
     * Like `next_impl`, the reindexing trigger is called once capacity is reached.
     * @note A batch is provided with a maximum size - it may be smaller, as it will
     *       not extend beyond the current maximum capactiy.
     *
     * @tparam FTraits The data-format traits.
     * @tparam IndexTrigger The type of the reindex callback trigger.
     * @param[in/out] state The iteration state held by the caller (i.e. DataSource)
     * @param[in] batch_size The maximum number of steps in the batch.
     * @param[in] max_capacity The current max capacity (currently available indices).
     * @param[in] trigger The reindex callback routine.
     * @returns The next batch of steps.
     */
    template <class FTraits, class IndexTrigger>
    static StepBatch<typename FTraits::StepIdxType>
    next_batch_impl(IterationState& state,
                    std::size_t batch_size,
                    typename FTraits::StepIdxType& max_capacity,
                    IndexTrigger&& trigger) {
      using StepIdx = typename FTraits::StepIdxType;

      const StepIdx first { next_impl<FTraits>(state, max_capacity, trigger) };
      if (first == FTraits::ExhaustedSentinel) {
        return { FTraits::ExhaustedSentinel, FTraits::ExhaustedSentinel };
      }

      const StepIdx want { first + static_cast<StepIdx>(batch_size > 0 ? batch_size : 1) };
      const StepIdx last { want < max_capacity ? want : max_capacity };
      state.m_event_idx = last;

      return { first, last };
    }

  private:
    static inline std::shared_ptr<spdlog::logger> m_logger;
  };

  /**
   * A serial execution policy using host-memory management.
   */
  using SerialExecution = Composed<SerialDistribution, HostMemory>;
} // namespace sbio

#endif // SBIO_EXECUTION_SERIAL_HH
