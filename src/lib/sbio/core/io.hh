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

  template <typename Derived>
  struct IOPolicy {
  public:
    IOPolicy()
      : m_file_size(0)
      , m_read_count(0)
      , m_total_bytes_read(0)
    {}

    template <typename... Args>
    SBIO_HD inline IOStatus connect(Args&&... args) {
      return static_cast<Derived*>(this)->connect(hd_std::forward<Args>(args)...);
    }

    SBIO_HD inline IOStatus read(hd_std::uint64_t offset,
                                 hd_std::size_t size,
                                 void* dest) {
      return static_cast<Derived*>(this)->read(offset, size, dest);
    }

    SBIO_HD inline IOStatus iread(hd_std::uint64_t offset,
                                  hd_std::size_t size,
                                  void* dest) {
      if constexpr (requires { static_cast<Derived*>(this)->iread(offset, size, dest); }) {
        return static_cast<Derived*>(this)->iread(offset, size, dest);
      } else {
        m_cached_offset = offset;
        m_cached_size = size;
        m_cached_dest = dest;
        return IOStatus::FunctionUnavailable;
      }
    }

    SBIO_HD inline IOStatus wait() {
      if constexpr (requires { static_cast<Derived*>(this)->wait(); }) {
        return static_cast<Derived*>(this)->wait();
      } else {
        // If the IO policy does not have iread/wait functionality, fall back and
        // use a normal synchronous read when caller uses wait.
        return static_cast<Derived*>(this)->read(m_cached_offset,
                                                 m_cached_size,
                                                 m_cached_dest);
      }
    }

    SBIO_HD inline hd_std::size_t file_size() const { return m_file_size; }

    SBIO_HD inline hd_std::size_t read_count() const { return m_read_count; }

    SBIO_HD inline hd_std::size_t total_bytes_read() const { return m_total_bytes_read; }

  protected:
    hd_std::size_t m_file_size;
    hd_std::size_t m_read_count; ///< Bytes read on the last read
    hd_std::size_t m_total_bytes_read; ///< Cummulative bytes read over all reads

  private:
    hd_std::uint64_t m_cached_offset;
    hd_std::size_t m_cached_size;
    void* m_cached_dest;
  };

  template <typename T>
  concept HasRead = requires(T io, hd_std::uint64_t offset, hd_std::size_t size, void* dest) {
    { io.read(offset, size, dest) } -> hd_std::convertible_to<IOStatus>;
    { io.read_count() } -> hd_std::convertible_to<hd_std::size_t>;
    { io.file_size() } -> hd_std::convertible_to<hd_std::size_t>;
  };

  template <typename T>
  concept IOTraits = HasRead<T>;
} // namespace sbio
#endif // SBIO_CORE_IO_HH
