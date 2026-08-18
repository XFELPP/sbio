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
    , m_fd(-1)
  {
    std::memset(&m_descr, 0, sizeof(m_descr));
  }

  cuFileIO::~cuFileIO() {
    if (m_handle) {
      cuFileHandleDeregister(m_handle);
    }

    if (m_fd >= 0) {
      ::close(m_fd);
    }

    cuFileDriverClose();
  }

  IOStatus cuFileIO::connect(const char* path) {
    if (checkCuFile(cuFileDriverOpen())) {
      std::cerr << "Error opening cuFile driver" << std::endl;
      return IOStatus::OpenFailed;
    }

    m_fd = ::open(path, O_RDONLY | O_DIRECT);
    if (m_fd < 0) {
      return IOStatus::OpenFailed;
    }

    m_descr.handle.fd = m_fd;
    m_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;

    if (checkCuFile(cuFileHandleRegister(&m_handle, &m_descr))) {
      std::cerr << "Error registering file handle for fd " << m_fd << std::endl;
      ::close(m_fd);
      cuFileDriverClose();
      m_fd = -1;
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
