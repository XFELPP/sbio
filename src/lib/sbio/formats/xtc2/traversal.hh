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

#ifndef SBIO_FORMATS_XTC2_TRAVERSAL_HH
#define SBIO_FORMATS_XTC2_TRAVERSAL_HH

#include "sbio/formats/xtc2/xtc2.hh"

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <vector>

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

namespace sbio {
  namespace XTC2 {
    inline void safe_strncpy(char* dst, const char* src) {
      std::size_t i { 0 };
      for (; i < XTC2::MaxNameSize - 1 && src[i] != '\0'; ++i) {
        dst[i] = src[i];
      }
      dst[i] = '\0';
    }

    struct MetadataCollector {
    public:
      SBIO_HD inline void add_detector(std::uint32_t name_id,
                                       const char* name,
                                       const char* alg,
                                       const char* type,
                                       unsigned segment,
                                       std::uint32_t offset) {
        safe_strncpy(m_detectors[name_id].name, name);
        safe_strncpy(m_detectors[name_id].type, type);
        safe_strncpy(m_detectors[name_id].alg, alg);
        m_detectors[name_id].segment = segment;
        m_sd_offsets_discovered[name_id] = offset;
      }

      SBIO_HD inline void add_field(std::uint32_t name_id,
                                    std::uint32_t field_idx,
                                    const char* field_name,
                                    XTC2::DType dtype,
                                    std::uint32_t rank) {
        m_fields[name_id].emplace_back(field_name, rank, dtype);
      }

      SBIO_HD inline void add_sd_offset(std::uint32_t nid, std::uint32_t offset) {
        m_sd_offsets_discovered[nid] = offset;
      }

      SBIO_HD inline const std::map<std::uint32_t, XTC2::XtcDetectorDescr>&
      detectors() const { return m_detectors; }

      SBIO_HD inline const std::map<std::uint32_t, std::vector<XTC2::XtcFieldDescr>>&
      fields() const { return m_fields; }

      SBIO_HD inline const std::map<std::uint32_t, std::uint32_t>&
      sd_offsets() const { return m_sd_offsets_discovered; }

    private:
      std::map<std::uint32_t, XTC2::XtcDetectorDescr> m_detectors;
      std::map<std::uint32_t, std::vector<XTC2::XtcFieldDescr>> m_fields;
      std::map<std::uint32_t, std::uint32_t> m_sd_offsets_discovered;
    };

    SBIO_HD void handle_xtc2_names(XTC2::Names* names,
                                   MetadataCollector& collector,
                                   std::uint32_t offset);

    SBIO_HD void handle_xtc2_shapes_data(XTC2::ShapesData* sd,
                                         MetadataCollector& collector,
                                         std::uint32_t current_offset);

    SBIO_HD void inspect_xtc2(XTC2::Xtc* xtc,
                              MetadataCollector& collector,
                              std::uint32_t current_offset = 0);
  } // namespace XTC2
};

#endif // SBIO_FORMATS_XTC2_TRAVERSAL_HH
