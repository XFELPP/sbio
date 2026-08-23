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

#ifndef PYSBIO_BINDING_BUILDERS_HH
#define PYSBIO_BINDING_BUILDERS_HH

#include "pysbio/formats/pyformat_traits.hh"

#include "sbio/core/io.hh"
#include "sbio/core/result.hh"
#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif
#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2_traits.hh"
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdlib>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>

namespace py = pybind11;

namespace pysbio::impl {
  /**
   * Given a concrete specialization of a StreamBroker, bind its functions in Python.
   *
   * @tparam BrokerT The type of the C++ StreamBroker concrete specialization.
   * @tparam PyBroker The type of the pybind11 bindings class.
   * @param[out] broker_cls The class instance to which functions will be bound.
   */
  template <typename BrokerT, typename PyBroker>
  void bind_broker_functions(PyBroker& broker_cls) {
    broker_cls.def(py::init<>())
      .def("prepare", &BrokerT::prepare)
      .def("index_stream", &BrokerT::index_stream)
      .def("capacity", &BrokerT::capacity)
      .def("metadata", [](const BrokerT& self) { return self.metadata(); })
      .def("set_metadata", &BrokerT::set_metadata)
      .def("fetch_step",
           &BrokerT::fetch_step,
           py::arg("step_idx"),
           py::arg("access_ptn"))
      .def("fetch_steps",
           &BrokerT::fetch_steps,
           py::arg("steps"),
           py::arg("access_ptn"))
      .def("get_data_in_buffer",
           &BrokerT::get_data_in_buffer,
           py::arg("data_req"),
           py::arg("access_ptn"),
           py::arg("batch_idx"));
  }

  /**
   * Given a concrete specialization of a BrokerGroup, bind its functions in Python.
   *
   * @tparam BrokerGroupT The type of the C++ BrokerGroup concrete specialization.
   * @tparam SegRef The type of the SegmentReference struct defined inside the Group.
   * @tparam PyBrokerGroup The type of the pybind11 bindings class.
   * @param[out] group_cls The class instance to which functions will be bound.
   */
  template <typename BrokerGroupT, typename SegRef, typename PyBrokerGroup>
  void bind_broker_group_functions(PyBrokerGroup& group_cls) {
    group_cls.def(py::init<>())
      .def(py::init<const char*, const char*, std::size_t, SegRef*>())
      .def("group_name", &BrokerGroupT::group_name)
      .def("group_type", &BrokerGroupT::group_type)
      .def("num_segments", &BrokerGroupT::num_segments)
      // .def("segments", &BrokerGroupT::segments)
      // .def("segments", &BrokerGroupT::segments)
      .def("num_stream_brokers", &BrokerGroupT::num_stream_brokers)
      .def("fetch_next_for",
           &BrokerGroupT::fetch_next_for,
           py::arg("step_idx"),
           py::arg("broker_num"))
      .def("fetch_steps_for",
           &BrokerGroupT::fetch_steps_for,
           py::arg("steps"),
           py::arg("broker_num"))
      .def("get_data",
           [](BrokerGroupT& self,
              std::size_t& step_idx,
              py::function callback,
              py::args args) {
             // For now, we basically only accept strings. So just handle them
             // explicitly... I don't know an elegant way atm to handle the
             // forwarding otherwise.

             using DataResult = sbio::DataResult;

             auto cpp_cb = [&](DataResult res) {
               py::gil_scoped_acquire acquire;
               callback(res);
             };

             if (args.size() == 2) {
               auto alg { args[0].cast<std::string>() };
               auto field { args[1].cast<std::string>() };

               return self.get_data(step_idx,
                                    cpp_cb,
                                    const_cast<const char*>(alg.c_str()),
                                    const_cast<const char*>(field.c_str()));
             } else {
               throw
                 std::runtime_error("Unsupported number of arguments: " +
                                    std::to_string(args.size()));
             }
           },
           py::arg("step_idx"),
           py::arg("unit_get_data_callback"))
      .def("get_data",
           [](BrokerGroupT& self,
              std::size_t& step_idx,
              py::args args) {
             // For now, we basically only accept strings. So just handle them
             // explicitly... I don't know an elegant way atm to handle the
             // forwarding otherwise.

             if (args.size() == 2) {
               auto alg { args[0].cast<std::string>() };
               auto field { args[1].cast<std::string>() };

               return self.get_data(step_idx,
                                    const_cast<const char*>(alg.c_str()),
                                    const_cast<const char*>(field.c_str()));
             } else {
               throw
                 std::runtime_error("Unsupported number of arguments: " +
                                    std::to_string(args.size()));
             }
           },
           py::arg("step_idx"))
      .def("get_multi_data",
           [](BrokerGroupT& self,
              const std::initializer_list<std::size_t>& steps,
              py::function callback,
              py::args args) {
             // For now, we basically only accept strings. So just handle them
             // explicitly... I don't know an elegant way atm to handle the
             // forwarding otherwise.

             using DataResult = typename sbio::DataResult;

             auto cpp_cb = [&](DataResult res) {
               py::gil_scoped_acquire acquire;
               callback(res);
             };

             if (args.size() == 2) {
               auto alg { args[0].cast<std::string>() };
               auto field { args[1].cast<std::string>() };

               return self.get_multi_data(steps,
                                          cpp_cb,
                                          const_cast<const char*>(alg.c_str()),
                                          const_cast<const char*>(field.c_str()));
             } else {
               throw
                 std::runtime_error("Unsupported number of arguments: " +
                                    std::to_string(args.size()));
             }
           },
           py::arg("steps"),
           py::arg("unit_get_data_callback"))
      .def("get_multi_data",
           [](BrokerGroupT& self,
              const std::initializer_list<std::size_t>& steps,
              py::args args) {
             // For now, we basically only accept strings. So just handle them
             // explicitly... I don't know an elegant way atm to handle the
             // forwarding otherwise.

             if (args.size() == 2) {
               auto alg { args[0].cast<std::string>() };
               auto field { args[1].cast<std::string>() };

               return self.get_multi_data(steps,
                                          const_cast<const char*>(alg.c_str()),
                                          const_cast<const char*>(field.c_str()));
             } else {
               throw
                 std::runtime_error("Unsupported number of arguments: " +
                                    std::to_string(args.size()));
             }
           },
           py::arg("steps"));
  }

  /**
   * Given a concrete specialization of the DataSource, bind its functions in Python.
   *
   * @tparam DataSourceT The type of the C++ DataSource concrete specialization.
   * @tparam PyDataSource The type of the pybind11 bindings class.
   * @param[out] datasource_cls The class instance to which functions will be bound.
   */
  template <typename DataSourceT, typename PyDataSource>
  void bind_datasource_functions(PyDataSource& datasource_cls) {
    datasource_cls.def(py::init<>())
      // Execution policy config must've been previously bound
      .def(py::init<const typename DataSourceT::EPolicyConfig&>())
      .def("configure_execution_policy",
           &DataSourceT::configure_execution_policy,
           py::arg("epolicy_cfg"))
      .def("load_run",
           [](DataSourceT& self,
              typename DataSourceT::StreamConfig base_cfg,
              py::args args) {
             if (args.size() == 0) {
               throw std::runtime_error("Must provide at least base StreamParameters for "
                                        "file format to configure and load run of "
                                        "DataSource!");
             } else if (args.size() == 2) {
               // Case for XTC1Traits and XTC2Traits: experiment, run
               auto exp = args[0].cast<std::string>();
               auto run_num = args[1].cast<unsigned>();

               return self.load_run(base_cfg, exp, run_num);
             } else {
               throw std::runtime_error("Unsupported number of arguments to load_run: " +
                                        std::to_string(args.size()));
             }
      })
      .def("discover_metadata", &DataSourceT::discover_metadata)
      .def("next", &DataSourceT::next)
      .def("get_stream_group", [](DataSourceT& self, const char* name) {
        return self.get_stream_group(name);
      },
        py::arg("name"))
      .def("num_data_streams", &DataSourceT::num_data_streams)
      // Get brokers
      // .def("data_streams", &DataSourceT::data_streams)
      // .def("data_streams", &DataSourceT::data_streams)
      .def("get_stream_broker", [](const DataSourceT& self, std::size_t b_num) {
        return self.data_stream(b_num);
      },
        py::arg("broker_num"));
  }

  /**
   * Given a concrete specialization of an Execution policy, create Python bindings.
   *
   * @tparam Exec The type of the C++ Execution policy.
   * @tparam PyExec The type of the pybind11 bindings class.
   * @param[out] exec_cls The class instance to which functions will be bound.
   */
  template <typename Exec, typename PyExec>
  void bind_epolicy_functions(PyExec& exec_cls) {
    exec_cls.def(py::init<>())
      .def_static("configure", [](const typename Exec::Config& cfg) {
        Exec::configure(cfg);
      },
        py::arg("config"))
      .def_static("should_index", &Exec::should_index)
      .def_static("should_process",
                  [](std::size_t& step_idx,
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
                  },
                  py::arg("step_idx"),
                  py::arg("ftraits"))
      .def_static("next",
                  [](std::size_t& capacity,
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
                  py::arg("reindex_trigger_callback"),
                  py::arg("ftraits"))
      .def_static("get_data",
                  [](std::size_t step_idx,
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
      .def_static("get_data_steps",
                  [](const std::initializer_list<std::size_t>& steps,
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
} // namespace pysbio::impl

#endif // PYSBIO_BINDING_BUILDERS_HH
