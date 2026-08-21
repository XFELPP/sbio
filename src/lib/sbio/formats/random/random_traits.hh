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

#ifndef SBIO_FORMATS_RANDOM_RANDOM_TRAITS_HH
#define SBIO_FORMATS_RANDOM_RANDOM_TRAITS_HH

#include "sbio/formats/format_traits.hh"
#include "sbio/formats/random/randfmt.hh"

#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/sync.hh"
#include "sbio/export_macro.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/util/string.hh"

#include <ncarray/ncarrays.hh>

#ifdef __CUDACC__

#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/cstring>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

namespace sbio {
  struct SBIO_API RandomTraits : public BaseTraits {
    static constexpr hd_std::size_t HeaderSize { sizeof(randfmt::Header) };
    static constexpr hd_std::uint16_t MaxRank { 5 };
    static constexpr hd_std::uint16_t MaxNameSize { 64 };

    using DataUnit = randfmt::Block;
    using StepIdxType = hd_std::size_t;
    static constexpr StepIdxType ExhaustedSentinel { static_cast<StepIdxType>(-1) };

    static constexpr StreamPartitioningStrategy PartitioningStrategy {
        StreamPartitioningStrategy::SubDivide
    };

    enum Roles { Data };
    static constexpr hd_std::size_t RoleCount { 1 };
    enum class DataAccessPtn : hd_std::uint8_t {
      Default = 0
    };
    static constexpr hd_std::size_t DataAccessPtnCount { 1 };

    enum class IndexingMode : hd_std::uint8_t {
      IndexAll   = 0, ///< When index_stream is called, all events will be indexed.
      IndexBatch = 1, ///< When index_stream is called a batch is indexed. This allows reindexing later.
      NoIndex    = 2  ///< Offsets are not written, so indexing is disabled and data is traversed linearly.
    };

    struct StreamParameters {
      hd_std::size_t num_events { 100000 };
      hd_std::size_t event_size { 0x100000 };
      hd_std::size_t max_batch { 1 };
      hd_std::uint32_t seed { 42 };
      hd_std::uint8_t pattern_type { 0 }; ///< 0 = PNRG, 1 = Sequential, 2 = Fixed fill
      bool enable_subblock_offsets { true };
      IndexingMode indexing_mode { IndexingMode::IndexAll };
      hd_std::size_t indexing_batch_size { 100 }; ///< If using IndexBatch, how many steps to index at a time

#ifdef _WIN32
      HANDLE h_file;
#else
      int fd;
#endif
    };

    struct SBIO_API DataSourceParameters {
      randfmt::DetectorSpec detectors[10];
      hd_std::uint8_t num_detectors { 0 };

      DataSourceParameters() = default;
    };

    template <typename DS>
    static bool make_stream_brokers(DS& ds,
                                    const DataSourceParameters& ds_params,
                                    StreamParameters& base_cfg) {
      // For now, will create the file(s) when trying to look for them...
#ifndef __CUDA_ARCH__
      hd_std::size_t nstream { 0 };
      char name_buf[MaxNameSize];
      for (hd_std::uint8_t d = 0; d < ds_params.num_detectors; ++d) {
        randfmt::DetectorSpec spec { ds_params.detectors[d] };

        // TODO: In the future, will want to add ability to split data into multiple streams
        hd_std::size_t streams_per_det { 1 };

        for (hd_std::size_t s = 0; s < streams_per_det; ++s) {
          int cnt = snprintf(name_buf, MaxNameSize, "sbio_random_stream_%zu", nstream);

          StreamParameters stream_cfg { base_cfg };

#ifdef _WIN32
          HANDLE h_file = CreateFileA(name_buf,
                                      GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                      nullptr,
                                      CREATE_ALWAYS,
                                      FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                                      nullptr);
          if (h_file == INVALID_HANDLE_VALUE) {
            return false;
          }

          stream_cfg.h_file = h_file;
          randfmt::FileHandle f_handle { h_file };
#else
          int fd = memfd_create(name_buf, 0);
          if (fd < 0) {
            return false;
          }

          stream_cfg.fd = fd;
          randfmt::FileHandle f_handle { fd };
#endif
          // Write the SuperBlock offsets only if the NoIndex mode was NOT requested
          bool enable_superblock_offsets { true };
          if (stream_cfg.indexing_mode == IndexingMode::NoIndex) {
            enable_superblock_offsets = false;
          }

          hd_std::uint64_t curr_offset { 0 };

          hd_std::uint8_t flags { 0 };
          if (stream_cfg.enable_subblock_offsets) {
            flags |= (1 << static_cast<hd_std::uint8_t>(randfmt::FormatFlags::SubBlockOffsetTable));
          }
          if (enable_superblock_offsets) {
            flags |= (1 << static_cast<hd_std::uint8_t>(randfmt::FormatFlags::SuperBlockOffsetTable));
          }
          // Just write 1 detector per stream for now...
          randfmt::DetectorSpec* stream_detectors { &spec };
          hd_std::uint8_t num_detectors_per_stream { 1 };
          hd_std::uint8_t* det_block_ids { &d };
          randfmt::write_sbiornd_file(f_handle,
                                      curr_offset,
                                      num_detectors_per_stream,
                                      stream_detectors,
                                      det_block_ids,
                                      stream_cfg.seed,
                                      stream_cfg.pattern_type,
                                      stream_cfg.num_events,
                                      flags);

          ds.add_data_stream(stream_cfg);
          nstream++;
        }
      }

      return ds.num_data_streams() > 0;
#else
      return false;
#endif
    }

    struct EventOffset {
      hd_std::uint64_t offset;
      hd_std::uint64_t size;
    };

    using BrokerBufferRequirements = TypeList <
      BufferDescriptor<MetadataRole, 0, sizeof(randfmt::Block)>,  /* Buffer for transition */
      BufferDescriptor<DataRole, 0, sizeof(randfmt::Block)>,      /* Buffer for events */
      BufferDescriptor<IndexRole, 0, sizeof(EventOffset)>         /* EventOffsets buffer */
    >;

    /**
     * A convenience struct to keep track of indices into offset buffers
     * and the total number of events and transitions seen.
     */
    struct DiscoveryState {
      hd_std::size_t events_per_read { 0 }; ///< Number of offsets to read per indexing
      hd_std::size_t num_events { 0 };      ///< Number of event (offset)s read
      hd_std::size_t curr_offset { 0 };     ///< Current offset along file
    };

    struct SBIO_API DataRequest {
      DataRequest() = default;

      DataRequest(const DataRequest& other) = default;
      DataRequest& operator=(const DataRequest& other) = default;
      DataRequest(DataRequest&& other) noexcept = default;
      DataRequest& operator=(DataRequest&& other) noexcept = default;

      DataRequest(const char* name_, const char* type_) {
        safe_strncpy(name, name_, MaxNameSize);
        safe_strncpy(type, type_, MaxNameSize);
      }

      char name[MaxNameSize];
      char type[MaxNameSize];
      std::uint32_t segment_number { 0 };
    };

    struct SBIO_API MetadataInventory {
      struct Entry {
        char name[MaxNameSize];
        char type[MaxNameSize];
        hd_std::uint16_t rank;
        hd_std::uint32_t shape[MaxRank];
        ncarray::DType dtype;
        mutable hd_std::size_t offset_in_event { 0 }; ///< Byte offset in the flat event buffer
        hd_std::size_t size { 0 };                    ///< Total bytes for this detector's data
      };

      Entry entries[16];
      hd_std::size_t count { 0 };

      SBIO_HD void add_detector(const char* name_,
                                const char* type_,
                                hd_std::uint16_t rank_,
                                const hd_std::uint32_t* shape_,
                                ncarray::DType dtype);
    };

    struct DataResult {
      const void* data;
      hd_std::size_t size;
      hd_std::uint16_t rank;
      hd_std::uint32_t shape[MaxRank];
      ncarray::DType dtype;
    };

    template <class DataBrokerType, class SegmentRef>
    SBIO_HD static std::size_t find_group_segments(const MetadataInventory& inv,
                                                   const char* name,
                                                   SegmentRef* ref_out,
                                                   std::size_t max_out,
                                                   DataBrokerType* broker,
                                                   char* dettype = nullptr,
                                                   DataAccessPtn ptn = DataAccessPtn::Default) {
      hd_std::size_t matched { 0 };
      for (hd_std::size_t i = 0; i < inv.count && matched < max_out; ++i) {
        if (hd_std::strcmp(inv.entries[i].name, name) == 0 || std::strcmp(name, "*") == 0) {
          if (dettype != nullptr) {
            safe_strncpy(dettype, inv.entries[i].type, MaxNameSize);
          }

          ref_out[matched] = SegmentRef { broker, i };
          matched++;
        }
      }

      return matched;
    }

    SBIO_HD static AllocationRequest<RandomTraits> get_allocation_request(StreamParameters& cfg) {
      AllocationRequest<RandomTraits> request;

      request.size_requests[0] = cfg.event_size;
      request.size_requests[1] = cfg.event_size;
      request.size_requests[2] = cfg.num_events * sizeof(EventOffset);

      return request;
    }

    SBIO_HD static std::size_t max_batch_count(StreamParameters& cfg) {
      return cfg.max_batch;
    }

    template <IOTraits IO>
    SBIO_HD static IOStatus open_streams(Stream<IO, RandomTraits>* streams,
                                         const StreamParameters& cfg) {
#ifdef _WIN32
      if (streams[Data].connect(cfg.h_file) != IOStatus::Success) {
        return IOStatus::OpenFailed;
      }
#else
      if (streams[Data].connect(cfg.fd) != IOStatus::Success) {
        return IOStatus::OpenFailed;
      }
#endif

      return IOStatus::Success;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus discover_metadata(Stream<IO, RandomTraits>* streams,
                                              StorageViewT& storage,
                                              MetadataInventory& inv) {
      auto* buf =
        storage.template acquire<MetadataRole, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
      hd_std::size_t buf_size { storage.template size<MetadataRole>() };

      IOStatus status = streams[Data].read_one(buf,
                                               storage.template size<MetadataRole>());

      if (status != IOStatus::Success) {
        storage.template release<MetadataRole, 0>(buf);

        return status;
      }

      const auto* blk0 { reinterpret_cast<const randfmt::Block*>(buf) };
      if (!blk0->valid_magic() || blk0->block_type() != randfmt::BlockType::Super) {
        storage.template release<MetadataRole, 0>(buf);

        return IOStatus::HeaderReadError;
      }

      const auto* sb0 { reinterpret_cast<const randfmt::SuperBlock*>(blk0->data()) };
      const auto* sub_blk { blk0->closest_block() };

      inv.count = 0;
      for (hd_std::uint8_t i = 0; i < sb0->num_blocks; ++i) {
        if (!sub_blk->valid_magic()) {
          break;
        }

        if (sub_blk->block_type() == randfmt::BlockType::Metadata) {
          const auto* meta { reinterpret_cast<const randfmt::MetadataBlock*>(sub_blk->data()) };

          inv.add_detector(meta->name,
                           meta->type,
                           meta->rank,
                           meta->shape,
                           static_cast<ncarray::DType>(meta->dtype));
        }

        sub_blk = sub_blk->closest_block();
      }

      storage.template release<MetadataRole, 0>(buf);
      return IOStatus::Success;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus index_stream(Stream<IO, RandomTraits>* streams,
                                         StorageViewT& storage,
                                         DiscoveryState& stream_state,
                                         const StreamParameters& cfg) {
      auto* idx_buf { storage.template acquire<IndexRole, 0, ncarray::HostTag>() };
      auto* event_offsets { reinterpret_cast<EventOffset*>(idx_buf) };
      hd_std::size_t stream_size { streams[Data].file_size() };
      hd_std::size_t event_count { 0 };

      // TODO: Double check... I think maybe right path. Need scratch buffer I think...
      if (cfg.indexing_mode == IndexingMode::IndexAll ||
          cfg.indexing_mode == IndexingMode::IndexBatch) {
        if (stream_size >= sizeof(randfmt::FileTrailer)) {
          randfmt::FileTrailer trailer {};
          hd_std::size_t trailer_offset { stream_size - sizeof(randfmt::FileTrailer) };

          IOStatus status = streams[Data].read_at(&trailer, trailer_offset, sizeof(trailer));
          if (status == IOStatus::Success) {
            if (hd_std::memcmp(trailer.magic_tail, randfmt::MagicTail, 8) == 0) {
              auto* meta_buf =
                storage.template acquire<MetadataRole, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
              hd_std::size_t meta_buf_size { storage.template size<MetadataRole>() };
              hd_std::size_t read_size { meta_buf_size };
              if (stream_size - trailer.idx_blk_offset < read_size) {
                read_size = stream_size - trailer.idx_blk_offset;
              }

              status = streams[Data].read_at(meta_buf,
                                             trailer.idx_blk_offset,
                                             read_size);

              const auto* super_blk { reinterpret_cast<const randfmt::Block*>(meta_buf) };
              if (status == IOStatus::Success &&
                  super_blk->block_type() == randfmt::BlockType::Super) {
                const auto* sub_blk { super_blk->closest_block() };
                if (sub_blk->block_type() == randfmt::BlockType::Index) {
                  const auto* idx_blk { reinterpret_cast<const randfmt::IndexBlock*>(sub_blk->data()) };
                  const auto* entries { reinterpret_cast<const randfmt::IndexEntry*>(idx_blk + 1) };

                  hd_std::size_t total_events { 0 };
                  if (idx_blk->num_super_blocks > 2) {
                    // Subtract off the config/meta and index super blocks at start and end
                    total_events = idx_blk->num_super_blocks - 2;
                  }

                  hd_std::size_t start_evt { 0 };
                  hd_std::size_t end_evt { total_events };
                  if (cfg.indexing_mode == IndexingMode::IndexBatch) {
                    // If reading in batches only index the chunk requested.
                    // NOTE: This is very wasteful ATM since everything is reread each time..
                    //       But... this is not supposed to be a high-performance format. Just testing...
                    start_evt = stream_state.num_events;
                    end_evt = cfg.indexing_batch_size;
                  }

                  for (hd_std::size_t i = start_evt; i < end_evt; ++i) {
                    event_offsets[i].offset = entries[i + 1].offset;
                    event_offsets[i].size = entries[i + 1].sb_size;
                  }

                  event_count = end_evt;
                  stream_state.num_events = event_count;

                  storage.template release<MetadataRole, 0>(meta_buf);
                  storage.template release<IndexRole, 0>(idx_buf);

                  return IOStatus::Success;
                }

                storage.template release<MetadataRole, 0>(meta_buf);
              }
            }
          }
        }
      }

      // IndexMode::NoIndex --> Always return the next step
      if (stream_state.curr_offset >= stream_size) {
        stream_state.num_events = 0;
        storage.template release<IndexRole, 0>(idx_buf);

        return IOStatus::AllRequestedRead;
      }

      // TODO: This is bad....
      randfmt::Block blk{};
      IOStatus status = streams[Data].read_at(&blk, stream_state.curr_offset, sizeof(blk));
      if (status != IOStatus::Success || !blk.valid_magic()) {
        stream_state.num_events = 0;
        storage.template release<IndexRole, 0>(idx_buf);

        return IOStatus::HeaderReadError;
      }

      if (blk.block_type() == randfmt::BlockType::Super) {
        randfmt::SuperBlock sb{};
        status = streams[Data].read_at(&sb,
                                       stream_state.curr_offset + sizeof(randfmt::Header),
                                       sizeof(sb));
        if (status != IOStatus::Success) {
          stream_state.num_events = 0;
          storage.template release<IndexRole, 0>(idx_buf);

          return IOStatus::GeneralIOError;
        }

        if (sb.sequence_num == 0) {
          stream_state.curr_offset += sizeof(randfmt::Header) + blk.hdr.payload_size;

          if (stream_state.curr_offset >= stream_size) {
            stream_state.num_events = 0;
            storage.template release<IndexRole, 0>(idx_buf);
            return IOStatus::AllRequestedRead;
          }

          status = streams[Data].read_at(&blk, stream_state.curr_offset, sizeof(blk));
        }
      }

      std::size_t event_sb_size { sizeof(randfmt::Block) + blk.payload_size() };
      event_offsets[0].offset = stream_state.curr_offset;
      event_offsets[0].size = event_sb_size;

      stream_state.curr_offset += event_sb_size;
      stream_state.num_events = 1;
      storage.template release<IndexRole, 0>(idx_buf);

      return IOStatus::Success;
    }

    SBIO_HD static auto sync_vars(DiscoveryState& state) {
      return make_sync_group(state.num_events);
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_step(Stream<IO, RandomTraits>* streams,
                                       StorageViewT& storage,
                                       DiscoveryState& stream_state,
                                       const StreamParameters& cfg,
                                       StepIdxType step_idx,
                                       DataAccessPtn ptn) {
      if (step_idx >= stream_state.num_events) {
        return IOStatus::AllRequestedRead;
      }

      auto* idx_buf { storage.template acquire<IndexRole, 0, ncarray::HostTag>() };
      const auto* event_offsets { reinterpret_cast<const EventOffset*>(idx_buf) };

      const auto& evt_off { event_offsets[step_idx] };
      std::size_t file_offset { evt_off.offset };
      std::size_t read_size { evt_off.size };

      auto* data_buf { storage.template acquire<DataRole, 0, ncarray::HostTag>() };
      IOStatus status = streams[Data].read_at(data_buf, file_offset, read_size);

      storage.template release<IndexRole, 0>(idx_buf);
      storage.template release<DataRole, 0>(data_buf);

      return status;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_multi_steps(Stream<IO, RandomTraits>* streams,
                                              StorageViewT& storage,
                                              DiscoveryState& stream_state,
                                              const StreamParameters& cfg,
                                              StepIdxType step_idx,
                                              StepIdxType count,
                                              DataAccessPtn ptn) {
      if (step_idx + count > stream_state.num_events) {
        return IOStatus::AllRequestedRead;
      }

      auto* idx_buf { storage.template acquire<IndexRole, 0, ncarray::HostTag>() };
      const auto* event_offsets { reinterpret_cast<const EventOffset*>(idx_buf) };
      const auto& start_off { event_offsets[step_idx] };
      const auto& end_off { event_offsets[step_idx + count - 1] };

      hd_std::size_t file_offset { start_off.offset };
      hd_std::size_t read_size { (end_off.offset + end_off.size) - file_offset };

      auto* data_buf { storage.template acquire<DataRole, 0, ncarray::HostTag>() };
      IOStatus status = streams[Data].read_at(data_buf, file_offset, read_size);

      storage.template release<IndexRole, 0>(idx_buf);
      storage.template release<DataRole, 0>(data_buf);

      return status;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_multi_steps_stride(Stream<IO, RandomTraits>* streams,
                                                     StorageViewT& storage,
                                                     DiscoveryState& stream_state,
                                                     const StreamParameters& cfg,
                                                     StepIdxType step_idx,
                                                     StepIdxType count,
                                                     StepIdxType stride,
                                                     DataAccessPtn ptn) {
      return IOStatus::FunctionUnavailable;
    }


    template <class StorageViewT>
    SBIO_HD static DataResult get_data_in_buffer(StorageViewT& storage,
                                                 const MetadataInventory& inv,
                                                 const DataRequest& req,
                                                 DataAccessPtn ptn,
                                                 std::size_t batch_idx = 0) {
      void* data_buf =
          storage.template acquire<DataRole, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);

      const char* ptr { reinterpret_cast<const char*>(data_buf) };
      for (hd_std::size_t b = 0; b < batch_idx; ++b) {
        const auto* blk { reinterpret_cast<const randfmt::Block*>(ptr) };
        ptr += sizeof(randfmt::Header) + blk->payload_size();
      }

      DataResult res { resolve_data(const_cast<char*>(ptr), inv, req) };
      res.data = storage.template release<DataRole, 0>(data_buf, res.data);

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
      auto* buf { storage.template acquire<DataRole, 0, ncarray::HostTag>() };
      storage.template release<DataRole, 0>(buf);

      return buf;
    }

    SBIO_HD static DataResult resolve_data(void* buffer,
                                           const MetadataInventory& inv,
                                           const DataRequest& req) {
      DataResult res{};
      const auto* super_blk { reinterpret_cast<const randfmt::Block*>(buffer) };
      if (!super_blk->valid_magic() || super_blk->block_type() != randfmt::BlockType::Super) {
        return res;
      }

      const auto* sb_payload { reinterpret_cast<const randfmt::SuperBlock*>(super_blk->data()) };
      const MetadataInventory::Entry* entry { nullptr };
      hd_std::uint8_t det_idx { 0 };
      for (hd_std::size_t i = 0; i < inv.count; ++i) {
        if (hd_std::strcmp(inv.entries[i].name, req.name) == 0) {
          entry = &inv.entries[i];
          det_idx = static_cast<hd_std::uint8_t>(i);
          break;
        }
      }

      if (!entry) {
        return res;
      }

      if (entry->offset_in_event) {
        const auto* blk =
          reinterpret_cast<const randfmt::Block*>(super_blk->data() + entry->offset_in_event);
        res.data = blk->data();
        res.size = entry->size;
        res.rank = entry->rank;
        for (hd_std::uint16_t r= 0; r < entry->rank; ++r) {
          res.shape[r] = entry->shape[r];
        }
        res.dtype = entry->dtype;

        return res;
      }

      if (super_blk->test_flag(randfmt::FormatFlags::SubBlockOffsetTable)) {
        const hd_std::uint32_t* sub_offsets { sb_payload->offsets() };

        hd_std::uint32_t blk_offset { sub_offsets[det_idx] };
        entry->offset_in_event = static_cast<hd_std::size_t>(blk_offset);

        const char* sb_payload_start { super_blk->data() };
        const auto* blk =
          reinterpret_cast<const randfmt::Block*>(sb_payload_start + blk_offset);

        res.data = blk->data();
        res.size = entry->size;
        res.rank = entry->rank;
        for (hd_std::uint16_t r = 0; r < entry->rank; ++r) {
          res.shape[r] = entry->shape[r];
        }
        res.dtype = entry->dtype;

        return res;
      }

      const auto* sub_blk { super_blk->closest_block() };
      for (hd_std::uint8_t b = 0; b < sb_payload->num_blocks; ++b) {
        if (!sub_blk->valid_magic()) {
          break;
        }

        if (sub_blk->block_type() == randfmt::BlockType::Data && sub_blk->block_id() == det_idx) {
          res.data = sub_blk->data();
          res.size = entry->size;
          res.rank = entry->rank;
          for (hd_std::uint16_t r = 0; r < entry->rank; ++r) {
            res.shape[r] = entry->shape[r];
          }
          res.dtype = entry->dtype;

          hd_std::size_t blk_offset =
            static_cast<hd_std::size_t>(reinterpret_cast<const char*>(sub_blk) - super_blk->data());
          entry->offset_in_event = blk_offset;

          return res;
        }

        sub_blk = sub_blk->closest_block();
      }
      return res;
    }
    SBIO_HD static inline std::size_t get_payload_size(void* buf) {
      return reinterpret_cast<randfmt::Block*>(buf)->payload_size();
    }
  };
} // namespace sbio

#endif // SBIO_FORMATS_RANDOM_RANDOM_TRAITS_HH
