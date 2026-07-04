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

#ifndef SBIO_MPI_STORAGE_HH
#define SBIO_MPI_STORAGE_HH

#include <mpi.h>

#include <cstddef>

namespace sbio {
  /**
   * MpiSharedBuffer: A buffer implementation backed by an MPI Shared Window.
   * This is used for roles like IndexRole that data processing ranks need to
   * access concurrently.
   */
  struct MpiSharedBuffer {
    inline void* ptr() const {
      return m_ptr;
    }
    inline std::size_t size() const { return m_size; }
    inline MPI_Win window() const { return m_window; }

    inline void set_memory(void* ptr, std::size_t size) {
      m_ptr = ptr;
      m_size = size;
    }

    inline void set_window(MPI_Win win) { m_window = win; }

  private:
    void* m_ptr { nullptr };
    std::size_t m_size { 0 };
    MPI_Win m_window { MPI_WIN_NULL };
  };
} // namespace sbio

#endif // SBIO_MPI_STORAGE_HH
