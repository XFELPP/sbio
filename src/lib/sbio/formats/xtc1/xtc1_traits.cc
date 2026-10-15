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

#include "sbio/formats/xtc1/xtc1_traits.hh"

#include "sbio/core/metadata.hh"
#include "sbio/core/result.hh"
#include "sbio/core/state_handle.hh"
#include "sbio/formats/xtc1/traversal.hh"
#include "sbio/formats/xtc1/xtc1_schemas.hh"
#include "sbio/util/string.hh"

#include <ncarray/dtype.hh>

#include <array>
#include <cstddef>
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

  SBIO_HD ncarray::DType to_ncarray_dtype(XTC1::DType type) {
    switch (type) {
    case XTC1::DType::UINT8:
      return ncarray::DType::uint8;
    case XTC1::DType::UINT16:
      return ncarray::DType::uint16;
    case XTC1::DType::UINT32:
      return ncarray::DType::uint32;
    case XTC1::DType::UINT64:
      return ncarray::DType::uint64;
    case XTC1::DType::INT8:
      return ncarray::DType::int8;
    case XTC1::DType::INT16:
      return ncarray::DType::int16;
    case XTC1::DType::INT32:
    case XTC1::DType::ENUMVAL:
    case XTC1::DType::ENUMDICT:
      return ncarray::DType::int32;
    case XTC1::DType::INT64:
      return ncarray::DType::int64;
    case XTC1::DType::FLOAT:
      return ncarray::DType::float32;
    case XTC1::DType::DOUBLE:
      return ncarray::DType::float64;
    default: // CHARSTR
      return ncarray::DType::uint8;
    }
  }

  SBIO_HD void bake_metadata(MetadataInventory<XTC1Traits>& inv,
                             XTC1::MetadataCollector& collector) {
    const auto& fields { collector.fields() };
    const auto& sd_offsets_discovered { collector.sd_offsets() };

    for (const auto& [nid, det] : collector.detectors()) {
      const auto& src_name { collector.src_names().at(nid) };

      // The run control's configuration is updated at each BeginCalibCycle (scan steps)
      const bool is_control { collector.types().at(nid).id() == XTC1::TypeId::Id_ControlConfig };
      const auto ptn {
        is_control ? XTC1Traits::DataAccessPtn::BeginStep : XTC1Traits::DataAccessPtn::L1Accept
      };

      auto g_id { inv.register_group(det.name, det.type, det.segment, ptn, src_name.c_str()) };

      auto fields_it { fields.find(nid) };
      if (fields_it == fields.end()) {
        continue;
      }

      const std::uint32_t offset { sd_offsets_discovered.at(nid) };
      for (const auto& f_descr : fields_it->second) {
        XTC1Traits::FieldMetadata meta {
          offset,
          f_descr.evt_type_id,
          collector.src_logs().at(nid),
          collector.src_phys().at(nid),
          f_descr.field_idx
        };

        inv.add_field(g_id,
                      offset,
                      to_ncarray_dtype(f_descr.dtype),
                      static_cast<std::uint16_t>(f_descr.rank),
                      nullptr,
                      meta,
                      f_descr.name);
      }
    }

    inv.finalize();
  }
} // anonymous namespace

namespace sbio {

  SBIO_HD void XTC1Traits::discover_metadata(XTC1::Dgram* buffer,
                                             MetadataInventory<XTC1Traits>& inv,
                                             std::size_t offset) {
    if (buffer->service() == XTC1::TransitionId::Configure) {
      XTC1::MetadataCollector collector;

      // Discover the aliases from AliasConfig
      collector.populate_aliases(&buffer->xtc);

      // Start traversing from the top-level xtc inside the Configure datagram
      // Offsets are from the start of the datagram
      // the Xtc follows the Sequence (16 bytes) and Env (4 bytes)
      inspect_xtc1(&buffer->xtc,
                   collector,
                   static_cast<std::uint32_t>(sizeof(XTC1::Dgram) - sizeof(XTC1::Xtc)));

      bake_metadata(inv, collector);
    }
  }

  SBIO_HD const XTC1::SmlData::OrigDgramOffsetV1* XTC1Traits::find_smd_offset(const XTC1::Xtc* xtc) {
    if (xtc->contains.id() == XTC1::TypeId::Id_SmlDataOrigDgramOffset) {
      return reinterpret_cast<const XTC1::SmlData::OrigDgramOffsetV1*>(xtc->payload());
    }
    if (xtc->contains.id() == XTC1::TypeId::Id_Xtc) {
      std::size_t sub_offset = 0;
      const std::size_t payload_size = xtc->sizeofPayload();
      while (sub_offset + sizeof(XTC1::Xtc) <= payload_size) {
        auto* subxtc = reinterpret_cast<const XTC1::Xtc*>(xtc->payload() + sub_offset);
        if (subxtc->extent < sizeof(XTC1::Xtc)) {
          break; // Corrupt extent -- would never advance
        }

        if (auto* res = find_smd_offset(subxtc)) {
          return res;
        }
        sub_offset += subxtc->extent;
      }
    }

    return nullptr;
  }

  SBIO_HD DataResult XTC1Traits::resolve_data(void* buffer,
                                              const MetadataInventory<XTC1Traits>& inv,
                                              const XTC1Traits::DataRequest& req,
                                              PayloadOffsetCache& cache,
                                              const void* config) {
    auto* entry { inv.lookup(req) };
    if (entry == nullptr) {
      return {};
    }

    const auto& descr { entry->descriptor };
    const auto& meta { descr.format_meta };

    const char* cfg_dgram { static_cast<const char*>(config != nullptr ? config : buffer) };
    const auto* config_xtc { reinterpret_cast<const XTC1::Xtc*>(cfg_dgram + meta.config_offset) };

    const XTC1::Xtc* xtc { nullptr };
    if (meta.event_type == 0) {
      // Configuration field is at the offset recorded during discovery
      xtc = reinterpret_cast<const XTC1::Xtc*>(static_cast<const char*>(buffer) + meta.config_offset);
    } else {
      auto* dg { reinterpret_cast<const XTC1::Dgram*>(buffer) };
      const std::size_t dgram_size { sizeof(XTC1::Dgram) - sizeof(XTC1::Xtc) + dg->xtc.extent };
      auto matches = [&](const XTC1::Xtc* xtc_) {
        return xtc_->contains.id() == static_cast<XTC1::TypeId::Type>(meta.event_type) &&
               xtc_->src.log() == meta.src_log &&
               xtc_->src.phy() == meta.src_phy;
      };

      // Try the Xtc offset found in the previous datagram first
      const std::uint32_t field_id { inv.field_id(entry) };
      std::uint32_t cached_offset { 0 };
      if (cache.find(field_id, cached_offset) &&
          cached_offset + sizeof(XTC1::Xtc) <= dgram_size) {
        const auto* cached {
          reinterpret_cast<const XTC1::Xtc*>(static_cast<const char*>(buffer) + cached_offset)
        };
        if (matches(cached) && cached_offset + cached->extent <= dgram_size) {
          xtc = cached;
        }
      }

      if (xtc == nullptr) {
        xtc = XTC1::find_xtc_by_type(&dg->xtc,
                                     static_cast<XTC1::TypeId::Type>(meta.event_type),
                                     meta.src_log,
                                     meta.src_phy);
        if (xtc == nullptr) {
          return {}; // Not in this event (e.g. damaged or dropped contribution)
        }

        cache.store(field_id,
                    static_cast<std::uint32_t>(reinterpret_cast<const char*>(xtc) -
                                               static_cast<const char*>(buffer)));
      }
    }

    XTC1::DataResult xtc1_res {
      XTC1::resolve_xtc_pointer(xtc, req.get<"field">().c_str(), meta.field_idx, config_xtc)
    };

    if (xtc1_res.data == nullptr) {
      return {};
    }

    DataResult res {};
    res.data = xtc1_res.data;
    res.size = xtc1_res.size;
    res.rank = xtc1_res.rank;
    for (std::uint16_t r = 0; r < xtc1_res.rank && r < res.shape.size(); ++r) {
      res.shape[r] = xtc1_res.shape[r];
    }
    res.dtype = to_ncarray_dtype(xtc1_res.dtype);

    return res;
  }
} // namespace sbio
