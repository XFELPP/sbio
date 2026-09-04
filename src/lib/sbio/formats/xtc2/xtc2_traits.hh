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
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/stream.hh"
#include "sbio/core/sync.hh"
#include "sbio/export_macro.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/formats/xtc2/traversal.hh"
#include "sbio/formats/xtc2/xtc2.hh"
#include "sbio/locators/path_pattern.hh"
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

    struct SBIO_API DataSourceParameters {
#ifndef __CUDA_ARCH__
      DataSourceParameters(const std::string& exp, unsigned run_) {
        safe_strncpy(experiment, exp.c_str(), exp.size() + 1);
        run = run_;
      }
#endif
      DataSourceParameters() = default;

      char experiment[MaxNameSize];
      unsigned run;
    };

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

    /**
     * A convenience struct to keep track of indices into offset buffers
     * and the total number of events and transitions seen.
     */
    struct DiscoveryState {
      ssize_t last_l1_idx_seen { -1 };                             ///< The index of the last L1Accept seen
      std::uint64_t next_bd_offset { 0 };                          ///< The next L1Accept offset
      bool seen_end_run { false };                                 ///< Whether an EndRun transition has been passed
      std::size_t l1_offset_idx { 0 };                             ///< Current index into L1Accept offsets
      std::size_t trans_offset_idx { 0 };                          ///< Current index into transition offsets
      std::size_t events_per_read { 0 };                           ///< Number of offsets to read per indexing
      std::size_t num_transitions { 0 };                           ///< Number of transition (offset)s read
      std::size_t num_events { 0 };                                ///< Number of event (offset)s read
      std::size_t curr_smd_offset { 0 };                           ///< Current offset in .smd
      DataAccessPtn last_accessed_ptn { DataAccessPtn::L1Accept }; ///< Indicate last buffer used
    };

    using RequestSchema = sbio::RequestFieldSchema<"alg", "field">;
    using DataRequest = sbio::DataRequest<RequestSchema>;

    using GroupKeys = sbio::GroupMetadataKeys<"serial_number">;

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
                                                DiscoveryState& state,
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

    SBIO_HD static auto sync_vars(DiscoveryState& state) {
      return make_sync_group(state.num_events, state.num_transitions);
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus index_stream(Stream<IO, XTC2Traits>* streams,
                                         StorageViewT& storage,
                                         DiscoveryState& stream_state,
                                         const GenericStreamConfig<XTC2Traits>& cfg) {
      auto events_per_read { cfg.index_batch_size };
      stream_state.events_per_read = events_per_read;

      std::size_t read_size { (sizeof(XTC2::Dgram) + 80) * events_per_read };

      std::size_t missing_chunk { 0 };
      if (stream_state.curr_smd_offset) {
        std::size_t last_bytes_read = get_stream<SMD>(streams).read_count();
        missing_chunk = last_bytes_read - stream_state.curr_smd_offset;
      }

      // Read a lot into a scratch buffer
      auto* smd_buf =
        storage.template acquire<roles::Metadata, 1, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
      IOStatus status = get_stream<SMD>(streams).read_batch(smd_buf, read_size, missing_chunk);

      if (status == IOStatus::ZeroBytesRead) {
        stream_state.num_events = 0;
        stream_state.num_transitions = 0;
      } else if (status == IOStatus::Success) {
        void* idx0_buf =
          storage.template acquire<roles::Index, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        auto* l1_offsets { reinterpret_cast<EventOffset*>(idx0_buf) };

        void* idx1_buf =
          storage.template acquire<roles::Index, 1, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        auto* transition_offsets { reinterpret_cast<TransitionOffset*>(idx1_buf) };

        std::size_t bytes_read = get_stream<SMD>(streams).read_count();
        stream_state.curr_smd_offset = 0;
        std::size_t n_events { 0 };
        std::size_t n_transitions { 0 };
        while (stream_state.curr_smd_offset < bytes_read) {
          if (n_events >= events_per_read) {
            status = IOStatus::AllRequestedRead;
            break;
          }

          if (stream_state.curr_smd_offset + sizeof(XTC2::Dgram) > bytes_read) {
            // Will want to deal with this differently for handling "Live Mode"
            status = IOStatus::TruncatedRead;
            break;
          }

          auto* dg = reinterpret_cast<XTC2::Dgram*>(reinterpret_cast<char*>(smd_buf) +
                                                    stream_state.curr_smd_offset);

          stream_state.curr_smd_offset += populate_offsets(dg,
                                                           stream_state,
                                                           0,
                                                           l1_offsets,
                                                           transition_offsets,
                                                           stream_state.curr_smd_offset);

          if (dg->service() == XTC2::TransitionId::L1Accept) {
            n_events++;
          } else {
            n_transitions++;
          }
        }

        stream_state.num_events = n_events;
        stream_state.num_transitions = n_transitions;

        storage.template release<roles::Index, 0>(l1_offsets);
        storage.template release<roles::Index, 1>(transition_offsets);
      }

      storage.template release<roles::Metadata, 1>(smd_buf);
      return status;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_step(Stream<IO, XTC2Traits>* streams,
                                       StorageViewT& storage,
                                       DiscoveryState& stream_state,
                                       const GenericStreamConfig<XTC2Traits>& cfg,
                                       StepIdxType step_idx,
                                       DataAccessPtn ptn) {
      stream_state.events_per_read = cfg.index_batch_size;
      stream_state.last_accessed_ptn = ptn;
      if (ptn == XTC2Traits::DataAccessPtn::L1Accept) {
        std::size_t adjusted_index { step_idx % stream_state.events_per_read };
        if (adjusted_index >= stream_state.num_events) {
          return IOStatus::AllRequestedRead;
        }

        void* idx0_buf =
          storage.template acquire<roles::Index, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
        auto* l1_offsets { reinterpret_cast<EventOffset*>(idx0_buf) };

        auto& offset = l1_offsets[adjusted_index];
        std::size_t file_offset { offset.offset };
        std::size_t read_size { offset.size };

        auto* bd_buf = storage.template acquire<roles::Data, 0, ncarray::HostTag>();
        IOStatus status = get_stream<BD>(streams).read_at(bd_buf, file_offset, read_size);

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
        // TODO: Update for BufferMemorySpace vs CallerMemorySpace
        XTC2::TransitionId transition_id { XTC2::TransitionId::Unused_11 };
        if (ptn == DataAccessPtn::SlowUpdate) {
          transition_id = XTC2::TransitionId::SlowUpdate;
        } else if (ptn == DataAccessPtn::BeginStep) {
          transition_id = XTC2::TransitionId::BeginStep;
        }

        std::size_t adjusted_index { step_idx % stream_state.events_per_read };
        if (adjusted_index >= stream_state.num_events) {
          return IOStatus::AllRequestedRead;
        }

        auto* l1_offsets =
          reinterpret_cast<EventOffset*>(storage.template acquire<roles::Index, 0, ncarray::HostTag>());
        auto* transition_offsets =
          reinterpret_cast<TransitionOffset*>(storage.template acquire<roles::Index, 1, ncarray::HostTag>());

        auto& curr_transition_index { stream_state.trans_offset_idx };
        auto& offset { transition_offsets[curr_transition_index] };

        // We'll try to add some minimal support to go backwards after you've gone
        // through once - this only will work within the events_per_read batch of
        // indices though
        if (offset.previous_l1_index > static_cast<ssize_t>(step_idx)) {
          curr_transition_index = 0;      // Just reset back to beginning of buffer
          offset = transition_offsets[0]; // It will re-iterate through below
        }

        while (offset.transition_id != transition_id ||
               offset.previous_l1_index <= static_cast<ssize_t>(step_idx)) {
          curr_transition_index++;

          if (curr_transition_index >= stream_state.num_transitions) {
            return IOStatus::AllRequestedRead;
          }

          auto& next_transition_offset { transition_offsets[curr_transition_index] };
          if (next_transition_offset.previous_l1_index > static_cast<ssize_t>(step_idx)) {
            // Since we incremented at the statr of the loop, check if we went past
            // and break if so - we haven't set the offset again yet
            break;
          }

          if (next_transition_offset.transition_id == transition_id) {
            offset = transition_offsets[curr_transition_index];
          }
        }

        std::int64_t prev_l1_index { offset.previous_l1_index };

        std::size_t read_size { 0 };
        std::size_t file_offset { 0 };
        if (prev_l1_index <= static_cast<std::int64_t>(step_idx) &&
            curr_transition_index < stream_state.num_transitions) {
          read_size = offset.size;
          if (prev_l1_index == -1) {
            auto& l1_offset = l1_offsets[0];
            std::size_t total_offset_from_l1 { read_size };
            std::size_t transition_index { curr_transition_index };

            auto& next_transition_offset { transition_offsets[transition_index] };
            while (next_transition_offset.previous_l1_index == -1) {
              total_offset_from_l1 += next_transition_offset.size;
              transition_index++;
              next_transition_offset = transition_offsets[transition_index];
            }

            file_offset = l1_offset.offset - total_offset_from_l1;
          } else {
            file_offset = offset.offset;
          }
        }

        auto* bd_buf { storage.template acquire<roles::Metadata, 0, ncarray::HostTag>() };
        IOStatus status = get_stream<BD>(streams).read_at(bd_buf, file_offset, read_size);

        if (status == IOStatus::Success) {
          std::size_t read_count = get_stream<BD>(streams).read_count();
          if (read_count == 0) {
            status = IOStatus::ZeroBytesRead;
          } else {
            status = IOStatus::Success;
          }
        }

        storage.template release<roles::Index, 0>(l1_offsets);
        storage.template release<roles::Index, 1>(transition_offsets);
        storage.template release<roles::Metadata, 0>(bd_buf);
        return status;
      }

      return IOStatus::GeneralIOError;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_multi_steps(Stream<IO, XTC2Traits>* streams,
                                              StorageViewT& storage,
                                              DiscoveryState& stream_state,
                                              const GenericStreamConfig<XTC2Traits>& cfg,
                                              StepIdxType step_idx,
                                              StepIdxType count,
                                              DataAccessPtn ptn) {
      stream_state.events_per_read = cfg.index_batch_size;
      stream_state.last_accessed_ptn = ptn;
      if (ptn == XTC2Traits::DataAccessPtn::L1Accept) {
        std::size_t start_index { step_idx % stream_state.events_per_read };
        std::size_t end_index { start_index + count - 1 };

        if (end_index >= stream_state.num_events) {
          return IOStatus::AllRequestedRead;
        }

        auto* l1_offsets =
          reinterpret_cast<EventOffset*>(storage.template acquire<roles::Index, 0, ncarray::HostTag>());

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
                                                     DiscoveryState& stream_state,
                                                     const GenericStreamConfig<XTC2Traits>& cfg,
                                                     StepIdxType step_idx,
                                                     StepIdxType count,
                                                     StepIdxType stride,
                                                     DataAccessPtn ptn) {
      stream_state.events_per_read = cfg.index_batch_size;
      stream_state.last_accessed_ptn = ptn;
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
          std::size_t i { 0 };
          for (; i < 5; ++i) {
            corrected_req.group_name[i] = epics_det_name[i];
          }
          corrected_req.group_name[i] = '\0';

          corrected_req.set<"field">(req.group_name);
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
          if (std::strcmp(corrected_req.group_name, "scan") != 0) {
            const char* scan_det_name { "scan" };
            std::size_t i { 0 };
            for (; i < 4; ++i) {
              corrected_req.group_name[i] = scan_det_name[i];
            }
            corrected_req.group_name[i] = '\0';

            corrected_req.set<"field">(req.group_name);
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

    template <class StorageViewT>
    SBIO_HD static auto capacity(const StorageViewT& storage,
                                 const DiscoveryState& state) {
      return state.num_events;
    }

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
  };
} // namespace sbio

#endif // SBIO_FORMATS_XTC2_XTC2_TRAITS_HH
