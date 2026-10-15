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

#ifndef SBIO_FORMATS_XTC1_XTC1_TRAITS_HH
#define SBIO_FORMATS_XTC1_XTC1_TRAITS_HH

#include "sbio/core/request.hh"
#include "sbio/core/roles.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/stream.hh"
#include "sbio/export_macro.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/formats/xtc1/xtc1.hh"
#include "sbio/locators/path_pattern.hh"
#include "sbio/util/parameters.hh"
#include "sbio/util/string.hh"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  namespace literals {
    // No alg in XTC1
    // TODO: Where to put literals?? Want scoping by trait? This is defined already
    //       in xtc2_traits.hh
    // SBIO_HD constexpr auto operator ""_field(const char* str, std::size_t) {
    //   return NamedArg<"field"> { str };
    // }
  } // namespace literals

  struct SBIO_API XTC1Traits : public BaseTraits {
    using DefaultLocator = PathPatternLocator;

    // Sizeof dgram header
    // 16 bytes (Sequence) + 4 bytes (Env) + sizeof(Xtc)
    // 16 + 4 + 20 = 40 bytes
    static constexpr std::size_t HeaderSize { 40 };
    static constexpr std::uint16_t MaxRank { 10 };
    static constexpr std::uint16_t MaxNameSize { 256 };

    // Using method 2 - data from a detector is always written in its entirety to one
    // stream, but a different stream is selected each time and must be read in order
    // to reconstruct the time axis.
    static constexpr StreamPartitioningStrategy PartitioningStrategy {
      StreamPartitioningStrategy::Chronological
    };

     // For the types of streams/files
    struct SMD
      : public StreamVariant<roles::Metadata, roles::Index, roles::Data> {};
    struct BD
      : public StreamVariant<roles::Metadata, roles::Data> {};

    using StreamTypes = StreamSet<SMD, BD>;

    // NOTE: The EPICS data comes in the L1Accept in XTC1, Begin/EndCalibCycle are scans
    // TODO: These access patterns aren't accurate for EPICS in XTC1 - need update
    enum class DataAccessPtn : std::uint8_t {
      L1Accept = 0, // "Normal detectors"     - Look for data in L1Accept dgrams
      SlowUpdate,   // "EPICS Arch detectors" - Look for data in SlowUpdate dgrams
      BeginStep     // "Scan detectors"       - Look for data in BeginStep dgrams
    };
    static constexpr std::size_t DataAccessPtnCount { 3 };

    // The XTC1 transitions
    enum class StepKind : std::uint8_t {
      Unknown         =  0,
      Reset           =  1,
      Map             =  2,
      Unmap           =  3,
      Configure       =  4,
      Unconfigure     =  5,
      BeginRun        =  6,
      EndRun          =  7,
      BeginCalibCycle =  8,
      EndCalibCycle   =  9,
      Enable          = 10,
      Disable         = 11,
      L1Accept        = 12
    };

    static constexpr std::size_t NumStepKinds { static_cast<std::size_t>(StepKind::L1Accept) + 1 };

    // Events are spread over the streams; transitions are in every stream
    static constexpr StepKind ChronologicalKind { StepKind::L1Accept };

    // TODO: XTC1 schemas are currently host-only -- all requests must be resolved
    //       host side. Pure device-side policies are therefore not currently
    //       supported.
    static constexpr bool ResolvesOnDevice { false };

    using DataUnit = XTC1::Dgram;
    using StepIdxType = std::size_t; // Unit type for indexing and selecting data units ("events")
    static constexpr StepIdxType ExhaustedSentinel { static_cast<StepIdxType>(-1) }; // Indicator all units read

    struct StreamParameters {};

    /**
     * Specify all the buffers that are required to implement a reader of XTC1.
     * We need to hold:
     * 1. The Configure transition (kept as configuration fields are read from it)
     * 2. Datagrams for each access pattern
     * 3. The step index
     *
     * The execution policy will determine how to fulfill allocation and synchronization
     * of these requested buffers.
     */
    using BrokerBufferRequirements = RequirementsList<
      BufferDescriptor<roles::Metadata, 0, sizeof(XTC1::Dgram)>,  /* Configure transition */
      BufferDescriptor<roles::Data, 0, sizeof(XTC1::Dgram)>,      /* Buffer for events */
      BufferDescriptor<roles::Data, 1, sizeof(XTC1::Dgram)>,      /* Buffer for SlowUpdate (unused) */
      BufferDescriptor<roles::Data, 2, sizeof(XTC1::Dgram)>,      /* Buffer for BeginCalibCycle */
      BufferDescriptor<roles::Index, index_ids::Steps>,
      BufferDescriptor<roles::Metadata, index_ids::Scratch>
    >;

    // XTC1 data is looked up by a single `field` key
    using RequestSchema = sbio::NamedKeys<"field">;
    using DataRequest = sbio::DataRequest<RequestSchema>;

    // The full source name (e.g. CxiDs1.0:Jungfrau.0), whether or not an alias names
    // the group
    using GroupKeys = sbio::NamedKeys<"src">;
    // using GroupKeys = sbio::NamedKeys<"serial_number">;

    /**
     * How to find a field's Xtc, and its detector's configuration.
     */
    struct FieldMetadata {
      std::uint32_t config_offset { 0 }; ///< The detector's Xtc offset in the Configure dgram
      std::uint32_t event_type { 0 };    ///< The TypeId::Type in events, or 0 for configuration fields
      std::uint32_t src_log { 0 };       ///< Source of the detector's Xtc (logical)
      std::uint32_t src_phy { 0 };       ///< Source of the detector's Xtc (physical)
      std::uint32_t field_idx { 0 };     ///< Field index in the schema seen at Configure
    };

    SBIO_HD static inline std::size_t get_payload_size(void* buf) {
      return reinterpret_cast<XTC1::Dgram*>(buf)->xtc.sizeofPayload();
    }

    /**
     * Build the inventory from a Configure transition.
     *
     * @param[in] buffer The Configure datagram.
     * @param[out] inv The inventory to fill.
     * @param[in] offset Unused.
     */
    SBIO_HD static void discover_metadata(DataUnit* buffer,
                                          MetadataInventory<XTC1Traits>& inv,
                                          std::size_t offset);

     /**
     * Resolve a request in a datagram.
     *
     * @param[in] buffer The datagram holding the field. For configuration fields
     *            this is the Configure datagram.
     * @param[in] inv The inventory.
     * @param[in] req The request.
     * @param[in] cache Offsets of fields' Xtcs found in previous datagrams.
     * @param[in] config The Configure datagram (for configuration-dependent shapes).
     *            If nullptr, `buffer` is used.
     */
    SBIO_HD static DataResult resolve_data(void* buffer,
                                           const MetadataInventory<XTC1Traits>& inv,
                                           const DataRequest& req,
                                           PayloadOffsetCache& cache,
                                           const void* config);
    SBIO_HD static DataResult resolve_data(void* buffer,
                                           const MetadataInventory<XTC1Traits>& inv,
                                           const DataRequest& req,
                                           PayloadOffsetCache& cache) {
      return resolve_data(buffer, inv, req, cache, nullptr);
    }

    SBIO_HD static AllocationRequest<XTC1Traits> get_allocation_request(GenericStreamConfig<XTC1Traits>& cfg) {
      AllocationRequest<XTC1Traits> request;
      request.size_requests[0] = cfg.max_buffer_size;
      request.size_requests[1] = cfg.max_buffer_size * cfg.max_batch_size;
      request.size_requests[2] = cfg.max_buffer_size;
      request.size_requests[3] = cfg.max_buffer_size * cfg.max_batch_size;
      request.size_requests[4] =
        (cfg.index_batch_size + NumStepKinds) * sizeof(StepOffset<XTC1Traits>);
      // SMD L1Accepts are read whole to find their offsets in the BD file (see locate_bytes)
      request.size_requests[5] = cfg.index_batch_size * (sizeof(XTC1::Dgram) + 128);

      return request;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus discover_metadata(Stream<IO, XTC1Traits>* streams,
                                              StorageViewT& storage,
                                              MetadataInventory<XTC1Traits>& inv) {
      // Leave the CONFIGURE transition in the buffer and pull out all the necessary
      // fields from it
      auto* cfg_buf =
        storage.template acquire<roles::Metadata, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);

      IOStatus status = get_stream<SMD>(streams).read_unit_at(cfg_buf,
                                                              storage.template size<roles::Metadata, 0>(),
                                                              0).status;

      if (status == IOStatus::Success) {
        auto* dg = reinterpret_cast<XTC1::Dgram*>(cfg_buf);

        if (dg->service() == XTC1::TransitionId::Configure) {
          XTC1Traits::discover_metadata(dg, inv, 0);
        } else {
          status = IOStatus::HeaderReadError; // A run starts with Configure
        }
      }

      storage.template release<roles::Metadata, 0>(cfg_buf);
      return status;
    }

    SBIO_HD static constexpr StepKind kind_for_ptn(DataAccessPtn ptn) {
      switch (ptn) {
      case DataAccessPtn::BeginStep:  { return StepKind::BeginCalibCycle; }
      case DataAccessPtn::SlowUpdate: { return StepKind::Unknown; } // Never indexed
      default:                        { return StepKind::L1Accept; }
      }
    }

    SBIO_HD static std::optional<StepKind> kind_for_step(const DataUnit* step_data) {
      if (step_data == nullptr || step_data->xtc.extent < sizeof(XTC1::Xtc)) {
        return std::nullopt;
      }

      const auto service { static_cast<unsigned>(step_data->service()) };
      if (service == XTC1::TransitionId::Unknown || service > XTC1::TransitionId::L1Accept) {
        return std::nullopt;
      }

      return static_cast<StepKind>(service);
    }

    /**
     * A step's position in time, to merge the streams.
     */
    SBIO_HD static std::uint64_t order_key(const DataUnit* dg) {
      const auto& clock { dg->seq.clock() };
      return (static_cast<std::uint64_t>(clock.seconds()) << 32) | clock.nanoseconds();
    }

    SBIO_HD static std::size_t locate_bytes(const DataUnit* dg) {
      if (dg->service() == XTC1::TransitionId::L1Accept) {
        // The L1Accept BD offset information (OrigDgramOffsetV1) can be present anwyhere
        // inside the SMD's L1Accept so must be read whole
        return sizeof(XTC1::Dgram) - sizeof(XTC1::Xtc) + dg->xtc.extent;
      }

      return sizeof(XTC1::Dgram);
    }

    /**
     * Find the BD offset of an SMD L1Accept.
     *
     * @note A nullptr is returned if no DgramOffset is present - this occurs if
     *       reading a BD instead of SMD file.
     *
     * @param[in] The Xtc for the L1Accept
     * @returns The OrigDgramOffsetV1, or nullptr if the datagram has none.
     */
    SBIO_HD static const XTC1::SmlData::OrigDgramOffsetV1* find_smd_offset(const XTC1::Xtc* xtc);

    SBIO_HD static ByteRegion locate_step(const DataUnit* dg,
                                          StepKind kind,
                                          std::uint32_t scanned_stream,
                                          std::uint64_t scan_offset) {
      constexpr auto smd { static_cast<std::uint32_t>(StreamTypes::index_of<SMD>) };
      constexpr auto bd { static_cast<std::uint32_t>(StreamTypes::index_of<BD>) };

      if (kind == StepKind::L1Accept && scanned_stream == smd) {
        if (const auto* orig { find_smd_offset(&dg->xtc) }) {
          // Since alignment is only 4-byte, use copies to get fields out
          std::int64_t file_offset { 0 };
          std::uint32_t extent { 0 };
          const char* raw { reinterpret_cast<const char*>(orig) };
          std::memcpy(&file_offset, raw, sizeof(file_offset));
          std::memcpy(&extent, raw + sizeof(file_offset), sizeof(extent));

          return {
            static_cast<std::uint64_t>(file_offset),
            static_cast<std::uint64_t>(extent),
            bd
          };
        }
      }

      // The transition is always present (and complete) in any stream so can be
      // read in place. This is also true for L1Accepts that are read without using
      // offsets (the stream holds the data as scanned)
      return {
        scan_offset,
        sizeof(XTC1::Dgram) + static_cast<std::uint64_t>(dg->xtc.sizeofPayload()),
        scanned_stream
      };
    }

    template <class StorageViewT>
    SBIO_HD static DataResult get_data_in_buffer(StorageViewT& storage,
                                                 const MetadataInventory<XTC1Traits>& inv,
                                                 const DataRequest& req,
                                                 PayloadOffsetCache& cache,
                                                 DataAccessPtn ptn,
                                                 std::size_t batch_idx = 0) {
      const auto* entry { inv.lookup(req) };
      if (entry == nullptr) {
        return {};
      }

      void* cfg_buf {
        storage.template acquire<roles::Metadata, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace)
      };

      DataResult res {};
      if (entry->descriptor.format_meta.event_type == 0) {
        // Configuration fields are in the Configure transition
        res = XTC1Traits::resolve_data(cfg_buf, inv, req, cache, cfg_buf);
      } else {
        void* bd_buf { nullptr };
        auto acquire_ptn_buffer = [&](auto P) {
          bd_buf =
            storage.template acquire<
              roles::Data,
              decltype(P)::value,
              ViewMemTag
            >(AcquireIntent::CallerMemorySpace);
        };
        visit_ptn<XTC1Traits>(ptn, acquire_ptn_buffer);

        auto* dg { reinterpret_cast<XTC1::Dgram*>(bd_buf) };

        // A batch is read in one block, but as a result, it may contain intervening
        // transitions which then need to be skipped over
        if (batch_idx > 0) {
          const auto target { static_cast<unsigned>(kind_for_ptn(ptn)) };

          std::size_t matched_count { 0 };
          while (matched_count < batch_idx) {
            const std::size_t dgram_size { sizeof(XTC1::Dgram) + dg->xtc.sizeofPayload() };
            dg = reinterpret_cast<XTC1::Dgram*>(reinterpret_cast<char*>(dg) + dgram_size);

            if (static_cast<unsigned>(dg->service()) == target) {
              matched_count++;
            }
          }
        }

        res = XTC1Traits::resolve_data(dg, inv, req, cache, cfg_buf);
        auto release_ptn_buffer = [&](auto P) {
          res.data =
            storage.template release<roles::Data, decltype(P)::value>(bd_buf, res.data);
        };

        visit_ptn<XTC1Traits>(ptn, release_ptn_buffer);
      }

      storage.template release<roles::Metadata, 0>(cfg_buf);
      return res;
    }
  };
} // namespace sbio

#endif // SBIO_FORMATS_XTC1_XTC1_TRAITS_HH
