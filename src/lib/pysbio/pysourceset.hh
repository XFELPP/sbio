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

#ifndef PYSBIO_PYSOURCESET_HH
#define PYSBIO_PYSOURCESET_HH

#include "pysbio/pydatasource.hh"

#include "pysbio/pyiteration.hh"

#include "sbio/core/datasource.hh"
#include "sbio/core/iteration.hh"

#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi_threaded.hh"
#endif

#ifdef SBIO_HAS_HD5
#include "sbio/formats/hdf5/cxi_spec.hh"
#endif

#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif

#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2_traits.hh"
#endif

#include <pybind11/pybind11.h>

#include <cstddef>
#include <memory>
#include <type_traits>
#include <variant>

namespace py = pybind11;

namespace pysbio {
  using SourceSetV = std::variant<
    sbio::SourceSet<
      sbio::DataSource<
        sbio::SyncPOSIXIO,
        sbio::MPIThreadedExecution,
        sbio::XTC1Traits
      >,
      sbio::DataSource<
        sbio::SyncPOSIXIO,
        sbio::MPIThreadedExecution,
        sbio::XTC2Traits
      >
    >,
    sbio::SourceSet<
      sbio::DataSource<
        sbio::SyncPOSIXIO,
        sbio::MPIThreadedExecution,
        sbio::XTC1Traits
      >,
      sbio::DataSource<
        sbio::SyncPOSIXIO,
        sbio::MPIThreadedExecution,
        sbio::CXITraits
      >
    >,
    sbio::SourceSet<
      sbio::DataSource<
        sbio::SyncPOSIXIO,
        sbio::MPIThreadedExecution,
        sbio::XTC2Traits
      >,
      sbio::DataSource<
        sbio::SyncPOSIXIO,
        sbio::MPIThreadedExecution,
        sbio::CXITraits
      >
    >
  >;
  template <class T, class Var>
  struct in_variant : std::false_type {};

  template <class T, class... Ts>
  struct in_variant<T, std::variant<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
  {};

  class PySourceSet;

  template <class... PySources>
  std::unique_ptr<PySourceSet> make_source_set(PySources&... py_sources) {
    auto build_inplace = [](auto&... srcs) -> std::unique_ptr<PySourceSet> {
      using Set = sbio::SourceSet<std::remove_cvref_t<decltype(srcs)>...>;

      if constexpr (in_variant<Set, SourceSetV>::value) {
        return std::make_unique<PySourceSet>(std::in_place_type<Set>, srcs...);
      } else {
        throw std::invalid_argument("Unsupported SourceSet combination!");
      }
    };

    return std::visit(build_inplace, py_sources...);
  }

  class PySourceSet {
  public:
    template <class Set, class... Srcs>
    PySourceSet(std::in_place_type_t<Set> set, Srcs&... srcs)
      : m_set(set, srcs...)
    {}

    std::size_t next();

    std::unique_ptr<StepIterator> steps(bool batched,
                                        std::size_t batch_size);

    std::unique_ptr<ZipIterator> zip(const std::vector<const BrokerGroupWrapper*>& groups,
                                     bool batched);

    SourceSetV& set() { return m_set; }
    const SourceSetV& set() const { return m_set; }

  private:
    SourceSetV m_set;
  };
} // namespace pysbio

#endif // PYSBIO_PYSOURCESET_HH
