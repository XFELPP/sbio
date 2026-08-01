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

#include "pysbio/pybroker_group.hh"

#include "pysbio/utilities.hh"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>

namespace py = pybind11;

namespace pysbio {
  py::object wrap_xtc_detector(py::module& m, BrokerGroupV detv) {
    // Use the internal MethodType to make sure attached data getters don't require
    // passing `self` as the first argument (i.e., they behave as true Python methods)
    auto MethodType = py::module_::import("types").attr("MethodType");

    py::object py_det { py::cast(std::make_shared<BrokerGroupWrapper>(detv)) };
    std::shared_ptr<BrokerGroupWrapper> cpp_det {
      py_det.cast<std::shared_ptr<BrokerGroupWrapper>>()
    };

    auto det_visitor = [&](auto& det) {
      using SGT = std::decay_t<decltype(det)>;
      using StepIdx = typename SGT::StepIdxType;

      std::string det_serial_number;
      auto alg_and_fields =
          alg_fields(det.stream_brokers(),
                     det.group_name(),
                     det.num_stream_brokers(),
                     det_serial_number);

      cpp_det->detector_type = det.group_type();
      cpp_det->serial_number = det_serial_number;

      for (auto const& [alg, fields] : alg_and_fields) {
        auto py_alg { py::cast(std::make_shared<AlgWrapper>(det, alg)) };

        for (auto const& field : fields) {
          auto cpp_data_getter = [det, alg, field](AlgWrapper& self,
                                                   StepIdx step) {
            return det.get_data(step, alg.c_str(), field.c_str());
          };

          auto py_data_getter = py::cpp_function(cpp_data_getter, py::is_method(py_alg));

          // For each "field", attach it to the "algorithm" as a data fetch method
          py::setattr(py_alg, field.c_str(), MethodType(py_data_getter, py_alg));
        }

        // For each "algorithm", attach it to the detector as an object with data
        // fetching methods for each of its associated fields
        py::setattr(py_det, alg.c_str(), py_alg);
      }
    };

    std::visit(det_visitor, detv);

    return py_det;
  }

  ncarray::SOViewFor<ncarray::HostTag>
  BrokerGroupWrapper::get_data(std::size_t step, const char* alg, const char* field) {
    auto getter = [&](auto& grp) {

      return grp.get_data(step, alg, field);
    };

    return std::visit(getter, this->grp);
  }

  ncarray::SOViewFor<ncarray::HostTag>
  BrokerGroupWrapper::get_multi_data(std::initializer_list<std::size_t> steps,
                                     const char* alg,
                                     const char* field) {
    auto getter = [&](auto& grp) { return grp.get_multi_data(steps, alg, field); };

    return std::visit(getter, this->grp);
  }
} // namespace pysbio
