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

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <vector>

namespace sbio {
  template <class StorageT, class EPolicy>
  class StorageView {
  public:

    SBIO_HD StorageView(StorageT& storage)
      : m_storage(storage)
    {}

    template <class Role, std::size_t Index = 0>
    SBIO_HD inline auto size() const {
      auto& buf { m_storage.template get<Role, Index>() };

      return buf.size();
    }

    template <class Role, std::size_t Index = 0, class CallerMemTag>
    SBIO_HD inline auto acquire(AcquireIntent intent) {
      auto& buf { m_storage.template get<Role, Index>() };

      if (intent == AcquireIntent::BufferMemorySpace ||
          is_locality_match<CallerMemTag>(buf.memory_space())) {
        return buf.ptr();
      }

      return EPolicy::acquire_broker_view(buf);
    }

    template <class Role, std::size_t Index, class ViewT, class ResultT>
    SBIO_HD inline auto release(ViewT view, ResultT res) {
      auto& buf { m_storage.template get<Role, Index>() };

      // TODO: This isn't great. The Result is variable for FormatTraits conforming impls.
      //       But the remap has to happen no matter what. Relying on a `data` field existing...
      if constexpr (requires { res.data; }) {
        if (view != buf.ptr()) {
          std::size_t offset =
            reinterpret_cast<const char*>(res.data) - reinterpret_cast<const char*>(view);
          res.data = reinterpret_cast<const char*>(buf.ptr()) + offset;
        }
      }

      return res;
    }

  private:
    StorageT& m_storage;
  };
} // namespace sbio
#endif // SBIO_CORE_STORAGE_VIEW_HH
