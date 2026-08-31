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

#ifndef SBIO_LOCATORS_CUSTOM_LAMBDA_HH
#define SBIO_LOCATORS_CUSTOM_LAMBDA_HH

#include "sbio/core/locator.hh"

#include <fmt/args.h>
#include <fmt/format.h>

#include <array>
#include <cstddef>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace fs = std::filesystem;

namespace sbio {
  struct CustomLambdaLocator;

  template <typename FTraits>
  struct CustomLambdaLocatorTraits {
    using Parameters = LocatorParameters_t<CustomLambdaLocator, FTraits>;

    static constexpr auto finder_lambda =
      []<typename DS>(DS& ds,
                      const Parameters& params,
                      const typename DS::DataFormat::StreamParameters& base_cfg) {
      return ds.num_data_streams() > 0;
    };

    /**
     * Given a filename, determine the stream identifier number.
     *
     * If the format allows for multiple streams, it must assign a numeric value
     * to them, even if it is not encoded directly in the filename. Additionally,
     * if there are multiple stream roles which must be grouped together under a
     * single StreamBroker for processing, the stream identifiers must match for
     * the roles that will be grouped.
     *
     * @returns A numeric identifier for the stream. This is unique within a single
     *          role, but different roles may, and should, have streams with the
     *          same identifier when they must be grouped together.
     */
    static std::size_t id_stream() { return 0; }

    /**
     * Given a filename, determine the stream's order in a processing chain.
     *
     * Some formats allow, or require, streams to be processed in sequence. The
     * chain identifier positions a given stream in the correct order of the
     * processing sequence. E.g., chain id 0 will be processed, then chain id 1,
     * and so on. As with the stream identifiers, if there are multiple roles that
     * must be grouped together, the streams corresponding to different roles must
     * have matched chain identifiers to ensure proper grouping.
     *
     * @returns A numeric identifier for the stream's position in a sequence/chain.
     */
    static std::size_t id_chain_order() { return 0; }

    static void update_stream_parameters(typename FTraits::StreamParameters& cfg) {}
  };

  struct CustomLambdaLocator {
    template <typename FTraits>
    using LocatorTraits = CustomLambdaLocatorTraits<FTraits>;

    template <typename FTraits>
    using LocatorParameters = typename LocatorTraits<FTraits>::Parameters;

    template <typename DS>
    static bool find_streams(DS& ds,
                             const LocatorParameters<typename DS::DataFormat>& params,
                             const typename DS::DataFormat::StreamParameters& base_cfg) {
      using FTraits = typename DS::DataFormat;

      return LocatorTraits<FTraits>::finder_lam(ds, params, base_cfg);
    }
  };
} // namespace sbio

#endif // SBIO_LOCATORS_CUSTOM_LAMBDA_HH
