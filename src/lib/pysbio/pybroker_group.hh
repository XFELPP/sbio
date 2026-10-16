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

#include <ncarray/ncarrays.hh>
#include <ncarray/soarrays.hh>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <string>
#include <variant>

namespace py = pybind11;

namespace pysbio {

  /**
   * A BrokerGroup for any bound policy/format combo (see policies.hh)
   */
  using BrokerGroupV = VariantOver<BrokerGroupFor>;

  struct BrokerGroupWrapper {
    BrokerGroupWrapper(BrokerGroupV g)
      : grp(g)
    {}

    const std::string& get_detector_type() const { return detector_type; }
    const std::string& get_serial_number() const { return serial_number; }

    py::object get_data(std::size_t step, const char* alg, const char* field);

    py::object get_multi_data(std::initializer_list<std::size_t> steps,
                              const char* alg,
                              const char* field);

    py::object resolve_data(std::size_t step, const char* alg, const char* field);

    py::object resolve_multi_data(std::size_t first,
                                  std::size_t last,
                                  const char* alg,
                                  const char* field);

    BrokerGroupV grp;
    std::string detector_type;
    std::string serial_number;
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
