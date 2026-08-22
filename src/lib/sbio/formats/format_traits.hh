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

#ifndef SBIO_FORMATS_FORMAT_TRAITS_HH
#define SBIO_FORMATS_FORMAT_TRAITS_HH

#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/stream.hh"

#include <concepts>
#include <cstdint>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  /**
   * @brief Indicates the strategy used to partition data across streams.
   *
   * When partitioning data from a single "logical unit" across multiple data streams
   * written over time, one can imagine two basic strategies:
   *  1. At each point in time, the logical unit is sub-divided and a portion
   *     is written to each data stream.
   *  2. At each point in time, the entire logical unit is written to one stream,
   *     and then the data is round-robined (or via some other selection mechanism
   *     distributed) across the other streams.
   * - SubDivide indicates the format uses strategy 1.
   * - Chronological indicates the format uses strategy 2.
   */
  enum class StreamPartitioningStrategy : std::uint8_t {
    SubDivide = 0,
    Chronological = 1
  };

  enum class StreamSentinels : std::uint8_t {
    RequestExhausted = 0, ///< All data units from this read request were read
    StreamExhausted = 1   ///< The stream is entirely exhausted (no more data)
  };

  /**
   * Base class tag for a data-format implementation.
   *
   * This tag, paired iwth the FormatTraits concept below indicate a sbio data-format
   * implementation.
   */
  struct BaseTraits {};

  template <typename T>
  concept HasBoundedDataDimensions = requires {
    { T::MaxRank } -> std::convertible_to<std::uint16_t>;
    { T::MaxNameSize } -> std::convertible_to<std::uint16_t>;
    { T::HeaderSize } -> std::convertible_to<std::size_t>;
  };

  template <typename T>
  concept HasCountableDataUnits = requires {
    typename T::DataUnit;

    // Unit type for indexing and selecting data units ("events")
    typename T::StepIdxType;
    requires std::convertible_to<typename T::StepIdxType, std::size_t>;

    // Indicator all units read
    { T::ExhaustedSentinel } -> std::convertible_to<typename T::StepIdxType>;
  };

  namespace impl {
    // Placeholders for DataSource, StreamBroker and BrokerGroup, etc. to satisfy concepts
    struct PlaceholderBroker {};
    struct PlaceholderBrokerGroup {};
    struct PlaceholderSegmentRef {};
    struct PlaceholderDataSource {};
  }; // namespace impl

  template <typename T>
  concept CanFindAndConfigureStreams = requires(impl::PlaceholderDataSource& ds,
                                                const typename T::DataSourceParameters& spec,
                                                typename T::StreamParameters& cfg) {
    // Master DataSource parameters for finding the streams
    typename T::DataSourceParameters;

    // Fill in the StreamBrokers
    { T::make_stream_brokers(ds, spec, cfg) } -> std::convertible_to<bool>;

    // Specifies the Stream partitioning strategy
    { T::PartitioningStrategy } -> std::convertible_to<StreamPartitioningStrategy>;

    // Has enumerator of Stream roles and a count of the total roles
    typename T::Roles;
    { T::RoleCount } -> std::convertible_to<std::size_t>;

    // Has definition of supported access patterns
    typename T::DataAccessPtn;
    { T::DataAccessPtnCount } -> std::convertible_to<std::size_t>;

    // Exposes a StreamParameters struct to allow runtime config
    typename T::StreamParameters;
    requires std::is_trivially_copyable_v<typename T::StreamParameters>;
    requires !std::is_void_v<std::remove_cvref_t<typename T::StreamParameters>>;
  };

  template <typename T>
  concept CanAllocateStorage = requires(typename T::StreamParameters& cfg) {
    // Has a type list of buffer descriptors to provide StreamBroker with Storage reqs.
    typename T::BrokerBufferRequirements;

    // Provides an interface to populate a Storage request based on runtime config
    { T::get_allocation_request(cfg) } -> std::convertible_to<AllocationRequest<T>>;

    // Provides a method to tell the BrokerGroup what the maximum number of steps
    // will be when using batched read APIs
    { T::max_batch_count(cfg) } -> std::convertible_to<std::size_t>;
  };

  template <typename T>
  concept HasDataRequest = requires {
    typename T::DataRequest;
    requires std::is_trivially_copyable_v<typename T::DataRequest>;
    requires !std::is_void_v<std::remove_cvref_t<typename T::DataRequest>>;
  };

  template <typename T, typename StorageViewT>
  concept HasStreamState = requires(StorageViewT& storage,
                                    const typename T::DiscoveryState& state) {
    // Exposable metadata about what is in current Streams.
    typename T::MetadataInventory;
    // Tracking information for maintaining position in a Stream.
    typename T::DiscoveryState;

    // Can query indexing capacity and retrieve the currently filled buffer
    { T::capacity(storage, state) } -> std::convertible_to<std::size_t>;
    { T::current_buffer(storage, state) } -> std::convertible_to<void*>;
  };

  template <typename T, typename IO>
  concept CanOpenStreams = requires(Stream<IO, T>* streams,
                                    const typename T::StreamParameters& cfg) {
    { T::open_streams(streams, cfg) } -> std::convertible_to<IOStatus>;
  };

  template <typename T, typename IO, typename StorageViewT>
  concept CanDiscoverMetadata = requires(Stream<IO, T>* streams,
                                         StorageViewT& storage,
                                         typename T::MetadataInventory& inv) {
    { T::discover_metadata(streams, storage, inv) } -> std::convertible_to<IOStatus>;
  };

  template <typename T>
  concept CanFindGroupSegments = requires(const typename T::MetadataInventory& inv,
                                          const char* name,
                                          impl::PlaceholderSegmentRef* ref_out,
                                          std::size_t max_out,
                                          impl::PlaceholderBroker* broker,
                                          char* dettype,
                                          typename T::DataAccessPtn ptn) {
    { T::find_group_segments(inv,
                             name,
                             ref_out,
                             max_out,
                             broker,
                             dettype,
                             ptn) } -> std::convertible_to<std::size_t>;
  };

  template <typename T, typename IO, typename StorageViewT>
  concept CanIndexStreams = requires(Stream<IO, T>* streams,
                                     StorageViewT& storage,
                                     typename T::DiscoveryState& state,
                                     const typename T::StreamParameters& cfg) {
    { T::index_stream(streams, storage, state, cfg) } -> std::convertible_to<IOStatus>;
  };

  template <typename T, typename IO, typename StorageViewT>
  concept CanFetchStreamData = requires(Stream<IO, T>* streams,
                                        StorageViewT& storage,
                                        typename T::DiscoveryState& state,
                                        const typename T::StreamParameters& cfg,
                                        typename T::StepIdxType step_idx,
                                        typename T::DataAccessPtn ptn) {
    { T::fetch_step(streams, storage, state, cfg, step_idx, ptn) } -> std::convertible_to<IOStatus>;
  };

  template <typename T>
  concept CanResolveData = requires(void* buf,
                                    const typename T::MetadataInventory& inv,
                                    const typename T::DataRequest& req) {
    // Defines a struct for the result of data resolution requests
    typename T::DataResult;
    { T::resolve_data(buf, inv, req) } -> std::same_as<typename T::DataResult>;

    // Can give the total size of the retrieved data
    { T::get_payload_size(buf) } -> std::convertible_to<std::size_t>;
  };

  template <typename T, typename IO, class StorageViewT>
  concept CanFillBuffer = requires(StorageViewT& storage,
                                   const typename T::MetadataInventory& inv,
                                   const typename T::DataRequest& req,
                                   typename T::DataAccessPtn ptn,
                                   std::size_t batch_idx) {
    { T::get_data_in_buffer(storage, inv, req, ptn, batch_idx) } -> std::convertible_to<typename T::DataResult>;
  };

  template <typename T>
  concept HasStateSynch = requires(typename T::DiscoveryState& state) {
    { T::sync_vars(state) };
  };

  /**
   * The `FormatTraits` concept defines a conforming data-format implementation.
   *
   * A struct implementing the following constraints is both necessary and sufficient
   * to serve as a data-format implementation for the rest of the generic sbio
   * infrastructure.
   *
   * There are a number of additional concepts which can be used to test for optional
   * APIs and features of a data-format implementation.
   *
   * The concept is subdivided into a number of sub-concepts for various pieces of
   * the overall API. A conforming struct will look something like the following:
   *
   * @code{.cpp}
   * struct ImplementsFormatTraits {
   *   // HasBoundedDataDimensions
   *   // ------------------------
   *   static constexpr std::size_t HeaderSize { 0 };
   *   static constexpr std::uint16_t MaxRank { 1 };
   *   static constexpr std::uint16_t MaxNameSize { 1 };
   *
   *   // HasCountableDataUnits
   *   // ---------------------
   *   using DataUnit = void;
   *   using StepIdxType = std::size_t;
   *   static constexpr StepIdxType ExhaustedSentinel { static_cast<StepIdxType>(-1) };
   *
   *   // CanFindAndConfigureStreams
   *   // --------------------------
   *   struct DataSourceParameters {};
   *   static constexpr StreamPartitioningStrategy PartitioningStrategy {
   *     StreamPartitioningStrategy::SubDivide
   *   };
   *   enum Roles { };
   *   static constexpr std::size_t RoleCount { 0 };
   *   enum class DataAccessPtn : std::uint8_t { };
   *   static constexpr std::size_t DataAccessPtnCount { 0 };
   *   struct StreamParameters {};
   *
   *   template <typename DS>
   *   static bool make_stream_brokers(DS& ds,
   *                                   const DataSourceParameters& ds_params,
   *                                   StreamParameters& cfg);
   *
   *   // CanAllocateStorage
   *   // ------------------
   *   using BrokerBufferRequirements = TypeList<>;
   *   static AllocationRequest<T> get_allocation_request(StreamParameters& cfg);
   *   static std::size_t max_batch_count(StreamParameters& cfg);
   *
   *   // HasDataRequest
   *   // --------------
   *   struct DataRequest { };
   *
   *   // HasStreamState
   *   // --------------
   *   struct MetadataInventory { };
   *   struct DiscoveryState { };
   *
   *   template <class StorageViewT>
   *   static auto capacity(const StorageViewT& storage, const DiscoveryState& state);
   *
   *   template <class StorageViewT>
   *   static auto current_buffer(StorageViewT& storage, const DiscoveryState& state);
   *
   *   // CanOpenStreams
   *   // --------------
   *   template <IOTraits IO>
   *   static IOStatus open_streams(Stream<IO, T>* streams, const StreamParameters& cfg);
   *
   *   // CanDiscoverMetadata && CanFindGroupSegments
   *   // -------------------------------------------
   *   template <IOTraits IO, class StorageViewT>
   *   static IOStatus discover_metadata(Stream<IO, T>* streams,
   *                                     StorageViewT& storage,
   *                                     MetadataInventory& inv);
   *
   *    template <class DataBrokerType, class SegmentRef>
   *    static std::size_t find_group_segments(const MetadataInventory& inv,
   *                                           const char* name,
   *                                           SegmentRef* ref_out,
   *                                           std::size_t max_out,
   *                                           DataBrokerType* broker,
   *                                           char* grouptype = nullptr,
   *                                           DataAccessPtn ptn = DataAccessPtn::L1Accept);
   *
   *   // CanIndexStreams  [[ OPTIONAL ]]
   *   // ---------------
   *   template <IOTraits IO, class StorageViewT>
   *   static IOStatus index_stream(Stream<IO, T>* streams,
   *                                StorageViewT& storage,
   *                                DiscoveryState& stream_state,
   *                                const StreamParameters& cfg);
   *
   *   // CanFetchStreamData
   *   // ------------------
   *   template <IOTraits IO, class StorageViewT>
   *   static IOStatus fetch_step(Stream<IO, T>* streams,
   *                              StorageViewT& storage,
   *                              DiscoveryState& stream_state,
   *                              const StreamParameters& cfg,
   *                              StepIdxType step_idx,
   *                              DataAccessPtn ptn);
   *
   *   // CanResolveData && CanFillBuffer
   *   // -------------------------------
   *   static inline DataResult resolve_data(void* buf,
   *                                         const MetadataInventory& inv,
   *                                         const DataRequest& req);
   *   static std::size_t get_payload_size(void* buf);
   *
   *   template <class StorageViewT>
   *   static DataResult get_data_in_buffer(StorageViewT& storage,
   *                                        const MetadataInventory& inv,
   *                                        const DataRequest& req,
   *                                        DataAccessPtn ptn,
   *                                        std::size_t batch_idx);
   *  };
   * @endcode
   */
  template <typename T, typename IO, typename EPolicy>
  concept FormatTraits =
    // Indicates size of headers, etc.
    HasBoundedDataDimensions<T>                                                     &&
    // Definition of "streamable" - Countable units, and indicates exhaustion:
    HasCountableDataUnits<T>                                                        &&
    CanFindAndConfigureStreams<T>                                                   &&
    CanAllocateStorage<T>                                                           &&
    HasDataRequest<T>                                                               &&
    HasStreamState<
      T,
      StorageView<Storage<typename T::BrokerBufferRequirements, EPolicy>, EPolicy>> &&
    CanOpenStreams<T, IO>                                                           &&
    CanDiscoverMetadata<
      T,
      IO,
      StorageView<Storage<typename T::BrokerBufferRequirements, EPolicy>, EPolicy>> &&
    CanFindGroupSegments<T>                                                         &&
    CanFetchStreamData<
      T,
      IO,
      StorageView<Storage<typename T::BrokerBufferRequirements, EPolicy>, EPolicy>> &&
    // Simple data fetching API
    CanResolveData<T>                                                               &&
    // Advanced data fetching API
    CanFillBuffer<
      T,
      IO,
      StorageView<Storage<typename T::BrokerBufferRequirements, EPolicy>, EPolicy>>;

  template <typename T>
  concept HasEventOffset = requires {
    typename T::EventOffset;
    requires std::is_trivially_copyable_v<typename T::EventOffset>;
    // TODO: Why fail?
    // requires !std::is_void_v<std::remove_cvref_t<typename T::EventOffset>;
  };

  template <typename T>
  concept HasTransitionOffset = requires {
    typename T::TransitionOffset;
    requires std::is_trivially_copyable_v<typename T::TransitionOffset>;
  };

  template <typename T, typename IO, class StorageViewT>
  concept IndexableFormatTraits =
    FormatTraits<T, IO, StorageViewT> && CanIndexStreams<T, IO, StorageViewT>;

  template <typename T, typename IO, class StorageViewT>
  concept SynchableFormatTraits = FormatTraits<T, IO, StorageViewT> && HasStateSynch<T>;

  template <typename T, typename IO, class StorageViewT>
  concept EventOffsetFormatTraits = FormatTraits<T, IO, StorageViewT> && HasEventOffset<T>;

  template <typename T, typename IO, class StorageViewT>
  concept TransitionOffsetFormatTraits = FormatTraits<T, IO, StorageViewT> && HasTransitionOffset<T>;


  template <typename T, typename IO, class StorageViewT>
  concept OffsetBasedFormatTraits =
    FormatTraits<T, IO, StorageViewT> && HasEventOffset<T> && HasTransitionOffset<T>;
} // namespace sbio

#endif // SBIO_FORMATS_FORMAT_TRAITS_HH
