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

#include "sbio/formats/xtc2/xtc2_traits.hh"

#include "sbio/core/metadata.hh"
#include "sbio/core/result.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/formats/xtc2/traversal.hh"
#include "sbio/util/string.hh"

#include <ncarray/dtype.hh>

#include <array>
#include <cstdint>
#include <cstring>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace {
  using namespace sbio;

  SBIO_HD ncarray::DType to_ncarray_dtype(XTC2::DType type) {
    switch (type) {
    case XTC2::DType::UINT8:
      return ncarray::DType::uint8;
    case XTC2::DType::UINT16:
      return ncarray::DType::uint16;
    case XTC2::DType::UINT32:
      return ncarray::DType::uint32;
    case XTC2::DType::UINT64:
      return ncarray::DType::uint64;
    case XTC2::DType::INT8:
      return ncarray::DType::int8;
    case XTC2::DType::INT16:
      return ncarray::DType::int16;
    case XTC2::DType::INT32:
      return ncarray::DType::int32;
    case XTC2::DType::INT64:
      return ncarray::DType::int64;
    case XTC2::DType::FLOAT:
      return ncarray::DType::float32;
    case XTC2::DType::DOUBLE:
      return ncarray::DType::float64;
    default:
      return ncarray::DType::uint8;
    }
  }

  SBIO_HD void bake_metadata(MetadataInventory<XTC2Traits>& inv,
                             XTC2::MetadataCollector& collector) {
    const auto& fields { collector.fields() };
    const auto& sd_offsets_discovered { collector.sd_offsets() };
    for (const auto& [nid, det] : collector.detectors()) {
      XTC2Traits::DataAccessPtn ptn { XTC2Traits::DataAccessPtn::L1Accept };
      if (std::strcmp(det.name, "epics") == 0) {
        ptn = XTC2Traits::DataAccessPtn::SlowUpdate;
      } else if (std::strcmp(det.name, "scan") == 0) {
        ptn = XTC2Traits::DataAccessPtn::BeginStep;
      }

      auto g_id { inv.register_group(det.name, det.type, det.segment, ptn, det.detId) };

      auto offset { sd_offsets_discovered.at(nid) };
      for (std::uint32_t f_idx = 0; f_idx < fields.at(nid).size(); ++f_idx) {
        const auto& f_descr { fields.at(nid)[f_idx] };

        XTC2::Name field_name(f_descr.name, f_descr.dtype, f_descr.rank);
        XTC2Traits::FieldMetadata xtc2_meta { nid, f_idx, field_name };

        auto dtype { to_ncarray_dtype(f_descr.dtype) };

        inv.add_field(g_id, offset, dtype, f_descr.rank, nullptr, xtc2_meta, det.alg, f_descr.name);
        if (ptn == XTC2Traits::DataAccessPtn::SlowUpdate) {
          // All EPICS (ie EPICSArch) detectors are under the `epics` name
          // So check if there is a field under that detector for a semantic lookup
          auto alias_g_id = inv.register_group_alias(f_descr.name, "epics", det.segment, ptn, det.detId);
          inv.add_field(alias_g_id, offset, dtype, f_descr.rank, nullptr, xtc2_meta, det.alg, "raw");
        } else if (ptn == XTC2Traits::DataAccessPtn::BeginStep) {
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
          auto alias_g_id = inv.register_group_alias(f_descr.name, "scan", det.segment, ptn, det.detId);
          inv.add_field(alias_g_id, offset, dtype, f_descr.rank, nullptr, xtc2_meta, det.alg, "raw");
        }
      }
    }

    inv.finalize();
  }
} // anonymous namespace

namespace sbio {
  SBIO_HD void XTC2Traits::discover_metadata(XTC2::Dgram* buffer,
                                             MetadataInventory<XTC2Traits>& inv,
                                             std::size_t offset) {
    if (buffer->service() == XTC2::TransitionId::Configure) {
      XTC2::MetadataCollector collector;
      XTC2::inspect_xtc2(&buffer->xtc, collector, 12);
      bake_metadata(inv, collector);
    }
  }

  SBIO_HD DataResult XTC2Traits::resolve_data(void* buffer,
                                              const MetadataInventory<XTC2Traits>& inv,
                                              const XTC2Traits::DataRequest& req,
                                              PayloadOffsetCache& cache) {
    auto* entry { inv.lookup(req) };
    if (entry == nullptr) {
      return {};
    }

    const auto& descr { entry->descriptor };
    const auto& xtc2_meta { descr.format_meta };

    std::uint32_t nid { xtc2_meta.names_id };
    std::uint32_t f_idx { xtc2_meta.field_idx };
    const XTC2::Name* field_schema { &(xtc2_meta.field_name) };

    // Try starting from the last cached offset (if there)
    // Fallback on the offset recorded during discovery phase
    // NOTE: Because these offsets and so on were recorded at a transition in
    // smd (likely, at any rate) they may be incorrect for actual data.
    const std::uint32_t field_id { inv.field_id(entry) };
    std::uint32_t sd_offset { descr.payload_offset };
    std::uint32_t cached_offset { 0 };
    if (cache.find(field_id, cached_offset)) {
      const auto* dg { reinterpret_cast<XTC2::Dgram*>(buffer) };
      const std::size_t dgram_size { sizeof(XTC2::Dgram) + dg->xtc.sizeofPayload() };
      if (cached_offset + sizeof(XTC2::ShapesData) <= dgram_size) {
        const auto* cached =
          reinterpret_cast<XTC2::ShapesData*>(reinterpret_cast<char*>(buffer) + cached_offset);
        if (cached->namesId() == nid) {
          sd_offset = cached_offset;
        }
      }
    }

    // Double check here if they match what we want.
    auto* shapes_data =
        reinterpret_cast<XTC2::ShapesData*>(reinterpret_cast<char*>(buffer) + sd_offset);
    if (shapes_data->namesId() != nid) {
      // Need to update the offset
      std::uint32_t total_offset = sd_offset;
      do {
        std::size_t shapes_data_size { shapes_data->sizeofPayload() + sizeof(XTC2::Xtc) };
        total_offset += shapes_data_size;

        shapes_data =
          reinterpret_cast<XTC2::ShapesData*>(reinterpret_cast<char*>(buffer) + total_offset);
      } while(shapes_data->namesId() != nid);

      // Cache the newly found offset
      cache.store(field_id, sd_offset);
    }

    XTC2::DataResult xtc2_res =
      XTC2::resolve_xtc2_pointer(buffer, sd_offset, field_schema, nid, f_idx);

    DataResult res {
      xtc2_res.data,
      xtc2_res.size,
      xtc2_res.rank,
      std::array<std::uint32_t, 10> {
        xtc2_res.shape[0],
        xtc2_res.shape[1],
        xtc2_res.shape[2],
        xtc2_res.shape[3],
        xtc2_res.shape[4]
      },
      descr.dtype
    };

    return res;
  }
} // namespace sbio
