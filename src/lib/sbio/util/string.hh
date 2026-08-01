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

#ifndef SBIO_UTIL_STRING_HH
#define SBIO_UTIL_STRING_HH

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

#include <cstddef>

namespace sbio {
  SBIO_HD inline void safe_strncpy(char* dst, const char* src, std::size_t max_len) {
    std::size_t i { 0 };
    for (; i < max_len - 1 && src[i] != '\0'; ++i) {
      dst[i] = src[i];
    }
    dst[i] = '\0';
  }
} // namespace sbio
#endif // SBIO_UTIL_STRING_HH
