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

#ifndef SBIO_FORMATS_XTC2_XTC2_LOCATOR_HH
#define SBIO_FORMATS_XTC2_XTC2_LOCATOR_HH

#include "sbio/formats/xtc2/xtc2_traits.hh"
#include "sbio/locators/path_pattern.hh"
#include "sbio/locators/single_file.hh"
#include "sbio/util/string.hh"

#include <array>
#include <charconv>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

namespace sbio {
  // --- Specializations for locator traits --- //

  template <>
  struct PathPatternLocatorTraits<XTC2Traits> {
    static constexpr std::size_t VariantCount { XTC2Traits::StreamTypes::size() };

    struct Parameters {
      char experiment[XTC2Traits::MaxNameSize];
      unsigned run;

      Parameters(std::string_view exp, unsigned run_)
        : run(run_)
      {
        safe_strncpy(experiment, exp.data(), XTC2Traits::MaxNameSize);
      }

      static constexpr auto get_metadata() {
        return std::make_tuple(make_named("exp", &Parameters::experiment),
                               make_named("run", &Parameters::run));
      }
    };

    // Must update this to use SIT_PSDM_DATA
    // Need Locator to understand env var syntax then?
    static constexpr std::array<const char*, VariantCount> role_patterns {{
      "/sdf/data/lcls/ds/{exp:.3}/{exp}/xtc/{exp}-r{run:04d}",
      "/sdf/data/lcls/ds/{exp:.3}/{exp}/xtc/smalldata/{exp}-r{run:04d}"
    }};

    static std::size_t id_stream(std::string_view filename) {
      return parse_stream_chunk_tokens(filename, "-s");
    }

    static std::size_t id_chain_order(std::string_view filename) {
      return parse_stream_chunk_tokens(filename, "-c");
    }

    static void update_stream_parameters(typename XTC2Traits::StreamParameters& cfg,
                                         const std::array<std::string, VariantCount>& paths) {
      safe_strncpy(cfg.smd_path, paths[0].c_str(), XTC2Traits::MaxNameSize);
      safe_strncpy(cfg.xtc_path, paths[1].c_str(), XTC2Traits::MaxNameSize);
    }

  private:
    /**
     * Return either the stream number or chunk number encoded in a filename.
     *
     * The XTC2 format encodes stream and "chunk" as `-s000` and `-c000`.
     *
     * @param[in] filename The XTC2 filename.
     * @param[in] token_pattern The token search pattern - should be `-s` or `-c`.
     * @returns The stream or chunk identifier pulled from the filename.
     */
    static std::size_t parse_stream_chunk_tokens(std::string_view filename,
                                                 std::string token_pattern) {
      std::size_t id { 0 };

      auto s_pos { filename.find(token_pattern) };
      if (s_pos != std::string_view::npos && s_pos + 5 <= filename.size()) {
        std::size_t tmp { 0 };
        std::size_t start_pos { s_pos + 2 };
        std::size_t end_pos { s_pos + 5 };
        std::from_chars_result res =
            std::from_chars(filename.data() + start_pos, filename.data() + end_pos, tmp);

        if (res.ec == std::errc() && res.ptr == filename.data() + end_pos) {
          id = tmp;
        }
      }

      return id;
    }
  };

  template <>
  struct SingleFileLocatorTraits<XTC2Traits> {
    struct Parameters {
      char path[1024];

      static constexpr auto get_metadata() {
        return std::make_tuple(make_named("path", &Parameters::path));
      }
    };

    /**
     * For a single file, there is only a single stream by definition.
     *
     * @returns A numeric identifier for the stream. Zero by definition in this case.
     */
    static std::size_t id_stream() { return 0; }

    /**
     * For a single file, there is only a single StreamBroker in the sequence.
     *
     * @returns A numeric identifier for the stream's position in a sequence/chain.
     *          Zero by definition in this case.
     */
    static std::size_t id_chain_order() { return 0; }

    static void update_stream_parameters(typename XTC2Traits::StreamParameters& cfg,
                                         Parameters& params) {
      safe_strncpy(cfg.smd_path, params.path, XTC2Traits::MaxNameSize);
      safe_strncpy(cfg.xtc_path, params.path, XTC2Traits::MaxNameSize);
    }
  };
} // namespace sbio

#endif // SBIO_FORMATS_XTC2_XTC2_LOCATOR_HH
