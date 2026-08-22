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

#ifndef SBIO_CORE_TOPOLOGY_HH
#define SBIO_CORE_TOPOLOGY_HH

#include "sbio/core/broker.hh"
#include "sbio/core/types.hh"

#ifdef __CUDACC__

#include <cuda/std/array>
#include <cuda/std/cstddef>
#include <cuda/std/cstdint>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <array>
#include <cstddef>
#include <cstdint>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

namespace sbio {
  /**
   * Map a StreamBroker to a logical position within a BrokerGroup.
   */
  template <IsStreamBroker BrokerType>
  struct SegmentRef {
    /**
     * The type of data being read.
     */
    using DataFormat = typename BrokerType::DataFormat;

    /**
     * The type of the enumerator used to specify access patterns used for the format.
     */
    using DataAccessPtn = typename DataFormat::DataAccessPtn;

    /**
     * The pointer to the StreamBroker where you find this logical segment.
     */
    BrokerType* broker { nullptr };
    /**
     * The identifier provided by the actual data format specification.
     *
     * The format identifier does not necessarily equal the logical_slot. In some cases,
     * the format may not even provide an identifier.
     */
    hd_std::uint32_t format_segment_id { 0 };
    /**
     * The format-specific access strategy used to access this logical segment.
     *
     * Some formats expose multiple mechanisms to traverse data. The access pattern
     * controls which mechanism to use.
     */
    DataAccessPtn access_ptn;
    /**
     * The logical order of the segment within a BrokerGroup.
     */
    hd_std::uint32_t logical_slot { 0 };
  };

  /**
   * The layout, post any sorting, of all segment references in a BrokerGroup.
   *
   * The topology organization is established once during the DISCOVERY state
   * transition when metadata is parsed.
   */
  template <IsStreamBroker BrokerType, hd_std::size_t MaxSegments = 128>
  struct GroupTopology {
    /**
     * The type of data being read.
     */
    using DataFormat = typename BrokerType::DataFormat;

    /**
     * The type of the enumerator used to specify access patterns used for the format.
     */
    using DataAccessPtn = typename DataFormat::DataAccessPtn;

    /**
     * The type used to request a specific step from the Stream.
     *
     * This type is required and guaranteed to be convertible std::size_t; however,
     * different data format's may use different underlying types.
     */
    using StepIdxType = typename DataFormat::StepIdxType;

    static constexpr StreamPartitioningStrategy strategy {
      DataFormat::PartitioningStrategy
    };

    char group_name[256] { 0 };
    char group_type[256] { 0 };

    hd_std::array<BrokerType*, MaxSegments> stream_brokers { nullptr };
    hd_std::array<DataAccessPtn, MaxSegments> broker_access_ptns {};
    hd_std::size_t num_stream_brokers { 0 };

    hd_std::array<SegmentRef<BrokerType>, MaxSegments> segments {};
    hd_std::size_t num_segments { 0 };

    bool empty() const { return num_segments == 0; }

    // --- Strategy and traits aware getters --- //
    /**
     * Retrieve the correct, active, StreamBroker for a specified step.
     *
     * Depending on the data format and various traits and strategies, not all
     * StreamBrokers may be active/available for every step. This function translates
     * a request for a specific broker, for a specific step, into the correct active
     * StreamBroker.
     *
     * @param[in] step_idx The step for which the active broker is needed.
     * @param[in] broker_no The uncorrected index for the StreamBroker.
     * @returns The active StreamBroker - this may more may not correspond to the
     *          StreamBroker indicated by `broker_no`.
     */
    BrokerType* active_stream_broker(hd_std::size_t step_idx,
                                     hd_std::size_t broker_no) {
      if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
        auto active_broker_idx { step_idx % num_segments };

        return stream_brokers[active_broker_idx];
      } else {
        auto active_broker_idx { broker_no };

        return stream_brokers[active_broker_idx];
      }
    }

    /**
     * Retrieve the correct DataAccessPtn for the currently active StreamBroker.
     *
     * Depending on the data format and various traits and strategies, not all
     * StreamBrokers may be active/available for every step. This function translates
     * a request for a specific broker, for a specific step, into the correct active
     * StreamBroker and returns its associated DataAccessPtn.
     *
     * @param[in] step_idx The step for which the active broker is needed.
     * @param[in] broker_no The uncorrected index for the StreamBroker.
     * @returns The DataAccessPtn for the active StreamBroker.
     */
    const DataAccessPtn& active_access_ptn(hd_std::size_t step_idx,
                                           hd_std::size_t broker_no) {
      if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
        auto active_broker_idx { step_idx % num_segments };

        return broker_access_ptns[active_broker_idx];
      } else {
        auto active_broker_idx { broker_no };

        return broker_access_ptns[active_broker_idx];
      }
    }

    /**
     * Remap a specific step index to account for StreamBroker inactivity and types.
     *
     * Depending on the data format and various traits and strategies, not all
     * StreamBrokers may be active/available for every step. As a result, the step
     * indices may increment faster than the logical count of steps (because it is
     * simultaneously counting steps, and cycling between active brokers). This
     * function will appropriately remap an uncorrected step index to account for
     * this.
     *
     * Furthermore, all data formats are required to use a type (StepIdxType) that can
     * be converted into a simple integral value (size_t). How this conversion is done
     * is not explicitly specified - this function will convert the index to a size_t.
     * Anything downstream of this can use size_t instead of the data format specific
     * types.
     *
     * @param[in] step_idx The unadjusted step index.
     * @returns The 'corrected' step index, accounting for any StreamBroker activity
     *          cycling, and in a size_t integral format.
     */
    const hd_std::size_t remap_step_idx(hd_std::size_t step_idx) {
      // All formats currently just use size_t directly, but can add in a conversion
      // function to the FormatTraits specifications if it becomes necessary.
      // That call/interface would then be hidden here.
      if constexpr (strategy == StreamPartitioningStrategy::Chronological) {
        return static_cast<hd_std::size_t>(step_idx / num_segments);
      } else {
        // No remapping required, so this is a no-op for the SubDivide strategy
        return static_cast<hd_std::size_t>(step_idx);
      }
    }


    // --- Direct getters/setters --- //
    const SegmentRef<BrokerType>& segment(hd_std::size_t seg_no) const {
      return segments[seg_no];
    }
    void set_segment(hd_std::size_t seg_no, SegmentRef<BrokerType>& seg) {
      segments[seg_no] = seg;
    }
    void set_stream_broker(hd_std::size_t broker_no,
                           BrokerType* broker,
                           DataAccessPtn ptn) {
      stream_brokers[broker_no] = broker;
      broker_access_ptns[broker_no] = ptn;
      num_stream_brokers++;
    }

    BrokerType* broker_for_segment(hd_std::size_t seg_no) {
      return segments[seg_no].broker;
    }
    DataAccessPtn& pattern_for_segment(hd_std::size_t seg_no) {
      return segments[seg_no].access_ptn;
    }

    BrokerType* stream_broker(hd_std::size_t broker_no) {
      return stream_brokers[broker_no];
    }
    const DataAccessPtn& access_ptn(hd_std::size_t broker_no) const {
      return broker_access_ptns[broker_no];
    }
  };
} // namespace sbio

#endif // SBIO_CORE_TOPOLOGY_HH
