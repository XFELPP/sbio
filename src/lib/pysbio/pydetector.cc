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

#include "pysbio/pydetector.hh"

#include "pysbio/utilities.hh"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <string>
#include <type_traits>
#include <variant>

namespace py = pybind11;

namespace pysbio {
  py::object wrap_detector(py::module& m, DetectorV detv) {
    // Use the internal MethodType to make sure attached data getters don't require
    // passing `self` as the first argument (i.e., they behave as true Python methods)
    auto MethodType = py::module_::import("types").attr("MethodType");

    py::object py_det { py::cast(std::make_shared<DetectorWrapper>(detv)) };
    std::shared_ptr<DetectorWrapper> cpp_det {
      py_det.cast<std::shared_ptr<DetectorWrapper>>()
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

          // Setup a calibration routine if applicable for the data field
          if (alg == "raw" && field == "raw") {
            // TODO: Expose a callback hook for BrokerGroups that allows running
            //       after collecting all portions (only supports per-seg callbacks atm)
            // TODO: For Python bindings add the context/pipeline bindings which
            //       already support this.
            // TODO: Allow calibration to run per-segment as well.
            auto calibrate_data = [det, alg, field, cpp_det](AlgWrapper& self, StepIdx step) {
              if (!cpp_det->calibrator) {
                throw std::runtime_error("Run the Calibration staging first!");
              }

              ncarray::NCArrayView raw_arr = det.get_data(step,
                                                          alg.c_str(),
                                                          field.c_str());

              if (!cpp_det->calib_buffer) {
                // Will reuse a single buffer for better memory management.
                cpp_det->calib_buffer = std::make_shared<ncarray::NCArray>(raw_arr.ndim(),
                                                                           raw_arr.shape(),
                                                                           ncarray::DType::float32);
              }

              // NOTE: Due to underlying template machinery, we must explicitly
              //       convert to view here. Normally, the NCArray auto converts
              //       but xalgospp::Algorithm has complex templating that would
              //       interfere, and result in a misinterpretation.
              //       Not even an `auto` to extract the reference will work...
              ncarray::NCArrayView calib_buffer { (*(cpp_det->calib_buffer)).view() };
              cpp_det->calibrator->process(raw_arr, calib_buffer);

              return *(cpp_det->calib_buffer);
            };

            auto py_calib_data = py::cpp_function(calibrate_data, py::is_method(py_alg));
            py::setattr(py_alg, "calib", MethodType(py_calib_data, py_alg));
          }
        }

        // For each "algorithm", attach it to the detector as an object with data
        // fetching methods for each of its associated fields
        py::setattr(py_det, alg.c_str(), py_alg);
      }
    };

    std::visit(det_visitor, detv);

    return py_det;
  }
} // namespace pysbio
