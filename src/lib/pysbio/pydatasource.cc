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
#include "pysbio/policies.hh"
#include "pysbio/pybroker_group.hh"
#include "pysbio/pyiteration.hh"

#include "sbio/core/iteration.hh"
#include "sbio/core/stream.hh"
#include "sbio/locators/path_pattern.hh"
#include "sbio/locators/single_file.hh"

#ifdef SBIO_HAS_HDF5
#include "sbio/formats/hdf5/cxi_locator.hh"
#include "sbio/formats/hdf5/cxi_spec.hh"
#endif

#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1_locator.hh"
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif

#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2_locator.hh"
#include "sbio/formats/xtc2/xtc2_traits.hh"
#endif

#ifdef SBIO_HAS_MPI
#include <mpi.h>
#endif
#include <pybind11/pybind11.h>

#include <iostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace py = pybind11;

namespace {
  template <class DS>
  void load_run(DS& ds,
                const std::string& exp,
                unsigned run,
                int evt_per_read,
                int dgram_size) {
    typename DS::StreamConfig cfg;
    cfg.index_batch_size = evt_per_read;
    cfg.max_buffer_size = dgram_size;

    ds.template load_source_with<sbio::PathPatternLocator>(cfg, exp, run);
    ds.discover_metadata();
  }

  template <class DS>
  void load_at_prefix(DS& ds,
                      const std::string& directory,
                      const std::string& prefix,
                      int index_batch_size,
                      int max_buffer_size,
                      const std::string& tag,
                      const std::string& ext) {
    typename DS::StreamConfig cfg;
    cfg.index_batch_size = index_batch_size;
    cfg.max_buffer_size = max_buffer_size;

    ds.template load_source_with<sbio::PathPatternLocator>(cfg,
                                                           directory,
                                                           prefix,
                                                           tag,
                                                           ext);
    ds.discover_metadata();
  }

  template <class DS>
  void load_file(DS& ds,
                 const std::string& path,
                 int index_batch_size,
                 int max_buffer_size) {
    typename DS::StreamConfig cfg;
    cfg.index_batch_size = index_batch_size;
    cfg.max_buffer_size = max_buffer_size;

    ds.template load_source_with<sbio::SingleFileLocator>(cfg, path);
    ds.discover_metadata();
  }
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
  {
    auto make_for_format = [&](auto FormatTag) -> DataSourceV {
      using Format = typename decltype(FormatTag)::type;
      auto make_for_policy = [&](auto PolicyTag) -> DataSourceV {
        using Policy = typename decltype(PolicyTag)::type;

        ensure_parallel_environment<Policy>();
        DataSourceFor<Policy, Format> ds(parse_epolicy_config<Policy>(exec_cfg));

        load_run(ds, exp, run, evt_per_read, dgram_size);

        return DataSourceV { std::move(ds) };
      };

      return with_policy(epolicy, make_for_policy);
    };

    m_ds = with_format(data_fmt, make_for_format);

    // Get a list of BrokerGroups (load_run runs discover_metadata)
    enumerate_group_names();
  }

  // Search for multiple files using a directory/prefix/tag/ext combo
  PyDataSource::PyDataSource(std::string directory,
                             std::string prefix,
                             int index_batch_size,
                             int max_buffer_size,
                             std::string tag,
                             std::string ext,
                             pysbio::FTraits data_fmt,
                             pysbio::ExecutionPolicy epolicy,
                             py::dict exec_cfg,
                             pysbio::IOPolicy io_policy)
  {
    auto make_for_format = [&](auto FormatTag) -> DataSourceV {
      using Format = typename decltype(FormatTag)::type;
      auto make_for_policy = [&](auto PolicyTag) -> DataSourceV {
        using Policy = typename decltype(PolicyTag)::type;

        ensure_parallel_environment<Policy>();
        DataSourceFor<Policy, Format> ds(parse_epolicy_config<Policy>(exec_cfg));

#if defined(SBIO_HAS_HDF5)
        if constexpr (std::is_same_v<Format, sbio::CXITraits>) {
          load_at_prefix(ds,
                         directory,
                         prefix,
                         index_batch_size,
                         max_buffer_size,
                         tag,
                         ext);
        } else {
          throw std::runtime_error("XTC1 and XTC2 do not support this locator strategy!");
        }
#else
        throw std::runtime_error("XTC1 and XTC2 do not support this locator strategy!");
#endif
        return DataSourceV { std::move(ds) };
      };

      return with_policy(epolicy, make_for_policy);
    };

    m_ds = with_format(data_fmt, make_for_format);

    // Get a list of BrokerGroups (load_run runs discover_metadata)
    enumerate_group_names();
  }

  // Setup to read 1 file (generally, HDF5, but XTC1/XTC2 work too)
  PyDataSource::PyDataSource(std::string path,
                             int index_batch_size,
                             int max_buffer_size,
                             pysbio::FTraits data_fmt,
                             pysbio::ExecutionPolicy epolicy,
                             py::dict exec_cfg,
                             pysbio::IOPolicy io_policy)
  {
    auto make_for_format = [&](auto FormatTag) -> DataSourceV {
      using Format = typename decltype(FormatTag)::type;
      auto make_for_policy = [&](auto PolicyTag) -> DataSourceV {
        using Policy = typename decltype(PolicyTag)::type;

        ensure_parallel_environment<Policy>();
        DataSourceFor<Policy, Format> ds(parse_epolicy_config<Policy>(exec_cfg));

        load_file(ds, path, index_batch_size, max_buffer_size);

        return DataSourceV { std::move(ds) };
      };

      return with_policy(epolicy, make_for_policy);
    };

    m_ds = with_format(data_fmt, make_for_format);

    // Get a list of BrokerGroups (load_file runs discover_metadata)
    enumerate_group_names();
  }

  void PyDataSource::enumerate_group_names() {
    auto check_inventories = [&](auto& ds) {
      for (std::size_t i = 0; i < ds.num_data_streams(); ++i) {
        const auto& broker { ds.data_stream(i) };
        const auto& inv { broker.metadata() };
        for (const auto& inv_grp : inv.groups()) {
          const auto& grp_key { inv_grp.key };
          if (m_available_groups.find(grp_key.group_name) == m_available_groups.end()) {
            m_available_groups.insert(grp_key.group_name);
          }
        }
      }
    };

    std::visit(check_inventories, m_ds);
  }

  PyDataSource::PyDataSource(DataSourceV ds,
                             std::shared_ptr<ContextWrapper> context)
    : m_ds(std::move(ds))
    , m_context(std::move(context))
  {
    enumerate_group_names();
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
        // TODO: Need to fix this... Need to bind for differetn Traits?
        return py::cast(std::make_shared<BrokerGroupWrapper>(grp));
      }
    };

    return std::visit(group_maker, m_ds);
  }

  std::size_t PyDataSource::next() {
    auto next_evt = [](auto& ds) -> std::size_t {
      py::gil_scoped_release release;
      return ds.next();
    };

    return std::visit(next_evt, m_ds);
  }

  std::unique_ptr<StepIterator> PyDataSource::steps(bool batched,
                                                    std::size_t batch_size) {
    auto make = [&](auto& ds) {
      return std::make_unique<StepIterator>(ds, batched, batch_size);
    };

    return std::visit(make, m_ds);
  }

  std::unique_ptr<ZipIterator> PyDataSource::zip(const std::vector<const BrokerGroupWrapper*>& groups,
                                                 bool batched) {
    return make_zip(*this, groups, batched);
  }

  /// --- Context --- ///
  ContextWrapper::ContextWrapper(pysbio::ExecutionPolicy epolicy,
                                 py::dict exec_cfg,
                                 pysbio::IOPolicy io_policy)
    : m_epolicy(epolicy)
  {
    auto make = [&](auto PolicyTag) -> decltype(m_context) {
      using Policy = typename decltype(PolicyTag)::type;

      ensure_parallel_environment<Policy>();

      return
        std::make_unique<sbio::Context<sbio::SyncPOSIXIO, Policy>>(parse_epolicy_config<Policy>(exec_cfg));
    };

    m_context = with_policy(epolicy, make);
  }

  bool ContextWrapper::ok() const {
    auto ctx_ok = [](const auto& ctx) { return ctx->ok(); };

    return std::visit(ctx_ok, m_context);
  }

  std::string ContextWrapper::error() const {
    auto ctx_error = [](const auto& ctx) { return ctx->error(); };

    return std::visit(ctx_error, m_context);
  }

  std::string ContextWrapper::env(const std::string& key, const std::string& fallback) const {
    auto ctx_env = [&](const auto& ctx) { return ctx->env(key, fallback); };

    return std::visit(ctx_env, m_context);
  }

  bool ContextWrapper::has_env(const std::string& key) const {
    auto ctx_has_env = [&](const auto& ctx) { return ctx->has_env(key); };

    return std::visit(ctx_has_env, m_context);
  }

  void ContextWrapper::set_env(const std::string& key, const std::string& value) {
    auto ctx_set_env = [&](auto& ctx) { ctx->set_env(key, value); };

    return std::visit(ctx_set_env, m_context);
  }

  PyDataSource ContextWrapper::data_source(const std::shared_ptr<ContextWrapper>& self,
                                           std::string exp,
                                           unsigned run,
                                           int evt_per_read,
                                           int dgram_size,
                                           pysbio::FTraits data_fmt) {
    if (!self->ok()) {
      throw std::runtime_error("Invalid Context: " + self->error());
    }

    auto make_for_format = [&](auto FormatTag) -> DataSourceV {
      using Format = typename decltype(FormatTag)::type;

      auto make = [&](const auto& ctx) -> DataSourceV {
        auto ds { ctx->template data_source<Format>() };

        load_run(ds, exp, run, evt_per_read, dgram_size);

        return DataSourceV { std::move(ds) };
      };

      return std::visit(make, self->m_context);
    };

    // Load before handing `exp` over (argument evaluation order is unspecified)
    DataSourceV ds { with_format(data_fmt, make_for_format) };

    return PyDataSource(std::move(ds), self);
  }

  PyDataSource ContextWrapper::data_source(const std::shared_ptr<ContextWrapper>& self,
                                           std::string directory,
                                           std::string prefix,
                                           int index_batch_size,
                                           int max_buffer_size,
                                           std::string tag,
                                           std::string ext,
                                           pysbio::FTraits data_fmt) {
    if (!self->ok()) {
      throw std::runtime_error("Invalid Context: " + self->error());
    }

    auto make_for_format = [&](auto FormatTag) -> DataSourceV {
      using Format = typename decltype(FormatTag)::type;

      auto make = [&](const auto& ctx) -> DataSourceV {
        auto ds { ctx->template data_source<Format>() };

#if defined(SBIO_HAS_HDF5)
        if constexpr (std::is_same_v<Format, sbio::CXITraits>) {
          load_at_prefix(ds,
                         directory,
                         prefix,
                         index_batch_size,
                         max_buffer_size,
                         tag,
                         ext);
        } else {
          throw std::runtime_error("XTC1 and XTC2 do not support this locator strategy!");
        }
#else
        throw std::runtime_error("XTC1 and XTC2 do not support this locator strategy!");
#endif

        return DataSourceV { std::move(ds) };
      };

      return std::visit(make, self->m_context);
    };

    // Load before handing `exp` over (argument evaluation order is unspecified)
    DataSourceV ds { with_format(data_fmt, make_for_format) };

    return PyDataSource(std::move(ds), self);
  }

  PyDataSource ContextWrapper::data_source(const std::shared_ptr<ContextWrapper>& self,
                                           std::string path,
                                           int index_batch_size,
                                           int max_buffer_size,
                                           pysbio::FTraits data_fmt) {
    if (!self->ok()) {
      throw std::runtime_error("Invalid Context: " + self->error());
    }

    auto make_for_format = [&](auto FormatTag) -> DataSourceV {
      using Format = typename decltype(FormatTag)::type;

      auto make = [&](const auto& ctx) -> DataSourceV {
        auto ds { ctx->template data_source<Format>() };

        load_file(ds, path, index_batch_size, max_buffer_size);

        return DataSourceV { std::move(ds) };
      };

      return std::visit(make, self->m_context);
    };

    // Load before handing `exp` over (argument evaluation order is unspecified)
    DataSourceV ds { with_format(data_fmt, make_for_format) };

    return PyDataSource(std::move(ds), self);
  }
} // namespace pysbio
