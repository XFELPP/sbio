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

#ifndef SBIO_STORAGE_THREAD_LOCAL_BUFFER_HH
#define SBIO_STORAGE_THREAD_LOCAL_BUFFER_HH

#include "sbio/core/storage.hh"

#include <cstddef>

namespace sbio {
  struct ThreadLocalBuffer {

    static constexpr MemorySpace memory_space() { return MemorySpace::Host; }

    // ThreadLocalBuffer() = default;

    // ThreadLocalBuffer(const ThreadLocalBuffer&) = delete;
    // ThreadLocalBuffer& operator=(const ThreadLocalBuffer&) = delete;

    // ThreadLocalBuffer(ThreadLocalBuffer&&) noexcept = default;
    // ThreadLocalBuffer& operator=(ThreadLocalBuffer&&) noexcept = default;

    inline void* ptr() const {
      // Create a registry to allow multiple thread_local ptrs - 1 per storage
      // Otherwise, a single static thread_local would be program-wide, so if
      // there were multiple buffers (e.g., 1 for each of multiple brokers)
      // they would clash on the single shared pointer.

      thread_local const ThreadLocalBuffer* last_buf { nullptr };
      thread_local void* last_ptr { nullptr };

      if (last_buf == this) {
        return last_ptr;
      }

      struct PointerEntry {
        const ThreadLocalBuffer* buf;
        void* ptr;
      };

      // This is implicitly static thread_local
      thread_local std::vector<PointerEntry> registry;

      for (const auto& entry : registry) {
        if (entry.buf == this) {
          last_buf = this;
          last_ptr = entry.ptr;
          return last_ptr;
        }
      }

      void* new_ptr { nullptr };
      if (m_size > 0) {
        new_ptr = std::malloc(m_size);
      }

      registry.push_back({ this, new_ptr });

      last_buf = this;
      last_ptr = new_ptr;

      return last_ptr;
    }

    std::size_t size() const {
      return m_size;
    }

    /**
     * The ThreadLocalBuffer sets memory in a delayed fashion, only storing size here.
     *
     * The actual memory allocation occurs on the first retrieval of the underlying
     * buffer pointer. This allows each thread to maintain its own buffer.
     *
     * @param[in] ptr Location to allocate memory over. Ignored.
     * @param[in] size The size of the buffer required.
     */
    inline void set_memory(void* ptr, std::size_t size) { m_size = size; }

    ~ThreadLocalBuffer() {
      // TODO: Implement this!
    }

  private:
    mutable std::size_t m_size { 0 };
  };

} // namespace sbio
#endif // SBIO_STORAGE_THREAD_LOCAL_BUFFER_HH
