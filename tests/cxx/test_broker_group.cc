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
#include "sbio/core/stream.hh"
#include "sbio/execution/serial.hh"
#include "sbio/formats/random/randfmt.hh"
#include "sbio/formats/random/random_locator.hh"
#include "sbio/formats/random/random_traits.hh"
#include "sbio/io/posix.hh"
#include "sbio/locators/custom_lambda.hh"

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

class BrokerGroupTest : public ::testing::Test {
protected:
  using RandomDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::RandomTraits
  >;
};

TEST_F(BrokerGroupTest, WildcardDetectorMatching) {
  RandomDataSource ds;
  sbio::randfmt::DetectorSpec detectors[10];
  std::uint8_t num_detectors { 2 };

  snprintf(detectors[0].name, sbio::RandomTraits::MaxNameSize, "det0");
  snprintf(detectors[0].type, sbio::RandomTraits::MaxNameSize, "test_det0");
  detectors[0].rank = 2;
  detectors[0].shape[0] = 16;
  detectors[0].shape[1] = 16;
  detectors[0].dtype = ncarray::DType::uint8;

  snprintf(detectors[1].name, sbio::RandomTraits::MaxNameSize, "det1");
  snprintf(detectors[1].type, sbio::RandomTraits::MaxNameSize, "test_det1");
  detectors[1].rank = 2;
  detectors[1].shape[0] = 16;
  detectors[1].shape[1] = 16;
  detectors[1].dtype = ncarray::DType::uint8;

  sbio::RandomTraits::StreamParameters base_cfg;
  base_cfg.num_events = 10;
  base_cfg.pattern_type = 2;
  base_cfg.indexing_mode = sbio::RandomTraits::IndexingMode::IndexAll;

  sbio::GenericStreamConfig<sbio::RandomTraits> cfg;
  cfg.format_params = base_cfg;

  ASSERT_TRUE(ds.load_source_with<sbio::CustomLambdaLocator>(cfg, detectors, num_detectors));
  ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);

  // Wildcard match '*'
  auto wildcard_grp = ds.get_stream_group("*");
  EXPECT_GE(wildcard_grp.num_segments(), 2u);

  // Unknown name search should return empty group (0 segments)
  auto empty_grp = ds.get_stream_group("nonexistent");
  EXPECT_EQ(empty_grp.num_segments(), 0u);
}
