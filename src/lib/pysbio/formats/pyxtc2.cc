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

#include "sbio/formats/xtc2/xtc2.hh"

#include <pybind11/native_enum.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>

namespace py = pybind11;

PYBIND11_MODULE(pyxtc2, xtc2_module, py::mod_gil_not_used()) {
  xtc2_module.doc() = "sbio Python bindings for the XTC2 data format.";

  py::native_enum<sbio::XTC2::DType>(xtc2_module,
                                     "DType",
                                     "enum.Enum",
                                     "Type enumerators supported by XTC2.")
    .value("UINT8", sbio::XTC2::DType::UINT8)
    .value("UINT16", sbio::XTC2::DType::UINT16)
    .value("UINT32", sbio::XTC2::DType::UINT32)
    .value("UINT64", sbio::XTC2::DType::UINT64)
    .value("INT8", sbio::XTC2::DType::INT8)
    .value("INT16", sbio::XTC2::DType::INT16)
    .value("INT32", sbio::XTC2::DType::INT32)
    .value("INT64", sbio::XTC2::DType::INT64)
    .value("FLOAT", sbio::XTC2::DType::FLOAT)
    .value("DOUBLE", sbio::XTC2::DType::DOUBLE)
    .value("CHARSTR", sbio::XTC2::DType::CHARSTR)
    .value("ENUMVAL", sbio::XTC2::DType::ENUMVAL)
    .value("ENUMDICT", sbio::XTC2::DType::ENUMDICT)
    .export_values()
    .finalize();

  py::native_enum<sbio::XTC2::TransitionId>(xtc2_module,
                                            "TransitionId",
                                            "enum.Enum",
                                            "Transition ID enumerators supported by XTC2.")
    .value("ClearReadout", sbio::XTC2::TransitionId::ClearReadout)
    .value("Reset", sbio::XTC2::TransitionId::Reset)
    .value("Configure", sbio::XTC2::TransitionId::Configure)
    .value("Unconfigure", sbio::XTC2::TransitionId::Unconfigure)
    .value("BeginRun", sbio::XTC2::TransitionId::BeginRun)
    .value("EndRun", sbio::XTC2::TransitionId::EndRun)
    .value("BeginStep", sbio::XTC2::TransitionId::BeginStep)
    .value("EndStep", sbio::XTC2::TransitionId::EndStep)
    .value("Enable", sbio::XTC2::TransitionId::Enable)
    .value("Disable", sbio::XTC2::TransitionId::Disable)
    .value("SlowUpdate", sbio::XTC2::TransitionId::SlowUpdate)
    .value("Unused_11", sbio::XTC2::TransitionId::Unused_11)
    .value("L1Accept", sbio::XTC2::TransitionId::L1Accept)
    .value("NumberOfTransitionIds", sbio::XTC2::TransitionId::NumberOf)
    .export_values()
    .finalize();


  // NOTE: The enumerator names would conflict with struct names which breaks the
  //       pybind11 binding strategy since they're global objects. Append `T` to each
  py::native_enum<sbio::XTC2::TypeId::Type>(xtc2_module,
                                            "TypeIdType",
                                            "enum.Enum",
                                            "Types of XTC Containers.")
    .value("ParentT", sbio::XTC2::TypeId::Type::Parent)
    .value("ShapesDataT", sbio::XTC2::TypeId::Type::ShapesData)
    .value("ShapesT", sbio::XTC2::TypeId::Type::Shapes)
    .value("DataT", sbio::XTC2::TypeId::Type::Data)
    .value("NamesT", sbio::XTC2::TypeId::Type::Names)
    .value("NumberOfXtcTypes", sbio::XTC2::TypeId::Type::NumberOf)
    .export_values()
    .finalize();

  py::classh<sbio::XTC2::TypeId>(xtc2_module, "TypeId")
    .def(py::init<sbio::XTC2::TypeId::Type, unsigned>(),
         py::arg("type"),
         py::arg("version"))
    .def(py::init<>())
    .def("value", &sbio::XTC2::TypeId::value)
    .def("version", &sbio::XTC2::TypeId::version)
    .def("id", &sbio::XTC2::TypeId::id);

  py::classh<sbio::XTC2::Src>(xtc2_module, "Src")
    // .def(py::init<std::uint32_t>()) // TODO: Figure out placement new
    .def("value", &sbio::XTC2::Src::value);

  py::native_enum<sbio::XTC2::Damage::Value>(xtc2_module,
                                             "DamageValue",
                                             "enum.Enum",
                                             "Types of Damage marker.")
    .value("Truncate", sbio::XTC2::Damage::Value::Truncate)
    .value("OutOfOrder", sbio::XTC2::Damage::Value::OutOfOrder)
    .value("OutOfSynch", sbio::XTC2::Damage::Value::OutOfSynch)
    .value("Corrupted", sbio::XTC2::Damage::Value::Corrupted)
    .value("DroppedContribution", sbio::XTC2::Damage::Value::DroppedContribution)
    .value("MissingData", sbio::XTC2::Damage::Value::MissingData)
    .value("TimedOut", sbio::XTC2::Damage::Value::TimedOut)
    .value("UserDefined", sbio::XTC2::Damage::Value::UserDefined)
    .export_values()
    .finalize();

  py::classh<sbio::XTC2::Damage>(xtc2_module, "Damage")
    // .def(py::init<std::uint16_t>()) // TODO: Figure out placement new
    .def("value", &sbio::XTC2::Damage::value)
    .def("increase", [&](sbio::XTC2::Damage& self, sbio::XTC2::Damage::Value val) {
      self.increase(val);
    })
    .def("increase", [&](sbio::XTC2::Damage& self, std::uint16_t val) {
      self.increase(val);
    })
    .def("bits", &sbio::XTC2::Damage::bits)
    .def("userBits", [&](sbio::XTC2::Damage& self) { return self.userBits(); })
    .def("userBits", [&](sbio::XTC2::Damage& self, std::uint16_t val) {
      self.userBits(val);
    });

  py::classh<sbio::XTC2::Xtc>(xtc2_module, "Xtc")
    .def(py::init<>())
    .def(py::init<sbio::XTC2::Src, sbio::XTC2::Damage, sbio::XTC2::TypeId, std::uint32_t>())
    .def_readwrite("src", &sbio::XTC2::Xtc::src)
    .def_readwrite("damage", &sbio::XTC2::Xtc::damage)
    .def_readwrite("contains", &sbio::XTC2::Xtc::contains)
    .def_readwrite("extent", &sbio::XTC2::Xtc::extent)
    .def("payload", [&](const sbio::XTC2::Xtc& self) { return self.payload(); })
    .def("payload", [&](sbio::XTC2::Xtc& self) { return self.payload(); })
    .def("sizeofPayload", &sbio::XTC2::Xtc::sizeofPayload)
    .def("next", [&](const sbio::XTC2::Xtc& self) { return self.next(); })
    .def("next", [&](sbio::XTC2::Xtc& self) { return self.next(); });

  py::native_enum<sbio::XTC2::Dgram::Type>(xtc2_module,
                                           "DgramType",
                                           "enum.Enum",
                                           "Dgram type marker.")
    .value("Event", sbio::XTC2::Dgram::Type::Event)
    .value("Ocurrence", sbio::XTC2::Dgram::Type::Occurrence)
    .value("Marker", sbio::XTC2::Dgram::Type::Marker)
    // NumberOfTypes cannot be bound.
    .export_values()
    .finalize();

  py::classh<sbio::XTC2::Dgram>(xtc2_module, "Dgram")
    .def(py::init<>())
    .def(py::init<std::uint32_t, std::uint32_t, std::uint32_t, sbio::XTC2::Xtc>())
    .def_readwrite("nanoseconds", &sbio::XTC2::Dgram::nanoseconds)
    .def_readwrite("seconds", &sbio::XTC2::Dgram::seconds)
    .def_readwrite("env", &sbio::XTC2::Dgram::env)
    .def_readwrite("xtc", &sbio::XTC2::Dgram::xtc)
    .def("readoutGroups", &sbio::XTC2::Dgram::readoutGroups)
    .def("control", &sbio::XTC2::Dgram::control)
    .def("service", &sbio::XTC2::Dgram::service)
    .def("type", &sbio::XTC2::Dgram::type)
    .def("isEvent", &sbio::XTC2::Dgram::isEvent)
    .def("reserved", &sbio::XTC2::Dgram::reserved)
    .def("keepRaw", &sbio::XTC2::Dgram::keepRaw);

  py::classh<sbio::XTC2::AlgVersion>(xtc2_module, "AlgVersion")
    .def(py::init<std::uint8_t, std::uint8_t, std::uint8_t>())
    .def("major", &sbio::XTC2::AlgVersion::major)
    .def("minor", &sbio::XTC2::AlgVersion::minor)
    .def("micro", &sbio::XTC2::AlgVersion::micro)
    .def("version", &sbio::XTC2::AlgVersion::version);

  py::classh<sbio::XTC2::Alg>(xtc2_module, "Alg")
    .def(py::init<const char*, std::uint8_t, std::uint8_t, std::uint8_t>())
    .def("version", &sbio::XTC2::Alg::version)
    .def("name", &sbio::XTC2::Alg::name);

  py::classh<sbio::XTC2::Name>(xtc2_module, "Name")
    .def(py::init<>())
    .def(py::init<const char*, sbio::XTC2::DType, std::uint32_t>())
    .def(py::init<const char*, sbio::XTC2::DType, sbio::XTC2::Alg&>())
    .def(py::init<const char*, sbio::XTC2::Alg&>())
    .def("name", &sbio::XTC2::Name::name)
    .def("type", &sbio::XTC2::Name::type)
    .def("rank", &sbio::XTC2::Name::rank)
    .def("alg", [&](const sbio::XTC2::Name& self) { return self.alg(); })
    .def("alg", [&](sbio::XTC2::Name& self) { return self.alg(); });

  py::classh<sbio::XTC2::Shape>(xtc2_module, "Shape")
    // .def(py::init<>()) // TODO: Figure out placement new
    .def("size", [&](const sbio::XTC2::Shape& self, const sbio::XTC2::Name& name) {
      return self.size(name);
    })
    .def("size", [&](sbio::XTC2::Shape& self, sbio::XTC2::Name& name) {
      return self.size(name);
    })
    .def("shape", [&](const sbio::XTC2::Shape& self) { return self.shape(); })
    .def("shape", [&](sbio::XTC2::Shape& self) { return self.shape(); });

  py::classh<sbio::XTC2::NamesId>(xtc2_module, "NamesId")
    .def(py::init<>())
    .def("nodeId", &sbio::XTC2::NamesId::nodeId)
    .def("namesId", &sbio::XTC2::NamesId::namesId);

  py::classh<sbio::XTC2::Names>(xtc2_module, "Names")
    // .def(py::init<>()) // TODO: Figure out placement new
    .def("numArrays", &sbio::XTC2::Names::numArrays)
    .def("detName", &sbio::XTC2::Names::detName)
    .def("detType", &sbio::XTC2::Names::detType)
    .def("detId", &sbio::XTC2::Names::detId)
    .def("segment", &sbio::XTC2::Names::segment)
    .def("alg", [&](sbio::XTC2::Names& self) { return self.alg(); })
    .def("alg", [&](const sbio::XTC2::Names& self) { return self.alg(); })
    .def("get", &sbio::XTC2::Names::get)
    .def("num", &sbio::XTC2::Names::num)
    .def("namesId", &sbio::XTC2::Names::namesId);


  py::classh<sbio::XTC2::Data>(xtc2_module, "Data")
    .def(py::init<>());

  py::classh<sbio::XTC2::ShapesData>(xtc2_module, "ShapesData")
    .def(py::init<>())
    .def("namesId", [&](const sbio::XTC2::ShapesData& self) { return self.namesId(); })
    .def("namesId", [&](sbio::XTC2::ShapesData& self) { return self.namesId(); })
    .def("data", &sbio::XTC2::ShapesData::data)
    .def("shapes", &sbio::XTC2::ShapesData::shapes);
} // xtc2_module
