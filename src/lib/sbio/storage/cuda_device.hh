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

#ifndef SBIO_STORAGE_CUDA_DEVICE_HH
#define SBIO_STORAGE_CUDA_DEVICE_HH

#include "sbio/core/storage.hh"

#include <cstddef>
#include <cstring>

namespace sbio {
  struct CudaDeviceBuffer {

    static constexpr MemorySpace memory_space() { return MemorySpace::Device; }

    CudaDeviceBuffer() = default;

    void* ptr() const { return m_ptr; }

    std::size_t size() const { return m_size; }

    void set_memory(void* ptr, std::size_t size) {
      m_ptr = ptr;
      m_size = size;
    }

  private:
    void* m_ptr { nullptr };
    std::size_t m_size { 0 };
  };

  struct CudaPinnedBuffer {

    static constexpr MemorySpace memory_space() { return MemorySpace::Pinned; }

    inline void* ptr() const { return m_ptr; }

    inline std::size_t size() const { return m_size; }

    void set_memory(void* ptr, std::size_t size) {
      m_ptr = ptr;
      m_size = size;
    }

  private:
    void* m_ptr { nullptr };
    std::size_t m_size { 0 };
  };

  /**
   * A GPU device buffer with a pinner replica on the host.
   *
   * This buffer is intended to be used for staged IO. `ptr()` gives access to the
   * "real" buffer livingin device memory. IO can write to `host_ptr()` which is the
   * replica on the host, however.
   */
  struct CudaMirroredBuffer {
    static constexpr MemorySpace memory_space() { return MemorySpace::Device; }

    void* ptr() const { return m_ptr; }

    std::size_t size() const { return m_size; }

    void set_memory(void* ptr, std::size_t size) {
      m_ptr = ptr;
      m_size = size;
    }

    /**
     * A pinned replica of device memory buffer on the host.
     */
    void* host_ptr() const { return m_host_ptr; }

    void set_host_memory(void* host_ptr) { m_host_ptr = host_ptr; }

    /**
     * The number of bytes from start of buffer that need to be copied to host.
     *
     * When the data is written to the GPU buffer directly, the host-side replica
     * should be marked as stale by indicating the number of new bytes to copy
     * over.
     */
    std::size_t host_stale() const { return m_host_stale; }

    /**
     * Indicate how many bytes need to be copied into the host replica.
     *
     * When the data is written to the GPU buffer directly, the host-side
     * replica should be marked as stale by indicating the number of new bytes
     * to copy over.
     */
    void set_host_stale(std::size_t nbytes) { m_host_stale = nbytes; }

  private:
    void* m_ptr { nullptr };
    void* m_host_ptr { nullptr };
    std::size_t m_size { 0 };
    std::size_t m_host_stale { 0 };
  };
} // namespace sbio

#endif // SBIO_STORAGE_CUDA_DEVICE_HH
