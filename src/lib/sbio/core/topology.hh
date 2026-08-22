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
  template <IsStreamBroker BrokerType, typename DataAccessPtn>
  struct SegmentRef {
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
  template <
    IsStreamBroker BrokerType,
    typename DataAccessPtn,
    hd_std::size_t MaxSegments = 128
  >
  struct GroupTopology {
    char group_name[256] { 0 };
    char group_type[256] { 0 };

    hd_std::array<BrokerType*, MaxSegments> stream_brokers { nullptr };
    hd_std::array<DataAccessPtn, MaxSegments> broker_access_ptns {};
    hd_std::size_t num_stream_brokers { 0 };

    hd_std::array<SegmentRef<BrokerType, DataAccessPtn>, MaxSegments> segments {};
    hd_std::size_t num_segments { 0 };

    StreamPartitioningStrategy strategy { StreamPartitioningStrategy::SubDivide };

    bool empty() const { return num_segments == 0; }

    const SegmentRef<BrokerType, DataAccessPtn>& segment(hd_std::size_t seg_no) const {
      return segments[seg_no];
    }
    void set_segment(hd_std::size_t seg_no,
                     SegmentRef<BrokerType, DataAccessPtn>& seg) {
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
