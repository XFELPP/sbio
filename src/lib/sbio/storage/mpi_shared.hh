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

#ifndef SBIO_STORAGE_MPI_SHARED_HH
#define SBIO_STORAGE_MPI_SHARED_HH

#include "sbio/util/rc.hh"

#include <mpi.h>

#include <cstddef>
#include <utility>

namespace sbio {
  /**
   * A custom Deleter for an RC managed MPI_Win.
   */
  struct MPIWinDeleter {
    MPI_Comm shmem_comm { MPI_COMM_NULL };

    void operator()(MPI_Win* win) {
      int initialized { 0 };
      MPI_Initialized(&initialized);

      int finalized { 0 };
      MPI_Finalized(&finalized);

      if (initialized && !finalized) {
        MPI_Win_free(win);
      }
    }
  };

  /**
   * A custom Allocator for an RC managed MPI_Win.
   */
  struct MPIWinAllocator {
    static void allocate(std::size_t win_size,
                         void* dest,
                         MPI_Comm comm,
                         std::size_t mem_size,
                         void** out_baseptr) {
      MPI_Win win;
      MPI_Aint sz_out;
      int disp;
      void* baseptr { nullptr };

      MPI_Win_allocate_shared(mem_size, 1, MPI_INFO_NULL, comm, &baseptr, &win);
      MPI_Win_shared_query(win, 0, &sz_out, &disp, &baseptr);

      // Build the Window inplace inside the RC
      new (dest) MPI_Win(win);

      if (out_baseptr) {
        // The caller needs the pointer to the final memory
        *out_baseptr = baseptr;
      }
    }
  };

  /**
   * A buffer implementation backed by an MPI Shared Window.
   *
   * This is used for roles like IndexRole that data processing ranks need to
   * access concurrently.
   *
   * The expectation is that the allocation of the memory for backing the Window
   * be done by some controlling execution policy. This buffer then serves as a
   * light view over it. It furthermore WILL be responsible for eventually cleaning
   * up the memory; however, this is only done if there are no remaining references to
   * the Window. It wraps the MPI_Win in an RC which manages the reference counting and
   * performs destruction when the count reaches zero.
   *
   * As there is the possibility for multiple StreamBrokers/other objects to run
   * concurrently within a single rank, each of which using their own shared buffer,
   * the MPISharedBuffer is tagged. The tag can be used for point-to-point
   * communciation.
   */
  struct MPISharedBuffer {

    MPISharedBuffer() = default;

    MPISharedBuffer(const MPISharedBuffer& other)
      : m_ptr(other.m_ptr)
      , m_size(other.m_size)
      , m_window(other.m_window)
      , m_tag(other.m_tag)
    {}

    /**
     * A shallow copy constructor.
     *
     * If the Window is non-null at the time of move, then increment the reference
     * count. The other MPISharedBuffer is NULL'd.
     */
    MPISharedBuffer& operator=(const MPISharedBuffer& other) {
      if (this != &other) {
        m_ptr = other.m_ptr;
        m_size = other.m_size;
        m_window = other.m_window;
        m_tag = other.m_tag;
      }

      return *this;
    }

    /**
     * A shallow move constructor.
     *
     * If the Window is non-null at the time of move, the reference count is not
     * incremented as the other's is moved and NULL'd.
     */
    MPISharedBuffer(MPISharedBuffer&& other) noexcept
      : m_ptr(other.m_ptr)
      , m_size(other.m_size)
      , m_window(std::move(other.m_window))
      , m_tag(other.m_tag)
    {
      other.m_ptr = nullptr;
      other.m_size = 0;
      other.m_tag = -1;
    }

    /**
     * A shallow move assignment.
     *
     * If the Window is non-null at the time of move, the reference count is not
     * incremented as the other's is moved and NULL'd.
     */
    MPISharedBuffer& operator=(MPISharedBuffer&& other) noexcept {
      if (this != &other) {
        m_ptr = other.m_ptr;
        m_size = other.m_size;
        m_window = std::move(other.m_window);
        m_tag = other.m_tag;

        other.m_ptr = nullptr;
        other.m_size = 0;
        other.m_tag = -1;
      }

      return *this;
    }

    inline void* ptr() const {
      return m_ptr;
    }
    inline std::size_t size() const { return m_size; }
    inline const MPI_Win& window() const { return *m_window; }

    inline void set_memory(void* ptr, std::size_t size) {
      m_ptr = ptr;
      m_size = size;
    }

    /**
     * Alternative interface for allocating an MPI_Win.
     *
     * This alternative API, additional to the standard buffer concept, encapsulates
     * creation of a fully managed RC<MPI_Win, MPIWinAllocator, MPIWinDeleter>.
     *
     * @param comm The communicator for creation of the shared MPI_Win.
     * @param size The size of the shared buffer.
     */
    inline void allocate(MPI_Comm comm, std::size_t size) {
      void* baseptr { nullptr };

      m_window = RC<MPI_Win, MPIWinAllocator, MPIWinDeleter>::make_rc(use_rc_alloc_del,
                                                                      MPIWinAllocator {},
                                                                      MPIWinDeleter { comm },
                                                                      comm,
                                                                      size,
                                                                      &baseptr);

      set_memory(baseptr, size);
    }

    inline int tag() const { return m_tag; }
    inline void set_tag(int tag) { m_tag = tag; }

    inline bool is_dirty() const { return false; }
    inline void set_dirty(bool) {}

  private:
    void* m_ptr { nullptr };                              ///< Pointer to the shared memory buffer
    std::size_t m_size { 0 };                             ///< Total size of the buffer/Window
    RC<MPI_Win, MPIWinAllocator, MPIWinDeleter> m_window; ///< Window over the shared memory buffer
    int m_window_idx { 0 };                               ///< The index of the Window for ref counting
    int m_tag { -1 };                                     ///< Tag to allow point-to-point communication
  };
} // namespace sbio

#endif // SBIO_STORAGE_MPI_SHARED_HH
