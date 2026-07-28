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

#include "pysbio/pybroker_group.hh"
#include "pysbio/pydatasource.hh"

#include <ncarray/ncarrays.hh>
#ifdef SBIO_HAS_MPI
#include <mpi.h>
#endif
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>

namespace py = pybind11;

using namespace pysbio;

namespace {
  static constexpr const char* ds_doc = R"a(Create a datasource for reading XTC1 or XTC2 data.

Args:
    exp (str): The LCLS experiment.

    run (int): The run number for the experiment.

    events_per_read (int): The number of event offsets to index on each attempt.

    max_dgram_size (int): The maximum size of a Datagram to accept.

    xtc_ver (int): The version of XTC data (1 or 2).

    ds_type (str): The desired DataSource type - currently supported are `serial` and `mpi`.

    exec_cfg (dict): Configuration for the Execution policy (fields depend on selection).)a";
} // anonymous namespace

PYBIND11_MODULE(_pysbio, pysbio_module, py::mod_gil_not_used()) {
  // NOTE: This must be in PYTHONPATH (or generally accessible!) or init will fail
  py::module_::import("ncarray");
  pysbio_module.doc() = "sbio Python bindings.";

  py::module_::import("sbio.formats.pyformat_traits");
#ifdef SBIO_HAS_XTC1
  py::module_::import("sbio.formats.pyxtc1");
#endif
#ifdef SBIO_HAS_XTC2
  py::module_::import("sbio.formats.pyxtc2");
#endif

  py::module_::import("sbio.io");
  py::module_::import("sbio.execution");
  py::module_::import("sbio.brokers");

  // TODO: Try to rework these wrappers to remove dynamic_attr
  //       Instead, create the *classes* dynamically and attach the methods
  py::classh<BrokerGroupWrapper>(pysbio_module, "BrokerGroupWrapper", py::dynamic_attr())
    .def_property_readonly("group_type", &BrokerGroupWrapper::get_detector_type)
    .def_property_readonly("serial_number", &BrokerGroupWrapper::get_serial_number)
    .def("get_data", &BrokerGroupWrapper::get_data)
    .def("get_multi_data", &BrokerGroupWrapper::get_multi_data);

  py::classh<AlgWrapper>(pysbio_module, "AlgWrapper", py::dynamic_attr());

  py::classh<PyDataSource>(pysbio_module, "DataSource")
    .def(py::init<std::string, unsigned, int, int, unsigned, std::string, py::dict>(),
         py::arg("exp"),
         py::arg("run"),
         py::arg("events_per_read") = 43200,    // 6 minutes at 120 Hz
         py::arg("max_dgram_size") = 0x4000000, // ~67 MB
         py::arg("xtc_ver") = 2,
         py::arg("ds_type") = "mpi",
         py::arg("exec_cfg") = py::dict(),
         ds_doc)
    .def("group", [&](PyDataSource& self, const char* name) {
      return self.group(pysbio_module, name);
    },
      py::arg("group_name"))
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

#ifdef SBIO_HAS_MPI
  auto atexit = py::module_::import("atexit");
  atexit.attr("register")(py::cpp_function([]() {
    int initialized  { 0 };
    MPI_Initialized(&initialized);

    int finalized { 0 };
    if (initialized && !finalized) {
      // At exit, the atexit handler may be called before the GC has collected
      // remaining objects - we make sure to wait here, so any shared MPI resources
      // are valid for other ranks by registering our own handler with a Barrier
      MPI_Barrier(MPI_COMM_WORLD);
    }
  }));
#endif
} // pysbio_module
