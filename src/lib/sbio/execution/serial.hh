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
#include "sbio/core/storage.hh"
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
   * SerialExecution is a simple policy, with single-process single-thread host buffers.
   *
   * This execution policy provides purely synchronous behaviour for use with a
   * single buffer in a host-only pipeline. No synchronization is necessary.
   */
  class SerialExecution : public Execution<SerialExecution> {
  public:
    /**
     * SerialExecution's buffer selector always points to HostBuffer.
     *
     * Regardless of role, a `HostBuffer` will always be used for this policy.
     */
    template <typename Descriptor>
    using BufferTypeFor = HostBuffer;

    struct Config {};

    static constexpr std::bitset<
      static_cast<std::size_t>(ParallelizationMethods::NUM_METHODS)
    > ParallelSupport { 0x0 }; // 0b00 - NONE

    static void configure_impl(const Config&) {}

    /**
     * Allocate HostBuffer storage for requested roles.
     *
     * The SerialExecution policy does not perform any complex logic when fulfilling
     * allocation requests. It will serve the request (unless the machine is actually
     * exhausted of memory).
     *
     * @tparam Requirements The per-buffer-role requirement specifications.
     * @tparam FTraits The data-format traits.
     * @param[in] request The allocation request.
     * @returns Allocated storage per the request.
     */
    template <IsTypeList Requirements, class IO, class FTraits>
    requires FormatTraits<FTraits, IO, SerialExecution>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::SerialExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::SerialExecution");
      } else {
        m_logger = logger;
      }
      return allocate_impl_helper(Requirements{}, request);
    }

    template <typename... Descriptors, class FTraits>
    static auto allocate_impl_helper(TypeList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      Storage<TypeList<Descriptors...>, SerialExecution> s;

      std::size_t i { 0 };

      auto make_host_buffers = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;

        std::size_t final_sz { std::max(Descriptor::min_size, request.size_requests[i++]) };
        auto& buf { s.template get<Descriptor>() };

        buf.set_memory(new char[final_sz], final_sz);
      };
      ( (make_host_buffers(std::type_identity<Descriptors>{})), ... );

      return s;
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
     * @param[in] max_capacity The current max capacity (currently available indices).
     * @param[in] trigger The reindex callback routine.
     * @returns The next step_idx.
     */
    template <class FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType
    next_impl(typename FTraits::StepIdxType& max_capacity, IndexTrigger&& trigger) {
      static typename FTraits::StepIdxType event_idx { 0 };

      if (event_idx >= max_capacity) {
        if (!trigger()) {
          m_logger->debug("Trigger returned exhausted: "
                          "max_cap = {}, local_idx = {}",
                          max_capacity,
                          event_idx);
          return FTraits::ExhaustedSentinel;
        }

        if (event_idx >= max_capacity) {
          m_logger->debug("Index exceeded capacity: "
                          "max_cap = {}, local_idx = {}",
                          max_capacity,
                          event_idx);
          return FTraits::ExhaustedSentinel;
        }
      }

      return event_idx++;
    }

  private:
    static inline std::shared_ptr<spdlog::logger> m_logger;
  };
} // namespace sbio

#endif // SBIO_EXECUTION_SERIAL_HH
