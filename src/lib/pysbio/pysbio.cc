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

#include "pysbio/execution/pyexecution.hh"
#include "pysbio/formats/pyformat_traits.hh"
#include "pysbio/io/pyio.hh"
#include "pysbio/pybroker_group.hh"
#include "pysbio/pydatasource.hh"

#include "sbio/core/types.hh"

#include <ncarray/ncarrays.hh>
#ifdef SBIO_HAS_MPI
#include <mpi.h>
#endif
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>

namespace py = pybind11;

using namespace pysbio;

namespace {
  static constexpr const char* xtc_ds_doc = R"a(Create a datasource for reading XTC1 or XTC2 data.

Args:
    exp (str): The LCLS experiment.

    run (int): The run number for the experiment.

    events_per_read (int): The number of event offsets to index on each attempt.

    max_dgram_size (int): The maximum size of a Datagram to accept.

    data_fmt (pysbio.FTraits): The data format to read (XTC1, XTC2, etc.)

    epolicy (pysbio.ExecutionPolicy): The Execution policy to use (Serial, MPI, Threaded, etc.)

    exec_cfg (dict): Configuration for the Execution policy (fields depend on selection).

    io_policy (pysbio.IOPolicy): The IO policy to use (POSIX, cuFile, etc).)a";

  static constexpr const char* file_ds_doc = R"a(Create a datasource for reading XT1/XTC2, or CXI data.

Args:
    path (str): The path to the XTC1/XTC2, or CXI file.

    index_batch_size (int): The number of step offsets to index in one go.

    max_buffer_size (int): The maximum size for a single data buffer.

    data_fmt (pysbio.FTraits): The data format to read (XTC1, XTC2, etc.)

    epolicy (pysbio.ExecutionPolicy): The Execution policy to use (Serial, MPI, Threaded, etc.)

    exec_cfg (dict): Configuration for the Execution policy (fields depend on selection).

    io_policy (pysbio.IOPolicy): The IO policy to use (POSIX, cuFile, etc).)a";
} // anonymous namespace

PYBIND11_MODULE(_pysbio, pysbio_module, py::mod_gil_not_used()) {
  // NOTE: This must be in PYTHONPATH (or generally accessible!) or init will fail
  py::module_::import("ncarray");
  pysbio_module.doc() = "sbio Python bindings.";

  py::module_::import("sbio.formats.pyformat_traits");
#ifdef SBIO_HAS_XTC1
  py::module_::import("sbio.formats.pyxtc1");
#endif
#ifdef SBIO_HAS_XTC2
  py::module_::import("sbio.formats.pyxtc2");
#endif

  //py::module_::import("sbio.io");
  py::module_::import("sbio.execution");
  py::module_::import("sbio.brokers");
  py::module_::import("sbio.broker_groups");
  py::module_::import("sbio.datasources");

  // TODO: Try to rework these wrappers to remove dynamic_attr
  //       Instead, create the *classes* dynamically and attach the methods
  py::classh<BrokerGroupWrapper>(pysbio_module, "BrokerGroupWrapper", py::dynamic_attr())
    .def_property_readonly("group_type", &BrokerGroupWrapper::get_detector_type)
    .def_property_readonly("group_fields", &BrokerGroupWrapper::group_fields)
    .def_property_readonly("serial_number", &BrokerGroupWrapper::get_serial_number)
    .def("get_data",
         [](BrokerGroupWrapper& self,
            std::size_t step,
            const char* alg,
            const char* field) {
           return self.get_data(step, alg, field);
         },
         py::arg("step"),
         py::arg("alg"),
         py::arg("field"))
    .def("get_data",
         [](BrokerGroupWrapper& self, std::size_t step, const char* field) {
           return self.get_data(step, field);
         },
         py::arg("step"),
         py::arg("field"))
    .def("get_multi_data",
         [](BrokerGroupWrapper& self,
            std::initializer_list<std::size_t> steps,
            const char* alg,
            const char* field) {
           return self.get_multi_data(steps, alg, field);
         },
         py::arg("steps"),
         py::arg("alg"),
         py::arg("field"))
    .def("get_multi_data",
         [](BrokerGroupWrapper& self,
            std::initializer_list<std::size_t> steps,
            const char* field) {
           return self.get_multi_data(steps, field);
         },
         py::arg("steps"),
         py::arg("field"))
    .def("resolve_data",
         [](BrokerGroupWrapper& self,
            std::size_t step,
            const char* alg,
            const char* field) {
           return self.resolve_data(step, alg, field);
         },
         py::arg("step"),
         py::arg("alg"),
         py::arg("field"))
    .def("resolve_data",
         [](BrokerGroupWrapper& self,
            std::size_t step,
            const char* field) {
           return self.resolve_data(step, field);
         },
         py::arg("step"),
         py::arg("field"))
    .def("resolve_multi_data",
         [](BrokerGroupWrapper& self,
            std::size_t first,
            std::size_t last,
            const char* alg,
            const char* field) {
           return self.resolve_multi_data(first, last, alg, field);
         },
         py::arg("first"),
         py::arg("last"),
         py::arg("alg"),
         py::arg("field"))
    .def("resolve_multi_data",
         [](BrokerGroupWrapper& self,
            std::size_t first,
            std::size_t last,
            const char* field) {
           return self.resolve_multi_data(first, last, field);
         },
         py::arg("first"),
         py::arg("last"),
         py::arg("field"));

  py::classh<AlgWrapper>(pysbio_module, "AlgWrapper", py::dynamic_attr());

  using Batch = sbio::StepBatch<std::size_t>;
  py::classh<Batch>(pysbio_module, "StepBatch", "Contiguous steps [first, last).")
    .def(py::init<>())
    .def(py::init([](std::size_t first, std::size_t last) { return Batch { first, last }; }),
         py::arg("first"),
         py::arg("last"))
    .def_readwrite("first", &Batch::first)
    .def_readwrite("last", &Batch::last)
    .def("count", &Batch::count)
    .def("__len__", &Batch::count)
    .def("__iter__", [](const Batch& self) {
      return py::module_::import("builtins").attr("range")(self.first, self.last).attr("__iter__")();
    })
    .def("__repr__", [](const Batch& self) {
      return "StepBatch(" + std::to_string(self.first) + ", " + std::to_string(self.last) + ")";
    });

  py::classh<StepIterator>(pysbio_module,
                           "StepIterator",
                           "Iterates the steps (or batches) of a DataSource. Leaving the "
                           "iteration (exhaustion, close(), a with block, or freeing the "
                           "iterator after a break) ends it for the calling unit.")
    .def("__iter__", [](StepIterator& self) -> StepIterator& { return self; })
    .def("__next__", &StepIterator::next)
    .def("close", &StepIterator::close)
    .def("__enter__", [](StepIterator& self) -> StepIterator& { return self; })
    .def("__exit__", [](StepIterator& self, py::args) { self.close(); });

  py::classh<ZipStep>(pysbio_module, "ZipStep", "A step of a zipped iteration.")
    .def_readonly("step", &ZipStep::step)
    .def_readonly("status", &ZipStep::status)
    .def("__int__", [](const ZipStep& self) { return self.step; })
    .def("__index__", [](const ZipStep& self) { return self.step; })
    .def("get",
         [](const ZipStep& self, BrokerGroupWrapper& group, const char* alg, const char* field) {
           if (self.status != sbio::IOStatus::Success) {
             throw std::runtime_error("The step's fetch failed (see `status`).");
           }
           return group.resolve_data(self.step, alg, field);
         },
         py::arg("group"),
         py::arg("alg"),
         py::arg("field"),
         "The group's data for this step (the group must be one of the zipped groups).");

  py::classh<ZipBatch>(pysbio_module, "ZipBatch", "A batch of a zipped iteration.")
    .def_readonly("first", &ZipBatch::first)
    .def_readonly("last", &ZipBatch::last)
    .def_readonly("status", &ZipBatch::status)
    .def("count", [](const ZipBatch& self) { return self.last - self.first; })
    .def("__len__", [](const ZipBatch& self) { return self.last - self.first; })
    .def("get",
         [](const ZipBatch& self, BrokerGroupWrapper& group, const char* alg, const char* field) {
           if (self.status != sbio::IOStatus::Success) {
             throw std::runtime_error("The batch's fetch failed (see `status`).");
           }
           return group.resolve_multi_data(self.first, self.last, alg, field);
         },
         py::arg("group"),
         py::arg("alg"),
         py::arg("field"),
         "The group's data for this batch (the group must be one of the zipped groups).");

  py::classh<ZipIterator>(pysbio_module,
                          "ZipIterator",
                          "Iterates the steps (or batches) of a DataSource with the fetches "
                          "for a set of groups zipped. Ends as a StepIterator does.")
    .def("__iter__", [](ZipIterator& self) -> ZipIterator& { return self; })
    .def("__next__", &ZipIterator::next)
    .def("close", &ZipIterator::close)
    .def_property_readonly("num_fetches", &ZipIterator::num_fetches)
    .def("__enter__", [](ZipIterator& self) -> ZipIterator& { return self; })
    .def("__exit__", [](ZipIterator& self, py::args) { self.close(); });

  py::classh<PyDataSource>(pysbio_module, "DataSource")
    .def(py::init<
           std::string,
           unsigned,
           int,
           int,
           pysbio::FTraits,
           pysbio::ExecutionPolicy,
           py::dict,
           pysbio::IOPolicy
         >(),
         py::arg("exp"),
         py::arg("run"),
         py::arg("events_per_read") = 43200,    // 6 minutes at 120 Hz
         py::arg("max_dgram_size") = 0x4000000, // ~67 MB
         py::arg("data_fmt") = pysbio::FTraits::XTC2,
         py::arg("epolicy") = pysbio::ExecutionPolicy::MPI,
         py::arg("exec_cfg") = py::dict(),
         py::arg("io_policy") = pysbio::IOPolicy::SyncPOSIX,
         xtc_ds_doc)
    .def(py::init<
           std::string,
           int,
           int,
           pysbio::FTraits,
           pysbio::ExecutionPolicy,
           py::dict,
           pysbio::IOPolicy
         >(),
         py::arg("path"),
         py::arg("index_batch_size") = 43200,    // 6 minutes at 120 Hz
         py::arg("max_buffer_size") = 0x4000000, // ~67 MB
         py::arg("data_fmt") = pysbio::FTraits::CXI,
         py::arg("epolicy") = pysbio::ExecutionPolicy::MPI,
         py::arg("exec_cfg") = py::dict(),
         py::arg("io_policy") = pysbio::IOPolicy::SyncPOSIX,
         file_ds_doc)
    .def_property_readonly("available_groups", &PyDataSource::available_groups)
    .def("group", [&](PyDataSource& self, const char* name) {
      return self.group(pysbio_module, name);
    },
      py::arg("group_name"))
    // Event generation and iteration
    // NOTE: Iterators are generated at run-time due to type-erasure and variant issues
    // making binding the iterator itself rather annoying.
    .def("next", &PyDataSource::next)
    .def("__iter__", [](PyDataSource& self) {
      auto iterator_maker = [](auto& ds) {
        return py::make_iterator(ds.begin(), ds.end());
      };

      return std::visit(iterator_maker, self.ds());
    },
      py::keep_alive<0, 1>())
    .def("steps", [](PyDataSource& self) {
      return self.steps(false, 0);
    },
      py::keep_alive<0, 1>())
    .def("batches", [](PyDataSource& self, std::size_t batch_size) {
      return self.steps(true, batch_size);
    },
      py::arg("batch_size") = 0,
      py::keep_alive<0, 1>(),
      "Iterate contiguous batches of steps (at most batch_size, or max_batch_size).")
    .def("zip", [](PyDataSource& self, py::args groups) {
      return self.zip(groups.cast<std::vector<py::object>>(), false);
    },
      py::keep_alive<0, 1>(),
      "Iterate the steps, fetching once for all of the given groups (detectors).")
    .def("zip_batches", [](PyDataSource& self, py::args groups) {
      return self.zip(groups.cast<std::vector<py::object>>(), true);
     },
      py::keep_alive<0, 1>(),
      "Iterate batches of steps, fetching once for all of the given groups.");

  py::classh<ContextWrapper>(pysbio_module,
                             "Context",
                             "Checks that an Execution policy can be used (build and "
                             "environment), configures it once, and creates DataSources.")
    .def(py::init<pysbio::ExecutionPolicy, py::dict, pysbio::IOPolicy>(),
         py::arg("epolicy"),
         py::arg("exec_cfg") = py::dict(),
         py::arg("io_policy") = pysbio::IOPolicy::SyncPOSIX)
    .def_property_readonly("ok", &ContextWrapper::ok)
    .def_property_readonly("error", &ContextWrapper::error)
    .def_property_readonly("epolicy", &ContextWrapper::policy)
    .def("env", &ContextWrapper::env, py::arg("key"), py::arg("fallback") = "")
    .def("has_env", &ContextWrapper::has_env, py::arg("key"))
    .def("set_env", &ContextWrapper::set_env, py::arg("key"), py::arg("value"))
    .def("data_source",
         &ContextWrapper::data_source,
         py::arg("exp"),
         py::arg("run"),
         py::arg("events_per_read") = 43200,    // 6 minutes at 120 Hz
         py::arg("max_dgram_size") = 0x4000000, // ~67 MB
         py::arg("data_fmt") = pysbio::FTraits::XTC2,
         "Create a DataSource for a run, using this Context (which it keeps alive).");

#ifdef SBIO_HAS_MPI
  auto atexit = py::module_::import("atexit");
  atexit.attr("register")(py::cpp_function([]() {
    int initialized  { 0 };
    MPI_Initialized(&initialized);

    int finalized { 0 };
    if (initialized && !finalized) {
      // At exit, the atexit handler may be called before the GC has collected
      // remaining objects - we make sure to wait here, so any shared MPI resources
      // are valid for other ranks by registering our own handler with a Barrier
      MPI_Barrier(MPI_COMM_WORLD);
    }
  }));
#endif
} // pysbio_module
