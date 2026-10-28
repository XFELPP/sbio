/*
 * sbio - Stream Broker IO
 *
 * Copyright (C) 2025-2026 Gabriel Dorlhiac
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "sbio/formats/hdf5/vfd_driver.hh"

#include <H5FDdevelop.h>
#include <hdf5.h>

#include <algorithm>

namespace sbio {
  // Create a single static driver_id in shared library
  hid_t HDF5VFDDriver::register_driver() {
    static std::once_flag once;
    static hid_t driver_id { H5I_INVALID_HID };

    auto register_driver = []() {
      driver_id = H5FDregister(&driver_class());
    };
    std::call_once(once, register_driver);

    return driver_id;
  }
} // namespace sbio
