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

#ifndef PYSBIO_IO_PYIO_HH
#define PYSBIO_IO_PYIO_HH

#include "sbio/core/stream.hh"

#include <array>
#include <cstdint>
#include <string>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace pysbio {
  namespace impl {
    /**
     * Construct the stream configuration for a data format from the GenericStreamConfig.
     *
     * @tparam FTraits The data format.
     * @param[out] mod The module to build the bindings in.
     * @param[in] name The name of bindings class.
     */
    template <typename FTraits>
    void bind_stream_config(py::module_& mod, const std::string& name) {
      using StreamConfig = sbio::GenericStreamConfig<FTraits>;
      py::classh<StreamConfig>(mod, name.c_str())
        .def(py::init<>())
        .def(py::init([](const sbio::StreamIdentity& id,
                         const std::array<sbio::StreamResource, StreamConfig::VariantCount>& res,
                         std::size_t max_buf_size,
                         std::size_t max_batch_size,
                         std::size_t index_batch_size) {
          StreamConfig cfg;
          cfg.identity = id;
          cfg.resources = res;
          cfg.max_buffer_size = max_buf_size;
          cfg.index_batch_size = index_batch_size;
          return cfg;
        }),
             py::arg("identity"),
             py::arg("resources"),
             py::arg("max_buffer_size") = 0x4000000,
             py::arg("max_batch_size") = 1,
             py::arg("index_batch_size") = 43200)
        .def_readwrite("identity", &StreamConfig::identity)
        .def_readonly_static("variant_count", &StreamConfig::VariantCount)
        .def_readwrite("resources", &StreamConfig::resources)
        .def_readwrite("max_buffer_size", &StreamConfig::max_buffer_size)
        .def_readwrite("max_batch_size", &StreamConfig::max_batch_size)
        .def_readwrite("index_batch_size", &StreamConfig::index_batch_size);
        //.def_readwrite("format_params", &StreamConfig::format_params);
    }
  } // namespace impl

  enum class IOPolicy : std::uint8_t {
    SyncPOSIX = 0
  };
} // namespace pysbio

#endif // PYSBIO_IO_PYIO_HH
