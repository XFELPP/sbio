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

#include "sbio/core/datasource.hh"
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

PYBIND11_MODULE(pydatasources, datasource_module, py::mod_gil_not_used()) {
  datasource_module.doc() = "sbio Python bindings for DataSources.";

#ifdef SBIO_HAS_XTC1
  using XTC1SerialDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC1Traits
  >;

  using XTC1ThreadedDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::ThreadedExecution,
    sbio::XTC1Traits
  >;

  auto serial_ds_xtc1 =
      py::classh<XTC1SerialDataSource>(datasource_module, "XTC1SerialDataSource");
  pysbio::impl::bind_datasource_functions<XTC1SerialDataSource>(serial_ds_xtc1);

  auto threaded_ds_xtc1 =
      py::classh<XTC1ThreadedDataSource>(datasource_module, "XTC1ThreadedDataSource");
  pysbio::impl::bind_broker_group_functions<XTC1ThreadedDataSource>(threaded_ds_xtc1);

#ifdef SBIO_HAS_MPI
  using XTC1MPIDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC1Traits
  >;

  using XTC1ThreadedMPIDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIThreadedExecution,
    sbio::XTC1Traits
  >;

  auto mpi_ds_xtc1 =
    py::classh<XTC1MPIDataSource>(datasource_module, "XTC1MPIDataSource");
  pysbio::impl::bind_datasource_functions<XTC1MPIDataSource>(mpi_ds_xtc1);

  auto mpi_threaded_ds_xtc1 =
      py::classh<XTC1ThreadedMPIDataSource>(datasource_module,
                                            "XTC1ThreadedMPIDataSource");
  pysbio::impl::bind_broker_group_functions<XTC1ThreadedMPIDataSource>(mpi_threaded_ds_xtc1);

#endif // SBIO_HAS_MPI
#endif // SBIO_HAS_XTC1

#ifdef SBIO_HAS_XTC2
  using XTC2SerialDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC2Traits
  >;

  using XTC2ThreadedDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::ThreadedExecution,
    sbio::XTC2Traits
  >;

  auto serial_ds_xtc2 =
    py::classh<XTC2SerialDataSource>(datasource_module, "XTC2SerialDataSource");
  pysbio::impl::bind_datasource_functions<XTC2SerialDataSource>(serial_ds_xtc2);

  auto threaded_ds_xtc2 =
      py::classh<XTC2ThreadedDataSource>(datasource_module, "XTC2ThreadedDataSource");
  pysbio::impl::bind_datasource_functions<XTC2ThreadedDataSource>(threaded_ds_xtc2);

#ifdef SBIO_HAS_MPI
  using XTC2MPIDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC2Traits
  >;

  using XTC2ThreadedMPIDataSource = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIThreadedExecution,
    sbio::XTC2Traits
  >;

  auto mpi_ds_xtc2 =
    py::classh<XTC2MPIDataSource>(datasource_module, "XTC2MPIDataSource");
  pysbio::impl::bind_datasource_functions<XTC2MPIDataSource>(mpi_ds_xtc2);

  auto mpi_threaded_ds_xtc2 =
      py::classh<XTC2ThreadedMPIDataSource>(datasource_module, "XTC2ThreadedMPIDataSource");
  pysbio::impl::bind_datasource_functions<XTC2ThreadedMPIDataSource>(mpi_threaded_ds_xtc2);

#endif // SBIO_HAS_MPI
#endif // SBIO_HAS_XTC2
}
