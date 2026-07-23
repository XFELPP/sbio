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

#ifndef SBIO_UTIL_MPI_HH
#define SBIO_UTIL_MPI_HH

#include <mpi.h>

#include <cstdint>
#include <type_traits>

namespace sbio::mpi {
  /**
   * A static dispatcher for converting C++ types to MPI types.
   *
   * @tparam T The input data type.
   * @returns The MPI_Datatype signifier for T.
   */
  template <typename T>
  inline MPI_Datatype type_for() {
    using U = std::remove_cvref_t<T>;

    if constexpr (std::is_same_v<U, bool>) {
      return MPI_C_BOOL;
    } else if constexpr (std::is_same_v<U, int>) {
      return MPI_INT;
    } else if constexpr (std::is_same_v<U, unsigned int>) {
      return MPI_UNSIGNED;
    } else if constexpr (std::is_same_v<U, long>) {
      return MPI_LONG;
    } else if constexpr (std::is_same_v<U, unsigned long>) {
      return MPI_UNSIGNED_LONG;
    } else if constexpr (std::is_same_v<U, long long>) {
      return MPI_LONG_LONG;
    } else if constexpr (std::is_same_v<U, unsigned long long>) {
      return MPI_UNSIGNED_LONG_LONG;
    } else if constexpr (std::is_same_v<U, float>) {
      return MPI_FLOAT;
    } else if constexpr (std::is_same_v<U, double>) {
      return MPI_DOUBLE;
    } else if constexpr (std::is_same_v<U, char>) {
      return MPI_CHAR;
    } else if constexpr (std::is_same_v<U, std::uint8_t>) {
      return MPI_UINT8_T;
    } else if constexpr (std::is_same_v<U, std::uint32_t>) {
      return MPI_UINT32_T;
    } else if constexpr (std::is_same_v<U, std::uint64_t>) {
      return MPI_UINT64_T;
    } else if constexpr (std::is_same_v<U, std::int32_t>) {
      return MPI_INT32_T;
    } else if constexpr (std::is_same_v<U, std::int64_t>) {
      return MPI_INT64_T;
    }

    return MPI_DATATYPE_NULL;
  }

  /**
   * Return the supported upper bound for a tag.
   *
   * The routine finds the largest supported tag value by the current MPI implementation
   * which can be used to avoid overflows in tag calculations when sending messages.
   *
   * @returns The tag upper bound.
   */
  inline int tag_upper_bound() {
    // 32767 is the minimum upper bound for a compliant MPI implementation
    int max_tag { 32767 };

    // But most implementations support more, so query
    int initialized { 0 };
    MPI_Initialized(&initialized);
    if (initialized) {
      int* tag_ub_ptr { nullptr };
      int flag { 0 };
      MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_TAG_UB, &tag_ub_ptr, &flag);
      if (flag && tag_ub_ptr) {
        max_tag = *tag_ub_ptr;
      }
    }

    return max_tag;
  }
} // namespace sbio::mpi

#endif // SBIO_UTIL_MPI_HH
