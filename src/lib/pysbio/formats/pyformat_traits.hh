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

#ifndef PYSBIO_FORMATS_PYFORMAT_TRAITS_HH
#define PYSBIO_FORMATS_PYFORMAT_TRAITS_HH

#include <cstdint>
#include <type_traits>

namespace pysbio {
  enum class FTraits : std::uint8_t {
    XTC1 = 0,
    XTC2 = 1,
    CXI  = 2
  };

  /**
   * Call `fn` with the request keys of a group's data format, from an (alg, field) pair.
   *
   * Formats whose requests only name a field (e.g. XTC1) have no algorithms: the
   * bindings present their fields under the "raw" algorithm, which is dropped here.
   *
   * @tparam Group The BrokerGroup type.
   */
  template <class Group, class Fn>
  decltype(auto) with_request_keys(const char* alg, const char* field, Fn&& fn) {
    using Schema = typename std::remove_cvref_t<Group>::DataFormat::RequestSchema;
    if constexpr (Schema::count == 1) {
      return fn(field);
    } else {
      return fn(alg, field);
    }
  }
} // namespace pysbio

#endif // PYSBIO_FORMATS_PYFORMAT_TRAITS_HH
