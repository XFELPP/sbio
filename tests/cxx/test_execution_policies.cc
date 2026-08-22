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
#include "sbio/execution/threaded.hh"
#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi.hh"
#include "sbio/execution/mpi_threaded.hh"
#endif
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
#include <utility>

template <typename PolicyPair>
class ExecutionPolicyTest : public ::testing::Test {
protected:
  using IOPolicy = typename PolicyPair::first_type;
  using EPolicy  = typename PolicyPair::second_type;

  using DS = sbio::DataSource<IOPolicy, EPolicy, sbio::RandomTraits>;
};

using PolicyTypes = ::testing::Types<
    std::pair<sbio::SyncPOSIXIO, sbio::SerialExecution>
  , std::pair<sbio::SyncPOSIXIO, sbio::ThreadedExecution>
#ifdef SBIO_HAS_MPI
  , std::pair<sbio::SyncPOSIXIO, sbio::MPIExecution>
  , std::pair<sbio::SyncPOSIXIO, sbio::MPIThreadedExecution>
#endif
>;

TYPED_TEST_SUITE(ExecutionPolicyTest, PolicyTypes);

TYPED_TEST(ExecutionPolicyTest, StreamBrokerPipeline) {
  using DS = typename TestFixture::DS;
  DS ds;

  sbio::RandomTraits::DataSourceParameters ds_params;
  ds_params.num_detectors = 2;
  snprintf(ds_params.detectors[0].name, sbio::RandomTraits::MaxNameSize, "det0");
  snprintf(ds_params.detectors[0].type, sbio::RandomTraits::MaxNameSize, "test_det0");
  ds_params.detectors[0].rank = 2;
  ds_params.detectors[0].shape[0] = 128;
  ds_params.detectors[0].shape[1] = 128;
  ds_params.detectors[0].dtype = ncarray::DType::uint16;

  snprintf(ds_params.detectors[1].name, sbio::RandomTraits::MaxNameSize, "det1");
  snprintf(ds_params.detectors[1].type, sbio::RandomTraits::MaxNameSize, "test_det1");
  ds_params.detectors[1].rank = 1;
  ds_params.detectors[1].shape[0] = 512;
  ds_params.detectors[1].dtype = ncarray::DType::float32;

  sbio::RandomTraits::StreamParameters base_cfg;
  base_cfg.num_events = 25;
  base_cfg.pattern_type = 1; // Sequential

  ASSERT_TRUE(ds.load_run(base_cfg, ds_params));
  ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);

  auto grp0 = ds.get_stream_group("det0");
  ASSERT_GT(grp0.num_segments(), 0u);

  auto grp1 = ds.get_stream_group("det1");
  ASSERT_GT(grp1.num_segments(), 0u);

  std::size_t processed { 0 };
  for (auto step = ds.next(); step < 25; step = ds.next()) {
    auto arr0 = grp0.get_data(step);
    auto arr1 = grp1.get_data(step);

    EXPECT_EQ(arr0.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr0.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr0.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr0.shape(i), 128) << "Segment size is incorrect!";
      }
    }

    EXPECT_EQ(arr1.ndim(), 2) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr1.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr1.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr1.shape(i), 512) << "Segment size is incorrect!";
      }
    }

    processed++;
  }

  EXPECT_EQ(processed, 25u);
}
