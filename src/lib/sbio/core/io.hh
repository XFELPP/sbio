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

#ifndef SBIO_CORE_IO_HH
#define SBIO_CORE_IO_HH

#ifdef __CUDACC__

#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/type_traits>
#include <cuda/std/utility>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

namespace sbio {
  enum class IOStatus : hd_std::uint32_t {
    Success = 0,
    Pending,    // IO in flight
    WouldBlock, // Resource busy (NIC/Disk queue full e.g.)
    OpenFailed,
    GeneralIOError,
    ZeroBytesRead,
    TruncatedRead,
    FunctionUnavailable,
    NoOffsetInData,
    HeaderReadError,
    PayloadTruncatedError,
    AllRequestedRead
  };

  struct ReadResult {
    IOStatus status { IOStatus::Success };
    hd_std::size_t bytes { 0 };

    SBIO_HD constexpr bool ok() const noexcept { return status == IOStatus::Success; }
  };

  /**
   * Assign an IOStatus based on a retrieved and desired byte count.
   *
   * @param[in] got The number of bytes retrieved (or negative if error).
   * @param[in] want The number of bytes that were requested.
   * @returns A ReadResult with the byte count and IOStatus.
   */
  SBIO_HD constexpr ReadResult classify_read(ssize_t got, hd_std::size_t want) {
    if (got < 0) {
      return { IOStatus::GeneralIOError, 0 };
    }

    if (got == 0) {
      return { IOStatus::ZeroBytesRead, 0 };
    }

    const auto n { static_cast<hd_std::size_t>(got) };
    return { (n == want) ? IOStatus::Success : IOStatus::TruncatedRead, n };
  }

  template <typename Derived>
  struct IOPolicy {
  public:
    IOPolicy()
      : m_file_size(0)
    {}

    template <typename... Args>
    SBIO_HD inline IOStatus connect(Args&&... args) {
      return static_cast<Derived*>(this)->connect(hd_std::forward<Args>(args)...);
    }

    SBIO_HD inline ReadResult read(hd_std::uint64_t offset,
                                   hd_std::size_t size,
                                   void* dest) const {
      return static_cast<const Derived*>(this)->read_impl(offset, size, dest);
    }

    SBIO_HD inline hd_std::size_t file_size() const {
      return m_file_size;
    }

  protected:
    hd_std::size_t m_file_size { 0 }; // Only written once at connect
  };

  template <typename T>
  concept HasRead = requires(const T io, hd_std::uint64_t offset, hd_std::size_t size, void* dest) {
    { io.read(offset, size, dest) } -> hd_std::same_as<ReadResult>;
    { io.file_size() } -> hd_std::convertible_to<hd_std::size_t>;
  };

  template <typename T>
  concept IOTraits = HasRead<T>;
} // namespace sbio
#endif // SBIO_CORE_IO_HH
