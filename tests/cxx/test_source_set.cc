#include "sbio/core/datasource.hh"
#include "sbio/core/iteration.hh"
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

  struct BatchParams {
    std::size_t index_batch_size { 10 };
    std::size_t max_batch_size { 1 };
  };

  template <class Source>
  void setup_datasource(Source& ds,
                        const char* det_name,
                        sbio::RandomTraits::IndexingMode mode,
                        std::size_t num_events,
                        const BatchParams bp = BatchParams{}) {
    sbio::randfmt::DetectorSpec detectors[10];

    snprintf(detectors[0].name, sbio::RandomTraits::MaxNameSize, "%s", det_name);
    snprintf(detectors[0].type, sbio::RandomTraits::MaxNameSize, "test_det");

    detectors[0].rank = 2;
    detectors[0].shape[0] = 32;
    detectors[0].shape[1] = 32;
    detectors[0].dtype = ncarray::DType::uint16;

    sbio::RandomTraits::StreamParameters params;
    params.num_events = num_events;
    params.pattern_type = 1; // Sequential
    params.indexing_mode = mode;
    params.indexing_batch_size = bp.index_batch_size;

    sbio::GenericStreamConfig<sbio::RandomTraits> cfg;
    cfg.format_params = params;
    cfg.max_batch_size = bp.max_batch_size;

    ASSERT_TRUE(ds.load_source(cfg, detectors, 1));
    ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);
  }

  template <class Array>
  bool bytes_match(const Array& arr, std::size_t step) {
    const auto* tbl { reinterpret_cast<const void* const*>(arr.data()) };
    const auto* bytes { static_cast<const std::uint8_t*>(tbl[0]) };

    return bytes != nullptr                                      &&
           bytes[0] == static_cast<std::uint8_t>(step & 0xFF)    &&
           bytes[1] == static_cast<std::uint8_t>((step + 1) & 0xFF);
  }

  // Setup DataSources with different groups, indexing windows, lengths etc.
  // Should end after the minimum capacity, and reindexing should only happen for
  // the DataSource that hit ExhaustedSentinel so the other doesn't get overwritten.
  TEST(SourceSet, SerialLockstepEndsAtShortest) {
    SerialRandomDataSource ds_a;
    SerialRandomDataSource ds_b;

    setup_datasource(ds_a,
                     "det_a",
                     sbio::RandomTraits::IndexingMode::IndexBatch
                     60,
                     { 7, 1 });
    setup_datasource(ds_b,
                     "det_b",
                     sbio::RandomTraits::IndexingMode::IndexBatch,
                     45,
                     { 5, 1 });

    auto grp_a = ds_a.get_stream_group("det_a");
    auto grp_b = ds_b.get_stream_group("det_b");

    auto set = sbio::source_set(ds_a, ds_b);

    std::size_t expected { 0 };
    std::size_t bad { 0 };
    for (auto step : sbio::zip(set, grp_a, grp_b)) {
      EXPECT_EQ(step.status(), sbio::IOStatus::Success);
      EXPECT_EQ(static_cast<std::size_t>(step), expected);

      auto arr_a = step.get(grp_a);
      auto arr_b = step.get(grp_b);

      bad += !bytes_match(arr_a, step);
      bad += !bytes_match(arr_b, step);

      expected++;
    }

    EXPECT_EQ(expected, 45u);
    EXPECT_EQ(bad, 0u);
  }
} // anonymous namespace
