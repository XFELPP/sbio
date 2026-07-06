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

#ifndef PYSBIO_PYDATASOURCE_HH
#define PYSBIO_PYDATASOURCE_HH

#include "sbio/core/datasource.hh"
#ifdef SBIO_HAS_MPI
#include "sbio/mpi/execution.hh"
#endif
#include "sbio/posix_io.hh"
#ifdef SBIO_HAS_XTC1
#include "sbio/xtc1_broker.hh"
#endif
#ifdef SBIO_HAS_XTC2
#include "sbio/xtc2_broker.hh"
#endif

#include <ncarray/ncarrays.hh>
#include <mpi.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdlib>
#include <string>
#include <variant>

namespace py = pybind11;

namespace pysbio {
#ifdef SBIO_HAS_XTC1
  using SerialDataSource1 = sbio::IDataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC1Traits,
    sbio::XTC1StreamBroker<sbio::SyncPOSIXIO, sbio::SerialExecution>
  >;

#ifdef SBIO_HAS_MPI
  using MPIDataSource1 = sbio::IDataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC1Traits,
    sbio::XTC1StreamBroker<sbio::SyncPOSIXIO, sbio::MPIExecution>
  >;
#endif
#endif

#ifdef SBIO_HAS_XTC2
  using SerialDataSource2 = sbio::IDataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC2Traits,
    sbio::XTC2StreamBroker<sbio::SyncPOSIXIO, sbio::SerialExecution>
  >;

#ifdef SBIO_HAS_MPI
  using MPIDataSource2 = sbio::IDataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC2Traits,
    sbio::XTC2StreamBroker<sbio::SyncPOSIXIO, sbio::MPIExecution>
  >;
#endif
#endif

#if defined(SBIO_HAS_XTC1) && defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using DataSourceV = std::variant<
    SerialDataSource1,
    MPIDataSource1,
    SerialDataSource2,
    MPIDataSource2
  >;
#else
  using DataSourceV = std::variant<SerialDataSource1, SerialDataSource2>;
#endif

#elif defined(SBIO_HAS_XTC1)
#ifdef SBIO_HAS_MPI
  using DataSourceV = std::variant<SerialDataSource1, MPIDataSource1>;
#else
  using DataSourceV = std::variant<SerialDataSource1>;
#endif

#elif defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using DataSourceV = std::variant<SerialDataSource2, MPIDataSource2>;
#else
  using DataSourceV = std::variant<SerialDataSource2>;
#endif

#endif

  class PyDataSource {
  public:
    PyDataSource(std::string type,
                 std::string exp,
                 unsigned run,
                 int evt_per_read,
                 int dgram_size,
                 unsigned xtc_ver = 2);

    py::object detector(py::module& m, const char* name);

    std::size_t next();

    // TODO: Switch to making this const after the ConstIterator works when
    //       IDataSource::next is properly restructured to support it.
    DataSourceV& ds() { return m_ds; }

  private:
    DataSourceV m_ds;
  };

} // namespace pysbio

#endif // PYSBIO_PYDATASOURCE_HH
