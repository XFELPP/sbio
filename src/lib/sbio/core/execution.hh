#ifndef SBIO_CORE_EXECUTION_HH
#define SBIO_CORE_EXECUTION_HH

#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/formats/format_traits.hh"

#include <atomic>
#include <concepts>
#include <iostream>
#include <mutex>
#include <type_traits>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  /**
   * The execution model is the central control point for the management of Broker
   * life-cycle (transitioning through state machines), as well as coordination of
   * higher level abstractions.
   *
   * The model provides hooks that control the movement between various states, as
   * well as providing the means to allocate storage solutions appropriate to the
   * model, and the mechanisms to synchronize concurrently running instances.
   *
   * The model provides control over Brokers, BrokerGroups, and DataSources.
   *
   * In general, the stages requiring management at the Broker level are:
   * 1. Resource allocation
   * 2. Connection
   * 3. Metadata discovery
   * 4. Indexing (if applicable) - to make traversal faster
   * 5. Data streaming and retrieval
   *
   * The Execution defines a model that allows for abstracting the control of these
   * stages out of the stream broker itself. It also provides hooks which can be used
   * to synchronize resources before and after state transitions.
   *
   * The Execution model works in tandem with the base class for the broker. The latter
   * ensures that the execution model gets called.
   *
   * The execution model defines the following hooks for customization corresponding
   * to the broker stages above:
   * 1. Allocation strategies are defined by the execution model.
   *    -> A broker implementation may request resources, and the file format will
   *       by definition provide minimums, but the final authority is the execution
   *       model.
   * 2. ***Connection is unmanaged by the execution model.***
   * 3. Metadata discovery includes 2 hooks
   *    -> A pre state transition hook. I.e. control what happens before metadata
   *    -> A post state transition hook. I.e. control what happens immediately
   *       after the metadata discovery step.
   * 4. Indexing stage contains 1 explicit hook: A switch to control whether the broker
   *    should index at all. The synchronizations hooks below will also be used.
   * 5. For data access the policy defines 1 hook that controls whether the
   *    broker should proceed with retrieval (like stage 4 indexing above).
   *
   * In addition to the above, the policy has two powerful functions:
   * - pre_update
   * - post_update
   *
   * These are used for resource synchronization immediately before and after
   * a broker will touch any execution model managed memory. They can be used
   * for synchronization when the policy is managing parallel processing units.
   * E.g. for an MPI model, it may be used to implement fences and syncs on shared
   * buffers across ranks.
   *
   * Note: Hooks are optional (with the exception of storage allocation). They are
   * called via compile time constexpr checks - so a no-op is just a no-op.
   *
   * At the level of the BrokerGroup, the model provides control mechanisms for:
   * 1. Allocation of BrokerGroup memory - principally, tables required for the
   *    management of the various Brokers under the umbrella of the Group.
   * 2. The `get_data` hook determines how a Detector will distribute read and
   *    retrieval operations among potentially multiple brokers. A Detector may
   *    be gathering data together from multiple streams. This hook can control
   *    whether you read from all first, and then inspect for data, or proceed
   *    in order reading and inspecting, as an example.
   *
   * Finally, at the level of the DataSource, the following control points exist:
   * 1. The execution model can determine how a DataSource distributes
   *    event/step indexing among parallel resources. (In whatever unit makes sense
   *    for the file format).
   */
  template <typename Derived>
  class Execution {
  public:
    /**
     * Sub-classes must define the used buffer types.
     */
    template <typename Descriptor>
    using BufferTypeFor = void;

    // --- StreamBroker level policies --- //
    // ----------------------------------- //

    /**
     * Controls the allocation of storage.
     *
     * @param[in] request A request from the stream broker for resources for each of the
     *            buffers it will need.
     * @return storage The storage object containing all buffers.
     */
    template <class IO, class FTraits>
    requires FormatTraits<FTraits, IO, Derived>
    SBIO_HD static auto allocate_storage(AllocationRequest<FTraits>& request) {
      using Requirements = typename FTraits::BrokerBufferRequirements;
      return Derived::template allocate_storage_impl<Requirements, IO, FTraits>(request);
    }

    template <IsTypeList Requirements, class IO, class FTraits>
    requires FormatTraits<FTraits, IO, Derived>
    SBIO_HD static auto allocate_storage(AllocationRequest<FTraits>& request) {
      return Derived::template allocate_storage_impl<Requirements, IO, FTraits>(request);
    }

    /**
     * The stage 2 hooks - provide the mechanism for metadata synchronization across
     * the various parallel processors.
     */
    template <class BrokerType, class FTraits>
    requires FormatTraits<FTraits, typename BrokerType::IOType, Derived>
    SBIO_HD static void pre_discovery(BrokerType& broker,
                                      typename FTraits::MetadataInventory& inv) {
      if constexpr (requires {
          Derived::template pre_discovery_impl<BrokerType, FTraits>(broker, inv);
        }) {
        Derived::template pre_discovery_impl<BrokerType, FTraits>(broker, inv);
      }
    }

    template <class BrokerType, class FTraits>
    requires FormatTraits<FTraits, typename BrokerType::IOType, Derived>
    SBIO_HD static void on_discovery(BrokerType& broker,
                                     typename FTraits::MetadataInventory& inv,
                                     IOStatus status) {
      if constexpr (requires {
          Derived::template on_discovery_impl<BrokerType, FTraits>(broker, inv, status);
        }) {
        Derived::template on_discovery_impl<BrokerType, FTraits>(broker, inv, status);
      }
    }

    /**
     * The stage 3 hook - provide the decision making when doing bulk data processing.
     * Each parallel processor needs to know what to process.
     */
    SBIO_HD static bool should_index() {
      if constexpr (requires { Derived::should_index_impl(); }) {
        return Derived::should_index_impl();
      } else {
        return true;
      }
    }

    template <class FTraits>
    SBIO_HD static bool should_process(typename FTraits::StepIdxType& step_idx) {
      if constexpr (requires { Derived::template should_process_impl<FTraits>(step_idx); }) {
        return Derived::template should_process_impl<FTraits>(step_idx);
      } else {
        return true;
      }
    }

    /**
     * The stage 4 hook - provide any necessary resource management after data
     * fetches.
     */
    SBIO_HD static void on_step(void* buffer) {
      Derived::on_step(buffer);
    }
    /**
     * Stages as yet to be named...
     */

    /**
     * pre_update: Hook called before a broker writes to a metadata/index buffer.
     * In MPI, this implements the window acquisition (e.g., lock or fence).
     */
    template <class Role, class StorageT>
    SBIO_HD static void pre_update(StorageT& storage) {
      if constexpr (requires { Derived::template pre_update_impl<Role, StorageT>(storage); }) {
        Derived::template pre_update_impl<Role, StorageT>(storage);
      }
    }

    /**
     * post_update: Hook called after a broker finishes writing to a buffer.
     * In MPI, this handles the flush and window release (e.g., unlock).
     */
    template <class Role, class StorageT, class SyncT>
    SBIO_HD static void post_update(StorageT& storage, SyncT&& sync_vars, IOStatus status) {
      if constexpr (requires {
          Derived::template post_update_impl<Role, StorageT>(storage, sync_vars, status);
        }) {
        Derived::template post_update_impl<Role, StorageT>(storage,
                                                           std::forward<SyncT>(sync_vars),
                                                           status);
      }
    }

    template <class AccessStyle>
    SBIO_HD static IOStatus execute_read() {
      return Derived::execute_read();
    }

    template <typename Buffer>
    SBIO_HD static void* acquire_broker_view(Buffer& buf) {
      if constexpr (requires { Derived::acquire_broker_view_impl(buf); }) {
        return Derived::acquire_broker_view_impl(buf);
      } else {
        return buf.ptr();
      }
    }

    template <typename Buffer, typename Result>
    SBIO_HD static Result release_broker_view(Buffer& buf, void* broker_view, Result res) {
      if constexpr (requires { Derived::release_broker_view_impl(buf, broker_view, res); }) {
        return Derived::release_broker_view_impl(buf, broker_view, res);
      } else {
        return res;
      }
    }

    // --- BrokerGroup level policies --- //
    // ---------------------------------- //
    template <class IO, class FTraits>
    requires FormatTraits<FTraits, IO, Derived>
    SBIO_HD static auto allocate_group_storage(std::size_t num_segments) {
      using PtrTableRequirements =
          TypeList<BufferDescriptor<TableRole, 0, sizeof(void*)>>;
          //TypeList<BufferDescriptor<TableRole, 0, sizeof(void*), Shareable>>;
      if constexpr (requires {
          Derived::template allocate_group_storage_impl<PtrTableRequirements, FTraits>(num_segments);
      }) {
        return Derived::template allocate_group_storage_impl<PtrTableRequirements, FTraits>(num_segments);
      } else {
        AllocationRequest<FTraits> alloc_request;
        alloc_request.size_requests[0] = sizeof(void*) * num_segments;

        return Derived::template allocate_storage<PtrTableRequirements, IO, FTraits>(alloc_request);
      }
    }

    template <class FTraits, class FetchCBType, class GetCBType>
    SBIO_HD static IOStatus get_data(typename FTraits::StepIdxType step_idx,
                                     FetchCBType&& unit_fetcher,
                                     std::size_t num_fetches,
                                     GetCBType&& unit_get_data,
                                     std::size_t num_accesses) {
      if constexpr (requires {
          Derived::template get_data_impl<FTraits>(step_idx,
                                                   std::forward<FetchCBType>(unit_fetcher),
                                                   num_fetches,
                                                   std::forward<GetCBType>(unit_get_data),
                                                   num_accesses);
        }) {
        return Derived::template get_data_impl<FTraits>(step_idx,
                                                        std::forward<FetchCBType>(unit_fetcher),
                                                        num_fetches,
                                                        std::forward<GetCBType>(unit_get_data),
                                                        num_accesses);
      } else {
        // By default, we will do IO for all units (segments)
        // Then afterwards we will do data retrieval.
        IOStatus status = IOStatus::Success;
        for (std::size_t i = 0; i < num_fetches; ++i) {
          IOStatus s = unit_fetcher(i);
          if (s != IOStatus::Success) {
            status = s;
            break;
          }
        }
        if (status == IOStatus::Success) {
          for (std::size_t i = 0; i < num_accesses; ++i) {
            unit_get_data(i);
          }
        }
        return status;
      }
    }

    // --- DataSource level policies --- //
    // --------------------------------- //
    template <class FTraits, class IndexTrigger>
    SBIO_HD static typename FTraits::StepIdxType
    next(typename FTraits::StepIdxType& max_capacity, IndexTrigger&& trigger) {
      if constexpr (requires {
          Derived::template next_impl<FTraits>(max_capacity,
                                               std::forward<IndexTrigger>(trigger));
        }) {
        return
          Derived::template next_impl<FTraits>(max_capacity,
                                               std::forward<IndexTrigger>(trigger));
      }
    }
  };
} // namespace sbio

#endif // SBIO_CORE_EXECUTION_HH
