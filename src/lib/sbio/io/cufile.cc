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

#include "sbio/io/cufile.hh"

#include "sbio/core/io.hh"
#include "sbio/cuda/utilities.hh"

#include <cufile.h>

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

#include <windows.h>
#else
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace sbio {
  cuFileIO::cuFileIO()
    : IOPolicy<cuFileIO>()
#ifdef _WIN32
    , m_file(INVALID_HANDLE_VALUE)
#else
    , m_fd(-1)
#endif
  {
    std::memset(&m_descr, 0, sizeof(m_descr));
  }

  cuFileIO::~cuFileIO() {
    if (m_handle) {
      cuFileHandleDeregister(m_handle);
    }

#ifdef _WIN32
    if (m_file != INVALID_HANDLE_VALUE) {
      ::CloseHandle(m_file);
    }
#else
    if (m_fd >= 0) {
      ::close(m_fd);
    }
#endif // _WIN32

    cuFileDriverClose();
  }

  IOStatus cuFileIO::connect(const char* path) {
    if (checkCuFile(cuFileDriverOpen())) {
      std::cerr << "Error opening cuFile driver" << std::endl;
      return IOStatus::OpenFailed;
    }

#ifdef _WIN32
    // TODO: FILE_FLAG_OVERLAPPED on Windows has restrictions that O_DIRECT
    //       does not --> Need to understand what these are and how to handle...
    m_file = CreateFileA(path,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         nullptr,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED | FILE_FLAG_OVERLAPPED,
                         nullptr);

    if (m_file == INVALID_HANDLE_VALUE) {
      return IOStatus::OpenFailed;
    }
#else
    m_fd = ::open(path, O_RDONLY | O_DIRECT);
    if (m_fd < 0) {
      return IOStatus::OpenFailed;
    }
#endif // _WIN32

#ifdef _WIN32
    m_descr.handle.handle = m_file;
    m_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_WIN32;
#else
    m_descr.handle.fd = m_fd;
    m_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
#endif // _WIN32

    if (checkCuFile(cuFileHandleRegister(&m_handle, &m_descr))) {
#ifdef _WIN32
      std::cerr << "Error registering file handle" << std::endl;
      ::CloseHandle(m_file);
      m_file = INVALID_HANDLE_VALUE;
#else
      std::cerr << "Error registering file handle for fd " << m_fd << std::endl;
      ::close(m_fd);
      m_fd = -1;
#endif // _WIN32
      cuFileDriverClose();
      return IOStatus::OpenFailed;
    } else {
      std::cout << "cuFile handle registered" << std::endl;
    }

    fs::path fs_path { path };
    m_file_size = fs::file_size(fs_path);
    return IOStatus::Success;
  }

  IOStatus cuFileIO::read(std::uint64_t offset, std::size_t size, void* dest) {
    if (checkCuFile(cuFileBufRegister(dest, size, 0))) {
      std::cerr << "Unable to register GPU buffer with cuFile." << std::endl;
      return IOStatus::GeneralIOError;
    }

    ssize_t read_count { cuFileRead(m_handle, dest, size, offset, 0) };
    cuFileBufDeregister(dest);

    if (read_count < 0) {
      this->m_read_count = 0;

      return IOStatus::GeneralIOError;
    } else if (read_count == 0) {
      this->m_read_count = 0;

      return IOStatus::ZeroBytesRead;
    }

    this->m_read_count = read_count;
    this->m_total_bytes_read += read_count;

    if (read_count != static_cast<ssize_t>(size)) {
      return IOStatus::TruncatedRead;
    }

    return IOStatus::Success;
  }
} // namespace sbio
