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

#ifndef SBIO_CORE_TYPES_HH
#define SBIO_CORE_TYPES_HH

#ifdef __CUDACC__

#include <cuda/std/cstdint>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <cstdint>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

namespace sbio {
  /**
   * @brief Indicates the strategy used to partition data across streams.
   *
   * When partitioning data from a single "logical unit" across multiple data streams
   * written over time, one can imagine two basic strategies:
   *  1. At each point in time, the logical unit is sub-divided and a portion
   *     is written to each data stream.
   *  2. At each point in time, the entire logical unit is written to one stream,
   *     and then the data is round-robined (or via some other selection mechanism
   *     distributed) across the other streams.
   * - SubDivide indicates the format uses strategy 1.
   * - Chronological indicates the format uses strategy 2.
   */
  enum class StreamPartitioningStrategy : hd_std::uint8_t {
    SubDivide     = 0, ///< A single logical unit is sub-divided into multiple streams.
    Chronological = 1  ///< A complete logical unit is written to one stream at a time.
  };

  enum class StreamSentinels : hd_std::uint8_t {
    RequestExhausted = 0, ///< All data units from this read request were read
    StreamExhausted = 1   ///< The stream is entirely exhausted (no more data)
  };

  /**
   * A contiguous batch of steps: [first, last).
   *
   * A batch is a pair of step indices indicating a contiguous range that has been
   * handed out together. Exhaustion is indicated by both first and last equal to
   * the format's ExhaustedSentinel.
   *
   * @tparam StepIdxType The step index type of the data format.
   */
  template <typename StepIdxType>
  struct StepBatch {
    StepIdxType first; ///< The first step of the batch.
    StepIdxType last;  ///< One past the last step of the batch.

    /**
     * The number of steps in the batch.
     *
     * @returns The number of steps in the batch.
     */
    SBIO_HD constexpr StepIdxType count() const { return last - first; }
  };
} // namespace sbio

#endif // SBIO_CORE_TYPES_HH
