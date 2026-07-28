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

#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/sync.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/formats/xtc2/traversal.hh"
#include "sbio/formats/xtc2/xtc2.hh"
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

namespace fs = std::filesystem;

namespace sbio {
  struct XTC2Traits : public BaseTraits {
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
    enum Roles { SMD, BD };
    static constexpr std::size_t RoleCount { 2 };

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

    struct StreamParameters {
      char smd_path[MaxNameSize];
      char xtc_path[MaxNameSize];
      std::size_t max_dgram_size { 0x4000000 };
      std::size_t max_dgram_batch { 1 };
      std::size_t events_per_read { 43200 };
    };

    struct DataSourceParameters {
#ifndef __CUDA_ARCH__
      DataSourceParameters(const std::string& exp, unsigned run_) {
        safe_strncpy(experiment, exp.c_str(), exp.size());
        run = run_;
      }
#endif
      DataSourceParameters() = default;

      char experiment[MaxNameSize];
      unsigned run;
    };

    template <typename DS>
    static bool make_stream_brokers(DS& ds,
                                    const DataSourceParameters& ds_params,
                                    StreamParameters& base_cfg) {
#ifndef __CUDA_ARCH__
      std::string SIT_PSDM_DATA = std::getenv("SIT_PSDM_DATA");
      if (SIT_PSDM_DATA.empty()) {
        SIT_PSDM_DATA = "/sdf/data/lcls/ds";
      }

      unsigned run { ds_params.run };
      std::string exp(ds_params.experiment);
      std::string hutch { exp.substr(0,3) };
      std::ostringstream dir_oss;
      dir_oss << SIT_PSDM_DATA << "/" << hutch << "/" << exp << "/xtc";
      std::string xtc_dir = dir_oss.str();

      std::ostringstream oss;
      oss << exp << "-r" << std::setw(4) << std::setfill('0') << run;
      std::string file_base_ptn = oss.str();
      for (auto const& dir_entry : fs::directory_iterator(xtc_dir)) {
        std::string xtc_path = dir_entry.path();
        if (xtc_path.find(file_base_ptn) != std::string::npos) {
          std::string xtc_stem = dir_entry.path().stem();
          std::string ext = dir_entry.path().extension().string();

          std::string smd_path;
          if (ext == ".xtc") {
            smd_path = xtc_dir + "/smalldata/" + xtc_stem + ".smd.xtc";
          } else {
            smd_path = xtc_dir + "/smalldata/" + xtc_stem + ".smd.xtc2";
          }

          StreamParameters stream_cfg = base_cfg;
          safe_strncpy(stream_cfg.smd_path, smd_path.c_str(), smd_path.size() + 1);
          safe_strncpy(stream_cfg.xtc_path, xtc_path.c_str(), xtc_path.size() + 1);

          // NOTE: add_data_stream increments m_num_data_streams
          ds.add_data_stream(stream_cfg);
        }
      }

      return ds.num_data_streams() > 0;
#else
      return false;
#endif
    }

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
    using BrokerBufferRequirements = TypeList <
      BufferDescriptor<MetadataRole, 0, sizeof(XTC2::Dgram)>,  /* Buffer for transition */
      BufferDescriptor<MetadataRole, 1, sizeof(XTC2::Dgram)>,  /* Scratch buffer for SMD */
      BufferDescriptor<DataRole, 0, sizeof(XTC2::Dgram)>,      /* Buffer for events */
      BufferDescriptor<IndexRole, 0, sizeof(EventOffset)>,     /* EventOffsets buffer */
      BufferDescriptor<IndexRole, 1, sizeof(TransitionOffset)> /* TransitionOffsets buffer */
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

    struct DataRequest {
      DataRequest() = default;

      DataRequest(const DataRequest& other) = default;
      DataRequest& operator=(const DataRequest& other) = default;
      DataRequest(DataRequest&& other) noexcept = default;
      DataRequest& operator=(DataRequest&& other) noexcept = default;

      DataRequest(const char* name,
                  const char* type,
                  const char* alg,
                  const char* field) {
        safe_strncpy(detector_name, name, MaxNameSize);
        safe_strncpy(detector_type, type, MaxNameSize);
        safe_strncpy(alg_name, alg, MaxNameSize);
        safe_strncpy(field_name, field, MaxNameSize);
      }

      DataRequest(const char* name, const char* type) {
        safe_strncpy(detector_name, name, MaxNameSize);
        safe_strncpy(detector_type, type, MaxNameSize);
        safe_strncpy(alg_name, "raw", MaxNameSize);
        safe_strncpy(field_name, "raw", MaxNameSize);
      }

      char detector_type[MaxNameSize];
      char detector_name[MaxNameSize];
      std::uint32_t segment_number { 0 };
      char alg_name[MaxNameSize];
      char field_name[MaxNameSize];
    };

    // Defined below (larger)
    /**
     * The struct to hold maps for fast data lookup.
     * This should be flat and allow for O(1) retrieval of offsets into a
     * a datagram for a combination of (Detector, Algorithm, Field)
     */
    struct MetadataInventory;

    /**
     * The struct to be used to return results when data is requested.
     * This is defined in the XTC2 namespace and aliased here.
     * It has the structure:
     * struct DataResult {
     *   const void* data;           ///< Pointer to the raw data
     *   std::size_t size;           ///< Total bytes of payload
     *   std::uint16_t rank;         ///< Rank of data (num dimensions)
     *   const std::uint32_t* shape; ///< Shape of the data
     *   XTC2::DType dtype;          ///< Data type
     * };
     */
    using DataResult = typename XTC2::DataResult;

    // Defined at end of the header, after MetadataInventory is completed
    template <class DataBrokerType, class SegmentRef>
    SBIO_HD static std::size_t find_group_segments(const MetadataInventory& inv,
                                                   const char* name,
                                                   SegmentRef* ref_out,
                                                   std::size_t max_out,
                                                   DataBrokerType* broker,
                                                   char* dettype = nullptr,
                                                   DataAccessPtn ptn = DataAccessPtn::L1Accept);

    SBIO_HD static inline std::size_t get_payload_size(void* buf) {
      return reinterpret_cast<XTC2::Dgram*>(buf)->xtc.sizeofPayload();
    }

    SBIO_HD static void discover_metadata(DataUnit* buffer,
                                          MetadataInventory& inv,
                                          std::size_t offset);

    SBIO_HD static DataResult resolve_data(void* buffer,
                                           const MetadataInventory& inv,
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

    SBIO_HD static AllocationRequest<XTC2Traits> get_allocation_request(StreamParameters& cfg) {
      AllocationRequest<XTC2Traits> request;
      request.size_requests[0] = cfg.max_dgram_size * cfg.max_dgram_batch; // Transition buf
      request.size_requests[1] = // Scratch buffer for reading ahead in smd file
        cfg.events_per_read * (sizeof(XTC2::Dgram) + 80);
      request.size_requests[2] = cfg.max_dgram_size * cfg.max_dgram_batch; // Event buf
      request.size_requests[3] = cfg.events_per_read * sizeof(XTC2Traits::EventOffset);
      request.size_requests[4] =
        cfg.events_per_read * sizeof(XTC2Traits::TransitionOffset);

      return request;
    }

    SBIO_HD static std::size_t max_batch_count(StreamParameters& cfg) {
      return cfg.max_dgram_batch;
    }

    template <IOTraits IO>
    SBIO_HD static IOStatus open_streams(Stream<IO, XTC2Traits>* streams,
                                         const StreamParameters& cfg) {
      if (streams[SMD].connect(cfg.smd_path) != IOStatus::Success) {
        return IOStatus::OpenFailed;
      }

      if (streams[BD].connect(cfg.xtc_path) != IOStatus::Success) {
        return IOStatus::OpenFailed;
      }

      return IOStatus::Success;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus discover_metadata(Stream<IO, XTC2Traits>* streams,
                                              StorageViewT& storage,
                                              MetadataInventory& inv) {
      auto* smd_buf = storage.template acquire<MetadataRole>();

      IOStatus status = streams[SMD].read_one(smd_buf,
                                              storage.template size<MetadataRole>());

      if (status == IOStatus::Success) {
        auto* dg = reinterpret_cast<XTC2::Dgram*>(smd_buf);

        if (dg->service() == XTC2::TransitionId::Configure) {
          XTC2Traits::discover_metadata(dg, inv, 0);
        }
      }

      storage.template release<MetadataRole, 0>(smd_buf, status);
      return status;
    }

    SBIO_HD static auto sync_vars(DiscoveryState& state) {
      return make_sync_group(state.num_events, state.num_transitions);
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus index_stream(Stream<IO, XTC2Traits>* streams,
                                         StorageViewT& storage,
                                         DiscoveryState& stream_state,
                                         const StreamParameters& cfg) {
      auto events_per_read { cfg.events_per_read };
      stream_state.events_per_read = events_per_read;

      std::size_t read_size { (sizeof(XTC2::Dgram) + 80) * events_per_read };

      std::size_t missing_chunk { 0 };
      if (stream_state.curr_smd_offset) {
        std::size_t last_bytes_read = streams[SMD].read_count();
        missing_chunk = last_bytes_read - stream_state.curr_smd_offset;
      }

      // Read a lot into a scratch buffer
      auto* smd_buf = storage.template acquire<MetadataRole, 1>();
      IOStatus status = streams[SMD].read_batch(smd_buf, read_size, missing_chunk);

      if (status == IOStatus::ZeroBytesRead) {
        stream_state.num_events = 0;
        stream_state.num_transitions = 0;
      } else if (status == IOStatus::Success) {
        auto* l1_offsets =
          reinterpret_cast<EventOffset*>(storage.template acquire<IndexRole, 0>());
        auto* transition_offsets =
            reinterpret_cast<TransitionOffset*>(storage.template acquire<IndexRole, 1>());

        std::size_t bytes_read = streams[SMD].read_count();
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

        storage.template release<IndexRole, 0>(l1_offsets, status);
        storage.template release<IndexRole, 1>(transition_offsets, status);
      }

      storage.template release<MetadataRole, 1>(smd_buf, status);
      return status;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_step(Stream<IO, XTC2Traits>* streams,
                                       StorageViewT& storage,
                                       DiscoveryState& stream_state,
                                       const StreamParameters& cfg,
                                       StepIdxType step_idx,
                                       DataAccessPtn ptn) {
      stream_state.events_per_read = cfg.events_per_read;
      stream_state.last_accessed_ptn = ptn;
      if (ptn == XTC2Traits::DataAccessPtn::L1Accept) {
        std::size_t adjusted_index { step_idx % stream_state.events_per_read };
        if (adjusted_index >= stream_state.num_events) {
          return IOStatus::AllRequestedRead;
        }

        auto* l1_offsets =
          reinterpret_cast<EventOffset*>(storage.template acquire<IndexRole, 0>());

        auto& offset = l1_offsets[adjusted_index];
        std::size_t file_offset { offset.offset };
        std::size_t read_size { offset.size };

        auto* bd_buf = storage.template acquire<DataRole, 0>();
        IOStatus status = streams[BD].read_at(bd_buf, file_offset, read_size);

        if (status == IOStatus::Success) {
          std::size_t read_count = streams[BD].read_count();
          if (read_count == 0) {
            status = IOStatus::ZeroBytesRead;
          } else {
            status = IOStatus::Success;
          }
        }

        storage.template release<IndexRole, 0>(l1_offsets, status);
        storage.template release<DataRole, 0>(bd_buf, status);
        return status;
      } else {
        XTC2::TransitionId transition_id;
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
          reinterpret_cast<EventOffset*>(storage.template acquire<IndexRole, 0>());
        auto* transition_offsets =
          reinterpret_cast<TransitionOffset*>(storage.template acquire<IndexRole, 1>());

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

        auto* bd_buf { storage.template acquire<MetadataRole, 0>() };
        IOStatus status = streams[BD].read_at(bd_buf, file_offset, read_size);

        if (status == IOStatus::Success) {
          std::size_t read_count = streams[BD].read_count();
          if (read_count == 0) {
            status = IOStatus::ZeroBytesRead;
          } else {
            status = IOStatus::Success;
          }
        }

        storage.template release<IndexRole, 0>(l1_offsets, status);
        storage.template release<IndexRole, 1>(transition_offsets, status);
        storage.template release<MetadataRole, 0>(bd_buf, status);
        return status;
      }

      return IOStatus::GeneralIOError;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_multi_steps(Stream<IO, XTC2Traits>* streams,
                                              StorageViewT& storage,
                                              DiscoveryState& stream_state,
                                              const StreamParameters& cfg,
                                              StepIdxType step_idx,
                                              StepIdxType count,
                                              DataAccessPtn ptn) {
      stream_state.events_per_read = cfg.events_per_read;
      stream_state.last_accessed_ptn = ptn;
      if (ptn == XTC2Traits::DataAccessPtn::L1Accept) {
        std::size_t start_index { step_idx % stream_state.events_per_read };
        std::size_t end_index { start_index + count - 1 };

        if (end_index >= stream_state.num_events) {
          return IOStatus::AllRequestedRead;
        }

        auto* l1_offsets =
          reinterpret_cast<EventOffset*>(storage.template acquire<IndexRole, 0>());

        auto& start_offset = l1_offsets[start_index];
        auto& end_offset = l1_offsets[end_index];
        std::size_t file_offset { start_offset.offset };
        std::size_t read_size { (end_offset.offset + end_offset.size) - file_offset };

        std::size_t bd_buf_size { storage.template size<DataRole, 0>() };
        auto* bd_buf = storage.template acquire<DataRole, 0>();

        IOStatus status { IOStatus::Success };
        if (read_size <= bd_buf_size) {
          status = streams[BD].read_at(bd_buf, file_offset, read_size);
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
              status = streams[BD].read_at(dst_ptr, l1_offsets[i].offset, l1_offsets[i].size);

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
          std::size_t read_count = streams[BD].read_count();
          if (read_count == 0) {
            status = IOStatus::ZeroBytesRead;
          } else {
            status = IOStatus::Success;
          }
        }

        storage.template release<IndexRole, 0>(l1_offsets, status);
        storage.template release<DataRole, 0>(bd_buf, status);
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
                                                     const StreamParameters& cfg,
                                                     StepIdxType step_idx,
                                                     StepIdxType count,
                                                     StepIdxType stride,
                                                     DataAccessPtn ptn) {
      stream_state.events_per_read = cfg.events_per_read;
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
                                                 const MetadataInventory& inv,
                                                 const DataRequest& req,
                                                 DataAccessPtn ptn,
                                                 std::size_t batch_idx = 0) {
      DataRequest corrected_req { req };
      XTC2::TransitionId target_service;
      void* bd_buf { nullptr };
      XTC2::Dgram* dg { nullptr };
      if (ptn == DataAccessPtn::L1Accept) {
        bd_buf = storage.template acquire<DataRole, 0>();
        dg = reinterpret_cast<XTC2::Dgram*>(bd_buf);

        target_service = XTC2::TransitionId::L1Accept;
      } else {
        if (ptn == DataAccessPtn::SlowUpdate) {
          target_service = XTC2::TransitionId::SlowUpdate;

          // Semantic Mapping: Requested name is actually a PV field in "epics"
          const char* epics_det_name { "epics" };
          std::size_t i { 0 };
          for (; i < 5; ++i) {
            corrected_req.detector_name[i] = epics_det_name[i];
          }
          corrected_req.detector_name[i] = '\0';

          i = 0;
          for (; i < XTC2Traits::MaxNameSize - 1 && req.detector_name[i] != '\0'; ++i) {
            corrected_req.field_name[i] = req.detector_name[i];
          }
          corrected_req.field_name[i] = '\0';
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
          if (std::strcmp(corrected_req.detector_name, "scan") != 0) {
            const char* scan_det_name { "scan" };
            std::size_t i { 0 };
            for (; i < 4; ++i) {
              corrected_req.detector_name[i] = scan_det_name[i];
            }
            corrected_req.detector_name[i] = '\0';

            i = 0;
            for (; i < XTC2Traits::MaxNameSize - 1 && req.detector_name[i] != '\0'; ++i) {
              corrected_req.field_name[i] = req.detector_name[i];
            }
            corrected_req.field_name[i] = '\0';
          }
        }

        bd_buf = storage.template acquire<MetadataRole, 0>();
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
        storage.template release<DataRole, 0>(bd_buf, res);
      } else {
        storage.template release<MetadataRole, 0>(bd_buf, res);
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
        auto* buf = storage.template acquire<DataRole, 0>();
        storage.template release<DataRole, 0>(buf);

        return buf;
      } else {
        auto* buf = storage.template acquire<MetadataRole, 0>();
        storage.template release<MetadataRole, 0>(buf);

        return buf;
      }
    }
  };

  struct XTC2Traits::MetadataInventory {
    // Resolve a (Detector, Segment, Algorithm) into a NamesId for lookup
    struct DetAlgKey {
      char dettype[MaxNameSize];
      char detname[MaxNameSize];
      std::uint32_t segment;
      char algname[MaxNameSize];
      char detId[MaxNameSize];

      SBIO_HD inline bool operator<(const DetAlgKey& other) const {
        int cmp = std::strcmp(dettype, other.dettype);
        if (cmp != 0) {
          return cmp < 0;
        }

        cmp = std::strcmp(detname, other.detname);

        if (cmp != 0) {
          return cmp < 0;
        }

        if (segment != other.segment) {
          return segment < other.segment;
        }

        return std::strcmp(algname, other.algname) < 0;
      }
    };

    struct NamesIdMap {
      DetAlgKey key;
      std::uint32_t names_id;

      SBIO_HD inline bool operator<(const NamesIdMap& other) const {
        return key < other.key;
      }

      SBIO_HD inline bool operator<(const DetAlgKey& other) const {
        return key < other;
      }
    };

    // Resolve a (NamesId, FieldName) into a field index
    struct FieldKey {
      std::uint32_t names_id;
      char fieldname[XTC2Traits::MaxNameSize];

      SBIO_HD inline bool operator<(const FieldKey& other) const {
        if (names_id != other.names_id) {
          return names_id < other.names_id;
        }
        return std::strcmp(fieldname, other.fieldname) < 0;
      }
    };

    struct FieldMap {
      FieldKey key;
      std::uint32_t field_idx;

      SBIO_HD inline bool operator<(const FieldMap& other) const {
        return key < other.key;
      }

      SBIO_HD inline bool operator<(const FieldKey& other) const {
        return key < other;
      }
    };

    struct SdOffsetMap {
      std::uint32_t names_id;
      std::uint32_t offset;

      SBIO_HD SdOffsetMap(std::uint32_t nid, std::uint32_t off)
        : names_id(nid)
        , offset(off)
      {}

      SdOffsetMap() = default;

      SBIO_HD inline bool operator<(const SdOffsetMap& other) const {
        return names_id < other.names_id;
      }

      SBIO_HD inline bool operator<(std::uint32_t other_nid) const {
        return names_id < other_nid;
      }
    };

    struct SchemaOffset {
      std::uint32_t names_id;
      std::uint32_t start_idx; // Index into the flattened m_schemas array

      SBIO_HD inline bool operator<(const SchemaOffset& other) const {
        return names_id < other.names_id;
      }

      SBIO_HD inline bool operator<(std::uint32_t other_nid) const {
        return names_id < other_nid;
      }
    };

    // Storage: Usually allocated as a single memory block
    // during the Configure transition.
    NamesIdMap* m_names_id_table { nullptr };
    FieldMap* m_field_table { nullptr };
    SdOffsetMap* m_sd_offsets { nullptr };
    SchemaOffset* m_schema_offsets { nullptr };
    XTC2::Name* m_schemas { nullptr };

    std::size_t m_names_id_count { 0 };
    std::size_t m_field_count { 0 };
    std::size_t m_schema_offset_count { 0 };
    std::size_t m_sd_offset_count { 0 };

    // Lookup methods
    SBIO_HD std::uint32_t get_sd_offset(std::uint32_t nid) const;

    SBIO_HD const XTC2::Name* get_schema(std::uint32_t nid) const;

    SBIO_HD std::uint32_t resolve_names_id(const DataRequest& req) const;

    SBIO_HD std::uint32_t resolve_field_idx(std::uint32_t nid, const char* field) const;
  };

  template <class DataBrokerType, class SegmentRef>
  SBIO_HD inline std::size_t XTC2Traits::find_group_segments(const XTC2Traits::MetadataInventory& inv,
                                                             const char* name,
                                                             SegmentRef* ref_out,
                                                             std::size_t max_out,
                                                             DataBrokerType* broker,
                                                             char* dettype,
                                                             XTC2Traits::DataAccessPtn ptn) {
    std::size_t n_found { 0 };

    if (ptn == DataAccessPtn::L1Accept) {
      for (std::size_t i = 0; i < inv.m_names_id_count && n_found < max_out; ++i) {
        if (std::strcmp(inv.m_names_id_table[i].key.detname, name) == 0) {
          std::uint32_t segment_no = inv.m_names_id_table[i].key.segment;

          // If the detector has multiple algorithms it will appear various times
          // So don't record it again
          bool duplicate { false };
          for (std::size_t j = 0; j < n_found; ++j) {
            if (ref_out[j].segment_no == segment_no) {
              duplicate = true;
              break;
            }
          }

          if (!duplicate) {
            ref_out[n_found++] = {
              broker,
              segment_no,
              ptn
            };

            // If provided, populate the detector type as well
            const char* dettype_ = inv.m_names_id_table[i].key.dettype;
            std::size_t k { 0 };

            for (; k < XTC2Traits::MaxNameSize - 1 &&  dettype_[k] != '\0'; ++k) {
              dettype[k] = dettype_[k];
            }
            dettype[k] = '\0';
          }
        }
      }

      if (n_found > 0) {
        return n_found;
      }
    } else if (ptn == DataAccessPtn::SlowUpdate) {
      // Continue on to check for EPICS detectors
      // All EPICS (ie EPICSArch) detectors are under the `epics` name
      for (std::size_t i = 0; i < inv.m_names_id_count && n_found < max_out; ++i) {
        if (std::strcmp(inv.m_names_id_table[i].key.detname, "epics") == 0) {
          std::uint32_t nid = inv.m_names_id_table[i].names_id;
          if (inv.resolve_field_idx(nid, name) != 0xFFFFFFFF) {
            ref_out[n_found++] = {
              broker,
              inv.m_names_id_table[i].key.segment,
              DataAccessPtn::SlowUpdate
            };
          }
        }
      }

      if (n_found > 0) {
        return n_found;
      }
    } else {
      // NOTE: The `scan` detector behaves much like the normal detectors, but
      // has the data in BeginStep buffers, instead of L1Accept buffers.
      // It can always be access via `scan` detector above. However, for a syntactic
      // sugar, like with EPICS above, we'll allow detectors to be created based on
      // the scan variable name directly.
      // Normally, the scan will have a single algorithm, with these fields:
      // - `step_value`     : INT64
      // - `step_docstring` : CHARSTR, optional (but usually present)
      // - `scan_var_xxx`   : ANY (the actual scanned variable - may have multiple)
      // So we'll match the scan_var_names as we did above with EPICS
      for (std::size_t i = 0; i < inv.m_names_id_count && n_found < max_out; ++i) {
        if (std::strcmp(inv.m_names_id_table[i].key.detname, "scan") == 0) {
          std::uint32_t nid = inv.m_names_id_table[i].names_id;
          if (inv.resolve_field_idx(nid, name) != 0xFFFFFFFF) {
            ref_out[n_found++] = {
              broker,
              inv.m_names_id_table[i].key.segment,
              DataAccessPtn::BeginStep
            };
          }
        }
      }
    }
    return n_found;
  }
} // namespace sbio

#endif // SBIO_FORMATS_XTC2_XTC2_TRAITS_HH
