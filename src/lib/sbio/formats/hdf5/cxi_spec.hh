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

#ifndef SBIO_FORMATS_HDF5_CXI_SPEC_HH
#define SBIO_FORMATS_HDF5_CXI_SPEC_HH

#include "sbio/formats/hdf5/hdf5_format.hh"

#include <array>

namespace sbio {
struct CXISpec {
  static constexpr const char* Name { "cxi" };

  // Usually a soft link to /entry_1/instrument_1/detector_1/data
  // - H5Dopen2 follows it
  static constexpr const char* FramesPath { "/entry_1/data_1/data" };

  // Cheetah-style per-frame data
  static constexpr std::array<const char*, 2> PerStepRoots {
    "/LCLS",
    "/entry_1/result_1",
  };

  // TODO: Example... setup real ones
  static constexpr std::array<const char*, 4> StaticPaths {
    "/entry_1/instrument_1/detector_1/distance",
    "/entry_1/instrument_1/detector_1/x_pixel_size",
    "/entry_1/instrument_1/detector_1/y_pixel_size",
    "/entry_1/instrument_1/source_1/energy",
  };
};

using CXITraits = HDF5Format<CXISpec>;
} // namespace sbio

#endif // SBIO_FORMATS_HDF5_CXI_SPEC_HH
