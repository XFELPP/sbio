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

#ifndef SBIO_LOCATORS_PATH_PATTERN_HH
#define SBIO_LOCATORS_PATH_PATTERN_HH

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
  template <typename FTraits>
  struct PathPatternLocatorTraits {
    struct Parameters {};

    static constexpr std::array<const char*, FTraits::RoleCount> role_patterns {};

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
    static std::size_t id_stream(std::string_view filename) { return 0; }

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
    static std::size_t id_chain_order(std::string_view filename) { return 0; }

    static void update_stream_parameters(typename FTraits::StreamParameters& cfg,
                                         std::array<const char*, FTraits::RoleCount>& paths) {}
  };

  struct PathPatternLocator {
    template <typename FTraits>
    using LocatorTraits = PathPatternLocatorTraits<FTraits>;

    template <typename FTraits>
    using LocatorParameters = typename LocatorTraits<FTraits>::Parameters;

    template <std::size_t RoleCount>
    using StreamChainMap = std::map<std::size_t, std::array<std::string, RoleCount>>;

    template <std::size_t RoleCount>
    using StreamMap = std::map<std::size_t, StreamChainMap<RoleCount>>;

    template <typename DS>
    static bool find_streams(DS& ds,
                             const LocatorParameters<typename DS::DataFormat>& params,
                             const typename DS::DataFormat::StreamParameters& base_cfg) {
      using FTraits = typename DS::DataFormat;
      constexpr auto patterns { PathPatternLocatorTraits<FTraits>::role_patterns };

      StreamMap<FTraits::RoleCount> streams;
      for (std::size_t r = 0; r < FTraits::RoleCount; ++r) {
        std::string base_pattern = format_parameter_string(patterns[r], params);

        fs::path prefix_path(base_pattern);
        fs::path parent_dir { prefix_path.parent_path() };
        std::string prefix_filename { prefix_path.filename().string() };

        if (!fs::exists(parent_dir)) {
          continue;
        }

        for (const auto& entry : fs::directory_iterator(parent_dir)) {
          std::string path_str { entry.path().string() };
          std::string filename { entry.path().filename().string() };

          if (filename.find(prefix_filename) == 0) {
            auto stream_id { PathPatternLocatorTraits<FTraits>::id_stream(filename) };
            auto chain_id { PathPatternLocatorTraits<FTraits>::id_chain_order(filename) };

            streams[stream_id][chain_id][r] = path_str;
          }
        }
      }

      for (const auto& [stream_id, stream_chain_map] : streams) {
        if (stream_chain_map.size() == 1) {
          // Have one single StreamBroker to construct
          typename FTraits::StreamParameters cfg { base_cfg };
          const auto& role_paths { stream_chain_map.begin()->second };

          LocatorTraits<FTraits>::update_stream_parameters(cfg, role_paths);

          ds.add_data_stream(cfg);
        } else {
          // Have a chain of StreamBrokers that will run sequentially
          // TODO: Need to work out the StreamBrokerChain mechanism.
        }
      }

      return ds.num_data_streams() > 0;
    }

    template <typename Params>
    static auto format_parameter_string(std::string_view fmt_string, Params& params) {
      auto meta { Params::get_metadata() };

      constexpr auto tuple_size { std::tuple_size_v<decltype(meta)> };

      fmt::dynamic_format_arg_store<fmt::format_context> store;
      add_tuple_to_store(params, meta, store, std::make_index_sequence<tuple_size>{});

      return fmt::vformat(fmt_string, store);
    }

  private:
    template <typename Class, typename Tuple, std::size_t... Is>
    static void add_tuple_to_store(const Class& obj,
                                   const Tuple& tup,
                                   fmt::dynamic_format_arg_store<fmt::format_context>& store,
                                   std::index_sequence<Is...>) {
      auto add_to_store = [&](const char* kwarg_name, auto member_ptr) {
        store.push_back(fmt::arg(kwarg_name, obj.*member_ptr));
      };

      ( add_to_store(std::get<Is>(tup).name, std::get<Is>(tup).ptr), ... );
    }
  };
} // namespace sbio

#endif // SBIO_LOCATORS_PATH_PATTERN_HH
