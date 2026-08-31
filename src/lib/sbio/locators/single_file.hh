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

#ifndef SBIO_LOCATORS_SINGLE_FILE_HH
#define SBIO_LOCATORS_SINGLE_FILE_HH

#include "sbio/core/locator.hh"
#include "sbio/core/stream.hh"

#include <array>
#include <cstddef>
#include <tuple>

namespace sbio {
  struct SingleFileLocator;

  template <typename FTraits>
  struct SingleFileLocatorTraits {
    using Parameters = LocatorParameters_t<SingleFileLocator, FTraits>;

    static constexpr std::size_t VariantCount { FTraits::StreamTypes::size() };

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

  struct SingleFileLocator {
    template <typename FTraits>
    using LocatorTraits = SingleFileLocatorTraits<FTraits>;

    template <typename FTraits>
    using LocatorParameters = typename LocatorTraits<FTraits>::Parameters;

    template <typename DS>
    static bool find_streams(DS& ds,
                             const GenericStreamConfig<typename DS::DataFormat>& base_cfg,
                             const char* filepath) {
      using FTraits = typename DS::DataFormat;
      auto cfg { base_cfg };

      for (std::size_t r = 0; r < cfg.VariantCount; ++r) {
        cfg.resources[r] = StreamResource::from_path(filepath);
      }

      ds.add_data_stream(cfg);
      return true;
    }
  };
} // namespace sbio

#endif // SBIO_LOCATORS_SINGLE_FILE_HH
