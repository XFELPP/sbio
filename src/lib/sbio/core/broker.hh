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
#include "sbio/core/metadata.hh"
#include "sbio/core/result.hh"
#include "sbio/core/roles.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/stream.hh"
#include "sbio/core/sync.hh"
#include "sbio/core/transaction.hh"
#include "sbio/formats/format_traits.hh"

#include <ncarray/storage.hh>

#ifdef __CUDACC__

#include <cuda/std/concepts>
#include <cuda/std/cstdint>
#include <cuda/std/initializer_list>
#include <cuda/std/utility>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <utility> // std::forward

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

namespace sbio {
  /**
   * Determines whether a class conforms to the StreamBroker interface.
   */
  template <typename T>
  concept IsStreamBroker = requires(T broker,
                                    typename T::StepIdxType step_idx,
                                    typename T::DataAccessPtn ptn,
                                    typename T::DataRequest req,
                                    SegmentCursor<typename T::DataFormat>& cursor) {
    { broker.allocate_storage() };
    { broker.open_data_stream() } -> std::convertible_to<IOStatus>;
    { broker.discover_metadata() } -> std::convertible_to<IOStatus>;
    { broker.fetch_step(step_idx, ptn, cursor) } -> std::convertible_to<IOStatus>;
    { broker.get_data_in_buffer(req, ptn) } -> std::convertible_to<DataResult>;
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
     * Whether the broker builds and queries the index itself (generic path).
     */
    static constexpr bool UsesGenericIndex { HasGenericIndex<FTraits> };

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
     * The type of the general metadata object for the data format's Stream.
     */
    using StreamMetadata = MetadataInventory<FTraits>;

    /**
     * The type of the enumerator used to specify access patterns used for the format.
     */
    using DataAccessPtn = typename FTraits::DataAccessPtn;
    /**
     * The type of a request object used to query for data.
     */
    using DataRequest = typename FTraits::DataRequest;
    /**
     * The type used to request a specific step from the Stream.
     *
     * This type is required and guaranteed to be convertible std::size_t; however,
     * different data format's may use different underlying types.
     */
    using StepIdxType = typename FTraits::StepIdxType;

    /**
     * Based on choice of ExecutionPolicy, data will return in host or device buffers.
     *
     * As data is returned via ncarray array objects, the MemTag converts a MemorySpace
     * indicator in sbio to the tagging system used for host/device array constructs in
     * ncarray.
     */
    using MemTag = hd_std::conditional_t<
      ExecutionPolicy::result_memory_space() == MemorySpace::Host,
      ncarray::HostTag,
      ncarray::DevTag
    >;

    static constexpr std::size_t StreamCount { FTraits::StreamTypes::size() };

    static constexpr std::size_t NumStepKinds = []() {
      if constexpr (requires { FTraits::NumStepKinds; }) {
        return FTraits::NumStepKinds;
      } else {
        return 1;
      }
    }();

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
    StreamBroker(const GenericStreamConfig<DataFormat>& cfg)
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
    SBIO_HD inline void configure_broker(const GenericStreamConfig<DataFormat>& cfg) {
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
        for (std::size_t r = 0; r < m_config.VariantCount; ++r) {
          const auto& res { m_config.resources[r] };

          IOStatus status { IOStatus::OpenFailed };
          switch (res.type) {
          case StreamResourceType::Path: {
            status = m_streams[r].connect(res.handle.path);
            break;
          }
          case StreamResourceType::FileDescriptor: {
            status = m_streams[r].connect(res.handle.fd);
            break;
          }
          //case StreamResourceType::MemoryRegion: {
          //  status = m_streams[r].connect(res.handle.memory.ptr,
          //                                res.handle.memory.size);
          //  break;
          //}
          default: {
            return IOStatus::FunctionUnavailable;
          }
          }

          if (status != IOStatus::Success) {
            return status;
          }
        }

        return IOStatus::Success;
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

      auto txn { Transaction<roles::Metadata, ExecutionPolicy, SBStorageType>(m_storage) };
      auto sv { txn.view() };

      IOStatus status;
      if constexpr (!std::is_void_v<Derived>) {
        status = static_cast<Derived*>(this)->discover_metadata_impl();
      } else {
        status = FTraits::discover_metadata(m_streams, sv, m_metadata_inv);
      }

      txn.commit(sync_vars(), status);

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

      auto txn { Transaction<roles::Index, ExecutionPolicy, SBStorageType>(m_storage) };
      auto sv { txn.view() };

      IOStatus status { IOStatus::Success };
      if (EPolicy::should_index()) {
        if constexpr (!std::is_void_v<Derived>) {
          status = static_cast<Derived*>(this)->index_stream_impl();
        } else {
          if constexpr (UsesGenericIndex) {
            status = generic_index(sv);
          } else {
            // Must ensure that the signatures match to avoid silent failures
            const auto& cfg { m_config };
            status = FTraits::index_stream(m_streams, sv, m_catalog, m_indexing_cursor, cfg);
          }
        }
      }

      // self.sync_vars returns a SyncGroup object (see core/sync.hh)
      // This object must contain references to various attributes - it is the
      // responsibility of the execution policy to handle appropriate assingment
      // when synchronization is required.
      txn.commit(sync_vars(), status);

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
                                       const DataAccessPtn ptn,
                                       SegmentCursor<FTraits>& cursor) {
      m_broker_state = BrokerState::STREAMING;

      auto txn { Transaction<roles::Data, ExecutionPolicy, SBStorageType>(m_storage) };
      auto sv { txn.view() };

      IOStatus status { IOStatus::Success };
      if (EPolicy::template should_process<FTraits>(step_idx)) {
        if constexpr (!std::is_void_v<Derived>) {
          status = static_cast<Derived*>(this)->fetch_step_impl(step_idx, ptn, cursor);
        } else {
          if constexpr (UsesGenericIndex) {
            status = generic_fetch(sv, step_idx, ptn, cursor);
          } else {
            status =
              FTraits::fetch_step(m_streams, sv, m_catalog, cursor, m_config, step_idx, ptn);
          }
        }
      }

      txn.commit(sync_vars(), status);

      // Should do an error check to set state properly.
      m_broker_state = BrokerState::READY;

      return status;
    }

    SBIO_HD inline IOStatus fetch_steps(std::initializer_list<StepIdxType> steps,
                                        const DataAccessPtn ptn,
                                        SegmentCursor<FTraits>& cursor) {
      if (steps.size() == 1) {
        return fetch_step(*steps.begin(), ptn, cursor);
      } else if (steps.size() <= 3) {
        m_broker_state = BrokerState::STREAMING;

        auto txn { Transaction<roles::Data, ExecutionPolicy, SBStorageType>(m_storage) };
        auto sv { txn.view() };

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
            if constexpr (UsesGenericIndex) {
              status = generic_fetch_range(sv, first, count, ptn, cursor);
            } else {
              status = FTraits::fetch_multi_steps(m_streams,
                                                  sv,
                                                  m_catalog,
                                                  cursor,
                                                  m_config,
                                                  first,
                                                  count,
                                                  ptn);
            }
          }
        }

        txn.commit(sync_vars(), status);

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
      } else if constexpr (UsesGenericIndex) {
        // TODO: This is an unspoken rule then... the first access pattern determines
        //       the overall "count".
        //       ... should consider how to make convention more explicit/smarter ...
        return m_catalog.num_steps[kind_idx(FTraits::kind_for_ptn(DataAccessPtn {}))];
      } else {
        return m_catalog.max_capacity();
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
        //StorageView<SBStorageType, EPolicy> sv(m_storage);
        //return FTraits::current_buffer(sv, m_stream_state);
        // TODO: Need new implementation for this now!
        return nullptr;
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
    SBIO_HD inline GenericStreamConfig<DataFormat>& config() { return m_config; }
    /**
     * The set of StreamParameters configuration used to instantiate the broker.
     *
     * @returns The StreamParameters configuration.
     */
    SBIO_HD inline const GenericStreamConfig<DataFormat>& config() const { return m_config; }

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
      } else {
        return make_sync_group(m_catalog.index_epoch,
                               m_catalog.num_steps,
                               m_catalog.cummulative_steps,
                               m_catalog.num_rows,
                               m_catalog.first_row,
                               m_catalog.last_row);
      }
    }

    ~StreamBroker() = default;

    // TODO: Needs to implement some sortable index (mostly for Chronological mode)
    SBIO_HD std::uint32_t stream_idx() const { return 0; }

    SBIO_HD inline StreamCatalog<FTraits>& catalog() noexcept {
      return m_catalog;
    }
    SBIO_HD inline const StreamCatalog<FTraits>& catalog() const noexcept {
      return m_catalog;
    }

    template <typename KindT>
    requires UsesGenericIndex
    SBIO_HD inline IOStatus preceding(KindT kind,
                                      KindT ref_kind,
                                      hd_std::uint64_t ref_ordinal,
                                      hd_std::uint64_t& ordinal,
                                      SegmentCursor<FTraits>& cursor) {
      StorageView<SBStorageType, EPolicy> sv(m_storage);
      const auto* rows { acquire_rows(sv) };

      hd_std::uint32_t ref_row { NoRow };
      IOStatus status {
        find_row(rows, ref_kind, ref_ordinal, cursor.offset_index[kind_idx(ref_kind)], ref_row)
      };

      if (status == IOStatus::Success) {
        cursor.offset_index[kind_idx(ref_kind)] = ref_row;

        const auto k { kind_idx(kind) };
        hd_std::uint32_t r { static_cast<hd_std::uint32_t>(cursor.offset_index[k]) };
        if (r < m_catalog.num_rows && rows[r].step_kind == kind && r < ref_row) {
          // Hint is before the reference: walk forward
          while (rows[r].next_of_kind != NoRow && rows[r].next_of_kind < ref_row) {
            r = rows[r].next_of_kind;
          }
        } else {
          // Otherwise walk back from the last step of this kind
          r = m_catalog.last_row[k];
          while (r != NoRow && r > ref_row) {
            r = rows[r].prev_of_kind;
          }
        }

        if (r == NoRow) {
          // Nothing of this kind before the reference
          status = IOStatus::NoOffsetInData;
        } else {
          cursor.offset_index[k] = r;
          ordinal = rows[r].ordinal;
        }
      }

      sv.template release<roles::Index, index_ids::Steps>(const_cast<StepOffset<FTraits>*>(rows));
      return status;
    }

  protected:
    StreamType m_streams[StreamCount];
    GenericStreamConfig<DataFormat> m_config;
    BrokerState m_broker_state;

    StreamMetadata m_metadata_inv;
    SBStorageType m_storage;

    StreamCatalog<FTraits> m_catalog {};
    IndexingCursor<FTraits> m_indexing_cursor {};

  private:
    template <typename KindT>
    SBIO_HD static constexpr hd_std::size_t kind_idx(KindT kind) {
      return static_cast<hd_std::size_t>(kind);
    }

    template <typename UnitT>
    SBIO_HD static hd_std::size_t unit_size(const UnitT* unit) {
      if constexpr (requires { FTraits::unit_size(unit); }) {
        return FTraits::unit_size(unit);
      } else {
        return sizeof(UnitT) + FTraits::get_payload_size(const_cast<UnitT*>(unit));
      }
    }

    template <class SV>
    SBIO_HD static StepOffset<FTraits>* acquire_rows(SV& sv) {
      // TODO: Will eventually need to replace the hard-coded HostTag
      return
        static_cast<StepOffset<FTraits>*>(sv.template acquire<
                                           roles::Index,
                                           index_ids::Steps,
                                           MemTag
                                          >(AcquireIntent::CallerMemorySpace));
    }

    template <typename KindT>
    SBIO_HD IOStatus find_row(const StepOffset<FTraits>* rows,
                              KindT kind,
                              hd_std::uint64_t ordinal,
                              hd_std::size_t hint,
                              hd_std::uint32_t& row) const {
      const auto k { kind_idx(kind) };
      if (ordinal >= m_catalog.cummulative_steps[k]) {
        return IOStatus::AllRequestedRead;
      }

      const hd_std::uint32_t first { m_catalog.first_row[k] };
      if (m_catalog.index_epoch == 0 ||
          first == NoRow             ||
          ordinal < rows[first].ordinal) {
        return IOStatus::NoOffsetInData;
      }

      hd_std::uint32_t r { first };
      if (hint < m_catalog.num_rows    &&
          rows[hint].step_kind == kind &&
          rows[hint].ordinal <= ordinal) {
        r = static_cast<hd_std::uint32_t>(hint);
      }

      while (rows[r].ordinal < ordinal) {
        r = rows[r].next_of_kind;
      }

      row = r;
      return IOStatus::Success;
    }

    template <class SV>
    SBIO_HD IOStatus generic_index(SV& sv) {
      using Unit = typename FTraits::DataUnit;
      constexpr hd_std::size_t NumKinds { StreamCatalog<FTraits>::NumStepKinds };

      constexpr auto IndexSlotSigned {
        FTraits::StreamTypes::template index_of_role<roles::Index>
      };
      static_assert(IndexSlotSigned >= 0, "No StreamVariant is indexable!");
      constexpr auto IndexSlot { static_cast<hd_std::uint32_t>(IndexSlotSigned) };

      const auto& stream { m_streams[IndexSlot] };
      auto& pos { m_indexing_cursor.next_offset[IndexSlot] };

      auto* rows { acquire_rows(sv) };
      auto* scratch {
        static_cast<char*>(sv.template acquire<
                            roles::Metadata,
                            index_ids::Scratch,
                            MemTag
                           >(AcquireIntent::CallerMemorySpace))
      };

      const hd_std::size_t rows_capacity {
        sv.template size<roles::Index, index_ids::Steps>() / sizeof(StepOffset<FTraits>)
      };
      const hd_std::size_t scratch_size {
        sv.template size<roles::Metadata, index_ids::Scratch>()
      };

      hd_std::array<StepOffset<FTraits>, NumKinds> carried {};
      hd_std::array<bool, NumKinds> has_carried {};
      for (hd_std::size_t k = 0; k < NumKinds; ++k) {
        has_carried[k] = (m_catalog.index_epoch > 0 && m_catalog.last_row[k] != NoRow);
        if (has_carried[k]) {
          carried[k] = rows[m_catalog.last_row[k]];
        }
      }

      m_catalog.num_rows = 0;
      for (hd_std::size_t k = 0; k < NumKinds; ++k) {
        m_catalog.num_steps[k] = 0;
        m_catalog.first_row[k] = NoRow;
        m_catalog.last_row[k] = NoRow;

        if (has_carried[k]) {
          const auto r { static_cast<hd_std::uint32_t>(m_catalog.num_rows++) };
          rows[r] = carried[k];
          rows[r].next_of_kind = NoRow;
          rows[r].prev_of_kind = NoRow;
          m_catalog.first_row[k] = r;
          m_catalog.last_row[k] = r;
        }
      }
      const hd_std::size_t carried_rows { m_catalog.num_rows };

      IOStatus status { IOStatus::Success };
      if (carried_rows >= rows_capacity) {
        status = IOStatus::PayloadTruncatedError; // Index buffer can't hold more than the carried rows
      }

      // --- Scan
      bool at_end { false };
      while (status == IOStatus::Success && !at_end && m_catalog.num_rows < rows_capacity) {
        const auto res { stream.read_span(scratch, pos, scratch_size) };
        if (res.bytes == 0) {
          if (res.status != IOStatus::ZeroBytesRead) {
            status = res.status;
          }
          break;
        }

        hd_std::size_t used { 0 };
        while (used + FTraits::HeaderSize <= res.bytes && m_catalog.num_rows < rows_capacity) {
          const auto* unit { reinterpret_cast<const Unit*>(scratch + used) };

          // locate_bytes will only read the first HeaderSize bytes
          // The other routines can read up to the entire `locate_bytes` count
          if (used + FTraits::locate_bytes(unit) > res.bytes) {
            break; // Refill starting at this unit
          }

          const auto kind_opt { FTraits::kind_for_step(unit) };
          if (!kind_opt) {
            at_end = true; // Not a unit, means end of indexable data
            break;
          }

          const auto kind { *kind_opt };
          const auto k { kind_idx(kind) };
          const auto r { static_cast<hd_std::uint32_t>(m_catalog.num_rows++) };

          auto& step { rows[r] };
          step.region = FTraits::locate_step(unit, kind, IndexSlot, pos + used);
          step.next_of_kind = NoRow;
          step.prev_of_kind = m_catalog.last_row[k];
          step.step_kind = kind;
          step.ordinal = m_catalog.cummulative_steps[k];

          if (m_catalog.last_row[k] != NoRow) {
            rows[m_catalog.last_row[k]].next_of_kind = r;
          } else {
            m_catalog.first_row[k] = r;
          }
          m_catalog.last_row[k] = r;
          m_catalog.num_steps[k]++;
          m_catalog.cummulative_steps[k]++;

          // The increment may push past the read byte count --> Refill from that point
          // next
          used += unit_size(unit);
        }

        if (used == 0 && !at_end) {
          if (pos + res.bytes >= stream.file_size()) {
            // We found extra bytes that don't constitute a full DataUnit
            at_end = true; // Trailing bytes that don't form a unit
          } else {
            // A unit's locate_bytes exceed the scratch buffer
            status = IOStatus::PayloadTruncatedError;
          }
        }
        pos += used;
      }

      m_catalog.index_epoch++;

      sv.template release<roles::Metadata, index_ids::Scratch>(scratch);
      sv.template release<roles::Index, index_ids::Steps>(rows);

      if (status != IOStatus::Success) {
        return status;
      }
      return (m_catalog.num_rows > carried_rows) ? IOStatus::Success : IOStatus::ZeroBytesRead;
    }

    template <class SV>
    SBIO_HD IOStatus generic_fetch(SV& sv,
                                   StepIdxType step_idx,
                                   DataAccessPtn ptn,
                                   SegmentCursor<FTraits>& cursor) {
      const auto kind { FTraits::kind_for_ptn(ptn) };
      const auto k { kind_idx(kind) };

      const auto* rows { acquire_rows(sv) };

      hd_std::uint32_t row { NoRow };
      IOStatus status { find_row(rows, kind, step_idx, cursor.offset_index[k], row) };

      if (status == IOStatus::Success) {
        cursor.offset_index[k] = row;
        const ByteRegion region { rows[row].region };

        visit_ptn<FTraits>(ptn, [&](auto P) {
          constexpr hd_std::size_t Id { decltype(P)::value };
          if (region.length > sv.template size<roles::Data, Id>()) {
            status = IOStatus::PayloadTruncatedError;
            return;
          }

          auto* buf { sv.template acquire<roles::Data, Id, MemTag>() };
          const auto res { m_streams[region.stream].read_at(buf, region.offset, region.length) };
          sv.template release<roles::Data, Id>(buf);

          cursor.read_count = res.bytes;
          status = res.status;
        });

        cursor.last_region = region;
      }

      sv.template release<roles::Index, index_ids::Steps>(const_cast<StepOffset<FTraits>*>(rows));
      return status;
    }

    template <class SV>
    SBIO_HD IOStatus generic_fetch_range(SV& sv,
                                         StepIdxType first,
                                         StepIdxType count,
                                         DataAccessPtn ptn,
                                         SegmentCursor<FTraits>& cursor) {
      const auto kind { FTraits::kind_for_ptn(ptn) };
      const auto k { kind_idx(kind) };

      const auto* rows { acquire_rows(sv) };

      hd_std::uint32_t first_row { NoRow };
      hd_std::uint32_t last_row { NoRow };
      IOStatus status { find_row(rows, kind, first, cursor.offset_index[k], first_row) };
      if (status == IOStatus::Success) {
        status = find_row(rows, kind, first + count - 1, first_row, last_row);
      }

      if (status == IOStatus::Success) {
        const ByteRegion first_region { rows[first_row].region };
        const ByteRegion last_region { rows[last_row].region };

        visit_ptn<FTraits>(ptn, [&](auto P) {
          constexpr hd_std::size_t Id { decltype(P)::value };
          const hd_std::size_t buf_size { sv.template size<roles::Data, Id>() };
          auto* buf {
            static_cast<char*>(sv.template acquire<roles::Data, Id, MemTag>())
          };

          const bool one_span {
            first_region.stream == last_region.stream && last_region.offset >= first_region.offset
          };
          const hd_std::size_t span {
            one_span ? (last_region.offset + last_region.length - first_region.offset) : 0
          };

          hd_std::size_t total { 0 };
          if (one_span && span <= buf_size) {
            // Will read the entire range in one read.
            // This means we may end up with interleaved steps of different kinds.
            // `get_data_in_buffer` will skip these when it walks over the batch
            const auto res { m_streams[first_region.stream].read_at(buf, first_region.offset, span) };
            status = res.status;
            total = res.bytes;
          } else {
            hd_std::uint32_t r { first_row };
            for (StepIdxType s = 0; s < count && status == IOStatus::Success; ++s) {
              const ByteRegion& region { rows[r].region };
              if (total + region.length > buf_size) {
                status = IOStatus::PayloadTruncatedError;
                break;
              }

              const auto res { m_streams[region.stream].read_at(buf + total, region.offset, region.length) };
              status = res.status;
              total += res.bytes;
              r = rows[r].next_of_kind;
            }
          }

          sv.template release<roles::Data, Id>(buf);
          cursor.read_count = total;
        });

        cursor.offset_index[k] = last_row;
        cursor.last_region = last_region;
      }

      sv.template release<roles::Index, index_ids::Steps>(const_cast<StepOffset<FTraits>*>(rows));
      return status;
    }
  };
} // namespace sbio

#endif // SBIO_CORE_BROKER_HH
