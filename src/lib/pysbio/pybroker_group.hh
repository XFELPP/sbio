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

#include "pysbio/policies.hh"

#include <ncarray/soarrays.hh>
#ifdef SBIO_HAS_CUDA
#include <ncarray/sodevarrays.cuh>
#endif
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace py = pybind11;

namespace pysbio {

  /**
   * A BrokerGroup for any bound policy/format combo (see policies.hh)
   */
  using BrokerGroupV = VariantOver<BrokerGroupFor>;

  struct BrokerGroupWrapper {
    BrokerGroupWrapper(BrokerGroupV g)
      : grp(g)
    {
      init_group_fields();
    }

    inline const std::vector<std::vector<std::string>>& group_fields() const {
      return m_fields;
    }

    const std::string& get_detector_type() const { return detector_type; }
    const std::string& get_serial_number() const { return serial_number; }

    template <typename... Args>
    py::object get_data(std::size_t step, Args&&... args) {
      auto getter = [&](auto& grp) -> py::object {
        using BGType = std::remove_cvref_t<decltype(grp)>;
        using MemTag = typename BGType::MemTag;

        auto inner = [&]() -> ncarray::SOViewFor<MemTag> {
          py::gil_scoped_release release;
          using Schema = typename BGType::DataFormat::RequestSchema;

          if constexpr (sizeof...(Args) == Schema::count) {
            return grp.get_data(step, args...);
          } else {
            throw std::runtime_error("Incorrect arguments for BrokerGroup RequestSchema!");
          }
        };

        auto arr { inner() };

        return py::cast(std::move(arr));
      };

      return std::visit(getter, this->grp);
    }

    template <typename... Args>
    py::object get_multi_data(std::initializer_list<std::size_t> steps,
                              Args&&... args) {
      auto getter = [&](auto& grp) -> py::object {
        using BGType = std::remove_cvref_t<decltype(grp)>;
        using MemTag = typename BGType::MemTag;

        auto inner = [&]() -> ncarray::SOViewFor<MemTag> {
          py::gil_scoped_release release;
          using Schema = typename BGType::DataFormat::RequestSchema;

          if constexpr (sizeof...(Args) == Schema::count) {
            return grp.get_multi_data(steps, args...);
          } else {
            throw std::runtime_error("Incorrect arguments for BrokerGroup RequestSchema!");
          }
        };

        auto arr { inner() };

        return py::cast(std::move(arr));
      };

      return std::visit(getter, this->grp);
    }

    template <typename... Args>
    py::object resolve_data(std::size_t step, Args&&... args) {
      auto getter = [&](auto& grp) -> py::object {
        using BGType = std::remove_cvref_t<decltype(grp)>;
        using MemTag = typename BGType::MemTag;

        auto inner = [&]() -> ncarray::SOViewFor<MemTag> {
          py::gil_scoped_release release;
          using Schema = typename BGType::DataFormat::RequestSchema;

          if constexpr (sizeof...(Args) == Schema::count) {
            return grp.resolve_data(step, args...);
          } else {
            throw std::runtime_error("Incorrect arguments for BrokerGroup RequestSchema!");
          }
        };

        auto arr { inner() };

        return py::cast(std::move(arr));
      };

      return std::visit(getter, this->grp);
    }

    template <typename... Args>
    py::object resolve_multi_data(std::size_t first, std::size_t last, Args&&... args) {
      auto getter = [&](auto& grp) -> py::object {
        using BGType = std::remove_cvref_t<decltype(grp)>;
        using MemTag = typename BGType::MemTag;

        auto inner = [&]() -> ncarray::SOViewFor<MemTag> {
          py::gil_scoped_release release;
          using Schema = typename BGType::DataFormat::RequestSchema;

          if constexpr (sizeof...(Args) == Schema::count) {
            return grp.resolve_multi_data(first, last, args...);
          } else {
            throw std::runtime_error("Incorrect arguments for BrokerGroup RequestSchema!");
          }
        };

        auto arr { inner() };

        return py::cast(std::move(arr));
      };

      return std::visit(getter, this->grp);
    }

    BrokerGroupV grp;
    std::string detector_type;
    std::string serial_number;

  private:
    void init_group_fields();

    std::vector<std::vector<std::string>> m_fields;
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
