#include "sbio/core/datasource.hh"
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
    params.indexing_mode = sbio::RandomTraits::IndexingMode::IndexBatch;
    params.indexing_batch_size = batch_size;

    sbio::GenericStreamConfig<sbio::RandomTraits> cfg;
    cfg.format_params = params;

    ASSERT_TRUE(ds.load_source(cfg, detectors, num_detectors)); // Test loading with default Locator
    ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);
  }

  TEST(ThreadedIterationState, TwoDataSourcesIterateIndependently) {
    constexpr std::size_t NumEvents { 50 };

    // Both DS will look at same data, independently
    ThreadedRandomDataSource ds_a;
    setup_datasource(ds_a, sbio::RandomTraits::IndexingMode::IndexBatch, NumEvents, 7);
    ThreadedRandomDataSource ds_b;
    setup_datasource(ds_b, sbio::RandomTraits::IndexingMode::IndexBatch, NumEvents, 7);

    // Test that both DataSources can run independently
    std::size_t num_a { 0 };
    std::size_t num_b { 0 };
    auto step_a = ds_a.next();
    auto step_b = ds_b.next();
    for (; step_a < NumEvents || step_b < NumEvents; step_a = ds_a.next(), step_b = ds_b.next()) {
      if (step_a < NumEvents) {
        EXPECT_EQ(step_a, num_a);
        ++num_a;
      }

      if (step_b < NumEvents) {
        EXPECT_EQ(step_b, num_b);
        ++num_b;
      }
    }

    EXPECT_EQ(num_a, NumEvents);
    EXPECT_EQ(num_b, NumEvents);
  }
} // anonymous namespace
