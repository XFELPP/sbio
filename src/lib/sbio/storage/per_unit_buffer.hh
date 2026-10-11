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

#ifndef SBIO_STORAGE_PER_UNIT_BUFFER_HH
#define SBIO_STORAGE_PER_UNIT_BUFFER_HH

#include "sbio/core/storage.hh"

#include <cstddef>
#include <memory>

namespace sbio {
  /**
   * A buffer with one instance per parallel unit (`BufferScope::PerUnit`).
   *
   * Different execution policies use a different base parallel unit (e.g. thread, or
   * MPI rank/process). Each of the base units has its own instance of a PerUnitBuffer
   * through the `SegmentState` that the execution policy defines.
   *
   * @note A call to `Storage::get` will unwrap the PerUnitBuffer so that a caller
   *       only ever sees the underlying buffer (defined by `Inner` tparam)
   *
   * @note The buffer memory is allocated upon first access to the buffer.
   *
   * @tparam Inner The actual underlying buffer type of each unit's instance.
   * @tparam Policy The Execution policy.
   */
  template <class Inner, class Policy>
  struct PerUnitBuffer {
    static constexpr MemorySpace memory_space() { return Inner::memory_space(); }

    /**
     * The calling unit's instance, allocated on first access.
     */
    Inner& local() const {
      std::shared_ptr<Slot>& slot { m_units.get() };
      if (!slot) {
        slot = std::make_shared<Slot>();
        Policy::allocate_buffer(slot->buffer, m_size);
      }

      return slot->buffer;
    }

    void* ptr() const { return local().ptr(); }

    std::size_t size() const { return m_size; }

    /**
     * Set the size of each unit's instance (allocated later, per unit).
     */
    void set_memory(void*, std::size_t size) { m_size = size; }

  private:
    struct Slot {
      Inner buffer {};
    };

    typename Policy::template SegmentState<std::shared_ptr<Slot>> m_units {};
    std::size_t m_size { 0 };
  };
} // namespace sbio

#endif // SBIO_STORAGE_PER_UNIT_BUFFER_HH
