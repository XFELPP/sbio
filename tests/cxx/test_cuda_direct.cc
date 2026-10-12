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
#include "sbio/execution/cuda.hh"
#include "sbio/formats/random/randfmt.hh"
#include "sbio/formats/random/random_locator.hh"
#include "sbio/formats/random/random_traits.hh"
#include "sbio/io/cufile.hh"

#include <cuda_runtime.h>
#include <gtest/gtest.h>
#include <ncarray/soarrays.hh>
#include <ncarray/sodevarrays.cuh>

#include <cstdint>
#include <cstdio>

namespace {
  /// --- cuFileIO reads directly to device memory so check caps match --- ///
  static_assert(sbio::CudaMemory::reads_to_device<sbio::cuFileIO>());

  static_assert(sbio::CudaExecution::required_io_capabilities<sbio::cuFileIO>() ==
                (sbio::IOCapability::ReadToDevice | sbio::IOCapability::ReadToHost));

  using Ctx = sbio::Context<sbio::cuFileIO, sbio::CudaExecution>;

  constexpr std::size_t NumEvents { 30 };

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

  bool on_device(const void* p) {
    cudaPointerAttributes attr {};
    return cudaPointerGetAttributes(&attr, p) == cudaSuccess && attr.type == cudaMemoryTypeDevice;
  }

  template <class Array>
  bool bytes_match(const Array& arr, std::size_t step) {
    /// Need to copy over
    ncarray::SOOwnerFor<ncarray::HostTag> h_arr(arr.ndim(),
                                                arr.shape(),
                                                ncarray::DType::uint16);
    arr.copy_into(h_arr.data());

    ssize_t shape[2] { 1, h_arr.nbytes() };
    ssize_t strides[2] { h_arr.nbytes(), 1 };
    // NOTE: The data type is uint16, but the rules from randfmt still hold
    //       here for the sequential pattern - byte i of event n is (n + i) & 0xFF
    ncarray::NCArrayView bytes(h_arr.data(), 2, shape, strides, ncarray::DType::uint8, -1, false);

    const std::uint8_t b0 { bytes[{0, 0}] };
    const std::uint8_t b1 { bytes[{0, 1}] };

    return b0 == static_cast<std::uint8_t>(step & 0xFF)    &&
           b1 == static_cast<std::uint8_t>((step + 1) & 0xFF);
  }
} // anonymous namespace

TEST(CudaDirect, StepsReturnDeviceData) {
  Ctx ctx;
  ASSERT_TRUE(ctx.ok()) << ctx.error();

  auto ds = ctx.data_source<sbio::RandomTraits>();
  setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, 40, 7);
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

    const void* data { reinterpret_cast<const void* const*>(arr.data())[0] };
    EXPECT_TRUE(on_device(data)) << "Data not on device for step " << step;
    EXPECT_TRUE(bytes_match(arr, step)) << "Bad bytes for step " << step;
    ++seen;
  }
  EXPECT_EQ(seen, 40);
}

TEST(CudaDirect, BatchesReturnDeviceData) {
  Ctx ctx;
  ASSERT_TRUE(ctx.ok()) << ctx.error();

  auto ds = ctx.data_source<sbio::RandomTraits>();
  setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, 40, 7);
  auto grp = ds.get_stream_group("det0");

  std::size_t seen { 0 };
  for (auto batch : ds.batches(3)) {
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

    const auto* table { reinterpret_cast<const void* const*>(arr.data()) };
    for (std::size_t step = 0; step < batch.count(); ++step) {
      EXPECT_TRUE(on_device(table[step])) << "Data not on device for step " << step;
      EXPECT_TRUE(bytes_match(arr(step), batch.first + step)) << "Bad bytes for step " << step;
      ++seen;
    }
  }

  EXPECT_EQ(seen, 40);
}
