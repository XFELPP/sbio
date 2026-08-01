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

#ifndef SBIO_CORE_SYNC_HH
#define SBIO_CORE_SYNC_HH

#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/formats/format_traits.hh"

#include <concepts>
#include <type_traits>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {

  /**
   * The SyncGroup specifies a series of references that may require synchronization.
   * This can be exposed by any data stream broker, allowing an execution policy to
   * to generically use appropriate synchronization mechanisms without apriori knowledge
   * of the number, or type.
   *
   * E.g. a broker may expose a SyncGroup of numbers that all parallel units of the
   * broker should be aware of. An MPI-based execution policy may use a Bcast to
   * facilitate the synchronization.
   */
  template <typename... Args>
  struct SyncGroup;
  // Recursive definition for a list of references
  template <typename T, typename... Rest>
  struct SyncGroup<T, Rest...> : SyncGroup<Rest...> {
    T& val;
    SBIO_HD SyncGroup(T& v, Rest&... rest)
      : SyncGroup<Rest...>(rest...)
      , val(v)
    {}

    /**
     * Recursively visit and run a callback on the held references.
     */
    template <class CBType>
    SBIO_HD void for_each(CBType&& callback) {
      callback(val);                          // Apply to current reference
      SyncGroup<Rest...>::for_each(callback); // ... and recurse
    }
  };

  // Null/empty base case
  template <>
  struct SyncGroup<> {
    template <class CBType>
    SBIO_HD void for_each(CBType&& callback) {}
  };

  /**
   * A variadic helper function to transform any number of objects into a SyncGroup.
   */
  template <typename... Args>
  auto make_sync_group(Args&... args) {
    return SyncGroup<Args...>(args...);
  }

} // namespace sbio

#endif // SBIO_CORE_SYNC_HH
