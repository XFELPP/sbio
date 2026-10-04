#include "sbio/core/datasource.hh"
#include "sbio/execution/serial.hh"
#include "sbio/execution/threaded.hh"
#include "sbio/formats/random/randfmt.hh"
#include "sbio/formats/random/random_locator.hh"
#include "sbio/formats/random/random_traits.hh"
#include "sbio/io/posix.hh"

#include <gtest/gtest.h>
#include <ncarray/ncarrays.hh>
#include <ncarray/soarrays.hh>

#include <cstddef>
#include <cstdint>

namespace {
  using RandomDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::RandomTraits
  >;

  template <class DS>
  void setup_datasource(DS& ds,
                        sbio::RandomTraits::IndexingMode mode,
                        std::size_t num_events = 50,
                        std::size_t batch_size = 10) {
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
    params.indexing_mode = sbio::RandomTraits::IndexingMode::IndexBatch;
    params.indexing_batch_size = batch_size;

    sbio::GenericStreamConfig<sbio::RandomTraits> cfg;
    cfg.format_params = params;

    ASSERT_TRUE(ds.load_source(cfg, detectors, num_detectors)); // Test loading with default Locator
    ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);
  }

  TEST(StepIteration, SerialRangeVisitsEveryStepInOrder) {
    constexpr std::size_t NumEvents { 30 };
    RandomDataSource ds;
    setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, NumEvents, 7);

    auto grp = ds.get_stream_group("det0");
    ASSERT_GT(grp.num_segments(), 0u);

    std::size_t expected { 0 };
    for (auto step : ds.steps()) {
      auto arr = grp.get_data(step);

      EXPECT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
      for (ssize_t i = 0; i < 3; ++i) {
        if (i == 0) {
          EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
        } else {
          EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
        }
      }

      EXPECT_EQ(step, expected);
      ++expected;
    }
    EXPECT_EQ(expected, NumEvents);
  }

  TEST(StepIteration, SerialRangeBreakEarly) {
    RandomDataSource ds;
    setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, 30, 7);

    auto grp = ds.get_stream_group("det0");
    ASSERT_GT(grp.num_segments(), 0u);

    std::size_t count { 0 };
    for (auto step : ds.steps()) {
      auto arr = grp.get_data(step);

      EXPECT_EQ(arr.ndim(), 3) << "Unexpected dimensionality!";
      for (ssize_t i = 0; i < 3; ++i) {
        if (i == 0) {
          EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count " << arr.shape(0);
        } else {
          EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect! " << arr.shape(i);
        }
      }

      if (count == 5) {
        break;
      }

      count++;
    }

    EXPECT_EQ(count, 5u);
  }
} // anonymous namespace
