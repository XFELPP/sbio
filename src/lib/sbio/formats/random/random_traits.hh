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

#endif

#ifdef _WIN32
#include <BaseTsd.h>
#include <windows.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
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

    struct StreamParameters {
      hd_std::size_t num_events { 100000 };
      hd_std::size_t event_size { 0x100000 };
      hd_std::uint32_t seed { 42 };
      hd_std::uint32_t pattern_type { 0 }; ///< 0 = PNRG, 1 = Sequential, 2 = Fixed fill
      bool enable_subblock_offsets { true };
      bool enable_superblock_offsets { true };

#ifdef _WIN32
      HANDLE h_file;
#else
      int fd;
#endif
    };

    struct SBIO_API DataSourceParameters {
      struct DetectorSpec {
        char name[MaxNameSize];
        char type[MaxNameSize];
        hd_std::uint16_t rank { 2 };
        hd_std::uint32_t shape[MaxRank] { 1024, 1024, 0, 0, 0 };
        ncarray::DType dtype { ncarray::DType::uint16 };
        hd_std::uint16_t num_streams { 1 }; ///< How many streams to distribute across
      };

      DetectorSpec detectors[10];
      hd_std::size_t num_detectors { 0 };

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
      for (hd_std::size_t d = 0; d < ds_params.num_detectors; ++d) {
        DataSourceParameters::DetectorSpec spec { ds_params.detectors[d] };
        hd_std::size_t streams_per_det { spec.num_streams > 0 ? spec.num_streams : 1 };

        for (hd_std::size_t s = 0; s < streams_per_det; ++s) {
          int cnt = snprintf(name_buf, MaxNameSize, "sbio_random_stream_%zu", nstream);

          StreamParameters stream_cfg = base_cfg;
          randfmt::Header hdr{};
          hd_std::memcpy(hdr.magic, randfmt::MagicBytes, 8);
          hdr.block_type = static_cast<hd_std::uint8_t>(randfmt::BlockType::Metadata);
          hdr.block_id = static_cast<hd_std::uint8_t>(d);
          hdr.payload_size = sizeof(randfmt::MetadataBlock);

          randfmt::MetadataBlock meta_blk;
          safe_strncpy(meta_blk.name, spec.name, MaxNameSize);
          safe_strncpy(meta_blk.type, spec.type, MaxNameSize);
          meta_blk.rank = spec.rank;
          for (hd_std::size_t i = 0; i < spec.rank; ++i) {
            meta_blk.shape[i] = spec.shape[i];
          }
          meta_blk.dtype = static_cast<hd_std::uint16_t>(spec.dtype);

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

          auto write_bytes = [h_file](const void* buf,
                                      hd_std::size_t sz,
                                      hd_std::uint64_t off) {
            OVERLAPPED ov {};
            ov.Offset = static_cast<DWORD>(off);
            ov.OffsetHigh = static_cast<DWORD>(off >> 32);
            DWORD written { 0 };
            WriteFile(h_file, buf, static_cast<DWORD>(sz), &written, &ov);
          };

          stream_cfg.h_file = h_file;
#else
          int fd = memfd_create(name_buf, 0);
          if (fd < 0) {
            return false;
          }

          auto write_bytes = [fd](const void* buf, std::size_t sz, std::uint64_t off) {
            ::pwrite(fd, buf, sz, static_cast<off_t>(off));
          };

          stream_cfg.fd = fd;
#endif

          hd_std::uint64_t curr_offset { 0 };

          std::uint64_t sb0_hdr_offset { curr_offset };
          std::uint8_t sb0_num_sub_blocks  { 2 };

          hd_std::size_t config_sub_bytes { sizeof(randfmt::Header) + sizeof(randfmt::ConfigBlock) };
          hd_std::size_t meta_sub_bytes { sizeof(randfmt::Header) + sizeof(randfmt::MetadataBlock) };
          hd_std::size_t sb0_offsets_bytes { 0 };
          if (stream_cfg.enable_subblock_offsets) {
            sb0_offsets_bytes = sb0_num_sub_blocks * sizeof(hd_std::uint32_t);
          }

          hd_std::uint32_t sb0_payload_size =
            static_cast<hd_std::uint32_t>(sizeof(randfmt::SuperBlock) +
                                          sb0_offsets_bytes +
                                          config_sub_bytes +
                                          meta_sub_bytes);

          randfmt::Header sb0_hdr{};
          hd_std::memcpy(sb0_hdr.magic, randfmt::MagicBytes, 8);
          sb0_hdr.block_type = static_cast<hd_std::uint8_t>(randfmt::BlockType::Super);
          sb0_hdr.payload_size = sb0_payload_size;

          write_bytes(&sb0_hdr, sizeof(sb0_hdr), curr_offset);

          curr_offset += sizeof(sb0_hdr);
          hd_std::uint64_t sb0_payload_start { curr_offset };

          randfmt::SuperBlock sb0{};
          sb0.sequence_num = 0; // Start counting at 0, just increment by 1 for now...
          sb0.num_blocks = sb0_num_sub_blocks;
          if (stream_cfg.enable_subblock_offsets) {
            sb0.flags = 1;
          } else {
            sb0.flags = 0;
          }

          write_bytes(&sb0, sizeof(sb0), curr_offset);
          curr_offset += sizeof(sb0);

          hd_std::uint64_t sb0_offsets_pos = curr_offset;
          if (stream_cfg.enable_superblock_offsets) {
            curr_offset += sb0_offsets_bytes;
          }

          std::vector<std::uint32_t> sb0_rel_offsets;

          sb0_rel_offsets.push_back(static_cast<hd_std::uint32_t>(curr_offset - sb0_payload_start));

          randfmt::Header cfg_hdr{};
          hd_std::memcpy(cfg_hdr.magic, randfmt::MagicBytes, 8);
          cfg_hdr.block_type = static_cast<hd_std::uint8_t>(randfmt::BlockType::Config);
          cfg_hdr.payload_size = sizeof(randfmt::ConfigBlock);

          randfmt::ConfigBlock cfg_blk{};
          cfg_blk.seed = stream_cfg.seed;
          cfg_blk.pattern_type = stream_cfg.pattern_type;
          cfg_blk.total_events = stream_cfg.num_events;
          cfg_blk.flags = 0;
          if (stream_cfg.enable_subblock_offsets) {
            cfg_blk.flags |= 1;
          }
          if (stream_cfg.enable_superblock_offsets) {
            cfg_blk.flags |= 2;
          }

          write_bytes(&cfg_hdr, sizeof(cfg_hdr), curr_offset);
          curr_offset += sizeof(cfg_hdr);

          write_bytes(&cfg_blk, sizeof(cfg_blk), curr_offset);
          curr_offset += sizeof(cfg_blk);

          sb0_rel_offsets.push_back(static_cast<hd_std::uint32_t>(curr_offset - sb0_payload_start));

          randfmt::Header meta_hdr{};
          hd_std::memcpy(meta_hdr.magic, randfmt::MagicBytes, 8);
          meta_hdr.block_type = static_cast<hd_std::uint8_t>(randfmt::BlockType::Metadata);
          meta_hdr.block_id = static_cast<hd_std::uint8_t>(d);
          meta_hdr.payload_size = sizeof(randfmt::MetadataBlock);

          randfmt::MetadataBlock meta_blk{};
          safe_strncpy(meta_blk.name, spec.name, MaxNameSize);
          safe_strncpy(meta_blk.type, spec.type, MaxNameSize);
          meta_blk.rank = spec.rank;
          for (hd_std::size_t r = 0; r < spec.rank; ++r) {
            meta_blk.shape[r] = spec.shape[r];
          }
          meta_blk.dtype = static_cast<std::uint16_t>(spec.dtype);

          write_bytes(&meta_hdr, sizeof(meta_hdr), curr_offset);
          curr_offset += sizeof(meta_hdr);

          write_bytes(&meta_blk, sizeof(meta_blk), curr_offset);
          curr_offset += sizeof(meta_blk);

          if (stream_cfg.enable_superblock_offsets) {
            write_bytes(sb0_rel_offsets.data(), sb0_offsets_bytes, sb0_offsets_pos);
          }

          std::vector<char> data_payload(stream_cfg.event_size, 0x5A);

          // For now, just have 1 data per SuperBlock
          hd_std::uint8_t event_num_sub_blocks { 1 };
          hd_std::size_t data_sub_bytes { sizeof(randfmt::Header) + stream_cfg.event_size };
          hd_std::size_t event_offsets_bytes { 0 };
          if (stream_cfg.enable_subblock_offsets) {
            event_offsets_bytes = event_num_sub_blocks * sizeof(hd_std::uint32_t);
          }

          hd_std::uint32_t event_sb_payload_size =
            static_cast<hd_std::uint32_t>(sizeof(randfmt::SuperBlock) +
                                          event_offsets_bytes +
                                          data_sub_bytes);

          for (hd_std::size_t ev = 0; ev < stream_cfg.num_events; ++ev) {
            hd_std::uint64_t sb_hdr_offset { curr_offset };

            randfmt::Header sb_hdr{};
            hd_std::memcpy(sb_hdr.magic, randfmt::MagicBytes, 8);
            sb_hdr.block_type = static_cast<hd_std::uint8_t>(randfmt::BlockType::Super);
            sb_hdr.payload_size = event_sb_payload_size;

            write_bytes(&sb_hdr, sizeof(sb_hdr), curr_offset);
            curr_offset += sizeof(sb_hdr);

            hd_std::uint64_t sb_payload_start { curr_offset };

            randfmt::SuperBlock sb{};
            sb.sequence_num = ev + 1; // Just increment by 1 for now...
            sb.num_blocks = event_num_sub_blocks;
            sb.flags = 0;
            if (stream_cfg.enable_subblock_offsets) {
              sb.flags = 1;
            }

            write_bytes(&sb, sizeof(sb), curr_offset);
            curr_offset += sizeof(sb);

            hd_std::uint64_t offsets_pos { curr_offset };
            if (stream_cfg.enable_superblock_offsets) {
              curr_offset += event_offsets_bytes;
            }

            std::vector<hd_std::uint32_t> rel_offsets;
            rel_offsets.push_back(static_cast<hd_std::uint32_t>(curr_offset - sb_payload_start));

            // Data
            randfmt::Header data_hdr{};
            hd_std::memcpy(data_hdr.magic, randfmt::MagicBytes, 8);
            data_hdr.block_type = static_cast<hd_std::uint8_t>(randfmt::BlockType::Data);
            data_hdr.block_id = static_cast<hd_std::uint8_t>(d);
            data_hdr.payload_size = static_cast<hd_std::uint32_t>(stream_cfg.event_size);

            write_bytes(&data_hdr, sizeof(data_hdr), curr_offset);
            curr_offset += sizeof(data_hdr);

            write_bytes(data_payload.data(), data_payload.size(), curr_offset);
            curr_offset += data_payload.size();

            // Offset tables
            if (stream_cfg.enable_subblock_offsets) {
              write_bytes(rel_offsets.data(), event_offsets_bytes, offsets_pos);
            }

            randfmt::IndexEntry ie{};
            ie.sequence_num = ev + 1;
            ie.file_offset = sb_hdr_offset;
            ie.total_size = sizeof(randfmt::Header) + event_sb_payload_size;
            index_entries.push_back(ie);
          }

          // Deal with the final trailer block, and the SuperBlock offset table (if enabled)
          if (stream_cfg.enable_tail_index) {
            hd_std::uint64_t index_hdr_offset = curr_offset;

            randfmt::Header idx_hdr{};
            hd_std::memcpy(idx_hdr.magic, randfmt::MagicBytes, 8);
            idx_hdr.block_type = static_cast<hd_std::uint8_t>(randfmt::BlockType::Index);
            idx_hdr.payload_size =
              static_cast<hd_std::uint32_t>(sizeof(randfmt::IndexBlock) +
                                            (index_entries.size() * sizeof(randfmt::IndexEntry)));

            randfmt::IndexBlock idx_blk{};
            idx_blk.num_super_blocks = index_entries.size();

            write_bytes(&idx_hdr, sizeof(idx_hdr), curr_offset);
            curr_offset += sizeof(idx_hdr);

            write_bytes(&idx_blk, sizeof(idx_blk), curr_offset);
            curr_offset += sizeof(idx_blk);

            write_bytes(index_entries.data(),
                        index_entries.size() * sizeof(randfmt::IndexEntry),
                        curr_offset);
            curr_offset += index_entries.size() * sizeof(randfmt::IndexEntry);

            randfmt::FileTrailer trailer{};
            trailer.index_offset = index_hdr_offset;
            hd_std::memcpy(trailer.magic_tail, randfmt::TrailerMagicBytes, 8);

            write_bytes(&trailer, sizeof(trailer), curr_offset);
          }

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
      std::size_t events_per_read { 0 };                          ///< Number of offsets to read per indexing
      std::size_t num_events { 0 };                               ///< Number of event (offset)s read
      DataAccessPtn last_accessed_ptn { DataAccessPtn::Default }; ///< Indicate last buffer used
    };

    struct SBIO_API DataRequest {
      DataRequest() = default;

      DataRequest(const DataRequest& other) = default;
      DataRequest& operator=(const DataRequest& other) = default;
      DataRequest(DataRequest&& other) noexcept = default;
      DataRequest& operator=(DataRequest&& other) noexcept = default;

      DataRequest(const char* name_, const char* type_) {
        safe_strncpy(detector_name, name, MaxNameSize);
        safe_strncpy(detector_type, type, MaxNameSize);
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
        hd_std::size_t offset_in_event; // Byte offset in the flat event buffer
        hd_std::size_t size;            // Total bytes for this detector's data
      };

      Entry entries[16];
      hd_std::size_t count { 0 };
    };

    struct DataResult {
      const void* data;
      hd_std::size_t size;
      hd_std::uint16_t rank;
      const hd_std::uint32_t* shape;
      hd_std::uint32_t dtype;
    };

    // Defined at end of the header, after MetadataInventory is completed
    template <class DataBrokerType, class SegmentRef>
    SBIO_HD static std::size_t find_group_segments(const MetadataInventory& inv,
                                                   const char* name,
                                                   SegmentRef* ref_out,
                                                   std::size_t max_out,
                                                   DataBrokerType* broker,
                                                   char* dettype = nullptr,
                                                   DataAccessPtn ptn = DataAccessPtn::Default);

    SBIO_HD static AllocationRequest<RandomTraits> get_allocation_request(StreamParameters& cfg) {
      AllocationRequest<RandomTraits> request;

      return request;
    }

    SBIO_HD static std::size_t max_batch_count(StreamParameters& cfg) {
      return cfg.max_dgram_batch;
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
        if (!sub_block->valid_magic()) {
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
      std::size_t stream_size { streams[Data].file_size() };
      std::size_t event_count { 0 };

      // TODO: Double check... I think maybe right path. Need scratch buffer I think...
      if (stream_size >= sizeof(randfmt::FileTrailer)) {
        randfmt::FileTrailer trailer {};
        std::size_t trailer_offset { stream_size - sizeof(randfmt::FileTrailer) };

        IOStatus status = streams[Data].read_at(&trailer, trailer_offset, sizeof(trailer));
        if (status == IOStatus::Success) {
          if (hd_std::memcmp(trailer.magic_tail, randfmt::MagicTail, 8) == 0) {
            auto* meta_buf =
              storage.template acquire<MetadataRole, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);
            std::size_t meta_buf_size { storage.template size<MetadataRole>() };

            status = streams[Data].read_at(meta_buf,
                                           trailer.idx_blk_offset,
                                           sizeof(randfmt::Block));

            const auto* blk { reinterpret_cast<const Block*>(meta_buf) };
            if (status == IOStatus::Success && blk->block_type() == randfmt::BlockType::Index) {
              status = streams[Data].read_at(blk->data(),
                                             trailer.idx_blk_offset + sizeof(randfmt::Header),
                                             blk->payload_size());

              if (payload_size <= meta_buf_size && status == IOStatus::Success) {
                const auto* idx_blk { reinterpret_cast<const randfmt::IndexBlock*>(blk->data()) };
                const auto* entries { reinterpret_cast<const randfmt::IndexEntry*>(idx_blk + 1) };

                for (std::size_t i = 0; i < idx_blk->num_super_blocks; ++i) {
                  event_offsets[i].offset = entries[i].offset;
                  event_offsets[i].size = entries[i].sb_size;
                }

                event_count = idx_blk->num_super_blocks;
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
          stream_state.curr_offset += sizeof(randfmt::Header) + hdr.payload_size;

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
      return make_sync_group(state.num_events, state.num_transitions);
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_step(Stream<IO, RandomTraits>* streams,
                                       StorageViewT& storage,
                                       DiscoveryState& stream_state,
                                       const StreamParameters& cfg,
                                       StepIdxType step_idx,
                                       DataAccessPtn ptn) {

    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus fetch_multi_steps(Stream<IO, RandomTraits>* streams,
                                              StorageViewT& storage,
                                              DiscoveryState& stream_state,
                                              const StreamParameters& cfg,
                                              StepIdxType step_idx,
                                              StepIdxType count,
                                              DataAccessPtn ptn) {

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
    }


    template <class StorageViewT>
    SBIO_HD static DataResult get_data_in_buffer(StorageViewT& storage,
                                                 const MetadataInventory& inv,
                                                 const DataRequest& req,
                                                 DataAccessPtn ptn,
                                                 std::size_t batch_idx = 0) {
    }

    template <class StorageViewT>
    SBIO_HD static auto capacity(const StorageViewT& storage,
                                 const DiscoveryState& state) {
    }

    template <class StorageViewT>
    SBIO_HD static auto current_buffer(StorageViewT& storage,
                                       const DiscoveryState& state) {
    }

    SBIO_HD static DataResult resolve_data(void* buffer,
                                           const MetadataInventory& inv,
                                           const DataRequest& req) {
    }
    SBIO_HD static inline std::size_t get_payload_size(void* buf) {
      return reinterpret_cast<randfmt::Block*>(buf)->payload_size();
    }
  };
} // namespace sbio

#endif // SBIO_FORMATS_RANDOM_RANDOM_TRAITS_HH
