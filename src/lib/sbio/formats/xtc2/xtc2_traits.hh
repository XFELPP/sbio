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

#ifndef SBIO_FORMATS_XTC2_XTC2_TRAITS_HH
#define SBIO_FORMATS_XTC2_XTC2_TRAITS_HH

#include "sbio/core/request.hh"
#include "sbio/core/roles.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/stream.hh"
#include "sbio/core/sync.hh"
#include "sbio/export_macro.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/formats/xtc2/traversal.hh"
#include "sbio/formats/xtc2/xtc2.hh"
#include "sbio/locators/path_pattern.hh"
#include "sbio/util/parameters.hh"
#include "sbio/util/string.hh"

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

#include <cstddef>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace fs = std::filesystem;

namespace sbio {
  namespace literals {
    SBIO_HD constexpr auto operator ""_alg(const char* str, std::size_t) {
      return NamedArg<"alg"> { str };
    }

    SBIO_HD constexpr auto operator ""_field(const char* str, std::size_t) {
      return NamedArg<"field"> { str };
    }
  } // namespace literals

  struct SBIO_API XTC2Traits : public BaseTraits {
    using DefaultLocator = PathPatternLocator;

    // Sizeof dgram header
    // 4 bytes + 4 bytes + 4 bytes + sizeof(Xtc)
    // Xtc: 4 bytes (src) + 2 bytes (damage) + 2 (TypeId) + 4 bytes extent
    // Total: 12 + 12 = 24
    static constexpr std::size_t HeaderSize { 24 };
    static constexpr std::uint16_t MaxRank { 10 };
    static constexpr std::uint16_t MaxNameSize { 256 };

    // Using method 1 - data from a detector is sub-divided into portions, i.e.,
    // "segments", that are distributed across streams. To read the entire data at
    // any time point, you must coallesce the data from all streams.
    static constexpr StreamPartitioningStrategy PartitioningStrategy {
      StreamPartitioningStrategy::SubDivide
    };

    // For the types of streams/files
    struct SMD
      : public StreamVariant<roles::Metadata, roles::Index, roles::Data> {};
    struct BD
      : public StreamVariant<roles::Metadata, roles::Data> {};

    using StreamTypes = StreamSet<SMD, BD>;

    // Ways of looking up data inside of an XTC2 file for different detector kinds
    enum class DataAccessPtn : std::uint8_t {
      L1Accept = 0, // "Normal detectors"     - Look for data in L1Accept dgrams
      SlowUpdate,   // "EPICS Arch detectors" - Look for data in SlowUpdate dgrams
      BeginStep     // "Scan detectors"       - Look for data in BeginStep dgrams
    };
    static constexpr std::size_t DataAccessPtnCount { 3 };

    enum class StepKind : std::uint8_t {
      ClearReadout =  0,
      Reset        =  1,
      Configure    =  2,
      Unconfigure  =  3,
      BeginRun     =  4,
      EndRun       =  5,
      BeginStep    =  6,
      EndStep      =  7,
      Enable       =  8,
      Disable      =  9,
      SlowUpdate   = 10,
      Unused       = 11,
      L1Accept     = 12
    };

    static constexpr std::size_t NumStepKinds { static_cast<std::size_t>(StepKind::L1Accept) + 1 };

    using DataUnit = XTC2::Dgram;
    using StepIdxType = std::size_t; // Unit type for indexing and selecting data units ("events")
    static constexpr StepIdxType ExhaustedSentinel { static_cast<StepIdxType>(-1) }; // Indicator all units read

    struct StreamParameters {};

    /**
     * Specify all the buffers that are required to implement a reader of XTC2.
     * We need to hold:
     * 1. Transition datagrams
     * 2. Event datagrams
     * 3. Event offsets
     * 4. Transition offsets
     *
     * The execution policy will determine how to fulfill allocation and synchronization
     * of these requested buffers.
     */
    using BrokerBufferRequirements = RequirementsList<
      BufferDescriptor<roles::Metadata, 0, sizeof(XTC2::Dgram)>,  /* Buffer for transition */
      BufferDescriptor<roles::Data, 0, sizeof(XTC2::Dgram)>,      /* Buffer for events */
      BufferDescriptor<roles::Data, 1, sizeof(XTC2::Dgram)>,      /* Buffer for SlowUpdate */
      BufferDescriptor<roles::Data, 2, sizeof(XTC2::Dgram)>,      /* Buffer for scan */
      BufferDescriptor<roles::Index, index_ids::Steps>,
      BufferDescriptor<roles::Metadata, index_ids::Scratch>
    >;

    using RequestSchema = sbio::NamedKeys<"alg", "field">;
    using DataRequest = sbio::DataRequest<RequestSchema>;

    using GroupKeys = sbio::NamedKeys<"serial_number">;

    struct FieldMetadata {
      std::uint32_t names_id { 0 };
      std::uint32_t field_idx { 0 };
      XTC2::Name field_name;
    };

    SBIO_HD static inline std::size_t get_payload_size(void* buf) {
      return reinterpret_cast<XTC2::Dgram*>(buf)->xtc.sizeofPayload();
    }

    SBIO_HD static void discover_metadata(DataUnit* buffer,
                                          MetadataInventory<XTC2Traits>& inv,
                                          std::size_t offset);

    SBIO_HD static DataResult resolve_data(void* buffer,
                                           const MetadataInventory<XTC2Traits>& inv,
                                           const DataRequest& req,
                                           PayloadOffsetCache& cache);

    SBIO_HD static AllocationRequest<XTC2Traits> get_allocation_request(GenericStreamConfig<XTC2Traits>& cfg) {
      AllocationRequest<XTC2Traits> request;
      request.size_requests[0] = cfg.max_buffer_size;
      request.size_requests[1] = cfg.max_buffer_size * cfg.max_batch_size;
      request.size_requests[2] = cfg.max_buffer_size;
      request.size_requests[3] = cfg.max_buffer_size;
      request.size_requests[4] =
        (cfg.index_batch_size + NumStepKinds) * sizeof(StepOffset<XTC2Traits>);
      request.size_requests[5] = cfg.index_batch_size * (sizeof(XTC2::Dgram) + 80);

      return request;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus discover_metadata(Stream<IO, XTC2Traits>* streams,
                                              StorageViewT& storage,
                                              MetadataInventory<XTC2Traits>& inv) {
      auto* smd_buf =
        storage.template acquire<roles::Metadata, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);

      IOStatus status = get_stream<SMD>(streams).read_unit_at(smd_buf,
                                                              storage.template size<roles::Metadata>(),
                                                              0).status;

      if (status == IOStatus::Success) {
        auto* dg = reinterpret_cast<XTC2::Dgram*>(smd_buf);

        if (dg->service() == XTC2::TransitionId::Configure) {
          XTC2Traits::discover_metadata(dg, inv, 0);
        }
      }

      storage.template release<roles::Metadata, 0>(smd_buf);
      return status;
    }

    SBIO_HD static constexpr StepKind kind_for_ptn(DataAccessPtn ptn) {
      switch (ptn) {
      case DataAccessPtn::SlowUpdate: { return StepKind::SlowUpdate; }
      case DataAccessPtn::BeginStep:  { return StepKind::BeginStep; }
      default:                        { return StepKind::L1Accept; }
      }
    }

    SBIO_HD static std::optional<StepKind> kind_for_step(const DataUnit* step_data) {
      if (step_data != nullptr) {
        switch (step_data->service()) {
        case XTC2::TransitionId::ClearReadout: { return StepKind::ClearReadout; }
        case XTC2::TransitionId::Reset:        { return StepKind::Reset; }
        case XTC2::TransitionId::Configure:    { return StepKind::Configure; }
        case XTC2::TransitionId::Unconfigure:  { return StepKind::Unconfigure; }
        case XTC2::TransitionId::BeginRun:     { return StepKind::BeginRun; }
        case XTC2::TransitionId::EndRun:       { return StepKind::EndRun; }
        case XTC2::TransitionId::BeginStep:    { return StepKind::BeginStep; }
        case XTC2::TransitionId::EndStep:      { return StepKind::EndStep; }
        case XTC2::TransitionId::Enable:       { return StepKind::Enable; }
        case XTC2::TransitionId::Disable:      { return StepKind::Disable; }
        case XTC2::TransitionId::SlowUpdate:   { return StepKind::SlowUpdate; }
        case XTC2::TransitionId::Unused_11:    { return StepKind::Unused; }
        case XTC2::TransitionId::L1Accept:     { return StepKind::L1Accept; }
        default:                               { return std::nullopt; }
        }
      }

      return std::nullopt;
    }

    SBIO_HD static std::size_t locate_bytes(const DataUnit* dg) {
      if (dg->service() == XTC2::TransitionId::L1Accept) {
        // L1Accept is 48 bytes into the SMD payload
        // And since offset and size are both 8 bytes, size is 1 unit after
        std::size_t offset_in_payload { 48 };
        return sizeof(XTC2::Dgram) - sizeof(XTC2::Xtc) + offset_in_payload + 2 * sizeof(std::uint64_t);
      }
      return sizeof(XTC2::Dgram);
    }

    SBIO_HD static ByteRegion locate_step(const DataUnit* dg,
                                          StepKind kind,
                                          std::uint32_t scanned_stream,
                                          std::uint64_t scan_offset) {
      constexpr auto smd { static_cast<std::uint32_t>(StreamTypes::index_of<SMD>) };
      constexpr auto bd { static_cast<std::uint32_t>(StreamTypes::index_of<BD>) };

      if (kind == StepKind::L1Accept && scanned_stream == smd) {
        // L1Accept is 48 bytes into the SMD payload
        // And since offset and size are both 8 bytes, size is 1 unit after
        std::size_t offset_in_payload { 48 };
        std::uint64_t offset_size[2];
        std::memcpy(offset_size,
                    reinterpret_cast<const char*>(&dg->xtc) + offset_in_payload,
                    sizeof(offset_size));
        return { offset_size[0], offset_size[1], bd };
      }

      // Transitions are complete in whichever stream is being scanned: read them in place.
      return {
        scan_offset,
        sizeof(XTC2::Dgram) + static_cast<std::uint64_t>(dg->xtc.sizeofPayload()),
        scanned_stream
      };
    }

    template <class StorageViewT>
    SBIO_HD static DataResult get_data_in_buffer(StorageViewT& storage,
                                                 const MetadataInventory<XTC2Traits>& inv,
                                                 const DataRequest& req,
                                                 PayloadOffsetCache& cache,
                                                 DataAccessPtn ptn,
                                                 std::size_t batch_idx = 0) {
      DataRequest corrected_req { req };
      XTC2::TransitionId target_service { XTC2::TransitionId::L1Accept };
      void* bd_buf { nullptr };
      XTC2::Dgram* dg { nullptr };

      auto acquire_ptn_buffer = [&](auto P) {
        bd_buf =
          storage.template acquire<
            roles::Data,
            decltype(P)::value,
            ncarray::HostTag
          >(AcquireIntent::CallerMemorySpace);
      };
      visit_ptn<XTC2Traits>(ptn, acquire_ptn_buffer);
      dg = reinterpret_cast<XTC2::Dgram*>(bd_buf);

      if (ptn == DataAccessPtn::L1Accept) {
        target_service = XTC2::TransitionId::L1Accept;
      } else {
        if (ptn == DataAccessPtn::SlowUpdate) {
          target_service = XTC2::TransitionId::SlowUpdate;

          // Semantic Mapping: Requested name is actually a PV field in "epics"
          const char* epics_det_name { "epics" };
          corrected_req.group_name = epics_det_name;

          corrected_req.set<"field">(req.group_name.c_str());
        } else if (ptn == DataAccessPtn::BeginStep) {
          target_service = XTC2::TransitionId::BeginStep;

          // `scan` should be like a "normal" detector. However, it must be read from
          // the BeginStep transition buffers.
          // Those buffers will have:
          // - `step_value`     : INT64
          // - `step_docstring` : CHARSTR, optional (but usually present)
          // - `scan_var_xxx`   : ANY (the actual scanned variable - may have multiple)
          // We also allow for people to pass `scan_var_namexxx` as the name directly
          // So in the case we have Scan type access, but the name is not "scan" we must
          // do a rewrite
          if (corrected_req.group_name != "scan") {
            corrected_req.group_name = "scan";
            corrected_req.set<"field">(req.group_name.c_str());
          }
        }
      }


      if (batch_idx > 0) {
        std::size_t matched_count { 0 };
        while (matched_count < batch_idx) {
          // When reading batches, to simplify we allow reading a massive chunk that
          // may include intervening transitions.
          if (dg->service() == target_service) {
            matched_count++;
          }
          std::size_t dgram_size { sizeof(XTC2::Dgram) + dg->xtc.sizeofPayload() };
          dg = reinterpret_cast<XTC2::Dgram*>(reinterpret_cast<char*>(dg) + dgram_size);
        }
      }

      DataResult res = XTC2Traits::resolve_data(dg, inv, corrected_req, cache);

      auto release_ptn_buffer = [&](auto P) {
        res.data =
          storage.template release<roles::Data, decltype(P)::value>(bd_buf, res.data);
      };
      visit_ptn<XTC2Traits>(ptn, release_ptn_buffer);

      return res;
    }

    /*
    template <class StorageViewT>
    SBIO_HD static auto current_buffer(StorageViewT& storage,
                                       const DiscoveryState& state) {
      if (state.last_accessed_ptn == DataAccessPtn::L1Accept) {
        auto* buf = storage.template acquire<roles::Data, 0, ncarray::HostTag>();
        storage.template release<roles::Data, 0>(buf);

        return buf;
      } else {
        auto* buf = storage.template acquire<roles::Metadata, 0, ncarray::HostTag>();
        storage.template release<roles::Metadata, 0>(buf);

        return buf;
      }
    }
    */
  };
} // namespace sbio

#endif // SBIO_FORMATS_XTC2_XTC2_TRAITS_HH
