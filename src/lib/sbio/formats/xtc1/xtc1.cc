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

#include <cstdint>
#include <cstring>
#include <span>
#include <string_view>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace {
  using sbio::XTC1::DType;
  using sbio::XTC1::SimulatedSchema;
  using sbio::XTC1::Xtc;

  /**
   * Read a scalar (integral) configuration value of a field from a payload.
   *
   * @returns Whether the field's type could be read as an integer.
   */
  SBIO_HD bool read_scalar(const char* ptr, DType dtype, std::uint64_t& value) {
    switch (dtype) {
    case DType::UINT8:   { value = *reinterpret_cast<const std::uint8_t*>(ptr); return true; }
    case DType::UINT16:  { value = *reinterpret_cast<const std::uint16_t*>(ptr); return true; }
    case DType::UINT32:  { value = *reinterpret_cast<const std::uint32_t*>(ptr); return true; }
    case DType::UINT64:  { value = *reinterpret_cast<const std::uint64_t*>(ptr); return true; }
    case DType::INT8:    { value = *reinterpret_cast<const std::int8_t*>(ptr); return true; }
    case DType::INT16:   { value = *reinterpret_cast<const std::int16_t*>(ptr); return true; }
    case DType::INT32:   { value = *reinterpret_cast<const std::int32_t*>(ptr); return true; }
    case DType::INT64:   { value = *reinterpret_cast<const std::int64_t*>(ptr); return true; }
    case DType::ENUMVAL: { value = *reinterpret_cast<const std::int32_t*>(ptr); return true; }
    default:             { return false; }
    }

    return 0;
  }

  SBIO_HD bool schema_constant(const SimulatedSchema* schema,
                               std::string_view name,
                               std::uint64_t& value) {
    if (schema == nullptr) {
      return false;
    }

    for (const auto& c : schema->constants) {
      if (c.name == name) {
        value = c.value;
        return true;
      }
    }

    return false;
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

  /**
   * Value of a name used in a field's shape or offset expression.
   *
   * Names are looked up, in order, as:
   * 1. A scalar field of the detector's configuration (e.g. `numberOfModules`).
   * 2. A constant of the configuration's schema.
   * 3. A constant of the schema of the Xtc being resolved.
   * 4. A constant of any schema (first match).
   *
   * @param[in] name The name to look up.
   * @param[in] config_xtc The detector's configuration Xtc (may be nullptr).
   * @param[in] own_schema The schema of the Xtc being resolved (may be nullptr).
   */
  SBIO_HD std::uint64_t get_schema_constant_local(const std::string_view& name,
                                                  const Xtc* config_xtc,
                                                  const SimulatedSchema* own_schema) {
    std::uint64_t value { 0 };

    if (config_xtc != nullptr) {
      const auto* cfg_schema {
        sbio::XTC1::find_schema(config_xtc->contains.id(), config_xtc->contains.version())
      };

      if (cfg_schema != nullptr) {
        for (const auto& f : cfg_schema->fields) {
          if (f.name == name && f.rank == 0 &&
              f.offset + sbio::XTC1::DTypeSize[static_cast<int>(f.dtype)] <=
                static_cast<std::uint32_t>(config_xtc->sizeofPayload()) &&
              read_scalar(config_xtc->payload() + f.offset, f.dtype, value)) {
            return value;
          }
        }

        if (schema_constant(cfg_schema, name, value)) {
          return value;
        }
      }
    }

    if (schema_constant(own_schema, name, value)) {
      return value;
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

  SBIO_HD DataResult resolve_xtc_pointer(const Xtc* xtc,
                                         const char* field_name,
                                         std::uint32_t field_idx_hint,
                                         const Xtc* config_xtc) {
    DataResult result {};

    if (xtc == nullptr || field_name == nullptr) {
      return result;
    }

    const char* payload { xtc->payload() };

    TypeId::Type type { xtc->contains.id() };
    std::uint32_t version { xtc->contains.version() };

    auto* schema = find_schema(type, version);
    if (schema == nullptr) {
      return result; // Unknown schema type/version
    }

    // The index recorded at Configure may be for another version of the type (e.g. an
    // event type's version is only known from the events). Check it by name.
    const std::string_view name { field_name };
    std::uint32_t f_idx { field_idx_hint };
    if (f_idx >= schema->fields.size() || schema->fields[f_idx].name != name) {
      f_idx = static_cast<std::uint32_t>(schema->fields.size());
      for (std::uint32_t i = 0; i < schema->fields.size(); ++i) {
        if (schema->fields[i].name == name) {
          f_idx = i;
          break;
        }
      }

      if (f_idx >= schema->fields.size()) {
        return result; // No such field for this version
      }
    }

    const auto& field = schema->fields[f_idx];
    std::uint32_t offset { field.offset };

    for (std::uint32_t i = 0; i < MaxRank; ++i) {
      std::string_view of { field.offset_fields[i] };
      if (!of.empty()) {
        std::uint64_t val { get_schema_constant_local(of, config_xtc, schema) };
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

        // For an image the schema's flat pixel array is (height, width), with the pixel
        // type following the bit depth
        result.rank = 2;
        result.shape[0] = h;
        result.shape[1] = w;
        result.dtype = (depth_bytes > 1) ? DType::UINT16 : DType::UINT8;
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

          std::uint64_t val { get_schema_constant_local(sf, config_xtc, schema) };
          if (val == 0) {
            all_resolved = false;
            unresolved_idx = r;
          } else {
            result.shape[r] = val;
            resolved_product *= val;
          }
        }

        const std::uint32_t elem_size { DTypeSize[static_cast<int>(field.dtype)] };
        const std::uint32_t payload_size { static_cast<std::uint32_t>(xtc->sizeofPayload()) };
        const std::uint32_t field_payload_size { payload_size > offset ? payload_size - offset : 0 };

        if (all_resolved) {
          std::uint32_t expected_size { resolved_product * elem_size };

          if (expected_size > field_payload_size && field_payload_size > 0) {
            std::uint32_t rest_product { resolved_product / result.shape[0] };
            std::uint32_t total_elements { field_payload_size / elem_size };
            if (rest_product > 0 && (total_elements % rest_product == 0)) {
              result.shape[0] = total_elements / rest_product;
              expected_size = total_elements * elem_size;
            }
          }
          result.size = expected_size;
        } else if (unresolved_idx != 0xFFFFFFFF) {
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
