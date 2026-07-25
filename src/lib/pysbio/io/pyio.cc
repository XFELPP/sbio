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

#include "sbio/core/io.hh"

#include "sbio/io/posix.hh"

#include <pybind11/native_enum.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(pyio, io_module, py::mod_gil_not_used()) {
  io_module.doc() = "sbio Python bindings for IO drivers.";

  py::native_enum<sbio::IOStatus>(io_module,
                                  "DType",
                                  "enum.Enum",
                                  "Enumerators for IO operation statuses.")
    .value("Success", sbio::IOStatus::Success)
    .value("Pending", sbio::IOStatus::Pending)
    .value("WouldBlock", sbio::IOStatus::WouldBlock)
    .value("OpenFailed", sbio::IOStatus::OpenFailed)
    .value("GeneralIOError", sbio::IOStatus::GeneralIOError)
    .value("ZeroBytesRead", sbio::IOStatus::ZeroBytesRead)
    .value("TruncatedRead", sbio::IOStatus::TruncatedRead)
    .value("FunctionUnavailable", sbio::IOStatus::FunctionUnavailable)
    .value("NoOffsetInData", sbio::IOStatus::NoOffsetInData)
    .value("HeaderReadError", sbio::IOStatus::HeaderReadError)
    .value("PayloadTruncatedError", sbio::IOStatus::PayloadTruncatedError)
    .value("AllRequestedRead", sbio::IOStatus::AllRequestedRead)
    .export_values()
    .finalize();

  py::classh<sbio::SyncPOSIXIO>(io_module, "SyncPOSIXIO")
    .def(py::init<>())
    .def("connect", &sbio::SyncPOSIXIO::connect)
    .def("read", &sbio::SyncPOSIXIO::read);
} // io_module
