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

#include "pysbio/binding_builders.hh"

#include "sbio/core/broker.hh"
#include "sbio/execution/serial.hh"
#include "sbio/execution/threaded.hh"
#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi.hh"
#include "sbio/execution/mpi_threaded.hh"
#endif
#include "sbio/io/posix.hh"
#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif
#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2_traits.hh"
#endif

#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(pybrokers, broker_module, py::mod_gil_not_used()) {
  broker_module.doc() = "sbio Python bindings for Stream Brokers.";

#ifdef SBIO_HAS_XTC1
  using XTC1SerialBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC1Traits
  >;

  using XTC1ThreadedBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::ThreadedExecution,
    sbio::XTC1Traits
  >;

  auto serial_xtc1 =
    py::classh<XTC1SerialBroker>(broker_module, "XTC1SerialStreamBroker");
  pysbio::impl::bind_broker_functions<XTC1SerialBroker>(serial_xtc1);

  auto threaded_xtc1 =
    py::classh<XTC1ThreadedBroker>(broker_module, "XTC1ThreadedStreamBroker");
  pysbio::impl::bind_broker_functions<XTC1ThreadedBroker>(threaded_xtc1);

#ifdef SBIO_HAS_MPI
  using XTC1MPIBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC1Traits
  >;

  using XTC1ThreadedMPIBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::MPIThreadedExecution,
    sbio::XTC1Traits
  >;

  auto mpi_xtc1 = py::classh<XTC1MPIBroker>(broker_module, "XTC1MPIStreamBroker");
  pysbio::impl::bind_broker_functions<XTC1MPIBroker>(mpi_xtc1);

  auto mpi_threaded_xtc1 =
    py::classh<XTC1ThreadedMPIBroker>(broker_module, "XTC1ThreadedMPIStreamBroker");
  pysbio::impl::bind_broker_functions<XTC1ThreadedMPIBroker>(mpi_threaded_xtc1);

#endif
#endif

#ifdef SBIO_HAS_XTC2
  using XTC2SerialBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC2Traits
  >;

  using XTC2ThreadedBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::ThreadedExecution,
    sbio::XTC2Traits
  >;

  auto serial_xtc2 =
    py::classh<XTC2SerialBroker>(broker_module, "XTC2SerialStreamBroker");
  pysbio::impl::bind_broker_functions<XTC2SerialBroker>(serial_xtc2);

  auto threaded_xtc2 =
    py::classh<XTC2ThreadedBroker>(broker_module, "XTC2ThreadedStreamBroker");
  pysbio::impl::bind_broker_functions<XTC2ThreadedBroker>(threaded_xtc2);

#ifdef SBIO_HAS_MPI
  using XTC2MPIBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC2Traits
  >;

  using XTC2ThreadedMPIBroker = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::MPIThreadedExecution,
    sbio::XTC2Traits
  >;

  auto mpi_xtc2 = py::classh<XTC2MPIBroker>(broker_module, "XTC2MPIStreamBroker");
  pysbio::impl::bind_broker_functions<XTC2MPIBroker>(mpi_xtc2);

  auto mpi_threaded_xtc2 =
    py::classh<XTC2ThreadedMPIBroker>(broker_module, "XTC2ThreadedMPIStreamBroker");
  pysbio::impl::bind_broker_functions<XTC2ThreadedMPIBroker>(mpi_threaded_xtc2);

#endif
#endif
}
