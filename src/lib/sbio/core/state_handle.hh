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

#ifndef SBIO_CORE_STATE_HANDLE_HH
#define SBIO_CORE_STATE_HANDLE_HH

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

  struct ByteRegion {
    hd_std::size_t offset { 0 };
    hd_std::size_t length { 0 };

    SBIO_HD constexpr bool empty() const noexcept { return length == 0; }
  };

  template <typename FTraits>
  struct StreamCatalog {
    static constexpr hd_std::size_t NumStepKinds { FTraits::DataAccessPtnCount };

    hd_std::size_t index_epoch { 0 };
    hd_std::array<hd_std::size_t, NumStepKinds> num_steps         { 0 }; ///< Current readable steps (per pattern)
    hd_std::array<hd_std::size_t, NumStepKinds> cummulative_steps { 0 }; ///< Cummulative counts over all epochs

    SBIO_HD constexpr hd_std::size_t max_capacity() const noexcept {
      hd_std::size_t max { 0 };
      for (auto cnt : num_steps) {
        if (cnt > max) {
          max = cnt;
        }
      }

      return max;
    }

    template <hd_std::uint8_t AccessPtn>
    SBIO_HD constexpr hd_std::size_t capacity() const noexcept {
      return num_steps[AccessPtn];
    }
  };

  template <typename FTraits>
  struct IndexingCursor {
    static constexpr hd_std::size_t NumStepKinds { FTraits::DataAccessPtnCount };

    // Dont know which streams are used... could check with role investigation
    // but for now, can just over-estimate and use total stream variant count
    // to get it working.
    static constexpr hd_std::size_t NumStreams { FTraits::StreamTypes::size() };

    hd_std::array<hd_std::size_t, NumStreams> stream_offset { 0 }; ///< Maintained offset into stream
    hd_std::array<hd_std::size_t, NumStepKinds> offset_index { 0 }; ///< Per-pattern index into their offset buffer
  };

  template <typename FTraits>
  struct FetchCursor {
    using DataAccessPtn = typename FTraits::DataAccessPtn;

    static constexpr hd_std::size_t NumStepKinds { FTraits::DataAccessPtnCount };

    hd_std::array<hd_std::size_t, NumStepKinds> offset_index { 0 }; ///< Per-pattern index into their offset buffer

    DataAccessPtn last_access_ptn {};
  };
} // namespace sbio

#endif // SBIO_CORE_STATE_HANDLE_HH
