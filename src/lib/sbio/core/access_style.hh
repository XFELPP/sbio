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

#ifndef SBIO_CORE_ACCESS_STYLE
#define SBIO_CORE_ACCESS_STYLE

#include "sbio/core/io.hh"
#include "sbio/formats/format_traits.hh"

#include <concepts>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  enum class AccessType {
    NoOp,
    UnitSequential,
    BatchedSequential,
    RandomAccess
  };

  template <typename Derived>
  class IAccessStyle {
  public:
    static constexpr AccessType Type = AccessType::NoOp;

    template <typename... Args>
    SBIO_HD inline void initialize(Args&&... args) {
      static_cast<Derived*>(this)->initialize(std::forward<Args>(args)...);
    }

    template <typename StreamType, typename ReadRequest>
    SBIO_HD inline IOStatus read(StreamType& channel, const ReadRequest& req) {
      return static_cast<Derived*>(this)->read(channel, req);
    }

    //template <typename StreamType, typename WriteRequest>
    //SBIO_HD inline IOStatus write(StreamType& channel, const WriteRequest& req) {
    //  return static_cast<Derived*>(this)->write(channel, req);
    //}
  };

  /**
   * Read a long batch of bytes.
   * This only requires provided an offset and number of bytes.
   */
  class BatchedSequential : public IAccessStyle<BatchedSequential> {
  public:
    BatchedSequential()
      : m_file_size(0)
      , m_batch_size(0)
      , m_offset(0)
    {}

    static constexpr AccessType Type = AccessType::BatchedSequential;

    struct ReadRequest {
      std::size_t missing_chunk;
      void* dest;
    };

    struct WriteRequest {};

    SBIO_HD inline void initialize(std::size_t batch_size,
                                    std::size_t file_size,
                                    std::size_t offset = 0) {
      m_batch_size = batch_size;
      m_file_size = file_size;
      m_offset = offset;
    }

    template <typename StreamType>
    SBIO_HD inline IOStatus read(StreamType& channel, const ReadRequest& req) {
      //if (req.missing_chunk > m_offset) {
      //  m_offset = 0;
      //} else {
      //  m_offset -= req.missing_chunk;
      //}

      m_offset -= req.missing_chunk;
      size_t diff { m_file_size - m_offset };
      size_t read_size = m_batch_size > diff ? diff : m_batch_size;
      IOStatus bulk_read_status = channel.read(m_offset, read_size, req.dest);

      if (bulk_read_status == IOStatus::Success) {
        m_offset += read_size;
      }

      return bulk_read_status;
    }

  private:
    std::size_t m_file_size;
    std::size_t m_batch_size;
    std::size_t m_offset;
  };

  /**
   * Read a single unit.
   * This will check a header and payload for you.
   */
  template <FormatTraits FTraits>
  class UnitSequential : public IAccessStyle<UnitSequential<FTraits>> {
  public:
    static constexpr AccessType Type = AccessType::UnitSequential;

    struct ReadRequest {
      void* dest;
      std::size_t buf_size;
    };

    template <typename StreamType>
    SBIO_HD inline IOStatus read(StreamType& channel, const ReadRequest& req) {
      //IOStatus header_read_status = r.IO::read(m_offset, Traits::HeaderSize, req.dest);
      IOStatus header_read_status = channel.read(m_offset, FTraits::HeaderSize, req.dest);
      if (header_read_status != IOStatus::Success) {
        return header_read_status;
      }

      std::size_t payload_size = FTraits::get_payload_size(req.dest);
      std::uint32_t full_size = sizeof(typename FTraits::DataUnit) + payload_size;
      if (full_size > req.buf_size) {
        return IOStatus::PayloadTruncatedError;
      }
      IOStatus read_status = channel.read(m_offset + FTraits::HeaderSize,
                                          payload_size,
                                          reinterpret_cast<char*>(req.dest) + FTraits::HeaderSize);
      return read_status;
    }

  private:
    std::size_t m_offset;
  };


  class RandomAccess : public IAccessStyle<RandomAccess> {
  public:
    static constexpr AccessType Type = AccessType::RandomAccess;

    struct ReadRequest {
      void* dest;
    };

    SBIO_HD inline void initialize(std::size_t offset, std::size_t size) {
      m_next_offset = offset;
      m_next_size = size;
    }

    template <typename StreamType>
    SBIO_HD inline IOStatus read(StreamType& channel, const ReadRequest& req) {
      IOStatus bulk_read_status = channel.read(m_next_offset, m_next_size, req.dest);
      return bulk_read_status;
    }
  private:
    std::size_t m_next_offset;
    std::size_t m_next_size;
  };

  template <typename T>
  concept HasRequestRead = requires(const T style, typename T::ReadRequest req) {
    typename T::ReadRequest;
  };

  template <typename T>
  concept AccessStyle = HasRequestRead<T>;
  //concept AccessStyle = IAccessStyle<T> && HasRequestRead<T>;

  template <typename T>
  concept BatchedSequentialStyle = AccessStyle<T> && (T::Type == AccessType::BatchedSequential);

  template <typename T>
  concept UnitSequentialStyle = AccessStyle<T> &&(T::Type == AccessType::UnitSequential);

  template <typename T>
  concept RandomAccessStyle = AccessStyle<T> &&(T::Type == AccessType::RandomAccess);
} // namespace sbio
#endif // SBIO_CORE_ACCESS_STYLE_HH
