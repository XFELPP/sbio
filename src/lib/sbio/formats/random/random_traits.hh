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

#include "sbio/formats/random/randfmt.hh"

#include "sbio/core/metadata.hh"
#include "sbio/core/request.hh"
#include "sbio/core/result.hh"
#include "sbio/core/roles.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/core/storage.hh"
#include "sbio/core/storage_view.hh"
#include "sbio/core/sync.hh"
#include "sbio/export_macro.hh"
#include "sbio/formats/format_traits.hh"
#include "sbio/locators/custom_lambda.hh"
#include "sbio/util/parameters.hh"
#include "sbio/util/string.hh"

#include <ncarray/ncarrays.hh>

#ifdef __CUDACC__

#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/cstring>
#include <cuda/std/utility>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

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
    using DefaultLocator = CustomLambdaLocator;

    static constexpr hd_std::size_t HeaderSize { sizeof(randfmt::Header) };
    static constexpr hd_std::uint16_t MaxRank { 5 };
    static constexpr hd_std::uint16_t MaxNameSize { 64 };

    using DataUnit = randfmt::Block;
    using StepIdxType = hd_std::size_t;
    static constexpr StepIdxType ExhaustedSentinel { static_cast<StepIdxType>(-1) };

    static constexpr StreamPartitioningStrategy PartitioningStrategy {
        StreamPartitioningStrategy::SubDivide
    };

    struct DataStream
      : public StreamVariant<roles::Metadata, roles::Index, roles::Data> {};

    using StreamTypes = StreamSet<DataStream>;

    enum class DataAccessPtn : hd_std::uint8_t {
      Default = 0
    };
    static constexpr hd_std::size_t DataAccessPtnCount { 1 };

    enum class StepKind : hd_std::uint8_t {
      Default = 0,
      Other = 1
    };

    static constexpr std::size_t NumStepKinds { 2 };

    enum class IndexingMode : hd_std::uint8_t {
      IndexAll   = 0, ///< When index_stream is called, all events will be indexed.
      IndexBatch = 1, ///< When index_stream is called a batch is indexed. This allows reindexing later.
      NoIndex    = 2  ///< Offsets are not written, so indexing is disabled and data is traversed linearly.
    };

    struct StreamParameters {
      hd_std::size_t num_events { 100000 };
      hd_std::size_t event_size { 0x100000 };
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

    using BrokerBufferRequirements = RequirementsList<
      BufferDescriptor<roles::Metadata, 0, sizeof(randfmt::Block)>,  /* Buffer for transition */
      BufferDescriptor<roles::Data, 0, sizeof(randfmt::Block)>,
      BufferDescriptor<roles::Index, index_ids::Steps>,
      BufferDescriptor<roles::Metadata, index_ids::Scratch>
    >;

    using RequestSchema = sbio::NamedKeys<>;
    using DataRequest = sbio::DataRequest<RequestSchema>;

    using GroupKeys = sbio::NamedKeys<>;

    // Don't actually need this, but have it to test functionality until it can be
    // removed in other formats
    struct FieldMetadata {
      hd_std::uint8_t det_idx { 0 };
    };

    SBIO_HD static AllocationRequest<RandomTraits>
    get_allocation_request(GenericStreamConfig<RandomTraits>& cfg) {
      AllocationRequest<RandomTraits> request;

      request.size_requests[0] = cfg.format_params.event_size;
      request.size_requests[1] =
        cfg.format_params.event_size * (cfg.max_batch_size > 0 ? cfg.max_batch_size : 1);

      hd_std::size_t rows { 1 };
      if (cfg.format_params.indexing_mode == IndexingMode::IndexAll) {
        rows = cfg.format_params.num_events + NumStepKinds;
      } else if (cfg.format_params.indexing_mode == IndexingMode::IndexBatch) {
        rows = cfg.format_params.indexing_batch_size;
      }
      request.size_requests[2] = (rows + NumStepKinds) * sizeof(StepOffset<RandomTraits>);
      request.size_requests[3] = 0x10000;

      return request;
    }

    template <IOTraits IO, class StorageViewT>
    SBIO_HD static IOStatus discover_metadata(Stream<IO, RandomTraits>* streams,
                                              StorageViewT& storage,
                                              MetadataInventory<RandomTraits>& inv) {
      auto* buf =
        storage.template acquire<roles::Metadata, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);

      IOStatus status = get_stream<DataStream>(streams).read_unit_at(buf,
                                                                     storage.template size<roles::Metadata>(),
                                                                     0).status;

      if (status != IOStatus::Success) {
        storage.template release<roles::Metadata, 0>(buf);

        return status;
      }

      const auto* blk0 { reinterpret_cast<const randfmt::Block*>(buf) };
      if (!blk0->valid_magic() || blk0->block_type() != randfmt::BlockType::Super) {
        storage.template release<roles::Metadata, 0>(buf);

        return IOStatus::HeaderReadError;
      }

      const auto* sb0 { reinterpret_cast<const randfmt::SuperBlock*>(blk0->data()) };
      const auto* sub_blk { blk0->closest_block() };

      for (hd_std::uint8_t i = 0; i < sb0->num_blocks; ++i) {
        if (!sub_blk->valid_magic()) {
          break;
        }

        if (sub_blk->block_type() == randfmt::BlockType::Metadata) {
          const auto* meta { reinterpret_cast<const randfmt::MetadataBlock*>(sub_blk->data()) };

          auto g_id = inv.register_group(meta->name,
                                         meta->type,
                                         0,
                                         RandomTraits::DataAccessPtn::Default);
          inv.add_field(g_id,
                        0,
                        static_cast<ncarray::DType>(meta->dtype),
                        meta->rank,
                        meta->shape,
                        FieldMetadata { static_cast<hd_std::uint8_t>(g_id) });
        }

        sub_blk = sub_blk->closest_block();
      }

      storage.template release<roles::Metadata, 0>(buf);
      return IOStatus::Success;
    }

    SBIO_HD static constexpr StepKind kind_for_ptn(DataAccessPtn ptn) {
      return StepKind::Default;
    }

    SBIO_HD static hd_std::optional<StepKind> kind_for_step(const DataUnit* blk) {
      if (!blk->valid_magic()) {
        return hd_std::nullopt;
      }

      if (blk->block_type() == randfmt::BlockType::Super &&
          blk->closest_block()->block_type() == randfmt::BlockType::Data) {
        return StepKind::Default;
      }

      return StepKind::Other;
    }

    SBIO_HD static hd_std::size_t locate_bytes(const DataUnit* blk) {
      constexpr hd_std::size_t max_bytes {
        2 * sizeof(randfmt::Header) + sizeof(randfmt::SuperBlock) + 255 * sizeof(hd_std::uint32_t)
      };

      const hd_std::size_t size { unit_size(blk) };
      return (size < max_bytes) ? size : max_bytes;
    }

    SBIO_HD static hd_std::size_t unit_size(const DataUnit* blk) {
      return sizeof(randfmt::Header) + blk->payload_size();
    }

    SBIO_HD static ByteRegion locate_step(const DataUnit* blk,
                                          StepKind kind,
                                          hd_std::uint32_t scanned_stream,
                                          hd_std::uint64_t scan_offset) {
      return { scan_offset, unit_size(blk), scanned_stream };
    }

    template <class StorageViewT>
    SBIO_HD static DataResult get_data_in_buffer(StorageViewT& storage,
                                                 const MetadataInventory<RandomTraits>& inv,
                                                 const DataRequest& req,
                                                 PayloadOffsetCache& cache,
                                                 DataAccessPtn ptn,
                                                 std::size_t batch_idx = 0) {
      void* data_buf =
          storage.template acquire<roles::Data, 0, ncarray::HostTag>(AcquireIntent::CallerMemorySpace);

      const char* ptr { reinterpret_cast<const char*>(data_buf) };
      for (hd_std::size_t b = 0; b < batch_idx; ++b) {
        const auto* blk { reinterpret_cast<const randfmt::Block*>(ptr) };
        ptr += sizeof(randfmt::Header) + blk->payload_size();
      }

      DataResult res { resolve_data(const_cast<char*>(ptr), inv, req, cache) };
      res.data = storage.template release<roles::Data, 0>(data_buf, res.data);

      return res;
    }

    SBIO_HD static DataResult resolve_data(void* buffer,
                                           const MetadataInventory<RandomTraits>& inv,
                                           const DataRequest& req,
                                           PayloadOffsetCache& cache) {
      DataResult res{};
      const auto* super_blk { reinterpret_cast<const randfmt::Block*>(buffer) };
      if (!super_blk->valid_magic() || super_blk->block_type() != randfmt::BlockType::Super) {
        return res;
      }

      const auto* sb_payload { reinterpret_cast<const randfmt::SuperBlock*>(super_blk->data()) };

      auto* entry { inv.lookup(req) };
      if (entry == nullptr) {
        return res;
      }
      const auto& descr { entry->descriptor };
      const hd_std::uint32_t field_id { inv.field_id(entry) };

      // hd_std::uint8_t det_idx { static_cast<hd_std::uint8_t>(entry->key.group_id) };
      hd_std::uint8_t det_idx { descr.format_meta.det_idx };
      const char* sb_payload_start { super_blk->data() };

      auto fill_result = [&](const randfmt::Block* blk) {
        hd_std::size_t size { 1 };
        res.data = blk->data();
        res.rank = descr.rank;
        for (hd_std::uint16_t r = 0; r < descr.rank; ++r) {
          res.shape[r] = descr.shape[r];
          size *= res.shape[r];
        }
        res.dtype = descr.dtype;
        res.size = size;
      };

      auto block_at = [&](hd_std::uint32_t off) -> const randfmt::Block* {
        if (off == 0 || off >= super_blk->payload_size()) {
          return nullptr;
        }

        const auto* blk { reinterpret_cast<const randfmt::Block*>(sb_payload_start + off) };
        if (!blk->valid_magic()                           ||
            blk->block_type() != randfmt::BlockType::Data ||
            blk->block_id() != det_idx) {
          return nullptr;
        }

        return blk;
      };

      hd_std::uint32_t off { 0 };
      if (!cache.find(field_id, off)) {
        off = descr.payload_offset;
      }
      if (const auto* blk = block_at(off)) {
        fill_result(blk);
        return res;
      }

      if (super_blk->test_flag(randfmt::FormatFlags::SubBlockOffsetTable)) {
        const hd_std::uint32_t* sub_offsets { sb_payload->offsets() };

        hd_std::uint32_t blk_offset { sub_offsets[det_idx] };
        cache.store(field_id, blk_offset);

        fill_result(reinterpret_cast<const randfmt::Block*>(sb_payload_start + blk_offset));

        return res;
      }

      const auto* sub_blk { super_blk->closest_block() };
      for (hd_std::uint8_t b = 0; b < sb_payload->num_blocks; ++b) {
        if (!sub_blk->valid_magic()) {
          break;
        }

        if (sub_blk->block_type() == randfmt::BlockType::Data && sub_blk->block_id() == det_idx) {
          fill_result(sub_blk);

          const auto blk_offset {
            static_cast<hd_std::uint32_t>(reinterpret_cast<const char*>(sub_blk) - sb_payload_start)
          };
          cache.store(field_id, blk_offset);

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
