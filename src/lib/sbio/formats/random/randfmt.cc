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

#include "sbio/formats/random/randfmt.hh"

#include "sbio/util/string.hh"

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
#include <BaseTsd.h>
#include <windows.h>
typedef SSIZE_T ssize_t;
#else
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace sbio::randfmt {
#ifdef _WIN32
  using FileHandle = HANDLE;

  void write_bytes(FileHandle h_file,
                   const void* buf,
                   hd_std::size_t sz,
                   hd_std::uint64_t off) {
    OVERLAPPED ov {};
    ov.Offset = static_cast<DWORD>(off);
    ov.OffsetHigh = static_cast<DWORD>(off >> 32);

    DWORD written { 0 };
    WriteFile(h_file, buf, static_cast<DWORD>(sz), &written, &ov);
  }
#else
  using FileHandle = int;

  void write_bytes(FileHandle fd,
                   const void* buf,
                   hd_std::size_t sz,
                   hd_std::uint64_t off) {
    ::pwrite(fd, buf, sz, static_cast<off_t>(off));
  };
#endif // _WIN32

  void write_super_block(FileHandle f_handle,
                         hd_std::uint64_t seq_no,
                         hd_std::uint8_t flags,
                         hd_std::uint8_t num_sub_blocks,
                         BlockType* block_types,
                         hd_std::uint8_t* block_ids,
                         void** payloads,
                         hd_std::uint32_t* sizes,
                         hd_std::uint64_t& curr_offset) {
    Header super_hdr {
      { 'S', 'B', 'I', 'O', 'R', 'N', 'D', '0' },
      static_cast<hd_std::uint8_t>(BlockType::Super),
      0,
      flags
    };

    hd_std::uint32_t sb_payload_size {
      static_cast<hd_std::uint32_t>(sizeof(SuperBlock) + num_sub_blocks * sizeof(Header))
    };
    if (test_feature_flag(flags, FormatFlags::SubBlockOffsetTable)) {
      sb_payload_size += num_sub_blocks * sizeof(hd_std::uint32_t);
    }
    // Need to add the size of the *payload* of each sub-Block now.
    for (hd_std::size_t b = 0; b < num_sub_blocks; ++b) {
      sb_payload_size += sizes[b];
    }
    super_hdr.payload_size = sb_payload_size;

    // Ignore checksum for now... super_hdr.checksum
    write_bytes(f_handle, &super_hdr, sizeof(super_hdr), curr_offset);
    curr_offset += sizeof(super_hdr);

    // Not currently exposing the `user` field to write here.
    SuperBlock sb_payload { seq_no, num_sub_blocks };

    // In case there are sub-Block offsets, track relative offsets.
    // There are maximally 255 sub-Blocks. Still wasteful though...
    hd_std::uint32_t sub_blk_offsets[255];
    hd_std::uint64_t sb_payload_start { curr_offset };

    write_bytes(f_handle, &sb_payload, sizeof(sb_payload), curr_offset);
    curr_offset += sizeof(sb_payload);

    // We'll skip ahead, and then fill in afterwards to avoid looping twice.
    if (test_feature_flag(flags, FormatFlags::SubBlockOffsetTable)) {
      curr_offset += num_sub_blocks * sizeof(hd_std::uint32_t);
    }

    for (hd_std::size_t b = 0; b < num_sub_blocks; ++b) {
      // Store sub-Block offset in case its used
      sub_blk_offsets[b] = static_cast<hd_std::uint32_t>(curr_offset - sb_payload_start);
      hd_std::uint32_t blk_payload_size { sizes[b] };

      Header hdr {
        { 'S', 'B', 'I', 'O', 'R', 'N', 'D', '0' },
        static_cast<hd_std::uint8_t>(block_types[b]),
        block_ids[b],
        flags,
        blk_payload_size
      };

      write_bytes(f_handle, &hdr, sizeof(hdr), curr_offset);
      curr_offset += sizeof(hdr);

      write_bytes(f_handle, payloads[b], blk_payload_size, curr_offset);
      curr_offset += blk_payload_size;
    }

    // Go back and write the offsets if needed.
    if (test_feature_flag(flags, FormatFlags::SubBlockOffsetTable)) {
      write_bytes(f_handle,
                  sub_blk_offsets,
                  num_sub_blocks * sizeof(hd_std::uint32_t),
                  sb_payload_start + sizeof(SuperBlock));
    }
  }

  // 0 = PNRG, 1 = Sequential, 2 = Fixed
  void construct_data_block(hd_std::uint32_t size,
                            hd_std::uint8_t pattern_type,
                            hd_std::uint8_t*& payload) {
    if (payload == nullptr) {
      payload = new hd_std::uint8_t[size];
    }

    for (hd_std::uint32_t i = 0; i < size; ++i) {
      if (pattern_type == 0) {
        
      } else if (pattern_type == 1) {
        
      } else {
        payload[i] = 0x5A;
      }
    }
  }

  void construct_cfg_block(hd_std::uint32_t seed,
                           hd_std::uint8_t pattern_type,
                           hd_std::uint64_t total_events,
                           hd_std::uint8_t flags,
                           hd_std::uint8_t*& payload) {
    if (payload == nullptr) {
      payload = new hd_std::uint8_t[sizeof(ConfigBlock)];
    }

    new (payload) ConfigBlock { seed, pattern_type, total_events, flags };
  }

  void construct_meta_block(hd_std::uint8_t num_detectors,
                            const DetectorSpec* detector_specs,
                            hd_std::uint8_t*& payload) {
    if (payload == nullptr) {
      payload = new hd_std::uint8_t[sizeof(MetadataBlock) * num_detectors];
    }

    hd_std::size_t offset { 0 };
    for (hd_std::uint8_t d = 0; d < num_detectors; ++d) {
      auto* blk = new (payload + offset) MetadataBlock;

      const auto& spec { detector_specs[d] };

      safe_strncpy(blk->name, spec.name, MaxNameSize);
      safe_strncpy(blk->type, spec.type, MaxNameSize);
      blk->rank = spec.rank;
      for (hd_std::uint16_t r = 0; r < spec.rank; ++r) {
        blk->shape[r] = spec.shape[r];
      }
      blk->dtype = static_cast<hd_std::uint16_t>(spec.dtype);

      offset += sizeof(MetadataBlock);
    }
  }

  void write_sbiornd_file(FileHandle f_handle,
                          hd_std::uint64_t& curr_offset,
                          hd_std::uint8_t num_detectors,
                          const DetectorSpec* detector_specs,
                          hd_std::uint8_t* det_block_ids,
                          hd_std::uint32_t seed,
                          hd_std::uint8_t pattern_type,
                          hd_std::uint64_t total_events,
                          hd_std::uint8_t flags) {

    hd_std::uint64_t start_pt { curr_offset };

    hd_std::uint64_t seq_no { 0 };

    // Eh... just be wasteful. Can only have 255 sub-Blocks
    BlockType block_types[255];
    void* payloads[255];
    hd_std::uint32_t payload_sizes[255];

    hd_std::uint8_t* cfg_payload { nullptr };
    hd_std::uint8_t* meta_payload { nullptr };

    construct_cfg_block(seed, pattern_type, total_events, flags, cfg_payload);
    construct_meta_block(num_detectors, detector_specs, meta_payload);

    block_types[0] = BlockType::Config;
    payload_sizes[0] = sizeof(ConfigBlock);
    payloads[0] = reinterpret_cast<void*>(cfg_payload);
    for (hd_std::uint8_t d = 0; d < num_detectors; ++d) {
      block_types[d + 1] = BlockType::Metadata;
      payload_sizes[d + 1] = sizeof(MetadataBlock);
      payloads[d + 1] = reinterpret_cast<void*>(meta_payload + (d * sizeof(MetadataBlock)));
    }

    write_super_block(f_handle,
                      seq_no,
                      flags,
                      num_detectors + 1,
                      block_types,
                      det_block_ids,
                      payloads,
                      payload_sizes,
                      curr_offset);

    delete[] cfg_payload;
    delete[] meta_payload;

    hd_std::uint32_t total_det_bytes { 0 };
    for (hd_std::uint8_t d = 0; d < num_detectors; ++d) {
      const auto& spec { detector_specs[d] };

      hd_std::uint32_t det_bytes { static_cast<hd_std::uint32_t>(ncarray::itemsize(spec.dtype)) };
      for (hd_std::uint16_t r = 0; r < spec.rank; ++r) {
        det_bytes *= spec.shape[r];
      }

      payload_sizes[d] = det_bytes;
      total_det_bytes += det_bytes;
    }

    // Starting point minus current offset gives the config/meta SuperBlock size
    hd_std::uint32_t meta_sb_size { static_cast<hd_std::uint32_t>(curr_offset - start_pt) };

    // Assume all data payloads are same size right now. So can calculate where
    // offset table will be at the end.
    // Start with the SuperBlock header bytes
    hd_std::uint32_t data_sb_size { sizeof(Header) + sizeof(SuperBlock) };
    if (test_feature_flag(flags, FormatFlags::SubBlockOffsetTable)) {
      data_sb_size += num_detectors * sizeof(hd_std::uint32_t);
    }
    // Add the bytes for all data payloads + (num_detectors * sizeof(Header))
    data_sb_size += total_det_bytes + (num_detectors * sizeof(Header));
    // Final SuperBlock has the IndexBlock.
    hd_std::uint64_t idx_blk_offset { curr_offset + (total_events * data_sb_size) };

    // Total SuperBlocks = 1 Config/Metadata + N data + 1 IndexBlock at the end.
    hd_std::uint64_t num_super_blocks { total_events + 1 + 1 };
    if (test_feature_flag(flags, FormatFlags::SuperBlockOffsetTable)) {
      // Terribly inefficient... but go back and forth writing the offset.
      // Write just the Config/MetadataBlock entry now.
      hd_std::uint8_t idx_payload[sizeof(IndexBlock) + sizeof(IndexEntry)];

      new (idx_payload) IndexBlock { num_super_blocks, 0 };
      new (idx_payload + sizeof(IndexBlock)) IndexEntry { seq_no, 0, meta_sb_size };

      hd_std::uint32_t total_idx_sb_size {
        static_cast<hd_std::uint32_t>(sizeof(IndexBlock) + (num_super_blocks * sizeof(IndexEntry)))
      };

      payloads[0] = reinterpret_cast<void*>(idx_payload);
      block_types[0] = BlockType::Index;
      hd_std::uint8_t idx_block_id { 0 };
      write_super_block(f_handle,
                        0,
                        flags,
                        1, // 1 sub-block -> the IndexBlock
                        block_types,
                        &idx_block_id,
                        reinterpret_cast<void**>(&idx_payload),
                        &total_idx_sb_size,
                        idx_blk_offset);
      // After this, no more super-blocks -> Will just append IndexEntry directly.
    }

    hd_std::uint8_t* data_payload { new hd_std::uint8_t[total_det_bytes] };

    for (hd_std::uint64_t evt = 0; evt < total_events; ++evt) {
      hd_std::uint32_t offset { 0 };
      for (hd_std::uint8_t d = 0; d < num_detectors; ++d) {
        block_types[d] = BlockType::Data;
        auto* blk_data_ptr { data_payload + offset };
        construct_data_block(payload_sizes[d], pattern_type, blk_data_ptr);
        payloads[d] = data_payload + offset;

        offset += payload_sizes[d];
      }

      hd_std::uint64_t evt_start { curr_offset }; // If recording IndexEntrys
      write_super_block(f_handle,
                        seq_no,
                        flags,
                        num_detectors,
                        block_types,
                        det_block_ids,
                        payloads,
                        payload_sizes,
                        curr_offset);

      if (test_feature_flag(flags, FormatFlags::SuperBlockOffsetTable)) {
        // idx_blk_offset was incremented when the initial IndexBlock SuperBlock was
        // setup above. So it points to the next place to write an IndexEntry
        IndexEntry entry { seq_no, (evt_start - start_pt), data_sb_size };

        write_bytes(f_handle, &entry, sizeof(IndexEntry), idx_blk_offset);
        idx_blk_offset += sizeof(IndexEntry);
      }

      // For now, just increment the sequence number by 1 for each SuperBlock
      seq_no++;
    }

    // idx_blk_offset was incremented if writing the SuperBlock offsets.
    // If NOT using SuperBlock offsets, that value is meaningless anyway.
    FileTrailer trailer {
      (idx_blk_offset - (sizeof(IndexEntry) * total_events) - sizeof(IndexBlock)),
      { 'S', 'B', 'I', 'O', 'T', 'A', 'I', 'L' }
    };
    write_bytes(f_handle, &trailer, sizeof(FileTrailer), curr_offset);
    curr_offset += sizeof(FileTrailer);

    delete [] data_payload;
  }
} // namespace sbio::randfmt
