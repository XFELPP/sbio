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

#include <cstdint>
#include <cstddef>
#include <iostream>

class RandomPatternTest : public ::testing::Test {
protected:
  using RandomDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::RandomTraits
  >;

  void setup_datasource(RandomDataSource& ds,
                        std::uint8_t pattern_type,
                        std::size_t num_events = 20,
                        std::uint32_t seed = 42) {
    sbio::RandomTraits::DataSourceParameters ds_params;
    ds_params.num_detectors = 1;
    snprintf(ds_params.detectors[0].name, sbio::RandomTraits::MaxNameSize, "det0");
    snprintf(ds_params.detectors[0].type, sbio::RandomTraits::MaxNameSize, "test_det");

    ds_params.detectors[0].rank = 2;
    ds_params.detectors[0].shape[0] = 64;
    ds_params.detectors[0].shape[1] = 64;
    ds_params.detectors[0].dtype = ncarray::DType::uint16;

    sbio::RandomTraits::StreamParameters base_cfg;
    base_cfg.num_events = num_events;
    base_cfg.pattern_type = pattern_type;
    base_cfg.seed = seed;
    base_cfg.enable_subblock_offsets = true;
    base_cfg.indexing_mode = sbio::RandomTraits::IndexingMode::IndexAll;

    ASSERT_TRUE(ds.load_run(base_cfg, ds_params));
    ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);
  }
};

TEST_F(RandomPatternTest, FixedPatternFill) {
  RandomDataSource ds;
  setup_datasource(ds,
                   /* (fixed) pattern_type = */ 2,
                   5);

  auto grp = ds.get_stream_group("det0");
  ASSERT_GT(grp.num_segments(), 0u);
  for (auto step = ds.next(); step < 5; step = ds.next()) {
    auto arr = grp.get_data(step);

    EXPECT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 64) << "Segment size is incorrect!";
      }
    }

    // Don't use ncarray conversion since we don't want casts. Just make a new view from
    // the data pointer.
    ssize_t shape[2]   { 1, arr.nbytes() };
    ssize_t strides[2] { arr.nbytes(), 1 };

    ncarray::NCArrayView bytes_arr(arr.data(),
                                   2,
                                   shape,
                                   strides,
                                   ncarray::DType::uint8,
                                   0,
                                   false);
    std::size_t total_bytes { bytes_arr.nbytes() };

    for (std::size_t i = 0; i < total_bytes; ++i) {
      std::uint8_t& val { bytes_arr[{0, i}] };
      EXPECT_EQ(val, 0x5A) << "Mismatch at byte " << i << " on step " << step;
    }
  }
}

TEST_F(RandomPatternTest, SequentialPatternFill) {
  RandomDataSource ds;
  setup_datasource(ds,
                   /* (sequential) pattern_type = */1,
                   5);

  auto grp = ds.get_stream_group("det0");
  ASSERT_GT(grp.num_segments(), 0u);
  for (auto step = ds.next(); step < 5; step = ds.next()) {
    auto arr = grp.get_data(step);

    EXPECT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 64) << "Segment size is incorrect!";
      }
    }

    // Don't use ncarray conversion since we don't want casts. Just make a new view from
    // the data pointer.
    ssize_t shape[2]   { 1, arr.nbytes() };
    ssize_t strides[2] { arr.nbytes(), 1 };

    ncarray::NCArrayView bytes_arr(arr.data(),
                                   2,
                                   shape,
                                   strides,
                                   ncarray::DType::uint8,
                                   0,
                                   false);
    std::size_t total_bytes { bytes_arr.nbytes() };

    for (std::size_t i = 0; i < total_bytes; ++i) {
      std::uint8_t expected { static_cast<std::uint8_t>((step + i) & 0xFF) };
      std::uint8_t& val { bytes_arr[{0, i}] };
      EXPECT_EQ(val, expected) << "Sequential byte mismatch at " << i << " step " << step;
    }
  }
}

TEST_F(RandomPatternTest, PRNGPatternFillDeterminism) {
  RandomDataSource ds;
  std::uint32_t seed { 12345 };
  setup_datasource(ds,
                   /* (PRNG) pattern_type = */ 0,
                   5,
                   seed);
  auto grp = ds.get_stream_group("det0");
  ASSERT_GT(grp.num_segments(), 0u);

  for (auto step = ds.next(); step < 5; step = ds.next()) {
    auto arr = grp.get_data(step);

    EXPECT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 64) << "Segment size is incorrect!";
      }
    }

    // Don't use ncarray conversion since we don't want casts. Just make a new view from
    // the data pointer.
    ssize_t shape[2]   { 1, arr.nbytes() };
    ssize_t strides[2] { arr.nbytes(), 1 };

    ncarray::NCArrayView bytes_arr(arr.data(),
                                   2,
                                   shape,
                                   strides,
                                   ncarray::DType::uint8,
                                   0,
                                   false);
    std::size_t total_bytes { bytes_arr.nbytes() };

    // Verify LCG output formula: state = state * 1664525 + 1013904223
    std::uint32_t state { seed ^ static_cast<std::uint32_t>(step * 0x9E3779B9u + 1) };
    for (std::size_t i = 0; i < total_bytes; ++i) {
      state = state * 1664525u + 1013904223u;
      std::uint8_t expected { static_cast<std::uint8_t>(state >> 24) };
      std::uint8_t& val { bytes_arr[{0, i}] };
      EXPECT_EQ(val, expected) << "PRNG byte mismatch at index " << i << " step " << step;
    }
  }
}
