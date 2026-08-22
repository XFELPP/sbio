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
#include "sbio/execution/serial.hh"
#include "sbio/formats/random/random_traits.hh"
#include "sbio/io/posix.hh"

#include <gtest/gtest.h>
#include <ncarray/ncarrays.hh>
#include <ncarray/soarrays.hh>

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

#include <cstddef>
#include <cstdint>

class IndexingModeTest : public ::testing::Test {
protected:
  using RandomDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::RandomTraits
  >;

  void setup_datasource(RandomDataSource& ds,
                        sbio::RandomTraits::IndexingMode mode,
                        std::size_t num_events = 50,
                        std::size_t batch_size = 10) {
    sbio::RandomTraits::DataSourceParameters ds_params;
    ds_params.num_detectors = 1;
    snprintf(ds_params.detectors[0].name, sbio::RandomTraits::MaxNameSize, "det0");
    snprintf(ds_params.detectors[0].type, sbio::RandomTraits::MaxNameSize, "test_det");

    ds_params.detectors[0].rank = 2;
    ds_params.detectors[0].shape[0] = 32;
    ds_params.detectors[0].shape[1] = 32;
    ds_params.detectors[0].dtype = ncarray::DType::uint16;

    sbio::RandomTraits::StreamParameters base_cfg;
    base_cfg.num_events = num_events;
    base_cfg.pattern_type = 2; // Fixed
    base_cfg.indexing_mode = mode;
    base_cfg.indexing_batch_size = batch_size;

    ASSERT_TRUE(ds.load_run(base_cfg, ds_params));
    ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);
  }
};

TEST_F(IndexingModeTest, IndexAllModeFullTraverse) {
  RandomDataSource ds;
  setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexAll, 30);

  auto grp = ds.get_stream_group("det0");
  ASSERT_GT(grp.num_segments(), 0u);

  std::size_t count { 0 };
  for (auto step = ds.next(); step < 30; step = ds.next()) {
    auto arr = grp.get_data(step);

    EXPECT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
      }
    }

    count++;
  }
  EXPECT_EQ(count, 30u);
}

TEST_F(IndexingModeTest, IndexBatchModeReindexing) {
  RandomDataSource ds;
  setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, 50, 10);

  auto grp = ds.get_stream_group("det0");
  std::size_t count { 0 };
  for (auto step = ds.next(); step < 50; step = ds.next()) {
    auto arr = grp.get_data(step);

    EXPECT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
      }
    }

    count++;
  }
  EXPECT_EQ(count, 50u);
}

TEST_F(IndexingModeTest, NoIndexModeLinearTraversal) {
  RandomDataSource ds;
  setup_datasource(ds, sbio::RandomTraits::IndexingMode::NoIndex, 20);

  auto grp = ds.get_stream_group("det0");
  std::size_t count = 0;
  for (auto step = ds.next(); step < 20; step = ds.next()) {
    auto arr = grp.get_data(step);

    EXPECT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
      }
    }

    count++;
  }
  EXPECT_EQ(count, 20u);
}
