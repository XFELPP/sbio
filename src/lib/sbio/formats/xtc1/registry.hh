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

#ifndef SBIO_FORMATS_XTC1_REGISTRY_HH
#define SBIO_FORMATS_XTC1_REGISTRY_HH

#include "sbio/formats/xtc1/xtc1.hh"

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <span>

namespace sbio::XTC1 {
  struct SimulatedField {
    std::string_view name;
    DType dtype;
    std::uint32_t rank;
    std::uint32_t shape[MaxRank];
    std::uint32_t offset;
    bool is_dynamic;
    std::string_view shape_fields[MaxRank];
    std::uint32_t offset_multipliers[MaxRank] {};
    std::string_view offset_fields[MaxRank] {};
  };

  struct SchemaConstant {
    std::string_view name;
    std::uint64_t value;
  };

  struct SimulatedSchema {
    TypeId::Type type;
    std::uint32_t version;
    std::span<const SimulatedField> fields;
    std::span<const SchemaConstant> constants {};
    TypeId::Type event_type { TypeId::Any }; // Map schema to event schema. E.g. Epix10ka2MConfig -> Epix10kaArray
  };

  struct SchemaKey {
    TypeId::Type type;
    std::uint32_t version;

    bool operator==(const SchemaKey& other) const {
      return type == other.type && version == other.version;
    }
  };
} // namespace sbio::XTC1

namespace std {
  template <>
  struct hash<sbio::XTC1::SchemaKey> {
    std::size_t operator()(const sbio::XTC1::SchemaKey& k) const {
      return (static_cast<std::size_t>(k.type) << 16) | k.version;
    }
  };
} // namespace std

#endif // SBIO_FORMATS_XTC1_REGISTRY_HH
