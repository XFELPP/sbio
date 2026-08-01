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

#ifndef SBIO_CORE_BROKER_HH
#define SBIO_CORE_BROKER_HH

#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/stream.hh"
#include "sbio/core/sync.hh"
#include "sbio/formats/format_traits.hh"

#include <concepts>
#include <initializer_list>
#include <utility> // std::forward

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  /**
   * Determines whether a class conforms to the StreamBroker interface.
   */
  template <typename T>
  concept IsStreamBroker = requires(T broker,
                                    typename T::StepIdxType step_idx,
                                    typename T::DataAccessPtn ptn,
                                    typename T::DataRequest req) {
    { broker.allocate_storage() };
    { broker.open_data_stream() } -> std::convertible_to<IOStatus>;
    { broker.discover_metadata() } -> std::convertible_to<IOStatus>;
    { broker.fetch_step(step_idx, ptn) } -> std::convertible_to<IOStatus>;
    { broker.get_data_in_buffer(req, ptn) } -> std::convertible_to<typename T::DataResult>;
    { broker.process() } -> std::convertible_to<IOStatus>;
    { broker.capacity() } -> std::convertible_to<std::size_t>;
    { broker.sync_vars() };
  };

  /**
   * Determines whether a class conforms to the StreamBroker interface.
   */
  template <typename T>
  concept IsIndexableStreamBroker = IsStreamBroker<T> && requires(T broker) {
    { broker.index_stream() } -> std::convertible_to<IOStatus>;
  };

  /**
   * The indicators of the StreamBroker state machine.
   *
   * As data is read from the Stream(s) the Broker will transition through these
   * various states depending on the exact actions that are being undertaken.
   *
   * The Execution policy can also move the Broker through states, or make decisions,
   * i.e., be held up, or advanced, based on the state.
   *
   * Not all data formats require the same things, so the exact actions performed
   * by the Broker in each state will change. E.g., "INDEXING" may not always make
   * sense for all data formats (some may not be indexable).
   */
  enum class BrokerState {
    INIT,      ///< Initialization of any non-stream setup the Broker must do.
    ALLOCATE,  ///< Allocation of memory required by the data format to read properly.
    CONNECT,   ///< Connection/opening of the Streams managed by the Broker.
    DISCOVERY, ///< Determines the contents of the Stream.
    READY,     ///< Ready to begin reading/brokering/transmitting streamed data.
    INDEXING,  ///< Indexing the data stream (if meaningful).
    STREAMING, ///< In the process of fetching/transmitting streamed data.
    ERROR,     ///< An error state has been encountered and should be investigated.
    DONE       ///< The Stream is exhausted and has been closed.
  };

  /**
   * A StreamBroker organizes Streams and mediates data requests.
   *
   * The StreamBroker has a state machine and exposes data-format dependent metadata
   * and data buffers.
   *
   * @tparam IO The type of the IO strategy being used.
   * @tparam EPolicy The Execution policy to use for reading data.
   * @tparam FTraits The data format to read.
   * @tparam Derived The sub-class type if defined. All functionality can be exposed
   *         purely from the FTraits template parameter, so sub-classing is not
   *         strictly necessary. I.e., this class can be instantiated directly.
   */
  template <
    IOTraits IO,
    class EPolicy,
    FormatTraits<IO, EPolicy> FTraits,
    class Derived = void
  >
  class StreamBroker {
  public:
    /**
     * The type of the IO strategy being used.
     */
    using IOPolicy = IO;
    /**
     * The Execution policy type.
     */
    using ExecutionPolicy = EPolicy;
    /**
     * The type of data being read.
     */
    using DataFormat = FTraits;

    /**
     * The type of Stream: I.e., the IO strategy and data format being read.
     */
    using StreamType = Stream<IO, FTraits>;

    /**
     * The type of the StreamBroker's Storage.
     */
    using SBStorageType = Storage<typename FTraits::BrokerBufferRequirements, EPolicy>;

    /**
     * The Execution policy configuration object type.
     *
     * `epolicy_config` objects configure the global behaviour of the Execution policy
     * being used. The Execution policy must be configured before any Streams are
     * opened as it controls all aspects of IO down to the allocation of Storage.
     */
    using EPolicyConfig = typename EPolicy::Config;
    /**
     * The individual Stream configuration object type.
     *
     * `stream_config` objects are used to set up each individual Stream so that
     * it can connect and read from its individual data.
     */
    using StreamConfig = typename FTraits::StreamParameters;
    /**
     * The DataSource configuration object type.
     *
     * `ds_config` objects are used for initial discovery and connection of the
     * full set of Streams.
     */
    using DSConfig = typename FTraits::DataSourceParameters;

    /**
     * The type of state tracking object for the data format's Stream.
     */
    using StreamState = typename FTraits::DiscoveryState;
    /**
     * The type of the general metadata object for the data format's Stream.
     */
    using StreamMetadata = typename FTraits::MetadataInventory;

    /**
     * The type of the enumerator used to specify access patterns used for the format.
     */
    using DataAccessPtn = typename FTraits::DataAccessPtn;
    /**
     * The type of a request object used to query for data.
     */
    using DataRequest = typename FTraits::DataRequest;
    /**
     * The type of a result object received as a response when querying for data.
     */
    using DataResult = typename FTraits::DataResult;
    /**
     * The type used to request a specific step from the Stream.
     *
     * This type is required and guaranteed to be convertible std::size_t; however,
     * different data format's may use different underlying types.
     */
    using StepIdxType = typename FTraits::StepIdxType;

    static constexpr std::size_t StreamCount = FTraits::RoleCount;

    /**
     * A default constructor is provided for simplicity.
     *
     * The Broker will remain in the INIT state until explicitly configured, if
     * using this constructor.
     */
    StreamBroker()
      : m_broker_state(BrokerState::INIT)
    {}

    /**
     * Construct the Broker with a specific, data-format-dependent, set of parameters.
     *
     * @param[in] cfg The data-format-dependent configuration parameters.
     */
    StreamBroker(const StreamConfig& cfg)
      : m_config(cfg)
      , m_broker_state(BrokerState::INIT)
    {}

    /**
     * If a Broker has been default-constructed, it can be moved through the INIT
     * state via this function.
     *
     * This corresponds to the INIT stage of the state machine.
     *
     * @param[in] cfg The data format specif configuration parameters.
     */
    SBIO_HD inline void configure_broker(const StreamConfig& cfg) {
      m_config = cfg;
      m_broker_state = BrokerState::INIT;
    }

    /**
     * Perform data format specific allocations in preparation for streaming data.
     *
     * This corresponds to the ALLOCATE stage of the state machine.
     */
    SBIO_HD inline void allocate_storage() {
      m_broker_state = BrokerState::ALLOCATE;

      if constexpr (!std::is_void_v<Derived>) {
        auto& self = *static_cast<Derived*>(this);
        AllocationRequest<FTraits> request;

        // Broker sub-classes may optionally setup requests from the EPolicy allocation
        if constexpr (requires { self.get_allocation_request(); }) {
          request = self.get_allocation_request();
        }

        self.m_storage = EPolicy::template allocate_storage<IO, FTraits>(request);
      } else {
        AllocationRequest<FTraits> request = FTraits::get_allocation_request(m_config);

        m_storage = EPolicy::template allocate_storage<IO, FTraits>(request);
      }
    }

    /**
     * Connect and open brokered Streams.
     *
     * This corresponds to the CONNECT stage of the state machine.
     *
     * @returns The IOStatus result from connecting.
     */
    SBIO_HD inline IOStatus open_data_stream() {
      m_broker_state = BrokerState::CONNECT;

      if constexpr (!std::is_void_v<Derived>) {
        return static_cast<Derived*>(this)->open_data_stream_impl();
      } else {
        return FTraits::open_streams(m_streams, m_config);
      }
    }

    /**
     * After Stream connection, perform metadata discovery.
     *
     * This function determines what contents are available from the brokered Streams.
     * This information is then available to the caller (or higher-level abstractions).
     *
     * This corresponds to the DISCOVERY stage of the state machine.
     * @returns The IOStatus result for whether metadata reads were successful.
     */
    SBIO_HD inline IOStatus discover_metadata() {
      m_broker_state = BrokerState::DISCOVERY;

      EPolicy::template pre_discovery<StreamBroker, FTraits>(*this, m_metadata_inv);

      IOStatus status;
      if constexpr (!std::is_void_v<Derived>) {
        status = static_cast<Derived*>(this)->discover_metadata_impl();
      } else {
        StorageView<SBStorageType, EPolicy> sv(m_storage);
        status = FTraits::discover_metadata(m_streams, sv, m_metadata_inv);
      }

      EPolicy::template on_discovery<StreamBroker, FTraits>(*this,
                                                            m_metadata_inv,
                                                            status);
      m_broker_state = BrokerState::READY;

      return status;
    }

    /**
     * Run through the initial allocate, connect, discover steps in one.
     *
     * This is simply a convenience wrapper instead of calling each of the early
     * state transitions independently. The down-side is the status will be cummulative
     * so a failure may possibly not be immediately clear as originiating from one step
     * or another.
     *
     * @returns The IOStatus result from all 3 steps.
     */
    SBIO_HD inline IOStatus prepare() {
      IOStatus status { IOStatus::Success };
      if constexpr (!std::is_void_v<Derived>) {
        auto& self = *static_cast<Derived*>(this);
        self.allocate_storage();

        status = self.open_data_stream();
        if (status == IOStatus::Success) {
          status = self.discover_metadata();
        }
      } else {
        // Proceed through allocation
        allocate_storage();

        // Abandon metadata discovery if open/connect fails
        status = open_data_stream();
        if (status == IOStatus::Success) {
          status = discover_metadata();
        }
      }

      if (status != IOStatus::Success) {
        m_broker_state = BrokerState::ERROR;
      }

      return status;
    }

    /**
     * Index the data Stream if appropriate and implemented.
     *
     * Indexing affords the possibility of ordering the data from (a) Stream(s) for
     * indexable lookup. Not all data formats support a notion of indexing.
     *
     * This corresponds to the INDEXING stage of the state machine.
     * @returns An IOStatus for whether indexing was successful.
     */
    SBIO_HD inline IOStatus index_stream() {
      m_broker_state = BrokerState::INDEXING;

      EPolicy::template pre_update<IndexRole>(m_storage);

      IOStatus status { IOStatus::Success };
      if (EPolicy::should_index()) {
        if constexpr (!std::is_void_v<Derived>) {
          status = static_cast<Derived*>(this)->index_stream_impl();
        } else {
          StorageView<SBStorageType, EPolicy> sv(m_storage);

          // Must ensure that the signatures match to avoid silent failures
          const auto& cfg { m_config };
          if constexpr (requires {
            FTraits::index_stream(m_streams, sv, m_stream_state, cfg);
          }) {
            status = FTraits::index_stream(m_streams, sv, m_stream_state, cfg);
          }
        }
      }

      // self.sync_vars returns a SyncGroup object (see core/sync.hh)
      // This object must contain references to various attributes - it is the
      // responsibility of the execution policy to handle appropriate assingment
      // when synchronization is required.
      EPolicy::template post_update<IndexRole>(m_storage, sync_vars(), status);

      // Should do an error check to set state properly.
      m_broker_state = BrokerState::READY;

      return status;
    }

    /**
     * Retrieve data for the specified index using the provided lookup pattern.
     *
     * The index used for lookup is data format dependent. If the format supports
     * indexing, then it may correspond to a chunk of data other than the most recent.
     * If not, then this will always return the most recent data from the Stream.
     *
     * @param[in] step_idx The data format-dependent data chunk index.
     * @param[in] ptn A lookup pattern. Some data formats support access in various ways.
     * @returns An IOStatus for whether the lookup was succesful.
     */
    SBIO_HD inline IOStatus fetch_step(StepIdxType step_idx,
                                       const DataAccessPtn ptn) {
      m_broker_state = BrokerState::STREAMING;

      EPolicy::template pre_update<DataRole>(m_storage);

      IOStatus status { IOStatus::Success };
      if (EPolicy::template should_process<FTraits>(step_idx)) {
        if constexpr (!std::is_void_v<Derived>) {
          status = static_cast<Derived*>(this)->fetch_step_impl(step_idx, ptn);
        } else {
          StorageView<SBStorageType, EPolicy> sv(m_storage);
          status =
            FTraits::fetch_step(m_streams, sv, m_stream_state, m_config, step_idx, ptn);
        }
      }

      // Should do an error check to set state properly.
      m_broker_state = BrokerState::READY;

      return status;
    }

    SBIO_HD inline IOStatus fetch_steps(std::initializer_list<StepIdxType> steps,
                                        const DataAccessPtn ptn) {
      if (steps.size() == 1) {
        return fetch_step(*steps.begin(), ptn);
      } else if (steps.size() <= 3) {
        m_broker_state = BrokerState::STREAMING;

        EPolicy::template pre_update<DataRole>(m_storage);

        // Passed begin/end, steps of 1 unit or explicit as 3rd item
        bool passed_step { steps.size() == 3 };
        StepIdxType first { *steps.begin() };
        StepIdxType last = passed_step ? *(steps.end() - 2) : *(steps.end() - 1);

        // StepIdxType step = passed_step ? *(steps.end() - 1) : 1;
        // TODO: Support the striding, for now only use 1

        StepIdxType count { (last > first) ? (last - first) : 1 };

        IOStatus status { IOStatus::Success };
        if (EPolicy::template should_process<FTraits>(first)) {
          if constexpr (!std::is_void_v<Derived> && requires {
              static_cast<Derived*>(this)->fetch_steps_impl(steps, ptn);
            }) {
            status = static_cast<Derived*>(this)->fetch_steps_impl(steps, ptn);
          } else {


            StorageView<SBStorageType, EPolicy> sv(m_storage);
            status = FTraits::fetch_multi_steps(m_streams,
                                                sv,
                                                m_stream_state,
                                                m_config,
                                                first,
                                                count,
                                                ptn);
          }
        }

        // Should do an error check to set state properly.
        m_broker_state = BrokerState::READY;

        return status;
      } else {
        return IOStatus::GeneralIOError;
      }
    }

    SBIO_HD inline IOStatus process() {
      if constexpr (!std::is_void_v<Derived>) {
        return static_cast<Derived*>(this)->process();
      } else {
        return process();
      }
    }

    SBIO_HD inline IOStatus run() {
      if constexpr (!std::is_void_v<Derived>) {
        return static_cast<Derived*>(this)->run();
      } else {
        return run();
      }
    }

    // Provide option to include callback?
    template <class CBType>
    SBIO_HD inline IOStatus step(CBType&& callback) {
      if constexpr (!std::is_void_v<Derived>) {
        if constexpr (requires {
            static_cast<Derived*>(this)->step(std::forward<CBType>(callback));
          }) {
          return static_cast<Derived*>(this)->step(std::forward<CBType>(callback));
        }
      } else if constexpr (requires { step(std::forward<CBType>(callback)); }) {
        return step(std::forward<CBType>(callback));
      }
    }

    /**
     * Return the current broker state along the state machine.
     *
     * @returns The current broker state along the state machine.
     */
    SBIO_HD inline BrokerState state() const { return m_broker_state; }
    /**
     * Return the underlying StreamState of the brokered stream(s).
     *
     * The StreamState tracks data format-specific information about the streamed
     * data. This may include information such as counters, whether certain transitions
     * have been encountered, or whether the stream has been exhausted/will be soon.
     * Refer to the specific FormatTraits for the format of interest for more
     * information.
     *
     * In cases where the broker manages multiple Streams, there is still one
     * shared StreamState which encompasses all of them.
     *
     * @returns The current Stream(s) StreamState.
     */
    SBIO_HD inline StreamState stream_state() const { return m_stream_state; }

    /**
     * Return the current capacity for data formats that support indexing.
     *
     * When indexing is supported, the capacity indicates how many indices are
     * immediately available to jump to in the stream. Reading beyond the capacity
     * requires another INDEXING transition, or a move to reading in true streaming
     * manner.
     *
     * @returns The current capacity: the number of immediately available indices.
     */
    SBIO_HD inline std::size_t capacity() const {
      if constexpr (!std::is_void_v<Derived>) {
        return static_cast<const Derived*>(this)->capacity();
      } else {
        const StorageView<const SBStorageType, EPolicy> sv(m_storage);
        return FTraits::capacity(sv, m_stream_state);
      }
    }

    // Functions to set and retrieve buffers
    /**
     * Access the current data after a fetch.
     *
     * The action of performing a fetch pulls the data from the stream into available
     * memory managed by the Broker (in conjunction with the Execution policy). The
     * fetch APIs, however, do not return access to this memory directly. Instead,
     * this function should be used to access data after a fetch if the raw buffer
     * is required. Otherwise, more specific APIs can be used to parse specific
     * components from the buffer.
     *
     * @returns A pointer to the buffer filled after a fetch from (a) Stream(s).
     */
    SBIO_HD inline typename FTraits::DataUnit* current_buffer() {
      if constexpr (!std::is_void_v<Derived>) {
        return static_cast<Derived*>(this)->current_buffer();
      } else {
        StorageView<SBStorageType, EPolicy> sv(m_storage);
        return FTraits::current_buffer(sv, m_stream_state);
      }
    }

    /**
     * Parse specific data from the buffer after a fetch.
     *
     * This function parses, in a data format-dependent manner, a portion of data
     * from a buffer fetched from (a) Stream(s).
     *
     * @param[in] req A data-format-dependent struct with a parse request.
     * @param[in] ptn A data-format-dependent access pattern for formats that can
     *            be read in different ways.
     * @param[in] batch_idx If reading by batches, the index for which of the steps
     *            in the batch to be resolved.
     * @returns A data-format-dependent result object with the requested data.
     */
    SBIO_HD inline DataResult
    get_data_in_buffer(const DataRequest& req,
                       const DataAccessPtn ptn,
                       std::size_t batch_idx = 0) {
      if constexpr (!std::is_void_v<Derived>) {
        return static_cast<Derived*>(this)->get_data_in_buffer(req, ptn, batch_idx);
      } else {
        StorageView<SBStorageType, EPolicy> sv(m_storage);
        return FTraits::get_data_in_buffer(sv, m_metadata_inv, req, ptn, batch_idx);
      }
    }

    /**
     * The compiled metadata from the brokered Stream(s).
     *
     * This function should only be used after having passed the DISCOVERY state,
     * or having set the metadata explicitly.
     *
     * @returns The compiled metadata from the brokered Stream(s).
     */
    SBIO_HD inline StreamMetadata& metadata() { return m_metadata_inv; }
    /**
     * The compiled metadata from the brokered Stream(s).
     *
     * This function should only be used after having passed the DISCOVERY state,
     * or having set the metadata explicitly.
     *
     * @returns The compiled metadata from the brokered Stream(s).
     */
    SBIO_HD inline const StreamMetadata& metadata() const { return m_metadata_inv; }

    /**
     * The set of StreamParameters configuration used to instantiate the broker.
     *
     * @returns The StreamParameters configuration.
     */
    SBIO_HD inline StreamConfig& config() { return m_config; }
    /**
     * The set of StreamParameters configuration used to instantiate the broker.
     *
     * @returns The StreamParameters configuration.
     */
    SBIO_HD inline const StreamConfig& config() const { return m_config; }

    /**
     * Configure the Broker with a set of metadata.
     *
     * Under some Execution policies, not all Brokers will necessarily move through
     * all states of the state machine. In case the DISCOVERY state is not explicitly
     * moved to, then the metadata for the Broker can be set using this function.
     * For example, multiple Brokers may be reading different parts of the same Stream(s)
     * in parallel.
     *
     * The metadata is not only used by the caller, but also internally by the Broker,
     * or at least, it may be used. For that reason, this function is made available.
     *
     * @param[in] The metadata to provide the Broker with.
     */
    SBIO_HD inline void set_metadata(StreamMetadata& metadata) {
      m_metadata_inv = metadata;
    }

    SBIO_HD inline StreamType& stream(std::size_t role_idx) {
      return m_streams[role_idx];
    }

    SBIO_HD inline const StreamType& stream(std::size_t role_idx) const {
      return m_streams[role_idx];
    }

    SBIO_HD inline auto sync_vars() {
      if constexpr (!std::is_void_v<Derived>) {
        return static_cast<Derived*>(this)->sync_vars();
      } else if constexpr (requires { FTraits::sync_vars(m_stream_state); }) {
        return FTraits::sync_vars(m_stream_state);
      } else {
        return make_sync_group();
      }
    }

    ~StreamBroker() = default;

  protected:
    StreamType m_streams[StreamCount];
    StreamConfig m_config;
    BrokerState m_broker_state;
    StreamState m_stream_state;
    StreamMetadata m_metadata_inv;
    SBStorageType m_storage;
  };
} // namespace sbio

#endif // SBIO_CORE_BROKER_HH
