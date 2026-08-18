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

#ifndef SBIO_UTIL_MEM_POOL_HH
#define SBIO_UTIL_MEM_POOL_HH

#ifdef __CUDACC__

#include <cuda_runtime.h>

#include <cuda/std/cstddef>
#include <cuda/std/cstdlib>

namespace hd_std = cuda::std;

#else

#include <cstddef>
#include <cstdlib>

namespace hd_std = std;

#endif

namespace sbio {

  class MemPool {
  public:
    MemPool() = default;

    ~MemPool() { free(); }

    void realloc(hd_std::size_t bytes) {
      void* new_alloc { nullptr };
#ifdef SBIO_HAS_CUDA
      cudaMallocHost(&new_alloc, bytes);
      if (m_ptr != nullptr && m_idx > 0) {
        hd_std::memcpy(new_alloc, m_ptr, m_idx);
        cudaFreeHost(m_ptr);
      }
#else
      new_alloc = hd_std::realloc(m_ptr, bytes);
#endif

      m_ptr = static_cast<char*>(new_alloc);
      m_cap = bytes;
    }

    char* get_block(hd_std::size_t bytes, hd_std::size_t alignment = 16) {
      hd_std::size_t aligned { (bytes + (alignment - 1)) & ~(alignment - 1) };

      if (aligned > m_cap) {
        realloc(aligned);
      } else if (m_idx + aligned > m_cap) {
        m_idx = 0;

#ifdef SBIO_HAS_CUDA
        // Synchronize when wrapping to let out-standing ops finish
        cudaDeviceSynchronize();
#endif
      }

      std::size_t idx { m_idx };
      m_idx = (m_idx + aligned) % m_cap;

      return &m_ptr[idx];
    }

    void free() {
      if (m_ptr) {
#ifdef SBIO_HAS_CUDA
        cudaFreeHost(m_ptr);
#else
        std::free(m_ptr);
#endif
        m_ptr = nullptr;
      }

      m_idx = 0;
      m_cap = 0;
    }

  private:
    char* m_ptr { nullptr };
    hd_std::size_t m_idx { 0 };
    hd_std::size_t m_cap { 0 };
  };
} // namespace sbio

#endif // SBIO_UTIL_MEM_POOL_HH
