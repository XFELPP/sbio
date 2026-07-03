#ifndef SBIO_CORE_EXECUTION_HH
#define SBIO_CORE_EXECUTION_HH

#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/formats/format_traits.hh"

#include <concepts>
#include <iostream>
#include <type_traits>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {

  template <class FTraits, class DataSource>
  struct ExecutionContext {
  public:
    using StepIdx = typename FTraits::StepIdxType;

    ExecutionContext(DataSource& ds)
      : m_ds(ds)
    {}

    SBIO_HD IOStatus prepare() { return m_ds.discover_metadata(); }

    SBIO_HD bool next() {
      m_current_step = m_ds.next();

      return m_current_step != FTraits::ExhaustedSentinel;
    }

    template <class DetType, class StepIdx>
    SBIO_HD auto read(DetType& det,
                      const char* alg_name,
                      const char* field_name,
                      StepIdx& idx) {
      return make_source([&det, &idx, alg_name, field_name]() {
        return det.get_data(idx, alg_name, field_name);
      });
    }

    SBIO_HD StepIdx& current_step() {
      return m_current_step;
    }

  private:
    DataSource& m_ds;
    StepIdx m_current_step;
  };

  /**
   * The execution model provides hooks for managing a data broker's state machine
   * or lifecycle stages. It also provides two entry points for management of the
   * higher level abstractions.
   *
   * In general, the stages for a data stream are:
   * 1. Resource allocation
   * 2. Connection
   * 3. Metadata discovery
   * 4. Indexing (if applicable) - to make traversal faster
   * 5. Data streaming and retrieval
   *
   * The IExecution defines a model that allows for abstracting the control of these
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
   * 4. Indexing stage contains 1 hook: A switch to control whether the broker
   *    should index at all.
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
   * Beyond the raw data stream broker, the model controls two features for the
   * higher level abstractions provided by sbio.
   * 1. For the `Detector` abstraction:
   * - The `get_data` hook determines how a Detector will distribute read and
   *   retrieval operations among potentially multiple brokers. A Detector may
   *   be gathering data together from multiple streams. This hook can control
   *   whether you read from all first, and then inspect for data, or proceed
   *   in order reading and inspecting, as an example.
   *
   * 2. For the `DataSource` abstraction: Finally, at the top of the abstraciton
   *    hiearachy, the execution model can determine how a DataSource distributes
   *    event/step indexing among parallel resources. (In whatever unit makes sense
   *    for the file format).
   */
  template <typename Derived>
  class IExecution {
  public:
    /**
     * The default storage buffers for all roles will be simple host buffers.
     */
    template <typename Descriptor>
    using BufferTypeFor = HostBuffer;

    // --- Data broker level policies --- //
    // ---------------------------------- //

    /**
     * Controls the allocation of storage.
     *
     * @param[in] request A request from the stream broker for resources for each of the
     *            buffers it will need.
     * @return storage The storage object containing all buffers.
     */
    template <FormatTraits FTraits>
    SBIO_HD static auto allocate_storage(AllocationRequest<FTraits>& request) {
      return Derived::template allocate_storage_impl<FTraits>(request);
    }

    /**
     * The stage 2 hooks - provide the mechanism for metadata synchronization across
     * the various parallel processors.
     */
    template <class BrokerType, FormatTraits FTraits>
    SBIO_HD static void pre_discovery(BrokerType& broker,
                                      typename FTraits::MetadataInventory& inv) {
      if constexpr (requires {
          Derived::template pre_discovery_impl<BrokerType, FTraits>(broker, inv);
        }) {
        Derived::template pre_discovery_impl<BrokerType, FTraits>(broker, inv);
      }
    }

    template <class BrokerType, FormatTraits FTraits>
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

    template <FormatTraits FTraits>
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

    // --- `Detector` level policies --- //
    // --------------------------------- //
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
    template <FormatTraits FTraits, class IndexTrigger>
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

    // --- Pipeline level coordination --- //
    // ----------------------------------- //
    template <FormatTraits FTraits, class DataSource, class PipelineDef>
    SBIO_HD static auto dispatch(DataSource& ds, PipelineDef&& def) {
      ExecutionContext<FTraits, DataSource> ctx(ds);
      if (ctx.prepare() != IOStatus::Success) {
        return;
      }

      auto pipeline = def(ctx);

      return Derived::run(ctx, pipeline);
    }

    template <class Task>
    SBIO_HD static auto run_pipeline(Task&& task) {
      return task.execute();
    }

    template <class Context, class Pipeline>
    SBIO_HD static void run(Context& ctx, Pipeline& pipeline) {
      while (ctx.next()) {
        //Derived::pre_update(ctx);

        pipeline.transform(ctx);

        //Derived::post_update(ctx);
      }
    }
  };

  class SerialExecution : public IExecution<SerialExecution> {
  public:
    // TODO: Consider moving into Storage/Buffer directly
    //       There may be a need for this...
    // TODO: MAKE SURE TO ALLOW PASSING IN THE REQUESTS FOR DIFFERENT ROLES!
    template <FormatTraits FTraits>
    static auto allocate_storage_impl(const AllocationRequest<FTraits>& request) {
      using Requirements = typename FTraits::BufferRequirements;

      // Requirements is a TypeList<T1, T2, ...>
      return allocate_impl_helper(Requirements{}, request);
    }

    template <typename... Descriptors, FormatTraits FTraits>
    static auto allocate_impl_helper(TypeList<Descriptors...>,
                                     const AllocationRequest<FTraits>& request) {
      Storage<TypeList<Descriptors...>, SerialExecution> s;

      std::size_t i = 0;

      // This default policy will just take the request and satisfy it.
      (([&]() {
        std::size_t final_size = std::max(Descriptors::min_size, request.size_requests[i++]);
        s.template get<Descriptors>()
          .set_memory(new char[final_size], final_size);
      }()),
        ...);
      return s;
    }

    template <class BrokerType, FormatTraits FTraits>
    SBIO_HD static void on_discovery_impl(BrokerType& broker,
                                          typename FTraits::MetadataInventory& inv,
                                          IOStatus status) {
      std::cout << "[Serial] Commiting metadata" << std::endl;
    }

    template <FormatTraits FTraits>
    SBIO_HD static bool should_process_impl(typename FTraits::StepIdxType& step_idx) {
      // Always process
      return true;
    }

    template <FormatTraits FTraits, class IndexTrigger>
    SBIO_HD static typename FTraits::StepIdxType
    next_impl(typename FTraits::StepIdxType& max_capacity, IndexTrigger&& trigger) {
      static typename FTraits::StepIdxType event_idx { 0 };

      if (event_idx >= max_capacity) {
        if (!trigger()) {
          return FTraits::ExhaustedSentinel;
        }

        if (event_idx >= max_capacity) {
          return FTraits::ExhaustedSentinel;
        }
      }

      return event_idx++;
    }

    template <typename Descriptor>
    using BufferTypeFor = HostBuffer;
  };
} // namespace sbio

#endif // SBIO_CORE_EXECUTION_HH
