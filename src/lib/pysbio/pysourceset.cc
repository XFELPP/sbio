/*
 * sbio - Stream Broker IO
 *
 * Copyright (C) 2025-2026 Gabriel Dorlhiac
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pysbio/pysourceset.hh"

#include "pysbio/pybroker_group.hh"
#include "pysbio/pydatasource.hh"
#include "pysbio/pyiteration.hh"

#include <pybind11/pybind11.h>

#include <cstddef>
#include <memory>
#include <variant>

namespace py = pybind11;

namespace pysbio {

  std::size_t PySourceSet::next() {
    auto next_evt = [](auto& set) -> std::size_t {
      py::gil_scoped_release release;
      return set.next();
    };

    return std::visit(next_evt, m_set);
  }

  std::unique_ptr<StepIterator> PySourceSet::steps(bool batched,
                                                   std::size_t batch_size) {
    auto make = [&](auto& set) {
      return std::make_unique<StepIterator>(set, batched, batch_size);
    };

    return std::visit(make, m_set);
  }

  std::unique_ptr<ZipIterator> PySourceSet::zip(const std::vector<const BrokerGroupWrapper*>& groups,
                                                bool batched) {
    return make_zip(*this, groups, batched);
  }
} // namespace pysbio
