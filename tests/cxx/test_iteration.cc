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

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

namespace {
  using SerialRandomDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::RandomTraits
  >;

  using ThreadedRandomDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::ThreadedExecution,
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
    params.indexing_mode = mode;
    params.indexing_batch_size = batch_size;

    sbio::GenericStreamConfig<sbio::RandomTraits> cfg;
    cfg.format_params = params;

    ASSERT_TRUE(ds.load_source(cfg, detectors, num_detectors)); // Test loading with default Locator
    ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);
  }

  TEST(StepIteration, SerialRangeVisitsEveryStepInOrder) {
    constexpr std::size_t NumEvents { 30 };
    SerialRandomDataSource ds;
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
    SerialRandomDataSource ds;
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

  class ThreadedBreakEarly : public ::testing::TestWithParam<std::size_t> {};

  // Test that threads properly execute - don't mix and match bytes
  // - Also test early thread exit
  TEST_P(ThreadedBreakEarly, OtherThreadsFinish) {
    constexpr std::size_t NumEvents { 400 };
    constexpr std::size_t NumThreads { 8 };

    ThreadedRandomDataSource ds;
    setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, NumEvents, GetParam());

    auto grp = ds.get_stream_group("det0");
    ASSERT_GT(grp.num_segments(), 0u);

    std::atomic<std::size_t> bad { 0 };
    std::atomic<std::size_t> total { 0 };

    auto worker = [&](std::size_t tid) {
      for (auto step : ds.steps()) {
        if (tid == 1 && step > 20) {
          break; // Leave without fetching the step we were handed
        }

        auto arr = grp.get_data(step);

        ssize_t shape[2] { 1, arr.nbytes() };
        ssize_t strides[2] { arr.nbytes(), 1 };

        // NOTE: The data type is uint16, but the rules from randfmt still hold
        //       here for the sequential pattern - byte i of event n is (n + i) & 0xFF
        ncarray::NCArrayView bytes(arr.data(), 2, shape, strides, ncarray::DType::uint8, 0, false);

        const std::uint8_t b0 { bytes[{0, 0}] };
        const std::uint8_t b1 { bytes[{0, 1}] };
        if (b0 != static_cast<std::uint8_t>(step & 0xFF)       ||
            b1 != static_cast<std::uint8_t>((step + 1) & 0xFF)) {
          bad++;
        }

        total++;

        if (tid == 0) {
          break; // Leave after the first step
        }
      }
    };

    std::vector<std::thread> threads;
    for (std::size_t t = 0; t < NumThreads; ++t) {
      threads.emplace_back(worker, t);
    }

    for (auto& th : threads) {
      th.join();
    }

    EXPECT_EQ(bad.load(), 0u) << "steps returned another event's bytes!";
    EXPECT_GE(total.load(), NumEvents - 2) << "at most the two abandoned steps are missing";
  }

  INSTANTIATE_TEST_SUITE_P(BatchSizes, ThreadedBreakEarly, ::testing::Values(2, 3, 7, 50));
} // anonymous namespace
