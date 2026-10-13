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
#include "sbio/execution/composed.hh"
#include "sbio/execution/serial.hh"
#include "sbio/execution/threaded.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/storage/cuda_device.hh"
#include "sbio/storage/host_buffer.hh"

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cuda_runtime.h>

#include <algorithm>
#include <atomic>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace sbio {
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
     * A reference to buffer memory from the device.
     */
    template <MemorySpace Space>
    struct BufferRef {
      SBIO_HD static constexpr MemorySpace memory_space() { return Space; }

      SBIO_HD void* ptr() const { return m_ptr; }

      SBIO_HD std::size_t size() const { return m_size; }

      SBIO_HD void set_memory(void* ptr, std::size_t size) {
        m_ptr = ptr;
        m_size = size;
      }

      void* m_ptr { nullptr };
      std::size_t m_size { 0 };
    };

    template <class Policy>
    struct DeviceSnapshotPolicy {
      template <class Descriptor>
      using BufferTypeFor =
        BufferRef<Policy::template BufferTypeFor<Descriptor>::memory_space()>;
    };

    template <class StorageT>
    struct DeviceSnapshot;

    template <typename... Descriptors, class Policy>
    struct DeviceSnapshot<Storage<RequirementsList<Descriptors...>, Policy>> {
      using policy = Policy;
      using type = Storage<RequirementsList<Descriptors...>, DeviceSnapshotPolicy<Policy>>;

      static type take(Storage<RequirementsList<Descriptors...>, Policy>& storage) {
        type snapshot {};
        auto copy_ref = [&](auto DescTag) {
          using Descriptor = typename decltype(DescTag)::type;

          auto& buf { storage.template get<Descriptor>() };
          snapshot.template get_member<Descriptor>().set_memory(buf.ptr(), buf.size());
        };

        ( (copy_ref(std::type_identity<Descriptors> {})), ... );

        return snapshot;
      }
    };

    /**
     * Perform a DataRequest lookup directly on the device.
     *
     * This assumes the host-code was not GPU-optimized (synchronous direct lookup)
     * so it should be run from a single CUDA thread.
     */
    template <class FTraits, class Policy, class SnapshotT, class InventoryT>
    __global__ void resolve_in_buffer_kernel(SnapshotT storage,
                                             const Inventory* inv,
                                             typename FTraits::DataRequest req,
                                             PayloadOffsetCache cache,
                                             typename FTraits::DataAccessPtn ptn,
                                             std::size_t batch_idx,
                                             DeviceResolved* out) {
      StorageView<SnapshotT, Policy> sv(storage);
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
   * @note If the MemoryManger is configured, through `Config::devices`, to use multiple
   *       GPUs, the parallel units will be spread over them.
   */
  class CudaMemory {
  public:
    using UnitOrdinalFn = std::size_t (*)();

    template <typename Descriptor>
    using PlacementFor = std::conditional_t<
      std::is_same_v<typename Descriptor::role, roles::Data>,
      CudaMirroredBuffer,
      std::conditional_t<
        std::is_same_v<typename Descriptor::role, roles::Table>,
        CudaPinnedBuffer,
        HostBuffer
      >
    >;

    struct Config {
      int device { 0 };            ///< Which GPU (CUDA device), if devices is empty
      std::vector<int> devices {}; ///< Which GPUs to spread units over
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

      for (int device : config.devices) {
        if (device < 0 || device >= count) {
          error = "A requested CUDA device (of devices) does not exist! Check configuration!";
          return false;
        }
      }

      return true;
    }

    /**
     * Setup a spdlog logger.
     */
    static void init_logger() {
      spdlog::cfg::load_env_levels("SBIO_LOG_LEVEL");
      std::shared_ptr<spdlog::logger> logger = spdlog::get("sbio::CudaExecution");
      if (!logger) {
        m_logger = spdlog::stdout_color_mt("sbio::CudaExecution");
      } else {
        m_logger = logger;
      }
    }

    static void configure_impl(const Config& config) {
      init_logger();

      m_device.store(config.device, std::memory_order_release);
      m_devices = config.devices;
      bind_device();

      get_stream();
    }

    /**
     * Use the DistributionManager's numbering parallel units.
     */
    static void use_unit_ordinal(UnitOrdinalFn unit_ordinal) {
      m_unit_ordinal.store(unit_ordinal, std::memory_order_release);
    }

    template <class IO>
    static void use_io() {
      if constexpr (reads_to_device<IO>() &&
                    requires (void* ptr, std::size_t size) { IO::register_buffer(ptr, size); }) {
        m_register_buffer.store(&IO::register_buffer, std::memory_order_release);
      } else {
        m_register_buffer.store(nullptr, std::memory_order_release);
      }

      if constexpr (reads_to_device<IO>() &&
                    requires (void* ptr) { IO::deregister_buffer(ptr); }) {
        m_deregister_buffer.store(&IO::deregister_buffer, std::memory_order_release);
      } else {
        m_deregister_buffer.store(nullptr, std::memory_order_release);
      }
    }

    template <class Buffer>
    static void allocate_buffer(Buffer& buf, std::size_t size) {
      if constexpr (std::is_same_v<Buffer&, CudaMirroredBuffer&>) {
        bind_device();

        void* dev { nullptr };
        void* host { nullptr };

        check(cudaMalloc(&dev, size), "cudaMalloc [Data]");
        check(cudaMallocHost(&host, size), "cudaMallocHost [Data]");

        buf.set_memory(dev, size);
        buf.set_host_memory(host);

        auto register_buffer { m_register_buffer.load(std::memory_order_acquire) };
        if (register_buffer) { if (!register_buffer(dev, size)) {
            m_logger->warn("Could not register the Data buffer with IO policy!");
          }
        }
      } else if constexpr (std::is_same_v<Buffer&, CudaPinnedBuffer&>) {
        bind_device();

        void* pinned { nullptr };
        check(cudaMallocHost(&pinned, size), "cudaMallocHost [Table]");

        buf.set_memory(pinned, size);
      } else if constexpr (std::is_same_v<Buffer&, HostBuffer&>) {
        buf.set_memory(new char[size], size);
      }
    }

     /**
     * Release the memory of a buffer allocated by `allocate_buffer`.
     *
     * @note If there are errors, they're ignored. This function is likely to
     *       run late in the process lifecycle (e.g. CUDA shutting down), and things
     *       are going to be released anyway.
     */
    template <class Buffer>
    requires (std::is_same_v<Buffer, CudaMirroredBuffer> ||
              std::is_same_v<Buffer, CudaPinnedBuffer> ||
              std::is_same_v<Buffer, HostBuffer>)
    static void release_buffer(Buffer& buf) {
      if constexpr (std::is_same_v<Buffer, CudaMirroredBuffer>) {
        if (buf.ptr() != nullptr) {
          if (auto deregister_buffer { m_deregister_buffer.load(std::memory_order_acquire) }) {
            deregister_buffer(buf.ptr());
          }
          static_cast<void>(cudaFree(buf.ptr()));
        }

        if (buf.host_ptr() != nullptr) {
          static_cast<void>(cudaFreeHost(buf.host_ptr()));
        }

        buf.set_memory(nullptr, 0);
        buf.set_host_memory(nullptr);
      } else if constexpr (std::is_same_v<Buffer, CudaPinnedBuffer>) {
        if (buf.ptr() != nullptr) {
          static_cast<void>(cudaFreeHost(buf.ptr()));
        }

        buf.set_memory(nullptr, 0);
      } else {
        delete[] static_cast<char*>(buf.ptr());
        buf.set_memory(nullptr, 0);
      }
    }

    /**
     * Get a host view of a device buffer (using the replica in the buffer).
     */
    template <typename Buffer>
    static void* acquire_broker_view_impl(Buffer& buf) {
      if constexpr (requires { buf.host_ptr(); }) {
        if (buf.host_stale() > 0) {
          bind_device();
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
          bind_device();
          check(cudaMemcpyAsync(buf.ptr(), view, nbytes, cudaMemcpyHostToDevice, get_stream()),
                "cudaMemcpyAsync [publish view]");
        } else if (view == buf.ptr()) {
          // The device buffer is current, but the host replica is not so will need
          // to be copied over
          buf.set_host_stale(nbytes);
        }
      }
    }

    /**
     * Synchronize the stream so that results are returned only once copy completes.
     */
    static void after_get_data() {
      synchronize("cudaStreamSynchronize [get_data]");
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
      using Snapshot = impl::DeviceSnapshot<StorageT>;
      using Policy = typename Snapshot::policy;

      bind_device();
      const InventoryT* dev_inv { device_inventory<Policy>(inv) };
      impl::DeviceResolved* out { resolved_slot<Policy>() };

      impl::resolve_in_buffer_kernel<FTraits, Policy><<<1, 1, 0, get_stream()>>>(Snapshot::take(storage),
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
      bind_device();
      check(cudaStreamSynchronize(get_stream()), msg);
    }

    static cudaStream_t get_stream() {
      static thread_local cudaStream_t stream { nullptr };
      if (stream == nullptr) {
        bind_device();
        check(cudaStreamCreate(&stream), "cudaStreamCreate");
      }

      return stream;
    }

    /**
     * The device associated to the calling parallel unit.
     *
     * When CudaMemory is configured with `devices` (as opposed to just `device`)
     * a given parallelunit will be assigned a particular device to use from those
     * available. Otherwise, every parallel unit just uses `device`.
     *
     * @note The calculation is simple: `devices[ordinal % devices.size()]`
     *       `ordinal` is provided by the DistributionManager for numbering
     *       each of the parallel units. (E.g., it could be an MPI rank).
     */
    static int unit_device() {
      if (m_devices.empty()) {
        return m_device.load(std::memory_order_acquire);
      }

      const UnitOrdinalFn unit_ordinal {
        m_unit_ordinal.load(std::memory_order_acquire)
      };

      const std::size_t ordinal { unit_ordinal ? unit_ordinal() : 0 };

      return m_devices[ordinal % m_devices.size()];
    }

    /**
     * Make the calling unit's device the current device for the host htread.
     */
    static void bind_device() {
      const int device { unit_device() };
      thread_local int bound { -1 };
      if (bound != device) {
        check(cudaSetDevice(device), "cudaSetDevice");
        bound = device;
      }
    }

  private:
#ifdef __CUDACC__
    /**
     * A device-side copy of the MetadataInventory.
     *
     * @note The copy is updated each time the host-side copy changes. The tracking is
     *       done using the revision numbers which are globally unique (host-side) for
     *       all metadata inventories.
     * @note Each parallel unit has its own set of copies. These exist and are updated
     *       on that units assigned device, using its own stream, so that different
     *       units dont overwrite copies from each other.
     */
    template <class Policy, class InventoryT>
    static const InventoryT* device_inventory(const InventoryT& inv) {
      static_assert(std::is_trivially_copyable_v<InventoryT>,
                    "MetadataInventory must be trivally copyable to use on GPU.");

      struct DeviceCopy {
        void* ptr { nullptr };
        std::uint64_t revision { 0 };
      };
      static typename Policy:: template SegmentState<
        std::unordered_map<const void*, DeviceCopy>
      > unit_copies {};

      DeviceCopy& copy { unit_copies.get()[&inv] };
      if (copy.ptr == nullptr) {
        check(cudaMalloc(&copy.ptr, sizeof(InventoryT)), "cudaMalloc [device_inventory]");
      }

      if (copy.revision != inv.revision()) {
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
    template <class Policy>
    static impl::DeviceResolved* resolved_slot() {
      static typename Policy::template SegmentState<impl::DeviceResolved*> slots {};

      impl::DeviceResolved*& slot { slots.get() };
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
        if (m_logger) {
          m_logger->critical("{} failed: {}", what, cudaGetErrorString(err));
        }
        std::abort();
      }
    }

    using RegisterFn = bool (*)(void*, std::size_t);
    using DeregisterFn = void (*)(void*);

    static inline std::atomic<int> m_device { 0 };
    static inline std::vector<int> m_devices {}; ///< Only written once at configure
    static inline std::atomic<UnitOrdinalFn> m_unit_ordinal { nullptr };
    static inline std::atomic<RegisterFn> m_register_buffer { nullptr };
    static inline std::atomic<DeregisterFn> m_deregister_buffer { nullptr };
    static inline std::shared_ptr<spdlog::logger> m_logger;
  };

  using CudaExecution = Composed<SerialDistribution, CudaMemory>;

  using ThreadedCudaExecution = Composed<ThreadedDistribution, CudaMemory>;
} // namespace sbio

#endif // SBIO_EXECUTION_CUDA_HH
