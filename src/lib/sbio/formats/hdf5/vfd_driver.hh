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

#ifndef SBIO_FORMATS_HDF5_VFD_DRIVER_HH
#define SBIO_FORMATS_HDF5_VFD_DRIVER_HH

#include "sbio/core/io.hh"
#include "sbio/export_macro.hh"

#include <hdf5.h>
#include <H5FDdevelop.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>

static_assert(H5_VERSION_GE(1, 14, 0), "sbio HDF5 support requires HDF5 >= 1.14");

namespace sbio {
  /**
   * @brief HDF5 virtual file driver routing libhdf5 file IO through an sbio IOPolicy.
   *
   * @note This is read-only currently.
   *
   * @note The driver is never given ownership of IOPolicy. It borrows from the
   *       StreamBroker whose file is opened (via fapl). The IO policy (and StreamBroker)
   *       therefore must live longer than every HDF5 file id opened with it.
   */
  struct HDF5VFDDriver {
    static constexpr const char* Name { "sbio" };
    static constexpr H5FD_class_value_t Value { 600 }; // >= 512 - that is outside (HDF5) library/testing ranges

    /**
     * Type-erased view of an IOPolicy for the HDF5 driver.
     *
     * @note Must be trivially copyable as libhdf5 memcpy's it into the fapl
     *       (fapl_size set, fapl_copy null).
     */
    struct Info {
      const void* io { nullptr };

      ReadResult (*read)(const void* io,
                         std::uint64_t offset,
                         std::size_t size,
                         void* dest) { nullptr };

      std::size_t (*file_size)(const void* io) { nullptr };
    };

    struct FileHandle {
      H5FD_t pub;   ///< Must be first because libhdf5 casts between H5FD_t* and FileHandle*
      Info info;
      haddr_t eoa;
      haddr_t eof;
    };

    template <IOTraits IO>
    static Info make_info(const IO& io) {
      return Info {
        &io,
        [](const void* p, std::uint64_t off, std::size_t sz, void* dst) {
          return static_cast<const IO*>(p)->read(off, sz, dst);
        },
        [](const void* p) {
          return static_cast<std::size_t>(static_cast<const IO*>(p)->file_size());
        }
      };
    }

    /**
     * @brief Build a file access property list that opens files through `io`.
     *
     * @returns A fapl id (caller closes with H5Pclose), or H5I_INVALID_HID on failure.
     */
    template <IOTraits IO>
    static hid_t make_fapl(const IO& io) {
      const hid_t driver { register_driver() };
      if (driver < 0) {
        return H5I_INVALID_HID;
      }

      const hid_t fapl { H5Pcreate(H5P_FILE_ACCESS) };
      if (fapl < 0) {
        return H5I_INVALID_HID;
      }

      const Info info { make_info(io) };
      if (H5Pset_driver(fapl, driver, &info) < 0) {
        H5Pclose(fapl);
        return H5I_INVALID_HID;
      }

      return fapl;
    }

    // Put in shared library to get a single static driver_id
    // No public API as far as I can tell to retrieve a registered driver by name
    SBIO_API static hid_t register_driver();

  private:
    static const H5FD_class_t& driver_class() {
      static const H5FD_class_t cls = [] {
        H5FD_class_t H5FD_sbio_vfd = {
          /* struct_version  = */ H5FD_CLASS_VERSION,
          /* value           = */ Value,
          /* name            = */ Name,
          /* maxaddr         = */ HADDR_MAX,
          /* fc_degree       = */ H5F_CLOSE_WEAK,
          /* terminate       = */ nullptr,
          /* sb_size         = */ nullptr,
          /* sb_encode       = */ nullptr,
          /* sb_decode       = */ nullptr,
          /* fapl_size       = */ sizeof(Info),
          /* fapl_get        = */ nullptr,
          /* fapl_copy       = */ nullptr,
          /* fapl_free       = */ nullptr,
          /* dxpl_size       = */ 0,
          /* dxpl_copy       = */ nullptr,
          /* dxpl_free       = */ nullptr,
          /* open            = */ &open,
          /* close           = */ &close,
          /* cmp             = */ &cmp, // Compare two driver's files - consistent ordering.
          /* query           = */ &query, // Supported features (SWMR?, metadata alloc, metadata accum? etc.)
          /* get_type_map    = */ nullptr,
          /* alloc           = */ nullptr,
          /* free            = */ nullptr,
          /* get_eoa         = */ &get_eoa,
          /* set_eoa         = */ &set_eoa,
          /* get_eof         = */ &get_eof,
          /* get_handle      = */ nullptr,
          /* read            = */ &read,
          /* write           = */ &write,
          /* read_vector     = */ nullptr,
          /* write_vector    = */ nullptr,
          /* read_selection  = */ nullptr,
          /* write_selection = */ nullptr,
          /* flush           = */ nullptr,
          /* truncate        = */ nullptr,
          /* lock            = */ nullptr,
          /* unlock          = */ nullptr,
          /* delete          = */ nullptr,
          /* ctl             = */ nullptr,
          /* fl_map          = */ H5FD_FLMAP_DICHOTOMY
        };

        return H5FD_sbio_vfd;
      }();

      return cls;
    }

    static FileHandle* handle(H5FD_t* f) { return reinterpret_cast<FileHandle*>(f); }
    static const FileHandle* handle(const H5FD_t* f) {
      return reinterpret_cast<const FileHandle*>(f);
    }

    static H5FD_t* open(const char* name,
                        unsigned flags,
                        hid_t fapl_id,
                        haddr_t maxaddr) {
      if (flags & (H5F_ACC_RDWR | H5F_ACC_TRUNC | H5F_ACC_CREAT | H5F_ACC_EXCL)) {
        return nullptr; // read-only driver
      }

      if (maxaddr == 0 || maxaddr == HADDR_UNDEF) {
        return nullptr;
      }

      const auto* info { static_cast<const Info*>(H5Pget_driver_info(fapl_id)) };
      if (info == nullptr          ||
          info->io == nullptr      ||
          info->read == nullptr    ||
          info->file_size == nullptr) {
        return nullptr;
      }

      auto* fh { new (std::nothrow) FileHandle {} };
      if (fh == nullptr) {
        return nullptr;
      }

      fh->info = *info;
      fh->eof = static_cast<haddr_t>(info->file_size(info->io));
      fh->eoa = 0;

      return &fh->pub;
    }

    static herr_t close(H5FD_t* f) {
      delete handle(f); // IO is borrowed, so do not close here
      return 0;
    }

    static int cmp(const H5FD_t* a, const H5FD_t* b) {
      const auto* ia { static_cast<const std::byte*>(handle(a)->info.io) };
      const auto* ib { static_cast<const std::byte*>(handle(b)->info.io) };

      return (ia < ib) ? -1 : (ia > ib) ? 1 : 0;
    }

    static herr_t query(const H5FD_t* f, unsigned long* flags) {
      if (flags != nullptr) {
        *flags =
          H5FD_FEAT_AGGREGATE_METADATA  |
          H5FD_FEAT_ACCUMULATE_METADATA |
          H5FD_FEAT_DATA_SIEVE          |
          H5FD_FEAT_AGGREGATE_SMALLDATA;
      }

      return 0;
    }

    static haddr_t get_eoa(const H5FD_t* f, H5FD_mem_t) { return handle(f)->eoa; }

    static herr_t set_eoa(H5FD_t* f, H5FD_mem_t, haddr_t addr) {
      handle(f)->eoa = addr;

      return 0;
    }

    static haddr_t get_eof(const H5FD_t* f, H5FD_mem_t) { return handle(f)->eof; }

    static herr_t read(H5FD_t* f,
                       H5FD_mem_t,
                       hid_t,
                       haddr_t addr,
                       std::size_t size,
                       void* buf) {
      const FileHandle* fh { handle(f) };
      if (addr == HADDR_UNDEF || buf == nullptr) {
        return -1;
      }

      auto* out { static_cast<std::byte*>(buf) };
      std::size_t done { 0 };

      // Read what exists before EOF (looping on short reads) and zero-fill the rest
      while (done < size && addr + done < fh->eof) {
        const std::size_t want {
          static_cast<std::size_t>(std::min<haddr_t>(size - done, fh->eof - (addr + done)))
        };

        const ReadResult r { fh->info.read(fh->info.io, addr + done, want, out + done) };
        if (r.bytes == 0                                                          ||
            (r.status != IOStatus::Success && r.status != IOStatus::TruncatedRead)) {
          return -1;
        }

        done += r.bytes;
      }

      if (done < size) {
        std::memset(out + done, 0, size - done);
      }

      return 0;
    }

    static herr_t write(H5FD_t*, H5FD_mem_t, hid_t, haddr_t, std::size_t, const void*) {
      return -1; // read-only until the sbio write API exists
    }
  };

} // namespace sbio

#endif // SBIO_FORMATS_HDF5_VFD_DRIVER_HH
