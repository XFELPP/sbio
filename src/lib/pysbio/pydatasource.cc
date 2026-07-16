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

#include "pysbio/pydetector.hh"

#ifdef SBIO_HAS_MPI
#include <mpi.h>
#endif
#include <pybind11/pybind11.h>

#include <string>
#include <variant>

namespace py = pybind11;

namespace pysbio {
  PyDataSource::PyDataSource(std::string exp,
                             unsigned run,
                             int evt_per_read,
                             int dgram_size,
                             unsigned xtc_ver,
                             std::string type)
    : m_exp(exp)
    , m_run(run)
  {
    if (xtc_ver == 1) {
#ifdef SBIO_HAS_XTC1
      sbio::XTC1Traits::StreamParameters base_cfg;
      base_cfg.events_per_read = evt_per_read;
      base_cfg.max_dgram_size = dgram_size;
      if (type == "serial") {
        SerialDataSource1 ds;
        ds.load_run(exp.c_str(), run, base_cfg);
        ds.discover_metadata();
        m_ds = std::move(ds);
      } else if (type == "mpi") {
#ifdef SBIO_HAS_MPI
        // If MPI hasn't been initialized, we'll do it for the user here
        int initialized { 0 };
        MPI_Initialized(&initialized);

        if (!initialized) {
          int argc { 0 };
          char** argv { nullptr };
          MPI_Init(&argc, &argv);
        }

        MPIDataSource1 ds;
        ds.load_run(exp.c_str(), run, base_cfg);
        ds.discover_metadata();
        m_ds = std::move(ds);
#else
        throw std::runtime_error("Requiested MPI DataSource, but sbio built without MPI support!");
#endif
      } else {
        throw std::runtime_error("Unsupported DataSource type: " + type + "!");
      }
#else
      throw std::runtime_error("Requested XTC1 DataSource, but sbio built without XTC1 support!");
#endif
    } else if (xtc_ver == 2){
#ifdef SBIO_HAS_XTC2
      sbio::XTC2Traits::StreamParameters base_cfg;
      base_cfg.events_per_read = evt_per_read;
      base_cfg.max_dgram_size = dgram_size;

      if (type == "serial") {
        SerialDataSource2 ds;
        ds.load_run(exp.c_str(), run, base_cfg);
        ds.discover_metadata();
        m_ds = std::move(ds);
      } else if (type == "mpi") {
#ifdef SBIO_HAS_MPI
        // If MPI hasn't been initialized, we'll do it for the user here
        int initialized { 0 };
        MPI_Initialized(&initialized);

        if (!initialized) {
          int argc { 0 };
          char** argv { nullptr };
          MPI_Init(&argc, &argv);
        }

        MPIDataSource2 ds;
        ds.load_run(exp.c_str(), run, base_cfg);
        ds.discover_metadata();
        m_ds = std::move(ds);
#else
        throw std::runtime_error("Requiested MPI DataSource, but sbio built without MPI support!");
#endif
      } else {
        throw std::runtime_error("Unsupported DataSource type: " + type + "!");
      }
#else
      throw std::runtime_error("Requested XTC2 DataSource, but sbio built without XTC2 support!");
#endif
    } else {
      throw std::runtime_error("Unrecognized XTC version: " + std::to_string(xtc_ver));
    }
  }

  py::object PyDataSource::detector(py::module& m, const char* name) {
    // TODO: This is quite kludgy because it goes back and forth between sbio det,
    //       the C++ DetectorWrapper, and full Python det for various metadata.
    //       Should normalize this....
    auto det_maker = [&](auto& ds) -> py::object {
      auto det = ds.get_stream_group(name);

      // The wrapper routine will also construct a serial number from pieces
      return wrap_detector(m, det);
    };

    return std::visit(det_maker, m_ds);
  }

  std::size_t PyDataSource::next() {
    auto next_evt = [](auto& ds) -> std::size_t {
      return ds.next();
    };

    return std::visit(next_evt, m_ds);
  }
} // namespace pysbio
