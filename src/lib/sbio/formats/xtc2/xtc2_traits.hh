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
    struct DataAccessPtn {
      // Main access patterns
      enum type : std::uint8_t {
        L1Accept   = 0, // "Normal detectors"     - Look for data in L1Accept dgrams
        SlowUpdate = 1, // "EPICS Arch detectors" - Look for data in SlowUpdate dgrams
        BeginStep  = 2  // "Scan detectors"       - Look for data in BeginStep dgrams
      };

      // "Sub kind" filters
      enum sub_kind : std::uint8_t {
        ROG0 = 0,
        ROG1 = 1,
        ROG2 = 2,
        ROG3 = 3,
        ROG4 = 4,
        ROG5 = 5,
        ROG6 = 6,
        ROG7 = 7
      };

      static constexpr std::size_t Count { 3 };        ///< Number of main patterns
      static constexpr std::size_t SubKindCount { 8 }; ///< Number of filterable unique types

      DataAccessPtn() = default;
      DataAccessPtn(type ptn)
        : m_ptn(ptn)
      {}
      DataAccessPtn(std::uint8_t ptn)
        : DataAccessPtn(static_cast<type>(ptn))
      {}

      operator std::size_t() const { return static_cast<std::size_t>(m_ptn); }

      template <typename... ROG>
      requires ( std::is_same_v<ROG, sub_kind> && ... )
      void set_filter(ROG... rog) {
        ( ( m_filter |= (1 << rog) ), ... );
      }

      void set_filter(std::size_t filter) { m_filter = filter; }

      std::size_t filter() const { return m_filter; }

      bool same_subkind(DataAccessPtn other) const {
        // For XTC2's purposes, only care that the readout group was present, not all
        // readout groups. So return true if so. If other has more than one ROG,
        // they should all be present. So order doesn't matter, the StepKind with fewer
        // ROGs is used. I.e., all ROGs should be present for the smallest quantity.
        auto rogs { filter() };
        auto orogs { other.filter() };

        return m_ptn == other.m_ptn &&
          (rogs & orogs) == (rogs < orogs ? rogs : orogs);
      }

    private:
      type m_ptn { L1Accept };
      std::size_t m_filter { 0 };
    };
    static constexpr std::size_t DataAccessPtnCount { DataAccessPtn::Count };

    enum class DgramKind : std::uint8_t {
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

#pragma pack(push, 2)
    struct StepKind {
      static constexpr std::size_t NumSubKinds { 8 }; // UNIQUE kinds. Combos don't count.

      StepKind() = default;

      StepKind(DgramKind kind)
        : m_rogs(0)
        , m_dgram_kind(kind)
      {}

      StepKind(std::uint16_t rogs, DgramKind kind)
        : m_rogs(rogs)
        , m_dgram_kind(kind)
      {}

      operator std::size_t() const { return static_cast<std::size_t>(m_dgram_kind); }

      std::size_t subkind() const { return static_cast<std::size_t>(m_rogs); }

      bool same_subkind(StepKind other) const {
        // For XTC2's purposes, only care that the readout group was present, not all
        // readout groups. So return true if so. If other has more than one ROG,
        // they should all be present. So order doesn't matter, the StepKind with fewer
        // ROGs is used. I.e., all ROGs should be present for the smallest quantity.
        auto rogs { subkind() };
        auto orogs { other.subkind() };

        return m_dgram_kind == other.m_dgram_kind &&
          (rogs & orogs) == ( rogs < orogs ? rogs : orogs);
      }

    private:
      std::uint16_t m_rogs { 0 };
      DgramKind m_dgram_kind { DgramKind::L1Accept };
    };
#pragma pack(pop)

    // These are top-level "Main Kinds"
    static constexpr std::size_t NumStepKinds { static_cast<std::size_t>(DgramKind::L1Accept) + 1 };

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

      // When re-indexing, we can hold over previous entries for the various alignemnt
      // to pattern procedures. So we have to add space for a number of extra rows
      // 1 extra row for each "main kind", and then for each unique sub kind (ROG for XTC2).
      const auto extra_rows { NumStepKinds * (StepKind::NumSubKinds + 1) };
      request.size_requests[4] =
        (cfg.index_batch_size + extra_rows) * sizeof(StepOffset<XTC2Traits>);
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

    /**
     * Check whether a new BD chunk is begun from this datagram.
     *
     * For XTC2 the order of transitions to indicate chunk boundaries is:
     * 1. DISABLE
     * 2. ENDSTEP
     * 3. BEGINSTEP
     * 4. ENABLE
     *     |- New ChunkInfo with next `chunkid` and `filename` pointer
     *
     * @param[in] dg The current datagram
     * @param[in] inv The metadata inventory.
     * @returns Whether a new BD chunk file is beginning.
     */
    SBIO_HD static bool starts_chunk(const DataUnit* dg,
                                     const MetadataInventory<XTC2Traits>& inv) {
      if (dg->service() != XTC2::TransitionId::Enable) {
        // Anything not an ENABLE cannot be a boundary for chunks
        return false;
      }

      // NOTE: To consider, in this case, could use the chunkid here against the
      //       chain_id in the general infrastructure. Something to think on...

      // ChunkInfo detector and alg def are:
      // - chunkinfo det
      // - chunkinfo alg (version 0.0.1)
      // - Has chunkid
      //   and filename
      using namespace sbio::literals;
      DataRequest req("chunkinfo", "chunkinfo", "chunkinfo"_alg, "chunkid"_field);
      PayloadOffsetCache cache {};
      return resolve_data(const_cast<DataUnit*>(dg), inv, req, cache).data != nullptr;
    }

    SBIO_HD static constexpr StepKind kind_for_ptn(DataAccessPtn ptn) {
      switch (ptn) {
      case DataAccessPtn::SlowUpdate: {
        return StepKind {
          static_cast<std::uint16_t>(ptn.filter()),
          DgramKind::SlowUpdate
        };
      }
      case DataAccessPtn::BeginStep: {
        return StepKind {
          static_cast<std::uint16_t>(ptn.filter()),
          DgramKind::BeginStep
        };
      }
      default: {
        return StepKind {
          static_cast<std::uint16_t>(ptn.filter()),
          DgramKind::L1Accept
        };
      }
      }
    }

    SBIO_HD static std::optional<StepKind> kind_for_step(const DataUnit* step_data) {
      if (step_data != nullptr) {
        switch (step_data->service()) {
        case XTC2::TransitionId::ClearReadout: {
          return StepKind { step_data->readoutGroups(), DgramKind::ClearReadout };
        }
        case XTC2::TransitionId::Reset: {
          return StepKind { step_data->readoutGroups(), DgramKind::Reset };
        }
        case XTC2::TransitionId::Configure: {
          return StepKind { step_data->readoutGroups(), DgramKind::Configure };
        }
        case XTC2::TransitionId::Unconfigure: {
          return StepKind { step_data->readoutGroups(), DgramKind::Unconfigure };
        }
        case XTC2::TransitionId::BeginRun: {
          return StepKind { step_data->readoutGroups(), DgramKind::BeginRun };
        }
        case XTC2::TransitionId::EndRun: {
          return StepKind { step_data->readoutGroups(), DgramKind::EndRun };
        }
        case XTC2::TransitionId::BeginStep: {
          return StepKind { step_data->readoutGroups(), DgramKind::BeginStep };
        }
        case XTC2::TransitionId::EndStep: {
          return StepKind { step_data->readoutGroups(), DgramKind::EndStep };
        }
        case XTC2::TransitionId::Enable: {
          return StepKind { step_data->readoutGroups(), DgramKind::Enable };
        }
        case XTC2::TransitionId::Disable: {
          return StepKind { step_data->readoutGroups(), DgramKind::Disable };
        }
        case XTC2::TransitionId::SlowUpdate: {
          return StepKind { step_data->readoutGroups(), DgramKind::SlowUpdate };
        }
        case XTC2::TransitionId::Unused_11: {
          return StepKind { step_data->readoutGroups(), DgramKind::Unused };
        }
        case XTC2::TransitionId::L1Accept: {
          return StepKind { step_data->readoutGroups(), DgramKind::L1Accept };
        }
        default: {
          return std::nullopt;
        }
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

      if (kind == StepKind { DgramKind::L1Accept } && scanned_stream == smd) {
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
            ViewMemTag
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
        // The batch starts at a datagram of the target kind. Step past it, and
        // `batch_idx` more of the target kind.
        std::size_t matched_count { 0 };
        while (matched_count < batch_idx) {
          // When reading batches, to simplify we allow reading a massive chunk that
          // may include intervening transitions.
          std::size_t dgram_size { sizeof(XTC2::Dgram) + dg->xtc.sizeofPayload() };
          dg = reinterpret_cast<XTC2::Dgram*>(reinterpret_cast<char*>(dg) + dgram_size);
          if (dg->service() == target_service) {
            matched_count++;
          }
        }
      }

      DataResult res = XTC2Traits::resolve_data(dg, inv, corrected_req, cache);
      if (res.data != nullptr) {
        cache.common_subkind() &= dg->readoutGroups();
      }

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
