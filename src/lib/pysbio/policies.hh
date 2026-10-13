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

#ifndef PYSBIO_POLICIES_HH
#define PYSBIO_POLICIES_HH

#include "pysbio/execution/pyexecution.hh"
#include "pysbio/formats/pyformat_traits.hh"

#include "sbio/core/broker.hh"
#include "sbio/core/broker_group.hh"
#include "sbio/core/datasource.hh"
#include "sbio/core/types.hh"
#include "sbio/execution/serial.hh"
#include "sbio/execution/threaded.hh"
#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi.hh"
#include "sbio/execution/mpi_threaded.hh"
#endif
#ifdef SBIO_HAS_CUDA
#include "sbio/execution/cuda.hh"
#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi_cuda.hh"
#endif
#endif
#include "sbio/io/posix.hh"
#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif
#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2_traits.hh"
#endif

#ifdef SBIO_HAS_MPI
#include <mpi.h>
#endif
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace py = pybind11;

namespace pysbio {
  /**
   * The set of Execution policies and data formats with which bindings are built.
   *
   * @note This applies to the type-erased variant kinds of bindings. The lower-level
   *       bindings are managed separately.
   *
   * @note Adding policies here makes them available to all the type-erased objects.
   */
  template <class... Ts>
  struct TypeList {};

  using ExecutionPolicies = TypeList<
    sbio::SerialExecution,
    sbio::ThreadedExecution
#ifdef SBIO_HAS_MPI
    , sbio::MPIExecution
    , sbio::MPIThreadedExecution
#endif
#ifdef SBIO_HAS_CUDA
    , sbio::CudaExecution
    , sbio::ThreadedCudaExecution
#ifdef SBIO_HAS_MPI
    , sbio::MPICudaExecution
    , sbio::MPIThreadedCudaExecution
#endif
#endif
  >;

  using DataFormats = TypeList<
#if defined(SBIO_HAS_XTC1) && defined(SBIO_HAS_XTC2)
    sbio::XTC1Traits, sbio::XTC2Traits
#elif defined(SBIO_HAS_XTC1)
    sbio::XTC1Traits
#elif defined(SBIO_HAS_XTC2)
    sbio::XTC2Traits
#endif
  >;

  /// --- Bound Types from (EPolicy, DataFormat) Pairs --- ///
  // The variants represent holders of all types bound
  // Only SyncPOSIXIO is in use for the IO at the moment
  // TODO: Update these when the other IO policies are bound.

  template <class EPolicy, class FTraits>
  using BrokerFor = sbio::StreamBroker<sbio::SyncPOSIXIO, EPolicy, FTraits>;

  template <class EPolicy, class FTraits>
  using BrokerPtrFor = BrokerFor<EPolicy, FTraits>**;

  template <class EPolicy, class FTraits>
  using BrokerGroupFor = sbio::BrokerGroup<BrokerFor<EPolicy, FTraits>, FTraits>;

  template <class EPolicy, class FTraits>
  using DataSourceFor = sbio::DataSource<sbio::SyncPOSIXIO, EPolicy, FTraits>;

  // Various variant helpers
  namespace impl {
    template <template <class, class> class T, class Formats, class Policies>
    struct VariantOver;

    template <template <class, class> class T, class... Fs, class... Ps>
    struct VariantOver<T, TypeList<Fs...>, TypeList<Ps...>> {
      template <class F>
      using ForFormat = TypeList<T<Ps, F>...>;

      template <class... Lists>
      struct Join;

      template <class... As>
      struct Join<TypeList<As...>> {
        using type = std::variant<As...>;
      };

      template <class... As, class... Bs, class... Rest>
      struct Join<TypeList<As...>, TypeList<Bs...>, Rest...> {
        using type = typename Join<TypeList<As..., Bs...>, Rest...>::type;
      };

      using type = typename Join<ForFormat<Fs>...>::type;
    };
  } // namespace impl

  /**
   * Represents std::variant<ObjType<EPolicy, DataFormat>> for all data formats
   * and execution policies.
   *
   * @note Only SyncPOSIXIO is being handled for the IO at the moment.
   */
  template <template <class, class> class T>
  using VariantOver = typename impl::VariantOver<T, DataFormats, ExecutionPolicies>::type;

  /// --- Convert Python enums to Types --- ///

  /**
   * Call a function with the selected Execution policy.
   *
   * @throws std::runtime_error If this build of sbio does not support the policy.
   */
  template <class Fn>
  decltype(auto) with_policy(ExecutionPolicy epolicy, Fn&& fn) {
    switch (epolicy) {
    case ExecutionPolicy::Serial: {
      return fn(std::type_identity<sbio::SerialExecution> {});
    }
    case ExecutionPolicy::Threaded: {
      return fn(std::type_identity<sbio::ThreadedExecution> {});
    }
#ifdef SBIO_HAS_MPI
    case ExecutionPolicy::MPI: {
      return fn(std::type_identity<sbio::MPIExecution> {});
    }
    case ExecutionPolicy::MPIThreaded: {
      return fn(std::type_identity<sbio::MPIThreadedExecution> {});
    }
#endif
#ifdef SBIO_HAS_CUDA
    case ExecutionPolicy::Cuda: {
      return fn(std::type_identity<sbio::CudaExecution> {});
    }
    case ExecutionPolicy::ThreadedCuda: {
      return fn(std::type_identity<sbio::ThreadedCudaExecution> {});
    }
#ifdef SBIO_HAS_MPI
    case ExecutionPolicy::MPICuda: {
      return fn(std::type_identity<sbio::MPICudaExecution> {});
    }
    case ExecutionPolicy::MPIThreadedCuda: {
      return fn(std::type_identity<sbio::MPIThreadedCudaExecution> {});
    }
#endif
#endif
    default: {
      throw std::runtime_error("The requested Execution policy (" +
                               std::to_string(static_cast<int>(epolicy)) +
                               ") is not supported by this build of sbio!");
    }
    }
  }

  /**
   * Call a function with the selected data format.
   *
   * @throws std::runtime_error If this build of sbio does not support the format.
   */
  template <class Fn>
  decltype(auto) with_format(FTraits data_fmt, Fn&& fn) {
    switch (data_fmt) {
#ifdef SBIO_HAS_XTC1
    case FTraits::XTC1: {
      return fn(std::type_identity<sbio::XTC1Traits> {});
    }
#endif
#ifdef SBIO_HAS_XTC2
    case FTraits::XTC2: {
      return fn(std::type_identity<sbio::XTC2Traits> {});
    }
#endif
    default: {
      throw std::runtime_error("The requested data format (" +
                               std::to_string(static_cast<int>(data_fmt)) +
                               ") is not supported by this build of sbio!");
    }
    }
  }

  /**
   * Convert a Python dictionary into an Execution policy configuration.
   *
   * @note All fields the configuration has may be set by name.
   *
   * @note Composed policies will have the fields of both constituent Managers.
   *
   * @note Unknown keys are errors.
   */
  template <class EPolicy>
  typename EPolicy::Config parse_config(const py::dict& d) {
    typename EPolicy::Config cfg {};
    std::size_t used { 0 };

    auto set = [&](const char* key, auto& field) {
      if (d.contains(key)) {
        field = d[key].template cast<std::remove_reference_t<decltype(field)>>();
        ++used;
      }
    };

    if constexpr (requires { cfg.num_threads; }) {
      set("num_threads", cfg.num_threads);
    }
    if constexpr (requires { cfg.cpu_affinities; }) {
      set("cpu_affinities", cfg.cpu_affinities);
    }
    if constexpr (requires { cfg.active_ranks; }) {
      set("active_ranks", cfg.active_ranks);
    }
    if constexpr (requires { cfg.main_rank; }) {
      set("main_rank", cfg.main_rank);
    }
    if constexpr (requires { cfg.main_rank_loops; }) {
      set("main_rank_loops", cfg.main_rank_loops);
    }
    if constexpr (requires { cfg.device; }) {
      set("device", cfg.device);
    }
    if constexpr (requires { cfg.devices; }) {
      set("devices", cfg.devices);
    }

    if (used != d.size()) {
      std::string keys;
      for (const auto& item : d) {
        keys += (keys.empty() ? "" : ", ") + py::str(item.first).cast<std::string>();
      }

      throw std::runtime_error("Unknown option(s) in the Execution policy configuration "
                               "(given: " + keys + ")");
    }

    return cfg;
  }

  /**
   * Setup MPI for policies that require it.
   *
   * @note If this was done by the user, the function does nothing.
   *
   * @note Some policies require MPI initialization with certain thread level support.
   *       If initialized without that support, those policies cannot be used.
   */
  template <class EPolicy>
  void ensure_parallel_environment() {
#ifdef SBIO_HAS_MPI
    constexpr bool uses_mpi {
      EPolicy::ParallelSupport[static_cast<std::size_t>(sbio::ParallelizationMethods::MPI)]
    };
    constexpr bool uses_threads {
      EPolicy::ParallelSupport[static_cast<std::size_t>(sbio::ParallelizationMethods::THREADS)]
    };

    if constexpr (uses_mpi) {
      int initialized { 0 };
      MPI_Initialized(&initialized);

      if (!initialized) {
        int argc { 0 };
        char** argv { nullptr };

        if constexpr (uses_threads) {
          int provided { 0 };
          MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
          if (provided < MPI_THREAD_MULTIPLE) {
            // Need to check if you get the thread_multiple or not.
            std::cout << "Was not able to provide full MPI thread support." << std::endl
                      << "- Support was provied at level: " << provided << std::endl;
          }
        } else {
          MPI_Init(&argc, &argv);
        }
      }
    }
#endif
  }
} // namespace pysbio

#endif // PYSBIO_POLICIES_HH
