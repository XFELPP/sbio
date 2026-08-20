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

  /**
   * File format flags at the granularity of the SuperBlock.
   *
   * Currently, only addition of sub-block offsets tables is available.
   */


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

  struct SuperBlock {
    hd_std::uint64_t sequence_num; ///< Sequence number - e.g., timestamp, or similar
    hd_std::uint8_t num_blocks;    ///< Number of subequent blocks in super-block group
    hd_std::uint8_t user;          ///< Field blank for arbitrary user use
    hd_std::uint16_t _reserved;    ///< Reserved for future use

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

  struct ConfigBlock {
    hd_std::uint32_t seed { 42 };        ///< Seed if using PNRG data filling.
    hd_std::uint32_t pattern_type { 0 }; ///< Input data: 0 = PNRG, 1 = Sequential, 2 = Fixed.
    hd_std::uint64_t total_events { 0 }; ///< Total events in stream. 0 = Unknown.
    hd_std::uint32_t flags { 0 };        ///< Any additional flags (e.g. to write SuperBlock offsets)
  };

  struct MetadataBlock {
    char name[64];
    char type[64];
    hd_std::uint16_t rank;
    hd_std::uint32_t shape[5];
    hd_std::uint16_t dtype;
  };

  struct IndexBlock {
    hd_std::uint64_t num_super_blocks; ///< Total number of SuperBlocks (and IndexEntry as a result)
    hd_std::uint32_t _reserved;        ///< Reserved for future use.
  };

  struct IndexEntry {
    hd_std::uint64_t sequence_num; ///< Sequence number - e.g., timestamp, or similar (matches SuperBlock)
    hd_std::uint64_t offset;       ///< Absolute byte offset for the SuperBlock
    hd_std::uint32_t sb_size;      ///< Total byte size of SuperBlock (includes the enclosed sub-Blocks.)
  };

  struct Block {
    Header hdr;

    SBIO_HD inline bool valid_magic() const {
      return hd_std::memcmp(this->hdr.magic, randfmt::MagicBytes, 8) == 0;
    }

    SBIO_HD inline bool test_flag(FormatFlags feat) const {
      return this->hdr.flags & (1 << static_cast<hd_std::uint16_t>(feat));
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
} // namespace sbio::randfmt

#endif // SBIO_FORMATS_RANDOM_RANDFMT_HH
