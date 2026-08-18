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

#ifndef SBIO_STORAGE_HOST_BUFFER_HH
#define SBIO_STORAGE_HOST_BUFFER_HH

#include "sbio/core/storage.hh"

#include <cstddef>

namespace sbio {

  struct HostBuffer {

    static constexpr MemorySpace memory_space() { return MemorySpace::Host; }

    // HostBuffer() = default;
    // HostBuffer(const HostBuffer&) = delete;
    // HostBuffer& operator=(const HostBuffer&) = delete;

    // HostBuffer(HostBuffer&&) noexcept = default;
    // HostBuffer& operator=(HostBuffer&&) noexcept = default;

    inline void* ptr() const {
      return m_ptr;
    }

    std::size_t size() const {
      return m_size;
    }

    inline void set_memory(void* ptr, std::size_t size) {
      m_ptr = ptr;
      m_size = size;
    }

    inline bool is_dirty() const {
      return false;
    }
    inline void set_dirty(bool) {
    }

  private:
    void* m_ptr;
    std::size_t m_size;
  };
} // namespace sbio
#endif // SBIO_STORAGE_HOST_BUFFER_HH
