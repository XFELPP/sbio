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

#ifndef SBIO_CORE_DATASOURCE_HH
#define SBIO_CORE_DATASOURCE_HH

#include "sbio/core/broker.hh"
#include "sbio/core/broker_group.hh"
#include "sbio/core/chronology.hh"
#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/locator.hh"
#include "sbio/core/metadata.hh"
#include "sbio/core/result.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/stream.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/util/string.hh"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility> // std::forward
#include <vector>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  template <class... Sources>
  class SourceSet;

  /**
   * A tag indicating the EPolicy was configured prior to DataSource construction.
   *
   * @note This is used by the DataSource to prevent re-configuration. The configuration
   *       may have been done by the `Context`, e.g., if using that interface.
   */
  struct ExecutionPolicyConfigured {};

  /**
   * The highest-level abstraction for defining the set of StreamBrokers that will be used.
   *
   * The DataSource is used to find and collect together the group of all StreamBrokers
   * that will be relevant for the IO of a specific dataset. The DataSource can be used
   * to spawn BrokerGroups to organize the StreamBrokers into logical groupings, and can
   * be used to generate indices to distribute among said groups to order the way in which
   * data is fetched and queried.
   *
   * @tparam IO The type of the IO strategy being used.
   * @tparam EPolicy The Execution policy to use for reading data.
   * @tparam FTraits The data format to read.
   * @tparam BrokerType The type of the StreamBroker being used. In general, the
   *         StreamBroker does not need to be subclassed, and can be instantiated
   *         directly. In that case, this template parameter can be left on its
   *         default value. Only if using a StreamBroker subclass does it need to be
   *         changed.
   */
  template <
    IOTraits IO,
    class EPolicy,
    FormatTraits<IO, EPolicy> FTraits,
    IsStreamBroker BrokerType = StreamBroker<IO, EPolicy, FTraits>,
    std::size_t MaxDataStreams = 128
  >
  class DataSource {
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
    using StreamConfig = GenericStreamConfig<FTraits>;

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
     * A SourceSet drives the reindexing of the DataSources it iterates.
     *
     * Reindexing is private - it was easiest to let the SourceSet be a friend.
     */
    template <class... Sources>
    friend class SourceSet;

    /**
     * Whether the steps of the StreamBrokers are merged in time.
     *
     * Chronological formats with a generic index, which name the kind of step that is
     * spread over the streams (`ChronologicalKind`), are merged by the steps' order keys.
     * Otherwise steps are dealt round-robin to the StreamBrokers.
     */
    static constexpr bool MergesStreams {
      FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological &&
      BrokerType::UsesGenericIndex &&
      requires { FTraits::ChronologicalKind; }
    };

    DataSource() = default;

    /**
     * Construct a DataSource, with the execution policy already configured.
     *
     * @note This constructor is used by the `Context`, which manages various
     *       configuration steps itself.
     */
    explicit DataSource(ExecutionPolicyConfigured)
      : m_epolicy_configured(true)
    {}

    /**
     * Construct a DataSource and initialize the Execution policy.
     *
     * @param[in] ecfg The configuration for the Execution policy.
     */
    explicit DataSource(const EPolicyConfig& ecfg) {
      configure_execution_policy(ecfg);
      m_epolicy_configured = true;
    }

    /**
     * Configure the Execution policy.
     *
     * The Execution policy must be configured before Streams are opened and read from
     * as the policy controls the allocation of the buffers that will be read into.
     *
     * @param[in] ecfg The configuration for the Execution policy.
     */
    inline void configure_execution_policy(const EPolicyConfig& ecfg) {
      EPolicy::configure(ecfg);
      m_epolicy_configured = true;
    }

    /*
     * Retrieve an environment variable, with optional fallback.
     *
     * Locators use this to find data (e.g. a data root directory).
     *
     * @note Overrides set on the DataSource (e.g. from the `Context` that created it)
     *       are searched before the process environment.
     *
     * @param[in] key The environment variable name.
     * @param[in] fallback The value if the variable is not set.
     * @returns The value of the variable, or `fallback`.
     */
    std::string env(std::string_view key, std::string_view fallback = {}) const {
      if (auto it { m_env.find(std::string(key)) }; it != m_env.end()) {
        return it->second;
      }

      if (const char* value { std::getenv(std::string(key).c_str()) }) {
        return value;
      }

      return std::string(fallback);
    }

    /**
     * Provide an override for an environment variable that only this DataSource uses.
     *
     * @param[in] key The environment variable name.
     * @param[in] value The value to use for it.
     */
    void set_env(std::string_view key, std::string_view value) {
      m_env[std::string(key)] = std::string(value);
    }

    /**
     * Add a new StreamBroker to the set tracked by the DataSource.
     *
     * @param[in] cfg The base configuration for the StreamBroker's Stream(s).
     *            This configuration may be modified slightly for the parameters
     *            that are required by the Stream, but do not make sense to be
     *            set directly by the user.
     * @returns Whether the incorporation of the StreamBroker, and subsequent
     *          configuration was successful.
     */
    SBIO_HD inline bool add_data_stream(const GenericStreamConfig<DataFormat>& cfg) {
      if (m_num_data_streams >= MaxDataStreams) {
        return false;
      }

      m_data_streams[m_num_data_streams].configure_broker(cfg);
      m_num_data_streams++;

      return true;
    }

    /**
     * Add a StreamBroker that reads a chain of chunks, in order.
     *
     * @param[in] chunks The configuration of each chunk, in chain order.
     * @param[in] num_chunks The number of chunks.
     * @returns Whether the StreamBroker could be added.
     */
    inline bool add_data_chain(const StreamConfig* chunks, std::size_t num_chunks) {
      if (num_chunks == 0 || !add_data_stream(chunks[0])) {
        return false;
      }

      m_pending_chunks.resize(m_num_data_streams);
      for (std::size_t c = 1; c < num_chunks; ++c) {
        m_pending_chunks[m_num_data_streams - 1].push_back(chunks[c]);
      }
      return true;
    }

    /**
     * Append a chunk to a StreamBroker's chain (e.g. one that appeared while live).
     */
    inline void append_chunk(std::size_t n_stream, const StreamConfig& chunk) {
      m_pending_chunks.resize(m_num_data_streams);
      m_pending_chunks[n_stream].push_back(chunk);
    }

    template <typename... Args>
    bool load_source(const StreamConfig& cfg, Args&&... args) {
      using Locator = typename FTraits::DefaultLocator;
      using LocatorParams = typename Locator::template LocatorParameters<DataFormat>;
      auto locator_params { LocatorParams(std::forward<Args>(args)...) };

      return Locator::find_streams(*this, locator_params, cfg);
    }

    template <IsLocator<DataSource> Locator, typename... Args>
    bool load_source_with(const StreamConfig& cfg, Args&&... args) {
      using LocatorParams = typename Locator::template LocatorParameters<DataFormat>;
      auto locator_params { LocatorParams(std::forward<Args>(args)...) };

      return Locator::find_streams(*this, locator_params, cfg);
    }

    /**
     * After finding StreamBrokers and opening connections, determine available metadata.
     *
     * The metadata discovery stage is crucial to be able to organize StreamBrokers into
     * their logical BrokerGroups. The metadata informs the rest of the infrastructure
     * what kind of data is available in the set of Streams beign read from.
     *
     * @returns The IOStatus result from performing metadata discovery.
     */
    SBIO_HD inline IOStatus discover_metadata() {
      // If the EPolicy configuration did NOT already take place, make sure a reset
      // happens here.
      if (!m_epolicy_configured) {
        EPolicy::configure(EPolicyConfig{}); // Use the default since none provided
        m_epolicy_configured = true;
      }
      std::size_t steps_capacity { std::numeric_limits<std::size_t>::lowest() };
      for (std::size_t n_stream = 0; n_stream < m_num_data_streams; ++n_stream) {
        IOStatus status = m_data_streams[n_stream].prepare();

        // Abort if any stream fails
        if (status != IOStatus::Success) {
          return status;
        }

        if constexpr (IsIndexableStreamBroker<BrokerType>) {
          status = m_data_streams[n_stream].index_stream();
        }

        if (status != IOStatus::Success) {
          return status;
        }

        // Update our total steps capacity as the data stream brokers tell us
        std::size_t stream_steps = m_data_streams[n_stream].capacity();
        if (stream_steps > steps_capacity) {
          steps_capacity = stream_steps;
        }
      }

      m_steps_capacity = steps_capacity;

      if constexpr (MergesStreams) {
        m_chronology = std::make_shared<ChronologicalOrder>(m_num_data_streams);
        for (std::size_t n_stream = 0; n_stream < m_num_data_streams; ++n_stream) {
          add_indexed_steps(n_stream);
        }

        m_chronology->merge();
        m_steps_capacity = m_chronology->size();
      }

      return IOStatus::Success;
    }

    /**
     * Request the next index for a step to read data for.
     *
     * The step indices are used to request data for a specific step/event. In general,
     * this corresponds to chronological ordering (i.e. a step is mappable to a time
     * point); however, this is by no means a requirement.
     *
     * @returns The next step index to fetch and query data for, or the ExhaustedSentinel if
     *          no more data is available.
     */
    SBIO_HD inline typename FTraits::StepIdxType next() const {
      auto trigger_reindexing = [&]() {
        return reindex_trigger();
      };

      return EPolicy::template next<FTraits>(m_iteration_state,
                                             m_steps_capacity,
                                             trigger_reindexing);
    }

    /**
     * Request the next contiguous batch of indices for steps to read data for.
     *
     * @note A batch is provided with a maximum size - it may be smaller, as it will
     *       not extend beyond the current maximum capactiy.
     * @note If `batch_size` is left to 0, then the default size will be `max_batch_size`
     *       used in the initial configuration of the StreamBrokers.
     * @note The batch_size will never be larger than max_batch_size either.
     *
     * @param[in] batch_size The maximum size of the batch of step indices to read.
     * @returns The next batch of step indices [first, last) to fetch data for. The
     *          Exhausted batch is returned as first and last equal to ExhaustedSentinel
     */
    SBIO_HD inline StepBatch<typename FTraits::StepIdxType>
    next_batch(std::size_t batch_size = 0) const {
      if (batch_size == 0 || batch_size > m_data_streams[0].config().max_batch_size) {
        batch_size = m_data_streams[0].config().max_batch_size;
      }
      auto trigger_reindexing = [&]() { return reindex_trigger(); };

      return EPolicy::template next_batch<FTraits>(m_iteration_state,
                                                   batch_size,
                                                   m_steps_capacity,
                                                   trigger_reindexing);
    }

    /**
     * Construct a BrokerGroup by name from the set of StreamBrokers.
     *
     * @note The metadata discovery step MUST have been passed through for BrokerGroup
     *       construction to work. A group cannot be formed until it is known what data
     *       is available from the set of Streams.
     *
     * @tparam MaxSegments The maximum number of segments (components) that will be
     *         allowed in the BrokerGroup.
     * @returns The BrokerGroup of the given name. The success of the group formation
     *          can be checked by inspecting the number of segments the returned
     *          group has (0 indicating no group could be formed).
     */
    template <std::size_t MaxSegments = 128>
    SBIO_HD inline BrokerGroup<BrokerType, FTraits, MaxSegments>
    get_stream_group(const char* name) const {
      using GroupTopologyType = GroupTopology<BrokerType, MaxSegments>;

      auto topo =
        GroupTopologyType::build_topology(name, m_data_streams, m_num_data_streams);
      topo.chronology = m_chronology.get();

      return BrokerGroup<BrokerType, FTraits, MaxSegments>(topo);
    }

    /**
     * The number of data Streams in the DataSource's set.
     */
    SBIO_HD inline std::size_t num_data_streams() const { return m_num_data_streams; }

    /**
     * A pointer to the first StreamBroker of the set.
     *
     * @returns A pointer to the first StreamBroker of the set.
     */
    SBIO_HD inline const BrokerType* data_streams() const { return &m_data_streams[0]; }
    /**
     * A pointer to the first StreamBroker of the set.
     *
     * @returns A pointer to the first StreamBroker of the set.
     */
    SBIO_HD inline BrokerType* data_streams() { return &m_data_streams[0]; }
    /**
     * The requested StreamBroker.
     *
     * @note This function does not bounds check! Be sure to inspect the number of
     *       data streams before indexing out of bounds!
     *
     * @param[in] i The StreamBroker to return.
     * @returns The requested StreamBroker.
     */
    SBIO_HD inline const BrokerType& data_stream(std::size_t i) const { return m_data_streams[i]; }
    /**
     * The requested StreamBroker.
     *
     * @note This function does not bounds check! Be sure to inspect the number of
     *       data streams before indexing out of bounds!
     *
     * @param[in] i The StreamBroker to return.
     * @returns The requested StreamBroker.
     */
    SBIO_HD inline BrokerType& data_stream(std::size_t i) { return m_data_streams[i]; }

    /**
     * An iterator implementation to allow generating step indices from the DataSource.
     *
     * The iterator on a DataSource is used to automatically generate new step indices
     * until an ExhaustedSentinel is returned.
     *
     * @tparam DS The DataSource to iterate over.
     */
    template <class DS>
    class IteratorImpl {
    public:
      // Values generated on the fly so reference type is really value type
      using iterator_category = std::input_iterator_tag; // One-direction
      using difference_type = std::ptrdiff_t;
      using value_type = typename FTraits::StepIdxType;
      // using pointer = value_type*;
      using pointer = void;
      using reference = value_type;

      SBIO_HD IteratorImpl(DS& ds, value_type idx)
        : m_ds(ds)
        , m_idx(idx)
      {}

      // We're going to just generate indices and return by value for now.
      // The normal reference/pointer iterator semantics don't match perfectly
      // because there's nothing to "point to" or "reference".
      SBIO_HD reference operator*() const { return m_idx; }

      // pointer operator->() { } // Not immediately sure how to do this

      // Incrementing the iterator calls the associated datasource's next function
      // This will trigger stream reindexing and so on.
      // You'll either get a new index. Or the ExhaustedSentinel if done.
      SBIO_HD IteratorImpl& operator++() {
        m_idx = m_ds.next();
        return *this;
      }
      SBIO_HD IteratorImpl operator++(int) {
        IteratorImpl tmp = *this;
        ++(*this);
        return tmp;
      }

      SBIO_HD friend bool operator==(const IteratorImpl& a, const IteratorImpl& b) {
        // Need to figure out best way to compare DataSource
        // For now, just punt and return comparison of indices...
        return (a.m_idx == b.m_idx);
      }

      SBIO_HD friend bool operator!=(const IteratorImpl& a, const IteratorImpl& b) {
        return !(a == b);
      }

    private:
      DS& m_ds;
      typename FTraits::StepIdxType m_idx;
    };

    using Iterator = IteratorImpl<DataSource>;
    using ConstIterator = IteratorImpl<const DataSource>;

    /**
     * A range over the steps of a DataSource, for use in a range-based for loop.
     *
     * Each parallel executing unit (e.g. a thread or rank, etc.) iterating will setup
     * its own range-based loop (`for (auto step : ds.steps()) {}`).
     *
     * @note Early exit from a loop invokes the EPolicy end iteration implementation.
     *       This may be a no-op for some policies.
     *
     * @tparam DS The DataSource to iterate over (const/non-const...)
     */
    template <class DS>
    class StepRangeImpl {
    public:
      using It = IteratorImpl<DS>;

      SBIO_HD explicit StepRangeImpl(DS& ds)
        : m_ds(ds)
      {}

      SBIO_HD ~StepRangeImpl() {
        auto trigger_reindexing = [&]() { return m_ds.reindex_trigger(); };

        EPolicy::template end_iteration<FTraits>(m_ds.m_iteration_state,
                                                 m_ds.m_steps_capacity,
                                                 trigger_reindexing);
      }

      StepRangeImpl(const StepRangeImpl&) = delete;
      StepRangeImpl& operator=(const StepRangeImpl&) = delete;

      /**
       * Return an iterator at the first step index.
       *
       * @note This will invoke `next` to get the first step *for this unit*
       *
       * @returns An iterator at the first step index.
       */
      SBIO_HD It begin() const { return It(m_ds, m_ds.next()); }

      /**
       * Return an iterator pointing to the ExhaustedSentinel.
       *
       * @returns An iterator pointing to the ExhaustedSentinel.
       */
      SBIO_HD It end() const { return It(m_ds, FTraits::ExhaustedSentinel); }

    private:
      DS& m_ds;
    };

    using StepRange = StepRangeImpl<DataSource>;
    using ConstStepRange = StepRangeImpl<const DataSource>;

    /**
     * Return a range over the steps of this DataSource for the calling unit.
     *
     * @returns A range to iterate over in a range-based for loop.
     */
    SBIO_HD StepRange steps() { return StepRange(*this); }
    SBIO_HD ConstStepRange steps() const { return ConstStepRange(*this); }
    SBIO_HD ConstStepRange csteps() const { return ConstStepRange(*this); }

    /**
     * Return an iterator at the first step index.
     *
     * @note This will invoke `next` to get the first step *for this unit*
     * @note Using the DataSource directly for iteration doesn't use the EPolicy's
     *       end iteration implementation for early exits. This must be manually
     *       managed then, or instead, prefer the range-based loop with `steps()`.
     *
     * @returns An iterator at the first step index.
     */
    SBIO_HD Iterator begin() { return Iterator(*this, next()); }
    SBIO_HD ConstIterator begin() const { return ConstIterator(*this, next()); }
    /**
     * Return an iterator pointing to the ExhasutedSentinel.
     *
     * @returns An iterator pointing to the ExhaustedSentinel.
     */
    SBIO_HD Iterator end() { return Iterator(*this, FTraits::ExhaustedSentinel); }
    SBIO_HD ConstIterator end() const { return ConstIterator(*this, FTraits::ExhaustedSentinel); }

    SBIO_HD ConstIterator cbegin() const { return ConstIterator(*this, next()); }
    SBIO_HD ConstIterator cend() const { return ConstIterator(*this, FTraits::ExhaustedSentinel); }

    /**
     * An iterator generating batches of steps from the DataSource.
     *
     * @tparam DS The DataSource to iterate over (const/non-const...)
     */
    template <typename DS>
    class BatchIteratorImpl {
    public:
      // Values generated on the fly so reference type is really value type
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = StepBatch<typename FTraits::StepIdxType>;
      // using pointer = value_type*;
      using pointer = void;
      using reference = value_type;

      SBIO_HD BatchIteratorImpl(DS& ds, value_type batch, std::size_t batch_size)
        : m_ds(ds)
        , m_batch(batch)
        , m_batch_size(batch_size)
      {}

      SBIO_HD reference operator*() const { return m_batch; }

      SBIO_HD BatchIteratorImpl& operator++() {
        m_batch = m_ds.next_batch(m_batch_size);
        return *this;
      }

      SBIO_HD friend bool operator==(const BatchIteratorImpl& a, const BatchIteratorImpl& b) {
        // Need to figure out best way to compare DataSource
        // For now, just punt and return comparison of indices...
        return a.m_batch.first == b.m_batch.first;
      }

      SBIO_HD friend bool operator!=(const BatchIteratorImpl& a, const BatchIteratorImpl& b) {
        return !(a == b);
      }

    private:
      DS& m_ds;
      value_type m_batch;
      std::size_t m_batch_size;
    };

    using BatchIterator = BatchIteratorImpl<DataSource>;
    using ConstBatchIterator = BatchIteratorImpl<const DataSource>;

    /**
     * A range over contiguous batches of steps, for use in a range-based for
     * loop.
     *
     * Each parallel executing unit (e.g. a thread or rank, etc.) iterating will setup
     * its own range-based loop (`for (auto step : ds.batches()) {}`).
     *
     * @note Early exit from a loop invokes the EPolicy end iteration implementation
     *       This may be a no-op for some policies.
     *
     * @tparam DS The DataSource to iterate over (const/non const...)
     */
    template <class DS>
    class BatchRangeImpl {
    public:
      using BatchIt = BatchIteratorImpl<DS>;

      SBIO_HD BatchRangeImpl(DS& ds, std::size_t batch_size)
        : m_ds(ds)
        , m_batch_size(batch_size)
      {}

      SBIO_HD ~BatchRangeImpl() {
        auto trigger_reindexing = [&]() { return m_ds.reindex_trigger(); };

        EPolicy::template end_iteration<FTraits>(m_ds.m_iteration_state,
                                                 m_ds.m_steps_capacity,
                                                 trigger_reindexing);
      }

      BatchRangeImpl(const BatchRangeImpl&) = delete;
      BatchRangeImpl& operator=(const BatchRangeImpl&) = delete;

      SBIO_HD BatchIt begin() const {
        return BatchIt(m_ds, m_ds.next_batch(m_batch_size), m_batch_size);
      }

      SBIO_HD BatchIt end() const {
        using StepIdx = typename FTraits::StepIdxType;

        return BatchIt(m_ds,
                       StepBatch<StepIdx> {
                         FTraits::ExhaustedSentinel,
                         FTraits::ExhaustedSentinel
                       },
                       m_batch_size);
      }

    private:
      DS& m_ds;
      std::size_t m_batch_size;
    };

    using BatchRange = BatchRangeImpl<DataSource>;
    using ConstBatchRange = BatchRangeImpl<const DataSource>;

    /**
     * Return a range over contiguous batches of steps for the calling unit.
     *
     * @param[in] batch_size The maximum number of steps per batch. The default is the
     *            configured `max_batch_size`.
     * @returns A range to iterate over in a range-based for loop.
     */
    SBIO_HD BatchRange batches(std::size_t batch_size = 0) {
      return BatchRange(*this, batch_size);
    }
    SBIO_HD ConstBatchRange batches(std::size_t batch_size = 0) const {
      return ConstBatchRange(*this, batch_size);
    }
    SBIO_HD ConstBatchRange cbatches(std::size_t batch_size = 0) const {
      return ConstBatchRange(*this, batch_size);
    }

  private:
    /**
     * Move a StreamBroker to its next chunk, if its chunk is done and another is waiting.
     */
    inline void advance_chunk_if_done(std::size_t n_stream) const {
      if (n_stream >= m_pending_chunks.size() || m_pending_chunks[n_stream].empty()) {
        return;
      }

      auto& broker { m_data_streams[n_stream] };
      if (broker.catalog().chunk_complete) {
        broker.advance_chunk(m_pending_chunks[n_stream].front());
        m_pending_chunks[n_stream].pop_front();
      }
    }

    /**
     * Add a StreamBroker's newly indexed steps to the order to merge.
     *
     * A StreamBroker that indexed nothing new is exhausted.
     */
    inline void add_indexed_steps(std::size_t n_stream) const {
      auto& broker { m_data_streams[n_stream] };

      std::size_t new_rows { 0 };
      for (const auto n : broker.catalog().num_steps) {
        new_rows += n;
      }

      if (new_rows == 0) {
        m_chronology->set_exhausted(n_stream);
        return;
      }

      broker.visit_steps(FTraits::ChronologicalKind, [&](std::uint64_t ordinal, std::uint64_t key) {
        m_chronology->add(n_stream, ordinal, key);
      });
    }

    /**
     * Index StreamBrokers further, until more steps are merged or all are exhausted.
     *
     * StreamBrokers with steps waiting to be merged keep their index.
     *
     * @returns Whether more steps were merged.
     */
    inline bool merge_trigger() const {
      while (true) {
        bool indexed { false };
        for (std::size_t n_stream = 0; n_stream < m_num_data_streams; ++n_stream) {
          if (!m_chronology->may_index(n_stream)) {
            continue;
          }

          advance_chunk_if_done(n_stream);
          IOStatus status { m_data_streams[n_stream].index_stream() };

          // Continue through empty chunks in the chain (i.e. files/streams w/ no steps)
          while (status == IOStatus::ZeroBytesRead                &&
                 n_stream < m_pending_chunks.size()               &&
                 !m_pending_chunks[n_stream].empty()              &&
                 m_data_streams[n_stream].catalog().chunk_complete) {
            advance_chunk_if_done(n_stream);
            status = m_data_streams[n_stream].index_stream();
          }

          if (status == IOStatus::Success) {
            add_indexed_steps(n_stream);
          } else {
            // No more data (ZeroBytesRead), or it cannot be read
            m_chronology->set_exhausted(n_stream);
          }
          indexed = true;
        }

        if (m_chronology->merge() > 0) {
          m_steps_capacity = m_chronology->size();
          return true;
        }

        if (!indexed) {
          return false; // Everything is exhausted
        }
      }
    }

    SBIO_HD inline bool reindex_trigger() const {
      if constexpr (MergesStreams) {
        return merge_trigger();
      }

      using StepIdx = typename FTraits::StepIdxType;

      StepIdx total_capacity { std::numeric_limits<StepIdx>::lowest() };
      bool failed { false };

      for (std::size_t n_stream = 0; n_stream < m_num_data_streams; ++n_stream) {
        advance_chunk_if_done(n_stream);
        IOStatus status { m_data_streams[n_stream].index_stream() };

        // Continue through empty chunks in the chain (i.e. files/streams w/ no steps)
        while (status == IOStatus::ZeroBytesRead                &&
               n_stream < m_pending_chunks.size()               &&
               !m_pending_chunks[n_stream].empty()              &&
               m_data_streams[n_stream].catalog().chunk_complete) {
          advance_chunk_if_done(n_stream);
          status = m_data_streams[n_stream].index_stream();
        }

        if (status != IOStatus::Success) {
          failed = true;
          continue;
        }

        StepIdx stream_capacity = m_data_streams[n_stream].capacity();

        if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
          total_capacity += stream_capacity;
        } else {
          if (stream_capacity > total_capacity) {
            total_capacity = stream_capacity;
          }
        }
      }

      if (failed) {
        return total_capacity > 0;
      }

      m_steps_capacity += total_capacity;

      return total_capacity > 0;
    }

    /**
     * The set of StreamBrokers in the DataSource
     */
    mutable BrokerType m_data_streams[MaxDataStreams];

    /**
     * The total number of StreamBrokers in the DataSource
     */
    std::size_t m_num_data_streams { 0 };

    /**
     * The current steps capacity before reindexing is required.
     */
    mutable std::size_t m_steps_capacity { 0 };

    /**
     * The order of steps across StreamBrokers (see `MergesStreams`).
     *
     * @note This is held by pointer. BrokerGroups refer to it, and it must not
     *       move with the DataSource.
     */
    std::shared_ptr<ChronologicalOrder> m_chronology {};

    /**
     * Environment variable overrides (e.g. from the `Context`).
     */
    std::map<std::string, std::string> m_env {};

    /**
     * Remaining chunks to read per StreamBroker (in order).
     */
    mutable std::vector<std::deque<StreamConfig>> m_pending_chunks {};

    /**
     * The EPolicy's state for iterating and distribution steps from this DataSource.
     */
    mutable typename EPolicy::IterationState m_iteration_state {};

    /**
     * Flag to track whether the Execution policy has been configured.
     *
     * Since there may be global state in the policy, ensure that a configuration
     * gets called. If its explicitly provided, then that gets used. Otherwise, a
     * default configuration will be invoked before any IO. This allows any global
     * state to be reset if iteratively creating multiple DataSource instantiations
     * over time. This only applies to static state (generally, process and environment).
     * The remaining state is held in the member `m_iteration_state`.
     */
    bool m_epolicy_configured { false };
  };
} // namespace sbio

#endif // SBIO_CORE_DATASOURCE_HH
