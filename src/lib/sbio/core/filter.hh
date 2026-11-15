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

#ifndef SBIO_CORE_FILTER_HH
#define SBIO_CORE_FILTER_HH

#ifdef __CUDACC__

#include <cuda/std/bitset>
#include <cuda/std/cstddef>
#include <cuda/std/cstdint>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <bitset>
#include <cstddef>
#include <cstdint>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

namespace sbio {
  template <typename AccessPtn>
  struct SubKindFilter {
    SBIO_HD void enable_all() {
      m_filter.set();
    }

    SBIO_HD void disable_all() {
      m_filter.reset();
    }

    SBIO_HD void allow_subkind(hd_std::size_t sk) {
      m_filter.set(sk);
    }

    SBIO_HD void disable_subkind(hd_std::size_t sk) {
      m_filter.reset(sk);
    }

    SBIO_HD hd_std::size_t full_filter() const { return m_filter.to_ullong(); }

    SBIO_HD operator hd_std::size_t() const { return full_filter(); }

    SBIO_HD bool operator==(const SubKindFilter& other) const = default;

    SBIO_HD SubKindFilter& operator&=(const SubKindFilter& other) {
      if (this != &other) {
        m_filter &= static_cast<hd_std::size_t>(other);
      }

      return *this;
    }

    SBIO_HD SubKindFilter& operator|=(const SubKindFilter& other) {
      if (this != &other) {
        m_filter |= static_cast<hd_std::size_t>(other);
      }

      return *this;
    }

    SBIO_HD SubKindFilter operator&(const SubKindFilter& other) const {
      auto tmp { *this };
      tmp &= other;

      return tmp;
    }

    SBIO_HD SubKindFilter operator|(const SubKindFilter& other) const {
      auto tmp { *this };
      tmp |= other;

      return tmp;
    }

  private:
    hd_std::bitset<AccessPtn::SubKindCount> m_filter { static_cast<hd_std::size_t>(~0) };
  };

} // namespace sbio

#endif // SBIO_CORE_FILTER_HH
