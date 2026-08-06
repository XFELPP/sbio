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

#include "sbio/formats/xtc2/xtc2.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio::XTC2 {
  SBIO_HD DataResult resolve_xtc2_pointer(void* buffer,
                                          std::uint32_t sd_offset,
                                          const Name* field_schema,
                                          std::uint32_t nid,
                                          std::uint32_t f_idx) {
    auto* sd =
      reinterpret_cast<XTC2::ShapesData*>(reinterpret_cast<char*>(buffer) + sd_offset);

    const auto& shapes = sd->shapes();

    std::size_t byte_offset { 0 };
    std::uint32_t shape_idx { 0 };
    for (uint32_t i = 0; i < f_idx; ++i) {
      if (field_schema[i].rank() == 0) {
        byte_offset += XTC2::DTypeSize[static_cast<uint8_t>(field_schema[i].type())];
      } else {
        byte_offset += shapes.get(shape_idx++).size(field_schema[i]);
      }
    }

    const auto& target = field_schema[f_idx];
    DataResult res(sd->data().payload() + byte_offset);
    res.rank = target.rank();
    res.dtype = target.type();
    if (res.rank == 0) {
      res.size = XTC2::DTypeSize[static_cast<std::uint8_t>(target.type())];
    } else {
      const auto& s = shapes.get(shape_idx);
      res.size = s.size(const_cast<XTC2::Name&>(target));
      const std::uint32_t* shape_ptr = s.shape();
      for (size_t i = 0; i < res.rank; ++i) {
        res.shape[i] = shape_ptr[i];
      }
    }

    return res;
  }
} // namespace sbio::XTC2
