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

    using DataUnit = XTC2::Dgram;
    using StepIdxType = std::size_t; // Unit type for indexing and selecting data units ("events")
    static constexpr StepIdxType ExhaustedSentinel { static_cast<StepIdxType>(-1) }; // Indicator all units read

    struct StreamParameters {};

    struct EventOffset {
      std::uint64_t offset;
      std::uint64_t size;
    };

    struct TransitionOffset {
      std::uint64_t offset;
      std::uint64_t size;
      std::int64_t previous_l1_index;
      XTC2::TransitionId transition_id;
    };

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
      BufferDescriptor<roles::Metadata, 1, sizeof(XTC2::Dgram)>,  /* Scratch buffer for SMD */
      BufferDescriptor<roles::Data, 0, sizeof(XTC2::Dgram)>,      /* Buffer for events */
      BufferDescriptor<roles::Index, 0, sizeof(EventOffset)>,     /* EventOffsets buffer */
      BufferDescriptor<roles::Index, 1, sizeof(TransitionOffset)> /* TransitionOffsets buffer */
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
                                           const DataRequest& req);

    /**
     * Traverse a datagram to populate transition and event offsets.
     * The datagram should come from a .smd.xtc2 file. This should be called in a loop
     * if you want to load many offsets.
     * @param[in] dg The datagram to traverse
     * @param[in] state A reference to a state tracking variable to maintain a history
     *            of offsets and indices seen.
     * @param[in] beggining_offset A starting point.
     * @param[in] l1_offsets_buf The L1Accept offsets buffer - the state will tell us
     *            where to index into this buffer to record the new offset from dg
     * @param[in] transition_offsets_buf The transition offsets buffer - the state will
     *            tell us where to index into this buffer to record the new offset from dg
     * @param[in] access_ofsfet The access offset to be incremented.
     */
    SBIO_HD static std::size_t populate_offsets(DataUnit* dg,
                                                IndexingCursor<XTC2Traits>& cursor,
                                                StreamCatalog<XTC2Traits>& catalog,
                                                std::size_t events_per_read,
                                                std::size_t beginning_offset,
                                                EventOffset* l1_offsets_buf,
                                                TransitionOffset* transition_offsets_buf,
                                                std::size_t access_offset = 0);

    SBIO_HD static AllocationRequest<XTC2Traits> get_allocation_request(GenericStreamConfig<XTC2Traits>& cfg) {
      AllocationRequest<XTC2Traits> request;
      request.size_requests[0] = cfg.max_buffer_size * cfg.max_batch_size; // Transition buf
      request.size_requests[1] = // Scratch buffer for reading ahead in smd file
        cfg.index_batch_size * (sizeof(XTC2::Dgram) + 80);
      request.size_requests[2] = cfg.max_buffer_size * cfg.max_batch_size; // Event buf
      request.size_requests[3] = cfg.index_batch_size * sizeof(XTC2Traits::EventOffset);
      request.size_requests[4] =
        cfg.index_batch_size * sizeof(XTC2Traits::TransitionOffset);

      return request;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus discover_metadata(Stream<IO, XTC2Traits>* streams,
                                              StorageViewT& storage,
                                              MetadataInventory<XTC2Traits>& inv) {
      auto* smd_buf =
        storage.template acquire<roles::Metadata, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);

      IOStatus status = get_stream<SMD>(streams).read_one(smd_buf,
                                                          storage.template size<roles::Metadata>());

      if (status == IOStatus::Success) {
        auto* dg = reinterpret_cast<XTC2::Dgram*>(smd_buf);

        if (dg->service() == XTC2::TransitionId::Configure) {
          XTC2Traits::discover_metadata(dg, inv, 0);
        }
      }

      storage.template release<roles::Metadata, 0>(smd_buf);
      return status;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus index_stream(Stream<IO, XTC2Traits>* streams,
                                         StorageViewT& storage,
                                         StreamCatalog<XTC2Traits>& catalog,
                                         IndexingCursor<XTC2Traits>& cursor,
                                         const GenericStreamConfig<XTC2Traits>& cfg) {
      auto events_per_read { cfg.index_batch_size };
      std::size_t read_size { (sizeof(XTC2::Dgram) + 80) * events_per_read };
      std::size_t missing_chunk { 0 };
      std::size_t smd_stream_idx { 0 }; // SMD is stream variant 0
      if (cursor.stream_offset[smd_stream_idx]) {
        std::size_t last_bytes_read = get_stream<SMD>(streams).read_count();
        missing_chunk = last_bytes_read - cursor.stream_offset[smd_stream_idx];
      }

      auto* smd_buf =
        storage.template acquire<roles::Metadata, 1, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
      IOStatus status = get_stream<SMD>(streams).read_batch(smd_buf, read_size, missing_chunk);

      if (status == IOStatus::ZeroBytesRead) {
        catalog.num_steps[0] = 0; // L1Accept count
        catalog.num_steps[1] = 0; // Transition count
      } else if (status == IOStatus::Success) {
        auto* l1_offsets_buf =
          storage.template acquire<roles::Index, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        auto* l1_offsets { reinterpret_cast<EventOffset*>(l1_offsets_buf) };

        auto* transition_offsets_buf =
          storage.template acquire<roles::Index, 1, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        auto* transition_offsets { reinterpret_cast<TransitionOffset*>(transition_offsets_buf) };

        std::size_t bytes_read = get_stream<SMD>(streams).read_count();
        cursor.stream_offset[smd_stream_idx] = 0;

        std::size_t n_events { 0 };
        std::size_t n_transitions { 0 };
        while (cursor.stream_offset[smd_stream_idx] < bytes_read) {
          if (n_events >= events_per_read) {
            status = IOStatus::AllRequestedRead;
            break;
          }

          if (cursor.stream_offset[smd_stream_idx] + sizeof(XTC2::Dgram) > bytes_read) {
            status = IOStatus::TruncatedRead;
            break;
          }

          auto* dg =
            reinterpret_cast<XTC2::Dgram*>(reinterpret_cast<char*>(smd_buf) + cursor.stream_offset[smd_stream_idx]);
          cursor.stream_offset[smd_stream_idx] += populate_offsets(dg,
                                                                   cursor,
                                                                   catalog,
                                                                   events_per_read,
                                                                   0,
                                                                   l1_offsets,
                                                                   transition_offsets,
                                                                   cursor.stream_offset[smd_stream_idx]);
          if (dg->service() == XTC2::TransitionId::L1Accept) {
            n_events++;
          } else {
            n_transitions++;
          }
        }

        catalog.num_steps[0] = n_events;       // L1Accept
        catalog.num_steps[1] = n_transitions;  // Transitions
        storage.template release<roles::Index, 0>(l1_offsets);
        storage.template release<roles::Index, 1>(transition_offsets);
      }

      storage.template release<roles::Metadata, 1>(smd_buf);
      return status;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_step(Stream<IO, XTC2Traits>* streams,
                                       StorageViewT& storage,
                                       const StreamCatalog<XTC2Traits>& catalog,
                                       FetchCursor<XTC2Traits>& cursor,
                                       const GenericStreamConfig<XTC2Traits>& cfg,
                                       StepIdxType step_idx,
                                       DataAccessPtn ptn) {
      cursor.last_access_ptn = ptn;
      std::size_t ptn_idx { static_cast<std::size_t>(ptn) };
      if (ptn == DataAccessPtn::L1Accept) {
        std::size_t adjusted_index { step_idx % cfg.index_batch_size };
        if (adjusted_index >= catalog.num_steps[ptn_idx]) { // num_steps[0]
          return IOStatus::AllRequestedRead;
        }
        auto* l1_offsets_buf =
          storage.template acquire<roles::Index, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        auto* l1_offsets { reinterpret_cast<EventOffset*>(l1_offsets_buf) };

        const auto& offset { l1_offsets[adjusted_index] };
        auto* bd_buf { storage.template acquire<roles::Data, 0, ncarray::HostTag>() };
        IOStatus status = get_stream<BD>(streams).read_at(bd_buf, offset.offset, offset.size);
        if (status == IOStatus::Success) {
          status = (get_stream<BD>(streams).read_count() == 0) ? IOStatus::ZeroBytesRead : IOStatus::Success;
        }

        storage.template release<roles::Index, 0>(l1_offsets);
        storage.template release<roles::Data, 0>(bd_buf);

        return status;
      } else {
        // Non-L1 transition fetch using cursor.offset_index[1]
        std::size_t trans_ptn_idx { 1 };
        XTC2::TransitionId transition_id = (ptn == DataAccessPtn::SlowUpdate)
          ? XTC2::TransitionId::SlowUpdate
          : XTC2::TransitionId::BeginStep;

        std::size_t adjusted_index { step_idx % cfg.index_batch_size };
        if (adjusted_index >= catalog.num_steps[0]) {
          return IOStatus::AllRequestedRead;
        }
        auto* l1_offsets_buf =
          storage.template acquire<roles::Index, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        auto* l1_offsets { reinterpret_cast<EventOffset*>(l1_offsets_buf) };

        auto* transition_offsets_buf =
          storage.template acquire<roles::Index, 1, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        auto* transition_offsets { reinterpret_cast<TransitionOffset*>(transition_offsets_buf) };
        auto& curr_trans_idx { cursor.offset_index[trans_ptn_idx] }; // Access pattern 1 offset index
        auto& offset { transition_offsets[curr_trans_idx] };

        if (offset.previous_l1_index > static_cast<std::int64_t>(step_idx)) {
          curr_trans_idx = 0;
          offset = transition_offsets[0];
        }

        while (offset.transition_id != transition_id ||
               offset.previous_l1_index <= static_cast<std::int64_t>(step_idx)) {
          curr_trans_idx++;

          if (curr_trans_idx >= catalog.num_steps[trans_ptn_idx]) {
            storage.template release<roles::Index, 0>(l1_offsets);
            storage.template release<roles::Index, 1>(transition_offsets);

            return IOStatus::AllRequestedRead;
          }

          auto& next_trans_off { transition_offsets[curr_trans_idx] };
          if (next_trans_off.previous_l1_index > static_cast<std::int64_t>(step_idx)) {
            break;
          }

          if (next_trans_off.transition_id == transition_id) {
            offset = transition_offsets[curr_trans_idx];
          }
        }

        std::int64_t prev_l1_index { offset.previous_l1_index };
        std::size_t read_size { 0 };
        std::size_t file_offset { 0 };
        if (prev_l1_index <= static_cast<std::int64_t>(step_idx) &&
            curr_trans_idx < catalog.num_steps[trans_ptn_idx]) {
          read_size = offset.size;

          if (prev_l1_index == -1) {
            auto& l1_offset { l1_offsets[0] };
            std::size_t total_offset_from_l1 { read_size };
            std::size_t transition_index { curr_trans_idx };

            auto& next_trans_off { transition_offsets[transition_index] };
            while (next_trans_off.previous_l1_index == -1) {
              total_offset_from_l1 += next_trans_off.size;
              transition_index++;
              next_trans_off = transition_offsets[transition_index];
            }

            file_offset = l1_offset.offset - total_offset_from_l1;
          } else {
            file_offset = offset.offset;
          }
        }

        auto* bd_buf { storage.template acquire<roles::Metadata, 0, ncarray::HostTag>() };
        IOStatus status = get_stream<BD>(streams).read_at(bd_buf, file_offset, read_size);
        if (status == IOStatus::Success) {
          status = (get_stream<BD>(streams).read_count() == 0) ? IOStatus::ZeroBytesRead : IOStatus::Success;
        }

        storage.template release<roles::Index, 0>(l1_offsets);
        storage.template release<roles::Index, 1>(transition_offsets);
        storage.template release<roles::Metadata, 0>(bd_buf);

        return status;
      }
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_multi_steps(Stream<IO, XTC2Traits>* streams,
                                              StorageViewT& storage,
                                              const StreamCatalog<XTC2Traits>& catalog,
                                              FetchCursor<XTC2Traits>& cursor,
                                              const GenericStreamConfig<XTC2Traits>& cfg,
                                              StepIdxType step_idx,
                                              StepIdxType count,
                                              DataAccessPtn ptn) {
      cursor.last_access_ptn = ptn;
      if (ptn == XTC2Traits::DataAccessPtn::L1Accept) {
        std::size_t start_index { step_idx % cfg.index_batch_size };
        std::size_t end_index { start_index + count - 1 };

        if (end_index >= catalog.num_steps[0]) {
          return IOStatus::AllRequestedRead;
        }

        auto* l1_offsets_buf =
          storage.template acquire<roles::Index, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        auto* l1_offsets { reinterpret_cast<EventOffset*>(l1_offsets_buf) };

        auto& start_offset = l1_offsets[start_index];
        auto& end_offset = l1_offsets[end_index];
        std::size_t file_offset { start_offset.offset };
        std::size_t read_size { (end_offset.offset + end_offset.size) - file_offset };

        std::size_t bd_buf_size { storage.template size<roles::Data, 0>() };
        auto* bd_buf = storage.template acquire<roles::Data, 0, ncarray::HostTag>();

        IOStatus status { IOStatus::Success };
        if (read_size <= bd_buf_size) {
          status = get_stream<BD>(streams).read_at(bd_buf, file_offset, read_size);
        } else {
          // First case, we have interspersed transitions in a batched read.
          // If that's true, see if reading only the events in will get us under
          // the limit
          std::size_t l1s_size { 0 };
          for (std::size_t i = start_index; i <= end_index; ++i) {
            l1s_size += l1_offsets[i].size;
          }

          if (l1s_size <= bd_buf_size) {
            // Must read in chunks
            std::size_t dst_offset { 0 };
            for (std::size_t i = start_index; i <= end_index; ++i) {
              char* dst_ptr { reinterpret_cast<char*>(bd_buf) + dst_offset };
              status = get_stream<BD>(streams).read_at(dst_ptr, l1_offsets[i].offset, l1_offsets[i].size);

              if (status != IOStatus::Success) {
                break;
              }

              dst_offset += l1_offsets[i].size;
            }
          } else {
            // Too big sadly.
            status = IOStatus::TruncatedRead;
          }
        }

        if (status == IOStatus::Success) {
          std::size_t read_count = get_stream<BD>(streams).read_count();
          if (read_count == 0) {
            status = IOStatus::ZeroBytesRead;
          } else {
            status = IOStatus::Success;
          }
        }

        storage.template release<roles::Index, 0>(l1_offsets);
        storage.template release<roles::Data, 0>(bd_buf);
        return status;
      } else {
        /// TODO: Implement... something for this.
      }

      return IOStatus::GeneralIOError;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_multi_steps_stride(Stream<IO, XTC2Traits>* streams,
                                                     StorageViewT& storage,
                                                     const StreamCatalog<XTC2Traits>& catalog,
                                                     FetchCursor<XTC2Traits>& cursor,
                                                     const GenericStreamConfig<XTC2Traits>& cfg,
                                                     StepIdxType step_idx,
                                                     StepIdxType count,
                                                     StepIdxType stride,
                                                     DataAccessPtn ptn) {
      cursor.last_access_ptn = ptn;
      if (ptn == XTC2Traits::DataAccessPtn::L1Accept) {
        /// TODO: Implement... something for this.
      } else {
        /// TODO: Implement... something for this.
      }

      return IOStatus::GeneralIOError;
    }

    template <class StorageViewT>
    SBIO_HD static DataResult get_data_in_buffer(StorageViewT& storage,
                                                 const MetadataInventory<XTC2Traits>& inv,
                                                 const DataRequest& req,
                                                 DataAccessPtn ptn,
                                                 std::size_t batch_idx = 0) {
      DataRequest corrected_req { req };
      XTC2::TransitionId target_service { XTC2::TransitionId::L1Accept };
      void* bd_buf { nullptr };
      XTC2::Dgram* dg { nullptr };

      if (ptn == DataAccessPtn::L1Accept) {
        bd_buf =
          storage.template acquire<roles::Data, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        dg = reinterpret_cast<XTC2::Dgram*>(bd_buf);

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

        bd_buf =
          storage.template acquire<roles::Metadata, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        dg = reinterpret_cast<XTC2::Dgram*>(bd_buf);
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

      DataResult res = XTC2Traits::resolve_data(dg, inv, corrected_req);

      if (ptn == DataAccessPtn::L1Accept) {
        res.data = storage.template release<roles::Data, 0>(bd_buf, res.data);
      } else {
        res.data = storage.template release<roles::Metadata, 0>(bd_buf, res.data);
      }

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
