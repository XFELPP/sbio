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

#ifndef SBIO_FORMATS_XTC1_XTC1_LOCATOR_HH
#define SBIO_FORMATS_XTC1_XTC1_LOCATOR_HH

#include "sbio/core/locator.hh"
#include "sbio/formats/xtc1/xtc1_traits.hh"
#include "sbio/locators/path_pattern.hh"
#include "sbio/locators/single_file.hh"
#include "sbio/util/parameters.hh"
#include "sbio/util/string.hh"

#include <array>
#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>
#include <tuple>

namespace sbio {
  // --- Specializations for locator traits --- //

  template <>
  struct LocatorParametersFor<PathPatternLocator, XTC1Traits> {
    struct Type {
      FixedName<XTC1Traits::MaxNameSize> experiment;
      unsigned run;

      Type(std::string_view exp, unsigned run_)
        : experiment(exp)
        , run(run_)
      {}

      static constexpr auto get_metadata() {
        return std::make_tuple(make_named_member("exp", &Type::experiment),
                               make_named_member("run", &Type::run));
      }
    };
  };

  template <>
  struct PathPatternLocatorTraits<XTC1Traits> {
    using Parameters = LocatorParameters_t<PathPatternLocator, XTC1Traits>;

    using provides_variants = StreamSet<XTC1Traits::SMD, XTC1Traits::BD>;

    static constexpr std::size_t VariantCount { XTC1Traits::StreamTypes::size() };

    /**
     * Streams at or above this number are written by IOC recorders.
     *
     * IOC streams hold parts of events, which must be merged by time into the DAQ
     * streams' events. That is not supported, so they are not used.
     */
    static constexpr std::size_t FirstIOCStream { 80 };

    // The data root is SIT_PSDM_DATA (the Context's override, or the process environment)
    static constexpr std::array<EnvPatternArg, 1> env_args {{
      { "root", "SIT_PSDM_DATA", "/sdf/data/lcls/ds" }
    }};

    /**
     * XTC1 files are named using the following pattern:
     *   {exp}-r{run:04d}-s{ss}-c{cc}.xtc (or .smd.xtc for smalldata files)
     * There is an additional set that seems to be named with a number:
     *   e{N}-r{run:04d}-s{ss}-c{cc}.xtc
     * This latter pattern is an exception, not the rule. Maybe xpptut data?
     *   - The run can be checked by the `accepts` function using this pattern.
     *
     * @todo Setup support for multiple sets of patterns to account for these two
     *       possibilities.
     */
    static constexpr std::array<const char*, VariantCount> role_patterns {{
      "{root}/{exp:.3}/{exp}/xtc/smalldata/{exp}-r{run:04d}",
      "{root}/{exp:.3}/{exp}/xtc/{exp}-r{run:04d}"

      // The following patterns are waiting on multiple pattern set support:
      // "{root}/{exp:.3}/{exp}/xtc/smalldata/e",
      // "{root}/{exp:.3}/{exp}/xtc/e"
    }};

    static std::size_t id_stream(std::string_view filename) {
      return parse_token(filename, "-s");
    }

    static std::size_t id_chain_order(std::string_view filename) {
      return parse_token(filename, "-c");
    }

    /**
     * Only the run's complete files (not `.inprogress`) of DAQ streams.
     */
    static bool accepts(std::string_view filename, const Parameters& params) {
      char run_token[16];
      std::snprintf(run_token, sizeof(run_token), "-r%04u-s", params.run);

      constexpr std::string_view ext { ".xtc" };
      const bool is_xtc {
        filename.size() > ext.size() && filename.substr(filename.size() - ext.size()) == ext
      };

      return is_xtc &&
             filename.find(run_token) != std::string_view::npos &&
             id_stream(filename) < FirstIOCStream;
    }

  private:
    /**
     * Return the number following a token in a filename.
     *
     * XTC1 encodes the stream and "chunk" as `-s00` and `-c00`.
     *
     * @param[in] filename The XTC1 filename.
     * @param[in] token The token - `-s` or `-c`.
     * @returns The number after the token (0 if there is none).
     */
    static std::size_t parse_token(std::string_view filename, std::string_view token) {
      std::size_t id { 0 };

      auto pos { filename.find(token) };
      if (pos == std::string_view::npos) {
        return id;
      }

      for (pos += token.size(); pos < filename.size(); ++pos) {
        const char c { filename[pos] };
        if (c < '0' || c > '9') {
          break;
        }
        id = id * 10 + static_cast<std::size_t>(c - '0');
      }

      return id;
    }
  };

 template <>
  struct LocatorParametersFor<SingleFileLocator, XTC1Traits> {
    struct Type {
      FixedName<1024> path;

      Type(std::string_view path_)
        : path(path_)
     {}

      static constexpr auto get_metadata() {
        return std::make_tuple(make_named_member("path", &Type::path));
      }
    };
  };

  template <>
  struct SingleFileLocatorTraits<XTC1Traits> {
    using Parameters = LocatorParameters_t<SingleFileLocator, XTC1Traits>;

    // An XTC1 file is read as the indexed stream
    // In this case, L1Accepts without an offset into another file are read in place
    using provides_variants = StreamSet<XTC1Traits::SMD>;

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
  };
} // namespace sbio

#endif // SBIO_FORMATS_XTC1_XTC1_LOCATOR_HH
