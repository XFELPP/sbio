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
#include "sbio/io/posix.hh"

#include <cuda_runtime.h>
#include <gtest/gtest.h>
#include <ncarray/soarrays.hh>
#include <ncarray/sodevarrays.cuh>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

namespace {
  using Policy = sbio::ThreadedCudaExecution;

  // Scope comes from ThreadedDistribution, CudaMemory places the buffers
  using DataDesc = sbio::BufferDescriptor<sbio::roles::Data, 0, 64>;
  using TableDesc = sbio::BufferDescriptor<sbio::roles::Table, 0, 16>;

  static_assert(std::is_same_v<Policy::BufferTypeFor<DataDesc>,
                               sbio::PerUnitBuffer<sbio::CudaMirroredBuffer, Policy>>);
  static_assert(std::is_same_v<Policy::BufferTypeFor<TableDesc>,
                               sbio::PerUnitBuffer<sbio::CudaPinnedBuffer, Policy>>);

  // Capabilities and spaces should be inherited from managers
  static_assert(Policy::result_memory_space() == sbio::MemorySpace::Device);
  static_assert(Policy::required_io_capabilities<sbio::SyncPOSIXIO>() ==
                (sbio::IOCapability::ReadToPinned | sbio::IOCapability::ConcurrentReads));

  // Configuration comes from the constituent managers
  static_assert(std::is_base_of_v<sbio::ThreadedDistribution::Config, Policy::Config>);
  static_assert(std::is_base_of_v<sbio::CudaMemory::Config, Policy::Config>);

  using Ctx = sbio::Context<sbio::SyncPOSIXIO, Policy>;

  constexpr std::size_t NumEvents { 200 };
  constexpr std::size_t NumThreads { 4 };

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

  // Return the device or -1 if not a device pointer
  int on_device(const void* p) {
    cudaPointerAttributes attr {};
    if (cudaPointerGetAttributes(&attr, p) != cudaSuccess ||
        attr.type != cudaMemoryTypeDevice) {
      return -1; // Not a device pointer
    }

    return attr.device;
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

  // Track devices and steps seen, and any errors
  struct Outcome {
    std::size_t bad_bytes { 0 };
    std::size_t wrong_device { 0 };
    std::vector<std::size_t> seen {};
    std::set<int> devices {};
  };

  // constexpr int UnitDevice { -1 };
  constexpr int UnitDevice { 0 };

  // Check threads data is each on the expected device
  template <class DS>
  Outcome run_threads(DS& ds, int device) {
    auto grp = ds.get_stream_group("det0");

    Outcome out;
    std::mutex mtx;
    auto worker = [&]() {
      std::vector<std::size_t> mine;
      std::size_t bad { 0 };
      std::size_t wrong { 0 };
      std::set<int> used;

      const int expected { device == UnitDevice ? sbio::CudaMemory::unit_device() : device };

      for (auto step = ds.next(); step < NumEvents; step = ds.next()) {
        auto arr = grp.get_data(step);
        if (!bytes_match(arr, step)) {
          ++bad;
        }

        const void* data { reinterpret_cast<const void* const*>(arr.data())[0] };
        int found_dev { on_device(data) };
        if (found_dev != expected) {
          ++wrong;
        }
        used.insert(found_dev);
        mine.push_back(step);
      }

      std::lock_guard<std::mutex> lock(mtx);
      out.bad_bytes += bad;
      out.wrong_device += wrong;
      out.devices.insert(used.begin(), used.end());
      out.seen.insert(out.seen.end(), mine.begin(), mine.end());
    };

    std::vector<std::thread> threads;
    for (std::size_t thread = 0; thread < NumThreads; ++thread) {
      threads.emplace_back(worker);
    }

    for (auto& thread : threads) {
      thread.join();
    }

    std::sort(out.seen.begin(), out.seen.end());
    return out;
  }

  void expect_every_step_once(const Outcome& out) {
    ASSERT_EQ(out.seen.size(), NumEvents) << "every step exactly once";
    for (std::size_t i = 0; i < out.seen.size(); ++i) {
      ASSERT_EQ(out.seen[i], i);
    }
  }
} // anonymous namespace

class ThreadedCuda : public ::testing::TestWithParam<std::size_t> {};

TEST_P(ThreadedCuda, ThreadsGetTheirStepsOnDevice) {
  Ctx ctx;
  ASSERT_TRUE(ctx.ok()) << ctx.error();

  auto ds = ctx.data_source<sbio::RandomTraits>();
  setup_datasource(ds,
                   sbio::RandomTraits::IndexingMode::IndexBatch,
                   NumEvents,
                   GetParam());

  const Outcome out { run_threads(ds, 0) };
  EXPECT_EQ(out.bad_bytes, 0u) << "steps returned another event's bytes";
  EXPECT_EQ(out.wrong_device, 0u);

  // Make sure the threads are properly distributing steps out
  expect_every_step_once(out);
}

INSTANTIATE_TEST_SUITE_P(BatchSizes, ThreadedCuda, ::testing::Values(3, 7, 50));

TEST(ThreadedCudaDevice, WorkerThreadsUseTheConfiguredDevice) {
  int count { 0 };
  if (cudaGetDeviceCount(&count) != cudaSuccess || count < 2) {
    GTEST_SKIP() << "Needs 2 CUDA devices";
  }

  Ctx::EPolicyConfig config {};
  config.device = 1;
  Ctx ctx(config);
  ASSERT_TRUE(ctx.ok()) << ctx.error();

  auto ds = ctx.data_source<sbio::RandomTraits>();
  setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, NumEvents, 7);

  // Each thread gets its own Data buffer
  const Outcome out { run_threads(ds, 1) };
  EXPECT_EQ(out.bad_bytes, 0u);
  EXPECT_EQ(out.wrong_device, 0u) << "a worker thread allocated on another device";

  // Make sure the threads are properly distributing steps out
  expect_every_step_once(out);
}

TEST(ThreadedCudaDevice, ThreadsSpreadOverDevices) {
  int count { 0 };
  if (cudaGetDeviceCount(&count) != cudaSuccess || count < 2) {
    GTEST_SKIP() << "Needs 2 CUDA devices";
  }

  Ctx::EPolicyConfig config {};
  config.devices = { 0, 1 };
  Ctx ctx(config);
  ASSERT_TRUE(ctx.ok()) << ctx.error();

  auto ds = ctx.data_source<sbio::RandomTraits>();
  setup_datasource(ds, sbio::RandomTraits::IndexingMode::IndexBatch, NumEvents, 7);

  // Threads distributed between the devices with data on each
  const Outcome out { run_threads(ds, UnitDevice) };
  EXPECT_EQ(out.bad_bytes, 0u);
  EXPECT_EQ(out.wrong_device, 0u) << "a thread's data was not on its unit's device";
  EXPECT_EQ(out.devices, (std::set<int> { 0, 1 }));

  // Make sure the threads are properly distributing steps out
  expect_every_step_once(out);
}

TEST(ThreadedCudaDevice, UnknownDevicesAreRejected) {
  int count { 0 };
  ASSERT_EQ(cudaGetDeviceCount(&count), cudaSuccess);

  Ctx::EPolicyConfig config {};
  config.devices = { 0, count };

  Ctx ctx(config);
  EXPECT_FALSE(ctx.ok());
  EXPECT_NE(ctx.error().find("devices"), std::string::npos) << ctx.error();
}
