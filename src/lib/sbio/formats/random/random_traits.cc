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

#include "sbio/formats/random/random_traits.hh"

#include <ncarray/dtype.hh>

#ifdef __CUDACC__

#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/cstring>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

namespace sbio {
  void RandomTraits::MetadataInventory::add_detector(const char* name_,
                                                     const char* type_,
                                                     hd_std::uint16_t rank_,
                                                     const hd_std::uint32_t* shape_,
                                                     ncarray::DType dtype) {
    if (count < 16) {
      safe_strncpy(entries[count].name, name_, MaxNameSize);
      safe_strncpy(entries[count].type, type_, MaxNameSize);
      entries[count].rank = rank_;

      hd_std::size_t payload_size { ncarray::itemsize(dtype) };
      for (hd_std::uint16_t r = 0; r < rank_; ++r) {
        entries[count].shape[r] = shape_[r];
        payload_size *= shape_[r];
      }

      entries[count].dtype = dtype;
      // Need to setup the offset_in_event during fast pass over actual data
      entries[count].size = payload_size;

      count++;
    }
  }
}
