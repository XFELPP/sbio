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

#include "sbio/formats/xtc2/traversal.hh"

#include <cstdint>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace {
  using namespace sbio;

  SBIO_HD void bake_metadata(XTC2Traits::MetadataInventory& inventory,
                             XTC2::MetadataCollector& collector) {
    inventory.m_names_id_count = collector.detectors().size();
    inventory.m_sd_offset_count = collector.detectors().size();
    inventory.m_schema_offset_count = collector.detectors().size();

    std::size_t total_fields = 0;
    for (const auto& [nid, fields] : collector.fields()) {
      total_fields += fields.size();
    }
    inventory.m_field_count = total_fields;

    inventory.m_names_id_table =
      new XTC2Traits::MetadataInventory::NamesIdMap[inventory.m_names_id_count];
    inventory.m_sd_offsets =
      new XTC2Traits::MetadataInventory::SdOffsetMap[inventory.m_sd_offset_count];
    inventory.m_schema_offsets =
      new XTC2Traits::MetadataInventory::SchemaOffset[inventory.m_schema_offset_count];
    inventory.m_field_table =
      new XTC2Traits::MetadataInventory::FieldMap[total_fields];
    inventory.m_schemas = new XTC2::Name[total_fields];

    std::size_t nid_idx = 0;
    std::size_t field_idx = 0;
    std::size_t schema_idx = 0;
    const auto& sd_offsets_discovered = collector.sd_offsets();
    for (const auto& [nid, det] : collector.detectors()) {
      // String Table
      auto& n_map = inventory.m_names_id_table[nid_idx];
      XTC2::safe_strncpy(n_map.key.dettype, det.type);
      XTC2::safe_strncpy(n_map.key.detname, det.name);
      XTC2::safe_strncpy(n_map.key.algname, det.alg);
      n_map.key.segment = det.segment;
      n_map.names_id = nid;
      // Jump Tables
      inventory.m_sd_offsets[nid_idx] =
        XTC2Traits::MetadataInventory::SdOffsetMap(nid, sd_offsets_discovered.at(nid));
      inventory.m_schema_offsets[nid_idx] =
        { nid, static_cast<std::uint32_t>(schema_idx) };

      const auto& fields = collector.fields();
      // Schema Table
      for (std::uint32_t f_idx = 0; f_idx < fields.at(nid).size(); ++f_idx) {
        const auto& f_descr = fields.at(nid)[f_idx];
        inventory.m_schemas[schema_idx++] = XTC2::Name(f_descr.name, f_descr.dtype, f_descr.rank);

        auto& f_map = inventory.m_field_table[field_idx++];
        f_map.key.names_id = nid;
        XTC2::safe_strncpy(f_map.key.fieldname, f_descr.name);
        f_map.field_idx = f_idx;
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
} // anonymous namespace

namespace sbio {
  SBIO_HD void XTC2Traits::discover_metadata(XTC2::Dgram* buffer,
                                              XTC2Traits::MetadataInventory& inv,
                                              std::size_t offset) {
    if (buffer->service() == XTC2::TransitionId::Configure) {
      XTC2::MetadataCollector collector;
      XTC2::inspect_xtc2(&buffer->xtc, collector, 12);
      bake_metadata(inv, collector);
    }
  }

  SBIO_HD XTC2Traits::DataResult XTC2Traits::resolve_data(void* buffer,
                                                           const XTC2Traits::MetadataInventory& inv,
                                                           const XTC2Traits::DataRequest& req) {
    std::uint32_t nid = inv.resolve_names_id(req);
    if (nid == 0xFFFFFFFF) {
      // Detector/Algorithm not found
      return {};
    }

    std::uint32_t f_idx = inv.resolve_field_idx(nid, req.field_name);
    if (f_idx == 0xFFFFFFFF) {
      // Field not found
      return {};
    }

    std::uint32_t sd_offset = inv.get_sd_offset(nid);
    const XTC2::Name* field_schema = inv.get_schema(nid);
    // NOTE: Because these offsets and so on were recorded at a transition in smd
    // (likely, at any rate) they may be incorrect for actual data.
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

      for (std::size_t i = 0; i < inv.m_sd_offset_count; ++i) {
        auto& sd_map = inv.m_sd_offsets[i];
        if (sd_map.names_id == nid) {
          sd_map.offset = total_offset;
        }
      }
      sd_offset = inv.get_sd_offset(nid);
    }
    return XTC2::resolve_xtc2_pointer(buffer, sd_offset, field_schema, nid, f_idx);
  }


  SBIO_HD std::size_t XTC2Traits::populate_offsets(XTC2Traits::DataUnit* dg, // Dgram*
                                                    XTC2Traits::DiscoveryState& state,
                                                    std::size_t beginning_offset,
                                                    XTC2Traits::EventOffset* l1_offsets_buf,
                                                    XTC2Traits::TransitionOffset* transition_offsets_buf,
                                                    std::size_t access_offset) {
    int payload_size { dg->xtc.sizeofPayload() };

    if (dg->service() == XTC2::TransitionId::L1Accept) {
      auto* char_ptr = reinterpret_cast<char*>(&(dg->xtc));
      // L1Accept is 48 bytes into the SMD paylaod
      std::size_t offset_in_payload { 48 };
      auto* offset_ptr = reinterpret_cast<std::uint64_t*>(char_ptr + offset_in_payload);
      std::uint64_t offset { *offset_ptr };
      // Size is 1, 8 byte int later
      std::uint64_t size = *(offset_ptr + 1);

      auto buf_off = state.events_per_read
        ? state.l1_offset_idx % state.events_per_read
        : state.l1_offset_idx;

      new (l1_offsets_buf + buf_off) XTC2Traits::EventOffset(offset, size);
      state.last_l1_idx_seen++;
      state.l1_offset_idx++;
    } else {
      std::uint64_t size = sizeof(*dg) + payload_size;

      std::uint64_t offset;
      if (state.last_l1_idx_seen < 0) {
        // Have seen nothing but transitions... Then the smd file_offset can be used
        // This is because entire transitions are also stored in .smd.xtc2 files
        // If we have yet to see an L1Accept, then the offset in .smd.xtc2 is equal
        // to the offset in .xtc2
        /// TODO: The above actually doesn't seem to be true!!! Investigate why!
        /// The Configure Transition size doesn't match between the .smd.xtc2
        /// and .xtc2 files... The others do at least as far as I can tell.
        /// For now, the BDReader must do some hackery if prev_l1 is -1. It will then
        /// Calculate based on the size (which IS accurate at least) and the first
        /// L1Accept offset what the correct SlowUpdate offset should be...


        offset = beginning_offset + access_offset;
      } else if (state.l1_offset_idx != 0) {
        // Have seen L1 (and not wrapped)... Can use previous L1 offset+size
        // But... Have to see if any other previous transitions as well
        XTC2Traits::EventOffset prev_l1 = l1_offsets_buf[state.l1_offset_idx - 1];
        offset = prev_l1.offset + prev_l1.size;

        std::size_t prev_transition_idx = state.trans_offset_idx - 1;

        auto prev_transition = transition_offsets_buf[prev_transition_idx];

        while (prev_transition.previous_l1_index == state.last_l1_idx_seen) {
          offset += prev_transition.size;
          prev_transition_idx--;
          prev_transition = transition_offsets_buf[prev_transition_idx];
        }
      } else {
        XTC2Traits::EventOffset prev_l1 = l1_offsets_buf[state.l1_offset_idx - 1];
        offset = prev_l1.offset + prev_l1.size;
      }

      auto buf_off = state.events_per_read
        ? state.trans_offset_idx % state.events_per_read
        : state.trans_offset_idx;

      new (transition_offsets_buf + buf_off)
        XTC2Traits::TransitionOffset(offset,
                                     size,
                                     state.last_l1_idx_seen,
                                     dg->service());

      state.trans_offset_idx++;
      if (dg->service() == XTC2::TransitionId::EndRun) {
        state.seen_end_run = true;
      }
    }

    return sizeof(*dg) + payload_size;
  }

  // --- XTC2Traits::MetadataInventory (Plus its internal structs too) --- //
  SBIO_HD std::uint32_t
  XTC2Traits::MetadataInventory::get_sd_offset(std::uint32_t nid) const {
    auto* it = std::lower_bound(m_sd_offsets, m_sd_offsets + m_sd_offset_count, nid);

    if (it != m_sd_offsets + m_sd_offset_count && it->names_id == nid) {
      return it->offset;
    }

    return 0;
  }

  SBIO_HD const XTC2::Name*
  XTC2Traits::MetadataInventory::get_schema(std::uint32_t nid) const {
    auto* it = std::lower_bound(m_schema_offsets, m_schema_offsets + m_schema_offset_count, nid);

    if (it != m_schema_offsets + m_schema_offset_count && it->names_id == nid) {
      return &m_schemas[it->start_idx];
    }

    return nullptr;
  }

  SBIO_HD std::uint32_t
  XTC2Traits::MetadataInventory::resolve_names_id(const DataRequest& req) const {
    DetAlgKey search_key;
    XTC2::safe_strncpy(search_key.dettype, req.detector_type);
    XTC2::safe_strncpy(search_key.detname, req.detector_name);
    search_key.segment = req.segment_number;
    XTC2::safe_strncpy(search_key.algname, req.alg_name);

    auto* it = std::lower_bound(m_names_id_table, m_names_id_table + m_names_id_count, search_key);

    if (it != m_names_id_table + m_names_id_count && !(search_key < it->key)) {
      return it->names_id;
    }
    return 0xFFFFFFFF;
  }

  SBIO_HD std::uint32_t
  XTC2Traits::MetadataInventory::resolve_field_idx(std::uint32_t nid, const char* field) const {
    FieldKey search_key { nid };
    XTC2::safe_strncpy(search_key.fieldname, field);
    auto* it = std::lower_bound(m_field_table, m_field_table + m_field_count, search_key);

    if (it != m_field_table + m_field_count && !(search_key < it->key)) {
      return it->field_idx;
    }
    return 0xFFFFFFFF;
  }
} // namespace sbio
