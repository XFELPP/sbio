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

#include "pysbio/core/execution.hh"

#include "sbio/core/broker.hh"
#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi.hh"
#include "sbio/execution/mpi_threaded.hh"
#endif
#include "sbio/execution/serial.hh"
#include "sbio/execution/threaded.hh"
#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif
#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2_traits.hh"
#endif

#include "sbio/io/posix.hh"

#ifdef SBIO_HAS_MPI
#include <mpi.h>
#endif

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace {
  template <typename BrokerT, typename PyBroker>
  void bind_broker_functions(PyBroker& broker_cls) {
    broker_cls.def(py::init<>())
      .def("prepare", &BrokerT::prepare)
      .def("index_stream", &BrokerT::index_stream)
      .def("capacity", &BrokerT::capacity)
      .def("metadata", [](const BrokerT& self) { return self.metadata(); })
      .def("set_metadata", &BrokerT::set_metadata)
      .def("fetch_step", &BrokerT::fetch_step)
      .def("fetch_steps", &BrokerT::fetch_steps)
      .def("get_data_in_buffer", &BrokerT::get_data_in_buffer);
  }
} // anonymous namespace

PYBIND11_MODULE(_core, core_module, py::mod_gil_not_used()) {
  core_module.doc() = "sbio Python trampolines for core class overriding.";

  using PySerialExec = pysbio::PyExecution<sbio::SerialExecution>;
  py::classh<PySerialExec>(core_module, "PySerialExecution")
    .def(py::init<>());

  using PyThreadedExec = pysbio::PyExecution<sbio::ThreadedExecution>;
  py::classh<PyThreadedExec>(core_module, "PyThreadedExecution")
    .def(py::init<>());

#ifdef SBIO_HAS_XTC1
  using PyXTC1SerialBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    PySerialExec,
    sbio::XTC1Traits
  >;
  using PyXTC1ThreadedBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    PyThreadedExec,
    sbio::XTC1Traits
  >;

  auto pyserial_xtc1 =
    py::classh<PyXTC1SerialBroker>(core_module, "PyXTC1SerialStreamBroker");
  bind_broker_functions<PyXTC1SerialBroker>(pyserial_xtc1);

  auto pythreaded_xtc1 =
    py::classh<PyXTC1ThreadedBroker>(core_module, "PyXTC1ThreadedStreamBroker");
  bind_broker_functions<PyXTC1ThreadedBroker>(pythreaded_xtc1);
#endif

#ifdef SBIO_HAS_XTC2
  using PyXTC2SerialBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    PySerialExec,
    sbio::XTC2Traits
  >;
  using PyXTC2ThreadedBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    PyThreadedExec,
    sbio::XTC2Traits
  >;

  auto pyserial_xtc2 =
    py::classh<PyXTC2SerialBroker>(core_module, "PyXTC2SerialStreamBroker");
  bind_broker_functions<PyXTC2SerialBroker>(pyserial_xtc2);

  auto pythreaded_xtc2 =
      py::classh<PyXTC2ThreadedBroker>(core_module, "PyXTC2ThreadedStreamBroker");
  bind_broker_functions<PyXTC2ThreadedBroker>(pythreaded_xtc2);
#endif

#ifdef SBIO_HAS_MPI
  using PyMPIExec = pysbio::PyExecution<sbio::MPIExecution>;
  py::classh<PyMPIExec>(core_module, "PyMPIExecution")
    .def(py::init<>());

  using PyThreadedMPIExec = pysbio::PyExecution<sbio::MPIThreadedExecution>;
  py::classh<PyThreadedMPIExec>(core_module, "PyThreadedMPIExecution")
    .def(py::init<>());

#ifdef SBIO_HAS_XTC1
  using PyXTC1MPIBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    PyMPIExec,
    sbio::XTC1Traits
  >;
  using PyXTC1ThreadedMPIBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    PyThreadedMPIExec,
    sbio::XTC1Traits
  >;

  auto pympi_xtc1 =
    py::classh<PyXTC1MPIBroker>(core_module, "PyXTC1MPIStreamBroker");
  bind_broker_functions<PyXTC1MPIBroker>(pympi_xtc1);

  auto pympi_threaded_xtc1 =
    py::classh<PyXTC1ThreadedMPIBroker>(core_module, "PyXTC1ThreadedMPIStreamBroker");
  bind_broker_functions<PyXTC1ThreadedMPIBroker>(pympi_threaded_xtc1);
#endif

#ifdef SBIO_HAS_XTC2
  using PyXTC2MPIBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    PyMPIExec,
    sbio::XTC2Traits
  >;
  using PyXTC2ThreadedMPIBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    PyThreadedMPIExec,
    sbio::XTC2Traits
  >;

  auto pympi_xtc2 =
    py::classh<PyXTC2MPIBroker>(core_module, "PyXTC2MPIStreamBroker");
  bind_broker_functions<PyXTC2MPIBroker>(pympi_xtc2);

  auto pympi_threaded_xtc2 =
      py::classh<PyXTC2ThreadedMPIBroker>(core_module, "PyXTC2ThreadedMPIStreamBroker");
  bind_broker_functions<PyXTC2ThreadedMPIBroker>(pympi_threaded_xtc2);
#endif

#endif // SBIO_HAS_MPI
} // core_module
