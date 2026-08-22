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

#include "sbio/core/datasource.hh"
#include "sbio/core/execution.hh"
#include "sbio/execution/serial.hh"
#include "sbio/execution/threaded.hh"
#include "sbio/formats/random/random_traits.hh"
#include "sbio/io/posix.hh"

#include <ncarray/ncarrays.hh>

#include <iostream>
#include <cassert>

int main(int argc, char* argv[]) {
  std::cout << "=== Testing sbio with RandomTraits ===" << std::endl;

  using RandomDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    // sbio::ThreadedExecution,
    sbio::SerialExecution,
    sbio::RandomTraits
  >;

  RandomDataSource ds;

  sbio::RandomTraits::DataSourceParameters ds_params;
  ds_params.num_detectors = 1;
  snprintf(ds_params.detectors[0].name, sbio::RandomTraits::MaxNameSize, "test");
  snprintf(ds_params.detectors[0].type, sbio::RandomTraits::MaxNameSize, "test_type");
  ds_params.detectors[0].rank = 2;
  ds_params.detectors[0].shape[0] = 512;
  ds_params.detectors[0].shape[1] = 512;
  ds_params.detectors[0].dtype = ncarray::DType::uint16;

  sbio::RandomTraits::StreamParameters base_cfg;
  base_cfg.num_events = 500;
  base_cfg.pattern_type = 2;
  base_cfg.enable_subblock_offsets = true;

  // Can use IndexAll, IndexBatch, or NoIndex.
  // For IndexBatch, set base_cfg.indexing_batch_size as well
  base_cfg.indexing_mode = sbio::RandomTraits::IndexingMode::IndexAll;
  base_cfg.indexing_batch_size = 100;

  bool created { ds.load_run(base_cfg, ds_params) };
  assert(created && "Failed to create random stream brokers");

  if (created) {
    std::cout << "[Pass] Created " << ds.num_data_streams() << " stream broker(s)." << std::endl;

    sbio::IOStatus status = ds.discover_metadata();
    if (status == sbio::IOStatus::Success) {
      std::cout << "[Pass] Was able to discover metadata." << std::endl;

      auto grp { ds.get_stream_group("test") };
      std::cout << "[Pass] Retrieved stream group for `test`." << std::endl;

      for (auto step = ds.next(); step < 500; step = ds.next()) {
        auto cb = [&](sbio::RandomTraits::DataResult res) {
          if (step % 20 == 0) {
            std::cout << "Processed step " << step << std::endl;
          }
        };

        auto arr = grp.get_data(step, cb);

        if (step % 20 == 0) {
          std::uint16_t& raw_val = arr[{0, 102, 102}];

          std::cout << "Retrieved data: " << arr.repr() << std::endl;

          std::cout << "Array[0, 102, 102] = " << raw_val << std::endl;
        }
      }
    } else {
      std::cout << "[FAIL] discover_metadata failed!" << std::endl;
    }
  }

  return 0;
}
