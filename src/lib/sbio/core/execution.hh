#ifndef SBIO_CORE_EXECUTION_HH
#define SBIO_CORE_EXECUTION_HH

#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/formats/format_traits.hh"

#ifdef __CUDACC__

#include <cuda/std/bitset>
#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/initializer_list>
#include <cuda/std/utility>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <utility>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif

namespace sbio {
  /**
   * The set of parallelization methods that may be supported by Execution policies.
   *
   * A given policy may support one or more of the following parallelization strategies.
   */
  enum class ParallelizationMethods : hd_std::uint8_t {
    THREADS       = 0, ///< Support for multi-threaded programs
    MPI           = 1, ///< Support for MPI programs
    CUDA          = 2, ///< Support for single GPU-coprocessor with CUDA
    CUDA_MULTIGPU = 3, ///< Support for multiple GPUs (per process) with CUDA
    CUDA_FROMDEV  = 4, ///< Support for being invoked from inside CUDA device code
    NUM_METHODS   = 5
  };

  /**
   * The Execution policy is the central point for the management of IO activity.
   *
   * The policy is used to control the StreamBroker life-cycle (transitioning through
   * state machines), as well as the coordination of higher level abstractions, like
   * the `BrokerGroup` and `DataSource`.
   *
   * The policy has two overarching roles:
   * 1. It builds open the state machine of the StreamBroker to control transitioning
   *    between various states. This leads to the definition of "hooks" that are close
   *    to one-to-one with the StreamBroker states, with some additional functionality
   *    incorporated to address the higher-level abstractions.
   * 2. Coupled with a Storage class the policy provides for synchronization when using
   *    different parallelization strategies by defining a memory consistency model.
   *    When using an Execution policy for the parallelization strategies it supports,
   *    there is a guarantee that the data will be well syncrhonized and the program
   *    will be well formed.
   *
   * For a simpler overview, to address the first role, we can consider that the policy
   * defines hooks (in the form of static functions) to address the following state
   * transitions of a StreamBroker:
   *
   * The execution model defines the following hooks for customization corresponding
   * to the broker stages above:
   * 1. Allocation strategies are defined by the execution model.
   *    -> A broker implementation may request resources, and the file format will
   *       by definition provide minimums, but the final authority is the execution
   *       model.
   * 2. ***Connection is unmanaged by the execution model.***
   * 3. Indexing stage contains 1 explicit hook: A switch to control whether the broker
   *    should index at all. The synchronizations hooks below will also be used.
   * 4. For data access the policy defines 1 hook that controls whether the
   *    broker should proceed with retrieval (like stage 4 indexing above).
   *
   * At the level of the BrokerGroup, the model provides control mechanisms for:
   * 1. Allocation of BrokerGroup memory - principally, tables required for the
   *    management of the various Brokers under the umbrella of the Group.
   * 2. The `get_data` hook determines how a Detector will distribute read and
   *    retrieval operations among potentially multiple brokers. A Detector may
   *    be gathering data together from multiple streams. This hook can control
   *    whether you read from all first, and then inspect for data, or proceed
   *    in order reading and inspecting, as an example. There is an equivalent
   *    function for requesting batches of data.
   *
   * Finally, at the level of the DataSource, the following control points exist:
   * 1. The execution model can determine how a DataSource distributes
   *    event/step indexing among parallel resources. (In whatever unit makes sense
   *    for the file format).
   *
   * In addition to the above logical decisions, to address the second role, the policy
   * has two main functions:
   * - pre_update
   * - post_update
   *
   * These are used for resource synchronization immediately before and after
   * a broker will touch any execution model managed memory. They can be used
   * for synchronization when the policy is managing parallel processing units.
   * E.g. for an MPI model, it may be used to implement fences and syncs on shared
   * buffers across ranks. These are also used in conjunction with the concept of the
   * StorageView which may invoke these automatically.
   *
   * @tparam Derived The type of the sub-class Execution policy.
   */
  template <typename Derived>
  class Execution {
  public:
    /**
     * Sub-classes must define the used buffer types.
     */
    template <typename Descriptor>
    using BufferTypeFor = void;

    struct DefaultConfig {};

    /**
     * The set of parallelization methods supported by the Execution policy.
     */
    static constexpr hd_std::bitset<
      static_cast<hd_std::size_t>(ParallelizationMethods::NUM_METHODS)
    > ParallelSupport { 0x0 };

    /**
     * The memory space that results (via get_data) are returned in.
     */
    static constexpr MemorySpace result_memory_space() {
      if constexpr (requires { Derived::result_memory_space_impl(); }) {
        return Derived::result_memory_space_impl();
      } else {
        // For now, assume return to host memory as default fallback.
        return MemorySpace::Host;
      }
    }

    // --- Execution Policy Configuration (if applicable) --- //
    // ------------------------------------------------------ //

    template <class ConfigT>
    SBIO_HD static void configure(const ConfigT& config) {
      if constexpr (requires { Derived::configure_impl(config); }) {
        Derived::configure_impl(config);
      }
    }

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

    SBIO_HD static void* get_block_from_pool(std::size_t bytes, std::size_t alignment = 16) {
      if constexpr ( requires { Derived::get_block_from_pool_impl(bytes, alignment); }) {
        return Derived::get_block_from_pool_impl(bytes, alignment);
      } else {
        return nullptr;
      }
    }

    /**
     * Decide whether or not this StreamBroker should proceed to the indexing state.
     *
     * @returns Whether the StreamBroker should proceed to the indexing state.
     */
    SBIO_HD static bool should_index() {
      if constexpr (requires { Derived::should_index_impl(); }) {
        return Derived::should_index_impl();
      } else {
        return true;
      }
    }

    /**
     * Decide whether or not this StreamBroker should process this particular step.
     *
     * @tparam FTraits The data format type.
     * @param[in] step_idx The step to make the decision about.
     * @returns Whether the StreamBroker should process this step.
     */
    template <class FTraits>
    SBIO_HD static bool should_process(typename FTraits::StepIdxType& step_idx) {
      if constexpr (requires { Derived::template should_process_impl<FTraits>(step_idx); }) {
        return Derived::template should_process_impl<FTraits>(step_idx);
      } else {
        return true;
      }
    }

    /**
     * An opportunity to provide explicit synchronization immediately after every step.
     *
     * @param[in] buffer The underlying buffer which was filled by the step.
     */
    SBIO_HD static void on_step(void* buffer) {
      Derived::on_step(buffer);
    }

    /**
     * An opportunity to provide explicit synchronization before any Storage changes.
     *
     * @tparam Role The type of the buffer to be modified from the Storage.
     * @tparam StorageT The final derived type of the complet StreamBroker Storage.
     * @param[in] storage The StreamBroker's storage which will be modified.
     */
    template <class Role, class StorageT>
    SBIO_HD static void pre_update(StorageT& storage) {
      if constexpr (requires { Derived::template pre_update_impl<Role, StorageT>(storage); }) {
        Derived::template pre_update_impl<Role, StorageT>(storage);
      }
    }

    /**
     * An opportunity to provide explicit synchronization after any Storage changes.
     *
     * For example, a policy may release locks, flush data, synchronize Windows etc.
     *
     * @tparam Role The type of the buffer to be modified from the Storage.
     * @tparam StorageT The final derived type of the complet StreamBroker Storage.
     * @tparam SyncT The type of the sync_vars SyncGroup.
     * @param[in] storage The StreamBroker's storage which will be modified.
     * @param[in] sync_vars Additional variables from a StreamBroker which may need
     *            synchronization.
     * @param[in] status The IOStatus result from the preceeding update.
     */
    template <class Role, class StorageT, class SyncT>
    SBIO_HD static void post_update(StorageT& storage, SyncT&& sync_vars, IOStatus status) {
      if constexpr (requires {
          Derived::template post_update_impl<Role, StorageT>(storage, sync_vars, status);
        }) {
        Derived::template post_update_impl<Role, StorageT>(storage,
                                                           hd_std::forward<SyncT>(sync_vars),
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

    // --- BrokerGroup level policies --- //
    // ---------------------------------- //
    template <class IO, class FTraits>
    requires FormatTraits<FTraits, IO, Derived>
    SBIO_HD static auto allocate_group_storage(hd_std::size_t num_segments,
                                               hd_std::size_t max_batch_count = 1) {
      using PtrTableRequirements = TypeList<
        BufferDescriptor<TableRole, 0, sizeof(void*)>
      >;

      if constexpr (requires {
          Derived::template allocate_group_storage_impl<PtrTableRequirements, FTraits>(num_segments,
                                                                                       max_batch_count);
      }) {
        return Derived::template allocate_group_storage_impl<PtrTableRequirements, FTraits>(num_segments,
                                                                                            max_batch_count);
      } else {
        AllocationRequest<FTraits> alloc_request;
        alloc_request.size_requests[0] = sizeof(void*) * num_segments * max_batch_count;

        return Derived::template allocate_storage<PtrTableRequirements, IO, FTraits>(alloc_request);
      }
    }

    /**
     * Run the data fetching and then querying of the filled buffers.
     *
     * If the Derived class has not defined an implementation, in this case, the default
     * behaviour will be to run all the fetch callbacks to fill the buffers, and then
     * if all succeeded run each data retrieval in order afterwards.
     *
     * @tparam FTraits The data format type.
     * @tparam FetchCBType The type for a callback to run on each IO fetch.
     * @tparam GetCBType The type for a callback to run when inspecting each
     *         fetched buffer.
     * @param[in] step_idx The step to fetch data for.
     * @param[in] unit_fetcher The callback used to perform an IO fetch for a single
     *            StreamBroker.
     * @param[in] num_fetches The total number of fetches to perform.
     * @param[in] unit_get_data The callback used to read from a buffer after the
     *            fetch has been performed. Note that a single StreamBroker may
     *            end up being used multiple times via this callback if it must
     *            read from the buffer in different places.
     * @param[in] num_accesses The total number of calls to unit_get_data to perform.
     * @returns The final IOStatus after having fetched all data and run the querying
     *          callbacks afterwards.
     */
    template <class FTraits, class FetchCBType, class GetCBType>
    SBIO_HD static IOStatus get_data(typename FTraits::StepIdxType step_idx,
                                     FetchCBType&& unit_fetcher,
                                     hd_std::size_t num_fetches,
                                     GetCBType&& unit_get_data,
                                     hd_std::size_t num_accesses) {
      if constexpr (requires {
          Derived::template get_data_impl<FTraits>(step_idx,
                                                   hd_std::forward<FetchCBType>(unit_fetcher),
                                                   num_fetches,
                                                   hd_std::forward<GetCBType>(unit_get_data),
                                                   num_accesses);
        }) {
        return Derived::template get_data_impl<FTraits>(step_idx,
                                                        hd_std::forward<FetchCBType>(unit_fetcher),
                                                        num_fetches,
                                                        hd_std::forward<GetCBType>(unit_get_data),
                                                        num_accesses);
      } else {
        // By default, we will do IO for all units (segments)
        // Then afterwards we will do data retrieval.
        IOStatus status = IOStatus::Success;
        for (hd_std::size_t i = 0; i < num_fetches; ++i) {
          IOStatus s = unit_fetcher(i);
          if (s != IOStatus::Success) {
            status = s;
            break;
          }
        }
        if (status == IOStatus::Success) {
          for (hd_std::size_t i = 0; i < num_accesses; ++i) {
            unit_get_data(i);
          }
        }
        return status;
      }
    }

    /**
     * Run the data fetching and then querying of the filled buffers, for a BATCh of steps.
     *
     * If the Derived class has not defined an implementation, in this case, the default
     * behaviour will be to run all the fetch callbacks to fill the buffers, and then
     * if all succeeded run each data retrieval in order afterwards.
     *
     * @tparam FTraits The data format type.
     * @tparam FetchCBType The type for a callback to run on each IO fetch.
     * @tparam GetCBType The type for a callback to run when inspecting each
     *         fetched buffer.
     * @param[in] steps The batch of steps to fetch data for.
     * @param[in] unit_fetcher The callback used to perform an IO fetch for a single
     *            StreamBroker.
     * @param[in] num_fetches The total number of fetches to perform.
     * @param[in] unit_get_data The callback used to read from a buffer after the
     *            fetch has been performed. Note that a single StreamBroker may
     *            end up being used multiple times via this callback if it must
     *            read from the buffer in different places.
     * @param[in] num_accesses The total number of calls to unit_get_data to perform.
     * @returns The final IOStatus after having fetched all data and run the querying
     *          callbacks afterwards.
     */
    template <class FTraits, class FetchCBType, class GetCBType>
    SBIO_HD static IOStatus get_data_steps(const hd_std::initializer_list<typename FTraits::StepIdxType>& steps,
                                           FetchCBType&& unit_fetcher,
                                           hd_std::size_t num_fetches,
                                           GetCBType&& unit_get_data,
                                           hd_std::size_t num_accesses) {
      if constexpr (requires {
          Derived::template get_data_steps_impl<FTraits>(steps,
                                                         hd_std::forward<FetchCBType>(unit_fetcher),
                                                         num_fetches,
                                                         hd_std::forward<GetCBType>(unit_get_data),
                                                         num_accesses);
        }) {
        return Derived::template get_data_steps_impl<FTraits>(steps,
                                                              hd_std::forward<FetchCBType>(unit_fetcher),
                                                              num_fetches,
                                                              hd_std::forward<GetCBType>(unit_get_data),
                                                              num_accesses);
      } else {
        // By default, we will do IO for all units (segments)
        // Then afterwards we will do data retrieval.
        IOStatus status = IOStatus::Success;
        for (hd_std::size_t i = 0; i < num_fetches; ++i) {
          IOStatus s = unit_fetcher(i);
          if (s != IOStatus::Success) {
            status = s;
            break;
          }
        }
        if (status == IOStatus::Success) {
          bool passed_step { steps.size() == 3 };
          typename FTraits::StepIdxType first { *steps.begin() };
          typename FTraits::StepIdxType last {
            passed_step ? *(steps.end() - 2) : *(steps.end() - 1)
          };
          typename FTraits::StepIdxType count {
            (last > first) ? static_cast<hd_std::size_t>(last - first) : 1
          };

          for (hd_std::size_t cnt = 0; cnt < static_cast<hd_std::size_t>(count); ++cnt) {
            for (hd_std::size_t i = 0; i < num_accesses; ++i) {
              unit_get_data(i, cnt);
            }
          }
        }
        return status;
      }
    }


    // --- DataSource level policies --- //
    // --------------------------------- //
    /**
     * Request the next step index to read data for.
     *
     * @tparam FTraits The data format type.
     * @tparam IndexTrigger The type of the callback to be run on reaching the
     *         current maximum capacity.
     * @param[in] max_capacity The current max capacity that the StreamBroker has
     *            before reindexing is required. For data formats that do not
     *            support indexing, the max capacity will always be 1, and the
     *            reindexing callback may be a noop.
     * @param[in] trigger The callback to run to reindex upon reaching max_capacity,
     *            if applicable.
     * @returns The index of the next step to read data for.
     */
    template <class FTraits, class IndexTrigger>
    SBIO_HD static typename FTraits::StepIdxType
    next(typename FTraits::StepIdxType& max_capacity, IndexTrigger&& trigger) {
      if constexpr (requires {
          Derived::template next_impl<FTraits>(max_capacity,
                                               hd_std::forward<IndexTrigger>(trigger));
        }) {
        return
          Derived::template next_impl<FTraits>(max_capacity,
                                               hd_std::forward<IndexTrigger>(trigger));
      }
    }
  };
} // namespace sbio

#endif // SBIO_CORE_EXECUTION_HH
