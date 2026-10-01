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
    hd_std::uint32_t stream { 0 };

    SBIO_HD constexpr bool empty() const noexcept { return length == 0; }

    SBIO_HD friend constexpr bool operator==(const ByteRegion& a, const ByteRegion& b) noexcept {
      return a.offset == b.offset && a.length == b.length && a.stream == b.stream;
    }
  };

  namespace impl {
    template <typename FTraits>
    SBIO_HD constexpr hd_std::size_t num_step_kinds() {
      if constexpr (requires { FTraits::NumStepKinds; }) {
        return FTraits::NumStepKinds;
      } else {
        return FTraits::DataAccessPtnCount;
      }
    }
  } // namespace impl

  inline constexpr hd_std::size_t NoRow { 0xFFFFFFFFu };

  template <typename FTraits>
  struct StreamCatalog {
    static constexpr hd_std::size_t NumStepKinds { impl::num_step_kinds<FTraits>() };

    hd_std::size_t index_epoch { 0 };
    hd_std::array<hd_std::size_t, NumStepKinds> num_steps         { 0 }; ///< Current readable steps (per pattern)
    hd_std::array<hd_std::size_t, NumStepKinds> cummulative_steps { 0 }; ///< Cummulative counts over all epochs

    hd_std::size_t num_rows { 0 };
    hd_std::array<hd_std::uint32_t, NumStepKinds> first_row {};
    hd_std::array<hd_std::uint32_t, NumStepKinds> last_row {};

    SBIO_HD constexpr hd_std::size_t max_capacity() const noexcept {
      hd_std::size_t max { 0 };
      for (auto cnt : num_steps) {
        if (cnt > max) {
          max = cnt;
        }
      }

      return max;
    }

    template <hd_std::uint8_t Kind>
    SBIO_HD constexpr hd_std::size_t capacity() const noexcept {
      return num_steps[Kind];
    }
  };

  template <typename FTraits>
  struct StepOffset {
    using StepKind = typename FTraits::StepKind;

    ByteRegion region {};                    ///< The offset and size to read in bytes

    hd_std::uint32_t next_of_kind { NoRow }; ///< The next StepOffset of the same kind
    hd_std::uint32_t prev_of_kind { NoRow }; ///< The previous StepOffset of the same kind

    StepKind step_kind {};                   ///< The kind this StepOffset accesses
    hd_std::uint64_t ordinal { 0 };          ///< Which step of its kind this is, counted over all batches
  };

  namespace impl {
    struct NoIndexState {};

    template <typename FTraits>
    struct index_state_for { using type = NoIndexState; };

    template <typename FTraits>
    requires requires { typename FTraits::IndexState; } // Need both requires
    struct index_state_for<FTraits> { using type = typename FTraits::IndexState; };
  } // namespace impl

  template <typename FTraits>
  using IndexStateFor = typename impl::index_state_for<FTraits>::type;

  namespace index_ids {
    // Include a set of named identifiers for buffers used for generic indexing
    inline constexpr hd_std::size_t Steps { 0x0 };   ///< roles::Index    - StepOffset rows (+ carried slots)
    inline constexpr hd_std::size_t Scratch { 0x100 }; ///< roles::Metadata - read buffer for scanning
  } // namespace index_ids

  template <typename FTraits>
  struct IndexingCursor {
    static constexpr hd_std::size_t NumStreams { FTraits::StreamTypes::size() };
    static constexpr hd_std::size_t NumStepKinds { FTraits::DataAccessPtnCount };

    hd_std::array<hd_std::uint64_t, NumStreams> next_offset {}; ///< First unparsed byte, per stream variant slot
#ifdef _WIN32
    [[msvc::no_unique_address]] IndexStateFor<FTraits> format {};
#else
    [[no_unique_address]] IndexStateFor<FTraits> format {}; ///< FTraits::IndexState if defined, else an empty struct
#endif // _WIN32
  };

  template <typename FTraits>
  struct SegmentCursor {
    static constexpr hd_std::size_t NumStepKinds { impl::num_step_kinds<FTraits>() };

    hd_std::array<hd_std::size_t, NumStepKinds> offset_index {}; ///< Index into offset buffers
    ByteRegion last_region {}; ///< Last region read (to track size change / re-copy check)
    hd_std::size_t read_count { 0 };
  };
} // namespace sbio

#endif // SBIO_CORE_STATE_HANDLE_HH
