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
  using SerialBroker1 = sbio::XTC1StreamBroker<sbio::SyncPOSIXIO, sbio::SerialExecution>;

#ifdef SBIO_HAS_MPI
  using MPIBroker1 = sbio::XTC1StreamBroker<sbio::SyncPOSIXIO, sbio::MPIExecution>;

#endif
#endif

#ifdef SBIO_HAS_XTC2
  using SerialBroker2 = sbio::XTC2StreamBroker<sbio::SyncPOSIXIO, sbio::SerialExecution>;

#ifdef SBIO_HAS_MPI
  using MPIBroker2 = sbio::XTC2StreamBroker<sbio::SyncPOSIXIO, sbio::MPIExecution>;

#endif
#endif

#if defined(SBIO_HAS_XTC1) && defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using BrokerV = std::variant<SerialBroker1, SerialBroker2, MPIBroker1, MPIBroker2>;
  using BrokerPtrV = std::variant<SerialBroker1**, SerialBroker2**, MPIBroker1**, MPIBroker2**>;

#else
  using BrokerV = std::variant<SerialBroker1, SerialBroker2>;
  using BrokerPtrV = std::variant<SerialBroker1**, SerialBroker2**>;

#endif

#elif defined(SBIO_HAS_XTC1)
#ifdef SBIO_HAS_MPI
  using BrokerV = std::variant<SerialBroker1, MPIBroker1>;
  using BrokerPtrV = std::variant<SerialBroker1**, MPIBroker1**>;

#else
  using BrokerV = std::variant<SerialBroker1>;
  using BrokerPtrV = std::variant<SerialBroker1**>;

#endif

#elif defined(SBIO_HAS_XTC2)
#ifdef SBIO_HAS_MPI
  using BrokerV = std::variant<SerialBroker2, MPIBroker2>;
  using BrokerPtrV = std::variant<SerialBroker2**, MPIBroker2**>;

#else
  using BrokerV = std::variant<SerialBroker2>;
  using BrokerPtrV = std::variant<SerialBroker2**>;

#endif

#endif

  /**
   * Process the metadata from various StreamBrokers for a detector of the given name.
   *
   * This utility traverse the internal metadata structures and recomposes them into
   * a more friendly STL mapping for algorithms and data access fields. It will also
   * determine and format the serial number for the detector (if applicable).
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
