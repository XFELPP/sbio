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

#ifndef SBIO_EXECUTION_COMPOSED_HH
#define SBIO_EXECUTION_COMPOSED_HH

#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/types.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/storage/per_unit_buffer.hh"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace sbio {
  template <class Dist, class Mem>
  class Composed;

  namespace impl {
    /**
     * Release a set of buffers on destruction (via Storage owner.)
     */
    class ReleaseList {
    public:
      ReleaseList() = default;

      // No copy
      ReleaseList(const ReleaseList& other) = delete;
      ReleaseList& operator=(const ReleaseList& other) = delete;

      ~ReleaseList() {
        for (auto& release : m_releases) {
          release();
        }
      }

      void add(std::function<void()> release) {
        m_releases.push_back(std::move(release));
      }

    private:
      std::vector<std::function<void()>> m_releases {};
    };

    /**
     * A composed configuration is the distribution manager's config + memory manager's.
     *
     * @note The memory manager config is optional.
     */
    template <class Dist, class Mem>
    struct ComposedConfig {
      using type = typename Dist::Config;
    };

    template <class Dist, class Mem>
    requires requires { typename Mem::Config; }
    struct ComposedConfig<Dist, Mem> {
      struct type : Dist::Config, Mem::Config {};
    };

    /**
     * Retrieve the buffer type for the specified scope.
     *
     * @note The distribution manager chooses the scope, the memory manager decides
     *       the placement as a result.
     */
    template <BufferScope Scope, class Policy, class Dist, class Mem, class Descriptor>
    struct ScopedBuffer {
      using type = typename Mem::template PlacementFor<Descriptor>;
    };

    template <class Policy, class Dist, class Mem, class Descriptor>
    struct ScopedBuffer<BufferScope::PerUnit, Policy, Dist, Mem, Descriptor> {
      using type = PerUnitBuffer<typename Mem::template PlacementFor<Descriptor>, Policy>;
    };

    template <class Policy, class Dist, class Mem, class Descriptor>
    struct ScopedBuffer<BufferScope::NodeShared, Policy, Dist, Mem, Descriptor> {
      using type = typename Dist::template NodeSharedBuffer<Descriptor>;
    };

    /**
     * Retreive the scope of a buffer for the specified role.
     *
     * @note The distribution manager decides on the scope, with the default being
     *       BufferScope::Shared
     */
    template <class Dist, class Descriptor>
    constexpr BufferScope scope_of() {
      if constexpr (requires { Dist::template scope_for<Descriptor>(); }) {
        return Dist::template scope_for<Descriptor>();
      } else {
        return BufferScope::Shared;
      }
    }

    /**
     * Return the parallelization methods as the union of both managers.
     */
    template <class Dist, class Mem>
    constexpr auto parallel_support() {
      using Bits = std::remove_cv_t<decltype(Dist::ParallelSupport)>;

      unsigned long long bits { 0 };
      for (std::size_t method = 0; method < Bits {}.size(); ++method) {
        bool supported { Dist::ParallelSupport[method] };

        if constexpr (requires { Mem::ParallelSupport[method]; }) {
          supported = supported || Mem::ParallelSupport[method];
        }

        if (supported) {
          bits |= (1ull << method);
        }
      }

      return Bits { bits };
    }

    // Windows MSVC NON-CONFORMANT AND NEEDS THSI CONCEPT TO WOKR WITH END_ITERATION_IMPL BELOW
    template <class Dist, class FTraits, class IterState, class IndexTrigger>
    concept HasTemplateEndIter = requires (IterState& state,
                                           typename FTraits::StepIdxType& max_capacity,
                                           IndexTrigger&& trigger) {
      Dist::template end_iteration_impl<FTraits>(state,
                                                 max_capacity,
                                                 std::forward<IndexTrigger>(trigger));
    };
  } // namespace impl

  /**
   * An Execution policy allowing composition of a DistributionManager and MemoryManager.
   *
   * A DistributionManager should be setup to decide how steps are passed out, and how
   * state is synchronized. It should cover the following APIs from Execution:
   * - `next`, `next_batch`, `end_iteration` : For step iteration
   * - `should_index`, `pre_update`, `post_update` : For synchronization
   * - `get_data` : For data fetching and resolution
   *
   * A MemoryManager should be setup to decide where to place the buffers and how to
   * handle moving buffers between memory spaces. It should cover the following APIs:
   * - `allocate_storage` : Initial allocation and placement.
   * - `acquire_broker_view`, `publish_broker_view`, `resolve_in_buffer`,
   *   `result_memory_space` : Memory spaces and moving between them
   *
   * Additionally, the following APIs are unique to the composition system:
   * - `scope_for` on the DistributionManager
   * - `PlacementFor` on the MemoryManager
   *
   * The Composed class itself does nothing but forward directly to either manager.
   * It is purely a glue structure and does not implement any behaviour.
   *
   * @tparam Dist The distribution manager.
   * @tparam Mem The memory manager.
   */
  template <class Dist, class Mem>
  class Composed : public Execution<Composed<Dist, Mem>> {
  public:
    using DistributionManager = Dist;
    using MemoryManager = Mem;

    using Config = typename impl::ComposedConfig<Dist, Mem>::type;
    using IterationState = typename Dist::IterationState;

    template <class T>
    using SegmentState = typename Dist::template SegmentState<T>;

    template <class Descriptor>
    using BufferTypeFor = typename impl::ScopedBuffer<
      impl::scope_of<Dist, Descriptor>(),
      Composed,
      Dist,
      Mem,
      Descriptor
    >::type;

    static constexpr auto ParallelSupport { impl::parallel_support<Dist, Mem>() };

    /// --- Configuration --- ///

    static void configure_impl(const Config& config) {
      // Run any configuration needed by either manager
      if constexpr (requires { Dist::configure_impl(config); }) {
        Dist::configure_impl(config);
      }

      connect_managers();

      if constexpr (requires { Mem::configure_impl(memory_config(config)); }) {
        Mem::configure_impl(memory_config(config));
      }
    }

    static bool validate_environment_impl(const Config& config, const char*& error) {
      // Run any environment validation needed by eitehr manager
      if constexpr (requires { Dist::validate_environment_impl(config, error); }) {
        if (!Dist::validate_environment_impl(config, error)) {
          return false;
        }
      }

      if constexpr (requires { Mem::validate_environment_impl(memory_config(config), error); }) {
        if (!Mem::validate_environment_impl(memory_config(config), error)) {
          return false;
        }
      }

      return true;
    }

    /// --- Storage (Memory Manager) --- ///

    template <IsRequirementsList Requirements, class IO, class FTraits>
    requires FormatTraits<FTraits, IO, Composed>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      if constexpr (requires { Dist::init_logger(); }) {
        Dist::init_logger();
      }

      if constexpr (requires { Mem::init_logger(); }) {
        Mem::init_logger();
      }

      if constexpr (requires { Mem::template use_io<IO>(); }) {
        Mem::template use_io<IO>();
      }

      connect_managers();

      return allocate_helper(Requirements {}, request);
    }

    /**
     * Refer to the memory manager to allocate a buffer.
     */
    template <class Buffer>
    static void allocate_buffer(Buffer& buf, std::size_t size) {
      Mem::allocate_buffer(buf, size);
    }

    template <class Buffer>
    static void release_buffer(Buffer& buf)
    requires requires { Mem::release_buffer(buf); } {
      Mem::release_buffer(buf);
    }

    // NOTE: The syntax below is very ugly... but all the requires requies are needed
    //       at least for any functions not returning void. Leaving them on all is
    //       simplest.

    /// --- Distribution (Distribution Manager) --- ///
    // All these functions forward directly to DistributionManager without changes

    static bool should_index_impl()
    requires requires { Dist::should_index_impl(); } {
      return Dist::should_index_impl();
    }

    template <class FTraits>
    static bool should_process_impl(typename FTraits::StepIdxType& step_idx)
    requires requires { Dist::template should_process_impl<FTraits>(step_idx); } {
      return Dist::template should_process_impl<FTraits>(step_idx);
    }

    template <class Role, class StorageT>
    static void pre_update_impl(StorageT& storage)
    requires requires { Dist::template pre_update_impl<Role, StorageT>(storage); } {
      Dist::template pre_update_impl<Role, StorageT>(storage);
    }

    template <class Role, class StorageT, class SyncT>
    static void post_update_impl(StorageT& storage, SyncT&& sync_vars, IOStatus status)
    requires requires { Dist::template post_update_impl<Role, StorageT>(storage,
                                                                        std::forward<SyncT>(sync_vars),
                                                                        status);
    } {
      Dist::template post_update_impl<Role, StorageT>(storage,
                                                      std::forward<SyncT>(sync_vars),
                                                      status);
    }

    template <class FTraits, class FetchCBType, class GetCBType>
    static IOStatus get_data_impl(typename FTraits::StepIdxType step_idx,
                                  FetchCBType&& unit_fetcher,
                                  std::size_t num_fetches,
                                  GetCBType&& unit_get_data,
                                  std::size_t num_accesses) {
      IOStatus status { IOStatus::Success };
      if constexpr (requires {
          Dist::template get_data_impl<FTraits>(step_idx,
                                                std::forward<FetchCBType>(unit_fetcher),
                                                num_fetches,
                                                std::forward<GetCBType>(unit_get_data),
                                                num_accesses);
        }) {
        status = Dist::template get_data_impl<FTraits>(step_idx,
                                                       std::forward<FetchCBType>(unit_fetcher),
                                                       num_fetches,
                                                       std::forward<GetCBType>(unit_get_data),
                                                       num_accesses);
      } else {
        status = Base::template fetch_then_get<FTraits>(step_idx,
                                                        std::forward<FetchCBType>(unit_fetcher),
                                                        num_fetches,
                                                        std::forward<GetCBType>(unit_get_data),
                                                        num_accesses);
      }

      if constexpr (requires { Mem::after_get_data(); }) {
        Mem::after_get_data();
      }

      return status;
    }

    template <class FTraits, class FetchCBType, class GetCBType>
    static IOStatus get_data_steps_impl(const std::initializer_list<typename FTraits::StepIdxType> steps,
                                        FetchCBType&& unit_fetcher,
                                        std::size_t num_fetches,
                                        GetCBType&& unit_get_data,
                                        std::size_t num_accesses) {
      IOStatus status { IOStatus::Success };
      if constexpr (requires {
          Dist::template get_data_steps_impl<FTraits>(steps,
                                                      std::forward<FetchCBType>(unit_fetcher),
                                                      num_fetches,
                                                      std::forward<GetCBType>(unit_get_data),
                                                      num_accesses);
        }) {
        status = Dist::template get_data_steps_impl<FTraits>(steps,
                                                             std::forward<FetchCBType>(unit_fetcher),
                                                             num_fetches,
                                                             std::forward<GetCBType>(unit_get_data),
                                                             num_accesses);
      } else {
        status = Base::template fetch_then_get_steps<FTraits>(steps,
                                                              std::forward<FetchCBType>(unit_fetcher),
                                                              num_fetches,
                                                              std::forward<GetCBType>(unit_get_data),
                                                              num_accesses);
      }

      if constexpr (requires { Mem::after_get_data(); }) {
        Mem::after_get_data();
      }

      return status;
    }

    template <class FTraits, class IterState, class IndexTrigger>
    static typename FTraits::StepIdxType next_impl(IterState& state,
                                                   typename FTraits::StepIdxType& max_capacity,
                                                   IndexTrigger&& trigger)
    requires requires { Dist::template next_impl<FTraits>(state,
                                                          max_capacity,
                                                          std::forward<IndexTrigger>(trigger));
    } {
      return Dist::template next_impl<FTraits>(state,
                                               max_capacity,
                                               std::forward<IndexTrigger>(trigger));
    }

    template <class FTraits, class IterState, class IndexTrigger>
    static StepBatch<typename FTraits::StepIdxType>
    next_batch_impl(IterState& state,
                    std::size_t batch_size,
                    typename FTraits::StepIdxType& max_capacity,
                    IndexTrigger&& trigger)
    requires requires { Dist::template next_batch_impl<FTraits>(state,
                                                                batch_size,
                                                                max_capacity,
                                                                std::forward<IndexTrigger>(trigger));
    } {
      return Dist::template next_batch_impl<FTraits>(state,
                                                     batch_size,
                                                     max_capacity,
                                                     std::forward<IndexTrigger>(trigger));
    }

    // DIFFERENT BECAUSE WINDOWS MSVC BAD CONFORMANCE
    template <class FTraits, class IterState, class IndexTrigger>
    static void end_iteration_impl(IterState& state,
                                   typename FTraits::StepIdxType& max_capacity,
                                   IndexTrigger&& trigger)
    requires impl::HasTemplateEndIter<Dist, FTraits, IterState, IndexTrigger> {
      Dist::template end_iteration_impl<FTraits>(state,
                                                 max_capacity,
                                                 std::forward<IndexTrigger>(trigger));
    }

    template <class IterState>
    static void end_iteration_impl(IterState& state)
    requires requires { Dist::end_iteration_impl(state); } {
      Dist::end_iteration_impl(state);
    }

    /// --- Memory, and MemorySpace Handling (MemoryManager) --- ///
    // All these functions forward directly to MemoryManager without changes
    // With a few minor exceptions when checking capabilities

    static constexpr MemorySpace result_memory_space_impl()
    requires requires { Mem::result_memory_space_impl(); } {
      return Mem::result_memory_space_impl();
    }

    template <typename IO>
    static constexpr IOCapability required_io_capabilities_impl()
    requires requires { Mem::template required_io_capabilities_impl<IO>(); } {
      constexpr bool threaded {
        ParallelSupport[static_cast<std::size_t>(ParallelizationMethods::THREADS)]
      };

      if constexpr (threaded) {
        return
          Mem::template required_io_capabilities_impl<IO>() |
          IOCapability::ConcurrentReads;
      } else {
        return Mem::template required_io_capabilities_impl<IO>();
      }
    }

    template <typename Buffer>
    static void* acquire_broker_view_impl(Buffer& buf)
    requires requires { Mem::acquire_broker_view_impl(buf); } {
      return Mem::acquire_broker_view_impl(buf);
    }

    template <typename Buffer>
    static void publish_broker_view_impl(Buffer& buf, void* view, std::size_t nbytes)
    requires requires { Mem::publish_broker_view_impl(buf, view, nbytes); } {
      Mem::publish_broker_view_impl(buf, view, nbytes);
    }

    template <class FTraits, class IO, class StorageT, class InventoryT>
    static DataResult resolve_in_buffer_impl(StorageT& storage,
                                             const InventoryT& inv,
                                             const typename FTraits::DataRequest& req,
                                             PayloadOffsetCache& cache,
                                             typename FTraits::DataAccessPtn ptn,
                                             std::size_t batch_idx)
    requires requires { Mem::template resolve_in_buffer_impl<FTraits, IO>(storage,
                                                                          inv,
                                                                          req,
                                                                          cache,
                                                                          ptn,
                                                                          batch_idx);
    } {
      return Mem::template resolve_in_buffer_impl<FTraits, IO>(storage,
                                                               inv,
                                                               req,
                                                               cache,
                                                               ptn,
                                                               batch_idx);
    }

  private:
    using Base = Execution<Composed<Dist, Mem>>;

    /**
     * Tell MemoryManager which parallel unit is the calling one.
     *
     * @note The MemoryManager can use this information to distribute buffers
     *       effectively. E.g., if buffers need to be placed on devices (GPUs).
     */
    static void connect_managers() {
      if constexpr (requires { Mem::use_unit_ordinal(&Dist::unit_ordinal); }) {
        Mem::use_unit_ordinal(&Dist::unit_ordinal);
      }
    }

    static const auto& memory_config(const Config& config) {
      if constexpr (requires { typename Mem::Config; }) {
        return static_cast<const typename Mem::Config&>(config);
      } else {
        return config;
      }
    }

    /**
     * Allocate a node-shared buffer with the DistributionManager.
     *
     * @note A function template, so `Dist::allocate_node_shared` is only looked up for
     *       distributions with node-shared buffers (clang checks it in the discarded
     *       branch of `allocate_helper`'s lambda otherwise).
     */
    template <class Buffer>
    static void allocate_node_shared_buffer(Buffer& buf, std::size_t sz) {
      Dist::allocate_node_shared(buf, sz);
    }

    template <typename... Descriptors, class FTraits>
    static auto allocate_helper(RequirementsList<Descriptors...>,
                                const AllocationRequest<FTraits>& request) {
      Storage<RequirementsList<Descriptors...>, Composed> s;
      std::size_t i { 0 };

      // The memory of shared buffers is released by the MemorMan
      // when the last copy of the Storage is destroyed
      auto owner { std::make_shared<impl::ReleaseList>() };

      auto make_buffer = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;

        std::size_t sz { std::max(Descriptor::min_size, request.size_requests[i++]) };
        auto& buf { s.template get_member<Descriptor>() };

        constexpr BufferScope scope { impl::scope_of<Dist, Descriptor>() };
        if constexpr (scope == BufferScope::NodeShared) {
          static_assert(Mem::template PlacementFor<Descriptor>::memory_space() == MemorySpace::Host,
                        "Node-shared buffers live in host memory and the memory manager must "
                        "place this role in host memory.");
          allocate_node_shared_buffer(buf, sz);
        } else if constexpr (scope == BufferScope::PerUnit) {
          buf.set_memory(nullptr, sz); // Allocated per unit, on first access
        } else {
          Mem::allocate_buffer(buf, sz);

          if constexpr (requires { Mem::release_buffer(buf); }) {
            owner->add([released = buf]() mutable { Mem::release_buffer(released); });
          }
        }
      };

      ( (make_buffer(std::type_identity<Descriptors> {})), ... );

      if constexpr (requires { s.set_owner(owner); }) {
        s.set_owner(std::move(owner));
      }

      return s;
    }
  };
} // namespace sbio

#endif // SBIO_EXECUTION_COMPOSED_HH
