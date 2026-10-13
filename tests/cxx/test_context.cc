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

#include "sbio/core/context.hh"
#include "sbio/execution/serial.hh"
#include "sbio/execution/threaded.hh"
#ifdef SBIO_HAS_CUDA
#include "sbio/execution/cuda.hh"
#endif
#include "sbio/formats/random/randfmt.hh"
#include "sbio/formats/random/random_locator.hh"
#include "sbio/formats/random/random_traits.hh"
#include "sbio/io/posix.hh"

#include <gtest/gtest.h>
#include <ncarray/ncarrays.hh>
#include <ncarray/soarrays.hh>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace {
  // Ensure support-level for threaded tests
  static_assert(sbio::Context<
                  sbio::SyncPOSIXIO,
                  sbio::SerialExecution
                >::supports(sbio::ParallelizationMethods::THREADS));

  template <class DS>
  void setup_datasource(DS& ds,
                        sbio::RandomTraits::IndexingMode mode,
                        std::size_t num_events = 50,
                        std::size_t batch_size = 10,
                        std::size_t max_batch_size = 4) {
    sbio::randfmt::DetectorSpec detectors[10];
    std::uint8_t num_detectors { 1 };

    snprintf(detectors[0].name, sbio::RandomTraits::MaxNameSize, "det0");
    snprintf(detectors[0].type, sbio::RandomTraits::MaxNameSize, "test_det");

    detectors[0].rank = 2;
    detectors[0].shape[0] = 32;
    detectors[0].shape[1] = 32;
    detectors[0].dtype = ncarray::DType::uint16;

    sbio::RandomTraits::StreamParameters params;
    params.num_events = num_events;
    params.pattern_type = 1; // Sequential
    params.indexing_mode = mode;
    params.indexing_batch_size = batch_size;

    sbio::GenericStreamConfig<sbio::RandomTraits> cfg;
    cfg.format_params = params;
    cfg.max_batch_size = max_batch_size;

    ASSERT_TRUE(ds.load_source(cfg, detectors, num_detectors)); // Test loading with default Locator
    ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);
  }
} // anonymous namespace

TEST(Context, CreatesConfiguredDataSources) {
  sbio::Context<sbio::SyncPOSIXIO, sbio::SerialExecution> ctx;
  ASSERT_TRUE(ctx.ok()) << ctx.error();
  EXPECT_TRUE(ctx.error().empty());

  auto ds_a = ctx.data_source<sbio::RandomTraits>();
  setup_datasource(ds_a, sbio::RandomTraits::IndexingMode::IndexBatch, 10, 4);

  auto ds_b = ctx.data_source<sbio::RandomTraits>();
  setup_datasource(ds_b, sbio::RandomTraits::IndexingMode::IndexBatch, 7, 4);

  auto grp_a = ds_a.get_stream_group("det0");
  ASSERT_GT(grp_a.num_segments(), 0u);

  std::size_t seen_a { 0 };
  for (auto step : ds_a.steps()) {
    auto arr = grp_a.get_data(step);
    ASSERT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
      }
    }

    ++seen_a;
  }

  EXPECT_EQ(seen_a, 10u);

  auto grp_b = ds_b.get_stream_group("det0");
  ASSERT_GT(grp_b.num_segments(), 0u);

  std::size_t seen_b { 0 };
  for (auto step : ds_b.steps()) {
    auto arr = grp_b.get_data(step);
    ASSERT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
      }
    }

    ++seen_b;
  }

  EXPECT_EQ(seen_b, 7u);
}

TEST(Context, ThreadedPolicy) {
  sbio::ThreadedExecution::Config ecfg;
  sbio::Context<sbio::SyncPOSIXIO, sbio::ThreadedExecution> ctx(ecfg);
  ASSERT_TRUE(ctx.ok()) << ctx.error();

  auto ds = ctx.data_source<sbio::RandomTraits>();
  setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, 12, 4);

  auto grp = ds.get_stream_group("det0");
  ASSERT_GT(grp.num_segments(), 0u);

  std::size_t seen { 0 };
  for (auto step : ds.steps()) {
    auto arr = grp.get_data(step);
    ASSERT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
      }
    }

    ++seen;
  }

  EXPECT_EQ(seen, 12u);
}

TEST(Context, OnePerExecutionPolicy) {
  {
    sbio::Context<sbio::SyncPOSIXIO, sbio::SerialExecution> first;
    ASSERT_TRUE(first.ok());

    sbio::Context<sbio::SyncPOSIXIO, sbio::SerialExecution> second;
    EXPECT_FALSE(second.ok());
    EXPECT_FALSE(second.error().empty());

    // A different policy is independent
    sbio::Context<sbio::SyncPOSIXIO, sbio::ThreadedExecution> other;
    EXPECT_TRUE(other.ok()) << other.error();
  }

  // Once the first is gone, a new one is fine
  sbio::Context<sbio::SyncPOSIXIO, sbio::SerialExecution> again;
  EXPECT_TRUE(again.ok()) << again.error();
}

TEST(Context, EnvironmentOverrides) {
  sbio::Context<sbio::SyncPOSIXIO, sbio::SerialExecution> ctx;
  ASSERT_TRUE(ctx.ok());

  ::setenv("SBIO_TEST_CONTEXT_VAR", "from_process", 1);
  EXPECT_TRUE(ctx.has_env("SBIO_TEST_CONTEXT_VAR"));
  EXPECT_EQ(ctx.env("SBIO_TEST_CONTEXT_VAR"), "from_process");

  // Creating overrides is confined only to hte Context -- doesnt affect process env
  ctx.set_env("SBIO_TEST_CONTEXT_VAR", "from_context");
  EXPECT_EQ(ctx.env("SBIO_TEST_CONTEXT_VAR"), "from_context");
  EXPECT_STREQ(std::getenv("SBIO_TEST_CONTEXT_VAR"), "from_process");

  // Double check fallback works
  EXPECT_FALSE(ctx.has_env("SBIO_TEST_CONTEXT_UNSET"));
  EXPECT_EQ(ctx.env("SBIO_TEST_CONTEXT_UNSET", "fallback"), "fallback");
  ::unsetenv("SBIO_TEST_CONTEXT_VAR");
}

#ifdef SBIO_HAS_CUDA
TEST(Context, CudaDeviceMustExist) {
  // Cannot create a Context with bad configruation (non-existant GPU)
  sbio::CudaExecution::Config bad;
  bad.device = 1000;
  {
    sbio::Context<sbio::SyncPOSIXIO, sbio::CudaExecution> ctx(bad);
    EXPECT_FALSE(ctx.ok());
    EXPECT_NE(ctx.error().find("device"), std::string::npos) << ctx.error();
  }

  // Can still reconfigure afterwards
  sbio::Context<sbio::SyncPOSIXIO, sbio::CudaExecution> ctx;
  ASSERT_TRUE(ctx.ok()) << ctx.error();

  auto ds = ctx.data_source<sbio::RandomTraits>();
  setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, 9, 4);

  auto grp = ds.get_stream_group("det0");
  ASSERT_GT(grp.num_segments(), 0u);

  std::size_t seen { 0 };
  for (auto step : ds.steps()) {
    auto arr = grp.get_data(step);
    ASSERT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
    for (ssize_t i = 0; i < arr.ndim(); ++i) {
      if (i == 0) {
        EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
      } else {
        EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
      }
    }

    ++seen;
  }

  EXPECT_EQ(seen, 9u);
}
#endif // SBIO_HAS_CUDA
