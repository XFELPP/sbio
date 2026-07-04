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

#include "pysbio/pydatasource.hh"
#include "pysbio/pydetector.hh"

#include "ncarray/ncarrays.hh"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>

namespace py = pybind11;

using namespace pysbio;

PYBIND11_MODULE(_pysbio, pysbio_module, py::mod_gil_not_used()) {
  // NOTE: This must be in PYTHONPATH (or generally accessible!) or init will fail
  py::module_::import("ncarray");
  pysbio_module.doc() = "sbio Python bindings.";

  // TODO: Try to rework these wrappers to remove dynamic_attr
  //       Instead, create the *classes* dynamically and attach the methods
  py::classh<DetectorWrapper>(pysbio_module, "DetectorWrapper", py::dynamic_attr())
    .def_property_readonly("serial_number", &DetectorWrapper::get_serial_number)
    .def("fetch_calib_constants",
         &DetectorWrapper::stage_calibration,
         py::arg("params"));
  py::classh<AlgWrapper>(pysbio_module, "AlgWrapper", py::dynamic_attr());

  py::classh<PyDataSource>(pysbio_module, "DataSource")
    .def(py::init<std::string, std::string, unsigned, int, int, unsigned>(),
         py::arg("ds_type"),
         py::arg("exp"),
         py::arg("run"),
         py::arg("events_per_read"),
         py::arg("max_dgram_size"),
         py::arg("xtc_ver"))
    .def("detector", [&](PyDataSource& self, const char* name) {
      return self.detector(pysbio_module, name);
    },
      py::arg("detname"))
    // Event generation and iteration
    // NOTE: Iterators are generated at run-time due to type-erasure and variant issues
    // making binding the iterator itself rather annoying.
    .def("next", &PyDataSource::next)
    .def("__iter__", [](PyDataSource& self) {
      auto iterator_maker = [](auto& ds) {
        return py::make_iterator(ds.begin(), ds.end());
      };

      return std::visit(iterator_maker, self.ds());
    },
      py::keep_alive<0, 1>())
    .def("events", [](PyDataSource& self) {
      // This is just another name for the iter function, for API similarity
      auto iterator_maker = [](auto& ds) {
        return py::make_iterator(ds.begin(), ds.end());
      };

      return std::visit(iterator_maker, self.ds());
    },
      py::keep_alive<0, 1>());
} // pysbio_module
