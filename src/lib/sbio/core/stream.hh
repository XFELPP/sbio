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
    // Bring IO functions into scope
    using IO::connect;
    using IO::file_size;
    using IO::m_file_size;
    using IO::read;
    // using IO::write;

    /**
     * Dispatch a read algorithm (Style or AccessStyle) onto the channel.
     * The style maintains relative state to apply its algorithm such as seek offsets.
     * The ReadRequest contains the targets for reads (buffers).
     */
    template <typename Style>
    SBIO_HD inline auto apply_read(Style& style,
                                   const typename Style::ReadRequest& req) {
      return style.read(*this, req);
    }

    /**
     * As with apply_read, dispatch a styled write onto the channel.
     */
    template <typename Style>
    SBIO_HD inline auto apply_write(Style& style,
                                    const typename Style::WriteRequest& req) {
      return style.write(*this, req);
    }

    SBIO_HD inline IOStatus read_one(void* dest, std::size_t buf_size) {
      // The IO base classes track total bytes read
      // As reads may be random access, though, this cannot be used for offset tracking
      IOStatus header_read_status = read(m_current_offset, FTraits::HeaderSize, dest);
      if (header_read_status != IOStatus::Success) {
        return header_read_status;
      }

      std::size_t payload_size = FTraits::get_payload_size(dest);
      std::size_t full_size = sizeof(typename FTraits::DataUnit) + payload_size;
      if (full_size > buf_size) {
        return IOStatus::PayloadTruncatedError;
      }
      m_current_offset += FTraits::HeaderSize;
      IOStatus payload_read_status = read(m_current_offset,
                                          payload_size,
                                          reinterpret_cast<char*>(dest) + FTraits::HeaderSize);
      if (payload_read_status == IOStatus::Success) {
        m_current_offset += payload_size;
      }
      return payload_read_status;
    }

    SBIO_HD inline IOStatus read_batch(void* dest, std::size_t batch_size, std::size_t missing_chunk = 0) {
      if (missing_chunk) {
        // NOTE: Assumes batch and unit/read_one reads aren't mixed
        // TODO: Make this better.
        //m_current_offset -= m_missing_chunk;
        m_current_offset -= missing_chunk;
      }

      std::size_t diff { m_file_size - m_current_offset };
      //std::size_t read_size = m_batch_size > diff ? diff : m_batch_size;
      std::size_t read_size = batch_size > diff ? diff : batch_size;

      IOStatus bulk_read_status = read(m_current_offset, read_size, dest);

      if (bulk_read_status == IOStatus::Success) {
        m_current_offset += read_size;
      }

      return bulk_read_status;
    }

    SBIO_HD inline IOStatus read_at(void* dest, std::size_t offset, std::size_t read_size) {
      //if (m_current_offset + size > m_file_size) {
      //  return IOStatus::PayloadTruncatedError;
      //}
      //std::size_t read_size = m_batch_size > diff ? diff : m_batch_size;

      m_current_offset = offset;
      IOStatus ra_read_status = read(m_current_offset, read_size, dest);

      if (ra_read_status == IOStatus::Success) {
        m_current_offset += read_size;
      }

      return ra_read_status;
    }

  private:
    std::size_t m_current_offset { 0 };
    std::size_t m_batch_size { 0 };
    std::size_t m_missing_chunk { 0 };
  };
} // namespace sbio

#endif // SBIO_CORE_STREAM_HH
