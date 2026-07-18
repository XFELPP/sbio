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

#include "sbio/formats/xtc1/traversal.hh"
#include "sbio/formats/xtc1/xtc1_schemas.hh"
#include "sbio/util/string.hh"

#include <algorithm>
#include <cstdint>
#include <cstdlib>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace {
  using namespace sbio;

  SBIO_HD void bake_metadata(XTC1Traits::MetadataInventory& inventory,
                             XTC1::MetadataCollector& collector) {
    inventory.m_names_id_count = collector.detectors().size();
    inventory.m_sd_offset_count = collector.detectors().size();
    inventory.m_schema_offset_count = collector.detectors().size();

    std::size_t total_fields = 0;
    for (const auto& [nid, fields] : collector.fields()) {
      total_fields += fields.size();
    }
    inventory.m_field_count = total_fields;

    inventory.m_names_id_table =
      new XTC1Traits::MetadataInventory::NamesIdMap[inventory.m_names_id_count];
    inventory.m_sd_offsets =
      new XTC1Traits::MetadataInventory::SdOffsetMap[inventory.m_sd_offset_count];
    inventory.m_schema_offsets =
      new XTC1Traits::MetadataInventory::SchemaOffset[inventory.m_schema_offset_count];
    inventory.m_field_table =
      new XTC1Traits::MetadataInventory::FieldMap[total_fields];
    inventory.m_schemas = new XTC1::Name[total_fields];

    std::size_t nid_idx = 0;
    std::size_t field_idx = 0;
    std::size_t schema_idx = 0;
    const auto& sd_offsets_discovered = collector.sd_offsets();
    for (const auto& [nid, det] : collector.detectors()) {
      // String Table
      auto& n_map = inventory.m_names_id_table[nid_idx];
      safe_strncpy(n_map.key.dettype, det.type, XTC1::MaxNameSize);
      safe_strncpy(n_map.key.detname, det.name, XTC1::MaxNameSize);
      n_map.key.segment = det.segment;
      n_map.names_id = nid;

      std::uint32_t log_val { 0 };
      auto log_it { collector.src_logs().find(nid) };
      if (log_it != collector.src_logs().end()) {
        log_val = log_it->second;
      }

      std::uint32_t phy_val { 0 };
      auto phy_it { collector.src_phys().find(nid) };
      if (phy_it != collector.src_phys().end()) {
        phy_val = phy_it->second;
      }

      // Jump Tables
      inventory.m_sd_offsets[nid_idx] =
        XTC1Traits::MetadataInventory::SdOffsetMap(nid,
                                                   sd_offsets_discovered.at(nid),
                                                   log_val,
                                                   phy_val);

      inventory.m_schema_offsets[nid_idx] =
        { nid, static_cast<std::uint32_t>(schema_idx) };

      const auto& fields = collector.fields();
      // Schema Table
      auto fields_it = fields.find(nid);
      if (fields_it != fields.end()) {
        for (std::uint32_t f_idx = 0; f_idx < fields_it->second.size(); ++f_idx) {
          const auto& f_descr = fields_it->second[f_idx];
          inventory.m_schemas[schema_idx++] = XTC1::Name(f_descr.name, f_descr.dtype, f_descr.rank);
          auto& f_map = inventory.m_field_table[field_idx++];
          f_map.key.names_id = nid;
          safe_strncpy(f_map.key.fieldname, f_descr.name, XTC1::MaxNameSize);
          f_map.field_idx = f_descr.field_idx;
          f_map.is_config = f_descr.is_config;
          f_map.evt_type_id = f_descr.evt_type_id;
        }
      }
      nid_idx++;
    }
    // Sort all for binary search
    std::sort(inventory.m_names_id_table,
              inventory.m_names_id_table + inventory.m_names_id_count);

    std::sort(inventory.m_sd_offsets,
              inventory.m_sd_offsets + inventory.m_sd_offset_count);

    std::sort(inventory.m_schema_offsets,
              inventory.m_schema_offsets + inventory.m_schema_offset_count);

    std::sort(inventory.m_field_table,
              inventory.m_field_table + inventory.m_field_count);
  }

  // Helper to recursively find the OrigDgramOffsetV1 payload in the Xtc tree
  SBIO_HD const XTC1::SmlData::OrigDgramOffsetV1* find_smd_offset(const XTC1::Xtc* xtc) {
    if (xtc->contains.id() == XTC1::TypeId::Id_SmlDataOrigDgramOffset) {
      return reinterpret_cast<const XTC1::SmlData::OrigDgramOffsetV1*>(xtc->payload());
    }
    if (xtc->contains.id() == XTC1::TypeId::Id_Xtc) {
      std::size_t sub_offset = 0;
      std::size_t payload_size = xtc->sizeofPayload();
      while (sub_offset < payload_size) {
        auto* subxtc = reinterpret_cast<const XTC1::Xtc*>(xtc->payload() + sub_offset);
        auto* res = find_smd_offset(subxtc);
        if (res) {
          return res;
        }
        sub_offset += sizeof(XTC1::Xtc) + subxtc->sizeofPayload();
      }
    }
    return nullptr;
  }
} // anonymous namespace

namespace sbio {

  SBIO_HD void XTC1Traits::discover_metadata(XTC1::Dgram* buffer,
                                             XTC1Traits::MetadataInventory& inv,
                                             std::size_t offset) {
    if (buffer->service() == XTC1::TransitionId::Configure) {
      XTC1::MetadataCollector collector;

      // Discover the aliases from AliasConfig
      collector.populate_aliases(&buffer->xtc);

      // Start traversing from the top-level xtc inside the Configure datagram
      // Header offset is 40 bytes (sizeof Dgram)
      inspect_xtc1(&buffer->xtc, collector, 20);

      bake_metadata(inv, collector);
    }
  }

  SBIO_HD XTC1Traits::DataResult XTC1Traits::resolve_data(void* buffer,
                                                          const XTC1Traits::MetadataInventory& inv,
                                                          const XTC1Traits::DataRequest& req) {
    std::uint32_t nid { inv.resolve_names_id(req) };
    if (nid == 0xFFFFFFFF) {
      return {};
    }

    std::uint32_t f_idx { inv.resolve_field_idx(nid, req.field_name) };
    if (f_idx == 0xFFFFFFFF) {
      return {};
    }

    std::uint32_t sd_offset { inv.get_sd_offset(nid) };
    std::uint32_t evt_type_id { inv.get_evt_type_id(nid, req.field_name) };
    const XTC1::Name* field_schema { inv.get_schema(nid) };


    std::uint32_t src_log { inv.get_src_log(nid) };
    std::uint32_t src_phy { inv.get_src_phy(nid) };

    return XTC1::resolve_xtc_pointer(buffer,
                                     sd_offset,
                                     field_schema,
                                     nid,
                                     f_idx,
                                     evt_type_id,
                                     &inv,
                                     src_log,
                                     src_phy);
  }

  SBIO_HD std::size_t XTC1Traits::populate_offsets(XTC1Traits::DataUnit* dg,
                                                   XTC1Traits::DiscoveryState& state,
                                                   std::size_t beginning_offset,
                                                   XTC1Traits::EventOffset* l1_offsets_buf,
                                                   XTC1Traits::TransitionOffset* transition_offsets_buf,
                                                   std::size_t access_offset) {
    int payload_size { dg->xtc.sizeofPayload() };
    std::uint64_t total_size = sizeof(XTC1::Dgram) + payload_size;
    if (dg->service() == XTC1::TransitionId::L1Accept) {
      // Find the OrigDgramOffsetV1 struct containing the big data file offset and size
      const auto* smd_offset = find_smd_offset(&(dg->xtc));

      std::uint64_t offset;
      std::uint64_t size;
      if (smd_offset) {
        // If reading an SMD file: use the recorded big data offset/size
        offset = smd_offset->fileOffset();
        size = smd_offset->extent();
      } else {
        // Fallback if reading a BD file directly without an SMD file
        offset = beginning_offset + access_offset;
        size = total_size;
      }
      auto buf_off = state.events_per_read
        ? state.l1_offset_idx % state.events_per_read
        : state.l1_offset_idx;
      new (l1_offsets_buf + buf_off) XTC1Traits::EventOffset(offset, size);
      state.last_l1_idx_seen++;
      state.l1_offset_idx++;
    } else {
      // For transitions, record the offset of the transition datagram
      std::uint64_t offset = beginning_offset + access_offset;
      auto buf_off = state.events_per_read
        ? state.trans_offset_idx % state.events_per_read
        : state.trans_offset_idx;
      new (transition_offsets_buf + buf_off)
        XTC1Traits::TransitionOffset(offset,
                                     total_size,
                                     state.last_l1_idx_seen,
                                     dg->service());
      state.trans_offset_idx++;
      if (dg->service() == XTC1::TransitionId::EndRun) {
        state.seen_end_run = true;
      }
    }
    return total_size;
  }

  // --- XTC1Traits::MetadataInventory (Plus its internal structs too) --- //
  SBIO_HD std::uint32_t
  XTC1Traits::MetadataInventory::get_sd_offset(std::uint32_t nid) const {
    auto* it = std::lower_bound(m_sd_offsets, m_sd_offsets + m_sd_offset_count, nid);

    if (it != m_sd_offsets + m_sd_offset_count && it->names_id == nid) {
      return it->offset;
    }

    return 0;
  }

  SBIO_HD const XTC1::Name*
  XTC1Traits::MetadataInventory::get_schema(std::uint32_t nid) const {
    auto* it = std::lower_bound(m_schema_offsets, m_schema_offsets + m_schema_offset_count, nid);

    if (it != m_schema_offsets + m_schema_offset_count && it->names_id == nid) {
      return &m_schemas[it->start_idx];
    }

    return nullptr;
  }

  SBIO_HD std::uint32_t
  XTC1Traits::MetadataInventory::resolve_names_id(const DataRequest& req) const {
    DetKey search_key;
    safe_strncpy(search_key.dettype, req.detector_type, XTC1::MaxNameSize);
    safe_strncpy(search_key.detname, req.detector_name, XTC1::MaxNameSize);
    search_key.segment = req.segment_number;

    auto* it = std::lower_bound(m_names_id_table, m_names_id_table + m_names_id_count, search_key);

    if (it != m_names_id_table + m_names_id_count && !(search_key < it->key)) {
      return it->names_id;
    }

    // Segments across streams in XTC1 generally get registered all with "segment 0"
    // As a fallback, check again for 0
    search_key.segment = 0;
    it = std::lower_bound(m_names_id_table, m_names_id_table + m_names_id_count, search_key);
    if (it != m_names_id_table + m_names_id_count && !(search_key < it->key)) {
      return it->names_id;
    }

    return 0xFFFFFFFF;
  }

  SBIO_HD std::uint32_t
  XTC1Traits::MetadataInventory::resolve_field_idx(std::uint32_t nid, const char* field) const {
    FieldKey search_key { nid };
    safe_strncpy(search_key.fieldname, field, XTC1::MaxNameSize);
    auto* it = std::lower_bound(m_field_table, m_field_table + m_field_count, search_key);

    if (it != m_field_table + m_field_count && !(search_key < it->key)) {
      return it->field_idx;
    }
    return 0xFFFFFFFF;
  }

  SBIO_HD bool XTC1Traits::MetadataInventory::is_config_field(std::uint32_t nid,
                                                              const char* field) const {
    FieldKey search_key { nid };
    safe_strncpy(search_key.fieldname, field, XTC1::MaxNameSize);
    auto* it = std::lower_bound(m_field_table, m_field_table + m_field_count, search_key);
    if (it != m_field_table + m_field_count && !(search_key < it->key)) {
      return it->is_config;
    }
    return false;
  }

  SBIO_HD std::uint32_t
  XTC1Traits::MetadataInventory::get_evt_type_id(std::uint32_t nid,
                                                 const char* field) const {
    FieldKey search_key { nid };
    safe_strncpy(search_key.fieldname, field, XTC1::MaxNameSize);

    auto* it { std::lower_bound(m_field_table, m_field_table + m_field_count, search_key) };
    if (it != m_field_table + m_field_count && !(search_key < it->key)) {
      return it->evt_type_id;
    }

    return 0;
  }

  SBIO_HD std::uint32_t
  XTC1Traits::MetadataInventory::get_src_log(std::uint32_t nid) const {
    auto* it { std::lower_bound(m_sd_offsets, m_sd_offsets + m_sd_offset_count, nid) };
    if (it != m_sd_offsets + m_sd_offset_count && it->names_id == nid) {
      return it->log;
    }

    return 0;
  }
  SBIO_HD std::uint32_t
  XTC1Traits::MetadataInventory::get_src_phy(std::uint32_t nid) const {
    auto* it { std::lower_bound(m_sd_offsets, m_sd_offsets + m_sd_offset_count, nid) };
    if (it != m_sd_offsets + m_sd_offset_count && it->names_id == nid) {
      return it->phy;
    }

    return 0;
  }
} // namespace sbio
