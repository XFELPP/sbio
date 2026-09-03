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

      auto g_id { inv.register_group(det.name, det.type, det.segment, ptn) };

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
          auto alias_g_id = inv.register_group_alias(f_descr.name, "epics", det.segment, ptn);
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
          auto alias_g_id = inv.register_group_alias(f_descr.name, "scan", det.segment, ptn);
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
                                              const XTC2Traits::DataRequest& req) {
    auto* entry { inv.lookup(req) };
    if (entry == nullptr) {
      return {};
    }

    const auto& descr { entry->descriptor };
    const auto& xtc2_meta { descr.format_meta };

    std::uint32_t nid { xtc2_meta.names_id };
    std::uint32_t f_idx { xtc2_meta.field_idx };
    const XTC2::Name* field_schema { &(xtc2_meta.field_name) };

    std::uint32_t sd_offset { descr.payload_offset };
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

      descr.payload_offset = total_offset;
      sd_offset = total_offset;
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
} // namespace sbio
