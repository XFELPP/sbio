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

#ifndef PYSBIO_PYDETECTOR_HH
#define PYSBIO_PYDETECTOR_HH

#include "sbio/core/broker_group.hh"
#include "sbio/core/datasource.hh"
#include "sbio/execution/serial.hh"
#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi.hh"
#endif
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
#include <memory>
#include <string>
#include <variant>

namespace py = pybind11;

namespace pysbio {
#ifdef SBIO_HAS_XTC1
  using SerialDetector1 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::SerialExecution, sbio::XTC1Traits>,
    sbio::XTC1Traits
  >;

#ifdef SBIO_HAS_MPI
  using MPIDetector1 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::MPIExecution, sbio::XTC1Traits>,
    sbio::XTC1Traits
  >;
#endif
#endif

#ifdef SBIO_HAS_XTC2
  using SerialDetector2 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::SerialExecution, sbio::XTC2Traits>,
    sbio::XTC2Traits
  >;

#ifdef SBIO_HAS_MPI
  using MPIDetector2 = sbio::BrokerGroup<
    sbio::StreamBroker<sbio::SyncPOSIXIO, sbio::MPIExecution, sbio::XTC2Traits>,
    sbio::XTC2Traits
  >;
#endif
#endif

#if defined(SBIO_HAS_XTC1) && defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using DetectorV = std::variant<SerialDetector1, SerialDetector2, MPIDetector1, MPIDetector2>;
#else
  using DetectorV = std::variant<SerialDetector1, SerialDetector2>;
#endif

#elif defined(SBIO_HAS_XTC1)
#ifdef SBIO_HAS_MPI
  using DetectorV = std::variant<SerialDetector1, MPIDetector1>;
#else
  using DetectorV = std::variant<SerialDetector1>;
#endif

#elif defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using DetectorV = std::variant<SerialDetector2, MPIDetector2>;
#else
  using DetectorV = std::variant<SerialDetector2>;
#endif

#endif

  struct DetectorWrapper {
    DetectorWrapper(DetectorV d)
      : det(d)
    {}

    const std::string& get_detector_type() const { return detector_type; }
    const std::string& get_serial_number() const { return serial_number; }

    DetectorV det;
    std::string detector_type;
    std::string serial_number;
  };

  struct AlgWrapper {
    AlgWrapper(DetectorV d, std::string n)
      : det(d)
      , alg_name(n)
    {}

    DetectorV det;
    std::string alg_name;
  };

  py::object wrap_detector(py::module& m, DetectorV detv);
} // namespace pysbio

#endif // PYSBIO_PYDETECTOR_HH
