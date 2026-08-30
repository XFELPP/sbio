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

#ifndef SBIO_CORE_LOCATOR_HH
#define SBIO_CORE_LOCATOR_HH

#include "sbio/export_macro.hh"

#ifdef __CUDACC__

#include <cuda/std/concepts>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <concepts>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif

namespace sbio {
  template <typename T, typename DS>
  concept IsLocator = requires(DS& ds,
                               const typename T::template LocatorParameters<typename DS::DataFormat>& params,
                               const typename DS::DataFormat::StreamParameters& base_cfg) {
    typename T::template LocatorParameters<typename DS::DataFormat>;
    { T::template find_streams<DS>(ds, params, base_cfg) } -> hd_std::same_as<bool>;
  };

  template <typename Class, typename T>
  struct NamedParameter {
    const char* name; ///< Provided parameter name
    T Class::* ptr;   ///< Pointer to member where the parameter value is stored
  };

  template <typename Class, typename T>
  SBIO_HD SBIO_API constexpr NamedParameter<Class, T>
  make_named(const char* name, T Class::* ptr) {
    return { name, ptr };
  }
} // namespace sbio

#endif // SBIO_CORE_LOCATOR_HH
