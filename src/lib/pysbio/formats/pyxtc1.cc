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

#include "sbio/formats/xtc1/xtc1.hh"

#include <pybind11/native_enum.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>

namespace py = pybind11;

PYBIND11_MODULE(pyxtc1, xtc1_module, py::mod_gil_not_used()) {
  xtc1_module.doc() = "sbio Python bindings for the XTC1 data format.";

  py::native_enum<sbio::XTC1::DType>(xtc1_module,
                                     "DType",
                                     "enum.Enum",
                                     "Type enumerators supported by XTC1.")
    .value("UINT8", sbio::XTC1::DType::UINT8)
    .value("UINT16", sbio::XTC1::DType::UINT16)
    .value("UINT32", sbio::XTC1::DType::UINT32)
    .value("UINT64", sbio::XTC1::DType::UINT64)
    .value("INT8", sbio::XTC1::DType::INT8)
    .value("INT16", sbio::XTC1::DType::INT16)
    .value("INT32", sbio::XTC1::DType::INT32)
    .value("INT64", sbio::XTC1::DType::INT64)
    .value("FLOAT", sbio::XTC1::DType::FLOAT)
    .value("DOUBLE", sbio::XTC1::DType::DOUBLE)
    .value("CHARSTR", sbio::XTC1::DType::CHARSTR)
    .value("ENUMVAL", sbio::XTC1::DType::ENUMVAL)
    .value("ENUMDICT", sbio::XTC1::DType::ENUMDICT)
    .export_values()
    .finalize();

  py::native_enum<sbio::XTC1::TransitionId::Value>(xtc1_module,
                                                   "TransitionId",
                                                   "enum.Enum",
                                                   "Transition ID enumerators supported by XTC1.")
    .value("Unknown", sbio::XTC1::TransitionId::Unknown)
    .value("Reset", sbio::XTC1::TransitionId::Reset)
    .value("Map", sbio::XTC1::TransitionId::Map)
    .value("Unmap", sbio::XTC1::TransitionId::Unmap)
    .value("Configure", sbio::XTC1::TransitionId::Configure)
    .value("Unconfigure", sbio::XTC1::TransitionId::Unconfigure)
    .value("BeginRun", sbio::XTC1::TransitionId::BeginRun)
    .value("EndRun", sbio::XTC1::TransitionId::EndRun)
    .value("BeginCalibCycle", sbio::XTC1::TransitionId::BeginCalibCycle)
    .value("EndCalibCycle", sbio::XTC1::TransitionId::EndCalibCycle)
    .value("Enable", sbio::XTC1::TransitionId::Enable)
    .value("Disable", sbio::XTC1::TransitionId::Disable)
    .value("L1Accept", sbio::XTC1::TransitionId::L1Accept)
    .export_values()
    .finalize();


  py::native_enum<sbio::XTC1::Level::Type>(xtc1_module,
                                           "Level",
                                           "enum.Enum",
                                           "Levels of the XTC1 sources.")
    .value("Control", sbio::XTC1::Level::Control)
    .value("Source", sbio::XTC1::Level::Source)
    .value("Segment", sbio::XTC1::Level::Segment)
    .value("Event", sbio::XTC1::Level::Event)
    .value("Recorder", sbio::XTC1::Level::Recorder)
    .value("Observer", sbio::XTC1::Level::Observer)
    .value("Reporter", sbio::XTC1::Level::Reporter)
    .export_values()
    .finalize();

  // There are many type enumerators (get them with TypeId.name)
  py::classh<sbio::XTC1::TypeId>(xtc1_module, "TypeId")
    .def(py::init([](std::uint32_t type, std::uint32_t version) {
           return sbio::XTC1::TypeId(static_cast<sbio::XTC1::TypeId::Type>(type), version);
         }),
         py::arg("type"),
         py::arg("version"))
    .def("value", &sbio::XTC1::TypeId::value)
    .def("version", &sbio::XTC1::TypeId::version)
    .def("id", [](const sbio::XTC1::TypeId& self) { return static_cast<std::uint32_t>(self.id()); })
    .def("compressed", &sbio::XTC1::TypeId::compressed)
    .def("is_configuration", &sbio::XTC1::TypeId::is_configuration)
    .def_static("name", [](std::uint32_t type) {
                  return std::string(sbio::XTC1::TypeId::name(static_cast<sbio::XTC1::TypeId::Type>(type)));
                },
                py::arg("type"));

  py::classh<sbio::XTC1::Src>(xtc1_module, "Src")
    .def(py::init<>())
    .def("log", &sbio::XTC1::Src::log)
    .def("phy", &sbio::XTC1::Src::phy)
    .def("level", &sbio::XTC1::Src::level);

  py::classh<sbio::XTC1::Damage>(xtc1_module, "Damage")
    .def(py::init<std::uint32_t>())
    .def("value", &sbio::XTC1::Damage::value)
    .def("userBits", [](const sbio::XTC1::Damage& self) { return self.userBits(); });

  // Xtcs are views into datagrams (Xtc defines placement operator new) and cannot be constructed
  py::classh<sbio::XTC1::Xtc>(xtc1_module, "Xtc")
    .def_readonly("damage", &sbio::XTC1::Xtc::damage)
    .def_readonly("src", &sbio::XTC1::Xtc::src)
    .def_readonly("contains", &sbio::XTC1::Xtc::contains)
    .def_readonly("extent", &sbio::XTC1::Xtc::extent)
    .def("sizeofPayload", &sbio::XTC1::Xtc::sizeofPayload);

  py::classh<sbio::XTC1::ClockTime>(xtc1_module, "ClockTime")
    .def(py::init<unsigned, unsigned>(), py::arg("sec"), py::arg("nsec"))
    .def("seconds", &sbio::XTC1::ClockTime::seconds)
    .def("nanoseconds", &sbio::XTC1::ClockTime::nanoseconds)
    .def("asDouble", &sbio::XTC1::ClockTime::asDouble);

  py::classh<sbio::XTC1::TimeStamp>(xtc1_module, "TimeStamp")
    .def(py::init<>())
    .def("ticks", &sbio::XTC1::TimeStamp::ticks)
    .def("fiducials", &sbio::XTC1::TimeStamp::fiducials)
    .def("control", &sbio::XTC1::TimeStamp::control)
    .def("vector", &sbio::XTC1::TimeStamp::vector);

  py::classh<sbio::XTC1::Sequence>(xtc1_module, "Sequence")
    .def(py::init<>())
    .def("service", &sbio::XTC1::Sequence::service)
    .def("isEvent", &sbio::XTC1::Sequence::isEvent)
    .def("clock", &sbio::XTC1::Sequence::clock)
    .def("stamp", &sbio::XTC1::Sequence::stamp);

  py::classh<sbio::XTC1::Dgram>(xtc1_module, "Dgram")
    .def_readonly("seq", &sbio::XTC1::Dgram::seq)
    .def_readonly("xtc", &sbio::XTC1::Dgram::xtc)
    .def("service", &sbio::XTC1::Dgram::service);
} // xtc1_module
