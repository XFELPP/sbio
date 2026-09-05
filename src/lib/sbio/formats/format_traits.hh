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
#include "sbio/core/metadata.hh"
#include "sbio/core/result.hh"
#include "sbio/core/roles.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/stream.hh"
#include "sbio/core/types.hh"

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

  template <typename T>
  concept CanFindAndConfigureStreams = requires(GenericStreamConfig<T>& cfg) {
    // Specifies the Stream partitioning strategy
    { T::PartitioningStrategy } -> std::convertible_to<StreamPartitioningStrategy>;

    // Has a list of all possible Stream variants, with a size member on the list
    typename T::StreamTypes;
    requires IsStreamSet<typename T::StreamTypes>;
    // StreamSet inherits this already from type_list, but just in case add it explicitly
    // to prevent breaks unexpectedly if things are refactored
    { T::StreamTypes::size() } -> std::convertible_to<std::size_t>;

    // Has definition of supported access patterns
    typename T::DataAccessPtn;
    { T::DataAccessPtnCount } -> std::convertible_to<std::size_t>;

    // Exposes a StreamParameters struct to allow runtime config
    typename T::StreamParameters;
    requires std::is_trivially_copyable_v<typename T::StreamParameters>;
    requires !std::is_void_v<std::remove_cvref_t<typename T::StreamParameters>>;
  };

  template <typename T>
  concept CanAllocateStorage = requires(GenericStreamConfig<T>& cfg) {
    // Has a type list of buffer descriptors to provide StreamBroker with Storage reqs.
    typename T::BrokerBufferRequirements;

    // Provides an interface to populate a Storage request based on runtime config
    { T::get_allocation_request(cfg) } -> std::convertible_to<AllocationRequest<T>>;
  };

  template <typename T>
  concept HasDataRequest = requires {
    typename T::DataRequest;
    requires std::is_trivially_copyable_v<typename T::DataRequest>;
    requires !std::is_void_v<std::remove_cvref_t<typename T::DataRequest>>;
  };

  template <typename T, typename StorageViewT>
  concept HasStreamState = requires(StorageViewT& storage) {
    // Additional fields that may be associated to a detector segment
    // e.g., like a serial number. Not used for lookup and may be an empty schema.
    typename T::GroupKeys;
    // Additional fields that may be needed to traverse data using lookup tables
    // Otherwise, lookup table is generic. This type/struct can be empty though.
    typename T::FieldMetadata;
  };

  template <typename T, typename IO, typename StorageViewT>
  concept CanDiscoverMetadata = requires(Stream<IO, T>* streams,
                                         StorageViewT& storage,
                                         MetadataInventory<T> inv,
                                         std::size_t entry_no,
                                         const char* name,
                                         typename T::DataAccessPtn ptn) {
    { T::discover_metadata(streams, storage, inv) } -> std::convertible_to<IOStatus>;
  };

  template <typename T, typename IO, typename StorageViewT>
  concept CanIndexStreams = requires(Stream<IO, T>* streams,
                                     StorageViewT& storage,
                                     StreamCatalog<T>& catalog,
                                     IndexingCursor<T>& cursor,
                                     const GenericStreamConfig<T>& cfg) {
    { T::index_stream(streams, storage, catalog, cursor, cfg) } -> std::convertible_to<IOStatus>;
  };

  template <typename T, typename IO, typename StorageViewT>
  concept CanFetchStreamData = requires(Stream<IO, T>* streams,
                                        StorageViewT& storage,
                                        const StreamCatalog<T>& catalog,
                                        FetchCursor<T>& cursor,
                                        const GenericStreamConfig<T>& cfg,
                                        typename T::StepIdxType step_idx,
                                        typename T::DataAccessPtn ptn) {
    { T::fetch_step(streams, storage, catalog, cursor, cfg, step_idx, ptn) } -> std::convertible_to<IOStatus>;
  };

  template <typename T>
  concept CanResolveData = requires(void* buf,
                                    const MetadataInventory<T>& inv,
                                    const typename T::DataRequest& req) {
    // Can resolve data into a sbio DataResult
    { T::resolve_data(buf, inv, req) } -> std::same_as<DataResult>;

    // Can give the total size of the retrieved data
    { T::get_payload_size(buf) } -> std::convertible_to<std::size_t>;
  };

  template <typename T, typename IO, class StorageViewT>
  concept CanFillBuffer = requires(StorageViewT& storage,
                                   const MetadataInventory<T>& inv,
                                   const typename T::DataRequest& req,
                                   typename T::DataAccessPtn ptn,
                                   std::size_t batch_idx) {
    { T::get_data_in_buffer(storage, inv, req, ptn, batch_idx) } -> std::convertible_to<DataResult>;
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
   *   static constexpr StreamPartitioningStrategy PartitioningStrategy {
   *     StreamPartitioningStrategy::SubDivide
   *   };
   *
   *   struct DataStream : public StreamVariant<roles::Data> {};
   *   using StreamTypes = StreamSet<DataStream>;
   *
   *   enum class DataAccessPtn : std::uint8_t { };
   *   static constexpr std::size_t DataAccessPtnCount { 0 };
   *   struct StreamParameters {};
   *
   *   // CanAllocateStorage
   *   // ------------------
   *   using BrokerBufferRequirements = RequirementsList<>;
   *   static AllocationRequest<T> get_allocation_request(GenericStreamConfig<ImplementsFormatTraits>& cfg);
   *
   *   // HasDataRequest
   *   // --------------
   *   struct DataRequest { };
   *
   *   // HasStreamState
   *   // --------------
   *   struct FieldMetadata { };
   *
   *   // CanDiscoverMetadata
   *   // -------------------------------------------
   *   template <IOTraits IO, class StorageViewT>
   *   static IOStatus discover_metadata(Stream<IO, T>* streams,
   *                                     StorageViewT& storage,
   *                                     MetadataInventory<ImplementsFormatTraits>& inv);
   *
   *   // CanIndexStreams  [[ OPTIONAL ]]
   *   // ---------------
   *   template <IOTraits IO, class StorageViewT>
   *   static IOStatus index_stream(Stream<IO, T>* streams,
   *                                StorageViewT& storage,
   *                                StreamCatalog<ImplementsFormatTraits>& catalog,
   *                                IndexingCursor<ImplementsFormatTraits>& cursor,
   *                                const StreamParameters& cfg);
   *
   *   // CanFetchStreamData
   *   // ------------------
   *   template <IOTraits IO, class StorageViewT>
   *   static IOStatus fetch_step(Stream<IO, T>* streams,
   *                              StorageViewT& storage,
   *                              const StreamCatalog<ImplementsFormatTraits>& catalog,
   *                              FetchCursor<ImplementsFormatTraits>& cursor,
   *                              const GenericStreamConfig<ImplementsFormatTraits>& cfg,
   *                              StepIdxType step_idx,
   *                              DataAccessPtn ptn);
   *
   *   // CanResolveData && CanFillBuffer
   *   // -------------------------------
   *   static inline DataResult resolve_data(void* buf,
   *                                         const MetadataInventory<ImplementsFormatTraits>& inv,
   *                                         const DataRequest& req);
   *   static std::size_t get_payload_size(void* buf);
   *
   *   template <class StorageViewT>
   *   static DataResult get_data_in_buffer(StorageViewT& storage,
   *                                        const MetadataInventory<ImplementsFormatTraits>& inv,
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
    CanDiscoverMetadata<
      T,
      IO,
      StorageView<Storage<typename T::BrokerBufferRequirements, EPolicy>, EPolicy>> &&
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
  concept EventOffsetFormatTraits = FormatTraits<T, IO, StorageViewT> && HasEventOffset<T>;

  template <typename T, typename IO, class StorageViewT>
  concept TransitionOffsetFormatTraits = FormatTraits<T, IO, StorageViewT> && HasTransitionOffset<T>;


  template <typename T, typename IO, class StorageViewT>
  concept OffsetBasedFormatTraits =
    FormatTraits<T, IO, StorageViewT> && HasEventOffset<T> && HasTransitionOffset<T>;

  template <typename StreamVariant, typename FTraits, typename IO>
  SBIO_HD constexpr auto& get_stream(Stream<IO, FTraits>* streams) {
    constexpr std::size_t idx { FTraits::StreamTypes::template index_of<StreamVariant> };

    return streams[idx];
  }
} // namespace sbio

#endif // SBIO_FORMATS_FORMAT_TRAITS_HH
