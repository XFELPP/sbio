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

  struct BaseTraits {
    static constexpr std::size_t HeaderSize { 0 };
    static constexpr std::uint16_t MaxRank { 10 };
    static constexpr std::uint16_t MaxNameSize { 256 };
    // How data from a "logical unit" is written different streams over time.
    static constexpr StreamPartitioningStrategy PartitioningStrategy {
      StreamPartitioningStrategy::SubDivide
    };

    // For the types of streams/files
    enum Roles { };
    static constexpr std::size_t RoleCount { 0 };

    enum class DataAccessPtn : std::uint8_t { };
    static constexpr std::size_t DataAccessPtnCount { 0 };

    /// Need to encode types of buffers somehow

    using DataUnit = void;
    using StepIdxType = std::size_t; // Unit type for indexing and selecting data units ("events")
    static constexpr StepIdxType ExhaustedSentinel { static_cast<StepIdxType>(-1) }; // Indicator all units read

    using BrokerBufferRequirements = TypeList<>;
    using GroupBufferRequirements = TypeList<>;

    struct StreamParameters {};

    struct DiscoveryState {};
    struct DataRequest {};
    struct MetadataInventory {};
    // struct EventOffset {};
    // struct TransitionOffset {};

    template <class DataBrokerType, class SegmentRef>
    SBIO_HD inline static std::size_t find_detector_segments(const MetadataInventory& inv,
                                                              const char* name,
                                                              SegmentRef* ref_out,
                                                              std::size_t max_out,
                                                              DataBrokerType* broker,
                                                              DataAccessPtn ptn) {
      return 0; // Default to no segments found
    }

    struct DataResult {};
    SBIO_HD static std::size_t get_payload_size(void* buf) { return 0; }
    SBIO_HD static inline DataResult resolve_data(void* buf,
                                                   const MetadataInventory& inv,
                                                   const DataRequest& req) {
      return {};
    }

    SBIO_HD static inline void discover_metadata(DataUnit* buf,
                                                 MetadataInventory& inv,
                                                 std::size_t offset) {}

    template <IOTraits IO>
    SBIO_HD static IOStatus open_streams(Stream<IO, BaseTraits>* streams,
                                         const StreamParameters& cfg) {
      return IOStatus::Success;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus discover_metadata(Stream<IO, BaseTraits>* streams,
                                              StorageViewT& storage,
                                              MetadataInventory& inv) {
      return IOStatus::Success;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus index_stream(Stream<IO, BaseTraits>* streams,
                                         StorageViewT& storage,
                                         DiscoveryState& stream_state,
                                         const StreamParameters& cfg) {
      return IOStatus::Success;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_step(Stream<IO, BaseTraits>* streams,
                                       StorageViewT& storage,
                                       DiscoveryState& stream_state,
                                       const StreamParameters& cfg,
                                       StepIdxType step_idx,
                                       DataAccessPtn ptn) {
      return IOStatus::Success;
    }

    template <class StorageViewT>
    SBIO_HD static DataResult get_data_in_buffer(StorageViewT& storage,
                                                 const MetadataInventory& inv,
                                                 const DataRequest& req,
                                                 DataAccessPtn ptn) {
      return DataResult{};
    }

    SBIO_HD static auto sync_vars(DiscoveryState& state) {}

    template <class StorageViewT>
    SBIO_HD static auto capacity(const StorageViewT& storage,
                                 const DiscoveryState& state) {}

    template <class StorageViewT>
    SBIO_HD static auto current_buffer(StorageViewT& storage,
                                       const DiscoveryState& state) {}
  };

  template <typename T>
  concept HasBoundedDataDimensions = requires {
    { T::MaxRank } -> std::convertible_to<std::uint16_t>;
    { T::MaxNameSize } -> std::convertible_to<std::uint16_t>;
    { T::HeaderSize } -> std::convertible_to<std::size_t>;
  };

  template <typename T>
  concept HasStreamParameters = requires {
    typename T::StreamParameters;
    requires std::is_trivially_copyable_v<typename T::StreamParameters>;
    requires !std::is_void_v<std::remove_cvref_t<typename T::StreamParameters>>;
  };

  template <typename T>
  concept HasDataRequest = requires {
    typename T::DataRequest;
    requires std::is_trivially_copyable_v<typename T::DataRequest>;
    requires !std::is_void_v<std::remove_cvref_t<typename T::DataRequest>>;
  };

  template <typename T>
  concept HasEventOffset = requires {
    typename T::EventOffset;
    requires std::is_trivially_copyable_v<typename T::EventOffset>;
    // TODO: Why fail?
    //requires !std::is_void_v<std::remove_cvref_t<typename T::EventOffset>;
  };

  template <typename T>
  concept HasTransitionOffset = requires {
    typename T::TransitionOffset;
    requires std::is_trivially_copyable_v<typename T::TransitionOffset>;
  };

  template <typename T>
  concept HasResolveData = requires(void* buf,
                                    const typename T::MetadataInventory& inv,
                                    const typename T::DataRequest& req) {
    { T::resolve_data(buf, inv, req) } -> std::same_as<typename T::DataResult>;
  };

  template <typename T, typename IO>
  concept CanOpenStreams = requires(Stream<IO, T>* streams,
                                    const typename T::StreamParameters& cfg) {
    { T::open_streams(streams, cfg) } -> std::convertible_to<IOStatus>;
  };

  template <typename T>
  concept HasMetadataDiscovery = requires(typename T::DataUnit* buf,
                                          typename T::MetadataInventory& inv,
                                          std::size_t offset) {
    { T::discover_metadata(buf, inv, offset) } -> std::same_as<void>;
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

  template <typename T, typename IO, class StorageViewT>
  concept CanFillBuffer = requires(StorageViewT& storage,
                                   const typename T::MetadataInventory& inv,
                                   const typename T::DataRequest& req,
                                   typename T::DataAccessPtn ptn) {
    { T::get_data_in_buffer(storage, inv, req, ptn) } -> std::convertible_to<typename T::DataResult>;
  };

  template <typename T>
  concept HasStateSynch = requires(typename T::DiscoveryState& state) {
    { T::sync_vars(state) };
  };

  template <typename T, typename IO, typename EPolicy>
  concept FormatTraits =
    HasBoundedDataDimensions<T>                                                     &&
    HasStreamParameters<T>                                                          &&
    HasDataRequest<T>                                                               &&
    HasResolveData<T>                                                               &&
    CanOpenStreams<T, IO>                                                           &&
    CanFetchStreamData<
      T,
      IO,
      StorageView<Storage<typename T::BrokerBufferRequirements, EPolicy>, EPolicy>> &&
    CanFillBuffer<
      T,
      IO,
      StorageView<Storage<typename T::BrokerBufferRequirements, EPolicy>, EPolicy>> &&
    HasMetadataDiscovery<T>;

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
