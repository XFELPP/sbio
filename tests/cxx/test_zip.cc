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
  /**
   * SyncPOSIXIO which counts reads, to check how often the same bytes are fetched.
   *
   * Needed this minor wrapper to verify that the zipping process really reduces the
   * number of IO fetches.
   */
  struct CountingIO : public sbio::IOPolicy<CountingIO> {
    static inline std::atomic<std::size_t> reads { 0 };

    template <typename... Args>
    sbio::IOStatus connect(Args&&... args) {
      auto status { m_io.connect(std::forward<Args>(args)...) };
      m_file_size = m_io.file_size();

      return status;
    }

    sbio::ReadResult read_impl(std::uint64_t offset,
                               std::size_t size,
                               void* dest) const {
      reads++;

      return m_io.read(offset, size, dest);
    }

  private:
    sbio::SyncPOSIXIO m_io;
  };

  template <class EPolicy>
  using CountingDS = sbio::DataSource<CountingIO, EPolicy, sbio::RandomTraits>;

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

    ASSERT_TRUE(ds.load_source(cfg, detectors, num_detectors));
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

  // Test two BrokerGroups that use the same StreamBroker
  // - There should only be 1 fetch per step (the non-zip version will fetch twice, once
  //   per each BrokerGroup)
  TEST(ZippedGroups, SharedBrokerFetchedOncePerStep) {
    constexpr std::size_t NumEvents { 60 };
    // With IndexAll, this value shouldn't matter -- will index everything
    constexpr std::size_t IndexBatch { 1000 };

    // Check how many happen without zipping
    std::size_t straight_reads { 0 };
    {
      CountingDS<sbio::SerialExecution> ds;
      setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexAll, NumEvents, IndexBatch);

      auto grp1 = ds.get_stream_group("det0");
      ASSERT_GT(grp1.num_segments(), 0u);
      auto grp2 = ds.get_stream_group("det0");
      ASSERT_GT(grp2.num_segments(), 0u);

      const std::size_t before { CountingIO::reads.load() };
      for (auto step : ds.steps()) {
        auto arr1 = grp1.get_data(step); // Can use same step for both groups
        auto arr2 = grp2.get_data(step);

        for (ssize_t i = 0; i < arr1.ndim(); ++i) {
          if (i == 0) {
            EXPECT_EQ(arr1.shape(i), 1) << "Unexpected segment count (arr1)!";
            EXPECT_EQ(arr2.shape(i), 1) << "Unexpected segment count (arr2)!";
          } else {
            EXPECT_EQ(arr1.shape(i), 32) << "Segment size is incorrect (arr1)!";
            EXPECT_EQ(arr2.shape(i), 32) << "Segment size is incorrect (arr2)!";
          }
        }

        // Check the actual bytes as well
        EXPECT_TRUE(bytes_match(arr1, step));
        EXPECT_TRUE(bytes_match(arr2, step));
      }

      straight_reads = CountingIO::reads.load() - before;
    }

    CountingDS<sbio::SerialExecution> ds;
    setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, NumEvents, IndexBatch);

    auto grp1 = ds.get_stream_group("det0");
    ASSERT_GT(grp1.num_segments(), 0u);
    auto grp2 = ds.get_stream_group("det0");
    ASSERT_GT(grp2.num_segments(), 0u);

    // Now check how many fetches when zipped
    const std::size_t before { CountingIO::reads.load() };
    std::size_t expected { 0 };
    {
      auto zipped = sbio::zip(ds, grp1, grp2);
      EXPECT_EQ(zipped.num_fetches(), 1u);

      for (auto step : zipped) {
        EXPECT_EQ(step.status(), sbio::IOStatus::Success);
        EXPECT_EQ(static_cast<std::size_t>(step), expected);

        auto arr1 = step.get(grp1);
        auto arr2 = step.get(grp2);

        for (ssize_t i = 0; i < arr1.ndim(); ++i) {
          if (i == 0) {
            EXPECT_EQ(arr1.shape(i), 1) << "Unexpected segment count (arr1)!";
            EXPECT_EQ(arr2.shape(i), 1) << "Unexpected segment count (arr2)!";
          } else {
            EXPECT_EQ(arr1.shape(i), 32) << "Segment size is incorrect (arr1)!";
            EXPECT_EQ(arr2.shape(i), 32) << "Segment size is incorrect (arr2)!";
          }
        }

        EXPECT_TRUE(bytes_match(arr1, step));
        EXPECT_TRUE(bytes_match(arr2, step));

        expected++;
      }
    }
    const std::size_t zip_reads { CountingIO::reads.load() - before };

    EXPECT_EQ(expected, NumEvents);
    // Both loops make the same reindexing reads
    // The straight loop reads every step's data a second time for grp2.
    EXPECT_EQ(straight_reads - zip_reads, NumEvents) << "zip reads each shared broker once per step";
  }
} // anonymous namespace
