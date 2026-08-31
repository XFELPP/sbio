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

#ifndef SBIO_FORMATS_RANDOM_RANDOM_LOCATOR_HH
#define SBIO_FORMATS_RANDOM_RANDOM_LOCATOR_HH

#include "sbio/core/locator.hh"
#include "sbio/core/stream.hh"
#include "sbio/formats/random/randfmt.hh"
#include "sbio/formats/random/random_traits.hh"
#include "sbio/locators/custom_lambda.hh"
#include "sbio/util/string.hh"

#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

namespace sbio {
  // --- Specializations for locator traits --- //

  template <>
  struct LocatorParameters<CustomLambdaLocator, RandomTraits> {
    struct Type {
      randfmt::DetectorSpec detectors[10];
      hd_std::uint8_t num_detectors { 0 };

      Type() = default;
      Type(randfmt::DetectorSpec* dets, hd_std::uint8_t num)
        : num_detectors(num)
      {
        for (hd_std::uint8_t i = 0; i < num; ++i) {
          detectors[i] = dets[i];
        }
      }
    };
  };

  template <>
  struct CustomLambdaLocatorTraits<RandomTraits> {
    using Parameters = LocatorParameters_t<CustomLambdaLocator, RandomTraits>;

    static constexpr auto finder_lam =
      [] <typename DS> (DS& ds,
                        const Parameters& params,
                        const GenericStreamConfig<typename DS::DataFormat>& base_cfg) {
      // For now, will create the file(s) when trying to look for them...
#ifndef __CUDA_ARCH__
      hd_std::size_t nstream { 0 };
      char name_buf[RandomTraits::MaxNameSize];
      for (hd_std::uint8_t d = 0; d < params.num_detectors; ++d) {
        randfmt::DetectorSpec spec { params.detectors[d] };

        // TODO: In the future, will want to add ability to split data into multiple streams
        hd_std::size_t streams_per_det { 1 };

        for (hd_std::size_t s = 0; s < streams_per_det; ++s) {
          int cnt = snprintf(name_buf, RandomTraits::MaxNameSize, "sbio_random_stream_%zu", nstream);
          (void)cnt;

          GenericStreamConfig<RandomTraits> stream_cfg { base_cfg };

#ifdef _WIN32
          HANDLE h_file = CreateFileA(name_buf,
                                      GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                      nullptr,
                                      CREATE_ALWAYS,
                                      FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                                      nullptr);
          if (h_file == INVALID_HANDLE_VALUE) {
            return false;
          }

          stream_cfg.format_params.h_file = h_file;
          randfmt::FileHandle f_handle { h_file };
#else
          int fd = memfd_create(name_buf, 0);
          if (fd < 0) {
            return false;
          }

          stream_cfg.format_params.fd = fd;
          randfmt::FileHandle f_handle { fd };
#endif
          // Write the SuperBlock offsets only if the NoIndex mode was NOT requested
          bool enable_superblock_offsets { true };
          if (stream_cfg.format_params.indexing_mode == RandomTraits::IndexingMode::NoIndex) {
            enable_superblock_offsets = false;
          }

          hd_std::uint64_t curr_offset { 0 };

          hd_std::uint8_t flags { 0 };
          if (stream_cfg.format_params.enable_subblock_offsets) {
            flags |= (1 << static_cast<hd_std::uint8_t>(randfmt::FormatFlags::SubBlockOffsetTable));
          }
          if (enable_superblock_offsets) {
            flags |= (1 << static_cast<hd_std::uint8_t>(randfmt::FormatFlags::SuperBlockOffsetTable));
          }
          // Just write 1 detector per stream for now...
          randfmt::DetectorSpec* stream_detectors { &spec };
          hd_std::uint8_t num_detectors_per_stream { 1 };
          hd_std::uint8_t* det_block_ids { &d };
          randfmt::write_sbiornd_file(f_handle,
                                      curr_offset,
                                      num_detectors_per_stream,
                                      stream_detectors,
                                      det_block_ids,
                                      stream_cfg.format_params.seed,
                                      stream_cfg.format_params.pattern_type,
                                      stream_cfg.format_params.num_events,
                                      flags);

          for (hd_std::size_t r = 0; r < stream_cfg.VariantCount; ++r) {
#ifdef _WIN32
            stream_cfg.resources[r] = StreamResource::from_win_handle(f_handle);
#else
            stream_cfg.resources[r] = StreamResource::from_fd(f_handle);
#endif
          }

          ds.add_data_stream(stream_cfg);
          nstream++;
        }
      }

      return ds.num_data_streams() > 0;
#else
      return false;
#endif
    };
  };

} // namespace sbio

#endif // SBIO_FORMATS_RANDOM_RANDOM_LOCATOR_HH
