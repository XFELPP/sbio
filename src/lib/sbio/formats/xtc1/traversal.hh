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

#ifndef SBIO_FORMATS_XTC1_TRAVERSAL_HH
#define SBIO_FORMATS_XTC1_TRAVERSAL_HH

#include "sbio/formats/xtc1/registry.hh"
#include "sbio/formats/xtc1/xtc1.hh"
#include "sbio/formats/xtc1/xtc1_schemas.hh"
#include "sbio/util/string.hh"

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  namespace XTC1 {
    // Helper functions to map Detector/Device IDs to string names
    SBIO_HD const char* get_detector_name(std::uint8_t det_id);
    SBIO_HD const char* get_device_name(std::uint8_t dev_id);
    SBIO_HD const char* get_bld_name(std::uint32_t bld_type);

    inline std::string format_src_fullname(const Src& src) {
      if (src.level() == Level::Source) {
        std::uint32_t phy { src.phy() };
        std::uint32_t det { (phy >> 24) & 0xFF };
        std::uint32_t det_id { (phy >> 16) & 0xFF };
        std::uint32_t dev { (phy >> 8) & 0xFF };
        std::uint32_t dev_id { phy & 0xFF };

        char buf[MaxNameSize];
        std::snprintf(buf,
                      sizeof(buf),
                      "%s.%u:%s.%u",
                      get_detector_name(det),
                      det_id,
                      get_device_name(dev),
                      dev_id);

        return std::string(buf);
      } else if (src.level() == Level::Reporter) {
        return get_bld_name(src.phy());
      }

      return "Unknown";
    }

    struct MetadataCollector {
    public:
      // Populates the alias map by pre-scanning for AliasConfig
      SBIO_HD inline void populate_aliases(Xtc* xtc) {
        if (xtc->contains.id() == TypeId::Id_AliasConfig) {
          const char* payload { xtc->payload() };

          if (payload && xtc->sizeofPayload() >= 4) {
            std::uint32_t numSrcAlias { *reinterpret_cast<const std::uint32_t*>(payload) };
            const char* array_ptr { payload + 4 };

            for (std::uint32_t i = 0; i < numSrcAlias; ++i) {
              const char* src_alias_ptr { array_ptr + i * 40 };
              const Src& src { *reinterpret_cast<const Src*>(src_alias_ptr) };
              const char* alias_name { src_alias_ptr + 8 };

              m_alias_map[{src.level(), src.phy()}] = std::string(alias_name);
            }
          }
        }

        // Recurse if container
        if (xtc->contains.id() == TypeId::Id_Xtc) {
          const char* child_ptr { xtc->payload() };
          const char* end_ptr { child_ptr + xtc->sizeofPayload() };
          while (child_ptr + sizeof(Xtc) <= end_ptr) {
            auto* child = reinterpret_cast<Xtc*>(const_cast<char*>(child_ptr));
            populate_aliases(child);
            child_ptr += child->extent;
          }
        }
      }

      SBIO_HD inline void add_detector(XTC1::Xtc* xtc, std::uint32_t offset) {
        const Src& src { xtc->src };

        // Skip non-detector/non-BLD levels
        if (src.level() != Level::Source && src.level() != Level::Reporter) {
          return;
        }

        // Avoid registering duplicates
        if (m_registered_srcs.find(src) != m_registered_srcs.end()) {
          return;
        }

        std::uint32_t name_id { static_cast<std::uint32_t>(m_detectors.size() + 1) };
        m_registered_srcs[src] = name_id;

        // Determine detector type and primary name
        TypeId::Type type { xtc->contains.id() };
        std::uint32_t version { xtc->contains.version() };
        const char* type_name { TypeId::name(type) };

        std::string primary_name;
        auto alias_it = m_alias_map.find({src.level(), src.phy()});

        if (alias_it != m_alias_map.end() && !alias_it->second.empty()) {
          primary_name = alias_it->second; // e.g., "epix10k2M"
        } else {
          primary_name = format_src_fullname(src); // e.g., "EBeam" or "MfxEndstation.0:Epix10ka2M.0"
        }

        // Segment number is the lowest byte of the physical source ID
        std::uint32_t segment { src.phy() & 0xFF };
        safe_strncpy(m_detectors[name_id].name, primary_name.c_str(), XTC1::MaxNameSize);
        safe_strncpy(m_detectors[name_id].type, type_name, XTC1::MaxNameSize);
        m_detectors[name_id].segment = segment;

        m_sd_offsets_discovered[name_id] = offset;
        m_src_logs[name_id] = src.log();
        m_src_phys[name_id] = src.phy();

        // Register fields from the schema registry
        auto* schema = find_schema(type, version);
        if (schema != nullptr) {
          // For BLD do not use offsets stored from lookup of the Configure transition
          // Force resolution based on the type via full traversal of the L1Accept XTCs
          bool is_bld { (src.level() == Level::Reporter) };

          for (std::uint32_t f_idx = 0; f_idx < schema->fields.size(); ++f_idx) {
            const auto& field { schema->fields[f_idx] };
            m_fields[name_id].emplace_back(field.name.data(),
                                           field.rank,
                                           field.dtype,
                                           f_idx,
                                           true,
                                           !is_bld,
                                           is_bld ? type : 0);
          }

          // Register the corresponding event fields dynamically
          if (schema->event_type != TypeId::Any) {
            std::uint32_t evt_version { 1 };
            if (schema->event_type == TypeId::Id_EpixElement) {
              if (type == TypeId::Id_Epix10kaConfig) {
                evt_version = 3;
              } else if (type == TypeId::Id_Epix100aConfig) {
                evt_version = 2;
              }
            }

            auto* evt_schema = find_schema(schema->event_type, evt_version);
            if (evt_schema != nullptr) {
              for (std::uint32_t f_idx = 0; f_idx < evt_schema->fields.size(); ++f_idx) {
                const auto& field = evt_schema->fields[f_idx];
                bool duplicate { false };

                for (const auto& existing : m_fields[name_id]) {
                  if (existing.name == field.name.data()) {
                    duplicate = true;
                    break;
                  }
                }

                if (!duplicate) {
                  m_fields[name_id].emplace_back(field.name.data(),
                                                 field.rank,
                                                 field.dtype,
                                                 f_idx,
                                                 false,
                                                 schema->event_type);
                }
              }
            }
          }
        }
      }

      SBIO_HD inline void add_detector(std::uint32_t name_id,
                                       const char* name,
                                       const char* type,
                                       unsigned segment,
                                       std::uint32_t offset) {
        safe_strncpy(m_detectors[name_id].name, name, XTC1::MaxNameSize);
        safe_strncpy(m_detectors[name_id].type, type, XTC1::MaxNameSize);
        m_detectors[name_id].segment = segment;
        m_sd_offsets_discovered[name_id] = offset;
      }

      SBIO_HD inline void add_field(std::uint32_t name_id,
                                    std::uint32_t field_idx,
                                    const char* field_name,
                                    XTC1::DType dtype,
                                    std::uint32_t rank) {
        m_fields[name_id].emplace_back(field_name, rank, dtype);
      }

      SBIO_HD inline void add_sd_offset(std::uint32_t nid, std::uint32_t offset) {
        m_sd_offsets_discovered[nid] = offset;
      }

      SBIO_HD inline const std::map<std::uint32_t, XTC1::XtcDetectorDescr>&
      detectors() const { return m_detectors; }

      SBIO_HD inline const std::map<std::uint32_t, std::vector<XTC1::XtcFieldDescr>>&
      fields() const { return m_fields; }

      SBIO_HD inline const std::map<std::uint32_t, std::uint32_t>&
      sd_offsets() const { return m_sd_offsets_discovered; }

      SBIO_HD inline const std::map<std::uint32_t, std::uint32_t>&
      src_logs() const { return m_src_logs; }

      SBIO_HD inline const std::map<std::uint32_t, std::uint32_t>&
      src_phys() const { return m_src_phys; }

    private:
      std::map<std::uint32_t, XTC1::XtcDetectorDescr> m_detectors;
      std::map<std::uint32_t, std::vector<XTC1::XtcFieldDescr>> m_fields;
      std::map<std::uint32_t, std::uint32_t> m_sd_offsets_discovered;
      std::map<std::uint32_t, std::uint32_t> m_src_logs;
      std::map<std::uint32_t, std::uint32_t> m_src_phys;
      // Holds alias mappings: Src -> DAQ Alias
      std::map<std::pair<Level::Type, std::uint32_t>, std::string> m_alias_map;
      // Map to check if a detector has already been registered
      std::map<Src, std::uint32_t> m_registered_srcs;
    };

    SBIO_HD void inspect_xtc1(Xtc* xtc,
                              MetadataCollector& collector,
                              std::uint32_t current_offset = 0);
  } // namespace XTC1
} // namespace sbio

#endif // SBIO_FORMATS_XTC1_TRAVERSAL_HH
