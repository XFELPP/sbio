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
#include "pysbio/core/execution.hh"

#include "sbio/core/broker.hh"
#include "sbio/core/broker_group.hh"
#include "sbio/core/datasource.hh"
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

PYBIND11_MODULE(_core, core_module, py::mod_gil_not_used()) {
  core_module.doc() = "sbio Python trampolines for core class overriding.";

  // Execution policies
  using PySerialExec = pysbio::PyExecution<sbio::SerialExecution>;
  py::classh<PySerialExec>(core_module, "PySerialExecution")
    .def(py::init<>());

  using PyThreadedExec = pysbio::PyExecution<sbio::ThreadedExecution>;
  py::classh<PyThreadedExec>(core_module, "PyThreadedExecution")
    .def(py::init<>());

#ifdef SBIO_HAS_XTC1
  // Brokers
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
  pysbio::impl::bind_broker_functions<PyXTC1SerialBroker>(pyserial_xtc1);

  auto pythreaded_xtc1 =
    py::classh<PyXTC1ThreadedBroker>(core_module, "PyXTC1ThreadedStreamBroker");
  pysbio::impl::bind_broker_functions<PyXTC1ThreadedBroker>(pythreaded_xtc1);

  // Broker Groups
  using PyXTC1SerialBrokerGroup = sbio::BrokerGroup<
    PyXTC1SerialBroker,
    sbio::XTC1Traits
  >;

  using PyXTC1ThreadedBrokerGroup = sbio::BrokerGroup<
    PyXTC1ThreadedBroker,
    sbio::XTC1Traits
  >;

  using XTC1SerialBGRef = typename PyXTC1SerialBrokerGroup::DataSegmentRef;
  using XTC1ThreadedBGRef = typename PyXTC1ThreadedBrokerGroup::DataSegmentRef;

  auto pyserial_bg_xtc1 =
    py::classh<PyXTC1SerialBrokerGroup>(core_module, "PyXTC1SerialBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    PyXTC1SerialBrokerGroup, XTC1SerialBGRef
  >(pyserial_bg_xtc1);

  auto pythreaded_bg_xtc1 =
    py::classh<PyXTC1ThreadedBrokerGroup>(core_module, "PyXTC1ThreadedBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    PyXTC1ThreadedBrokerGroup, XTC1ThreadedBGRef
  >(pythreaded_bg_xtc1);

  // DataSources
  using PyXTC1SerialDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    PySerialExec,
    sbio::XTC1Traits
  >;
  using PyXTC1ThreadedDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    PyThreadedExec,
    sbio::XTC1Traits
  >;

  auto pyserial_ds_xtc1 =
    py::classh<PyXTC1SerialDataSource>(core_module, "PyXTC1SerialDataSource");
  pysbio::impl::bind_datasource_functions<PyXTC1SerialDataSource>(pyserial_ds_xtc1);

  auto pythreaded_ds_xtc1 =
      py::classh<PyXTC1ThreadedDataSource>(core_module, "PyXTC1ThreadedDataSource");
  pysbio::impl::bind_datasource_functions<PyXTC1ThreadedDataSource>(pythreaded_ds_xtc1);

#endif

#ifdef SBIO_HAS_XTC2
  // Brokers
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
  pysbio::impl::bind_broker_functions<PyXTC2SerialBroker>(pyserial_xtc2);

  auto pythreaded_xtc2 =
      py::classh<PyXTC2ThreadedBroker>(core_module, "PyXTC2ThreadedStreamBroker");
  pysbio::impl::bind_broker_functions<PyXTC2ThreadedBroker>(pythreaded_xtc2);

  // Broker Groups
  using PyXTC2SerialBrokerGroup = sbio::BrokerGroup<
    PyXTC2SerialBroker,
    sbio::XTC2Traits
  >;

  using PyXTC2ThreadedBrokerGroup = sbio::BrokerGroup<
    PyXTC2ThreadedBroker,
    sbio::XTC2Traits
  >;

  using XTC2SerialBGRef = typename PyXTC2SerialBrokerGroup::DataSegmentRef;
  using XTC2ThreadedBGRef = typename PyXTC2ThreadedBrokerGroup::DataSegmentRef;

  auto pyserial_bg_xtc2 =
    py::classh<PyXTC2SerialBrokerGroup>(core_module, "PyXTC2SerialBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    PyXTC2SerialBrokerGroup, XTC2SerialBGRef
  >(pyserial_bg_xtc2);

  auto pythreaded_bg_xtc2 =
    py::classh<PyXTC2ThreadedBrokerGroup>(core_module, "PyXTC2ThreadedBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    PyXTC2ThreadedBrokerGroup, XTC2ThreadedBGRef
  >(pythreaded_bg_xtc2);

  // DataSources
  using PyXTC2SerialDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    PySerialExec,
    sbio::XTC2Traits
  >;
  using PyXTC2ThreadedDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    PyThreadedExec,
    sbio::XTC2Traits
  >;

  auto pyserial_ds_xtc2 =
    py::classh<PyXTC2SerialDataSource>(core_module, "PyXTC2SerialDataSource");
  pysbio::impl::bind_datasource_functions<PyXTC2SerialDataSource>(pyserial_ds_xtc2);

  auto pythreaded_ds_xtc2 =
    py::classh<PyXTC2ThreadedDataSource>(core_module, "PyXTC2ThreadedDataSource");
  pysbio::impl::bind_datasource_functions<PyXTC2ThreadedDataSource>(pythreaded_ds_xtc2);

#endif

#ifdef SBIO_HAS_MPI
  // Execution Policies
  using PyMPIExec = pysbio::PyExecution<sbio::MPIExecution>;
  py::classh<PyMPIExec>(core_module, "PyMPIExecution")
    .def(py::init<>());

  using PyThreadedMPIExec = pysbio::PyExecution<sbio::MPIThreadedExecution>;
  py::classh<PyThreadedMPIExec>(core_module, "PyThreadedMPIExecution")
    .def(py::init<>());

#ifdef SBIO_HAS_XTC1
  // Brokers
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
  pysbio::impl::bind_broker_functions<PyXTC1MPIBroker>(pympi_xtc1);

  auto pympi_threaded_xtc1 =
    py::classh<PyXTC1ThreadedMPIBroker>(core_module, "PyXTC1ThreadedMPIStreamBroker");
  pysbio::impl::bind_broker_functions<PyXTC1ThreadedMPIBroker>(pympi_threaded_xtc1);

  // BrokerGroups
  using PyXTC1MPIBrokerGroup = sbio::BrokerGroup<
    PyXTC1MPIBroker,
    sbio::XTC1Traits
  >;

  using PyXTC1ThreadedMPIBrokerGroup = sbio::BrokerGroup<
    PyXTC1ThreadedMPIBroker,
    sbio::XTC1Traits
  >;

  using XTC1MPIBGRef = typename PyXTC1MPIBrokerGroup::DataSegmentRef;
  using XTC1ThreadedMPIBGRef = typename PyXTC1ThreadedMPIBrokerGroup::DataSegmentRef;

  auto pympi_bg_xtc1 =
    py::classh<PyXTC1MPIBrokerGroup>(core_module, "PyXTC1MPIBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    PyXTC1MPIBrokerGroup, XTC1MPIBGRef
  >(pympi_bg_xtc1);

  auto pympi_threaded_bg_xtc1 =
    py::classh<PyXTC1ThreadedMPIBrokerGroup>(core_module, "PyXTC1ThreadedMPIBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    PyXTC1ThreadedMPIBrokerGroup, XTC1ThreadedMPIBGRef
  >(pympi_threaded_bg_xtc1);

  // DataSources
  using PyXTC1MPIDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    PyMPIExec,
    sbio::XTC1Traits
  >;
  using PyXTC1ThreadedMPIDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    PyThreadedMPIExec,
    sbio::XTC1Traits
  >;

  auto pympi_ds_xtc1 = py::classh<PyXTC1MPIDataSource>(core_module, "PyXTC1MPIDataSource");
  pysbio::impl::bind_datasource_functions<PyXTC1MPIDataSource>(pympi_ds_xtc1);

  auto pympi_threaded_ds_xtc1 =
      py::classh<PyXTC1ThreadedMPIDataSource>(core_module, "PyXTC1ThreadedMPIDataSource");
  pysbio::impl::bind_datasource_functions<PyXTC1ThreadedMPIDataSource>(pympi_threaded_ds_xtc1);

#endif

#ifdef SBIO_HAS_XTC2
  // Brokers
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
  pysbio::impl::bind_broker_functions<PyXTC2MPIBroker>(pympi_xtc2);

  auto pympi_threaded_xtc2 =
      py::classh<PyXTC2ThreadedMPIBroker>(core_module, "PyXTC2ThreadedMPIStreamBroker");
  pysbio::impl::bind_broker_functions<PyXTC2ThreadedMPIBroker>(pympi_threaded_xtc2);

  // Broker Groups
  using PyXTC2MPIBrokerGroup = sbio::BrokerGroup<
    PyXTC2MPIBroker,
    sbio::XTC2Traits
  >;

  using PyXTC2ThreadedMPIBrokerGroup = sbio::BrokerGroup<
    PyXTC2ThreadedMPIBroker,
    sbio::XTC2Traits
  >;

  using XTC2MPIBGRef = typename PyXTC2MPIBrokerGroup::DataSegmentRef;
  using XTC2ThreadedMPIBGRef = typename PyXTC2ThreadedMPIBrokerGroup::DataSegmentRef;

  auto pympi_bg_xtc2 =
    py::classh<PyXTC2MPIBrokerGroup>(core_module, "PyXTC2MPIBrokerGroup");
  pysbio::impl::bind_broker_group_functions<PyXTC2MPIBrokerGroup, XTC2MPIBGRef>(pympi_bg_xtc2);

  auto pympi_threaded_bg_xtc2 =
    py::classh<PyXTC2ThreadedMPIBrokerGroup>(core_module, "PyXTC2ThreadedMPIBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    PyXTC2ThreadedMPIBrokerGroup, XTC2ThreadedMPIBGRef
  >(pympi_threaded_bg_xtc2);


  // DataSources
  using PyXTC2MPIDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    PyMPIExec,
    sbio::XTC2Traits
  >;
  using PyXTC2ThreadedMPIDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    PyThreadedMPIExec,
    sbio::XTC2Traits
  >;

  auto pympi_ds_xtc2 = py::classh<PyXTC2MPIDataSource>(core_module, "PyXTC2MPIDataSource");
  pysbio::impl::bind_datasource_functions<PyXTC2MPIDataSource>(pympi_ds_xtc2);

  auto pympi_threaded_ds_xtc2 =
      py::classh<PyXTC2ThreadedMPIDataSource>(core_module, "PyXTC2ThreadedMPIDataSource");
  pysbio::impl::bind_datasource_functions<PyXTC2ThreadedMPIDataSource>(pympi_threaded_ds_xtc2);

#endif

#endif // SBIO_HAS_MPI
} // core_module
