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

#include "sbio/io/posix.hh"

#include "sbio/core/io.hh"

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

#include <windows.h>
#else
#include <sys/types.h>
#endif

#include <cstddef>
#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;

namespace sbio {
  SyncPOSIXIO::SyncPOSIXIO()
    : IOPolicy<SyncPOSIXIO>()
#ifdef _WIN32
    , m_file(INVALID_HANDLE_VALUE)
#else
    , m_fd(-1)
#endif
  {}

  SyncPOSIXIO::~SyncPOSIXIO() {
#ifdef _WIN32
    if (m_file != INVALID_HANDLE_VALUE) {
      ::CloseHandle(m_file);
    }
#else
    if (m_fd >= 0) {
      ::close(m_fd);
    }
#endif // _WIN32
  }

  IOStatus SyncPOSIXIO::connect(const char* path) {
#ifdef _WIN32
    m_file = CreateFileA(path,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         nullptr,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,
                         nullptr);

    if (m_file == INVALID_HANDLE_VALUE) {
      return IOStatus::OpenFailed;
    }
#else
    m_fd = ::open(path, O_RDONLY);
    if (m_fd < 0) {
      return IOStatus::OpenFailed;
    }
#endif // _WIN32

    fs::path fs_path = path;
    m_file_size = fs::file_size(fs_path);
    return IOStatus::Success;
  }

  IOStatus SyncPOSIXIO::read(std::uint64_t offset, std::size_t size, void* dest) {
#ifdef _WIN32
    OVERLAPPED overlapped {};
    overlapped.Offset = static_cast<DWORD>(offset & 0xffffffff);
    overlapped.OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xffffffff);

    DWORD bytes_read { 0 };
    BOOL ok {
      ReadFile(m_file, dest, static_cast<DWORD>(size), &bytes_read, &overlapped)
    };

    ssize_t read_count { -1 };
    if (ok) {
      read_count = static_cast<ssize_t>(bytes_read);
    } else if (::GetLastError() == ERROR_IO_PENDING) {
      if (::GetOverlappedResult(m_file, &overlapped, &bytes_read, TRUE)) {
        read_count = static_cast<ssize_t>(bytes_read);
      }
    }
#else
    ssize_t read_count { ::pread(m_fd, dest, size, offset) };
#endif // _WIN32
    if (read_count == 0) {
      m_read_count = 0;
      return IOStatus::ZeroBytesRead;
    } else if (read_count == -1) {
      m_read_count = 0;
      return IOStatus::GeneralIOError;
    }

    m_read_count = read_count;
    m_total_bytes_read += read_count;
    if (read_count != static_cast<ssize_t>(size)) {
      return IOStatus::TruncatedRead;
    }
    return IOStatus::Success;
  }
} // namespace sbio
