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

#include "sbio/posix_io.hh"

#include "sbio/core/io.hh"

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
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
    , m_fd(-1)
  {}

  IOStatus SyncPOSIXIO::connect(const char* path) {
    m_fd = ::open(path, O_RDONLY);
    if (m_fd < 0) {
      return IOStatus::OpenFailed;
    }

    fs::path fs_path = path;
    m_file_size = fs::file_size(fs_path);
    return IOStatus::Success;
  }

  IOStatus SyncPOSIXIO::read(std::uint64_t offset, std::size_t size, void* dest) {
    ssize_t read_count { ::pread(m_fd, dest, size, offset) };
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
