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

#ifndef PYSBIO_UTILITIES_HH
#define PYSBIO_UTILITIES_HH

#include "sbio/core/broker.hh"
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

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdlib>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace py = pybind11;

namespace pysbio {
#ifdef SBIO_HAS_XTC1
  using SerialBroker1 = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC1Traits
  >;

  using ThreadedBroker1 = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::ThreadedExecution,
    sbio::XTC1Traits
  >;
#ifdef SBIO_HAS_MPI
  using MPIBroker1 = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC1Traits
  >;

  using ThreadedMPIBroker1 = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::MPIThreadedExecution,
    sbio::XTC1Traits
  >;
#endif
#endif

#ifdef SBIO_HAS_XTC2
  using SerialBroker2 = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::SerialExecution,
    sbio::XTC2Traits
  >;

  using ThreadedBroker2 = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::ThreadedExecution,
    sbio::XTC2Traits
  >;
#ifdef SBIO_HAS_MPI
  using MPIBroker2 = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::MPIExecution,
    sbio::XTC2Traits
  >;

  using ThreadedMPIBroker2 = sbio::StreamBroker<
    sbio::SyncPOSIXIO,
    sbio::MPIThreadedExecution,
    sbio::XTC2Traits
  >;
#endif
#endif

#if defined(SBIO_HAS_XTC1) && defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using BrokerV = std::variant<
    SerialBroker1,
    SerialBroker2,
    ThreadedBroker1,
    ThreadedBroker2,
    MPIBroker1,
    MPIBroker2,
    ThreadedMPIBroker1,
    ThreadedMPIBroker2
  >;
  using BrokerPtrV = std::variant<
    SerialBroker1**,
    SerialBroker2**,
    ThreadedBroker1**,
    ThreadedBroker2**,
    MPIBroker1**,
    MPIBroker2**,
    ThreadedMPIBroker1**,
    ThreadedMPIBroker2**
  >;

#else
  using BrokerV =
    std::variant<SerialBroker1, SerialBroker2, ThreadedBroker1, ThreadedBroker2>;
  using BrokerPtrV =
    std::variant<SerialBroker1**, SerialBroker2**, ThreadedBroker1**, ThreadedBroker2**>;

#endif

#elif defined(SBIO_HAS_XTC1)
#ifdef SBIO_HAS_MPI
  using BrokerV =
    std::variant<SerialBroker1, ThreadedBroker1, MPIBroker1, ThreadedMPIBroker1>;
  using BrokerPtrV =
    std::variant<SerialBroker1**, ThreadedBroker1**, MPIBroker1**, ThreadedMPIBroker1**>;

#else
  using BrokerV = std::variant<SerialBroker1, ThreadedBroker1>;
  using BrokerPtrV = std::variant<SerialBroker1**, ThreadedBroker1**>;

#endif

#elif defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using BrokerV =
    std::variant<SerialBroker2, ThreadedBroker2, MPIBroker2, ThreadedMPIBroker2>;
  using BrokerPtrV =
    std::variant<SerialBroker2**, ThreadedBroker2**, MPIBroker2**, ThreadedMPIBroker2**>;

#else
  using BrokerV = std::variant<SerialBroker2, ThreadedBroker2>;
  using BrokerPtrV = std::variant<SerialBroker2**, ThreadedBroker2**>;

#endif

#endif

  /**
   * Process the metadata from various StreamBrokers for a detector of the given name.
   *
   * This utility traverse the internal metadata structures and recomposes them into
   * a more friendly STL mapping for algorithms and data access fields. It will also
   * determine and format the serial number for the detector (if applicable).
   *
   * @note This is a utility specificaly for managing the metadata for XTC-type
   * broker groups.
   *
   * @param[in] brokersv The set of StreamBrokers managing the detector.
   * @param[in] detector_name The name of the detector to find metadata for.
   * @param[in] num_brokers The total number of StreamBrokers managing the detector.
   * @param[out] serial_number_out Where to write the formatted serial number.
   * @returns A map of algorithm names to all the data fields under them. I.e. a
   *          lookup of the form: <algorithm name> : [<field>, <field>].
   */
  std::map<std::string, std::vector<std::string>> alg_fields(BrokerPtrV brokersv,
                                                             const char* detector_name,
                                                             std::size_t num_brokers,
                                                             std::string& serial_number_out);
} // namespace pysbio

#endif // PYSBIO_UTILITIES_HH
