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

#ifndef SBIO_CORE_TRANSACTION_HH
#define SBIO_CORE_TRANSACTION_HH

#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/sync.hh"

#include <ncarray/ncarrays.hh>

#ifdef __CUDACC__

#include <cuda/std/atomic>
#include <cuda/std/cassert>
#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/type_traits>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <type_traits>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif

namespace sbio {
  template <class Role, class EPolicy, class StorageT>
  class Transaction {
  public:

    using StorageViewType = StorageView<StorageT, EPolicy>;

    SBIO_HD Transaction(StorageT& storage)
      : m_storage(storage)
    {
      EPolicy::template pre_update<Role>(storage);
#ifndef NDEBUG
      m_opened.store(true, std::memory_order_release);
#endif
    }

    SBIO_HD ~Transaction() {
      assert((!m_opened.load(hd_std::memory_order_acquire)) &&
             "Transaction destroyed without committing!");

      assert((m_view_count.load(std::memory_order_acquire) == 0) &&
             "Transaction destroyed with outstanding views!");
    }

    SBIO_HD Transaction(Transaction&& other) noexcept = delete;
    SBIO_HD Transaction& operator=(Transaction&& other) noexcept = delete;

    Transaction(const Transaction& other) = delete;
    Transaction& operator=(const Transaction& other) = delete;

    SBIO_HD StorageViewType view() {
#ifndef NDEBUG
      m_view_count.fetch_add(1, hd_std::memory_order_relaxed);
      return StorageViewType(m_storage, &m_view_count);
#else
      return StorageViewType(m_storage);
#endif
    }

    template <class SyncT>
    SBIO_HD void commit(SyncT&& sync_vars, IOStatus status) {
      EPolicy::template post_update<Role>(m_storage, sync_vars, status);
#ifndef NDEBUG
      m_opened.store(false, hd_std::memory_order_release);
#endif
    }

  private:
    StorageT& m_storage;
#ifndef NDEBUG
    hd_std::atomic<bool> m_opened { false };
    hd_std::atomic<hd_std::size_t> m_view_count { 0 };
#endif
  };
} // namespace sbio
#endif // SBIO_CORE_TRANSACTION_HH
