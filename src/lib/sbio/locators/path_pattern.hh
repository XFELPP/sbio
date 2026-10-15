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
#include "sbio/core/stream.hh"

#include <fmt/args.h>
#include <fmt/format.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace sbio {
  struct PathPatternLocator;

  /**
   * A path pattern argument whose value comes from the environment.
   */
  struct EnvPatternArg {
    const char* name;     ///< The argument's name in the pattern (e.g. "root")
    const char* env_var;  ///< The environment variable providing the value
    const char* fallback; ///< The value if the variable is not set
  };

  template <typename FTraits>
  struct PathPatternLocatorTraits {
    using Parameters = LocatorParameters_t<PathPatternLocator, FTraits>;

    static constexpr std::size_t VariantCount { FTraits::StreamTypes::size() };

    static constexpr std::array<const char*, VariantCount> role_patterns {};

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

    /**
     * Pattern arguments taken from the environment.
     *
     * Each entry adds the argument `name` to the role patterns, with the value of the
     * environment variable `env_var` (or `fallback` if unset). The environment is the
     * DataSource's (a Context's overrides, then the process environment).
     *
     * E.g. `{ "root", "SIT_PSDM_DATA", "/sdf/data/lcls/ds" }` allows patterns like
     * `"{root}/{exp:.3}/{exp}/xtc/{exp}-r{run:04d}"`.
     */
    static constexpr std::array<EnvPatternArg, 0> env_args {};

    /**
     * Whether a file matching a role's pattern (as a prefix) belongs to the source.
     *
     * Allows filtering files a prefix cannot, e.g. a run number that follows a
     * part of the filename that is not known in advance.
     *
     * @returns Whether to use the file.
     */
    static bool accepts(std::string_view filename, const Parameters& params) { return true; }
  };

  struct PathPatternLocator {
    template <typename FTraits>
    using LocatorTraits = PathPatternLocatorTraits<FTraits>;

    template <typename FTraits>
    using LocatorParameters = typename LocatorTraits<FTraits>::Parameters;

    template <std::size_t VariantCount>
    using StreamChainMap = std::map<std::size_t, std::array<std::string, VariantCount>>;

    template <std::size_t VariantCount>
    using StreamMap = std::map<std::size_t, StreamChainMap<VariantCount>>;

    template <typename DS>
    static bool find_streams(DS& ds,
                             const LocatorParameters<typename DS::DataFormat>& params,
                             const GenericStreamConfig<typename DS::DataFormat>& base_cfg) {
      using FTraits = typename DS::DataFormat;
      constexpr auto patterns { PathPatternLocatorTraits<FTraits>::role_patterns };

      constexpr std::size_t VariantCount { LocatorTraits<FTraits>::provides_variants::size() };

      // Arguments from the environment (e.g. the data root)
      using LTraits = PathPatternLocatorTraits<FTraits>;
      std::vector<std::pair<std::string, std::string>> env_values;
      if constexpr (requires { LTraits::env_args; }) {
        for (const auto& arg : LTraits::env_args) {
          env_values.emplace_back(arg.name, environment_value(ds, arg.env_var, arg.fallback));
        }
      }

      StreamMap<VariantCount> streams;
      for (std::size_t r = 0; r < VariantCount; ++r) {
        std::string base_pattern = format_parameter_string(patterns[r], params, env_values);

        fs::path prefix_path(base_pattern);
        fs::path parent_dir { prefix_path.parent_path() };
        std::string prefix_filename { prefix_path.filename().string() };

        if (!fs::exists(parent_dir)) {
          continue;
        }

        for (const auto& entry : fs::directory_iterator(parent_dir)) {
          std::string path_str { entry.path().string() };
          std::string filename { entry.path().filename().string() };

          bool accepted { filename.find(prefix_filename) == 0 };
          if constexpr (requires { LTraits::accepts(std::string_view {}, params); }) {
            accepted = accepted && LTraits::accepts(filename, params);
          }

          if (accepted) {
            auto stream_id { PathPatternLocatorTraits<FTraits>::id_stream(filename) };
            auto chain_id { PathPatternLocatorTraits<FTraits>::id_chain_order(filename) };

            streams[stream_id][chain_id][r] = path_str;
          }
        }
      }

      for (const auto& [stream_id, stream_chain_map] : streams) {
        // Have a chain of StreamBrokers that will run sequentially
        for (const auto& [stream_id, stream_chain_map] : streams) {
          // The map is ordered by chain id with the chain's chunks, in order
          std::vector<GenericStreamConfig<FTraits>> chunks;
          for (const auto& [chain_id, role_paths] : stream_chain_map) {
            auto cfg { base_cfg };
            cfg.identity.stream_id = stream_id;
            cfg.identity.chain_id = chain_id;

            for (std::size_t r = 0; r < VariantCount; ++r) {
              // A role without a file in this chunk keeps the previous
              // chunk's (e.g. the single SMD of an XTC2 run)
              cfg.resources[r] =
                role_paths[r].empty()
                  ? StreamResource{}
                  : StreamResource::from_path(role_paths[r].c_str());
            }

            chunks.push_back(cfg);
          }

          ds.add_data_chain(chunks.data(), chunks.size());
        }
      }

      return ds.num_data_streams() > 0;
    }

    template <typename Params>
    static auto format_parameter_string(std::string_view fmt_string,
                                        Params& params,
                                        const std::vector<std::pair<std::string, std::string>>& extra = {}) {
      auto meta { Params::get_metadata() };

      constexpr auto tuple_size { std::tuple_size_v<decltype(meta)> };

      fmt::dynamic_format_arg_store<fmt::format_context> store;
      add_tuple_to_store(params, meta, store, std::make_index_sequence<tuple_size>{});
      for (const auto& [name, value] : extra) {
        store.push_back(fmt::arg(name.c_str(), value));
      }

      return fmt::vformat(fmt_string, store);
    }

    /**
     * An environment variable as seen by a DataSource.
     *
     * Uses the DataSource's environment (e.g. with a Context's overrides) if it has
     * one, otherwise the process environment.
     */
    template <typename DS>
    static std::string environment_value(const DS& ds, const char* key, const char* fallback) {
      if constexpr (requires { { ds.env(key, fallback) } -> std::convertible_to<std::string>; }) {
        return ds.env(key, fallback);
      } else {
        const char* value { std::getenv(key) };
        return value != nullptr ? std::string(value) : std::string(fallback);
      }
    }

  private:
    template <typename Class, typename Tuple, std::size_t... Is>
    static void add_tuple_to_store(const Class& obj,
                                   const Tuple& tup,
                                   fmt::dynamic_format_arg_store<fmt::format_context>& store,
                                   std::index_sequence<Is...>) {
      auto add_to_store = [&](const char* kwarg_name, auto member_ptr) {
        auto val { obj.*member_ptr };
        using ParamType = std::decay_t<decltype(val)>;
        // Ensure it is a string if "stringy" -- may be using FixedNames etc.
        if constexpr (IsStringLike<ParamType>) {
          store.push_back(fmt::arg(kwarg_name, std::string(val)));
        } else {
          store.push_back(fmt::arg(kwarg_name, val));
        }
      };

      ( add_to_store(std::get<Is>(tup).name, std::get<Is>(tup).ptr), ... );
    }
  };
} // namespace sbio

#endif // SBIO_LOCATORS_PATH_PATTERN_HH
