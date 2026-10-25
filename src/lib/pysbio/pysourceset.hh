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

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <numeric>
#include <type_traits>
#include <variant>

namespace py = pybind11;

namespace pysbio {
  namespace {
#if defined(SBIO_HAS_HDF5)
    using CXISerial = sbio::DataSource<
      sbio::SyncPOSIXIO,
      sbio::SerialExecution,
      sbio::CXITraits
    >;
    using CXIParallel = sbio::DataSource<
      sbio::SyncPOSIXIO,
      sbio::MPIThreadedExecution,
      sbio::CXITraits
    >;
#endif
#if defined(SBIO_HAS_XTC1)
    using XTC1Serial = sbio::DataSource<
      sbio::SyncPOSIXIO,
      sbio::SerialExecution,
      sbio::XTC1Traits
    >;
    using XTC1Parallel = sbio::DataSource<
      sbio::SyncPOSIXIO,
      sbio::MPIThreadedExecution,
      sbio::XTC1Traits
    >;
#endif
#if defined(SBIO_HAS_XTC2)
    using XTC2Serial = sbio::DataSource<
      sbio::SyncPOSIXIO,
      sbio::SerialExecution,
      sbio::XTC2Traits
    >;
    using XTC2Parallel = sbio::DataSource<
      sbio::SyncPOSIXIO,
      sbio::MPIThreadedExecution,
      sbio::XTC2Traits
    >;
#endif
  } // anonymous namespace

  // NOTE: The order here MUST match the order of the DataSource variant
  //       since .index() is used to sort before creating the set
  //       (To reduce the number of instnatiations - order doesn't matter)
  // With all 3 data formats, the order is currently: CXI, XTC1, XTC2
  // --> See pysbio/policies.hh for the `DataFormats` order.
  using SourceSetV = std::variant<
#if defined(SBIO_HAS_HDF5) && defined(SBIO_HAS_XTC1) && defined(SBIO_HAS_XTC2)

    // Serial same format
    sbio::SourceSet<CXISerial, CXISerial>,
    sbio::SourceSet<XTC1Serial, XTC1Serial>,
    sbio::SourceSet<XTC2Serial, XTC2Serial>,

    // Serial cross-format
    sbio::SourceSet<CXISerial, XTC1Serial>,
    sbio::SourceSet<CXISerial, XTC2Serial>,
    sbio::SourceSet<XTC1Serial, XTC2Serial>,

    // Parallel same-format
    sbio::SourceSet<CXIParallel, CXIParallel>,
    sbio::SourceSet<XTC1Parallel, XTC1Parallel>,
    sbio::SourceSet<XTC2Parallel, XTC2Parallel>,

    // Parallel cross-format
    sbio::SourceSet<CXIParallel, XTC1Parallel>,
    sbio::SourceSet<CXIParallel, XTC2Parallel>,
    sbio::SourceSet<XTC1Parallel, XTC2Parallel>

#elif defined(SBIO_HAS_HDF5) && defined(SBIO_HAS_XTC1)

    // Serial same format
    sbio::SourceSet<CXISerial, CXISerial>,
    sbio::SourceSet<XTC1Serial, XTC1Serial>,

    // Serial cross-format
    sbio::SourceSet<CXISerial, XTC1Serial>,

    // Parallel same format
    sbio::SourceSet<CXIParallel, CXIParallel>,
    sbio::SourceSet<XTC1Parallel, XTC1Parallel>,

    // Parallel cross-format
    sbio::SourceSet<CXIParallel, XTC1Parallel>

#elif defined(SBIO_HAS_HDF5) && defined(SBIO_HAS_XTC2)

    // Serial same format
    sbio::SourceSet<CXISerial, CXISerial>,
    sbio::SourceSet<XTC2Serial, XTC2Serial>,

    // Serial cross-format
    sbio::SourceSet<CXISerial, XTC2Serial>,

    // Parallel same format
    sbio::SourceSet<CXIParallel, CXIParallel>,
    sbio::SourceSet<XTC2Parallel, XTC2Parallel>,

    // Parallel cross-format
    sbio::SourceSet<CXIParallel, XTC2Parallel>

#elif defined(SBIO_HAS_XTC1) && defined(SBIO_HAS_XTC2)

    // Serial same format
    sbio::SourceSet<XTC1Serial, XTC1Serial>,
    sbio::SourceSet<XTC2Serial, XTC2Serial>,

    // Serial cross-format
    sbio::SourceSet<XTC1Serial, XTC2Serial>,

    // Parallel same format
    sbio::SourceSet<XTC1Parallel, XTC1Parallel>,
    sbio::SourceSet<XTC2Parallel, XTC2Parallel>,

    // Parallel cross-format
    sbio::SourceSet<XTC1Parallel, XTC2Parallel>

#elif defined(SBIO_HAS_HDF5)

    // Serial same format
    sbio::SourceSet<CXISerial, CXISerial>,

    // Parallel cross-format
    sbio::SourceSet<CXIParallel, CXIParallel>

#elif defined(SBIO_HAS_XTC1)

    // Serial same format
    sbio::SourceSet<XTC1Serial, XTC1Serial>,

    // Parallel cross-format
    sbio::SourceSet<XTC1Parallel, XTC1Parallel>

#elif defined(SBIO_HAS_XTC2)

    // Serial same format
    sbio::SourceSet<XTC2Serial, XTC2Serial>,

    // Parallel cross-format
    sbio::SourceSet<XTC2Parallel, XTC2Parallel>

#endif
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
    using SrcV = std::common_type_t<PySources...>;

    constexpr std::size_t NumSrcs { sizeof...(PySources) };

    std::array<std::size_t, NumSrcs> order {};
    std::iota(order.begin(), order.end(), 0);

    std::array<std::size_t, NumSrcs> var_indices { py_sources.index()... };

    std::sort(order.begin(), order.end(), [&](auto a, auto b) {
      return var_indices[a] < var_indices[b];
    });

    const std::array<SrcV*, NumSrcs> py_srcs_ptr { &py_sources... };

    auto canonicalize = [&] <std::size_t... Idxs> (std::index_sequence<Idxs...>) {
      auto build_inplace = [](auto&... srcs) -> std::unique_ptr<PySourceSet> {
        using Set = sbio::SourceSet<std::remove_cvref_t<decltype(srcs)>...>;

        if constexpr (in_variant<Set, SourceSetV>::value) {
          return std::make_unique<PySourceSet>(std::in_place_type<Set>, srcs...);
        } else {
          throw std::invalid_argument("Unsupported SourceSet combination!");
        }
      };

      return std::visit(build_inplace, *py_srcs_ptr[order[Idxs]]...);
    };

    return canonicalize(std::make_index_sequence<NumSrcs> {});
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
