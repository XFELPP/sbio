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

#include "sbio/formats/xtc2/traversal.hh"

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
    SBIO_HD void handle_xtc2_names(XTC2::Names* names,
                                   MetadataCollector& collector,
                                   std::uint32_t offset) {
      std::uint32_t nid = names->namesId();

      collector.add_detector(nid,
                             names->detName(),
                             names->alg().name(),
                             names->detType(),
                             names->detId(),
                             names->segment(),
                             offset);

      for (std::uint32_t i = 0; i < names->num(); ++i) {
        const XTC2::Name& field = names->get(i);
        collector.add_field(nid, i, field.name(), field.type(), field.rank());
      }
    }


    SBIO_HD void handle_xtc2_shapes_data(XTC2::ShapesData* sd,
                                         MetadataCollector& collector,
                                         std::uint32_t current_offset) {
      std::uint32_t nid = sd->namesId();

      // Record the logical offset to be used for fast lookup from the start of Dgram
      std::uint32_t offset =
        reinterpret_cast<const char*>(sd->data().payload()) - reinterpret_cast<const char*>(sd);
      collector.add_sd_offset(nid, current_offset + offset);
    }

    SBIO_HD void inspect_xtc2(XTC2::Xtc* xtc,
                              MetadataCollector& collector,
                              std::uint32_t current_offset) {
      switch (xtc->contains.id()) {
      case (XTC2::TypeId::Parent): {
        int remaining = xtc->sizeofPayload();
        XTC2::Xtc* subxtc = reinterpret_cast<XTC2::Xtc*>(xtc->payload());
        while (remaining > 0) {
          inspect_xtc2(subxtc, collector, current_offset + sizeof(XTC2::Xtc));
          remaining -= subxtc->extent;
          subxtc = subxtc->next();
        }
        break;
      }
      case (XTC2::TypeId::Names): {
        handle_xtc2_names(static_cast<XTC2::Names*>(xtc),
                          collector,
                          current_offset);
        break;
      }
      case (XTC2::TypeId::ShapesData): {
        handle_xtc2_shapes_data(static_cast<XTC2::ShapesData*>(xtc),
                                collector,
                                current_offset);
        break;
      }
      default:
        break;
      }
    }
  } // namespace XTC2
} // namespace sbio
