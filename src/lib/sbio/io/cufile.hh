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

#ifdef _WIN32
#include <windows.h> // HANDLE for FileHandle
#endif // _WIN32

namespace sbio {
  /**
   * A IO protocol using cuFile APIs in conjunction with POSIX descriptors.
   */
  struct SBIO_API cuFileIO : public IOPolicy<cuFileIO> {
  public:
    /**
     * Read into device with cuFile and into host using pread.
     *
     * @note Device reads use O_DIRECT. cuFile can nhandle unaligned reads, but
     *       aligned sizes and offsets are faster.
     */
    static constexpr IOCapability Capabilities {
      IOCapability::ReadToDevice    |
      IOCapability::ReadToHost      |
      IOCapability::ReadToPinned    |
      IOCapability::ConcurrentReads
    };

    cuFileIO();
    ~cuFileIO();

    cuFileIO(const cuFileIO& other) = delete;
    cuFileIO& operator=(const cuFileIO& other) = default;

    cuFileIO(cuFileIO&& other) noexcept;
    cuFileIO& operator=(cuFileIO&& other) noexcept;

    IOStatus connect(const char* path);

#ifndef _WIN32
    IOStatus connect(int fd);
#endif // _WIN32

    ReadResult read_impl(std::uint64_t offset, std::size_t size, void* dest) const;

    /**
     * Hold a reference to the cuFile driver (opened by the first reference).
     *
     * A Context holds one for its lifetime, as does every connected cuFileIO.
     *
     * @param[out] error If the driver can not be opened, a description of the problem.
     * @returns `true` if the driver is open.
     */
    static bool open_environment(const char*& error);

    /**
     * Release a reference to the cuFile driver (closed with the last reference).
     */
    static void close_environment();

    /**
     * Register device memory with cuFile, for faster reads into it.
     *
     * Registration is optional: unregistered memory is read through cuFile's own
     * bounce buffers.
     */
    static bool register_buffer(void* ptr, std::size_t size);

    static void deregister_buffer(void* ptr);

  private:
    ReadResult read_host(std::uint64_t offset, std::size_t size, void* dest) const;

    void release();

    void take(cuFileIO& other);

#ifdef _WIN32
    HANDLE m_file;
#else
    int m_fd;
    int m_host_fd { -1 };
#endif // _WIN32
    CUfileDescr_t m_descr;
    CUfileHandle_t m_handle { nullptr };
    bool m_holds_driver { false };
  };
} // namespace sbio

#endif // SBIO_IO_CUFILE_HH
