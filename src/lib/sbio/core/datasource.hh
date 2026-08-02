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
#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/stream.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/util/string.hh"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <utility> // std::forward

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace fs = std::filesystem;

namespace sbio {
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

    DataSource() = default;

    /**
     * Construct a DataSource and initialize the Execution policy.
     *
     * @param[in] ecfg The configuration for the Execution policy.
     */
    explicit DataSource(const EPolicyConfig& ecfg) {
      configure_execution_policy(ecfg);
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
    SBIO_HD inline bool add_data_stream(const StreamConfig& cfg) {
      if (m_num_data_streams >= MaxDataStreams) {
        return false;
      }

      m_data_streams[m_num_data_streams].configure_broker(cfg);
      m_num_data_streams++;

      return true;
    }

    /**
     * load_run: Scans the standard hutch directory and automatically registers
     * all streams for a given experiment and run number.
     */
    template <typename... Args>
    bool load_run(StreamConfig base_cfg, Args&&... args) {
      auto ds_params = typename FTraits::DataSourceParameters(std::forward<Args>(args)...);

      return FTraits::make_stream_brokers(*this, ds_params, base_cfg);
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
      using StepIdx = typename FTraits::StepIdxType;

      auto trigger_reindexing = [&] () {
        StepIdx total_capacity { std::numeric_limits<StepIdx>::lowest() };
        bool failed { false };

        for (std::size_t n_stream = 0; n_stream < m_num_data_streams; ++n_stream) {
          IOStatus status = m_data_streams[n_stream].index_stream();

          if (status != IOStatus::Success) {
            failed = true;
            continue;
          }

          StepIdx stream_capacity = m_data_streams[n_stream].capacity();

          if constexpr (FTraits::PartitioningStrategy ==
                        StreamPartitioningStrategy::Chronological) {
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
      };

      return EPolicy::template next<FTraits>(m_steps_capacity, trigger_reindexing);
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
    get_stream_group(const char* name) {
      using BrokerGroupType = BrokerGroup<BrokerType, FTraits, MaxSegments>;
      typename BrokerGroupType::DataSegmentRef segments[MaxSegments] {};

      std::uint32_t stream_indices[MaxSegments] {};

      std::size_t n_streams_found { 0 };

      // File formats may have various logical groupings of the data streams, which
      // in turn may require different lookup procedures. The pass loop tests them
      // in turn, as just given a name, it cannot be known which access pattern is needed.
      char final_type[FTraits::MaxNameSize] = "unknown";
      for (std::size_t pass = 0; pass < FTraits::DataAccessPtnCount; ++pass) {
        auto ptn = static_cast<typename FTraits::DataAccessPtn>(pass);
        for (std::size_t i = 0; i < m_num_data_streams && n_streams_found < MaxSegments; ++i) {
          char dettype[FTraits::MaxNameSize] = "unknown";
          n_streams_found += FTraits::find_group_segments(m_data_streams[i].metadata(),
                                                          name,
                                                          &segments[n_streams_found],
                                                          MaxSegments - n_streams_found,
                                                          &m_data_streams[i],
                                                          dettype,
                                                          ptn);

          if constexpr (FTraits::PartitioningStrategy ==
                        StreamPartitioningStrategy::Chronological) {
            // In this case there will be 1 per stream, so on each iteration
            // collect the appropriate stream_idx to sort.
            stream_indices[n_streams_found - 1] =
              segments[n_streams_found - 1].broker->stream_idx();
          }

          if (std::strcmp(dettype, "unknown") != 0) {
            std::strcpy(final_type, dettype);
          }
        }
        // TODO: FIX SEARCH TO AVOID NAME COLLISIONS (E.G. "jungfrau" and PVs..)
        if (n_streams_found >= MaxSegments) {
          break;
        }
      }

      std::size_t num_segments { n_streams_found };

      // Some file formats will divide the data chronologically across streams
      // instead of sub-dividing a single logical unit for each time point. (e.g. XTC1)
      // The exposed stream_idx() returns a value that can be used to order the
      // the streams then. Use this to attach the correct ordering to the segment_no.
      // stream(s). The traits indicate whether they should forcibly be sequenced.
      if constexpr (FTraits::PartitioningStrategy == StreamPartitioningStrategy::Chronological) {
        if (num_segments > 0) {
          std::uint32_t final_stream_indices[MaxSegments] {};
          std::uint32_t smallest { stream_indices[0] };
          for (std::size_t j = 0; j < num_segments; ++j) {
            final_stream_indices[j] = j;
            if (stream_indices[j] < smallest) {
              smallest = stream_indices[j];
            }
          }

          // Sort with a circular wrapping accounted for in the fiducial.
          for (std::size_t i = 0; i < num_segments - 1; ++i) {
            std::uint32_t best { static_cast<std::uint32_t>(i) };
            for (std::size_t j = i + 1; j < num_segments; ++j) {
              std::uint32_t diff_best { stream_indices[final_stream_indices[best]] - smallest };
              std::uint32_t diff_j { stream_indices[final_stream_indices[j]] - smallest };

              if (diff_j < diff_best) {
                best = j;
              }
            }

            if (best != i) {
              std::uint32_t tmp { final_stream_indices[i] };
              final_stream_indices[i] = final_stream_indices[best];
              final_stream_indices[best] = tmp;
            }
          }

          // Use the sorted stream indices as the "segment"
          // The broker will know that Chronological partitioning requires a different
          // interpretation of the segment numbering
          typename BrokerGroupType::DataSegmentRef sorted_segments[MaxSegments] {};
          for (std::size_t j = 0; j < num_segments; ++j) {
            sorted_segments[j] = segments[final_stream_indices[j]];
            sorted_segments[j].segment_no = 0;
            // segments[j].segment_no = final_stream_indices[j];
          }

          return BrokerGroupType(name, final_type, num_segments, sorted_segments);
        } else {
          return BrokerGroupType(name, final_type, num_segments, segments);
        }
      } else {
        return BrokerGroupType(name, final_type, num_segments, segments);
      }
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
      using iterator_category = std::forward_iterator_tag; // One-direction
      using difference_type = std::ptrdiff_t;
      using value_type = typename FTraits::StepIdxType;
      // using pointer = value_type*;
      using pointer = void;
      using reference = value_type;

      IteratorImpl(DS& ds, value_type idx)
        : m_ds(ds)
        , m_idx(idx)
      {}

      // We're going to just generate indices and return by value for now.
      // The normal reference/pointer iterator semantics don't match perfectly
      // because there's nothing to "point to" or "reference".
      reference operator*() const { return m_idx; }

      // pointer operator->() { } // Not immediately sure how to do this

      // Incrementing the iterator calls the associated datasource's next function
      // This will trigger stream reindexing and so on.
      // You'll either get a new index. Or the ExhaustedSentinel if done.
      IteratorImpl& operator++() {
        m_idx = m_ds.next();
        return *this;
      }
      IteratorImpl operator++(int) {
        IteratorImpl tmp = *this;
        ++(*this);
        return tmp;
      }

      friend bool operator==(const IteratorImpl& a, const IteratorImpl& b) {
        // Need to figure out best way to compare DataSource
        // For now, just punt and return comparison of indices...
        return (a.m_idx == b.m_idx);
      }

      friend bool operator!=(const IteratorImpl& a, const IteratorImpl& b) {
        return !(a == b);
      }

    private:
      DS& m_ds;
      typename FTraits::StepIdxType m_idx;
    };

    using Iterator = IteratorImpl<DataSource>;
    using ConstIterator = IteratorImpl<const DataSource>;

    /**
     * Return an iterator at the first step index.
     *
     * @returns An iterator at the first step index.
     */
    Iterator begin() { return Iterator(*this, 0); }
    /**
     * Return an iterator pointing to the ExhasutedSentinel.
     *
     * @returns An iterator pointing to the ExhaustedSentinel.
     */
    Iterator end() { return Iterator(*this, FTraits::ExhaustedSentinel); }

    ConstIterator begin() const { return ConstIterator(*this, 0); }
    ConstIterator end() const { return ConstIterator(*this, FTraits::ExhaustedSentinel); }

  private:
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
  };
} // namespace sbio

#endif // SBIO_CORE_DATASOURCE_HH
