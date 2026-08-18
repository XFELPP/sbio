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

#ifndef SBIO_IO_CUFILE_HH
#define SBIO_IO_CUFILE_HH

#include "sbio/core/io.hh"
#include "sbio/export_macro.hh"

#include <cufile.h>

#include <cstddef>
#include <cstdint>

namespace sbio {
  /**
   * A IO protocol using cuFile APIs in conjunction with POSIX descriptors.
   */
  struct SBIO_API cuFileIO : public IOPolicy<cuFileIO> {
  public:
    cuFileIO();
    ~cuFileIO();

    IOStatus connect(const char* path);

    IOStatus read(std::uint64_t offset, std::size_t size, void* dest);

  private:
#ifdef _WIN32
    HANDLE m_file;
#else
    int m_fd;
#endif // _WIN32
    CUfileDescr_t m_descr;
    CUfileHandle_t m_handle;
  };
} // namespace sbio

#endif // SBIO_IO_CUFILE_HH
