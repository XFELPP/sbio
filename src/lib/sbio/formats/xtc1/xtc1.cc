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

#include "sbio/formats/xtc1/xtc1.hh"

#include "sbio/formats/xtc1/registry.hh"
#include "sbio/formats/xtc1/xtc1_schemas.hh"
#include "sbio/formats/xtc1/xtc1_traits.hh"

#include <cstdint>
#include <cstring>
#include <string_view>

namespace {
  SBIO_HD std::uint32_t
  get_sd_offset_local_helper(const sbio::XTC1Traits::MetadataInventory& inv,
                             std::uint32_t nid) {
    for (std::size_t i = 0; i < inv.m_sd_offset_count; ++i) {
      if (inv.m_sd_offsets[i].names_id == nid) {
        return inv.m_sd_offsets[i].offset;
      }
    }
    return 0;
  }

  SBIO_HD std::uint32_t
  resolve_field_idx_local_helper(const sbio::XTC1Traits::MetadataInventory& inv,
                                 std::uint32_t nid,
                                 const char* field) {
    for (std::size_t i = 0; i < inv.m_field_count; ++i) {
      if (inv.m_field_table[i].key.names_id == nid &&
          std::strcmp(inv.m_field_table[i].key.fieldname, field) == 0) {
        return inv.m_field_table[i].field_idx;
      }
    }
    return 0xFFFFFFFF;
  }

  SBIO_HD std::uint64_t get_schema_constant(const std::string_view& name) {
    for (const auto& schema : sbio::XTC1::g_schema_registry) {
      for (const auto& c : schema.constants) {
        if (c.name == name) {
          return c.value;
        }
      }
    }
    return 0;
  }

  SBIO_HD std::uint64_t get_schema_constant_local(const std::string_view& name,
                                                  const void* inv_ptr,
                                                  std::uint32_t nid) {
    if (!inv_ptr) {
      return get_schema_constant(name);
    }
    auto& inv = *reinterpret_cast<const sbio::XTC1Traits::MetadataInventory*>(inv_ptr);

    char name_buf[256];
    std::size_t name_len { std::min(name.size(), sizeof(name_buf) - 1) };
    std::memcpy(name_buf, name.data(), name_len);
    name_buf[name_len] = '\0';

    std::uint32_t f_idx { resolve_field_idx_local_helper(inv, nid, name_buf) };
    if (f_idx != 0xFFFFFFFF && inv.m_config_buffer != nullptr) {
      std::uint32_t sd_offset { get_sd_offset_local_helper(inv, nid) };
      const auto* xtc =
        reinterpret_cast<const sbio::XTC1::Xtc*>(reinterpret_cast<const char*>(inv.m_config_buffer) + sd_offset);

      sbio::XTC1::TypeId::Type type { static_cast<sbio::XTC1::TypeId::Type>(xtc->contains.id()) };
      std::uint32_t version { xtc->contains.version() };

      auto* schema { sbio::XTC1::find_schema(type, version) };
      if (schema && f_idx < schema->fields.size()) {
        const auto& f { schema->fields[f_idx] };
        const char* field_ptr { xtc->payload() + f.offset };

        if (f.dtype == sbio::XTC1::DType::UINT32) {
          return *reinterpret_cast<const std::uint32_t*>(field_ptr);
        } else if (f.dtype == sbio::XTC1::DType::UINT16) {
          return *reinterpret_cast<const std::uint16_t*>(field_ptr);
        } else if (f.dtype == sbio::XTC1::DType::UINT8) {
          return *reinterpret_cast<const std::uint8_t*>(field_ptr);
        } else if (f.dtype == sbio::XTC1::DType::INT32) {
          return *reinterpret_cast<const std::int32_t*>(field_ptr);
        } else if (f.dtype == sbio::XTC1::DType::INT16) {
          return *reinterpret_cast<const std::int16_t*>(field_ptr);
        } else if (f.dtype == sbio::XTC1::DType::INT8) {
          return *reinterpret_cast<const std::int8_t*>(field_ptr);
        }
      }
    }

    const char* dettype = nullptr;
    for (std::size_t i = 0; i < inv.m_names_id_count; ++i) {
      if (inv.m_names_id_table[i].names_id == nid) {
        dettype = inv.m_names_id_table[i].key.dettype;
        break;
      }
    }
    if (!dettype) {
      return get_schema_constant(name);
    }
    for (const auto& schema : sbio::XTC1::g_schema_registry) {
      const char* schema_name = sbio::XTC1::TypeId::name(schema.type);
      if (schema_name && std::strcmp(schema_name, dettype) == 0) {
        for (const auto& c : schema.constants) {
          if (c.name == name) {
            return c.value;
          }
        }
      }
    }
    return get_schema_constant(name);
  }
} // anonymous namespace

namespace sbio::XTC1 {
  SBIO_HD const Xtc* find_xtc_by_type(const Xtc* xtc,
                                      TypeId::Type target,
                                      std::uint32_t src_log,
                                      std::uint32_t src_phy) {
    if (!xtc || xtc->extent < sizeof(Xtc)) {
      return nullptr;
    }

    if (xtc->damage.value() & (1 << Damage::DroppedContribution)) {
      return nullptr;
    }

    if (xtc->contains.id() == static_cast<std::uint32_t>(target)) {
      if ((src_log == 0 && src_phy == 0) || (xtc->src.log() == src_log && xtc->src.phy() == src_phy)) {
        return xtc;
      }
    }

    if (xtc->contains.id() == static_cast<std::uint32_t>(TypeId::Id_Xtc)) {
      const char* p { xtc->payload() };
      const char* end { p + xtc->sizeofPayload() };
      while (p + sizeof(Xtc) <= end) {
        const Xtc* child { reinterpret_cast<const Xtc*>(p) };
        if (child->extent < sizeof(Xtc)) {
          break;
        }

        const Xtc* found { find_xtc_by_type(child, target, src_log, src_phy) };
        if (found) {
          return found;
        }

        p += child->extent;
      }
    }

    return nullptr;
  }

  SBIO_HD DataResult resolve_xtc_pointer(void* buffer,
                                         std::uint32_t sd_offset,
                                         const Name* field_schema,
                                         std::uint32_t nid,
                                         std::uint32_t f_idx,
                                         std::uint32_t evt_type_id,
                                         const void* inv,
                                         std::uint32_t src_log,
                                         std::uint32_t src_phy) {
    DataResult result {};

    const Xtc* xtc { nullptr };

    // Get the XTC payload by type lookup or event offset.
    if (evt_type_id != 0 && evt_type_id != static_cast<std::uint32_t>(TypeId::Id_Xtc)) {
      // Scan for the matching TypeId
      auto* dg { reinterpret_cast<Dgram*>(buffer) };
      xtc = find_xtc_by_type(&dg->xtc, static_cast<TypeId::Type>(evt_type_id), src_log, src_phy);

      if (!xtc) {
        return result;
      }
    } else {
      // Config data: Jump to the recorded offset
      xtc = reinterpret_cast<const Xtc*>(reinterpret_cast<char*>(buffer) + sd_offset);
    }

    const char* payload { xtc->payload() };

    TypeId::Type type { xtc->contains.id() };
    std::uint32_t version { xtc->contains.version() };

    auto* schema = find_schema(type, version);
    if (schema == nullptr || f_idx >= schema->fields.size()) {
      return result; // Schema type or field index out of range
    }

    const auto& field = schema->fields[f_idx];
    std::uint32_t offset { field.offset };

    for (std::uint32_t i = 0; i < MaxRank; ++i) {
      std::string_view of { field.offset_fields[i] };
      if (!of.empty()) {
        std::uint64_t val { get_schema_constant_local(of, inv, nid) };
        offset += field.offset_multipliers[i] * val;
      }
    }

    result.data = payload + offset;
    result.dtype = field.dtype;
    result.rank = field.rank;
    result.size = DTypeSize[static_cast<int>(field.dtype)];

    if (!field.is_dynamic) {
      std::memcpy(result.shape, field.shape, sizeof(result.shape));
      std::size_t num_elements = 1;
      for (std::uint32_t r = 0; r < field.rank; ++r) {
        num_elements *= field.shape[r];
      }
      result.size *= num_elements;
    } else {
      if (type == TypeId::Id_Arraychar && version == 1 && f_idx == 1) {
        // Read numChars from offset 0
        std::uint64_t num_chars = *reinterpret_cast<const std::uint64_t*>(payload);
        result.shape[0] = num_chars;
        result.size = num_chars * sizeof(uint8_t);
      } else if (type == TypeId::Id_Frame && version == 1 && f_idx == 4) {
        // Read width and height from offsets 0 and 4
        std::uint32_t w = *reinterpret_cast<const std::uint32_t*>(payload);
        std::uint32_t h = *reinterpret_cast<const std::uint32_t*>(payload + 4);
        std::uint32_t depth = *reinterpret_cast<const std::uint32_t*>(payload + 8);
        std::uint32_t depth_bytes = (depth + 7) / 8;

        result.shape[0] = h;
        result.shape[1] = w;
        result.size = h * w * depth_bytes;
      } else {
        bool all_resolved { true };
        std::uint32_t unresolved_idx { 0xFFFFFFFF };
        std::uint32_t resolved_product { 1 };

        for (std::uint32_t r = 0; r < field.rank; ++r) {
          std::string_view sf { field.shape_fields[r] };
          if (sf.empty()) {
            all_resolved = false;
            unresolved_idx = r;
            continue;
          }

          //std::uint64_t val { get_schema_constant(sf) };
          std::uint64_t val { get_schema_constant_local(sf, inv, nid) };
          if (val == 0) {
            all_resolved = false;
            unresolved_idx = r;
          } else {
            result.shape[r] = val;
            resolved_product *= val;
          }
        }

        if (all_resolved) {
          //result.size = resolved_product * DTypeSize[static_cast<int>(field.dtype)];
          std::uint32_t expected_size {
            resolved_product * DTypeSize[static_cast<int>(field.dtype)]
          };
          std::uint32_t actual_payload_size {
              xtc->sizeofPayload() > offset ? xtc->sizeofPayload() - offset : 0
          };

          if (expected_size > actual_payload_size && actual_payload_size > 0) {
            std::uint32_t rest_product { resolved_product / result.shape[0] };
            std::uint32_t elem_size { DTypeSize[static_cast<int>(field.dtype)] };
            std::uint32_t total_elements { actual_payload_size / elem_size };
            if (rest_product > 0 && (total_elements % rest_product == 0)) {
              result.shape[0] = total_elements / rest_product;
              expected_size = total_elements * elem_size;
            }
          }
          result.size = expected_size;
        } else if (unresolved_idx != 0xFFFFFFFF) {
          std::uint32_t field_payload_size { xtc->sizeofPayload() - offset };
          std::uint32_t elem_size { DTypeSize[static_cast<int>(field.dtype)] };
          std::uint32_t total_elements { field_payload_size / elem_size };
          if (resolved_product > 0 && (total_elements % resolved_product == 0)) {
            result.shape[unresolved_idx] = total_elements / resolved_product;
            result.size = total_elements * elem_size;
          }
        }
      }
    }

    return result;
  }
} // namespace sbio::XTC1
