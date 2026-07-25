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

#include "pysbio/formats/pyformat_traits.hh"

#include "sbio/util/string.hh"

#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1.hh"
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif
#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2.hh"
#include "sbio/formats/xtc2/xtc2_traits.hh"
#endif

#include <pybind11/native_enum.h>
#include <pybind11/pybind11.h>

#include <cstdint>
#include <vector>

namespace py = pybind11;

PYBIND11_MODULE(pyformat_traits, ftraits_module, py::mod_gil_not_used()) {
  ftraits_module.doc() = "sbio Python bindings for the generic data format traits.";

  py::native_enum<pysbio::FTraits>(ftraits_module,
                                   "FTraits",
                                   "enum.Enum",
                                   "Enumerators for selecting data formats.")
    .value("XTC1", pysbio::FTraits::XTC1)
    .value("XTC2", pysbio::FTraits::XTC2)
    .export_values()
    .finalize();

#ifdef SBIO_HAS_XTC1

  py::native_enum<sbio::XTC1Traits::DataAccessPtn>(ftraits_module,
                                                   "XTC1DataAccessPtn",
                                                   "enum.Enum",
                                                   "Enumerators for access styles of XTC1 data.")
    .value("L1Accept", sbio::XTC1Traits::DataAccessPtn::L1Accept)
    .value("SlowUpdate", sbio::XTC1Traits::DataAccessPtn::SlowUpdate)
    .value("BeginStep", sbio::XTC1Traits::DataAccessPtn::BeginStep)
    .export_values()
    .finalize();

  py::classh<sbio::XTC1Traits::StreamParameters>(ftraits_module, "XTC1StreamParameters")
    .def_property("smd_path",
                  [](const sbio::XTC1Traits::StreamParameters& self) -> std::string {
                    return self.smd_path;
                  },
                  [](sbio::XTC1Traits::StreamParameters& self,
                     const std::string& val) {
                    sbio::safe_strncpy(self.smd_path,
                                       val.c_str(),
                                       sbio::XTC1::MaxNameSize);
                  })
    .def_property("xtc_path",
                  [](const sbio::XTC1Traits::StreamParameters& self) -> std::string {
                    return self.xtc_path;
                  },
                  [](sbio::XTC1Traits::StreamParameters& self, const std::string& val) {
                    sbio::safe_strncpy(self.xtc_path,
                                       val.c_str(),
                                       sbio::XTC1::MaxNameSize);
                  })

    .def_readwrite("max_dgram_size",
                   &sbio::XTC1Traits::StreamParameters::max_dgram_size)
    .def_readwrite("events_per_read",
                   &sbio::XTC1Traits::StreamParameters::events_per_read);

  py::classh<sbio::XTC1Traits::DiscoveryState>(ftraits_module, "XTC1StreamState")
    .def(py::init<
      ssize_t,
      std::uint64_t,
      bool,
      std::size_t,
      std::size_t,
      std::size_t,
      std::size_t,
      std::size_t,
      std::size_t,
      sbio::XTC1Traits::DataAccessPtn
    >())
    .def_readwrite("last_l1_idx_seen",
                   &sbio::XTC1Traits::DiscoveryState::last_l1_idx_seen)
    .def_readwrite("next_bd_offset",
                   &sbio::XTC1Traits::DiscoveryState::next_bd_offset)
    .def_readwrite("seen_end_run",
                   &sbio::XTC1Traits::DiscoveryState::seen_end_run)
    .def_readwrite("l1_offset_idx",
                   &sbio::XTC1Traits::DiscoveryState::l1_offset_idx)
    .def_readwrite("trans_offset_idx",
                   &sbio::XTC1Traits::DiscoveryState::trans_offset_idx)
    .def_readwrite("events_per_read",
                   &sbio::XTC1Traits::DiscoveryState::events_per_read)
    .def_readwrite("num_transitions",
                   &sbio::XTC1Traits::DiscoveryState::num_transitions)
    .def_readwrite("num_events",
                   &sbio::XTC1Traits::DiscoveryState::num_events)
    .def_readwrite("curr_smd_offset",
                   &sbio::XTC1Traits::DiscoveryState::curr_smd_offset)
    .def_readwrite("last_accessed_ptn",
                   &sbio::XTC1Traits::DiscoveryState::last_accessed_ptn);

  py::classh<sbio::XTC1Traits::DataRequest>(ftraits_module, "XTC1DataRequest")
    .def(py::init<const char*, const char*, const char*, const char*>())
    .def(py::init<const char*, const char*>())
    .def_property("detector_type",
                  [](const sbio::XTC1Traits::DataRequest& self) -> std::string {
                    return self.detector_type;
                  },
                  [](sbio::XTC1Traits::DataRequest& self, const std::string& val) {
                    sbio::safe_strncpy(self.detector_type, val.c_str(), sbio::XTC1::MaxNameSize);
                  })
    .def_property("detector_name",
                  [](const sbio::XTC1Traits::DataRequest& self) -> std::string {
                    return self.detector_name;
                  },
                  [](sbio::XTC1Traits::DataRequest& self, const std::string& val) {
                    sbio::safe_strncpy(self.detector_name, val.c_str(), sbio::XTC1::MaxNameSize);
                  })
    .def_readwrite("segment_number", &sbio::XTC1Traits::DataRequest::segment_number)
    .def_property("alg_name",
                  [](const sbio::XTC1Traits::DataRequest& self) -> std::string {
                    return self.alg_name;
                  },
                  [](sbio::XTC1Traits::DataRequest& self, const std::string& val) {
                    sbio::safe_strncpy(self.alg_name, val.c_str(), sbio::XTC1::MaxNameSize);
                  })
    .def_property("field_name",
                  [](const sbio::XTC1Traits::DataRequest& self) -> std::string {
                    return self.field_name;
                  },
                  [](sbio::XTC1Traits::DataRequest& self, const std::string& val) {
                    sbio::safe_strncpy(self.field_name, val.c_str(), sbio::XTC1::MaxNameSize);
                  });

  py::classh<sbio::XTC1::DataResult>(ftraits_module, "XTC1DataResult")
    .def(py::init<>())
    .def_readwrite("data", &sbio::XTC1::DataResult::data)
    .def_readwrite("size", &sbio::XTC1::DataResult::size)
    .def_readwrite("rank", &sbio::XTC1::DataResult::rank)
    .def_property("shape",
                  [](const sbio::XTC1::DataResult& self) {
                    return std::vector(self.shape, self.shape + self.rank);
                  },
                  [](sbio::XTC1::DataResult& self, const std::vector<ssize_t> shape) {
                    if (shape.size() < self.rank) {
                      throw std::runtime_error("Provided shape is smaller than the rank!");
                    }
                    for (std::size_t i = 0; i < self.rank; ++i) {
                      self.shape[i] = shape[i];
                    }
                  })
    .def_readwrite("dtype", &sbio::XTC1::DataResult::dtype);

#endif

#ifdef SBIO_HAS_XTC2

  py::native_enum<sbio::XTC2Traits::DataAccessPtn>(ftraits_module,
                                                   "XTC2DataAccessPtn",
                                                   "enum.Enum",
                                                   "Enumerators for access styles of XTC2 data.")
    .value("L1Accept", sbio::XTC2Traits::DataAccessPtn::L1Accept)
    .value("SlowUpdate", sbio::XTC2Traits::DataAccessPtn::SlowUpdate)
    .value("BeginStep", sbio::XTC2Traits::DataAccessPtn::BeginStep)
    .export_values()
    .finalize();

  py::classh<sbio::XTC2Traits::StreamParameters>(ftraits_module, "XTC2StreamParameters")
    .def_property("smd_path",
                  [](const sbio::XTC2Traits::StreamParameters& self) -> std::string {
                    return self.smd_path;
                  },
                  [](sbio::XTC2Traits::StreamParameters& self,
                     const std::string& val) {
                    sbio::safe_strncpy(self.smd_path,
                                       val.c_str(),
                                       sbio::XTC2::MaxNameSize);
                  })
    .def_property("xtc_path",
                  [](const sbio::XTC2Traits::StreamParameters& self) -> std::string {
                    return self.xtc_path;
                  },
                  [](sbio::XTC2Traits::StreamParameters& self, const std::string& val) {
                    sbio::safe_strncpy(self.xtc_path,
                                       val.c_str(),
                                       sbio::XTC2::MaxNameSize);
                  })
    .def_readwrite("max_dgram_size",
                   &sbio::XTC2Traits::StreamParameters::max_dgram_size)
    .def_readwrite("events_per_read",
                   &sbio::XTC2Traits::StreamParameters::events_per_read);

  py::classh<sbio::XTC2Traits::DiscoveryState>(ftraits_module, "XTC2StreamState")
    .def(py::init<
      ssize_t,
      std::uint64_t,
      bool,
      std::size_t,
      std::size_t,
      std::size_t,
      std::size_t,
      std::size_t,
      std::size_t,
      sbio::XTC2Traits::DataAccessPtn
    >())
    .def_readwrite("last_l1_idx_seen",
                   &sbio::XTC2Traits::DiscoveryState::last_l1_idx_seen)
    .def_readwrite("next_bd_offset",
                   &sbio::XTC2Traits::DiscoveryState::next_bd_offset)
    .def_readwrite("seen_end_run",
                   &sbio::XTC2Traits::DiscoveryState::seen_end_run)
    .def_readwrite("l1_offset_idx",
                   &sbio::XTC2Traits::DiscoveryState::l1_offset_idx)
    .def_readwrite("trans_offset_idx",
                   &sbio::XTC2Traits::DiscoveryState::trans_offset_idx)
    .def_readwrite("events_per_read",
                   &sbio::XTC2Traits::DiscoveryState::events_per_read)
    .def_readwrite("num_transitions",
                   &sbio::XTC2Traits::DiscoveryState::num_transitions)
    .def_readwrite("num_events",
                   &sbio::XTC2Traits::DiscoveryState::num_events)
    .def_readwrite("curr_smd_offset",
                   &sbio::XTC2Traits::DiscoveryState::curr_smd_offset)
    .def_readwrite("last_accessed_ptn",
                   &sbio::XTC2Traits::DiscoveryState::last_accessed_ptn);

  py::classh<sbio::XTC2Traits::DataRequest>(ftraits_module, "XTC2DataRequest")
    .def(py::init<const char*, const char*, const char*, const char*>())
    .def(py::init<const char*, const char*>())
    .def_property("detector_type",
                  [](const sbio::XTC2Traits::DataRequest& self) -> std::string {
                    return self.detector_type;
                  },
                  [](sbio::XTC2Traits::DataRequest& self, const std::string& val) {
                    sbio::safe_strncpy(self.detector_type, val.c_str(), sbio::XTC2::MaxNameSize);
                  })
    .def_property("detector_name",
                  [](const sbio::XTC2Traits::DataRequest& self) -> std::string {
                    return self.detector_name;
                  },
                  [](sbio::XTC2Traits::DataRequest& self, const std::string& val) {
                    sbio::safe_strncpy(self.detector_name, val.c_str(), sbio::XTC2::MaxNameSize);
                  })
    .def_readwrite("segment_number", &sbio::XTC2Traits::DataRequest::segment_number)
    .def_property("alg_name",
                  [](const sbio::XTC2Traits::DataRequest& self) -> std::string {
                    return self.alg_name;
                  },
                  [](sbio::XTC2Traits::DataRequest& self, const std::string& val) {
                    sbio::safe_strncpy(self.alg_name, val.c_str(), sbio::XTC2::MaxNameSize);
                  })
    .def_property("field_name",
                  [](const sbio::XTC2Traits::DataRequest& self) -> std::string {
                    return self.field_name;
                  },
                  [](sbio::XTC2Traits::DataRequest& self, const std::string& val) {
                    sbio::safe_strncpy(self.field_name, val.c_str(), sbio::XTC2::MaxNameSize);
                  });

  py::classh<sbio::XTC2::DataResult>(ftraits_module, "XTC2DataResult")
    .def(py::init<>())
    .def_readwrite("data", &sbio::XTC2::DataResult::data)
    .def_readwrite("size", &sbio::XTC2::DataResult::size)
    .def_readwrite("rank", &sbio::XTC2::DataResult::rank)
    .def_property("shape",
                  [](const sbio::XTC2::DataResult& self) {
                    return std::vector(self.shape, self.shape + self.rank);
                  },
                  [](sbio::XTC2::DataResult& self, const std::vector<ssize_t> shape) {
                    if (shape.size() < self.rank) {
                      throw std::runtime_error("Provided shape is smaller than the rank!");
                    }
                    for (std::size_t i = 0; i < self.rank; ++i) {
                      self.shape[i] = shape[i];
                    }
                  })
    .def_readwrite("dtype", &sbio::XTC2::DataResult::dtype);
#endif

} // ftraits_module
