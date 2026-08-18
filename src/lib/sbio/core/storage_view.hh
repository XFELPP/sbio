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

#ifndef SBIO_CORE_STORAGE_VIEW_HH
#define SBIO_CORE_STORAGE_VIEW_HH

#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"

#include <ncarray/ncarrays.hh>

#ifdef __CUDACC__

#include <cuda/std/atomic>
#include <cuda/std/cstddef>
#include <cuda/std/type_traits>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <atomic>
#include <cstddef>
#include <type_traits>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif

namespace sbio {
  template <class MemTag>
  bool is_locality_match(MemorySpace memory_space) {
    if (memory_space == MemorySpace::Device) {
      if constexpr (hd_std::is_same_v<MemTag, ncarray::DevTag>) {
        return true;
      } else {
        return false;
      }
    } else if (memory_space == MemorySpace::Host) {
      if constexpr (hd_std::is_same_v<MemTag, ncarray::HostTag>) {
        return true;
      } else {
        return false;
      }
    }

    return true;
  }

  template <class StorageT, class EPolicy>
  class StorageView {
  public:

    SBIO_HD StorageView(StorageT& storage)
      : m_storage(storage)
    {}

#ifndef NDEBUG
    SBIO_HD StorageView(StorageT& storage, hd_std::atomic<hd_std::size_t>& num_views)
      : StorageView(storage)
      , m_num_views(&num_views)
    {}
#endif

    template <class Role, hd_std::size_t Index = 0>
    SBIO_HD inline auto size() const {
      auto& buf { m_storage.template get<Role, Index>() };

      return buf.size();
    }

    template <class Role, hd_std::size_t Index = 0, class CallerMemTag>
    SBIO_HD inline auto acquire(AcquireIntent intent = AcquireIntent::BufferMemorySpace) {
      auto& buf { m_storage.template get<Role, Index>() };

      if (intent == AcquireIntent::BufferMemorySpace ||
          is_locality_match<CallerMemTag>(buf.memory_space())) {
        return buf.ptr();
      }

      return EPolicy::acquire_broker_view(buf);
    }

    template <class Role, hd_std::size_t Index, class ViewT>
    SBIO_HD inline void release(ViewT view) {
#ifndef NDEBUG
      if (m_num_views != nullptr) {
        m_num_views->fetch_sub(1, hd_std::memory_order_acq_rel);
      }
#endif
    }

    template <class Role, hd_std::size_t Index, class ViewT, typename PtrT>
    requires hd_std::is_pointer_v<PtrT>
    SBIO_HD inline auto release(ViewT view, PtrT offset_ptr) {
      auto& buf { m_storage.template get<Role, Index>() };

      PtrT remapped { offset_ptr };

      if (offset_ptr != nullptr && view != buf.ptr()) {
        hd_std::size_t offset =
          reinterpret_cast<const char*>(offset_ptr) - reinterpret_cast<const char*>(view);

        remapped =
          reinterpret_cast<PtrT>(reinterpret_cast<const char*>(buf.ptr()) + offset);
      }

#ifndef NDEBUG
      if (m_num_views != nullptr) {
        m_num_views->fetch_sub(1, hd_std::memory_order_acq_rel);
      }
#endif

      return remapped;
    }

  private:
    StorageT& m_storage;

#ifndef NDEBUG
    hd_std::atomic<hd_std::size_t>* m_num_views { nullptr };
#endif
  };
} // namespace sbio
#endif // SBIO_CORE_STORAGE_VIEW_HH
