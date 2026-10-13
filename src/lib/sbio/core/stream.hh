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

#ifndef SBIO_CORE_STREAM_HH
#define SBIO_CORE_STREAM_HH

#include "sbio/core/io.hh"
#include "sbio/util/string.hh"

#include <array>
#include <cstdint>
#include <utility>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  /**
   * Identifiers for positioning within the topology of StreamBrokers.
   */
  struct StreamIdentity {
    std::size_t stream_id { 0 }; ///< Identifier among parallel slots
    std::size_t chain_id { 0 };  ///< Sequential index along a chain of slots (if applicable)
  };

  enum class StreamResourceType : std::uint8_t {
    Path = 0,
    FileDescriptor,
    WindowsHandle,
    MemoryRegion,
    NetworkEndpoint
  };

  struct StreamResource {
    StreamResourceType type { StreamResourceType::Path };
    union Handle {
      char path[1024];         ///< Null-terminated path or URI string

      int fd;                  ///< POSIX file descriptor
#ifdef _WIN32
      void* win_handle;        ///< Windows HANDLE
#endif

      struct {
        void* ptr;
        std::size_t size;
      } memory;                ///< In-memory region

      Handle() : path{0} {}
    } handle;

    SBIO_HD static StreamResource from_path(const char* p) {
      StreamResource res;
      res.type = StreamResourceType::Path;
      safe_strncpy(res.handle.path, p, 1024);
      return res;
    }

    SBIO_HD static StreamResource from_fd(int fd) {
      StreamResource res;
      res.type = StreamResourceType::FileDescriptor;
      res.handle.fd = fd;
      return res;
    }

#ifdef _WIN32
    SBIO_HD static StreamResource from_win_handle(void* h) {
      StreamResource res;
      res.type = StreamResourceType::WindowsHandle;
      res.handle.win_handle = h;
      return res;
    }
#endif

    SBIO_HD static StreamResource from_memory(void* ptr, std::size_t size) {
      StreamResource res;
      res.type = StreamResourceType::MemoryRegion;
      res.handle.memory.ptr = ptr;
      res.handle.memory.size = size;
      return res;
    }
  };

  template <typename FTraits>
  struct GenericStreamConfig {
    StreamIdentity identity;

    static constexpr std::size_t VariantCount { FTraits::StreamTypes::size() };

    std::array<StreamResource, VariantCount> resources;

    std::size_t max_buffer_size { 0x4000000 };
    std::size_t max_batch_size { 1 };
    std::size_t index_batch_size { 43200 };

    typename FTraits::StreamParameters format_params;
  };

  /**
   * A light-weight wrapper over an IO engine.
   *
   * The Stream abstraction provides access to the underlying IO mechanics
   * by dispatching format-specific access patterns. The channel itself does
   * not maintain state or own any memory buffers. These are provided by the
   * callers in their requests.
   */
  template <class IO, class FTraits>
  class Stream : public IO {
  public:
    using IO::connect;
    using IO::file_size;
    using IO::read;

    SBIO_HD inline ReadResult read_at(void* dest,
                                      hd_std::uint64_t offset,
                                      hd_std::size_t size) const {
      return read(offset, size, dest);
    }

    SBIO_HD inline ReadResult read_span(void* dest,
                                        hd_std::uint64_t offset,
                                        hd_std::size_t max) const {
      if (offset >= file_size()) {
        return { IOStatus::ZeroBytesRead, 0 };
      }

      const hd_std::size_t n { hd_std::min(max, file_size() - offset) };
      return read(offset, n, dest);
    }

    SBIO_HD inline ReadResult read_unit_at(void* dest,
                                           hd_std::size_t buf_size,
                                           hd_std::uint64_t offset) const {
      auto hdr { read(offset, FTraits::HeaderSize, dest) };
      if (!hdr.ok()) {
        return hdr;
      }

      const hd_std::size_t payload { FTraits::get_payload_size(dest) };
      if (FTraits::HeaderSize + payload > buf_size) {
        return { IOStatus::PayloadTruncatedError, hdr.bytes };
      }

      auto body {
        read(offset + FTraits::HeaderSize,
             payload,
             static_cast<char*>(dest) + FTraits::HeaderSize)
      };

      return { body.status, hdr.bytes + body.bytes };
    }
  };
} // namespace sbio

#endif // SBIO_CORE_STREAM_HH
