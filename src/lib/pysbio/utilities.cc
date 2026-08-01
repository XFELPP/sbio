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

#include "pysbio/utilities.hh"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <variant>
#include <vector>

namespace py = pybind11;

namespace pysbio {
  std::map<std::string, std::vector<std::string>> alg_fields(BrokerPtrV brokersv,
                                                             const char* detector_name,
                                                             std::size_t num_brokers,
                                                             std::string& serial_number_out) {
    std::map<std::string, std::unordered_set<std::string>> unique_metadata;
    std::map<std::uint32_t, std::string> segment_serial_nos;
    std::string detector_type;

    auto broker_visitor = [&](auto& brokers) {
      for (std::size_t i = 0; i < num_brokers; ++i) {
        using BrokerType = std::decay_t<decltype(*brokers[i])>;
        using InvT = typename BrokerType::StreamMetadata;
        // Access the broker's inventory
        auto& inv {
          const_cast<const InvT&>(brokers[i]->metadata())
        };

        for (std::size_t j = 0; j < inv.m_names_id_count; ++j) {
          auto const& key { inv.m_names_id_table[j].key };

          // Filter by this detector's name (e.g., "jungfrau")
          if (std::strcmp(key.detname, detector_name) == 0) {
            std::uint32_t nid { inv.m_names_id_table[j].names_id };

            // Setup serial numbers for calibdb usage
            segment_serial_nos[key.segment] = key.detId;
            if (detector_type.empty()) {
              detector_type = key.dettype;
            }

            // Find all fields associated with this NamesId
            for (std::size_t k = 0; k < inv.m_field_count; ++k) {
              if (inv.m_field_table[k].key.names_id == nid) {
                std::string working_alg_name;
                if constexpr (requires { key.algname; }) {
                  working_alg_name = key.algname;
                } else {
                  working_alg_name = "raw";
                }
                unique_metadata[working_alg_name].insert(inv.m_field_table[k].key.fieldname);
              }
            }
          }
        }
      }

      // If there are segment serial numbers, convert to the final LCLS form
      // The formatted serial number is: <detector_type>_<seg1>_<seg2>_...<segN>
      if (!segment_serial_nos.empty()) {
        serial_number_out = detector_type + "_";
        bool first { true };

        for (const auto& [seg, ser] : segment_serial_nos) {
          if (!first) {
            serial_number_out += "_";
          }

          serial_number_out += ser;
          first = false;
        }
      }

      // Convert to vectors for easier pybind11 handling
      std::map<std::string, std::vector<std::string>> results;
      for (auto& [alg, fields] : unique_metadata) {
        results[alg].assign(fields.begin(), fields.end());
      }
      return results;
    };

    return std::visit(broker_visitor, brokersv);
  }
} // namespace pysbio
