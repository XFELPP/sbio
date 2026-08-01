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

#ifndef PYSBIO_PYBROKER_GROUP_HH
#define PYSBIO_PYBROKER_GROUP_HH

#include "sbio/core/broker_group.hh"
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
#include <ncarray/soarrays.hh>
#include <mpi.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <string>
#include <variant>

namespace py = pybind11;

namespace pysbio {
#ifdef SBIO_HAS_XTC1
  using SerialBrokerGroup1 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::SerialExecution, sbio::XTC1Traits>,
    sbio::XTC1Traits
  >;

  using ThreadedBrokerGroup1 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::ThreadedExecution, sbio::XTC1Traits>,
    sbio::XTC1Traits
  >;
#ifdef SBIO_HAS_MPI
  using MPIBrokerGroup1 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::MPIExecution, sbio::XTC1Traits>,
    sbio::XTC1Traits
  >;

  using ThreadedMPIBrokerGroup1 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::MPIThreadedExecution, sbio::XTC1Traits>,
    sbio::XTC1Traits
  >;
#endif
#endif

#ifdef SBIO_HAS_XTC2
  using SerialBrokerGroup2 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::SerialExecution, sbio::XTC2Traits>,
    sbio::XTC2Traits
  >;

  using ThreadedBrokerGroup2 = sbio::BrokerGroup<
      sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::ThreadedExecution, sbio::XTC2Traits>,
      sbio::XTC2Traits
  >;
#ifdef SBIO_HAS_MPI
  using MPIBrokerGroup2 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::MPIExecution, sbio::XTC2Traits>,
    sbio::XTC2Traits
  >;

  using ThreadedMPIBrokerGroup2 = sbio::BrokerGroup<
      sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::MPIThreadedExecution, sbio::XTC2Traits>,
      sbio::XTC2Traits
  >;
#endif
#endif

#if defined(SBIO_HAS_XTC1) && defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using BrokerGroupV = std::variant<
    SerialBrokerGroup1,
    SerialBrokerGroup2,
    ThreadedBrokerGroup1,
    ThreadedBrokerGroup2,
    MPIBrokerGroup1,
    MPIBrokerGroup2,
    ThreadedMPIBrokerGroup1,
    ThreadedMPIBrokerGroup2
  >;
#else
  using BrokerGroupV = std::variant<
    SerialBrokerGroup1,
    ThreadedBrokerGroup1,
    SerialBrokerGroup2,
    ThreadedBrokerGroup2
  >;
#endif

#elif defined(SBIO_HAS_XTC1)
#ifdef SBIO_HAS_MPI
  using BrokerGroupV = std::variant<
    SerialBrokerGroup1,
    ThreadedBrokerGroup1,
    MPIBrokerGroup1,
    ThreadedMPIBrokerGroup1
  >;
#else
  using BrokerGroupV = std::variant<SerialBrokerGroup1, ThreadedBrokerGroup2>;
#endif

#elif defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using BrokerGroupV = std::variant<
    SerialBrokerGroup2,
    ThreadedBrokerGroup2,
    MPIBrokerGroup2,
    ThreadedMPIBrokerGroup2
  >;
#else
  using BrokerGroupV = std::variant<SerialBrokerGroup2, ThreadedBrokerGroup2>;
#endif

#endif

  struct BrokerGroupWrapper {
    BrokerGroupWrapper(BrokerGroupV g)
      : grp(g)
    {}

    const std::string& get_detector_type() const { return detector_type; }
    const std::string& get_serial_number() const { return serial_number; }

    ncarray::SOViewFor<ncarray::HostTag> get_data(std::size_t step,
                                                  const char* alg,
                                                  const char* field);

    ncarray::SOViewFor<ncarray::HostTag> get_multi_data(std::initializer_list<std::size_t> steps,
                                                        const char* alg,
                                                        const char* field);

    BrokerGroupV grp;
    std::string detector_type;
    std::string serial_number;
  };

  struct AlgWrapper {
    AlgWrapper(BrokerGroupV g, std::string n)
      : grp(g)
      , alg_name(n)
    {}

    BrokerGroupV grp;
    std::string alg_name;
  };

  py::object wrap_xtc_detector(py::module& m, BrokerGroupV detv);
} // namespace pysbio

#endif // PYSBIO_PYBROKER_GROUP_HH
