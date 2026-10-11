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

#ifndef SBIO_EXECUTION_CUDA_HH
#define SBIO_EXECUTION_CUDA_HH

#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/result.hh"
#include "sbio/core/roles.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/types.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/storage/cuda_device.hh"
#include "sbio/storage/host_buffer.hh"

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cuda_runtime.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <bitset>
#include <type_traits>
#include <unordered_map>

namespace sbio {
  class CudaExecution;

#ifdef __CUDACC__
  namespace impl {
    /**
     * A request that was resolved on the device side (with updated cache offsets)
     */
    struct DeviceResolved {
      DataResult result;
      PayloadOffsetCache cache;
    };

    /**
     * Perform a DataRequest lookup directly on the device.
     *
     * This assumes the host-code was not GPU-optimized (synchronous direct lookup)
     * so it should be run from a single CUDA thread.
     */
    template <class FTraits, class StorageT, class InventoryT>
    __global__ void resolve_in_buffer_kernel(StorageT storage,
                                             const Inventory* inv,
                                             typename FTraits::DataRequest req,
                                             PayloadOffsetCahce cache,
                                             typename FTraits::DataAccessPtn ptn,
                                             std::size_t batch_idx,
                                             DeviceResolved* out) {
      StorageView<StorageT, CudaExecution> sv(storage);
      out->result = FTraits::get_data_in_buffer(sv, *inv, req, cache, ptn, batch_idx);
      out->cache = cache;
    }
  } // namespace impl
#endif // __CUDAC__

  /**
   * CudaExecution is a policy for a single process to use one GPU.
   *
   * IO is staged first through the host. After which results are moved to GPU, and
   * returned in GPU device memory.
   * - All data buffers are device memory, and have a replica pinned host buffer
   *   by using `CudaMirroredBuffer`. IO reads into the host replica.
   * - Data is moved to device using `publish_broker_view`.
   * - Data format field resolution can happen on the host - so no copy back has
   *   to occur for this process. The pointers resolved on host are remapped directly
   *   on device.
   * - The Table for the BrokerGropu is pinned host memory so it can be read on device.
   *
   * Iteration happens from the host from a single thread (serial).
   *
   * `get_data` and friends stream synchronize before returns to guarantee results
   * are ready.
   *
   * For IO policies that support direct-to-GPU reads, CudaExecution will use that path
   * without the need for the staged IO.
   * - If compiled as device code with nvcc, a single-threaded kernel will do all
   *   resolution/lookup directly on the device as well.
   * - Otherwise, the lookup will still be done on the host normally, by staging via the
   *   host replica buffer.
   *
   * @note Field and offset resolution on device is not currently available in an EPolicy.
   */
  class CudaExecution : public Execution<CudaExecution> {
  public:
    template <typename Descriptor>
    using BufferTypeFor = std::conditional_t<
      std::is_same_v<typename Descriptor::role, roles::Data>,
      CudaMirroredBuffer,
      std::conditional_t<
        std::is_same_v<typename Descriptor::role, roles::Table>,
        CudaPinnedBuffer,
        HostBuffer
      >
    >;

    struct Config {
      int device { 0 }; ///< Which GPU (CUDA device)
    };

    static constexpr std::bitset<
      static_cast<std::size_t>(ParallelizationMethods::NUM_METHODS)
    > ParallelSupport { 0x4 }; // 0b100 - CUDA

    static constexpr MemorySpace result_memory_space_impl() { return MemorySpace::Device; }

    /**
     * Check if the IO policy can read into GPU memory.
     */
    template <typename IO>
    static constexpr bool reads_to_device() {
      return has_capabilities(io_capabilities<IO>(), IOCapability::ReadToDevice);
    }

    /**
     * IO currently gets staged through a pinned host buffer for this EPolicy.
     */
    template <typename IO>
    static constexpr IOCapability required_io_capabilities_impl() {
      static_assert(has_capabilities(io_capabilities<IO>(), IOCapability::ReadToDevice) ||
                    has_capabilities(io_capabilities<IO>(), IOCapability::ReadToPinned),
                    "CudaExecution reads foirst to pinned host memory or device memory.");
      if constexpr (reads_to_device<IO>()) {
        return IOCapability::ReadToDevice | IOCapability::ReadToHost;
      } else {
        return IOCapability::ReadToPinned;
      }
    }

    /**
     * The requirements are available CUDA devices (GPUs), and config selecting one of these.
     */
    static bool validate_environment_impl(const Config& config, const char*& error) {
      int count { 0 };
      if (cudaGetDeviceCount(&count) != cudaSuccess || count == 0) {
        error = "No CUDA devices are available!";
        return false;
      }

      if (config.device < 0 || config.device >= count) {
        error = "The requested CUDA device does not exist! Check configuration!";
        return false;
      }

      return true;
    }

    static void configure_impl(const Config& config) {
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::CudaExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::CudaExecution");
      } else {
        m_logger = logger;
      }

      check(cudaSetDevice(config.device), "cudaSetDevice");

      get_stream();
    }

    class IterationState {
      friend class CudaExecution;

      std::size_t m_event_idx { 0 };
    };

    template <class T>
    struct SegmentState {
      mutable T value {};
      T& get() const { return value; }
    };

    template <IsRequirementsList Requirements, class IO, class FTraits>
    requires FormatTraits<FTraits, IO, CudaExecution>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::CudaExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::CudaExecution");
      } else {
        m_logger = logger;
      }


      return allocate_impl_helper<IO>(Requirements{}, request);
    }

    template <class IO, typename... Descriptors, class FTraits>
    static auto allocate_impl_helper(RequirementsList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      Storage<RequirementsList<Descriptors...>, CudaExecution> s;
      std::size_t i { 0 };

      auto make_buffer = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;

        using BufRole = typename Descriptor::role;
        using BufHint = typename Descriptor::hint;

        std::size_t sz { std::max(Descriptor::min_size, request.size_requests[i++]) };
        auto& buf { s.template get<Descriptor>() };

        void* ptr { nullptr };

        if constexpr (std::is_same_v<BufRole, roles::Data>) {
          void* dev { nullptr };
          void* host { nullptr };
          check(cudaMalloc(&dev, sz), "cudaMalloc [Data]");
          check(cudaMallocHost(&host, sz), "cudaMallocHost [Data]");

          buf.set_memory(dev, sz);
          buf.set_host_memory(host);

          if constexpr (reads_to_device<IO>() && requires { IO::register_buffer(dev, sz); }) {
            // cuFile works better if you register the buffers first
            if (!IO::register_buffer(dev, sz)) {
              m_logger->warn("Could not register the Data buffer with IO policy!");
            }
          }
        } else if constexpr (std::is_same_v<BufRole, roles::Table>) {
          void* pinned { nullptr };
          check(cudaMallocHost(&pinned, sz), "cudaMallocHost [Table]");

          buf.set_memory(pinned, sz);
        } else {
          buf.set_memory(new char[sz], sz);
        }
      };

      ( (make_buffer(std::type_identity<Descriptors>{})), ... );

      return s;
    }

    /**
     * Get a host view of a device buffer (using the replica in the buffer).
     */
    template <typename Buffer>
    static void* acquire_broker_view_impl(Buffer& buf) {
      if constexpr (requires { buf.host_ptr(); }) {
        if (buf.host_stale() > 0) {
          check(cudaMemcpyAsync(buf.host_ptr(),
                                buf.ptr(),
                                buf.host_stale(),
                                cudaMemcpyDeviceToHost,
                                get_stream()),
                "cudaMemcpyAsync [acquire host view]");
          synchronize("cudaStreamSynchronize [acquire host view]");
          buf.set_host_stale(0);
        }

        return buf.host_ptr();
      } else {
        // Accidentally called on some host buffer so just return that.
        return buf.ptr();
      }
    }

    /**
     * Move the bytes IO read from staging buffer (host) to device buffer.
     */
    template <typename Buffer>
    static void publish_broker_view_impl(Buffer& buf, void* view, std::size_t nbytes) {
      if constexpr (requires { buf.host_ptr(); }) {
        if (view == buf.host_ptr() && nbytes > 0) {
          check(cudaMemcpyAsync(buf.ptr(), view, nbytes, cudaMemcpyHostToDevice, get_stream()),
                "cudaMemcpyAsync [publish view]");
        } else if (view == buf.ptr()) {
          // The device buffer is current, but the host replica is not so will need
          // to be copied over
          buf.set_host_stale(nbytes);
        }
      }
    }

    template <class FTraits, class FetchCBType, class GetCBType>
    static IOStatus get_data_impl(typename FTraits::StepIdxType step_idx,
                                  FetchCBType&& unit_fetcher,
                                  std::size_t num_fetches,
                                  GetCBType&& unit_get_data,
                                  std::size_t num_accesses) {
      IOStatus status { IOStatus::Success };
      for (std::size_t i = 0; i < num_fetches; ++i) {
        if (auto s = unit_fetcher(i); s != IOStatus::Success) {
          status = s;
          break;
        }
      }

      if (status == IOStatus::Success) {
        for (std::size_t i = 0; i < num_accesses; ++i) {
          unit_get_data(i);
        }
      }

      synchronize("cudaStreamSynchronize [get_data]");

      return status;
    }

    template <class FTraits, class FetchCBType, class GetCBType>
    static IOStatus get_data_steps_impl(const std::initializer_list<typename FTraits::StepIdxType> steps,
                                        FetchCBType&& unit_fetcher,
                                        std::size_t num_fetches,
                                        GetCBType&& unit_get_data,
                                        std::size_t num_accesses) {
      IOStatus status { IOStatus::Success };
      for (std::size_t i = 0; i < num_fetches; ++i) {
        if (auto s = unit_fetcher(i); s != IOStatus::Success) {
          status = s;
          break;
        }
      }

      if (status == IOStatus::Success) {
        bool passed_step { steps.size() == 3 };
        typename FTraits::StepIdxType first { *steps.begin() };
        typename FTraits::StepIdxType last { passed_step ? *(steps.end() - 2) : *(steps.end() - 1) };
        std::size_t count { (last > first) ? static_cast<std::size_t>(last - first) : 1 };
        for (std::size_t cnt = 0; cnt < count; ++cnt) {
          for (std::size_t seg = 0; seg < num_accesses; ++seg) {
            unit_get_data(seg, cnt);
          }
        }
      }

      synchronize("cudaStreamSynchronize [get_data_steps]");
      return status;
    }

    template <class FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType
    next_impl(IterationState& state,
              typename FTraits::StepIdxType& max_capacity,
              IndexTrigger&& trigger) {
      if (state.m_event_idx >= max_capacity) {
        if (!trigger() || state.m_event_idx >= max_capacity) {
          return FTraits::ExhaustedSentinel;
        }
      }


      return state.m_event_idx++;
    }

    template <class FTraits, class IndexTrigger>
    static StepBatch<typename FTraits::StepIdxType>
    next_batch_impl(IterationState& state,
                    std::size_t batch_size,
                    typename FTraits::StepIdxType& max_capacity,
                    IndexTrigger&& trigger) {
      using StepIdx = typename FTraits::StepIdxType;

      const StepIdx first { next_impl<FTraits>(state, max_capacity, trigger) };
      if (first == FTraits::ExhaustedSentinel) {
        return { FTraits::ExhaustedSentinel, FTraits::ExhaustedSentinel };
      }

      const StepIdx want { first + static_cast<StepIdx>(batch_size > 0 ? batch_size : 1) };
      const StepIdx last { want < max_capacity ? want : max_capacity };
      state.m_event_idx = last;

      return { first, last };
    }

#ifdef __CUDACC__
    /**
     * Resolve a DataRequest directly on the device, where it was read with cuFile.
     */
    template <class FTraits, class IO, class StorageT, class InventoryT>
    requires (reads_to_device<IO>())
    static DataResult resolve_in_buffer_impl(StorageT& storage,
                                             const InventoryT& inv,
                                             const typename FTraits::DataRequest& req,
                                             PayloadOffsetCache& cache,
                                             typename FTraits::DataAccessPtn ptn,
                                             std::size_t batch_idx) {
      const InventoryT* dev_inv { device_inventory(inv) };
      impl::DeviceResolved* out { resolved_slot() };

      impl::resolve_in_buffer_kernel<FTraits><<<1, 1, 0, get_stream()>>>(storage,
                                                                         dev_inv,
                                                                         req,
                                                                         cache,
                                                                         ptn,
                                                                         batch_idx,
                                                                         out);
      check(cudaGetLastError(), "resolve_in_buffer_kernel [launch]");
      synchronize("cudaStreamSynchronize [resolve_in_buffer]");

      cache = out->cache;

      return out->result;
    }
#endif // __CUDACC__

    static void synchronize(const char* msg) {
      check(cudaStreamSynchronize(get_stream()), msg);
    }

    static cudaStream_t get_stream() {
      if (m_stream == nullptr) {
        check(cudaStreamCreate(&m_stream), "cudaStreamCreate");
      }

      return m_stream;
    }

  private:
#ifdef __CUDACC__
    /**
     * A device-side copy of the MetadataInventory.
     *
     * @note The copy is updated each time the host-side copy changes. The tracking is
     *       done using the revision numbers which are globally unique (host-side) for
     *       all metadata inventories.
     */
    template <class InventoryT>
    static const InventoryT* device_inventory(const InventoryT& inv) {
      static_assert(std::is_trivially_copyable_v<InventoryT>,
                    "MetadataInventory must be trivally copyable to use on GPU.");

      struct DeviceCopy {
        void* ptr { nullptr };
        std::uint64_t revision { 0 };
      };
      static std::unordered_map<const void*, DeviceCopy> copies;

      DeviceCopy& copy { copies[&inv] };
      if (copy.ptr == nullptr) {
        check(cudaMemcpyAsync(copy.ptr,
                              &inv,
                              sizeof(InventoryT),
                              cudaMemcpyHostToDevice,
                              get_stream()),
              "cudaMemcpyAsync [device_inventory]");
        copy.revision = inv.revision();
      }

      return static_cast<const InventoryT*>(copy.ptr);
    }

    /**
     * Host memory (pinned) the resolution kernel puts results in.
     */
    static impl::DeviceResolved* resolved_slot() {
      static impl::DeviceResolved* slot { nullptr };
      if (slot == nullptr) {
        void* ptr { nullptr };
        check(cudaMallocHost(&ptr, sizeof(impl::DeviceResolved)),
              "cudaMallocHost [resolved_slot]");
        slot = static_cast<impl::DeviceResolved*>(ptr);
      }

      return slot;
    }
#endif // __CUDACC__

    static void check(cudaError_t err, const char* what) {
      if (err != cudaSuccess) {
        m_logger->critical("{} failed: {}", what, cudaGetErrorString(err));
        std::abort();
      }
    }

    static inline cudaStream_t m_stream { nullptr };
    static inline std::shared_ptr<spdlog::logger> m_logger;
  };

} // namespace sbio

#endif // SBIO_EXECUTION_CUDA_HH
