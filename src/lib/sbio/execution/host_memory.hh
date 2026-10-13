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

#ifndef SBIO_EXECUTION_HOST_MEMORY_HH
#define SBIO_EXECUTION_HOST_MEMORY_HH

#include "sbio/core/storage.hh"
#include "sbio/storage/host_buffer.hh"

#include <cstddef>

namespace sbio {
  /**
   * A very simple MemoryManager which places all buffers in pageable host memory.
   */
  struct HostMemory {
    template <typename Descriptor>
    using PlacementFor = HostBuffer;

    static void allocate_buffer(HostBuffer& buf, std::size_t size) {
      buf.set_memory(new char[size], size);
    }

    static void release_buffer(HostBuffer& buf) {
      delete[] static_cast<char*>(buf.ptr());
      buf.set_memory(nullptr, 0);
    }
  };
} // namespace sbio

#endif // SBIO_EXECUTION_HOST_MEMORY_HH
