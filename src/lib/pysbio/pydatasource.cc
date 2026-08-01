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

#include "pysbio/pydatasource.hh"

#include "pysbio/execution/pyexecution.hh"
#include "pysbio/formats/pyformat_traits.hh"
#include "pysbio/io/pyio.hh"
#include "pysbio/pybroker_group.hh"

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

#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace py = pybind11;

namespace {
  // Simple Execution policy configuration
  // The more technically inclined can use the full bindings via pysbio/execution

  sbio::ThreadedExecution::Config parse_threaded_config(const py::dict& d) {
    sbio::ThreadedExecution::Config cfg;
    if (d.contains("num_threads")) {
      cfg.num_threads = d["num_threads"].cast<std::size_t>();
    }
    if (d.contains("cpu_affinities")) {
      cfg.cpu_affinities = d["cpu_affinities"].cast<std::vector<int>>();
    }

    return cfg;
  }

#ifdef SBIO_HAS_MPI
  sbio::MPIExecution::Config parse_mpi_config(const py::dict& d) {
    sbio::MPIExecution::Config cfg;
    if (d.contains("active_ranks")) {
      cfg.active_ranks = d["active_ranks"].cast<std::vector<int>>();
    }

    if (d.contains("main_rank")) {
      cfg.main_rank = d["main_rank"].cast<int>();
    }

    if (d.contains("main_rank_loops")) {
      cfg.main_rank_loops = d["main_rank_loops"].cast<bool>();
    }
    return cfg;
  }

  sbio::MPIThreadedExecution::Config parse_mpi_threaded_config(const py::dict& d) {
    sbio::MPIThreadedExecution::Config cfg;
    if (d.contains("num_threads")) {
      cfg.num_threads = d["num_threads"].cast<std::size_t>();
    }

    if (d.contains("cpu_affinities")) {
      cfg.cpu_affinities = d["cpu_affinities"].cast<std::vector<int>>();
    }

    if (d.contains("active_ranks")) {
      cfg.active_ranks = d["active_ranks"].cast<std::vector<int>>();
    }

    if (d.contains("main_rank")) {
      cfg.main_rank = d["main_rank"].cast<int>();
    }

    if (d.contains("main_rank_loops")) {
      cfg.main_rank_loops = d["main_rank_loops"].cast<bool>();
    }

    return cfg;
  }
#endif
} // anonymous namespace


namespace pysbio {
  PyDataSource::PyDataSource(std::string exp,
                             unsigned run,
                             int evt_per_read,
                             int dgram_size,
                             pysbio::FTraits data_fmt,
                             pysbio::ExecutionPolicy epolicy,
                             py::dict exec_cfg,
                             pysbio::IOPolicy io_policy)
    : m_exp(exp)
    , m_run(run)
  {
    if (data_fmt == pysbio::FTraits::XTC1) {
#ifdef SBIO_HAS_XTC1
      sbio::XTC1Traits::StreamParameters base_cfg;
      base_cfg.events_per_read = evt_per_read;
      base_cfg.max_dgram_size = dgram_size;
      if (epolicy == pysbio::ExecutionPolicy::Serial) {
        SerialDataSource1 ds;
        ds.load_run(base_cfg, exp, run);
        ds.discover_metadata();
        m_ds = std::move(ds);
      } else if (epolicy == pysbio::ExecutionPolicy::Threaded) {
        auto ecfg = parse_threaded_config(exec_cfg);
        ThreadedDataSource1 ds(ecfg);
        ds.load_run(base_cfg, exp, run);
        ds.discover_metadata();
        m_ds = std::move(ds);
      } else if (epolicy == pysbio::ExecutionPolicy::MPI) {
#ifdef SBIO_HAS_MPI
        // If MPI hasn't been initialized, we'll do it for the user here
        int initialized { 0 };
        MPI_Initialized(&initialized);

        if (!initialized) {
          int argc { 0 };
          char** argv { nullptr };
          MPI_Init(&argc, &argv);
        }

        auto ecfg = parse_mpi_config(exec_cfg);
        MPIDataSource1 ds(ecfg);
        ds.load_run(base_cfg, exp, run);
        ds.discover_metadata();
        m_ds = std::move(ds);
#else
        throw std::runtime_error("Requiested MPI DataSource, but sbio built without MPI support!");
#endif
      } else if (epolicy == pysbio::ExecutionPolicy::MPIThreaded) {
#ifdef SBIO_HAS_MPI
        int initialized { 0 };
        MPI_Initialized(&initialized);

        if (!initialized) {
          int argc { 0 };
          char** argv { nullptr };

          int provided;
          MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
          if (provided < MPI_THREAD_MULTIPLE) {
            // Need to check if you get the thread_multiple or not.
            std::cout << "Was not able to provide full MPI thread support." << std::endl
                      << "- Support was provied at level: " << provided << std::endl;
          }
        }

        auto ecfg = parse_mpi_threaded_config(exec_cfg);
        ThreadedMPIDataSource1 ds(ecfg);
        ds.load_run(base_cfg, exp, run);
        ds.discover_metadata();
        m_ds = std::move(ds);
#else
        throw std::runtime_error("Requested Threaded MPI DataSource, but sbio built without MPI!");
#endif
      } else {
        throw std::runtime_error("Unsupported DataSource type: " +
                                 std::to_string(static_cast<int>(epolicy)) + "!");
      }
#else
      throw std::runtime_error("Requested XTC1 DataSource, but sbio built without XTC1 support!");
#endif
    } else if (data_fmt == pysbio::FTraits::XTC2){
#ifdef SBIO_HAS_XTC2
      sbio::XTC2Traits::StreamParameters base_cfg;
      base_cfg.events_per_read = evt_per_read;
      base_cfg.max_dgram_size = dgram_size;

      if (epolicy == pysbio::ExecutionPolicy::Serial) {
        SerialDataSource2 ds;
        ds.load_run(base_cfg, exp, run);
        ds.discover_metadata();
        m_ds = std::move(ds);
      } else if (epolicy == pysbio::ExecutionPolicy::Threaded) {
        auto ecfg = parse_threaded_config(exec_cfg);
        ThreadedDataSource2 ds(ecfg);
        ds.load_run(base_cfg, exp, run);
        ds.discover_metadata();
        m_ds = std::move(ds);
      } else if (epolicy == pysbio::ExecutionPolicy::MPI) {
#ifdef SBIO_HAS_MPI
        // If MPI hasn't been initialized, we'll do it for the user here
        int initialized { 0 };
        MPI_Initialized(&initialized);

        if (!initialized) {
          int argc { 0 };
          char** argv { nullptr };
          MPI_Init(&argc, &argv);
        }

        auto ecfg = parse_mpi_config(exec_cfg);
        MPIDataSource2 ds(ecfg);
        ds.load_run(base_cfg, exp, run);
        ds.discover_metadata();
        m_ds = std::move(ds);
#else
        throw std::runtime_error("Requiested MPI DataSource, but sbio built without MPI support!");
#endif
      } else if (epolicy == pysbio::ExecutionPolicy::MPIThreaded) {
#ifdef SBIO_HAS_MPI
        int initialized { 0 };
        MPI_Initialized(&initialized);

        if (!initialized) {
          int argc { 0 };
          char** argv { nullptr };

          int provided;
          MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
          if (provided < MPI_THREAD_MULTIPLE) {
            // Need to check if you get the thread_multiple or not.
            std::cout << "Was not able to provide full MPI thread support." << std::endl
                      << "- Support was provied at level: " << provided << std::endl;
          }
        }

        auto ecfg = parse_mpi_threaded_config(exec_cfg);
        ThreadedMPIDataSource2 ds(ecfg);
        ds.load_run(base_cfg, exp, run);
        ds.discover_metadata();
        m_ds = std::move(ds);
#else
        throw std::runtime_error("Requested Threaded MPI DataSource, but sbio built without MPI!");
#endif
      } else {
        throw std::runtime_error("Unsupported DataSource type: " +
                                 std::to_string(static_cast<int>(epolicy)) + "!");
      }
#else
      throw std::runtime_error("Requested XTC2 DataSource, but sbio built without XTC2 support!");
#endif
    } else {
      throw std::runtime_error("Unrecognized format specifier: " + std::to_string(static_cast<int>(data_fmt)));
    }
  }

  py::object PyDataSource::group(py::module& m, const char* name) {
    // TODO: This is quite kludgy because it goes back and forth between sbio det,
    //       the C++ DetectorWrapper, and full Python det for various metadata.
    //       Should normalize this....
    auto group_maker = [&](auto& ds) -> py::object {
      auto grp = ds.get_stream_group(name);

      using DataFormat = typename decltype(grp)::DataFormat;

      constexpr bool use_wrapper =
#ifdef SBIO_HAS_XTC1
        std::is_same_v<DataFormat, sbio::XTC1Traits> ||
#endif
#ifdef SBIO_HAS_XTC2
        std::is_same_v<DataFormat, sbio::XTC2Traits> ||
#endif
        false;

      // For XTC1 and XTC2 have special bindings to remap accessors as methods
      // The wrapper routine will also construct a serial number from pieces
      if constexpr (use_wrapper) {
        return wrap_xtc_detector(m, grp);
      } else {
        return grp;
      }
    };

    return std::visit(group_maker, m_ds);
  }

  std::size_t PyDataSource::next() {
    auto next_evt = [](auto& ds) -> std::size_t {
      return ds.next();
    };

    return std::visit(next_evt, m_ds);
  }
} // namespace pysbio
