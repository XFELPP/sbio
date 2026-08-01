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

#ifndef SBIO_FORMATS_XTC1_SCHEMAS_HH
#define SBIO_FORMATS_XTC1_SCHEMAS_HH

#include "sbio/formats/xtc1/registry.hh"
#include "sbio/formats/xtc1/xtc1.hh"

#include <span>

namespace sbio::XTC1 {
  inline constexpr SimulatedField _fields_0[] = {
    { "offsetAndGain", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "outputOptions", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "defectPixelCount", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "output_lookup_table", DType::UINT16, 1, {}, 12, true, { "Output_LUT_Sizeoutput_lookup_table_enabled", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "defect_pixel_coordinates", DType::ENUMVAL, 1, {}, 12, true, { "number_of_defect_pixels", "", "", "", "", "", "", "", "", "" }, { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "Output_LUT_Size", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_0[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_1[] = {
    { "options", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "rows", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_1[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_2[] = {
    { "sizeThreshold", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_2[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_3[] = {
    { "fileOffset", DType::INT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "extent", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_4[] = {
    { "fileOffset", DType::INT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "type", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "extent", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_5[] = {
    { "numSrcAlias", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "srcAlias", DType::ENUMVAL, 1, {}, 4, true, { "numSrcAlias", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_5[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SchemaConstant _consts_6[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_7[] = {
    { "forwarding", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "forward_prescale", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "processing", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiBegin", DType::ENUMVAL, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiEnd", DType::ENUMVAL, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "threshold", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "masked_pixel_count", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "masked_pixel_coordinates", DType::ENUMVAL, 1, {}, 28, true, { "number_of_masked_pixels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_7[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_8[] = {
    { "width", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "depth", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "offset", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "_int_pixel_data", DType::UINT8, 1, {}, 16, true, { "_width_height_depth78", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_9[] = {
    { "integral", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "xmean", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ymean", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "major_axis_width", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "minor_axis_width", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "major_axis_tilt", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_10[] = {
    { "width", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "depth", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "color_mode", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposure_time", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gain", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "manufacturer", DType::CHARSTR, 1, { 48 }, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "model", DType::CHARSTR, 1, { 48 }, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_10[] = {
    { "DESC_CHAR_MAX", 48 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_11[] = {
    { "cooling", DType::UINT8, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "overlap", DType::UINT8, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noiseFilter", DType::UINT8, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "blemishCorrection", DType::UINT8, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "shutter", DType::UINT8, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fanSpeed", DType::UINT8, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "readoutRate", DType::UINT8, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerMode", DType::UINT8, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gainMode", DType::UINT8, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "setpoint", DType::UINT8, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad0", DType::UINT16, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "width", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "orgX", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "orgY", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "binX", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "binY", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::DOUBLE, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerDelay", DType::DOUBLE, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_11[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_12[] = {
    { "timestamp", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 8, true, { "numPixelsY", "numPixelsX", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_13[] = {
    { "range", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calRange", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "reset", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasData", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calData", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasDacData", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calStrobe", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfSamples", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "trigDelay", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcDelay", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_13[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_14[] = {
    { "first", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "second", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frameNumber", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fiducials", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "range", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "laneStatus", DType::ENUMVAL, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "samples", DType::ENUMVAL, 1, {}, 32, true, { "numberOfSamples", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_14[] = {
    { "Nsbtemp", 4 }
  };

  inline constexpr SimulatedField _fields_15[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8FanMode", DType::UINT8, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8BaselineClamp", DType::UINT8, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8HighCapacity", DType::UINT8, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8GainIndex", DType::UINT8, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ReadoutSpeedIndex", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ExposureEventCode", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32NumDelayShots", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_15[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_16[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8FanMode", DType::UINT16, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8CropMode", DType::UINT16, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8BaselineClamp", DType::UINT8, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8HighCapacity", DType::UINT8, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8GainIndex", DType::UINT8, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad1", DType::UINT8, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ReadoutSpeedIndex", DType::UINT16, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ExposureEventCode", DType::UINT16, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32NumDelayShots", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_16[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_17[] = {
    { "iShotIdStart", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fReadoutTime", DType::FLOAT, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fTemperature", DType::FLOAT, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 12, true, { "numPixelsY", "numPixelsX", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_18[] = {
    { "width", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfFrames", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfBytesPerPixel", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorType", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOn", DType::UINT32, 1, { 2 }, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOff", DType::UINT32, 1, { 2 }, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "delay", DType::UINT32, 1, { 2 }, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "readOnlyPots", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pots", DType::DOUBLE, 1, { 13 }, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOn", DType::UINT32, 1, {}, 0, true, { "NumberOfSides", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOff", DType::UINT32, 1, {}, 0, true, { "NumberOfSides", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "delay", DType::UINT32, 1, {}, 0, true, { "NumberOfSides", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pots", DType::DOUBLE, 1, {}, 0, true, { "NumberOfPots", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_18[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_19[] = {
    { "roiEnable", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiRows", DType::ENUMVAL, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiFrames", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "width", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfFrames", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfBytesPerPixel", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorType", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOn", DType::UINT32, 1, { 2 }, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOff", DType::UINT32, 1, { 2 }, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "delay", DType::UINT32, 1, { 2 }, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "readOnlyPots", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pots", DType::DOUBLE, 1, { 13 }, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOn", DType::UINT32, 1, {}, 0, true, { "NumberOfSides", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOff", DType::UINT32, 1, {}, 0, true, { "NumberOfSides", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "delay", DType::UINT32, 1, {}, 0, true, { "NumberOfSides", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pots", DType::DOUBLE, 1, {}, 0, true, { "NumberOfPots", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_19[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_20[] = {
    { "roiEnable", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiRows", DType::ENUMVAL, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiFrames", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "oscillator", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "width", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfFrames", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfBytesPerPixel", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorType", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOn", DType::UINT32, 1, { 2 }, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOff", DType::UINT32, 1, { 2 }, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "delay", DType::UINT32, 1, { 2 }, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "readOnlyPots", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pots", DType::DOUBLE, 1, { 13 }, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOn", DType::UINT32, 1, {}, 0, true, { "NumberOfSides", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeOff", DType::UINT32, 1, {}, 0, true, { "NumberOfSides", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "delay", DType::UINT32, 1, {}, 0, true, { "NumberOfSides", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pots", DType::DOUBLE, 1, {}, 0, true, { "NumberOfPots", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_20[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "MaxModulesPerDetector", 8 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_21[] = {
    { "acquisitionCount", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timestamp", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "temperature", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frames", DType::UINT16, 3, {}, 16, true, { "numberOfFrames", "height", "width", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_22[] = {
    { "gain_a_b", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_shutter", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "control", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_22[] = {
    { "Row_Pixels", 480 },
    { "Column_Pixels", 640 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_23[] = {
    { "gain_a_b", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_shutter", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "control", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_23[] = {
    { "Row_Pixels", 480 },
    { "Column_Pixels", 640 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_24[] = {
    { "NChannels", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Length", DType::UINT32, 1, {}, 4, true, { "_NChannels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "SampleType", DType::UINT32, 1, {}, 4, true, { "_NChannels", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_NChannels", "", "", "", "", "", "", "", "", "" } },
    { "Offset", DType::INT32, 1, {}, 4, true, { "_NChannels", "", "", "", "", "", "", "", "", "" }, { 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_NChannels", "_NChannels", "", "", "", "", "", "", "", "" } },
    { "Period", DType::DOUBLE, 1, {}, 4, true, { "_NChannels", "", "", "", "", "", "", "", "", "" }, { 4, 4, 4, 0, 0, 0, 0, 0, 0, 0 }, { "_NChannels", "_NChannels", "_NChannels", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_24[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_25[] = {
    { "data_size", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "_int_data", DType::UINT8, 1, {}, 4, true, { "_data_size", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_26[] = {
    { "binning_f", DType::UINT8, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "binning_s", DType::UINT8, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad", DType::INT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposure", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "trigger", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "rawMode", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "darkFlag", DType::UINT16, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "readoutMode", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "deviceID", DType::CHARSTR, 1, { 40 }, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_26[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_27[] = {
    { "binning_f", DType::UINT8, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "binning_s", DType::UINT8, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "testPattern", DType::INT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposure", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "trigger", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "rawMode", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "darkFlag", DType::UINT16, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "readoutMode", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "deviceID", DType::CHARSTR, 1, { 40 }, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_27[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_28[] = {
    { "timeRange", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "mode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gate", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gain", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "shutter", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerMode", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "focusTimeOver", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibScale", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calib", DType::DOUBLE, 1, { 3 }, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calib", DType::DOUBLE, 1, {}, 0, true, { "NumCalibConstants", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_28[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_29[] = {
    { "base", DType::FLOAT, 1, { 16 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "scale", DType::FLOAT, 1, { 16 }, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "base", DType::FLOAT, 1, {}, 0, true, { "NRANGES", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "scale", DType::FLOAT, 1, {}, 0, true, { "NRANGES", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_29[] = {
    { "NRANGES", 3 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_30[] = {
    { "base", DType::FLOAT, 1, { 16 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "scale", DType::FLOAT, 1, { 16 }, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "base", DType::FLOAT, 1, {}, 0, true, { "NRANGES", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "scale", DType::FLOAT, 1, {}, 0, true, { "NRANGES", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_30[] = {
    { "NRANGES", 16 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_31[] = {
    { "value", DType::FLOAT, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_32[] = {
    { "diode", DType::ENUMVAL, 1, { 4 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "xscale", DType::FLOAT, 0, {}, 512, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "yscale", DType::FLOAT, 0, {}, 516, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "diode", DType::ENUMVAL, 1, {}, 0, true, { "NCHANNELS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_32[] = {
    { "NCHANNELS", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_33[] = {
    { "diode", DType::ENUMVAL, 1, { 4 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "xscale", DType::FLOAT, 0, {}, 512, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "yscale", DType::FLOAT, 0, {}, 516, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "diode", DType::ENUMVAL, 1, {}, 0, true, { "NCHANNELS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_33[] = {
    { "NCHANNELS", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_34[] = {
    { "channel", DType::FLOAT, 1, { 4 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sum", DType::FLOAT, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "xpos", DType::FLOAT, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ypos", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel", DType::FLOAT, 1, {}, 0, true, { "NCHANNELS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_34[] = {
    { "NCHANNELS", 4 }
  };

  inline constexpr SimulatedField _fields_35[] = {
    { "xscale", DType::FLOAT, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "yscale", DType::FLOAT, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_35[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_36[] = {
    { "numLinks", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payloadSizePerLink", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_36[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_37[] = {
    { "numLinks", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payloadSizePerLink", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numChannels", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numRows", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numSubmoduleChannels", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numSubmoduleRows", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numSubmodules", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "camexMagic", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "info", DType::CHARSTR, 1, { 256 }, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timingFName", DType::CHARSTR, 1, { 256 }, 288, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_37[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_38[] = {
    { "specialWord", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frameNumber", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeStampHi", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeStampLo", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, { 1024, 1024 }, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 0, true, { "1024", "1024", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_40[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8GainIndex", DType::UINT8, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8ReadoutSpeedIndex", DType::UINT8, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ExposureEventCode", DType::UINT16, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32NumDelayShots", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_40[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_41[] = {
    { "iShotIdStart", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fReadoutTime", DType::FLOAT, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fTemperature", DType::FLOAT, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 12, true, { "numPixelsY", "numPixelsX", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_42[] = {
    { "cooling", DType::UINT8, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "overlap", DType::UINT8, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noiseFilter", DType::UINT8, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "blemishCorrection", DType::UINT8, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "mcpIntelligate", DType::UINT8, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fanSpeed", DType::UINT8, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "readoutRate", DType::UINT8, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerMode", DType::UINT8, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gainMode", DType::UINT8, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gateMode", DType::UINT8, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "insertionDelay", DType::UINT8, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad0", DType::UINT8, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "mcpGain", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad1", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "width", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "orgX", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "orgY", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "binX", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "binY", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::DOUBLE, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerDelay", DType::DOUBLE, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_42[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_43[] = {
    { "readoutMode", DType::UINT16, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sweepCount", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "integrationTime", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nonIntegrationTime", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "preSkipPixels", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixels", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "postSkipPixels", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "overscanPixels", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "preSkipLines", DType::UINT16, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lines", DType::UINT16, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "postSkipLines", DType::UINT16, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "overScanLines", DType::UINT16, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "horizontalBinning", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "verticalBinning", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "rgh", DType::UINT16, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "rgl", DType::UINT16, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "shp", DType::UINT16, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "shd", DType::UINT16, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "st", DType::UINT16, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "stm1", DType::UINT16, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "at", DType::UINT16, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dwell1", DType::UINT16, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dwell2", DType::UINT16, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "rgHigh", DType::INT16, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "rgLow", DType::INT16, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sHigh", DType::INT16, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sLow", DType::INT16, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "aHigh", DType::INT16, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "aLow", DType::INT16, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "rgSlew", DType::INT16, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sSlew", DType::INT16, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "aSlew", DType::INT16, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "config", DType::CHARSTR, 1, { 256 }, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_43[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_44[] = {
    { "readoutMode", DType::UINT16, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureEventCode", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "configSize", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "preFrameSweepCount", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "idleSweepCount", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "integrationTime", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nonIntegrationTime", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "batches", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixels", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lines", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "horizontalBinning", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "verticalBinning", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorPixels", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorLines", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorTaps", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "st", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "stm1", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "at", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_44[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_45[] = {
    { "readoutMode", DType::UINT16, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "power", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureEventCode", DType::UINT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad0", DType::UINT16, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "configSize", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "preFrameSweepCount", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "idleSweepCount", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "integrationTime", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nonIntegrationTime", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "batches", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixels", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lines", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "horizontalBinning", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "verticalBinning", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorPixels", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorLines", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorTaps", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "st", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "stm1", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "at", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "bias", DType::UINT16, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasChan", DType::INT16, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasVoltage", DType::FLOAT, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "configVersion", DType::UINT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_45[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "MaxModulesPerDetector", 8 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_46[] = {
    { "readoutMode", DType::UINT16, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "power", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureEventCode", DType::UINT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad0", DType::UINT16, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "configSize", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "preFrameSweepCount", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "idleSweepCount", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "preSkipLines", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "integrationTime", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nonIntegrationTime", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "batches", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixels", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lines", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "horizontalBinning", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "verticalBinning", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorPixels", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorLines", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorTaps", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "st", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "stm1", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "at", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "bias", DType::UINT16, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasChan", DType::INT16, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasVoltage", DType::FLOAT, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "configVersion", DType::UINT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_46[] = {
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "MaxModulesPerDetector", 32 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_47[] = {
    { "version", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runTrigDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "daqTrigDelay", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacSetting", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPins", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicControls", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "acqToAsicR0Delay", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0ToAsicAcq", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqWidth", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqLToPPmatL", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicRoClkHalfT", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcReadsPerPixel", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcClkHalfT", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0Width", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Width", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Delay", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId0", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId1", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId0", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId1", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lastRowExclusions", DType::UINT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerRow", DType::UINT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerColumn", DType::UINT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerAsic", DType::UINT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfPixelsPerAsicRow", DType::UINT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "baseClockFrequency", DType::UINT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicMask", DType::UINT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPixelTestArray", DType::UINT32, 3, {}, 112, true, { "numberOfAsicsPerRownumberOfAsicsPerColumn", "numberOfRowsPerAsic", "numberOfPixelsPerAsicRow3132", "", "", "", "", "", "", "" }, { 84, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "", "", "", "", "", "", "", "", "" } },
    { "asicPixelMaskArray", DType::UINT32, 3, {}, 112, true, { "numberOfAsicsPerRownumberOfAsicsPerColumn", "numberOfRowsPerAsic", "numberOfPixelsPerAsicRow3132", "", "", "", "", "", "", "" }, { 84, 4, 1, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "numberOfAsicsPerRow", "3132", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_47[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_48[] = {
    { "version", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runTrigDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "daqTrigDelay", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacSetting", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPins", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicControls", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "DoutPipelineDelay", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "acqToAsicR0Delay", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0ToAsicAcq", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqWidth", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqLToPPmatL", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicRoClkHalfT", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcReadsPerPixel", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcClkHalfT", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0Width", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Sync", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Width", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Delay", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId0", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId1", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId0", DType::UINT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId1", DType::UINT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lastRowExclusions", DType::UINT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerRow", DType::UINT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerColumn", DType::UINT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerAsic", DType::UINT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfPixelsPerAsicRow", DType::UINT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "baseClockFrequency", DType::UINT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicMask", DType::UINT32, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Scope", DType::UINT32, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_1", DType::UINT32, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_2", DType::UINT32, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeWaveformSelects", DType::UINT32, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPixelConfigArray", DType::UINT16, 3, {}, 136, true, { "numberOfAsicsPerRownumberOfAsicsPerColumn", "numberOfRowsPerAsic", "numberOfPixelsPerAsicRow", "", "", "", "", "", "", "" }, { 84, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_48[] = {
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_49[] = {
    { "version", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runTrigDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "daqTrigDelay", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacSetting", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPins", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicControls", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "acqToAsicR0Delay", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0ToAsicAcq", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqWidth", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqLToPPmatL", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPPmatToReadout", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicRoClkHalfT", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcReadsPerPixel", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcClkHalfT", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0Width", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Sync", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Width", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Delay", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId0", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId1", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId0", DType::UINT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId1", DType::UINT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerRow", DType::UINT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerColumn", DType::UINT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerAsic", DType::UINT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfReadableRowsPerAsic", DType::UINT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfPixelsPerAsicRow", DType::UINT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRowCountPerASIC", DType::UINT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "environmentalRowCountPerASIC", DType::UINT32, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "baseClockFrequency", DType::UINT32, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicMask", DType::UINT32, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Scope", DType::UINT32, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_1", DType::UINT32, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_2", DType::UINT32, 0, {}, 136, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeWaveformSelects", DType::UINT32, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPixelConfigArray", DType::UINT16, 2, {}, 144, true, { "numberOfRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 100, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "", "", "", "", "", "", "", "", "" } },
    { "calibPixelConfigArray", DType::UINT8, 2, {}, 144, true, { "numberOfCalibrationRows2", "numberOfPixelsPerAsicRownumberOfAsicsPerRow", "", "", "", "", "", "", "", "" }, { 100, 2, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "numberOfRows", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_49[] = {
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfReadableRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "calibrationRowCountPerASIC", 2 },
    { "environmentalRowCountPerASIC", 1 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "numberOfCalibrationRows2", 2 }
  };

  inline constexpr SimulatedField _fields_50[] = {
    { "version", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "usePgpEvr", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evrRunCode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evrDaqCode", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evrRunTrigDelay", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "epixRunTrigDelay", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacSetting", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPins", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicControls", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "acqToAsicR0Delay", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0ToAsicAcq", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqWidth", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqLToPPmatL", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPPmatToReadout", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicRoClkHalfT", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcReadsPerPixel", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcClkHalfT", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0Width", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay0", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay1", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay2", DType::UINT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay3", DType::UINT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Sync", DType::UINT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Width", DType::UINT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Delay", DType::UINT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId0", DType::UINT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId1", DType::UINT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId0", DType::UINT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId1", DType::UINT32, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "carrierId0", DType::UINT32, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "carrierId1", DType::UINT32, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerRow", DType::UINT32, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerColumn", DType::UINT32, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerAsic", DType::UINT32, 0, {}, 136, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfReadableRowsPerAsic", DType::UINT32, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfPixelsPerAsicRow", DType::UINT32, 0, {}, 144, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRowCountPerASIC", DType::UINT32, 0, {}, 148, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "environmentalRowCountPerASIC", DType::UINT32, 0, {}, 152, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "baseClockFrequency", DType::UINT32, 0, {}, 156, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicMask", DType::UINT32, 0, {}, 160, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "enableAutomaticRunTrigger", DType::UINT32, 0, {}, 164, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOf125MhzTicksPerRunTrigger", DType::UINT32, 0, {}, 168, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Scope", DType::UINT32, 0, {}, 172, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_1", DType::UINT32, 0, {}, 176, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_2", DType::UINT32, 0, {}, 180, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeWaveformSelects", DType::UINT32, 0, {}, 184, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPixelConfigArray", DType::UINT16, 2, {}, 188, true, { "numberOfRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 100, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "", "", "", "", "", "", "", "", "" } },
    { "calibPixelConfigArray", DType::UINT8, 2, {}, 188, true, { "numberOfCalibrationRows2", "numberOfPixelsPerAsicRownumberOfAsicsPerRow", "", "", "", "", "", "", "", "" }, { 100, 2, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "numberOfRows", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_50[] = {
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfReadableRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "calibrationRowCountPerASIC", 2 },
    { "environmentalRowCountPerASIC", 1 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "numberOfCalibrationRows2", 2 }
  };

  inline constexpr SimulatedField _fields_51[] = {
    { "version", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runTrigDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "daqTrigDelay", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacSetting", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPins", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicControls", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "acqToAsicR0Delay", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0ToAsicAcq", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqWidth", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqLToPPmatL", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPPmatToReadout", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicRoClkHalfT", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcReadsPerPixel", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcClkHalfT", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0Width", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Sync", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Width", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Delay", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId0", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId1", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId0", DType::UINT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId1", DType::UINT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "carrierId0", DType::UINT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "carrierId1", DType::UINT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerRow", DType::UINT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerColumn", DType::UINT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerAsic", DType::UINT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfReadableRowsPerAsic", DType::UINT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfPixelsPerAsicRow", DType::UINT32, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRowCountPerASIC", DType::UINT32, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "environmentalRowCountPerASIC", DType::UINT32, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "baseClockFrequency", DType::UINT32, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicMask", DType::UINT32, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Scope", DType::UINT32, 0, {}, 136, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_1", DType::UINT32, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_2", DType::UINT32, 0, {}, 144, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeWaveformSelects", DType::UINT32, 0, {}, 148, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPixelConfigArray", DType::UINT16, 2, {}, 152, true, { "numberOfRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 84, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "", "", "", "", "", "", "", "", "" } },
    { "calibPixelConfigArray", DType::UINT8, 2, {}, 152, true, { "numberOfCalibrationRows2", "numberOfPixelsPerAsicRownumberOfAsicsPerRow", "", "", "", "", "", "", "", "" }, { 84, 2, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "numberOfRows", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_51[] = {
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfReadableRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "calibrationRowCountPerASIC", 2 },
    { "environmentalRowCountPerASIC", 1 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "numberOfCalibrationRows2", 2 }
  };

  inline constexpr SimulatedField _fields_52[] = {
    { "version", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "usePgpEvr", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evrRunCode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evrDaqCode", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evrRunTrigDelay", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "epixRunTrigDelay", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacSetting", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPins", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicControls", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "acqToAsicR0Delay", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0ToAsicAcq", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqWidth", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqLToPPmatL", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPPmatToReadout", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicRoClkHalfT", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcReadsPerPixel", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcClkHalfT", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0Width", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay0", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay1", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay2", DType::UINT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay3", DType::UINT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Sync", DType::UINT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Width", DType::UINT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Delay", DType::UINT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId0", DType::UINT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId1", DType::UINT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId0", DType::UINT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId1", DType::UINT32, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "carrierId0", DType::UINT32, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "carrierId1", DType::UINT32, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerRow", DType::UINT32, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerColumn", DType::UINT32, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerAsic", DType::UINT32, 0, {}, 136, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfReadableRowsPerAsic", DType::UINT32, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfPixelsPerAsicRow", DType::UINT32, 0, {}, 144, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRowCountPerASIC", DType::UINT32, 0, {}, 148, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "environmentalRowCountPerASIC", DType::UINT32, 0, {}, 152, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "baseClockFrequency", DType::UINT32, 0, {}, 156, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicMask", DType::UINT32, 0, {}, 160, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "enableAutomaticRunTrigger", DType::UINT32, 0, {}, 164, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOf125MhzTicksPerRunTrigger", DType::UINT32, 0, {}, 168, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Scope", DType::UINT32, 0, {}, 172, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_1", DType::UINT32, 0, {}, 176, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_2", DType::UINT32, 0, {}, 180, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeWaveformSelects", DType::UINT32, 0, {}, 184, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPixelConfigArray", DType::UINT16, 2, {}, 188, true, { "numberOfRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 104, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "", "", "", "", "", "", "", "", "" } },
    { "calibPixelConfigArray", DType::UINT8, 2, {}, 188, true, { "numberOfCalibrationRows2", "numberOfPixelsPerAsicRownumberOfAsicsPerRow", "", "", "", "", "", "", "", "" }, { 104, 2, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "numberOfRows", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_52[] = {
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfReadableRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "calibrationRowCountPerASIC", 2 },
    { "environmentalRowCountPerASIC", 1 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "numberOfCalibrationRows2", 2 }
  };

  inline constexpr SimulatedField _fields_53[] = {
    { "version", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "usePgpEvr", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evrRunCode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evrDaqCode", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evrRunTrigDelay", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "epixRunTrigDelay", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "epixDaqTrigDelay", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacSetting", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPins", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicControls", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "acqToAsicR0Delay", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0ToAsicAcq", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqWidth", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicAcqLToPPmatL", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPPmatToReadout", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicRoClkHalfT", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcClkHalfT", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicR0Width", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay0", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay1", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay2", DType::UINT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay3", DType::UINT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Sync", DType::UINT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Width", DType::UINT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "prepulseR0Delay", DType::UINT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId0", DType::UINT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId1", DType::UINT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId0", DType::UINT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId1", DType::UINT32, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "carrierId0", DType::UINT32, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "carrierId1", DType::UINT32, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerRow", DType::UINT32, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfAsicsPerColumn", DType::UINT32, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerAsic", DType::UINT32, 0, {}, 136, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfReadableRowsPerAsic", DType::UINT32, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfPixelsPerAsicRow", DType::UINT32, 0, {}, 144, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRowCountPerASIC", DType::UINT32, 0, {}, 148, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "environmentalRowCountPerASIC", DType::UINT32, 0, {}, 152, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "baseClockFrequency", DType::UINT32, 0, {}, 156, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicMask", DType::UINT32, 0, {}, 160, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "enableAutomaticRunTrigger", DType::UINT32, 0, {}, 164, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOf125MhzTicksPerRunTrigger", DType::UINT32, 0, {}, 168, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ghostCorrEn", DType::UINT32, 0, {}, 172, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "oversampleEn", DType::UINT32, 0, {}, 176, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "oversampleSize", DType::UINT32, 0, {}, 180, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Scope", DType::UINT32, 0, {}, 184, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_1", DType::UINT32, 0, {}, 188, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeTriggerParms_2", DType::UINT32, 0, {}, 192, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ScopeWaveformSelects", DType::UINT32, 0, {}, 196, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "firmwareHash", DType::CHARSTR, 1, { 64 }, 200, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "firmwareDesc", DType::CHARSTR, 1, { 256 }, 264, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "asicPixelConfigArray", DType::UINT16, 2, {}, 520, true, { "numberOfRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 104, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "", "", "", "", "", "", "", "", "" } },
    { "calibPixelConfigArray", DType::UINT8, 2, {}, 520, true, { "numberOfCalibrationRows2", "numberOfPixelsPerAsicRownumberOfAsicsPerRow", "", "", "", "", "", "", "", "" }, { 104, 2, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfAsicsPerRow", "numberOfRows", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_53[] = {
    { "FirmwareHashMax", 64 },
    { "FirmwareDescMax", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfReadableRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "calibrationRowCountPerASIC", 2 },
    { "environmentalRowCountPerASIC", 1 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "numberOfCalibrationRows2", 2 }
  };

  inline constexpr SimulatedField _fields_54[] = {
    { "evr", DType::ENUMVAL, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quad", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "elem", DType::ENUMVAL, 1, { 4 }, 820, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_54[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "numberOfElements", 4 }
  };

  inline constexpr SimulatedField _fields_55[] = {
    { "evr", DType::ENUMVAL, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quad", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "elem", DType::ENUMVAL, 1, { 4 }, 1164, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_55[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "numberOfElements", 4 }
  };

  inline constexpr SimulatedField _fields_56[] = {
    { "evr", DType::ENUMVAL, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quad", DType::ENUMVAL, 1, { 4 }, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "elem", DType::ENUMVAL, 1, { 16 }, 3256, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_56[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "numberOfElements", 16 }
  };

  inline constexpr SimulatedField _fields_57[] = {
    { "evr", DType::ENUMVAL, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quad", DType::ENUMVAL, 1, { 4 }, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "elem", DType::ENUMVAL, 1, { 16 }, 4632, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_57[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "numberOfElements", 16 }
  };

  inline constexpr SimulatedField _fields_58[] = {
    { "first", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "second", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frameNumber", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fiducials", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z0", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z1", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z2", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame", DType::UINT16, 2, {}, 32, true, { "numberOfRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "excludedRows", DType::UINT16, 2, {}, 32, true, { "lastRowExclusions", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfRows", "", "", "", "", "", "", "", "", "" } },
    { "temperatures", DType::UINT16, 1, {}, 32, true, { "numberOfAsics", "", "", "", "", "", "", "", "", "" }, { 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfRows", "lastRowExclusions", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_58[] = {
    { "Nsbtemp", 4 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 }
  };

  inline constexpr SimulatedField _fields_59[] = {
    { "first", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "second", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frameNumber", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fiducials", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z0", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z1", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z2", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame", DType::UINT16, 2, {}, 32, true, { "numberOfReadableRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRows", DType::UINT16, 2, {}, 32, true, { "numberOfCalibrationRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfReadableRows", "", "", "", "", "", "", "", "", "" } },
    { "environmentalRows", DType::UINT16, 2, {}, 32, true, { "numberOfEnvironmentalRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfReadableRows", "numberOfCalibrationRows", "", "", "", "", "", "", "", "" } },
    { "temperatures", DType::UINT16, 1, {}, 32, true, { "numberOfAsics", "", "", "", "", "", "", "", "", "" }, { 2, 2, 2, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfReadableRows", "numberOfCalibrationRows", "numberOfEnvironmentalRows", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_59[] = {
    { "Nsbtemp", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_60[] = {
    { "first", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "second", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frameNumber", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fiducials", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z0", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z1", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z2", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame", DType::UINT16, 2, {}, 32, true, { "numberOfReadableRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRows", DType::UINT16, 2, {}, 32, true, { "numberOfCalibrationRows", "numberOfColumns", "", "", "", "", "", "", "", "" }, { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfReadableRows", "", "", "", "", "", "", "", "", "" } },
    { "environmentalRows", DType::UINT32, 2, {}, 32, true, { "numberOfEnvironmentalRows", "numberOfColumns1", "", "", "", "", "", "", "", "" }, { 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfReadableRows", "numberOfCalibrationRows", "", "", "", "", "", "", "", "" } },
    { "temperatures", DType::UINT16, 1, {}, 32, true, { "numberOfAsics", "", "", "", "", "", "", "", "", "" }, { 2, 2, 4, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfReadableRows", "numberOfCalibrationRows", "numberOfEnvironmentalRows", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_60[] = {
    { "numberOfColumns1", 192 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_61[] = {
    { "frameNumber", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame", DType::UINT16, 3, {}, 4, true, { "numberOfElements", "numberOfReadableRows", "numberOfColumns", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRows", DType::UINT16, 3, {}, 4, true, { "numberOfElements", "numberOfCalibrationRows", "numberOfColumns", "", "", "", "", "", "", "" }, { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfElements", "", "", "", "", "", "", "", "", "" } },
    { "environmentalRows", DType::UINT32, 3, {}, 4, true, { "numberOfElements", "numberOfEnvironmentalRows", "numberOfColumns1", "", "", "", "", "", "", "" }, { 2, 2, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfElements", "numberOfElements", "", "", "", "", "", "", "", "" } },
    { "temperatures", DType::UINT16, 1, {}, 4, true, { "numberOfAsics", "", "", "", "", "", "", "", "", "" }, { 2, 2, 4, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfElements", "numberOfElements", "numberOfElements", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_61[] = {
    { "numberOfColumns1", 192 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_62[] = {
    { "control", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "reserved", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "duration", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvControls", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvMonitors", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pvControls", DType::ENUMVAL, 1, {}, 24, true, { "npvControls", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pvMonitors", DType::ENUMVAL, 1, {}, 24, true, { "npvMonitors", "", "", "", "", "", "", "", "", "" }, { 44, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "npvControls", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_62[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_63[] = {
    { "control", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "reserved", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "duration", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvControls", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvMonitors", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvLabels", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pvControls", DType::ENUMVAL, 1, {}, 28, true, { "npvControls", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pvMonitors", DType::ENUMVAL, 1, {}, 28, true, { "npvMonitors", "", "", "", "", "", "", "", "", "" }, { 44, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "npvControls", "", "", "", "", "", "", "", "", "" } },
    { "pvLabels", DType::ENUMVAL, 1, {}, 28, true, { "npvLabels", "", "", "", "", "", "", "", "", "" }, { 44, 52, 0, 0, 0, 0, 0, 0, 0, 0 }, { "npvControls", "npvMonitors", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_63[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_64[] = {
    { "control", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "reserved", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "duration", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvControls", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvMonitors", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvLabels", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pvControls", DType::ENUMVAL, 1, {}, 28, true, { "npvControls", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pvMonitors", DType::ENUMVAL, 1, {}, 28, true, { "npvMonitors", "", "", "", "", "", "", "", "", "" }, { 44, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "npvControls", "", "", "", "", "", "", "", "", "" } },
    { "pvLabels", DType::ENUMVAL, 1, {}, 28, true, { "npvLabels", "", "", "", "", "", "", "", "", "" }, { 44, 52, 0, 0, 0, 0, 0, 0, 0, 0 }, { "npvControls", "npvMonitors", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_64[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "MaxModulesPerDetector", 8 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_65[] = {
    { "control", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "reserved", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "duration", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvControls", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvMonitors", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npvLabels", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pvControls", DType::ENUMVAL, 1, {}, 28, true, { "npvControls", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pvMonitors", DType::ENUMVAL, 1, {}, 28, true, { "npvMonitors", "", "", "", "", "", "", "", "", "" }, { 140, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "npvControls", "", "", "", "", "", "", "", "", "" } },
    { "pvLabels", DType::ENUMVAL, 1, {}, 28, true, { "npvLabels", "", "", "", "", "", "", "", "", "" }, { 140, 148, 0, 0, 0, 0, 0, 0, 0, 0 }, { "npvControls", "npvMonitors", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_65[] = {
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "MaxModulesPerDetector", 32 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_66[] = {
    { "width", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "orgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "orgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "binX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "binY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "coolingTemp", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "readoutSpeed", DType::FLOAT, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gainMode", DType::UINT16, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcMode", DType::UINT16, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerMode", DType::UINT16, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad0", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeWidth", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeHeight", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeTopMargin", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeBottomMargin", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeLeftMargin", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeRightMargin", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "cleanCycleCount", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "cleanCycleHeight", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "cleanFinalHeight", DType::UINT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "cleanFinalHeightCount", DType::UINT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "maskedHeight", DType::UINT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "kineticHeight", DType::UINT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vsSpeed", DType::FLOAT, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "infoReportInterval", DType::INT16, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureEventCode", DType::UINT16, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numIntegrationShots", DType::UINT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_66[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_67[] = {
    { "iShotIdStart", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fReadoutTime", DType::FLOAT, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fTemperature", DType::FLOAT, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 12, true, { "numPixelsY", "numPixelsX", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_68[] = {
    { "version", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runTrigDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "daqTrigDelay", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "daqSetting", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcClkHalfT", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcPipelineDelay", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId0", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "digitalCardId1", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId0", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analogCardId1", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfChannels", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "samplesPerChannel", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "baseClockFrequency", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "bitControls", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_68[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_69[] = {
    { "first", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "second", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frameNumber", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fiducials", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z0", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z1", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z2", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame", DType::UINT16, 2, {}, 32, true, { "numberOfChannels", "samplesPerChannel", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "temperatures", DType::UINT16, 1, {}, 32, true, { "numberOfChannels", "", "", "", "", "", "", "", "", "" }, { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfChannels", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_69[] = {
    { "Nsbtemp", 4 }
  };

  inline constexpr SimulatedField _fields_70[] = {
    { "npulses", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noutputs", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pulses", DType::ENUMVAL, 1, {}, 8, true, { "_npulses", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "output_maps", DType::ENUMVAL, 1, {}, 8, true, { "_noutputs", "", "", "", "", "", "", "", "", "" }, { 24, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_npulses", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_70[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_71[] = {
    { "opcode", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npulses", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noutputs", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pulses", DType::ENUMVAL, 1, {}, 12, true, { "_npulses", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "output_maps", DType::ENUMVAL, 1, {}, 12, true, { "_noutputs", "", "", "", "", "", "", "", "", "" }, { 24, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_npulses", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_71[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_72[] = {
    { "neventcodes", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npulses", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noutputs", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventcodes", DType::ENUMVAL, 1, {}, 12, true, { "_neventcodes", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pulses", DType::ENUMVAL, 1, {}, 12, true, { "_npulses", "", "", "", "", "", "", "", "", "" }, { 16, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "", "", "", "", "", "", "", "", "" } },
    { "output_maps", DType::ENUMVAL, 1, {}, 12, true, { "_noutputs", "", "", "", "", "", "", "", "", "" }, { 16, 16, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "_npulses", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_72[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "MaxModulesPerDetector", 8 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_73[] = {
    { "neventcodes", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npulses", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noutputs", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventcodes", DType::ENUMVAL, 1, {}, 12, true, { "_neventcodes", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pulses", DType::ENUMVAL, 1, {}, 12, true, { "_npulses", "", "", "", "", "", "", "", "", "" }, { 24, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "", "", "", "", "", "", "", "", "" } },
    { "output_maps", DType::ENUMVAL, 1, {}, 12, true, { "_noutputs", "", "", "", "", "", "", "", "", "" }, { 24, 16, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "_npulses", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_73[] = {
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "MaxModulesPerDetector", 32 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_74[] = {
    { "neventcodes", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npulses", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noutputs", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventcodes", DType::ENUMVAL, 1, {}, 12, true, { "_neventcodes", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pulses", DType::ENUMVAL, 1, {}, 12, true, { "_npulses", "", "", "", "", "", "", "", "", "" }, { 40, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "", "", "", "", "", "", "", "", "" } },
    { "output_maps", DType::ENUMVAL, 1, {}, 12, true, { "_noutputs", "", "", "", "", "", "", "", "", "" }, { 40, 16, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "_npulses", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_74[] = {
    { "MaxPulses", 32 },
    { "EvrOutputs", 10 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_75[] = {
    { "neventcodes", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npulses", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noutputs", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventcodes", DType::ENUMVAL, 1, {}, 12, true, { "_neventcodes", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pulses", DType::ENUMVAL, 1, {}, 12, true, { "_npulses", "", "", "", "", "", "", "", "", "" }, { 40, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "", "", "", "", "", "", "", "", "" } },
    { "output_maps", DType::ENUMVAL, 1, {}, 12, true, { "_noutputs", "", "", "", "", "", "", "", "", "" }, { 40, 16, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "_npulses", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_75[] = {
    { "MaxPulses", 256 },
    { "MaxOutputs", 256 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_76[] = {
    { "neventcodes", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npulses", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noutputs", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventcodes", DType::ENUMVAL, 1, {}, 12, true, { "_neventcodes", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pulses", DType::ENUMVAL, 1, {}, 12, true, { "_npulses", "", "", "", "", "", "", "", "", "" }, { 44, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "", "", "", "", "", "", "", "", "" } },
    { "output_maps", DType::ENUMVAL, 1, {}, 12, true, { "_noutputs", "", "", "", "", "", "", "", "", "" }, { 44, 16, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "_npulses", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_76[] = {
    { "MaxPulses", 256 },
    { "MaxOutputs", 256 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_77[] = {
    { "neventcodes", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "npulses", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "noutputs", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventcodes", DType::ENUMVAL, 1, {}, 12, true, { "_neventcodes", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pulses", DType::ENUMVAL, 1, {}, 12, true, { "_npulses", "", "", "", "", "", "", "", "", "" }, { 36, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "", "", "", "", "", "", "", "", "" } },
    { "output_maps", DType::ENUMVAL, 1, {}, 12, true, { "_noutputs", "", "", "", "", "", "", "", "", "" }, { 36, 16, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_neventcodes", "_npulses", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_77[] = {
    { "MaxPulses", 12 },
    { "MaxOutputs", 12 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_78[] = {
    { "u32NumFifoEvents", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fifoEvents", DType::ENUMVAL, 1, {}, 4, true, { "_u32NumFifoEvents", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_78[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 }
  };

  inline constexpr SimulatedField _fields_79[] = {
    { "u32NumFifoEvents", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fifoEvents", DType::ENUMVAL, 1, {}, 4, true, { "_u32NumFifoEvents", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_80[] = {
    { "conn", DType::UINT16, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nchannels", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channels", DType::ENUMVAL, 1, {}, 4, true, { "_nchannels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_80[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_81[] = {
    { "nchannels", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channels", DType::ENUMVAL, 1, {}, 4, true, { "_nchannels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_81[] = {
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_82[] = {
    { "nbrConvertersPerChannel", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channelMask", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nbrBanks", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "trig", DType::ENUMVAL, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "horiz", DType::ENUMVAL, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vert", DType::ENUMVAL, 1, { 20 }, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vert", DType::ENUMVAL, 1, {}, 0, true, { "MaxChan", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_82[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_84[] = {
    { "channel", DType::ENUMVAL, 1, { 8 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "auxIO", DType::ENUMVAL, 1, { 2 }, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "veto", DType::ENUMVAL, 0, {}, 152, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channels", DType::ENUMVAL, 1, {}, 0, true, { "NChannels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "auxio", DType::ENUMVAL, 1, {}, 0, true, { "NAuxIO", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_84[] = {
    { "NChannels", 8 },
    { "NAuxIO", 2 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_85[] = {
    { "data", DType::ENUMVAL, 1, {}, 0, true, { "dim0", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_86[] = {
    { "bldMask", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numSources", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sources", DType::ENUMVAL, 1, {}, 12, true, { "numSources", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_86[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_87[] = {
    { "numWords", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numSources", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "bldMask", DType::UINT32, 1, {}, 8, true, { "numWords", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sources", DType::ENUMVAL, 1, {}, 8, true, { "numSources", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numWords", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_87[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_88[] = {
    { "f32ExposureTime", DType::FLOAT, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lfWaveLenCalibCoeff", DType::DOUBLE, 1, { 4 }, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lfNonlinCorrectCoeff", DType::DOUBLE, 1, { 8 }, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fStrayLightConstant", DType::DOUBLE, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "waveLenCalib", DType::DOUBLE, 1, {}, 0, true, { "4", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nonlinCorrect", DType::DOUBLE, 1, {}, 0, true, { "8", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_88[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_89[] = {
    { "f32ExposureTime", DType::FLOAT, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "iDeviceType", DType::INT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lfWaveLenCalibCoeff", DType::DOUBLE, 1, { 4 }, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lfNonlinCorrectCoeff", DType::DOUBLE, 1, { 8 }, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fStrayLightConstant", DType::DOUBLE, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "waveLenCalib", DType::DOUBLE, 1, {}, 0, true, { "4", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nonlinCorrect", DType::DOUBLE, 1, {}, 0, true, { "8", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_89[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_90[] = {
    { "u64FrameCounter", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u64NumDelayedFrames", DType::UINT64, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u64NumDiscardFrames", DType::UINT64, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "tsTimeFrameStart", DType::ENUMVAL, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "tsTimeFrameFirstData", DType::ENUMVAL, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "tsTimeFrameEnd", DType::ENUMVAL, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i32Version", DType::INT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i8NumSpectraInData", DType::INT8, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i8NumSpectraInQueue", DType::INT8, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i8NumSpectraUnused", DType::INT8, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "iReserved1", DType::INT8, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 1, {}, 0, true, { "iNumPixels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_90[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };

  inline constexpr SimulatedField _fields_91[] = {
    { "u64FrameCounter", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u64NumDelayedFrames", DType::UINT64, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u64NumDiscardFrames", DType::UINT64, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "tsTimeFrameStart", DType::ENUMVAL, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "tsTimeFrameFirstData", DType::ENUMVAL, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "tsTimeFrameEnd", DType::ENUMVAL, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i32Version", DType::INT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i8NumSpectraInData", DType::INT8, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i8NumSpectraInQueue", DType::INT8, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i8NumSpectraUnused", DType::INT8, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "iReserved1", DType::INT8, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 1, {}, 0, true, { "iNumPixels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_91[] = {
    { "iDataReadSize", 4608 },
    { "iNumPixels", 2048 },
    { "iActivePixelIndex", 0 },
    { "NEncoders", 3 },
    { "Depth", 14 },
    { "MaxPixelValue", 11810 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "ipimbAdcRange", 5 },
    { "ipimbAdcSteps", 65536 }
  };

  inline constexpr SimulatedField _fields_92[] = {
    { "u64FrameCounter", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u64NumDelayedFrames", DType::UINT64, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u64NumDiscardFrames", DType::UINT64, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "tsTimeFrameStart", DType::ENUMVAL, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "tsTimeFrameFirstData", DType::ENUMVAL, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "tsTimeFrameEnd", DType::ENUMVAL, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i32Version", DType::INT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i8NumSpectraInData", DType::INT8, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i8NumSpectraInQueue", DType::INT8, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i8NumSpectraUnused", DType::INT8, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "iReserved1", DType::INT8, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 1, {}, 0, true, { "iNumPixels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_92[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 }
  };

  inline constexpr SimulatedField _fields_93[] = {
    { "concentratorVersion", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventCode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "inactiveRunMode", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeRunMode", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "testDataIndex", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payloadPerQuad", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask0", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask1", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "AsicMask", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quadMask", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quads", DType::ENUMVAL, 1, { 4 }, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_93[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_94[] = {
    { "concentratorVersion", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventCode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "inactiveRunMode", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeRunMode", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "testDataIndex", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payloadPerQuad", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask0", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask1", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "AsicMask", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quadMask", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiMask", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quads", DType::ENUMVAL, 1, { 4 }, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_94[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_95[] = {
    { "concentratorVersion", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventCode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionThresholds", DType::ENUMVAL, 1, { 4 }, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionEnable", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "inactiveRunMode", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeRunMode", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "testDataIndex", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payloadPerQuad", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask0", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask1", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "AsicMask", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quadMask", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiMask", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quads", DType::ENUMVAL, 1, { 4 }, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionThresholds", DType::ENUMVAL, 1, {}, 0, true, { "MaxQuadsPerSensor", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_95[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "MaxModulesPerDetector", 8 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "MaxQuadsPerSensor", 4 }
  };

  inline constexpr SimulatedField _fields_96[] = {
    { "concentratorVersion", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventCode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionThresholds", DType::ENUMVAL, 1, { 4 }, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionEnable", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "inactiveRunMode", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeRunMode", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "testDataIndex", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payloadPerQuad", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask0", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask1", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "AsicMask", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quadMask", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiMask", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quads", DType::ENUMVAL, 1, { 4 }, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionThresholds", DType::ENUMVAL, 1, {}, 0, true, { "MaxQuadsPerSensor", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_96[] = {
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "MaxModulesPerDetector", 32 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "MaxQuadsPerSensor", 4 }
  };

  inline constexpr SimulatedField _fields_97[] = {
    { "concentratorVersion", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runDelay", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "eventCode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionThresholds", DType::ENUMVAL, 1, { 4 }, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionEnable", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "inactiveRunMode", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeRunMode", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "internalTriggerDelay", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "testDataIndex", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payloadPerQuad", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask0", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask1", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "AsicMask", DType::UINT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quadMask", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiMask", DType::UINT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quads", DType::ENUMVAL, 1, { 4 }, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionThresholds", DType::ENUMVAL, 1, {}, 0, true, { "MaxQuadsPerSensor", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_97[] = {
    { "MaxPulses", 32 },
    { "EvrOutputs", 10 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 },
    { "MaxQuadsPerSensor", 4 }
  };

  inline constexpr SchemaConstant _consts_98[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };

  inline constexpr SchemaConstant _consts_99[] = {
    { "iDataReadSize", 4608 },
    { "iNumPixels", 2048 },
    { "iActivePixelIndex", 0 },
    { "NEncoders", 3 },
    { "Depth", 14 },
    { "MaxPixelValue", 11810 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "ipimbAdcRange", 5 },
    { "ipimbAdcSteps", 65536 }
  };

  inline constexpr SimulatedField _fields_100[] = {
    { "module_id_len", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "desc_len", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_100[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_101[] = {
    { "accept", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_101[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };

  inline constexpr SimulatedField _fields_102[] = {
    { "accept", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_102[] = {
    { "iDataReadSize", 4608 },
    { "iNumPixels", 2048 },
    { "iActivePixelIndex", 0 },
    { "NEncoders", 3 },
    { "Depth", 14 },
    { "MaxPixelValue", 11810 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "ipimbAdcRange", 5 },
    { "ipimbAdcSteps", 65536 }
  };

  inline constexpr SimulatedField _fields_103[] = {
    { "concentratorVersion", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionThreshold", DType::ENUMVAL, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionEnable", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "inactiveRunMode", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeRunMode", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "testDataIndex", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payloadPerQuad", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "AsicMask", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiMask", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quad", DType::ENUMVAL, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_103[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_104[] = {
    { "concentratorVersion", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionThreshold", DType::ENUMVAL, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "protectionEnable", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "inactiveRunMode", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "activeRunMode", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "runTriggerDelay", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "testDataIndex", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payloadPerQuad", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "badAsicMask", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "AsicMask", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiMask", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quad", DType::ENUMVAL, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_104[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_105[] = {
    { "word0", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "word1", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "seq_count", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fiducials", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sbtemp", DType::UINT16, 1, { 4 }, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame_type", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_temp", DType::UINT16, 1, {}, 0, true, { "Nsbtemp", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::INT16, 3, {}, 32, true, { "ColumnsPerASIC", "MaxRowsPerASIC2", "2", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_105[] = {
    { "Nsbtemp", 4 },
    { "MaxRowsPerASIC2", 388 },
    { "ColumnsPerASIC", 185 }
  };

  inline constexpr SimulatedField _fields_106[] = {
    { "u16OutputMode", DType::UINT16, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_106[] = {
    { "Row_Pixels", 500 },
    { "Column_Pixels", 576 },
    { "Trimmed_Row_Pixels", 480 },
    { "Trimmed_Column_Pixels", 480 },
    { "width", 2048 },
    { "height", 2048 },
    { "trimmedWidth", 480 },
    { "trimmedHeight", 480 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_107[] = {
    { "outputMode", DType::UINT16, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ccdEnable", DType::UINT8, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "focusMode", DType::UINT8, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacVoltage", DType::FLOAT, 1, { 17 }, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "waveform", DType::UINT16, 1, { 15 }, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacVoltages", DType::FLOAT, 1, {}, 0, true, { "NVoltages", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "waveforms", DType::UINT16, 1, {}, 0, true, { "NWaveforms", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_107[] = {
    { "width", 2048 },
    { "height", 2048 },
    { "trimmedWidth", 480 },
    { "trimmedHeight", 480 },
    { "Row_Pixels", 500 },
    { "Column_Pixels", 1152 },
    { "Trimmed_Row_Pixels", 480 },
    { "Trimmed_Column_Pixels", 480 },
    { "NVoltages", 17 },
    { "NWaveforms", 15 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_108[] = {
    { "iNumPv", DType::INT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "getPvConfig", DType::ENUMVAL, 1, {}, 4, true, { "numPv", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_108[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_109[] = {
    { "voltageRange", DType::UINT16, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "firstChan", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lastChan", DType::UINT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "inputMode", DType::UINT16, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerMode", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dataFormat", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fps", DType::UINT16, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "autocalibEnable", DType::UINT8, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timeTagEnable", DType::UINT8, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_109[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_110[] = {
    { "timestamp", DType::UINT16, 1, { 3 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timestamp", DType::UINT16, 1, {}, 0, true, { "3", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channelValue", DType::UINT16, 1, {}, 6, true, { "numChannels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_110[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };

  inline constexpr SimulatedField _fields_111[] = {
    { "offsetAndGain", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "outputOptions", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "defectPixelCount", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "output_lookup_table", DType::UINT16, 1, {}, 12, true, { "Output_LUT_Sizeoutput_lookup_table_enabled", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "defect_pixel_coordinates", DType::ENUMVAL, 1, {}, 12, true, { "number_of_defect_pixels", "", "", "", "", "", "", "", "", "" }, { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "Output_LUT_Size", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_111[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_112[] = {
    { "offsetAndGain", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "outputOptions", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roi_lo", DType::ENUMVAL, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roi_hi", DType::ENUMVAL, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "defectPixelCount", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "output_lookup_table", DType::UINT16, 1, {}, 20, true, { "Output_LUT_Sizeoutput_lookup_table_enabled", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "defect_pixel_coordinates", DType::ENUMVAL, 1, {}, 20, true, { "number_of_defect_pixels", "", "", "", "", "", "", "", "", "" }, { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "Output_LUT_Size", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_112[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_113[] = {
    { "reverseX", DType::UINT8, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "reverseY", DType::UINT8, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "contrastEnable", DType::UINT8, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "correctionEnable", DType::UINT8, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "roiEnable", DType::UINT8, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad0", DType::UINT8, 1, { 3 }, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "correctionType", DType::UINT8, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "correctionSet", DType::UINT8, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixelMode", DType::UINT8, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerMode", DType::UINT8, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "width", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "offsetX", DType::UINT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "offsetY", DType::UINT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorWidth", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sensorHeight", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "contrastDarkLimit", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "contrastBrightLimit", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "contrastShape", DType::UINT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::DOUBLE, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "blackLevel", DType::DOUBLE, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gain", DType::DOUBLE, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gamma", DType::DOUBLE, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "manufacturer", DType::CHARSTR, 1, { 48 }, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "family", DType::CHARSTR, 1, { 48 }, 156, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "model", DType::CHARSTR, 1, { 48 }, 204, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "manufacturerId", DType::CHARSTR, 1, { 48 }, 252, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "version", DType::CHARSTR, 1, { 48 }, 300, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "serialNumber", DType::CHARSTR, 1, { 48 }, 348, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "firmwareId", DType::CHARSTR, 1, { 48 }, 396, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "firmwareVersion", DType::CHARSTR, 1, { 48 }, 444, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_113[] = {
    { "DESC_CHAR_MAX", 48 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_114[] = {
    { "frameid", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timestamp", DType::UINT64, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 16, true, { "numPixelsY", "numPixelsX", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_115[] = {
    { "iNumChars", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT8, 1, {}, 8, true, { "_iNumChars", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_115[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };

  inline constexpr SimulatedField _fields_116[] = {
    { "chan_num", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "count_mode", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quadrature_mode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "input_num", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "input_rising", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks_per_sec", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_116[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_117[] = {
    { "chan_mask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "count_mode", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quadrature_mode", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "input_num", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "input_rising", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks_per_sec", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_117[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_118[] = {
    { "33mhz_timestamp", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "encoder_count", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_118[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };

  inline constexpr SimulatedField _fields_119[] = {
    { "33mhz_timestamp", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "encoder_count", DType::UINT32, 1, { 3 }, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "encoder_count", DType::UINT32, 1, {}, 0, true, { "NEncoders", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_119[] = {
    { "iDataReadSize", 4608 },
    { "iNumPixels", 2048 },
    { "iActivePixelIndex", 0 },
    { "NEncoders", 3 },
    { "Depth", 14 },
    { "MaxPixelValue", 11810 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "ipimbAdcRange", 5 },
    { "ipimbAdcSteps", 65536 }
  };

  inline constexpr SimulatedField _fields_120[] = {
    { "Control", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "number_of_beam_event_codes", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "number_of_laser_event_codes", DType::UINT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "signal_cut", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sig_roi_lo", DType::ENUMVAL, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sig_roi_hi", DType::ENUMVAL, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_roi_lo", DType::ENUMVAL, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_roi_hi", DType::ENUMVAL, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_convergence", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_convergence", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "beam_logic", DType::ENUMVAL, 1, {}, 44, true, { "number_of_beam_event_codes", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "laser_logic", DType::ENUMVAL, 1, {}, 44, true, { "number_of_laser_event_codes", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "", "", "", "", "", "", "", "", "" } },
    { "weights", DType::DOUBLE, 1, {}, 44, true, { "number_of_weights", "", "", "", "", "", "", "", "", "" }, { 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "number_of_laser_event_codes", "", "", "", "", "", "", "", "" } },
    { "calib_poly", DType::DOUBLE, 1, {}, 44, true, { "calib_poly_dim", "", "", "", "", "", "", "", "", "" }, { 4, 4, 8, 0, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "number_of_laser_event_codes", "number_of_weights", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_120[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_121[] = {
    { "Control", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "number_of_beam_event_codes", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "number_of_laser_event_codes", DType::UINT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "signal_cut", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sig_roi_lo", DType::ENUMVAL, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sig_roi_hi", DType::ENUMVAL, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_roi_lo", DType::ENUMVAL, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_roi_hi", DType::ENUMVAL, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_convergence", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_roi_lo", DType::ENUMVAL, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_roi_hi", DType::ENUMVAL, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_convergence", DType::DOUBLE, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "beam_logic", DType::ENUMVAL, 1, {}, 52, true, { "number_of_beam_event_codes", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "laser_logic", DType::ENUMVAL, 1, {}, 52, true, { "number_of_laser_event_codes", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "", "", "", "", "", "", "", "", "" } },
    { "weights", DType::DOUBLE, 1, {}, 52, true, { "number_of_weights", "", "", "", "", "", "", "", "", "" }, { 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "number_of_laser_event_codes", "", "", "", "", "", "", "", "" } },
    { "calib_poly", DType::DOUBLE, 1, {}, 52, true, { "calib_poly_dim", "", "", "", "", "", "", "", "", "" }, { 4, 4, 8, 0, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "number_of_laser_event_codes", "number_of_weights", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_121[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_122[] = {
    { "Control", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "number_of_beam_event_codes", DType::UINT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "number_of_laser_event_codes", DType::UINT16, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "signal_cut", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fit_max_iterations", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fit_weights_factor", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sig_roi_lo", DType::ENUMVAL, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sig_roi_hi", DType::ENUMVAL, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_roi_lo", DType::ENUMVAL, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_roi_hi", DType::ENUMVAL, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sb_convergence", DType::DOUBLE, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_roi_lo", DType::ENUMVAL, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_roi_hi", DType::ENUMVAL, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_convergence", DType::DOUBLE, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "beam_logic", DType::ENUMVAL, 1, {}, 68, true, { "number_of_beam_event_codes", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "laser_logic", DType::ENUMVAL, 1, {}, 68, true, { "number_of_laser_event_codes", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "", "", "", "", "", "", "", "", "" } },
    { "weights", DType::DOUBLE, 1, {}, 68, true, { "number_of_weights", "", "", "", "", "", "", "", "", "" }, { 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "number_of_laser_event_codes", "", "", "", "", "", "", "", "" } },
    { "calib_poly", DType::DOUBLE, 1, {}, 68, true, { "calib_poly_dim", "", "", "", "", "", "", "", "", "" }, { 4, 4, 8, 0, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "number_of_laser_event_codes", "number_of_weights", "", "", "", "", "", "", "" } },
    { "fit_params", DType::DOUBLE, 1, {}, 68, true, { "fit_params_dim", "", "", "", "", "", "", "", "", "" }, { 4, 4, 8, 8, 0, 0, 0, 0, 0, 0 }, { "number_of_beam_event_codes", "number_of_laser_event_codes", "number_of_weights", "calib_poly_dim", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_122[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "MaxModulesPerDetector", 8 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_123[] = {
    { "event_type", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "amplitude", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "position_pixel", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "position_time", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "position_fwhm", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_amplitude", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nxt_amplitude", DType::DOUBLE, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "projected_signal", DType::INT32, 1, {}, 56, true, { "signal_projection_size", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "projected_sideband", DType::INT32, 1, {}, 56, true, { "sideband_projection_size", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "signal_projection_size", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_123[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };

  inline constexpr SimulatedField _fields_124[] = {
    { "event_type", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "amplitude", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "position_pixel", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "position_time", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "position_fwhm", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_amplitude", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nxt_amplitude", DType::DOUBLE, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "projected_signal", DType::INT32, 1, {}, 56, true, { "signal_projection_size", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "projected_sideband", DType::INT32, 1, {}, 56, true, { "sideband_projection_size", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "signal_projection_size", "", "", "", "", "", "", "", "", "" } },
    { "projected_reference", DType::INT32, 1, {}, 56, true, { "reference_projection_size", "", "", "", "", "", "", "", "", "" }, { 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, { "signal_projection_size", "sideband_projection_size", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_124[] = {
    { "iDataReadSize", 4608 },
    { "iNumPixels", 2048 },
    { "iActivePixelIndex", 0 },
    { "NEncoders", 3 },
    { "Depth", 14 },
    { "MaxPixelValue", 11810 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "ipimbAdcRange", 5 },
    { "ipimbAdcSteps", 65536 }
  };

  inline constexpr SimulatedField _fields_125[] = {
    { "event_type", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "z", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "amplitude", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "position_pixel", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "position_time", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "position_fwhm", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ref_amplitude", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nxt_amplitude", DType::DOUBLE, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "projected_signal", DType::INT32, 1, {}, 56, true, { "signal_projection_size", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "projected_sideband", DType::INT32, 1, {}, 56, true, { "sideband_projection_size", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "signal_projection_size", "", "", "", "", "", "", "", "", "" } },
    { "projected_reference", DType::INT32, 1, {}, 56, true, { "reference_projection_size", "", "", "", "", "", "", "", "", "" }, { 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, { "signal_projection_size", "sideband_projection_size", "", "", "", "", "", "", "", "" } },
    { "full_signal", DType::INT32, 2, {}, 56, true, { "signal_y_size", "signal_x_size", "", "", "", "", "", "", "", "" }, { 4, 4, 4, 0, 0, 0, 0, 0, 0, 0 }, { "signal_projection_size", "sideband_projection_size", "reference_projection_size", "", "", "", "", "", "", "" } },
    { "full_sideband", DType::INT32, 2, {}, 56, true, { "sideband_y_size", "sideband_x_size", "", "", "", "", "", "", "", "" }, { 4, 4, 4, 4, 0, 0, 0, 0, 0, 0 }, { "signal_projection_size", "sideband_projection_size", "reference_projection_size", "signal_y_size", "", "", "", "", "", "" } },
    { "full_reference", DType::INT32, 2, {}, 56, true, { "reference_y_size", "reference_x_size", "", "", "", "", "", "", "", "" }, { 4, 4, 4, 4, 4, 0, 0, 0, 0, 0 }, { "signal_projection_size", "sideband_projection_size", "reference_projection_size", "signal_y_size", "sideband_y_size", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_125[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 }
  };

  inline constexpr SimulatedField _fields_126[] = {
    { "readoutSpeed", DType::UINT8, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerMode", DType::UINT8, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad", DType::INT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "shutterTimeout", DType::INT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Ikrum", DType::INT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Disc", DType::INT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Preamp", DType::INT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0BufAnalogA", DType::INT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0BufAnalogB", DType::INT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Hist", DType::INT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0ThlFine", DType::INT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0ThlCourse", DType::INT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Vcas", DType::INT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Fbk", DType::INT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Gnd", DType::INT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Ths", DType::INT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0BiasLvds", DType::INT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0RefLvds", DType::INT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Ikrum", DType::INT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Disc", DType::INT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Preamp", DType::INT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1BufAnalogA", DType::INT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1BufAnalogB", DType::INT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Hist", DType::INT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1ThlFine", DType::INT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1ThlCourse", DType::INT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Vcas", DType::INT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Fbk", DType::INT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Gnd", DType::INT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Ths", DType::INT32, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1BiasLvds", DType::INT32, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1RefLvds", DType::INT32, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Ikrum", DType::INT32, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Disc", DType::INT32, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Preamp", DType::INT32, 0, {}, 136, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2BufAnalogA", DType::INT32, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2BufAnalogB", DType::INT32, 0, {}, 144, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Hist", DType::INT32, 0, {}, 148, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2ThlFine", DType::INT32, 0, {}, 152, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2ThlCourse", DType::INT32, 0, {}, 156, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Vcas", DType::INT32, 0, {}, 160, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Fbk", DType::INT32, 0, {}, 164, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Gnd", DType::INT32, 0, {}, 168, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Ths", DType::INT32, 0, {}, 172, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2BiasLvds", DType::INT32, 0, {}, 176, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2RefLvds", DType::INT32, 0, {}, 180, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Ikrum", DType::INT32, 0, {}, 184, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Disc", DType::INT32, 0, {}, 188, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Preamp", DType::INT32, 0, {}, 192, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3BufAnalogA", DType::INT32, 0, {}, 196, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3BufAnalogB", DType::INT32, 0, {}, 200, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Hist", DType::INT32, 0, {}, 204, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3ThlFine", DType::INT32, 0, {}, 208, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3ThlCourse", DType::INT32, 0, {}, 212, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Vcas", DType::INT32, 0, {}, 216, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Fbk", DType::INT32, 0, {}, 220, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Gnd", DType::INT32, 0, {}, 224, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Ths", DType::INT32, 0, {}, 228, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3BiasLvds", DType::INT32, 0, {}, 232, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3RefLvds", DType::INT32, 0, {}, 236, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_126[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_127[] = {
    { "readoutSpeed", DType::UINT8, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerMode", DType::UINT8, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad", DType::INT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timepixSpeed", DType::INT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Ikrum", DType::INT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Disc", DType::INT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Preamp", DType::INT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0BufAnalogA", DType::INT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0BufAnalogB", DType::INT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Hist", DType::INT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0ThlFine", DType::INT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0ThlCourse", DType::INT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Vcas", DType::INT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Fbk", DType::INT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Gnd", DType::INT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Ths", DType::INT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0BiasLvds", DType::INT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0RefLvds", DType::INT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Ikrum", DType::INT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Disc", DType::INT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Preamp", DType::INT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1BufAnalogA", DType::INT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1BufAnalogB", DType::INT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Hist", DType::INT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1ThlFine", DType::INT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1ThlCourse", DType::INT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Vcas", DType::INT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Fbk", DType::INT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Gnd", DType::INT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Ths", DType::INT32, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1BiasLvds", DType::INT32, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1RefLvds", DType::INT32, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Ikrum", DType::INT32, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Disc", DType::INT32, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Preamp", DType::INT32, 0, {}, 136, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2BufAnalogA", DType::INT32, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2BufAnalogB", DType::INT32, 0, {}, 144, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Hist", DType::INT32, 0, {}, 148, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2ThlFine", DType::INT32, 0, {}, 152, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2ThlCourse", DType::INT32, 0, {}, 156, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Vcas", DType::INT32, 0, {}, 160, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Fbk", DType::INT32, 0, {}, 164, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Gnd", DType::INT32, 0, {}, 168, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Ths", DType::INT32, 0, {}, 172, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2BiasLvds", DType::INT32, 0, {}, 176, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2RefLvds", DType::INT32, 0, {}, 180, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Ikrum", DType::INT32, 0, {}, 184, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Disc", DType::INT32, 0, {}, 188, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Preamp", DType::INT32, 0, {}, 192, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3BufAnalogA", DType::INT32, 0, {}, 196, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3BufAnalogB", DType::INT32, 0, {}, 200, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Hist", DType::INT32, 0, {}, 204, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3ThlFine", DType::INT32, 0, {}, 208, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3ThlCourse", DType::INT32, 0, {}, 212, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Vcas", DType::INT32, 0, {}, 216, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Fbk", DType::INT32, 0, {}, 220, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Gnd", DType::INT32, 0, {}, 224, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Ths", DType::INT32, 0, {}, 228, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3BiasLvds", DType::INT32, 0, {}, 232, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3RefLvds", DType::INT32, 0, {}, 236, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "driverVersion", DType::INT32, 0, {}, 240, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "firmwareVersion", DType::UINT32, 0, {}, 244, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixelThreshSize", DType::UINT32, 0, {}, 248, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixelThresh", DType::UINT8, 1, { 262144 }, 252, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip0Name", DType::CHARSTR, 1, { 16 }, 262396, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip1Name", DType::CHARSTR, 1, { 16 }, 262412, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip2Name", DType::CHARSTR, 1, { 16 }, 262428, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip3Name", DType::CHARSTR, 1, { 16 }, 262444, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip0ID", DType::INT32, 0, {}, 262460, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip1ID", DType::INT32, 0, {}, 262464, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip2ID", DType::INT32, 0, {}, 262468, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip3ID", DType::INT32, 0, {}, 262472, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixelThresh", DType::UINT8, 1, {}, 0, true, { "PixelThreshMax", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_127[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_128[] = {
    { "readoutSpeed", DType::UINT8, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timepixMode", DType::UINT8, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad", DType::INT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timepixSpeed", DType::INT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Ikrum", DType::INT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Disc", DType::INT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Preamp", DType::INT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0BufAnalogA", DType::INT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0BufAnalogB", DType::INT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Hist", DType::INT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0ThlFine", DType::INT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0ThlCourse", DType::INT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Vcas", DType::INT32, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Fbk", DType::INT32, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Gnd", DType::INT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0Ths", DType::INT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0BiasLvds", DType::INT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac0RefLvds", DType::INT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Ikrum", DType::INT32, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Disc", DType::INT32, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Preamp", DType::INT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1BufAnalogA", DType::INT32, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1BufAnalogB", DType::INT32, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Hist", DType::INT32, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1ThlFine", DType::INT32, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1ThlCourse", DType::INT32, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Vcas", DType::INT32, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Fbk", DType::INT32, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Gnd", DType::INT32, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1Ths", DType::INT32, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1BiasLvds", DType::INT32, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac1RefLvds", DType::INT32, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Ikrum", DType::INT32, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Disc", DType::INT32, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Preamp", DType::INT32, 0, {}, 136, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2BufAnalogA", DType::INT32, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2BufAnalogB", DType::INT32, 0, {}, 144, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Hist", DType::INT32, 0, {}, 148, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2ThlFine", DType::INT32, 0, {}, 152, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2ThlCourse", DType::INT32, 0, {}, 156, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Vcas", DType::INT32, 0, {}, 160, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Fbk", DType::INT32, 0, {}, 164, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Gnd", DType::INT32, 0, {}, 168, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2Ths", DType::INT32, 0, {}, 172, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2BiasLvds", DType::INT32, 0, {}, 176, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac2RefLvds", DType::INT32, 0, {}, 180, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Ikrum", DType::INT32, 0, {}, 184, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Disc", DType::INT32, 0, {}, 188, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Preamp", DType::INT32, 0, {}, 192, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3BufAnalogA", DType::INT32, 0, {}, 196, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3BufAnalogB", DType::INT32, 0, {}, 200, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Hist", DType::INT32, 0, {}, 204, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3ThlFine", DType::INT32, 0, {}, 208, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3ThlCourse", DType::INT32, 0, {}, 212, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Vcas", DType::INT32, 0, {}, 216, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Fbk", DType::INT32, 0, {}, 220, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Gnd", DType::INT32, 0, {}, 224, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3Ths", DType::INT32, 0, {}, 228, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3BiasLvds", DType::INT32, 0, {}, 232, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dac3RefLvds", DType::INT32, 0, {}, 236, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "dacBias", DType::INT8, 0, {}, 240, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "flags", DType::INT8, 0, {}, 244, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad2", DType::INT16, 0, {}, 248, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "driverVersion", DType::INT32, 0, {}, 252, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "firmwareVersion", DType::UINT32, 0, {}, 256, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixelThreshSize", DType::UINT32, 0, {}, 260, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixelThresh", DType::UINT8, 1, { 262144 }, 264, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip0Name", DType::CHARSTR, 1, { 16 }, 262408, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip1Name", DType::CHARSTR, 1, { 16 }, 262424, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip2Name", DType::CHARSTR, 1, { 16 }, 262440, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip3Name", DType::CHARSTR, 1, { 16 }, 262456, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip0ID", DType::INT32, 0, {}, 262472, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip1ID", DType::INT32, 0, {}, 262476, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip2ID", DType::INT32, 0, {}, 262480, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chip3ID", DType::INT32, 0, {}, 262484, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixelThresh", DType::UINT8, 1, {}, 0, true, { "PixelThreshMax", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_128[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "MaxModulesPerDetector", 8 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_129[] = {
    { "timestamp", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frameCounter", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lostRows", DType::UINT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, { 512, 512 }, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 0, true, { "Height", "Width", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_129[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };

  inline constexpr SimulatedField _fields_130[] = {
    { "width", DType::UINT16, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "height", DType::UINT16, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timestamp", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frameCounter", DType::UINT16, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "lostRows", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 12, true, { "height", "width", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_130[] = {
    { "iDataReadSize", 4608 },
    { "iNumPixels", 2048 },
    { "iActivePixelIndex", 0 },
    { "NEncoders", 3 },
    { "Depth", 14 },
    { "MaxPixelValue", 11810 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "ipimbAdcRange", 5 },
    { "ipimbAdcSteps", 65536 }
  };

  inline constexpr SimulatedField _fields_131[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32ReadoutSpeedIndex", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ReadoutEventCode", DType::UINT16, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16DelayMode", DType::UINT16, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_131[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_132[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16GainIndex", DType::UINT16, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ReadoutSpeedIndex", DType::UINT16, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ReadoutEventCode", DType::UINT16, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16DelayMode", DType::UINT16, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_132[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_133[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8GainIndex", DType::UINT8, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8ReadoutSpeedIndex", DType::UINT8, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ExposureEventCode", DType::UINT16, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32NumDelayShots", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_133[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "MaxModulesPerDetector", 8 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_134[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32MaskedHeight", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32KineticHeight", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32VsSpeed", DType::FLOAT, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8GainIndex", DType::UINT8, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8ReadoutSpeedIndex", DType::UINT8, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ExposureEventCode", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32NumDelayShots", DType::UINT32, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_134[] = {
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "MaxModulesPerDetector", 32 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_135[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16GainIndex", DType::UINT16, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ReadoutSpeedIndex", DType::UINT16, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32MaskedHeight", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32KineticHeight", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32VsSpeed", DType::FLOAT, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i16InfoReportInterval", DType::INT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ExposureEventCode", DType::UINT16, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32NumDelayShots", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_135[] = {
    { "MaxPulses", 32 },
    { "EvrOutputs", 10 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_136[] = {
    { "iShotIdStart", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fReadoutTime", DType::FLOAT, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 8, true, { "numPixelsY", "numPixelsX", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_137[] = {
    { "iShotIdStart", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fReadoutTime", DType::FLOAT, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fTemperature", DType::FLOAT, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 12, true, { "numPixelsY", "numPixelsX", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_138[] = {
    { "fTemperature", DType::FLOAT, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_139[] = {
    { "numberOfModules", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerModule", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfColumnsPerModule", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasVoltage", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gainMode", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "speedMode", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerDelay", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_139[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_140[] = {
    { "numberOfModules", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerModule", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfColumnsPerModule", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasVoltage", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gainMode", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "speedMode", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerDelay", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposurePeriod", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vb_ds", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vb_comp", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vb_pixbuf", DType::UINT16, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_ds", DType::UINT16, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_comp", DType::UINT16, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_prech", DType::UINT16, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vin_com", DType::UINT16, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vdd_prot", DType::UINT16, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_140[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_141[] = {
    { "numberOfModules", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerModule", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfColumnsPerModule", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasVoltage", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gainMode", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "speedMode", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerDelay", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposurePeriod", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vb_ds", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vb_comp", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vb_pixbuf", DType::UINT16, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_ds", DType::UINT16, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_comp", DType::UINT16, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_prech", DType::UINT16, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vin_com", DType::UINT16, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vdd_prot", DType::UINT16, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "moduleConfig", DType::ENUMVAL, 1, { 32 }, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_141[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "MaxModulesPerDetector", 8 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_142[] = {
    { "numberOfModules", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfRowsPerModule", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "numberOfColumnsPerModule", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "biasVoltage", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "gainMode", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "speedMode", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "triggerDelay", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposureTime", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "exposurePeriod", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vb_ds", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vb_comp", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vb_pixbuf", DType::UINT16, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_ds", DType::UINT16, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_comp", DType::UINT16, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vref_prech", DType::UINT16, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vin_com", DType::UINT16, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vdd_prot", DType::UINT16, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "moduleConfig", DType::ENUMVAL, 1, { 32 }, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_142[] = {
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "MaxModulesPerDetector", 32 },
    { "MaxRowsPerModule", 512 },
    { "MaxColumnsPerModule", 1024 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_143[] = {
    { "frameNumber", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fiducials", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame", DType::UINT16, 3, {}, 12, true, { "numberOfModules", "numberOfRowsPerModule", "numberOfColumnsPerModule", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_143[] = {
    { "Nsbtemp", 4 }
  };

  inline constexpr SimulatedField _fields_144[] = {
    { "frameNumber", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ticks", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fiducials", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame", DType::UINT16, 3, {}, 16, true, { "numberOfModules", "numberOfRowsPerModule", "numberOfColumnsPerModule", "", "", "", "", "", "", "" }, { 20, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numberOfModules", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_144[] = {
    { "Nsbtemp", 4 }
  };

  inline constexpr SimulatedField _fields_145[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ReadoutSpeed", DType::FLOAT, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16GainIndex", DType::UINT16, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16IntensifierGain", DType::UINT16, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f64GateDelay", DType::DOUBLE, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f64GateWidth", DType::DOUBLE, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32MaskedHeight", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32KineticHeight", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32VsSpeed", DType::FLOAT, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "i16InfoReportInterval", DType::INT16, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ExposureEventCode", DType::UINT16, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32NumIntegrationShots", DType::UINT32, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_145[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_146[] = {
    { "iShotIdStart", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fReadoutTime", DType::FLOAT, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fTemperature", DType::FLOAT, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 2, {}, 12, true, { "numPixelsY", "numPixelsX", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_147[] = {
    { "uWidth", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uHeight", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uNumSensors", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgX", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uOrgY", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinX", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "uBinY", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32ExposureTime", DType::FLOAT, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f32CoolingTemp", DType::FLOAT, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8FanMode", DType::UINT8, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8BaselineClamp", DType::UINT8, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8HighCapacity", DType::UINT8, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u8GainIndex", DType::UINT8, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ReadoutSpeedIndex", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u16ExposureEventCode", DType::UINT16, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32ExposureStartDelay", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "u32NumDelayShots", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_147[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_148[] = {
    { "iShotIdStart", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fReadoutTime", DType::FLOAT, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "temperature", DType::FLOAT, 1, {}, 8, true, { "numSensors", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::UINT16, 3, {}, 8, true, { "numSensors", "numPixelsY", "numPixelsX", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "numSensors", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_149[] = {
    { "id", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame_dim", DType::ENUMVAL, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "aux_dim", DType::ENUMVAL, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "env_dim", DType::ENUMVAL, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "number_of_registers", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "number_of_sequences", DType::UINT32, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "number_of_streams", DType::UINT32, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "payload_size", DType::UINT32, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pixel_settings", DType::UINT32, 2, {}, 44, true, { "frame_dimrows", "frame_dimcolumns", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sequence_length", DType::UINT32, 1, {}, 44, true, { "number_of_sequences", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "frame_dimrows", "", "", "", "", "", "", "", "", "" } },
    { "sequence", DType::ENUMVAL, 1, {}, 44, true, { "number_of_registers", "", "", "", "", "", "", "", "", "" }, { 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, { "frame_dimrows", "number_of_sequences", "", "", "", "", "", "", "", "" } },
    { "stream", DType::ENUMVAL, 1, {}, 44, true, { "number_of_streams", "", "", "", "", "", "", "", "", "" }, { 4, 4, 16, 0, 0, 0, 0, 0, 0, 0 }, { "frame_dimrows", "number_of_sequences", "number_of_registers", "", "", "", "", "", "", "" } },
    { "payload", DType::UINT32, 1, {}, 44, true, { "payload_size", "", "", "", "", "", "", "", "", "" }, { 4, 4, 16, 16, 0, 0, 0, 0, 0, 0 }, { "frame_dimrows", "number_of_sequences", "number_of_registers", "number_of_streams", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_149[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_150[] = {
    { "f_11_ENRC", DType::DOUBLE, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f_12_ENRC", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f_21_ENRC", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f_22_ENRC", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_151[] = {
    { "f_11_ENRC", DType::DOUBLE, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f_12_ENRC", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f_21_ENRC", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f_22_ENRC", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f_63_ENRC", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "f_64_ENRC", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_152[] = {
    { "uDamageMask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamCharge", DType::DOUBLE, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamL3Energy", DType::DOUBLE, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosX", DType::DOUBLE, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosY", DType::DOUBLE, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngX", DType::DOUBLE, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngY", DType::DOUBLE, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_153[] = {
    { "uDamageMask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamCharge", DType::DOUBLE, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamL3Energy", DType::DOUBLE, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosX", DType::DOUBLE, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosY", DType::DOUBLE, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngX", DType::DOUBLE, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngY", DType::DOUBLE, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC2", DType::DOUBLE, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_154[] = {
    { "uDamageMask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamCharge", DType::DOUBLE, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamL3Energy", DType::DOUBLE, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosX", DType::DOUBLE, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosY", DType::DOUBLE, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngX", DType::DOUBLE, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngY", DType::DOUBLE, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC2", DType::DOUBLE, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC2", DType::DOUBLE, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_155[] = {
    { "uDamageMask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamCharge", DType::DOUBLE, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamL3Energy", DType::DOUBLE, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosX", DType::DOUBLE, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosY", DType::DOUBLE, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngX", DType::DOUBLE, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngY", DType::DOUBLE, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC2", DType::DOUBLE, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC2", DType::DOUBLE, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC1", DType::DOUBLE, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC1", DType::DOUBLE, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_156[] = {
    { "uDamageMask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamCharge", DType::DOUBLE, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamL3Energy", DType::DOUBLE, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosX", DType::DOUBLE, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosY", DType::DOUBLE, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngX", DType::DOUBLE, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngY", DType::DOUBLE, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC2", DType::DOUBLE, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC2", DType::DOUBLE, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC1", DType::DOUBLE, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC1", DType::DOUBLE, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndPosX", DType::DOUBLE, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndPosY", DType::DOUBLE, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndAngX", DType::DOUBLE, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndAngY", DType::DOUBLE, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_157[] = {
    { "uDamageMask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamCharge", DType::DOUBLE, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamL3Energy", DType::DOUBLE, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosX", DType::DOUBLE, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosY", DType::DOUBLE, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngX", DType::DOUBLE, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngY", DType::DOUBLE, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC2", DType::DOUBLE, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC2", DType::DOUBLE, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC1", DType::DOUBLE, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC1", DType::DOUBLE, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndPosX", DType::DOUBLE, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndPosY", DType::DOUBLE, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndAngX", DType::DOUBLE, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndAngY", DType::DOUBLE, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamXTCAVAmpl", DType::DOUBLE, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamXTCAVPhase", DType::DOUBLE, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamDumpCharge", DType::DOUBLE, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_158[] = {
    { "uDamageMask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamCharge", DType::DOUBLE, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamL3Energy", DType::DOUBLE, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosX", DType::DOUBLE, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosY", DType::DOUBLE, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngX", DType::DOUBLE, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngY", DType::DOUBLE, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC2", DType::DOUBLE, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC2", DType::DOUBLE, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC1", DType::DOUBLE, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC1", DType::DOUBLE, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndPosX", DType::DOUBLE, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndPosY", DType::DOUBLE, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndAngX", DType::DOUBLE, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndAngY", DType::DOUBLE, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamXTCAVAmpl", DType::DOUBLE, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamXTCAVPhase", DType::DOUBLE, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamDumpCharge", DType::DOUBLE, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPhotonEnergy", DType::DOUBLE, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTU250", DType::DOUBLE, 0, {}, 148, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTU450", DType::DOUBLE, 0, {}, 156, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_159[] = {
    { "uDamageMask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamCharge", DType::DOUBLE, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamL3Energy", DType::DOUBLE, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosX", DType::DOUBLE, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUPosY", DType::DOUBLE, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngX", DType::DOUBLE, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTUAngY", DType::DOUBLE, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC2", DType::DOUBLE, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC2", DType::DOUBLE, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPkCurrBC1", DType::DOUBLE, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamEnergyBC1", DType::DOUBLE, 0, {}, 76, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndPosX", DType::DOUBLE, 0, {}, 84, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndPosY", DType::DOUBLE, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndAngX", DType::DOUBLE, 0, {}, 100, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamUndAngY", DType::DOUBLE, 0, {}, 108, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamXTCAVAmpl", DType::DOUBLE, 0, {}, 116, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamXTCAVPhase", DType::DOUBLE, 0, {}, 124, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamDumpCharge", DType::DOUBLE, 0, {}, 132, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamPhotonEnergy", DType::DOUBLE, 0, {}, 140, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTU250", DType::DOUBLE, 0, {}, 148, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fEbeamLTU450", DType::DOUBLE, 0, {}, 156, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_160[] = {
    { "nBPMS", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fBPM_X", DType::DOUBLE, 1, {}, 4, true, { "_nBPMS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fBPM_Y", DType::DOUBLE, 1, {}, 4, true, { "_nBPMS", "", "", "", "", "", "", "", "", "" }, { 8, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_nBPMS", "", "", "", "", "", "", "", "", "" } },
    { "fBPM_TMIT", DType::DOUBLE, 1, {}, 4, true, { "_nBPMS", "", "", "", "", "", "", "", "", "" }, { 8, 8, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_nBPMS", "_nBPMS", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_161[] = {
    { "fFitTime1", DType::DOUBLE, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fFitTime2", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fCharge1", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fCharge2", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_162[] = {
    { "fFitTime1", DType::DOUBLE, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fFitTime2", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fCharge1", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fCharge2", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_163[] = {
    { "ipimbData", DType::ENUMVAL, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ipimbConfig", DType::ENUMVAL, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ipmFexData", DType::ENUMVAL, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_164[] = {
    { "ipimbData", DType::ENUMVAL, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ipimbConfig", DType::ENUMVAL, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ipmFexData", DType::ENUMVAL, 0, {}, 92, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_165[] = {
    { "camConfig", DType::ENUMVAL, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pimConfig", DType::ENUMVAL, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "frame", DType::ENUMVAL, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_166[] = {
    { "config", DType::ENUMVAL, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fexConfig", DType::ENUMVAL, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::ENUMVAL, 0, {}, 272, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fexData", DType::ENUMVAL, 0, {}, 312, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_167[] = {
    { "strGasType", DType::CHARSTR, 1, { 32 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fPressure", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fTemperature", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fCurrent", DType::DOUBLE, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fHvMeshElectron", DType::DOUBLE, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fHvMeshIon", DType::DOUBLE, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fHvMultIon", DType::DOUBLE, 0, {}, 72, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fChargeQ", DType::DOUBLE, 0, {}, 80, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fPhotonEnergy", DType::DOUBLE, 0, {}, 88, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fMultPulseIntensity", DType::DOUBLE, 0, {}, 96, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fKeithleyPulseIntensity", DType::DOUBLE, 0, {}, 104, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fPulseEnergy", DType::DOUBLE, 0, {}, 112, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fPulseEnergyFEE", DType::DOUBLE, 0, {}, 120, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fTransmission", DType::DOUBLE, 0, {}, 128, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fTransmissionFEE", DType::DOUBLE, 0, {}, 136, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fSpare6", DType::DOUBLE, 0, {}, 144, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_168[] = {
    { "fMilliJoulesPerPulse", DType::DOUBLE, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fMilliJoulesAverage", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fCorrectedSumPerPulse", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fBgValuePerSample", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fRelativeEnergyPerPulse", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fSpare1", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_169[] = {
    { "fMilliJoulesPerPulse", DType::DOUBLE, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fMilliJoulesAverage", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fSumAllPeaksFiltBkgd", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fRawAvgBkgd", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fRelativeEnergyPerPulse", DType::DOUBLE, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fSumAllPeaksRawBkgd", DType::DOUBLE, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_170[] = {
    { "config", DType::ENUMVAL, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "data", DType::ENUMVAL, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_171[] = {
    { "hproj", DType::UINT32, 1, { 1024 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vproj", DType::UINT32, 1, { 256 }, 4096, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "hproj", DType::UINT32, 1, {}, 0, true, { "1024", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "vproj", DType::UINT32, 1, {}, 0, true, { "256", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_172[] = {
    { "width", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "hproj_y1", DType::UINT32, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "hproj_y2", DType::UINT32, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fComRaw", DType::DOUBLE, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fBaseline", DType::DOUBLE, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fCom", DType::DOUBLE, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "fIntegral", DType::DOUBLE, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nPeaks", DType::UINT32, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "hproj", DType::INT32, 1, {}, 48, true, { "_width", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "peakPos", DType::DOUBLE, 1, {}, 48, true, { "_nPeaks", "", "", "", "", "", "", "", "", "" }, { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_width", "", "", "", "", "", "", "", "", "" } },
    { "peakHeight", DType::DOUBLE, 1, {}, 48, true, { "_nPeaks", "", "", "", "", "", "", "", "", "" }, { 4, 8, 0, 0, 0, 0, 0, 0, 0, 0 }, { "_width", "_nPeaks", "", "", "", "", "", "", "", "" } },
    { "FWHM", DType::DOUBLE, 1, {}, 48, true, { "_nPeaks", "", "", "", "", "", "", "", "", "" }, { 4, 8, 8, 0, 0, 0, 0, 0, 0, 0 }, { "_width", "_nPeaks", "_nPeaks", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_173[] = {
    { "numChannels", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channelVoltages", DType::DOUBLE, 1, {}, 4, true, { "_numChannels", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SimulatedField _fields_174[] = {
    { "TotalIntensity", DType::DOUBLE, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "X_Position", DType::DOUBLE, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "Y_Position", DType::DOUBLE, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "peakA", DType::DOUBLE, 1, { 4 }, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "peakT", DType::UINT16, 1, { 4 }, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "peakA", DType::DOUBLE, 1, {}, 0, true, { "NCHANNELS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "peakT", DType::UINT16, 1, {}, 0, true, { "NCHANNELS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_174[] = {
    { "NCHANNELS", 16 }
  };

  inline constexpr SimulatedField _fields_175[] = {
    { "triggerCounter", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "serialID", DType::UINT64, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chargeAmpRange", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRange", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "resetLength", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "resetDelay", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chargeAmpRefVoltage", DType::FLOAT, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationVoltage", DType::FLOAT, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "diodeBias", DType::FLOAT, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "status", DType::UINT16, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "errors", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calStrobeLength", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad0", DType::UINT16, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "trigDelay", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_175[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_176[] = {
    { "triggerCounter", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "serialID", DType::UINT64, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chargeAmpRange", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationRange", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "resetLength", DType::UINT32, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "resetDelay", DType::UINT32, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "chargeAmpRefVoltage", DType::FLOAT, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calibrationVoltage", DType::FLOAT, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "diodeBias", DType::FLOAT, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "status", DType::UINT16, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "errors", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "calStrobeLength", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "pad0", DType::UINT16, 0, {}, 56, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "trigDelay", DType::UINT32, 0, {}, 60, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "trigPsDelay", DType::UINT32, 0, {}, 64, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "adcDelay", DType::UINT32, 0, {}, 68, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_176[] = {
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "MX340HS_Row_Pixels", 7680 },
    { "MX340HS_Column_Pixels", 7680 },
    { "MX170HS_Row_Pixels", 3840 },
    { "MX170HS_Column_Pixels", 3840 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "MaxConfigLines", 16384 },
    { "MaxConfigLineLength", 2048 },
    { "beamOn", 100 },
    { "baseRate", 40 },
    { "singleShot", 150 },
    { "numAsicsRead", 4 },
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "ChipCount", 4 },
    { "ChipNameMax", 16 },
    { "PixelThreshMax", 262144 },
    { "chipCount", 4 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_177[] = {
    { "triggerCounter", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "config0", DType::UINT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "config1", DType::UINT16, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "config2", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel0", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel1", DType::UINT16, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel2", DType::UINT16, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel3", DType::UINT16, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "checksum", DType::UINT16, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_177[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };

  inline constexpr SimulatedField _fields_178[] = {
    { "triggerCounter", DType::UINT64, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "config0", DType::UINT16, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "config1", DType::UINT16, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "config2", DType::UINT16, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel0", DType::UINT16, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel1", DType::UINT16, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel2", DType::UINT16, 0, {}, 28, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel3", DType::UINT16, 0, {}, 32, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel0ps", DType::UINT16, 0, {}, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel1ps", DType::UINT16, 0, {}, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel2ps", DType::UINT16, 0, {}, 44, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "channel3ps", DType::UINT16, 0, {}, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "checksum", DType::UINT16, 0, {}, 52, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_178[] = {
    { "iDataReadSize", 4608 },
    { "iNumPixels", 2048 },
    { "iActivePixelIndex", 0 },
    { "NEncoders", 3 },
    { "Depth", 14 },
    { "MaxPixelValue", 11810 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "ipimbAdcRange", 5 },
    { "ipimbAdcSteps", 65536 }
  };

  inline constexpr SimulatedField _fields_179[] = {
    { "chanMask", DType::UINT32, 0, {}, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "delayTime", DType::DOUBLE, 0, {}, 4, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "interleaveMode", DType::UINT32, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "nbrSamples", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "evtCode", DType::UINT32, 0, {}, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "sampRate", DType::DOUBLE, 0, {}, 24, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_179[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_180[] = {
    { "offset", DType::INT32, 1, { 4 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "scale", DType::DOUBLE, 1, { 4 }, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "name", DType::CHARSTR, 2, { 4, 48 }, 48, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "offset", DType::INT32, 1, {}, 0, true, { "NCHANNELS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "scale", DType::DOUBLE, 1, {}, 0, true, { "NCHANNELS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_180[] = {
    { "NCHANNELS", 4 },
    { "NAME_CHAR_MAX", 48 },
    { "numberOfRows", 352 },
    { "numberOfReadableRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 },
    { "numberOfAsics", 64 }
  };

  inline constexpr SimulatedField _fields_181[] = {
    { "encoder_value", DType::DOUBLE, 1, { 4 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "encoder_values", DType::DOUBLE, 1, {}, 0, true, { "Encoder_Inputs", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_181[] = {
    { "Encoder_Inputs", 4 }
  };

  inline constexpr SimulatedField _fields_182[] = {
    { "count_mode", DType::UINT32, 1, { 4 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quad_mode", DType::UINT32, 1, { 4 }, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "counting_mode", DType::UINT32, 1, {}, 0, true, { "NCHANNELS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "quadrature_mode", DType::UINT32, 1, {}, 0, true, { "NCHANNELS", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_182[] = {
    { "LUT_Size", 4096 },
    { "Row_Pixels", 2048 },
    { "Column_Pixels", 2048 },
    { "Output_LUT_Size", 4096 },
    { "STR_LEN_MAX", 64 },
    { "MaxNumberOfSamples", 1023 },
    { "NumberOfPots", 13 },
    { "NumberOfSides", 2 },
    { "BasePixelSize", 44 },
    { "DeviceIDMax", 40 },
    { "maxWidth", 2048 },
    { "maxHeight", 2048 },
    { "NumCalibConstants", 3 },
    { "FILENAME_CHAR_MAX", 256 },
    { "numberOfAsicsPerRow", 2 },
    { "numberOfAsicsPerColumn", 2 },
    { "numberOfRowsPerAsic", 176 },
    { "numberOfPixelsPerAsicRow", 192 },
    { "numberOfRows", 352 },
    { "numberOfColumns", 384 },
    { "numberOfAsics", 64 },
    { "MaxChan", 20 },
    { "numAsicsRead", 4 },
    { "LowestChannel", 0 },
    { "HighestChannel", 15 },
    { "LowestFps", 1 },
    { "HighestFps", 120 },
    { "ChipCount", 4 },
    { "NCHANNELS", 4 },
    { "numberOfReadableRows", 352 },
    { "numberOfCalibrationRows", 4 },
    { "numberOfEnvironmentalRows", 2 }
  };

  inline constexpr SimulatedField _fields_183[] = {
    { "header", DType::UINT8, 1, { 6 }, 0, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "din", DType::UINT8, 0, {}, 8, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "estop", DType::UINT8, 0, {}, 12, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "timestamp", DType::UINT32, 0, {}, 16, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "count", DType::UINT32, 1, { 4 }, 20, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "status", DType::UINT8, 1, { 4 }, 36, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "ain", DType::UINT16, 1, { 4 }, 40, false, { "", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "status", DType::UINT8, 1, {}, 0, true, { "4", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } },
    { "analog_in", DType::UINT16, 1, {}, 0, true, { "Analog_Inputs", "", "", "", "", "", "", "", "", "" }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { "", "", "", "", "", "", "", "", "", "" } }
  };

  inline constexpr SchemaConstant _consts_183[] = {
    { "iDataReadSize", 8192 },
    { "iNumPixels", 3840 },
    { "iActivePixelIndex", 22 },
    { "Height", 512 },
    { "Width", 512 },
    { "Depth", 14 },
    { "DepthBytes", 2 },
    { "MaxPixelValue", 11810 },
    { "width", 512 },
    { "height", 512 },
    { "depth", 14 },
    { "depth_bytes", 2 },
    { "Encoder_Inputs", 4 },
    { "Analog_Inputs", 4 },
    { "Digital_Inputs", 8 }
  };


  inline constexpr SimulatedSchema g_schema_registry[] = {
    { TypeId::Id_Opal1kConfig, 1, _fields_0, _consts_0, TypeId::Id_Frame },
    { TypeId::Id_OrcaConfig, 1, _fields_1, _consts_1, TypeId::Any },
    { TypeId::Id_SmlDataConfig, 1, _fields_2, _consts_2, TypeId::Any },
    { TypeId::Id_SmlDataOrigDgramOffset, 1, _fields_3, {}, TypeId::Any },
    { TypeId::Id_SmlDataProxy, 1, _fields_4, {}, TypeId::Any },
    { TypeId::Id_AliasConfig, 1, _fields_5, _consts_5, TypeId::Any },
    { TypeId::Id_FrameFccdConfig, 1, {}, _consts_6, TypeId::Any },
    { TypeId::Id_FrameFexConfig, 1, _fields_7, _consts_7, TypeId::Any },
    { TypeId::Id_Frame, 1, _fields_8, {}, TypeId::Any },
    { TypeId::Id_TwoDGaussian, 1, _fields_9, {}, TypeId::Any },
    { TypeId::Id_ControlsCameraConfig, 1, _fields_10, _consts_10, TypeId::Any },
    { TypeId::Id_ZylaConfig, 1, _fields_11, _consts_11, TypeId::Id_ZylaFrame },
    { TypeId::Id_ZylaFrame, 1, _fields_12, {}, TypeId::Any },
    { TypeId::Id_ImpConfig, 1, _fields_13, _consts_13, TypeId::Any },
    { TypeId::Id_ImpData, 1, _fields_14, _consts_14, TypeId::Any },
    { TypeId::Id_AndorConfig, 1, _fields_15, _consts_15, TypeId::Id_AndorFrame },
    { TypeId::Id_AndorConfig, 2, _fields_16, _consts_16, TypeId::Id_AndorFrame },
    { TypeId::Id_AndorFrame, 1, _fields_17, {}, TypeId::Any },
    { TypeId::Id_UxiConfig, 1, _fields_18, _consts_18, TypeId::Id_UxiFrame },
    { TypeId::Id_UxiConfig, 2, _fields_19, _consts_19, TypeId::Id_UxiFrame },
    { TypeId::Id_UxiConfig, 3, _fields_20, _consts_20, TypeId::Id_UxiFrame },
    { TypeId::Id_UxiFrame, 1, _fields_21, {}, TypeId::Any },
    { TypeId::Id_TM6740Config, 1, _fields_22, _consts_22, TypeId::Id_Frame },
    { TypeId::Id_TM6740Config, 2, _fields_23, _consts_23, TypeId::Id_Frame },
    { TypeId::Id_Generic1DConfig, 0, _fields_24, _consts_24, TypeId::Any },
    { TypeId::Id_Generic1DData, 0, _fields_25, {}, TypeId::Any },
    { TypeId::Id_RayonixConfig, 1, _fields_26, _consts_26, TypeId::Any },
    { TypeId::Id_RayonixConfig, 2, _fields_27, _consts_27, TypeId::Any },
    { TypeId::Id_StreakConfig, 1, _fields_28, _consts_28, TypeId::Any },
    { TypeId::Id_DiodeFexConfig, 1, _fields_29, _consts_29, TypeId::Any },
    { TypeId::Id_DiodeFexConfig, 2, _fields_30, _consts_30, TypeId::Any },
    { TypeId::Id_DiodeFex, 1, _fields_31, {}, TypeId::Any },
    { TypeId::Id_IpmFexConfig, 1, _fields_32, _consts_32, TypeId::Any },
    { TypeId::Id_IpmFexConfig, 2, _fields_33, _consts_33, TypeId::Any },
    { TypeId::Id_IpmFex, 1, _fields_34, _consts_34, TypeId::Any },
    { TypeId::Id_PimImageConfig, 1, _fields_35, _consts_35, TypeId::Any },
    { TypeId::Id_pnCCDconfig, 1, _fields_36, _consts_36, TypeId::Id_pnCCDframe },
    { TypeId::Id_pnCCDconfig, 2, _fields_37, _consts_37, TypeId::Id_pnCCDframe },
    { TypeId::Id_pnCCDframe, 1, _fields_38, {}, TypeId::Any },
    { TypeId::Id_pnCCDframe, 1, {}, {}, TypeId::Any },
    { TypeId::Id_FliConfig, 1, _fields_40, _consts_40, TypeId::Any },
    { TypeId::Id_FliFrame, 1, _fields_41, {}, TypeId::Any },
    { TypeId::Id_iStarConfig, 1, _fields_42, _consts_42, TypeId::Any },
    { TypeId::Id_ArchonConfig, 1, _fields_43, _consts_43, TypeId::Any },
    { TypeId::Id_ArchonConfig, 2, _fields_44, _consts_44, TypeId::Any },
    { TypeId::Id_ArchonConfig, 3, _fields_45, _consts_45, TypeId::Any },
    { TypeId::Id_ArchonConfig, 4, _fields_46, _consts_46, TypeId::Any },
    { TypeId::Id_EpixConfig, 1, _fields_47, _consts_47, TypeId::Id_EpixElement },
    { TypeId::Id_Epix10kConfig, 1, _fields_48, _consts_48, TypeId::Id_EpixElement },
    { TypeId::Id_Epix100aConfig, 1, _fields_49, _consts_49, TypeId::Id_EpixElement },
    { TypeId::Id_Epix100aConfig, 2, _fields_50, _consts_50, TypeId::Id_EpixElement },
    { TypeId::Id_EpixSConfig, 1, _fields_51, _consts_51, TypeId::Any },
    { TypeId::Id_Epix10kaConfig, 1, _fields_52, _consts_52, TypeId::Id_EpixElement },
    { TypeId::Id_Epix10kaConfig, 2, _fields_53, _consts_53, TypeId::Id_EpixElement },
    { TypeId::Id_Epix10kaQuadConfig, 1, _fields_54, _consts_54, TypeId::Any },
    { TypeId::Id_Epix10kaQuadConfig, 2, _fields_55, _consts_55, TypeId::Any },
    { TypeId::Id_Epix10ka2MConfig, 1, _fields_56, _consts_56, TypeId::Id_Epix10kaArray },
    { TypeId::Id_Epix10ka2MConfig, 2, _fields_57, _consts_57, TypeId::Id_Epix10kaArray },
    { TypeId::Id_EpixElement, 1, _fields_58, _consts_58, TypeId::Any },
    { TypeId::Id_EpixElement, 2, _fields_59, _consts_59, TypeId::Any },
    { TypeId::Id_EpixElement, 3, _fields_60, _consts_60, TypeId::Any },
    { TypeId::Id_Epix10kaArray, 1, _fields_61, _consts_61, TypeId::Any },
    { TypeId::Id_ControlConfig, 1, _fields_62, _consts_62, TypeId::Any },
    { TypeId::Id_ControlConfig, 2, _fields_63, _consts_63, TypeId::Any },
    { TypeId::Id_ControlConfig, 3, _fields_64, _consts_64, TypeId::Any },
    { TypeId::Id_ControlConfig, 4, _fields_65, _consts_65, TypeId::Any },
    { TypeId::Id_PixisConfig, 1, _fields_66, _consts_66, TypeId::Id_PixisFrame },
    { TypeId::Id_PixisFrame, 1, _fields_67, {}, TypeId::Any },
    { TypeId::Id_EpixSamplerConfig, 1, _fields_68, _consts_68, TypeId::Any },
    { TypeId::Id_EpixSamplerElement, 1, _fields_69, _consts_69, TypeId::Any },
    { TypeId::Id_EvrConfig, 1, _fields_70, _consts_70, TypeId::Id_EvrData },
    { TypeId::Id_EvrConfig, 2, _fields_71, _consts_71, TypeId::Id_EvrData },
    { TypeId::Id_EvrConfig, 3, _fields_72, _consts_72, TypeId::Id_EvrData },
    { TypeId::Id_EvrConfig, 4, _fields_73, _consts_73, TypeId::Id_EvrData },
    { TypeId::Id_EvrConfig, 5, _fields_74, _consts_74, TypeId::Id_EvrData },
    { TypeId::Id_EvrConfig, 6, _fields_75, _consts_75, TypeId::Id_EvrData },
    { TypeId::Id_EvrConfig, 7, _fields_76, _consts_76, TypeId::Id_EvrData },
    { TypeId::Id_EvsConfig, 1, _fields_77, _consts_77, TypeId::Any },
    { TypeId::Id_EvrData, 3, _fields_78, _consts_78, TypeId::Any },
    { TypeId::Id_EvrData, 4, _fields_79, {}, TypeId::Any },
    { TypeId::Id_EvrIOConfig, 1, _fields_80, _consts_80, TypeId::Any },
    { TypeId::Id_EvrIOConfig, 2, _fields_81, _consts_81, TypeId::Any },
    { TypeId::Id_AcqConfig, 1, _fields_82, _consts_82, TypeId::Id_AcqWaveform },
    { TypeId::Id_AcqWaveform, 1, {}, {}, TypeId::Any },
    { TypeId::Id_AcqTdcConfig, 1, _fields_84, _consts_84, TypeId::Any },
    { TypeId::Id_AcqTdcData, 1, _fields_85, {}, TypeId::Any },
    { TypeId::Id_PartitionConfig, 1, _fields_86, _consts_86, TypeId::Any },
    { TypeId::Id_PartitionConfig, 2, _fields_87, _consts_87, TypeId::Any },
    { TypeId::Id_OceanOpticsConfig, 1, _fields_88, _consts_88, TypeId::Any },
    { TypeId::Id_OceanOpticsConfig, 2, _fields_89, _consts_89, TypeId::Any },
    { TypeId::Id_OceanOpticsData, 1, _fields_90, _consts_90, TypeId::Any },
    { TypeId::Id_OceanOpticsData, 2, _fields_91, _consts_91, TypeId::Any },
    { TypeId::Id_OceanOpticsData, 3, _fields_92, _consts_92, TypeId::Any },
    { TypeId::Id_CspadConfig, 1, _fields_93, _consts_93, TypeId::Id_CspadElement },
    { TypeId::Id_CspadConfig, 2, _fields_94, _consts_94, TypeId::Id_CspadElement },
    { TypeId::Id_CspadConfig, 3, _fields_95, _consts_95, TypeId::Id_CspadElement },
    { TypeId::Id_CspadConfig, 4, _fields_96, _consts_96, TypeId::Id_CspadElement },
    { TypeId::Id_CspadConfig, 5, _fields_97, _consts_97, TypeId::Id_CspadElement },
    { TypeId::Id_CspadElement, 1, {}, _consts_98, TypeId::Any },
    { TypeId::Id_CspadElement, 2, {}, _consts_99, TypeId::Any },
    { TypeId::Id_L3TConfig, 1, _fields_100, _consts_100, TypeId::Any },
    { TypeId::Id_L3TData, 1, _fields_101, _consts_101, TypeId::Any },
    { TypeId::Id_L3TData, 2, _fields_102, _consts_102, TypeId::Any },
    { TypeId::Id_Cspad2x2Config, 1, _fields_103, _consts_103, TypeId::Id_Cspad2x2Element },
    { TypeId::Id_Cspad2x2Config, 2, _fields_104, _consts_104, TypeId::Id_Cspad2x2Element },
    { TypeId::Id_Cspad2x2Element, 1, _fields_105, _consts_105, TypeId::Any },
    { TypeId::Id_FccdConfig, 1, _fields_106, _consts_106, TypeId::Id_Frame },
    { TypeId::Id_FccdConfig, 2, _fields_107, _consts_107, TypeId::Id_Frame },
    { TypeId::Id_EpicsConfig, 1, _fields_108, _consts_108, TypeId::Any },
    { TypeId::Id_Gsc16aiConfig, 1, _fields_109, _consts_109, TypeId::Any },
    { TypeId::Id_Gsc16aiData, 1, _fields_110, _consts_110, TypeId::Any },
    { TypeId::Id_QuartzConfig, 1, _fields_111, _consts_111, TypeId::Any },
    { TypeId::Id_QuartzConfig, 2, _fields_112, _consts_112, TypeId::Any },
    { TypeId::Id_AlviumConfig, 1, _fields_113, _consts_113, TypeId::Id_VimbaFrame },
    { TypeId::Id_VimbaFrame, 1, _fields_114, {}, TypeId::Any },
    { TypeId::Id_Arraychar, 1, _fields_115, _consts_115, TypeId::Any },
    { TypeId::Id_EncoderConfig, 1, _fields_116, _consts_116, TypeId::Id_EncoderData },
    { TypeId::Id_EncoderConfig, 2, _fields_117, _consts_117, TypeId::Id_EncoderData },
    { TypeId::Id_EncoderData, 1, _fields_118, _consts_118, TypeId::Any },
    { TypeId::Id_EncoderData, 2, _fields_119, _consts_119, TypeId::Any },
    { TypeId::Id_TimeToolConfig, 1, _fields_120, _consts_120, TypeId::Any },
    { TypeId::Id_TimeToolConfig, 2, _fields_121, _consts_121, TypeId::Any },
    { TypeId::Id_TimeToolConfig, 3, _fields_122, _consts_122, TypeId::Any },
    { TypeId::Id_TimeToolData, 1, _fields_123, _consts_123, TypeId::Any },
    { TypeId::Id_TimeToolData, 2, _fields_124, _consts_124, TypeId::Any },
    { TypeId::Id_TimeToolData, 3, _fields_125, _consts_125, TypeId::Any },
    { TypeId::Id_TimepixConfig, 1, _fields_126, _consts_126, TypeId::Any },
    { TypeId::Id_TimepixConfig, 2, _fields_127, _consts_127, TypeId::Any },
    { TypeId::Id_TimepixConfig, 3, _fields_128, _consts_128, TypeId::Any },
    { TypeId::Id_TimepixData, 1, _fields_129, _consts_129, TypeId::Any },
    { TypeId::Id_TimepixData, 2, _fields_130, _consts_130, TypeId::Any },
    { TypeId::Id_PrincetonConfig, 1, _fields_131, _consts_131, TypeId::Id_PrincetonFrame },
    { TypeId::Id_PrincetonConfig, 2, _fields_132, _consts_132, TypeId::Id_PrincetonFrame },
    { TypeId::Id_PrincetonConfig, 3, _fields_133, _consts_133, TypeId::Id_PrincetonFrame },
    { TypeId::Id_PrincetonConfig, 4, _fields_134, _consts_134, TypeId::Id_PrincetonFrame },
    { TypeId::Id_PrincetonConfig, 5, _fields_135, _consts_135, TypeId::Id_PrincetonFrame },
    { TypeId::Id_PrincetonFrame, 1, _fields_136, {}, TypeId::Any },
    { TypeId::Id_PrincetonFrame, 2, _fields_137, {}, TypeId::Any },
    { TypeId::Id_PrincetonInfo, 1, _fields_138, {}, TypeId::Any },
    { TypeId::Id_JungfrauConfig, 1, _fields_139, _consts_139, TypeId::Id_JungfrauElement },
    { TypeId::Id_JungfrauConfig, 2, _fields_140, _consts_140, TypeId::Id_JungfrauElement },
    { TypeId::Id_JungfrauConfig, 3, _fields_141, _consts_141, TypeId::Id_JungfrauElement },
    { TypeId::Id_JungfrauConfig, 4, _fields_142, _consts_142, TypeId::Id_JungfrauElement },
    { TypeId::Id_JungfrauElement, 1, _fields_143, _consts_143, TypeId::Any },
    { TypeId::Id_JungfrauElement, 2, _fields_144, _consts_144, TypeId::Any },
    { TypeId::Id_PimaxConfig, 1, _fields_145, _consts_145, TypeId::Id_PimaxFrame },
    { TypeId::Id_PimaxFrame, 1, _fields_146, {}, TypeId::Any },
    { TypeId::Id_Andor3dConfig, 1, _fields_147, _consts_147, TypeId::Any },
    { TypeId::Id_Andor3dFrame, 1, _fields_148, {}, TypeId::Any },
    { TypeId::Id_GenericPgpConfig, 1, _fields_149, _consts_149, TypeId::Any },
    { TypeId::Id_FEEGasDetEnergy, 0, _fields_150, {}, TypeId::Any },
    { TypeId::Id_FEEGasDetEnergy, 1, _fields_151, {}, TypeId::Any },
    { TypeId::Id_EBeam, 0, _fields_152, {}, TypeId::Any },
    { TypeId::Id_EBeam, 1, _fields_153, {}, TypeId::Any },
    { TypeId::Id_EBeam, 2, _fields_154, {}, TypeId::Any },
    { TypeId::Id_EBeam, 3, _fields_155, {}, TypeId::Any },
    { TypeId::Id_EBeam, 4, _fields_156, {}, TypeId::Any },
    { TypeId::Id_EBeam, 5, _fields_157, {}, TypeId::Any },
    { TypeId::Id_EBeam, 6, _fields_158, {}, TypeId::Any },
    { TypeId::Id_EBeam, 7, _fields_159, {}, TypeId::Any },
    { TypeId::Id_EOrbits, 0, _fields_160, {}, TypeId::Any },
    { TypeId::Id_PhaseCavity, 0, _fields_161, {}, TypeId::Any },
    { TypeId::Id_PhaseCavity, 1, _fields_162, {}, TypeId::Any },
    { TypeId::Id_SharedIpimb, 0, _fields_163, {}, TypeId::Any },
    { TypeId::Id_SharedIpimb, 1, _fields_164, {}, TypeId::Any },
    { TypeId::Id_SharedPim, 1, _fields_165, {}, TypeId::Any },
    { TypeId::Id_SharedUsdUsb, 1, _fields_166, {}, TypeId::Any },
    { TypeId::Id_GMD, 0, _fields_167, {}, TypeId::Any },
    { TypeId::Id_GMD, 1, _fields_168, {}, TypeId::Any },
    { TypeId::Id_GMD, 2, _fields_169, {}, TypeId::Any },
    { TypeId::Id_SharedAcqADC, 1, _fields_170, {}, TypeId::Any },
    { TypeId::Id_Spectrometer, 0, _fields_171, {}, TypeId::Any },
    { TypeId::Id_Spectrometer, 1, _fields_172, {}, TypeId::Any },
    { TypeId::Id_AnalogInput, 1, _fields_173, {}, TypeId::Any },
    { TypeId::Id_BeamMonitorBldData, 1, _fields_174, _consts_174, TypeId::Any },
    { TypeId::Id_IpimbConfig, 1, _fields_175, _consts_175, TypeId::Id_IpimbData },
    { TypeId::Id_IpimbConfig, 2, _fields_176, _consts_176, TypeId::Id_IpimbData },
    { TypeId::Id_IpimbData, 1, _fields_177, _consts_177, TypeId::Any },
    { TypeId::Id_IpimbData, 2, _fields_178, _consts_178, TypeId::Any },
    { TypeId::Id_QuadAdcConfig, 0, _fields_179, _consts_179, TypeId::Any },
    { TypeId::Id_UsdUsbFexConfig, 1, _fields_180, _consts_180, TypeId::Any },
    { TypeId::Id_UsdUsbFexData, 1, _fields_181, _consts_181, TypeId::Any },
    { TypeId::Id_UsdUsbConfig, 1, _fields_182, _consts_182, TypeId::Any },
    { TypeId::Id_UsdUsbData, 1, _fields_183, _consts_183, TypeId::Any }
  };

  inline const SimulatedSchema* find_schema(TypeId::Type type, std::uint32_t version) {
    for (const auto& schema : g_schema_registry) {
      if (schema.type == type && schema.version == version) {
        return &schema;
      }
    }
    return nullptr;
  }
} // namespace sbio::XTC1

#endif // SBIO_FORMATS_XTC1_SCHEMAS_HH
