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

  template <class Array>
  std::size_t count_bad_batch(const Array& arr, std::size_t first, std::size_t last) {
    std::size_t bad { 0 };
    for (std::size_t s = first; s < last - 1; ++s) {
      // NOTE: The data type is actually uint16, but the randfmt byte pattern is
      //       still written by byte, so this comparison is accurate
      std::uint8_t& v1 = arr[{s - first, 0, 0, 0}];
      std::uint8_t& v2 = arr[{(s + 1) - first, 0, 0, 0}];
      bad += !(v1 == static_cast<std::uint8_t>(s & 0xFF)      &&
               v2 == static_cast<std::uint8_t>((s + 1) & 0xFF));
    }

    return bad;
  }

  // Setup DataSources with different groups, indexing windows, lengths etc.
  // Should end after the minimum capacity, and reindexing should only happen for
  // the DataSource that hit ExhaustedSentinel so the other doesn't get overwritten.
  TEST(SourceSet, SerialLockstepEndsAtShortest) {
    SerialRandomDataSource ds_a;
    SerialRandomDataSource ds_b;

    setup_datasource(ds_a,
                     "det_a",
                     sbio::RandomTraits::IndexingMode::IndexBatch,
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

  TEST(SourceSet, SerialStepsWithoutZip) {
    SerialRandomDataSource ds_a;
    SerialRandomDataSource ds_b;
    setup_datasource(ds_a,
                     "det_a",
                     sbio::RandomTraits::IndexingMode::IndexBatch,
                     30,
                     { 4, 1 });
    setup_datasource(ds_b,
                     "det_b",
                     sbio::RandomTraits::IndexingMode::IndexBatch,
                     30,
                     { 9, 1 });

    auto grp_a = ds_a.get_stream_group("det_a");
    auto grp_b = ds_b.get_stream_group("det_b");

    auto set = sbio::source_set(ds_a, ds_b);

    std::size_t expected { 0 };
    std::size_t bad { 0 };
    for (auto step : set.steps()) {
      EXPECT_EQ(step, expected);

      auto arr_a = grp_a.get_data(step);
      auto arr_b = grp_b.get_data(step);

      bad += !bytes_match(arr_a, step);
      bad += !bytes_match(arr_b, step);

      expected++;
    }

    EXPECT_EQ(expected, 30u);
    EXPECT_EQ(bad, 0u);
  }


  TEST(SourceSet, SerialBatches) {
    SerialRandomDataSource ds_a;
    setup_datasource(ds_a,
                     "det_a",
                     sbio::RandomTraits::IndexingMode::IndexBatch,
                     60,
                     { 7, 4 });
    SerialRandomDataSource ds_b;
    setup_datasource(ds_b,
                     "det_b",
                     sbio::RandomTraits::IndexingMode::IndexBatch,
                     45,
                     { 5, 3 });

    auto grp_a = ds_a.get_stream_group("det_a");
    auto grp_b = ds_b.get_stream_group("det_b");

    auto set = sbio::source_set(ds_a, ds_b);

    std::size_t expected { 0 };
    std::size_t bad { 0 };
    for (auto batch : sbio::zip_batches(set, grp_a, grp_b)) {
      EXPECT_EQ(batch.status(), sbio::IOStatus::Success);
      EXPECT_EQ(batch.first(), expected);
      EXPECT_LE(batch.count(), 3u) << "smallest max_batch_size of the sources";

      auto arr_a = batch.get(grp_a);
      auto arr_b = batch.get(grp_b);

      bad += count_bad_batch(arr_a, batch.first(), batch.last());
      bad += count_bad_batch(arr_b, batch.first(), batch.last());

      expected = batch.last();
    }

    EXPECT_EQ(expected, 45u);
    EXPECT_EQ(bad, 0u);
  }

  TEST(SourceSet, SerialBreakEarly) {
    SerialRandomDataSource ds_a;
    setup_datasource(ds_a,
                     "det_a",
                     sbio::RandomTraits::IndexingMode::IndexBatch,
                     30,
                     { 7, 1 });
    SerialRandomDataSource ds_b;
    setup_datasource(ds_b,
                     "det_b",
                     sbio::RandomTraits::IndexingMode::IndexBatch,
                     30,
                     { 5, 1 });

    auto grp_a = ds_a.get_stream_group("det_a");
    auto grp_b = ds_b.get_stream_group("det_b");

    auto set = sbio::source_set(ds_a, ds_b);

    std::size_t count { 0 };
    for (auto step : sbio::zip(set, grp_a, grp_b)) {
      auto arr_a = step.get(grp_a);

      EXPECT_TRUE(bytes_match(arr_a, step));
      if (++count == 5) {
        break;
      }
    }
    EXPECT_EQ(count, 5u);
  }
} // anonymous namespace
