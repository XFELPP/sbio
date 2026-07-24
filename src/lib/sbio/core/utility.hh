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

#ifndef SBIO_CORE_UTILITY_HH
#define SBIO_CORE_UTILITY_HH

#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif
#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2_traits.hh"
#endif

#include "ncarray/ncarrays.hh"

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

#include <cstddef>
#include <vector>

namespace sbio {
#ifdef SBIO_HAS_XTC1
  inline ncarray::DType to_ncarray_dtype(XTC1::DType type) {
    switch (type) {
    case XTC1::DType::UINT8:
      return ncarray::DType::uint8;
    case XTC1::DType::UINT16:
      return ncarray::DType::uint16;
    case XTC1::DType::UINT32:
      return ncarray::DType::uint32;
    case XTC1::DType::UINT64:
      return ncarray::DType::uint64;
    case XTC1::DType::INT8:
      return ncarray::DType::int8;
    case XTC1::DType::INT16:
      return ncarray::DType::int16;
    case XTC1::DType::INT32:
      return ncarray::DType::int32;
    case XTC1::DType::INT64:
      return ncarray::DType::int64;
    case XTC1::DType::FLOAT:
      return ncarray::DType::float32;
    case XTC1::DType::DOUBLE:
      return ncarray::DType::float64;
    default:
      return ncarray::DType::uint8;
    }
  }

  template <typename MemTag = ncarray::HostTag>
  ncarray::SOViewFor<MemTag> as_ncarray(const void** data,
                                        const std::size_t n_segments,
                                        const XTC1Traits::DataResult& res,
                                        const std::size_t batch_cnt = 1) {
    std::vector<ssize_t> shape;
    std::vector<ssize_t> suboffsets;

    if (batch_cnt > 1) {
      shape.push_back(batch_cnt);
      suboffsets.push_back(0);
    }

    shape.push_back(n_segments); // First axis is pointer to segments
    suboffsets.push_back(0);
    for (std::uint16_t i = 0; i < res.rank; ++i) {
      auto dim_shape = res.shape[i];
      if (dim_shape == 1) {
        // We can "auto-squeeze" for convenience.
        continue;
      }
      shape.push_back(dim_shape);
      suboffsets.push_back(-1);
    }

    // NOTE: The rank is now 1 larger than the data panels (for segment axis)
    std::size_t rank { shape.size() };
    ncarray::DType dtype { to_ncarray_dtype(res.dtype) };

    ssize_t cur { ncarray::itemsize(dtype) };
    std::vector<ssize_t> strides(rank, cur); // Axis (rank - 1) has stride itemsize
    int first_data { 1 };
    if (batch_cnt > 1) {
      first_data = 2;
      strides[0] = n_segments * sizeof(void*);
      strides[1] = 1;
    } else {
      strides[0] = sizeof(void*); // Axis 0 has stride of 1 (1 pointer at a time)
    }
    // Calculate strides for axes 1...N in reverse
    for (int i = rank - 1; i >= first_data; --i) {
      strides[i] = cur;
      cur *= shape[i];
    }

    ssize_t ptr_axis { 0 };

    ncarray::Metadata meta_shape;
    ncarray::Metadata meta_strides;
    ncarray::Metadata meta_suboffsets;

    meta_shape.set(shape.data(), static_cast<ssize_t>(shape.size()));
    meta_strides.set(strides.data(), static_cast<ssize_t>(shape.size()));
    meta_suboffsets.set(suboffsets.data(), static_cast<ssize_t>(shape.size()));

    return ncarray::SOViewFor<MemTag>(reinterpret_cast<void*>(const_cast<void**>(data)),
                                      meta_shape,
                                      meta_strides,
                                      meta_suboffsets,
                                      dtype,
                                      ptr_axis,
                                      true);

  }
#endif // SBIO_HAS_XTC1

#ifdef SBIO_HAS_XTC2
  inline ncarray::DType to_ncarray_dtype(XTC2::DType type) {
    switch (type) {
    case XTC2::DType::UINT8:
      return ncarray::DType::uint8;
    case XTC2::DType::UINT16:
      return ncarray::DType::uint16;
    case XTC2::DType::UINT32:
      return ncarray::DType::uint32;
    case XTC2::DType::UINT64:
      return ncarray::DType::uint64;
    case XTC2::DType::INT8:
      return ncarray::DType::int8;
    case XTC2::DType::INT16:
      return ncarray::DType::int16;
    case XTC2::DType::INT32:
      return ncarray::DType::int32;
    case XTC2::DType::INT64:
      return ncarray::DType::int64;
    case XTC2::DType::FLOAT:
      return ncarray::DType::float32;
    case XTC2::DType::DOUBLE:
      return ncarray::DType::float64;
    default:
      return ncarray::DType::uint8;
    }
  }

  template <typename MemTag = ncarray::HostTag>
  ncarray::SOViewFor<MemTag> as_ncarray(const void** data,
                                        const std::size_t n_segments,
                                        const XTC2Traits::DataResult& res,
                                        const std::size_t batch_cnt = 1) {
    std::vector<ssize_t> shape;
    std::vector<ssize_t> suboffsets;

    if (batch_cnt > 1) {
      shape.push_back(batch_cnt);
      suboffsets.push_back(0);
    }

    shape.push_back(n_segments); // First axis is pointer to segments
    suboffsets.push_back(0);
    for (std::uint16_t i = 0; i < res.rank; ++i) {
      auto dim_shape = res.shape[i];
      if (dim_shape == 1) {
        // We can "auto-squeeze" for convenience.
        continue;
      }
      shape.push_back(dim_shape);
      suboffsets.push_back(-1);
    }

    // NOTE: The rank is now 1 larger than the data panels (for segment axis)
    std::size_t rank { shape.size() };
    ncarray::DType dtype { to_ncarray_dtype(res.dtype) };

    ssize_t cur { ncarray::itemsize(dtype) };
    std::vector<ssize_t> strides(rank, cur); // Axis (rank - 1) has stride itemsize
    int first_data { 1 };
    if (batch_cnt > 1) {
      first_data = 2;
      strides[0] = n_segments * sizeof(void*);
      strides[1] = 1;
    } else {
      strides[0] = sizeof(void*); // Axis 0 has stride of 1 (1 pointer at a time)
    }

    // Calculate strides for axes 1...N in reverse
    for (int i = rank - 1; i >= first_data; --i) {
      strides[i] = cur;
      cur *= shape[i];
    }

    ssize_t ptr_axis { 0 };

    ncarray::Metadata meta_shape;
    ncarray::Metadata meta_strides;
    ncarray::Metadata meta_suboffsets;

    meta_shape.set(shape.data(), static_cast<ssize_t>(shape.size()));
    meta_strides.set(strides.data(), static_cast<ssize_t>(shape.size()));
    meta_suboffsets.set(suboffsets.data(), static_cast<ssize_t>(shape.size()));

    return ncarray::SOViewFor<MemTag>(reinterpret_cast<void*>(const_cast<void**>(data)),
                                      meta_shape,
                                      meta_strides,
                                      meta_suboffsets,
                                      dtype,
                                      ptr_axis,
                                      true);
  }
#endif // SBIO_HAS_XTC2
} // namespace sbio

#endif // SBIO_CORE_UTILITY_HH
