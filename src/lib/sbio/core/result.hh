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

#ifndef SBIO_CORE_RESULT_HH
#define SBIO_CORE_RESULT_HH

#include <ncarray/dtype.hh>
#include <ncarray/soarrays.hh>

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

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

#ifndef SBIO_MAX_NDIM
#define SBIO_MAX_NDIM NCARRAY_MAX_NDIM // Currently, 10
#endif

namespace sbio {

  /**
   * A result returned from a request to a single StreamBroker.
   */
  struct DataResult {
    const void* data { nullptr };
    hd_std::size_t size { 0 };
    hd_std::uint16_t rank { 0 };
    hd_std::array<hd_std::uint32_t, SBIO_MAX_NDIM> shape {};
    ncarray::DType dtype { ncarray::DType::uint8 };
  };

  /**
   * An organized result returned from a BrokerGroup managing one or more brokers.
   *
   * The BrokerGroup will return an organized data set that may contain data from
   * multiple segment references, which may be managed by multiple StreamBrokers.
   * If reading in batch, the final composite result may also have an access
   * that corresponds to the number of steps in the batch.
   *
   * @tparam MemTag Identifier for the location of the underlying memory (CPU or GPU).
   */
  template <typename MemTag = ncarray::HostTag>
  struct CompositeDataResult {
    const void** data { nullptr };     ///< Pointer table of SegmentRef results
    hd_std::size_t num_segments { 0 }; ///< The number of SegmentRef's in the result
    hd_std::size_t batch_count { 1 };  ///< The number of individaul steps/events

    /**
     * The rank of the DataResult returned by a StreamBroker.
     *
     * When organizing the composite result into an array, the final rank
     * will have 1 (or 2, if using batched reads) more dimensions.
     */
    hd_std::uint16_t segment_rank { 0 };
    /**
     * Shape of an individual result from a StreamBroker.
     */
    hd_std::array<hd_std::uint32_t, SBIO_MAX_NDIM> segment_shape {};
    ncarray::DType dtype { ncarray::DType::uint8 };

    SBIO_HD inline ncarray::SOViewFor<MemTag> as_ncarray() const {
      hd_std::uint16_t composite_rank { segment_rank + 1 };

      ncarray::Metadata meta_shape;
      ncarray::Metadata meta_strides;
      ncarray::Metadata meta_suboffsets;

      // Set composite axis metadata (the pointer table axes)
      ssize_t first_data { 1 };
      if (batch_count > 1) {
        first_data = 2;
        composite_rank += 1;

        meta_shape[0] = batch_count;
        meta_shape[1] = num_segments;

        meta_strides[0] = num_segments * sizeof(void*);
        meta_strides[1] = 1;

        meta_suboffsets[0] = 0;
        meta_suboffsets[1] = 0;
      } else {
        meta_shape[0] = num_segments;
        meta_strides[0] = sizeof(void*);
        meta_suboffsets[0] = 0;
      }

      ssize_t cur { static_cast<ssize_t>(ncarray::itemsize(dtype)) };
      for (ssize_t i = composite_rank - 1; i >= first_data; --i) {
        meta_shape[i] = segment_shape[i - 1];
        meta_strides[i] = cur;
        cur *= meta_shape[i];
      }

      meta_shape.ndim = composite_rank;
      meta_strides.ndim = composite_rank;
      meta_suboffsets.ndim = composite_rank;

      ssize_t ptr_axis { 0 }; // No effect for SOArray.

      return ncarray::SOViewFor<MemTag>(reinterpret_cast<void*>(const_cast<void**>(data)),
                                        meta_shape,
                                        meta_strides,
                                        meta_suboffsets,
                                        dtype,
                                        ptr_axis,
                                        true);
    }
  };
} // namespace sbio

#endif // SBIO_CORE_RESULT_HH
