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

#include "sbio/core/roles.hh"
#include "sbio/core/storage.hh"
#include "sbio/execution/serial.hh"
#include "sbio/execution/threaded.hh"
#include "sbio/formats/random/random_traits.hh"
#include "sbio/io/posix.hh"

#include <gtest/gtest.h>

#include <atomic>
#include <set>
#include <thread>
#include <type_traits>
#include <vector>

namespace {
  using DataDesc = sbio::BufferDescriptor<sbio::roles::Data, 0, 64>;
  using TableDesc = sbio::BufferDescriptor<sbio::roles::Table, 0, 16>;
  using IndexDesc = sbio::BufferDescriptor<sbio::roles::Index, 0, 32>;
  using Reqs = sbio::RequirementsList<DataDesc, TableDesc, IndexDesc>;

  // The DistributionManager determines the scope of a buffer based on the role
  // The MemoryManager determines where best to put that buffer
  static_assert(std::is_same_v<sbio::SerialExecution::BufferTypeFor<DataDesc>, sbio::HostBuffer>);
  static_assert(std::is_same_v<sbio::ThreadedExecution::BufferTypeFor<IndexDesc>, sbio::HostBuffer>);
  static_assert(std::is_same_v<sbio::ThreadedExecution::BufferTypeFor<DataDesc>,
                               sbio::PerUnitBuffer<sbio::HostBuffer, sbio::ThreadedExecution>>);

  // Configuration comes from the constituent managers as do support levels etc
  static_assert(std::is_same_v<
                  sbio::ThreadedExecution::Config,
                  sbio::ThreadedDistribution::Config
                >);
  static_assert(sbio::ThreadedExecution::ParallelSupport[static_cast<std::size_t>(sbio::ParallelizationMethods::THREADS)]);

  template <class Policy>
  auto make_storage() {
    sbio::Storage<Reqs, Policy> s;
    s.template get_member<DataDesc>().set_memory(nullptr, 64);
    s.template get_member<TableDesc>().set_memory(nullptr, 16);

    Policy::allocate_buffer(s.template get_member<IndexDesc>(), 32);

    return s;
  }
} // anonymous namespace

TEST(Composed, PerUnitBuffersArePerThread) {
  auto storage { make_storage<sbio::ThreadedExecution>() };

  constexpr int NumThreads { 4 };
  std::vector<void*> data(NumThreads);
  std::vector<void*> index(NumThreads);
  std::vector<void*> again(NumThreads);
  std::vector<std::thread> threads;
  for (int thread = 0; thread < NumThreads; ++thread) {
    auto fill_and_check_buffers = [&, thread]() {
      data[thread] = storage.template get<sbio::roles::Data>().ptr();
      index[thread] = storage.template get<sbio::roles::Index>().ptr();
      again[thread] = storage.template get<sbio::roles::Data>().ptr();

      EXPECT_EQ(storage.template get<sbio::roles::Data>().size(), 64u);
    };

    threads.emplace_back(fill_and_check_buffers);
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // Check 1 buffer per thread
  std::set<void*> distinct(data.begin(), data.end());
  EXPECT_EQ(distinct.size(), static_cast<std::size_t>(NumThreads));
  for (int thread = 0; thread < NumThreads; ++thread) {
    EXPECT_NE(data[thread], nullptr);
    // Buffer is stable for the thread
    EXPECT_EQ(data[thread], again[thread]);
    // Except the index buffer which is Shared
    EXPECT_EQ(index[thread], index[0]);
  }
}

TEST(Composed, SerialBuffersAreShared) {
  sbio::Storage<Reqs, sbio::SerialExecution> s;
  sbio::SerialExecution::allocate_buffer(s.template get_member<DataDesc>(), 64);

  EXPECT_NE(s.template get<sbio::roles::Data>().ptr(), nullptr);
  EXPECT_EQ(s.template get<sbio::roles::Data>().size(), 64u);
}

namespace {
  /**
   * A memory manager counting its live buffers.
   */
  struct CountingMemory {
    template <typename Descriptor>
    using PlacementFor = sbio::HostBuffer;

    static void allocate_buffer(sbio::HostBuffer& buf, std::size_t size) {
      buf.set_memory(new char[size], size);
      live++;
    }

    static void release_buffer(sbio::HostBuffer& buf) {
      delete[] static_cast<char*>(buf.ptr());
      buf.set_memory(nullptr, 0);
      live--;
    }

    static inline std::atomic<int> live { 0 };
  };

  using CountingSerial = sbio::Composed<sbio::SerialDistribution, CountingMemory>;
  using CountingThreaded = sbio::Composed<sbio::ThreadedDistribution, CountingMemory>;

  template <class Policy>
  auto allocate() {
    sbio::AllocationRequest<sbio::RandomTraits> request;
    return Policy::template allocate_storage<Reqs, sbio::SyncPOSIXIO, sbio::RandomTraits>(request);
  }
} // anonymous namespace

TEST(Composed, SharedBuffersReleasedWithTheLastCopy) {
  CountingMemory::live = 0;
  {
    auto storage { allocate<CountingSerial>() };
    EXPECT_EQ(CountingMemory::live.load(), 3);
    {
      auto copy { storage };
      EXPECT_EQ(copy.template get<sbio::roles::Data>().ptr(),
                storage.template get<sbio::roles::Data>().ptr());
    }
    EXPECT_EQ(CountingMemory::live.load(), 3); // The original still uses the buffers
  }
  EXPECT_EQ(CountingMemory::live.load(), 0);
}

TEST(Composed, PerUnitBuffersReleasedWithTheStorage) {
  CountingMemory::live = 0;
  constexpr int NumThreads { 4 };
  {
    auto storage { allocate<CountingThreaded>() };
    EXPECT_EQ(CountingMemory::live.load(), 1); // Only the shared Index so far

    std::vector<std::thread> threads;
    for (int thread = 0; thread < NumThreads; ++thread) {
      threads.emplace_back([&]() {
        static_cast<void>(storage.template get<sbio::roles::Data>().ptr());
        static_cast<void>(storage.template get<sbio::roles::Table>().ptr());
      });
    }
    for (auto& thread : threads) {
      thread.join();
    }

    EXPECT_EQ(CountingMemory::live.load(), 1 + 2 * NumThreads);
  }
  EXPECT_EQ(CountingMemory::live.load(), 0);
}
