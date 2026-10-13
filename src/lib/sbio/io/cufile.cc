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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <utility>

namespace fs = std::filesystem;

namespace sbio {
  namespace {
    // Keep track of how many references to the driver there are for closing
    std::mutex driver_mutex;
    std::size_t driver_refs { 0 };
  } // anonymous namespace

  bool cuFileIO::open_environment(const char*& error) {
    std::lock_guard<std::mutex> lock(driver_mutex);
    if (driver_refs == 0 && checkCuFile(cuFileDriverOpen())) {
      error = "Unable to open cuFile driver!";
      return false;
    }

    ++driver_refs;
    return true;
  }

  void cuFileIO::close_environment() {
    std::lock_guard<std::mutex> lock(driver_mutex);
    if (driver_refs > 0 && --driver_refs == 0) {
      cuFileDriverClose();
    }
  }

  bool cuFileIO::register_buffer(void* ptr, std::size_t size) {
    return checkCuFile(cuFileBufRegister(ptr, size, 0)) == 0;
  }

  void cuFileIO::deregister_buffer(void* ptr) {
    cuFileBufDeregister(ptr);
  }

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
    release();
  }

  cuFileIO::cuFileIO(cuFileIO&& other) noexcept
    : IOPolicy<cuFileIO>(std::move(other))
  {
    take(other);
  }

  cuFileIO& cuFileIO::operator=(cuFileIO&& other) noexcept {
    if (this != &other) {
      release();
      IOPolicy<cuFileIO>::operator=(std::move(other));
      take(other);
    }

    return *this;
  }

  void cuFileIO::take(cuFileIO& other) {
#ifdef _WIN32
    m_file = other.m_file;
    other.m_file = INVALID_HANDLE_VALUE;
#else
    m_fd = other.m_fd;
    m_host_fd = other.m_host_fd;
    other.m_fd = -1;
    other.m_host_fd = -1;
#endif // _WIN32
    std::memcpy(&m_descr, &other.m_descr, sizeof(m_descr));
    std::memset(&other.m_descr, 0, sizeof(other.m_descr));
    m_handle = other.m_handle;
    other.m_handle = nullptr;
    m_holds_driver = other.m_holds_driver;
    other.m_holds_driver = false;
  }

  void cuFileIO::release() {
    if (m_handle) {
      cuFileHandleDeregister(m_handle);
    }

#ifdef _WIN32
    if (m_file != INVALID_HANDLE_VALUE) {
      ::CloseHandle(m_file);
    }
#else
    if (m_host_fd >= 0 && m_host_fd != m_fd) {
      ::close(m_host_fd);
    }

    if (m_fd >= 0) {
      ::close(m_fd);
    }
#endif // _WIN32

    if (m_holds_driver) {
      close_environment();
    }

#ifdef _WIN32
    m_file = INVALID_HANDLE_VALUE;
#else
    m_fd = -1;
    m_host_fd = -1;
#endif // _WIN32
    m_handle = nullptr;
    m_holds_driver = false;
  }

  IOStatus cuFileIO::connect(const char* path) {
    const char* error { nullptr };
    if (!open_environment(error)) {
      std::cerr << "Error opening cuFile driver" << std::endl;
      return IOStatus::OpenFailed;
    }
    m_holds_driver = true;

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

    // Dont need to enforce O_DIRECT for host only
    m_host_fd = ::open(path, O_RDONLY);
    if (m_host_fd < 0) {
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
      return IOStatus::OpenFailed;
    } else {
      std::cout << "cuFile handle registered" << std::endl;
    }

    fs::path fs_path { path };
    m_file_size = fs::file_size(fs_path);
    return IOStatus::Success;
  }

#ifndef _WIN32
  IOStatus cuFileIO::connect(int fd) {
    const char* error { nullptr };
    if (!open_environment(error)) {
      std::cerr << "Error opening cuFile driver" << std::endl;
      return IOStatus::OpenFailed;
    }
    m_holds_driver = true;

    m_fd = fd;
    m_host_fd = fd;

    m_descr.handle.fd = m_fd;
    m_descr.type = CU_FILE_HANDLE_TYPE_OPAQUE_FD;
    if (checkCuFile(cuFileHandleRegister(&m_handle, &m_descr))) {
      std::cerr << "Error registering file handle for fd " << m_fd << std::endl;
      // Since we were passed the fd directly, dont close it here
      return IOStatus::OpenFailed;
    }

    struct stat st;
    if (::fstat(fd, &st) == 0) {
      m_file_size = st.st_size;
    }

    return IOStatus::Success;
  }
#endif // _WIN32

  ReadResult cuFileIO::read_impl(std::uint64_t offset,
                                 std::size_t size,
                                 void* dest) const {
    cudaPointerAttributes attr {};
    const bool on_device {
      cudaPointerGetAttributes(&attr, dest) == cudaSuccess && attr.type == cudaMemoryTypeDevice
    };

    if (!on_device) {
      // Apparently depending on version you can get an error for unregistered host memory
      // so just clear it here so to not confuse later
      cudaGetLastError();
      return read_host(offset, size, dest);
    }

    ssize_t read_count { cuFileRead(m_handle, dest, size, offset, 0) };

    return classify_read(read_count, size);
  }

  ReadResult cuFileIO::read_host(std::uint64_t offset,
                                 std::size_t size,
                                 void* dest) const {
#ifdef _WIN32
    // No Windows impl yet....
    (void)offset;
    (void)size;
    (void)dest;
    return { IOStatus::FunctionUnavailable, 0 };
#else
    char* out { static_cast<char*>(dest) };
    std::size_t total { 0 };
    while (total < size) {
      const ssize_t got {
        ::pread(m_host_fd, out + total, size - total, offset + total)
      };

      if (got < 0) {
        return classify_read(-1, size);
      } else if (got == 0) {
        break;
      }

      total += static_cast<std::size_t>(got);
    }

    return classify_read(static_cast<ssize_t>(total), size);
#endif // _WIN32
  }
} // namespace sbio
