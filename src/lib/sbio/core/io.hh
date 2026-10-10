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

  /**
   * Feature and capability flags for what an IO policy can do.
   *
   * IO implementations are composed (via template) with various Execution policies
   * completely freely; however, not all IOPolicy implementations have the same
   * capabilities. The IOPolicy, therefore, states the capabilities it provides/supports
   * using feature flags that can be queried at compile time.
   */
  enum class IOCapability : hd_std::uint32_t {
    None            =       0,
    ReadToHost      = 1u << 0, ///< Can read into (pageable) host memory
    ReadToPinned    = 1u << 1, ///< Can read into pinned (page-locked) host memory
    ReadToDevice    = 1u << 2, ///< Can read into device (GPU) memory
    ConcurrentReads = 1u << 3, ///< One Stream can be read by multiple threads at once
    DeviceCallable  = 1u << 4, ///< `read` APIs can be called directly from device (GPU) code
  };

  SBIO_HD constexpr IOCapability operator|(IOCapability cap_a, IOCapability cap_b) {
    return static_cast<IOCapability>(static_cast<hd_std::uint32_t>(cap_a) |
                                     static_cast<hd_std::uint32_t>(cap_b));
  }

  SBIO_HD constexpr IOCapability operator&(IOCapability cap_a, IOCapability cap_b) {
    return static_cast<IOCapability>(static_cast<hd_std::uint32_t>(cap_a) &
                                     static_cast<hd_std::uint32_t>(cap_b));
  }

  /**
   * Check if the set of capabilities matches some required set.
   */
  SBIO_HD constexpr bool has_capabilities(IOCapability set, IOCapability required) {
    return (set & required) == required;
  }

  /**
   * The default set of capabilities assumed, if the policy does not state its own.
   *
   * Before the introduction of the explicit IOCapability flags, this set was the
   * implicitly assumed set. I.e., that an IOPolicy could synchornously read into
   * host memory (from any executing thread).
   */
  inline constexpr IOCapability DefaultIOCapabilities {
    IOCapability::ReadToHost | IOCapability::ReadToPinned | IOCapability::ConcurrentReads
  };

  /**
   * Query the capabilties of a specific IO policy.
   *
   * @tparam IO The IOPolicy to check capabilities of.
   * @returns The policy's capabilties, or DefaultIOCapabilities if the policy does not
   *          define any explicitly.
   */
  template <typename IO>
  SBIO_HD constexpr IOCapability io_capabilities() {
    if constexpr (requires { IO::Capabilities; }) {
      return IO::Capabilities;
    } else {
      return DefaultIOCapabilities;
    }
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
