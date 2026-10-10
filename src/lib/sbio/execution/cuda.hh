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
#include "sbio/core/roles.hh"
#include "sbio/core/storage.hh"
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
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <bitset>
#include <type_traits>

namespace sbio {
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
   * @note This EPolicy doesnt actually support direct reads into GPU memory at the
   *       moment!
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
     * IO currently gets staged through a pinned host buffer for this EPolicy.
     */
    template <typename IO>
    static constexpr IOCapability required_io_capabilities_impl() {
      static_assert(has_capabilities(io_capabilities<IO>(), IOCapability::ReadToPinned),
                    "CudaExecution reads foirst to pinned host memory.");
      return IOCapability::ReadToPinned;
    }

    void configure_impl(const Config& config) {
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


      return allocate_impl_helper(Requirements{}, request);
    }

    template <typename... Descriptors, class FTraits>
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
