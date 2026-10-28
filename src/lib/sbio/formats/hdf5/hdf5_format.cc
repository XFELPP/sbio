/*
 * sbio - Stream Broker IO
 *
 * Copyright (C) 2025-2026 Gabriel Dorlhiac
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "sbio/formats/hdf5/hdf5_format.hh"

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <mutex>

namespace sbio {
  std::mutex& hdf5_mutex() {
    static std::mutex h5_m;

    return h5_m;
  }

  spdlog::logger& hdf5_logger() {
    static const std::shared_ptr<spdlog::logger> logger {
      []() {
        spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
        std::shared_ptr<spdlog::logger> existing { spdlog::get("sbio::HDF5") };
        if (!existing) {
          return spdlog::stdout_color_mt("sbio::HDF5");
        } else {
          return existing;
        }
      }()
    };

    return *logger;
  }
} // namespace sbio
