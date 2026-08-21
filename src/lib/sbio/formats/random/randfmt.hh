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

#ifndef SBIO_FORMATS_RANDOM_RANDFMT_HH
#define SBIO_FORMATS_RANDOM_RANDFMT_HH

#include <ncarray/dtype.hh>

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

namespace sbio::randfmt {
  /**
   * The maximum number of dimensions a data payload may have.
   */
  static constexpr hd_std::uint16_t MaxRank { 5 };

  /**
   * The maximum number of bytes for string/character payloads.
   */
  static constexpr hd_std::uint16_t MaxNameSize { 64 };

  /**
   * Magic byte start sequence: SBIORND0.
   *
   * Last byte (currently `0`) provides a version identifier.
   *
   * Regardless of any additional data, all blocks for the random format must begin
   * with the magic bytes.
   */
  static constexpr char MagicBytes[8] { 'S', 'B', 'I', 'O', 'R', 'N', 'D', '0' };

  /**
   * Magic byte tail sequence: SBIOTAIL.
   *
   * The final bytes are always the tail sequence. Preceeding this, a trailer includes
   * a byte offset of the IndexBlock (if that feature was enabled during writing).
   */
  static constexpr char MagicTail[8] { 'S', 'B', 'I', 'O', 'T', 'A', 'I', 'L' };

  enum class BlockType : hd_std::uint8_t {
    Super    = 0, ///< Block defining a group of upcoming blocks
    Config   = 1, ///< Block to define global configuration (like PRNG seed)
    Metadata = 2, ///< Block containing metadata for interpreting later Data blocks
    Data     = 3, ///< Block containing raw data payloads
    Index    = 4  ///< Block to write SuperBlock indices (if enabled)
  };

  /**
   * File format flags.
   *
   * Some flags control features at the granularity of super-blocks, some at the block
   * level, and some at the full stream level. They are always written to Config block
   * payloads, though, as well as into the Header of every block.
   */
  enum class FormatFlags : hd_std::uint8_t {
    SubBlockOffsetTable   = 0, ///< Each SuperBlock will write an offset table to jump within the SuperBlock.
    SuperBlockOffsetTable = 1  ///< Write an offset-table to each SuperBlock at the end
  };

  SBIO_HD inline bool test_feature_flag(hd_std::uint8_t flags, FormatFlags feat) {
    return flags & (1 << static_cast<hd_std::uint8_t>(feat));
  }

#pragma pack(push, 4)
  struct Header {
    char magic[8];                 ///< Magic bytes must always match
    hd_std::uint8_t block_type;    ///< Block type - payloads, or metadata
    hd_std::uint8_t block_id;      ///< Identifier to associate block to name/type
    hd_std::uint8_t flags;         ///< Feature flags
    hd_std::uint32_t payload_size; ///< Total size of the upcoming payload
    hd_std::uint32_t checksum;     ///< Corruption checksum
    hd_std::uint32_t _reserved;    ///< Reserved for future use
  };

  // --- Payload descriptors inside the relevant blocks --- //
  /**
   * The payload of a SuperBlock.
   *
   * This payload begins immediately following the Header. If the flags have been
   * configured to include the sub-block offset table, those offsets will begin
   * immediately following the `_reserved` field and can be accessed via the
   * `offsets` function. If the sub-block offset table has not been used, then
   * this function will be meaningless (or, more specifically, ends up pointing to
   * the first sub-block, i.e. it's Header).
   */
  struct SuperBlock {
    hd_std::uint64_t sequence_num { 0 }; ///< Sequence number - e.g., timestamp, or similar
    hd_std::uint8_t num_blocks { 0 };    ///< Number of subequent blocks in super-block group
    hd_std::uint8_t user { 0 };          ///< Field blank for arbitrary user use
    hd_std::uint16_t _reserved { 0 };    ///< Reserved for future use

    /**
     * If bit-0 of flags is enabled, a sub-block offset table can be accessed.
     *
     * The offset-table that is written contains jumps to each sub-block WITHIN the
     * SuperBlock. This is independent of the file-wide IndexEntry which is a
     * separate feature that can be toggled.
     *
     * @returns Pointer to the offset table. Only offsets if the flag was enabled.
     */
    SBIO_HD inline const hd_std::uint32_t* offsets() const {
      return reinterpret_cast<const hd_std::uint32_t*>(this + 1);
    }
  };

  /**
   * The payload of a ConfigBlock.
   */
  struct ConfigBlock {
    hd_std::uint32_t seed { 42 };        ///< Seed if using PNRG data filling.
    hd_std::uint8_t pattern_type { 0 };  ///< Input data: 0 = PNRG, 1 = Sequential, 2 = Fixed.
    hd_std::uint64_t total_events { 0 }; ///< Total events in stream. 0 = Unknown.
    hd_std::uint8_t flags { 0 };         ///< Any additional flags (e.g. to write SuperBlock offsets)
    hd_std::uint16_t _reserved { 0 };    ///< Reserved for future use
  };

  /**
   * The payload of a MetadataBlock.
   */
  struct MetadataBlock {
    char name[64];
    char type[64];
    hd_std::uint16_t rank;
    hd_std::uint32_t shape[5];
    hd_std::uint16_t dtype;
  };

  /**
   * The payload of an IndexBlock.
   *
   * If the super-block offset table has been configured via appropriate flags, then
   * immediately following the `_reserved` field there will be `num_super_blocks`
   * IndexEntry objects before the FileTrailer. If there are no super-block offsets,
   * then the FileTrailer will immediately follow this IndexBlock.
   */
  struct IndexBlock {
    hd_std::uint64_t num_super_blocks; ///< Total number of SuperBlocks (and IndexEntry as a result)
    hd_std::uint32_t _reserved;        ///< Reserved for future use.
  };

  /**
   * An entry to describe the location of a prior SuperBlock.
   *
   * The entries include the absolute byte offset in the stream, and the total size of
   * SuperBlock (the SuperBlock and its enclosed sub-Blocks). Each entry also includes
   * the sequence number which will (or... should) match the corresponding sequence
   * number in the referenced SuperBlock.
   */
  struct IndexEntry {
    hd_std::uint64_t sequence_num; ///< Sequence number - e.g., timestamp, or similar (matches SuperBlock)
    hd_std::uint64_t offset;       ///< Absolute byte offset for the SuperBlock
    hd_std::uint32_t sb_size;      ///< Total byte size of SuperBlock (includes the enclosed sub-Blocks.)
  };

  /**
   * The main descriptor of a Block of data in the sbio randfmt format.
   *
   * A Block consists of a Header, followed by a payload. The payload's can be
   * described by the relevant structs (E.g. SuperBlock, MetadataBlock, etc.).
   * Those other structs do NOT include the Header, and are not children of
   * the Block struct. They describe only the payload. The DataBlock does not
   * have a specific struct to describe its payload as it is an arbitrary byte
   * stream (which can be described by a single pointer).
   *
   * The sbio randfmt has a two-layer hieararchy. There are SuperBlocks which
   * enclose a set of sub-Blocks that contain data, metadata, config, etc. Every
   * Block has an identifier value in its Header, and both the SuperBlock and each
   * sub-block have their own Headers.
   *
   * With byte number increasing to the right, a single SuperBlock will look like:
   *
   * |                              SuperBlock0                                 |
   * | SuperBlock Header | <Offsets> | Sub0 Header | Sub0 Payload |     ...     |
   *
   * Immediately, following SuperBlock0, comes the next SuperBlock. The <Offsets>
   * entry is optional, controlled by a feature flag, and may or may not be present.
   * If present, it has the offsets to quickly jump to each *sub-Block* within the
   * SuperBlock. There is additionally, a separate feature flag which controls the
   * presence of a series of IndexEntry objects which, if enabled, will appear at
   * the end of the file, and contain the offsets to quickly jump to each *SuperBlock*.
   */
  struct Block {
    Header hdr;

    SBIO_HD inline bool valid_magic() const {
      return hd_std::memcmp(this->hdr.magic, randfmt::MagicBytes, 8) == 0;
    }

    SBIO_HD inline bool test_flag(FormatFlags feat) const {
      return this->hdr.flags & (1 << static_cast<hd_std::uint8_t>(feat));
    }

    SBIO_HD inline BlockType block_type() const {
      return static_cast<BlockType>(hdr.block_type);
    }

    SBIO_HD inline hd_std::uint8_t block_id() const { return hdr.block_id; }

    SBIO_HD inline hd_std::uint32_t payload_size() const { return hdr.payload_size; }

    /**
     * Access the first byte of the Block's payload.
     *
     * @returns A pointer to the first byte of the payload.
     */
    SBIO_HD inline const char* data() const {
      return reinterpret_cast<const char*>(this + 1);
    }
    SBIO_HD inline char* data() { return reinterpret_cast<char*>(this + 1); }

    /**
     * Return the next Block (if valid).
     *
     * Alternatively, this function can be thought of as returning the next
     * Header that will be encountered. For SuperBlocks, it will return the
     * first sub-block *inside* the SuperBlock. For everything else, it is
     * equivalent to calling `next_block`.
     *
     * @returns The next Block in the stream, or alternatively, the closest Header.
     */
    SBIO_HD inline const Block* closest_block() const {
      switch (this->block_type()) {
      case BlockType::Super: {
        hd_std::size_t total_offset { sizeof(SuperBlock) };
        if (this->test_flag(FormatFlags::SubBlockOffsetTable)) {
          const auto* sb { reinterpret_cast<const SuperBlock*>(this->data()) };
          total_offset += sb->num_blocks * sizeof(hd_std::uint32_t);
        }

        return reinterpret_cast<const Block*>(this->data() + total_offset);
      }
      case BlockType::Config:
      case BlockType::Metadata:
      case BlockType::Data:
      case BlockType::Index: {
        return reinterpret_cast<const Block*>(this->data() + this->payload_size());
      }
      }
    }

    /**
     * Return the next Block (if valid) at this hierarchy level.
     *
     * The sbio `randfmt` has two levels of the hierarchy, SuperBlocks and the
     * contained sub-blocks. If this is a SuperBlock, this function returns the
     * next SuperBlock. Otherwise, it returns the next sub-block. For the second
     * case, it is equivalent to the `closest_block` function.
     *
     * @returns The next Block at the same level of the two-level format hierarchy.
     */
    SBIO_HD inline const Block* next_block() const {
      return reinterpret_cast<const Block*>(this->data() + this->payload_size());
    }
  };

  struct FileTrailer {
    hd_std::uint64_t idx_blk_offset; ///< Absolute byte offset to the IndexBlock
    char magic_tail[8];              ///< Final magic bytes. Should read 'SBIOTAIL'
  };
#pragma pack(pop)

  /**
   * A simplified description of a `detector` to allow specifying how to write payloads.
   */
  struct DetectorSpec {
    char name[MaxNameSize];                                  ///< Name of the detector
    char type[MaxNameSize];                                  ///< Type of the detector
    hd_std::uint16_t rank { 2 };                             ///< Rank (dims) of the payload
    hd_std::uint32_t shape[MaxRank] { 1024, 1024, 0, 0, 0 }; ///< Shape of the payload
    ncarray::DType dtype { ncarray::DType::uint16 };         ///< Underlying payload data type
  };

#ifdef _WIN32
  using FileHandle = HANDLE;
#else
  using FileHandle = int;
#endif // _WIN32

  void write_bytes(FileHandle f_handle,
                   const void* buf,
                   hd_std::size_t sz,
                   hd_std::uint64_t off);

  void write_super_block(FileHandle f_handle,
                         hd_std::uint64_t seq_no,
                         hd_std::uint8_t flags,
                         hd_std::uint8_t num_sub_blocks,
                         BlockType* block_types,
                         hd_std::uint8_t* block_ids,
                         void** payloads,
                         hd_std::uint32_t* sizes,
                         hd_std::uint64_t& curr_offset);

  void construct_data_block(hd_std::uint32_t size,
                            hd_std::uint8_t pattern_type,
                            hd_std::uint8_t*& payload,
                            hd_std::uint32_t seed = 42,
                            hd_std::uint64_t seq_no = 0);

  void construct_cfg_block(hd_std::uint32_t seed,
                           hd_std::uint8_t pattern_type,
                           hd_std::uint64_t total_events,
                           hd_std::uint8_t flags,
                           hd_std::uint8_t*& payload);

  void construct_meta_block(hd_std::uint8_t num_detectors,
                            const DetectorSpec* detector_specs,
                            hd_std::uint8_t*& payload);

  void write_sbiornd_file(FileHandle f_handle,
                          hd_std::uint64_t& curr_offset,
                          hd_std::uint8_t num_detectors,
                          const DetectorSpec* detector_specs,
                          hd_std::uint8_t* det_block_ids,
                          hd_std::uint32_t seed,
                          hd_std::uint8_t pattern_type,
                          hd_std::uint64_t total_events,
                          hd_std::uint8_t flags);

} // namespace sbio::randfmt

#endif // SBIO_FORMATS_RANDOM_RANDFMT_HH
