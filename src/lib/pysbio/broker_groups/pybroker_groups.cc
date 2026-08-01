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
#include "sbio/core/broker_group.hh"
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

PYBIND11_MODULE(pybroker_groups, broker_group_module, py::mod_gil_not_used()) {
  broker_group_module.doc() = "sbio Python bindings for Broker Groups.";

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

  using XTC1SerialBrokerGroup = sbio::BrokerGroup<
    XTC1SerialBroker,
    sbio::XTC1Traits
  >;

  using XTC1ThreadedBrokerGroup = sbio::BrokerGroup<
    XTC1ThreadedBroker,
    sbio::XTC1Traits
  >;

  using XTC1SerialBGRef = typename XTC1SerialBrokerGroup::DataSegmentRef;
  using XTC1ThreadedBGRef = typename XTC1ThreadedBrokerGroup::DataSegmentRef;

  auto serial_bg_xtc1 =
      py::classh<XTC1SerialBrokerGroup>(broker_group_module,
                                        "XTC1SerialBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    XTC1SerialBrokerGroup, XTC1SerialBGRef
  >(serial_bg_xtc1);

  auto threaded_bg_xtc1 =
      py::classh<XTC1ThreadedBrokerGroup>(broker_group_module,
                                          "XTC1ThreadedBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    XTC1ThreadedBrokerGroup, XTC1ThreadedBGRef
  >(threaded_bg_xtc1);

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

  using XTC1MPIBrokerGroup = sbio::BrokerGroup<
    XTC1MPIBroker,
    sbio::XTC1Traits
  >;

  using XTC1ThreadedMPIBrokerGroup = sbio::BrokerGroup<
    XTC1ThreadedMPIBroker,
    sbio::XTC1Traits
  >;

  using XTC1MPIBGRef = typename XTC1MPIBrokerGroup::DataSegmentRef;
  using XTC1ThreadedMPIBGRef = typename XTC1ThreadedMPIBrokerGroup::DataSegmentRef;

  auto mpi_bg_xtc1 =
    py::classh<XTC1MPIBrokerGroup>(broker_group_module,
                                   "XTC1MPIBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    XTC1MPIBrokerGroup, XTC1MPIBGRef
  >(mpi_bg_xtc1);

  auto mpi_threaded_bg_xtc1 =
    py::classh<XTC1ThreadedMPIBrokerGroup>(broker_group_module,
                                           "XTC1ThreadedMPIBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    XTC1ThreadedMPIBrokerGroup, XTC1ThreadedMPIBGRef
  >(mpi_threaded_bg_xtc1);


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

  using XTC2SerialBrokerGroup = sbio::BrokerGroup<
    XTC2SerialBroker,
    sbio::XTC2Traits
  >;

  using XTC2ThreadedBrokerGroup = sbio::BrokerGroup<
    XTC2ThreadedBroker,
    sbio::XTC2Traits
  >;

  using XTC2SerialBGRef = typename XTC2SerialBrokerGroup::DataSegmentRef;
  using XTC2ThreadedBGRef = typename XTC2ThreadedBrokerGroup::DataSegmentRef;

  auto serial_bg_xtc2 =
    py::classh<XTC2SerialBrokerGroup>(broker_group_module,
                                      "XTC2SerialBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    XTC2SerialBrokerGroup, XTC2SerialBGRef
  >(serial_bg_xtc2);

  auto threaded_bg_xtc2 =
    py::classh<XTC2ThreadedBrokerGroup>(broker_group_module,
                                        "XTC2ThreadedBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    XTC2ThreadedBrokerGroup, XTC2ThreadedBGRef
  >(threaded_bg_xtc2);


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

  using XTC2MPIBrokerGroup = sbio::BrokerGroup<
    XTC2MPIBroker,
    sbio::XTC2Traits
  >;

  using XTC2ThreadedMPIBrokerGroup = sbio::BrokerGroup<
    XTC2ThreadedMPIBroker,
    sbio::XTC2Traits
  >;

  using XTC2MPIBGRef = typename XTC2MPIBrokerGroup::DataSegmentRef;
  using XTC2ThreadedMPIBGRef = typename XTC2ThreadedMPIBrokerGroup::DataSegmentRef;

  auto mpi_bg_xtc2 =
    py::classh<XTC2MPIBrokerGroup>(broker_group_module,
                                   "XTC2MPIBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    XTC2MPIBrokerGroup, XTC2MPIBGRef
  >(mpi_bg_xtc2);

  auto mpi_threaded_bg_xtc2 =
    py::classh<XTC2ThreadedMPIBrokerGroup>(broker_group_module,
                                           "XTC2ThreadedMPIBrokerGroup");
  pysbio::impl::bind_broker_group_functions<
    XTC2ThreadedMPIBrokerGroup, XTC2ThreadedMPIBGRef
  >(mpi_threaded_bg_xtc2);

#endif
#endif
}
