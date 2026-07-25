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

#include "pysbio/formats/pyformat_traits.hh"

#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi.hh"
#include "sbio/execution/mpi_threaded.hh"
#endif
#include "sbio/execution/serial.hh"
#include "sbio/execution/threaded.hh"
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

namespace py = pybind11;

namespace {
  template <typename Exec, typename PyExec>
  void bind_epolicy_functions(PyExec& exec_cls) {
    exec_cls.def(py::init<>())
      .def_static("configure", [](const typename Exec::Config& cfg) {
        Exec::configure(cfg);
      },
        py::arg("config"))
      .def_static("should_index", &Exec::should_index)
      .def_static("should_process", [](std::size_t& step_idx,
                                       pysbio::FTraits ftraits) {
        if (ftraits == pysbio::FTraits::XTC1) {
#ifdef SBIO_HAS_XTC1
          return Exec::template should_process<sbio::XTC1Traits>(step_idx);
#else
          throw std::runtime_error("XTC1 requested, but sbio was built without XTC1 support!");
#endif
        } else if (ftraits == pysbio::FTraits::XTC2) {
#ifdef SBIO_HAS_XTC2
          return Exec::template should_process<sbio::XTC2Traits>(step_idx);
#else
          throw std::runtime_error("XTC2 requested, but sbio was built without XTC2 support!");
#endif
        } else {
          throw std::runtime_error("Unrecognized data format selector provided!");
        }
      })
      .def_static("next", [](std::size_t& capacity,
                             py::function py_trigger,
                             pysbio::FTraits ftraits) {
        auto trigger = [&]() -> bool {
          py::gil_scoped_acquire acquire;

          return py_trigger().cast<bool>();
        };

        if (ftraits == pysbio::FTraits::XTC1) {
#ifdef SBIO_HAS_XTC1
          return Exec::template next<sbio::XTC1Traits>(capacity, trigger);
#else
          throw std::runtime_error("XTC1 requested, but sbio was built without XTC1 support!");
#endif
        } else if (ftraits == pysbio::FTraits::XTC2) {
#ifdef SBIO_HAS_XTC2
          return Exec::template next<sbio::XTC2Traits>(capacity, trigger);
#else
          throw std::runtime_error("XTC2 requested, but sbio was built without XTC2 support!");
#endif
        } else {
          throw std::runtime_error("Unrecognized data format selector provided!");
        }
      },
                  py::arg("max_capacity"),
                  py::arg("reindex_trigger"),
                  py::arg("ftraits"))
      .def_static("get_data", [](std::size_t step_idx,
                                 py::function py_fetcher,
                                 std::size_t num_fetches,
                                 py::function py_getter,
                                 std::size_t num_accesses,
                                 pysbio::FTraits ftraits) {
        auto fetcher = [&](std::size_t i) -> sbio::IOStatus {
          py::gil_scoped_acquire acquire;

          return py_fetcher(i).cast<sbio::IOStatus>();
        };

        auto getter = [&](std::size_t i) -> void {
          py::gil_scoped_acquire acquire;

          py_getter(i);
        };

        if (ftraits == pysbio::FTraits::XTC1) {
#ifdef SBIO_HAS_XTC1
          return Exec::template get_data<sbio::XTC1Traits>(step_idx,
                                                           std::forward<decltype(fetcher)>(fetcher),
                                                           num_fetches,
                                                           std::forward<decltype(getter)>(getter),
                                                           num_accesses);
#else
          throw std::runtime_error("XTC1 requested, but sbio was built without XTC1 support!");
#endif
        } else if (ftraits == pysbio::FTraits::XTC2) {
#ifdef SBIO_HAS_XTC2
          return Exec::template get_data<sbio::XTC2Traits>(step_idx,
                                                           std::forward<decltype(fetcher)>(fetcher),
                                                           num_fetches,
                                                           std::forward<decltype(getter)>(getter),
                                                           num_accesses);
#else
          throw std::runtime_error("XTC2 requested, but sbio was built without XTC2 support!");
#endif
        } else {
          throw std::runtime_error("Unrecognized data format selector provided!");
        }
      },
                  py::arg("step_idx"),
                  py::arg("unit_fetcher"),
                  py::arg("num_fetches"),
                  py::arg("unit_getter"),
                  py::arg("num_accesses"),
                  py::arg("ftraits"))
      .def_static("get_data_steps", [](const std::initializer_list<std::size_t>& steps,
                                       py::function py_fetcher,
                                       std::size_t num_fetches,
                                       py::function py_getter,
                                       std::size_t num_accesses,
                                       pysbio::FTraits ftraits) {
        auto fetcher = [&](std::size_t i) -> sbio::IOStatus {
          py::gil_scoped_acquire acquire;

          return py_fetcher(i).cast<sbio::IOStatus>();
        };

        auto getter = [&](std::size_t i, std::size_t cnt) -> void {
          py::gil_scoped_acquire acquire;

          py_getter(i, cnt);
        };

        if (ftraits == pysbio::FTraits::XTC1) {
#ifdef SBIO_HAS_XTC1
          return Exec::template get_data_steps<sbio::XTC1Traits>(steps,
                                                                 std::forward<decltype(fetcher)>(fetcher),
                                                                 num_fetches,
                                                                 std::forward<decltype(getter)>(getter),
                                                                 num_accesses);
#else
          throw std::runtime_error("XTC1 requested, but sbio was built without XTC1 support!");
#endif
        } else if (ftraits == pysbio::FTraits::XTC2) {
#ifdef SBIO_HAS_XTC2
          return Exec::template get_data_steps<sbio::XTC2Traits>(steps,
                                                                 std::forward<decltype(fetcher)>(fetcher),
                                                                 num_fetches,
                                                                 std::forward<decltype(getter)>(getter),
                                                                 num_accesses);
#else
          throw std::runtime_error("XTC2 requested, but sbio was built without XTC2 support!");
#endif
        } else {
          throw std::runtime_error("Unrecognized data format selector provided!");
        }
      },
                  py::arg("steps"),
                  py::arg("unit_fetcher"),
                  py::arg("num_fetches"),
                  py::arg("unit_getter"),
                  py::arg("num_accesses"),
                  py::arg("ftraits"));
  }
}

PYBIND11_MODULE(pyexecution, exec_module, py::mod_gil_not_used()) {
  exec_module.doc() = "sbio Python bindings for Execution policies.";

#ifdef SBIO_HAS_MPI
  using MPICfg = sbio::MPIExecution::Config;
  using MPIExec = sbio::MPIExecution;
  using MPIThreadedCfg = sbio::MPIThreadedExecution::Config;
  using MPIThreadedExec = sbio::MPIThreadedExecution;

  py::classh<MPICfg>(exec_module, "MPIExecutionConfig")
    .def("communicator", [](MPICfg& self, int comm_f) {
      MPI_Comm comm = MPI_Comm_f2c(comm_f);
      self.communicator = comm;
    })
    .def_readwrite("active_ranks", &MPICfg::active_ranks)
    .def_readwrite("main_rank", &MPICfg::main_rank)
    .def_readwrite("main_rank_loops", &MPICfg::main_rank_loops);

  auto mpiexec_cls = py::classh<MPIExec>(exec_module, "MPIExecution");
  bind_epolicy_functions<MPIExec>(mpiexec_cls);

  py::classh<MPIThreadedCfg>(exec_module, "MPIThreadedExecutionConfig")
    .def("communicator", [](MPIThreadedCfg& self, int comm_f) {
      MPI_Comm comm = MPI_Comm_f2c(comm_f);
      self.communicator = comm;
    })
    .def_readwrite("active_ranks", &MPIThreadedCfg::active_ranks)
    .def_readwrite("main_rank", &MPIThreadedCfg::main_rank)
    .def_readwrite("main_rank_loops", &MPIThreadedCfg::main_rank_loops)
    .def_readwrite("num_threads", &MPIThreadedCfg::num_threads)
    .def_readwrite("cpu_affinities", &MPIThreadedCfg::cpu_affinities);

  auto mpithreadedexec_cls = py::classh<MPIThreadedExec>(exec_module,
                                                         "MPIThreadedExecution");
  bind_epolicy_functions<MPIThreadedExec>(mpithreadedexec_cls);
#endif // SBIO_HAS_MPI

  using SerialCfg = sbio::SerialExecution::Config;
  using SerialExec = sbio::SerialExecution;
  using ThreadedCfg = sbio::ThreadedExecution::Config;
  using ThreadedExec = sbio::ThreadedExecution;

  py::classh<SerialCfg>(exec_module, "SerialExecutionConfig");

  auto serialexec_cls = py::classh<SerialExec>(exec_module, "SerialExecution");
  bind_epolicy_functions<SerialExec>(serialexec_cls);

  py::classh<ThreadedCfg>(exec_module, "ThreadedExecutionConfig")
    .def_readwrite("num_threads", &ThreadedCfg::num_threads)
    .def_readwrite("cpu_affinities", &ThreadedCfg::cpu_affinities);

  auto threadedexec_cls = py::classh<ThreadedExec>(exec_module, "ThreadedExecution");
  bind_epolicy_functions<ThreadedExec>(threadedexec_cls);
} // exec_module
