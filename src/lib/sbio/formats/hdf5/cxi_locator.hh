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

  template <>
  struct LocatorParametersFor<PathPatternLocator, CXITraits> {
    // TODO: Just do a single prefix? Specialize by facility/experiment?
    struct Type {
      FixedName<CXITraits::MaxNameSize> experiment;
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
  struct PathPatternLocatorTraits<CXITraits> {
    using Parameters = LocatorParameters_t<PathPatternLocator, CXITraits>;

    using provides_variants = StreamSet<CXITraits::DataStream>;

    static constexpr std::size_t VariantCount { CXITraits::StreamTypes::size() };

    // TODO: Need to setup real ones...
    static constexpr std::array<EnvPatternArg, 1> env_args {{
      { "root", "SIT_PSDM_DATA", "/sdf/data/lcls/ds" }
    }};

    static constexpr std::array<const char*, VariantCount> role_patterns {{
      "{root}/{exp:.3}/{exp}/xtc/smalldata/{exp}-r{run:04d}",
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
      std::size_t end { dot == std::string_view::npos ? filename.size() : dot };
      std::size_t begin { end };
      while (begin > 0 && filename[begin - 1] >= '0' && filename[begin - 1] <= '9') {
        --begin;
      }

      std::size_t id { 0 };
      std::from_chars(filename.data() + begin, filename.data() + end, id);
      return id;
    }

    static std::size_t accepts(std::string_view filename, const Parameters&) {
      return filename.ends_with(".cxi");
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
