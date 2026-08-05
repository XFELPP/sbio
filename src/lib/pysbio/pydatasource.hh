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

#include "pysbio/execution/pyexecution.hh"
#include "pysbio/formats/pyformat_traits.hh"
#include "pysbio/io/pyio.hh"

#include "sbio/core/datasource.hh"
#include "sbio/execution/serial.hh"
#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi.hh"
#include "sbio/execution/mpi_threaded.hh"
#endif
#include "sbio/execution/threaded.hh"
#include "sbio/io/posix.hh"
#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif
#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2_traits.hh"
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
  using SerialDataSource1 = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC1Traits
  >;

  using ThreadedDataSource1 = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::ThreadedExecution,
    sbio::XTC1Traits
  >;

  #ifdef SBIO_HAS_MPI
  using MPIDataSource1 = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC1Traits
  >;
  using ThreadedMPIDataSource1 = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIThreadedExecution,
    sbio::XTC1Traits
  >;
#endif
#endif

#ifdef SBIO_HAS_XTC2
  using SerialDataSource2 = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC2Traits
  >;

  using ThreadedDataSource2 = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::ThreadedExecution,
    sbio::XTC2Traits
  >;
#ifdef SBIO_HAS_MPI
  using MPIDataSource2 = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC2Traits
  >;

  using ThreadedMPIDataSource2 = sbio::DataSource<
    sbio::SyncPOSIXIO,
    sbio::MPIThreadedExecution,
    sbio::XTC2Traits
  >;
#endif
#endif

#if defined(SBIO_HAS_XTC1) && defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using DataSourceV = std::variant<
    SerialDataSource1,
    ThreadedDataSource1,
    MPIDataSource1,
    ThreadedMPIDataSource1,
    SerialDataSource2,
    ThreadedDataSource2,
    MPIDataSource2,
    ThreadedMPIDataSource2
  >;
#else
  using DataSourceV = std::variant<
    SerialDataSource1,
    ThreadedDataSource1,
    SerialDataSource2,
    ThreadedDataSource2
  >;
#endif

#elif defined(SBIO_HAS_XTC1)
#ifdef SBIO_HAS_MPI
  using DataSourceV =
    std::variant<SerialDataSource1, ThreadedDataSource1, MPIDataSource1, ThreadedMPIDataSource1>;
#else
  using DataSourceV = std::variant<SerialDataSource1, ThreadedDataSource1>;
#endif

#elif defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using DataSourceV =
    std::variant<SerialDataSource2, ThreadedDataSource2, MPIDataSource2, ThreadedMPIDataSource2>;
#else
  using DataSourceV = std::variant<SerialDataSource2, ThreadedDataSource2>;
#endif

#endif

  class PyDataSource {
  public:
    PyDataSource(std::string exp,
                 unsigned run,
                 int evt_per_read,
                 int dgram_size,
                 pysbio::FTraits data_fmt = pysbio::FTraits::XTC2,
                 pysbio::ExecutionPolicy epolicy = pysbio::ExecutionPolicy::MPI,
                 py::dict exec_cfg = py::dict(),
                 pysbio::IOPolicy io_policy = pysbio::IOPolicy::SyncPOSIX);

    py::object group(py::module& m, const char* name);

    std::size_t next();

    // TODO: Switch to making this const after the ConstIterator works when
    //       DataSource::next is properly restructured to support it.
    DataSourceV& ds() { return m_ds; }

  private:
    DataSourceV m_ds;

    std::string m_exp;
    unsigned m_run;
  };

} // namespace pysbio

#endif // PYSBIO_PYDATASOURCE_HH
