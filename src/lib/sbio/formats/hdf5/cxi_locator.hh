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

#ifndef SBIO_FORMATS_HDF5_CXI_LOCATOR_HH
#define SBIO_FORMATS_HDF5_CXI_LOCATOR_HH

#include "sbio/core/locator.hh"
#include "sbio/formats/hdf5/cxi_spec.hh"
#include "sbio/locators/path_pattern.hh"
#include "sbio/locators/single_file.hh"
#include "sbio/util/parameters.hh"
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

  /**
   * CXI files use a generic file naming pattern of: {directory}/{prefix}[-{tag}].{ext}
   *
   * A secondary constructor constructs a prefix from an experiment and run using
   * the format: `{exp}-{run}` without formatting (e.g. zero padding) on the run
   * number.
   */
  template <>
  struct LocatorParametersFor<PathPatternLocator, CXITraits> {
    struct Type {
      FixedName<1024> directory {};                ///< Root directory for output files
      FixedName<CXITraits::MaxNameSize> prefix {}; ///< The filename prefix
      FixedName<CXITraits::MaxNameSize> tag {};    ///< Additional tag appended to prefix (e.g. sample)
      FixedName<CXITraits::MaxNameSize> ext {};    ///< Filename extension

      Type(std::string_view dir,
           std::string_view prefix_,
           std::string_view tag_ = {},
           std::string_view ext_ = "cxi")
        : directory(dir)
        , prefix(prefix_)
        , tag(tag_)
        , ext(ext_)
      {}

      Type(//std::string_view dir,
           std::string_view exp,
           unsigned run,
           std::string_view tag_ = {},
           std::string_view ext_ = "cxi")
        : Type("", std::string(exp) + "-" + std::to_string(run), tag_, ext_)
        // : Type(dir, std::string(exp) + "-" + std::to_string(run), tag_, ext_)
      {}

      static constexpr auto get_metadata() {
        return std::make_tuple(make_named_member("directory", &Type::directory),
                               make_named_member("prefix", &Type::prefix),
                               make_named_member("tag", &Type::tag),
                               make_named_member("ext", &Type::ext));
      }
    };
  };

  template <>
  struct PathPatternLocatorTraits<CXITraits> {
    using Parameters = LocatorParameters_t<PathPatternLocator, CXITraits>;

    using provides_variants = StreamSet<CXITraits::DataStream>;

    static constexpr std::size_t VariantCount { CXITraits::StreamTypes::size() };

    // Unaware of any env vars used generally for CXI production
    // static constexpr std::array<EnvPatternArg, 0> env_args {{}};

    static constexpr std::array<const char*, VariantCount> role_patterns {{
      "{directory}/{prefix}",
    }};

    /**
     * All files are given stream 0.
     */
    static std::size_t id_stream(std::string_view filename) { return 0; }

    /**
     * Setup the chain order from the trailing numbers before `.cxi`.
     */
    static std::size_t id_chain_order(std::string_view filename) {
      const auto dot { filename.rfind('.') };
      const auto underscore { filename.rfind('_', dot) };

      if (dot == std::string_view::npos        ||
          underscore == std::string_view::npos ||
          underscore + 1 >= dot) {
        return 0;
      }

      std::size_t id { 0 };
      const auto res {
        std::from_chars(filename.data() + underscore + 1, filename.data() + dot, id)
      };
      if (res.ec == std::errc() && res.ptr == filename.data() + dot) {
        return id;
      }

      return 0;
    }

    /**
     * Check whether the filename matches: {prefix}[-tag][_N]{.ext}
     *
     * @note This is ignoring the fact that there may be a VDS (likely the case).
     *       VDS not currently handled - the actual underlying files will be read
     *       in order.
     */
    static std::size_t accepts(std::string_view filename, const Parameters& params) {
      const std::string_view prefix { params.prefix.c_str() };
      const std::string_view tag { params.tag.c_str() };
      const std::string_view ext { params.ext.c_str() };

      std::string_view rest { filename.substr(prefix.size()) };

      // Inspect for [-tag] presence
      if (!tag.empty()) {
        if (rest.size() < tag.size() + 1     ||
            rest[0] != '-'                   ||
            rest.substr(1, tag.size()) != tag) {
          return false;
        }

        rest.remove_prefix(tag.size() + 1);
      }

      // Verify [_N] rank suffix
      if (!rest.empty() && rest[0] == '_') {
        std::size_t n { 1 };
        while (n < rest.size() && rest[n] >= '0' && rest[n] <= '9') {
          ++n;
        }

        if (n == 1) {
          // File with `_` but no number following it.
          return false;
        }

        rest.remove_prefix(n);
      }

      // Check the extension matches
      if (rest.size() == ext.size() + 1 &&
          rest[0] == '.'                &&
          rest.substr(1) == ext) {
        return true;
      }

      return false;
    }
  };

  template <>
  struct LocatorParametersFor<SingleFileLocator, CXITraits> {
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
  struct SingleFileLocatorTraits<CXITraits> {
    using Parameters = LocatorParameters_t<SingleFileLocator, CXITraits>;

    using provides_variants = StreamSet<CXITraits::DataStream>;

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

#endif // SBIO_FORMATS_HDF5_CXI_LOCATOR_HH
