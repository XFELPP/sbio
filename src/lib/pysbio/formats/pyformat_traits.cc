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

#include "pysbio/io/pyio.hh"

#include "sbio/core/filter.hh"
#include "sbio/util/string.hh"

#ifdef SBIO_HAS_HDF5
#include "sbio/formats/hdf5/cxi_spec.hh"
#endif
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

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace py = pybind11;

namespace {

  template <typename AccessPtn, typename PyAccess>
  void bind_subkind_filter(PyAccess& ptn_cls) {
    if constexpr (requires { typename AccessPtn::sub_kind; }) {
      using Filter = sbio::SubKindFilter<AccessPtn>;

      py::classh<Filter>(ptn_cls, "SubKindFilter")
        .def(py::init<>())
        .def("enable_all", &Filter::enable_all, "Allow all sub-kinds.")
        .def("disable_all", &Filter::disable_all, "Disable (filter) all sub-kinds.")
        .def("allow_subkind",
             py::overload_cast<std::size_t>(&Filter::allow_subkind),
             py::arg("subkind"),
             "Allow/enable a single unique sub-kind.")
        .def("disable_subkind",
             py::overload_cast<std::size_t>(&Filter::disable_subkind),
             py::arg("subkind"),
             "Disable/filter a single unique sub-kind.")
        .def("full_filter",
             &Filter::full_filter,
             "Retrieve the integer representation of the full filter.");

      ptn_cls.def("filter", [](const AccessPtn& self) { return Filter(); });
    }
  }
} // anonymous namespace

PYBIND11_MODULE(pyformat_traits, ftraits_module, py::mod_gil_not_used()) {
  ftraits_module.doc() = "sbio Python bindings for the generic data format traits.";

  py::module_::import("sbio.io");

  py::native_enum<pysbio::FTraits>(ftraits_module,
                                   "FTraits",
                                   "enum.Enum",
                                   "Enumerators for selecting data formats.")
    .value("XTC1", pysbio::FTraits::XTC1)
    .value("XTC2", pysbio::FTraits::XTC2)
    .value("CXI", pysbio::FTraits::CXI)
    .export_values()
    .finalize();

#ifdef SBIO_HAS_HDF5
  py::native_enum<sbio::CXITraits::DataAccessPtn::type>(ftraits_module,
                                                        "CXIDataAccessPtnType",
                                                        "enum.IntEnum",
                                                        "Enumerators for access styles of CXI data.")
    .value("Frames", sbio::CXITraits::DataAccessPtn::Frames)
    .value("PerStep", sbio::CXITraits::DataAccessPtn::PerStep)
    //.export_values()
    .finalize();

  py::native_enum<sbio::CXITraits::DataAccessPtn::sub_kind>(ftraits_module,
                                                            "CXIDataAccessPtnSubKind",
                                                            "enum.IntEnum",
                                                            "Enumerators for sub kinds of CXI data.")
    .value("DSET", sbio::CXITraits::DataAccessPtn::DSET)
    .finalize();

  auto cxi_ptn = py::classh<sbio::CXITraits::DataAccessPtn>(ftraits_module, "CXIDataAccessPtn")
    .def(py::init<>())
    .def(py::init<sbio::CXITraits::DataAccessPtn::type>());

  cxi_ptn.attr("Frames") = py::cast(sbio::CXITraits::DataAccessPtn::Frames);
  cxi_ptn.attr("PerStep") = py::cast(sbio::CXITraits::DataAccessPtn::PerStep);

  cxi_ptn.attr("DSET") = py::cast(sbio::CXITraits::DataAccessPtn::DSET);

  bind_subkind_filter<sbio::CXITraits::DataAccessPtn>(cxi_ptn);

  pysbio::impl::bind_stream_config<sbio::CXITraits>(ftraits_module, "CXIStreamConfig");

  // TODO: Need to bind DataRequest for XTC1 and XTC2 (not the special ones?)
  py::classh<sbio::CXITraits::DataRequest>(ftraits_module, "CXIDataRequest")
    .def(py::init<const char*, const char*, const char*>())
    .def(py::init<const char*, const char*>())
    .def_property("detector_type",
                  [](const sbio::CXITraits::DataRequest& self) -> std::string {
                    return self.group_type;
                  },
                  [](sbio::CXITraits::DataRequest& self, const std::string& val) {
                    self.group_type = val;
                  })
    .def_property("detector_name",
                  [](const sbio::CXITraits::DataRequest& self) -> std::string {
                    return self.group_name;
                  },
                  [](sbio::CXITraits::DataRequest& self, const std::string& val) {
                    self.group_name = val;
                  })
    .def_readwrite("segment_number", &sbio::CXITraits::DataRequest::segment_number)
    .def_property("dataset",
                  [](const sbio::CXITraits::DataRequest& self) -> std::string {
                    return self["dataset"];
                  },
                  [](sbio::CXITraits::DataRequest& self, const std::string& val) {
                    self.set<"dataset">(val.c_str());
                  });

  py::classh<sbio::DataResult>(ftraits_module, "CXIDataResult")
    .def(py::init<>())
    .def_readwrite("data", &sbio::DataResult::data)
    .def_readwrite("size", &sbio::DataResult::size)
    .def_readwrite("rank", &sbio::DataResult::rank)
    .def_property("shape",
                  [](const sbio::DataResult& self) {
                    return std::vector(self.shape.data(), self.shape.data() + self.rank);
                  },
                  [](sbio::DataResult& self, const std::vector<ssize_t> shape) {
                    if (shape.size() < self.rank) {
                      throw std::runtime_error("Provided shape is smaller than the rank!");
                    }
                    for (std::size_t i = 0; i < self.rank; ++i) {
                      self.shape[i] = static_cast<std::uint32_t>(shape[i]);
                    }
                  })
    .def_readwrite("dtype", &sbio::DataResult::dtype);

#endif // SBIO_HAS_HDF5

#ifdef SBIO_HAS_XTC1
  py::native_enum<sbio::XTC1Traits::DataAccessPtn::type>(ftraits_module,
                                                         "XTC1DataAccessPtnType",
                                                         "enum.IntEnum",
                                                         "Enumerators for access styles of XTC1 data.")
    .value("L1Accept", sbio::XTC1Traits::DataAccessPtn::L1Accept)
    .value("SlowUpdate", sbio::XTC1Traits::DataAccessPtn::SlowUpdate)
    .value("BeginStep", sbio::XTC1Traits::DataAccessPtn::BeginStep)
    //.export_values()
    // Values don't export because of clash with the XTC2 version
    .finalize();

  py::native_enum<sbio::XTC1Traits::DataAccessPtn::sub_kind>(ftraits_module,
                                                             "XTC1DataAccessPtnSubKind",
                                                             "enum.IntEnum",
                                                             "Enumerators for sub kinds of XTC1 data.")
    .value("ROG0", sbio::XTC1Traits::DataAccessPtn::ROG0)
    .value("ROG1", sbio::XTC1Traits::DataAccessPtn::ROG1)
    .value("ROG2", sbio::XTC1Traits::DataAccessPtn::ROG2)
    .value("ROG3", sbio::XTC1Traits::DataAccessPtn::ROG3)
    .value("ROG4", sbio::XTC1Traits::DataAccessPtn::ROG4)
    .value("ROG5", sbio::XTC1Traits::DataAccessPtn::ROG5)
    .value("ROG6", sbio::XTC1Traits::DataAccessPtn::ROG6)
    .value("ROG7", sbio::XTC1Traits::DataAccessPtn::ROG7)
    .finalize();

  auto xtc1_ptn = py::classh<sbio::XTC1Traits::DataAccessPtn>(ftraits_module, "XTC1DataAccessPtn")
    .def(py::init<>())
    .def(py::init<sbio::XTC1Traits::DataAccessPtn::type>());

  xtc1_ptn.attr("L1Accept") = py::cast(sbio::XTC1Traits::DataAccessPtn::L1Accept);
  xtc1_ptn.attr("SlowUpdate") = py::cast(sbio::XTC1Traits::DataAccessPtn::SlowUpdate);
  xtc1_ptn.attr("BeginStep") = py::cast(sbio::XTC1Traits::DataAccessPtn::BeginStep);

  xtc1_ptn.attr("ROG0") = py::cast(sbio::XTC1Traits::DataAccessPtn::ROG0);
  xtc1_ptn.attr("ROG1") = py::cast(sbio::XTC1Traits::DataAccessPtn::ROG1);
  xtc1_ptn.attr("ROG2") = py::cast(sbio::XTC1Traits::DataAccessPtn::ROG2);
  xtc1_ptn.attr("ROG3") = py::cast(sbio::XTC1Traits::DataAccessPtn::ROG3);
  xtc1_ptn.attr("ROG4") = py::cast(sbio::XTC1Traits::DataAccessPtn::ROG4);
  xtc1_ptn.attr("ROG5") = py::cast(sbio::XTC1Traits::DataAccessPtn::ROG5);
  xtc1_ptn.attr("ROG6") = py::cast(sbio::XTC1Traits::DataAccessPtn::ROG6);
  xtc1_ptn.attr("ROG7") = py::cast(sbio::XTC1Traits::DataAccessPtn::ROG7);

  bind_subkind_filter<sbio::XTC1Traits::DataAccessPtn>(xtc1_ptn);

  pysbio::impl::bind_stream_config<sbio::XTC1Traits>(ftraits_module, "XTC1StreamConfig");

  py::classh<sbio::XTC1Traits::DataRequest>(ftraits_module, "XTC1DataRequest")
    .def(py::init<const char*, const char*, const char*>())
    .def(py::init<const char*, const char*>())
    .def_property("detector_type",
                  [](const sbio::XTC1Traits::DataRequest& self) -> std::string {
                    return self.group_type;
                  },
                  [](sbio::XTC1Traits::DataRequest& self, const std::string& val) {
                    self.group_type = val;
                  })
    .def_property("detector_name",
                  [](const sbio::XTC1Traits::DataRequest& self) -> std::string {
                    return self.group_name;
                  },
                  [](sbio::XTC1Traits::DataRequest& self, const std::string& val) {
                    self.group_name = val;
                  })
    .def_readwrite("segment_number", &sbio::XTC1Traits::DataRequest::segment_number)
    .def_property("field_name",
                  [](const sbio::XTC1Traits::DataRequest& self) -> std::string {
                    return self["field"];
                  },
                  [](sbio::XTC1Traits::DataRequest& self, const std::string& val) {
                    self.set<"field">(val.c_str());
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

#endif // SBIO_HAS_XTC1

#ifdef SBIO_HAS_XTC2
  py::native_enum<sbio::XTC2Traits::DataAccessPtn::type>(ftraits_module,
                                                         "XTC2DataAccessPtnType",
                                                         "enum.IntEnum",
                                                         "Enumerators for access styles of XTC2 data.")
    .value("L1Accept", sbio::XTC2Traits::DataAccessPtn::L1Accept)
    .value("SlowUpdate", sbio::XTC2Traits::DataAccessPtn::SlowUpdate)
    .value("BeginStep", sbio::XTC2Traits::DataAccessPtn::BeginStep)
    //.export_values()
    .finalize();

  py::native_enum<sbio::XTC2Traits::DataAccessPtn::sub_kind>(ftraits_module,
                                                             "XTC2DataAccessPtnSubKind",
                                                             "enum.IntEnum",
                                                             "Enumerators for sub kinds of XTC2 data.")
    .value("ROG0", sbio::XTC2Traits::DataAccessPtn::ROG0)
    .value("ROG1", sbio::XTC2Traits::DataAccessPtn::ROG1)
    .value("ROG2", sbio::XTC2Traits::DataAccessPtn::ROG2)
    .value("ROG3", sbio::XTC2Traits::DataAccessPtn::ROG3)
    .value("ROG4", sbio::XTC2Traits::DataAccessPtn::ROG4)
    .value("ROG5", sbio::XTC2Traits::DataAccessPtn::ROG5)
    .value("ROG6", sbio::XTC2Traits::DataAccessPtn::ROG6)
    .value("ROG7", sbio::XTC2Traits::DataAccessPtn::ROG7)
    .finalize();

  auto xtc2_ptn = py::classh<sbio::XTC2Traits::DataAccessPtn>(ftraits_module, "XTC2DataAccessPtn")
    .def(py::init<>())
    .def(py::init<sbio::XTC2Traits::DataAccessPtn::type>());

  xtc2_ptn.attr("L1Accept") = py::cast(sbio::XTC2Traits::DataAccessPtn::L1Accept);
  xtc2_ptn.attr("SlowUpdate") = py::cast(sbio::XTC2Traits::DataAccessPtn::SlowUpdate);
  xtc2_ptn.attr("BeginStep") = py::cast(sbio::XTC2Traits::DataAccessPtn::BeginStep);

  xtc2_ptn.attr("ROG0") = py::cast(sbio::XTC2Traits::DataAccessPtn::ROG0);
  xtc2_ptn.attr("ROG1") = py::cast(sbio::XTC2Traits::DataAccessPtn::ROG1);
  xtc2_ptn.attr("ROG2") = py::cast(sbio::XTC2Traits::DataAccessPtn::ROG2);
  xtc2_ptn.attr("ROG3") = py::cast(sbio::XTC2Traits::DataAccessPtn::ROG3);
  xtc2_ptn.attr("ROG4") = py::cast(sbio::XTC2Traits::DataAccessPtn::ROG4);
  xtc2_ptn.attr("ROG5") = py::cast(sbio::XTC2Traits::DataAccessPtn::ROG5);
  xtc2_ptn.attr("ROG6") = py::cast(sbio::XTC2Traits::DataAccessPtn::ROG6);
  xtc2_ptn.attr("ROG7") = py::cast(sbio::XTC2Traits::DataAccessPtn::ROG7);

  bind_subkind_filter<sbio::XTC2Traits::DataAccessPtn>(xtc2_ptn);

  pysbio::impl::bind_stream_config<sbio::XTC2Traits>(ftraits_module, "XTC2StreamConfig");

  py::classh<sbio::XTC2Traits::DataRequest>(ftraits_module, "XTC2DataRequest")
    .def(py::init<const char*, const char*, const char*, const char*>())
    .def(py::init<const char*, const char*>())
    .def_property("detector_type",
                  [](const sbio::XTC2Traits::DataRequest& self) -> std::string {
                    return self.group_type;
                  },
                  [](sbio::XTC2Traits::DataRequest& self, const std::string& val) {
                    self.group_type = val;
                  })
    .def_property("detector_name",
                  [](const sbio::XTC2Traits::DataRequest& self) -> std::string {
                    return self.group_name;
                  },
                  [](sbio::XTC2Traits::DataRequest& self, const std::string& val) {
                    self.group_name = val;
                  })
    .def_readwrite("segment_number", &sbio::XTC2Traits::DataRequest::segment_number)
    .def_property("alg_name",
                  [](const sbio::XTC2Traits::DataRequest& self) -> std::string {
                    return self["alg"];
                  },
                  [](sbio::XTC2Traits::DataRequest& self, const std::string& val) {
                    self.set<"alg">(val.c_str());
                  })
    .def_property("field_name",
                  [](const sbio::XTC2Traits::DataRequest& self) -> std::string {
                    return self["field"];
                  },
                  [](sbio::XTC2Traits::DataRequest& self, const std::string& val) {
                    self.set<"field">(val.c_str());
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
#endif // SBIO_HAS_XTC2
} // ftraits_module
