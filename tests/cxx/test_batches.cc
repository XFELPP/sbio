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
  using SerialRandomDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::RandomTraits
  >;

  struct BatchParams {
    std::size_t index_batch_size;
    std::size_t max_batch_size;
  };

  template <class DS>
  void setup_datasource(DS& ds,
                        sbio::RandomTraits::IndexingMode mode,
                        std::size_t num_events,
                        const BatchParams& bp) {
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
    params.indexing_batch_size = bp.index_batch_size;

    sbio::GenericStreamConfig<sbio::RandomTraits> cfg;
    cfg.format_params = params;
    cfg.max_batch_size = bp.max_batch_size;

    ASSERT_TRUE(ds.load_source(cfg, detectors, num_detectors));
    ASSERT_EQ(ds.discover_metadata(), sbio::IOStatus::Success);
  }

  // Can verify contiguoity using the sequential pattern of randfmt
  template <class Group>
  std::size_t count_bad_steps(Group& grp, std::size_t first, std::size_t last) {
    auto arr = grp.get_multi_data({ first, last });

    const auto* tbl { reinterpret_cast<const void* const*>(arr.data()) };
    const std::size_t num_segments { grp.num_segments() };

    std::size_t bad { 0 };
    for (std::size_t s = first; s < last; ++s) {
      const auto* bytes { static_cast<const std::uint8_t*>(tbl[(s - first) * num_segments]) };
      if (bytes == nullptr                                      ||
          bytes[0] != static_cast<std::uint8_t>(s & 0xFF)       ||
          bytes[1] != static_cast<std::uint8_t>((s + 1) & 0xFF)) {
        bad++;
      }
    }

    return bad;
  }

  class SerialBatches : public ::testing::TestWithParam<BatchParams> {};

  TEST_P(SerialBatches, ContiguousAndComplete) {
    constexpr std::size_t NumEvents { 100 };
    SerialRandomDataSource ds;
    setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, NumEvents, GetParam());

    auto grp = ds.get_stream_group("det0");
    ASSERT_GT(grp.num_segments(), 0u);

    std::size_t expected { 0 };
    std::size_t bad { 0 };
    for (auto batch : ds.batches()) {
      EXPECT_EQ(batch.first, expected) << "batches are contiguous";
      EXPECT_GE(batch.count(), 1u);
      EXPECT_LE(batch.count(), GetParam().max_batch_size);
      bad += count_bad_steps(grp, batch.first, batch.last);
      expected = batch.last;
    }

    EXPECT_EQ(expected, NumEvents);
    EXPECT_EQ(bad, 0u);
  }

  TEST_P(SerialBatches, BatchAxisAlwaysPresent) {
    SerialRandomDataSource ds;
    setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, 30, GetParam());

    auto grp = ds.get_stream_group("det0");
    ASSERT_GT(grp.num_segments(), 0u);

    for (auto batch : ds.batches()) {
      auto arr = grp.get_multi_data({ batch.first, batch.last });

      // Should have shape (batch, segment, 32, 32)
      EXPECT_EQ(arr.ndim(), 4) << "Unexpected dimensionality!";
      for (ssize_t i = 0; i < arr.ndim(); ++i) {
        if (i == 0) {
          EXPECT_EQ(arr.shape(i), batch.count()) << "Unexpected batch count!";
        } else if (i == 1) {
          EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
        } else {
          EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
        }
      }
    }
  }

  TEST_P(SerialBatches, BreakEarly) {
    SerialRandomDataSource ds;
    setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, 100, GetParam());

    auto grp = ds.get_stream_group("det0");
    ASSERT_GT(grp.num_segments(), 0u);

    std::size_t batches { 0 };
    for (auto batch : ds.batches()) {
      auto arr = grp.get_multi_data({batch.first, batch.last});

      EXPECT_EQ(arr.ndim(), 4) << "Unexpected dimensionality!";
      for (ssize_t i = 0; i < arr.ndim(); ++i) {
        if (i == 0) {
          EXPECT_EQ(arr.shape(i), batch.count()) << "Unexpected batch count!";
        } else if (i == 1) {
          EXPECT_EQ(arr.shape(i), 1) << "Unexpected segment count!";
        } else {
          EXPECT_EQ(arr.shape(i), 32) << "Segment size is incorrect!";
        }
      }

      if (++batches == 3) {
        break;
      }
    }

    EXPECT_EQ(batches, 3u);
  }

  INSTANTIATE_TEST_SUITE_P(Sizes, SerialBatches, ::testing::Values(BatchParams { 7, 1 },
                                                                   BatchParams { 7, 3 },
                                                                   BatchParams { 7, 7 },
                                                                   BatchParams { 7, 10 },
                                                                   BatchParams { 50, 8 }));
} // anonymous namespace
