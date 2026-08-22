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

#ifndef SBIO_IO_POSIX_HH
#define SBIO_IO_POSIX_HH

#include "sbio/core/io.hh"
#include "sbio/export_macro.hh"

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include <cstddef>
#include <cstdint>

namespace sbio {
  /**
   * A very basic IO implementation over standard synchronous POSIX APIs.
   *
   * This IO implementation provides solely synchronous IO capabilities over a
   * Stream using standard POSIX APIs.
   *
   * The term POSIX is a misnomer if on Windows as the POSIX `pread`, etc, APIs
   * are clearly not available. In this case, however, the equivalent behaviour
   * is provided using the Windows APIs.
   */
  struct SBIO_API SyncPOSIXIO : public IOPolicy<SyncPOSIXIO> {
  public:

    SyncPOSIXIO();

    ~SyncPOSIXIO();

    IOStatus connect(const char* path);

#ifdef _WIN32
    IOStatus connect(HANDLE h_file);
#else
    IOStatus connect(int fd);
#endif

    IOStatus read(std::uint64_t offset, std::size_t size, void* dest);

  private:
#ifdef _WIN32
    HANDLE m_file;
#else
    int m_fd;
#endif // _WIN32
  };
} // namespace sbio

#endif // SBIO_IO_POSIX_HH
