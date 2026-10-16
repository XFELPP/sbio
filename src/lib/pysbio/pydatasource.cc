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

#include "sbio/core/iteration.hh"
#include "sbio/core/stream.hh"

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
  // Simple Execution policy configuration
  // The more technically inclined can use the full bindings via pysbio/execution

  template <class DS>
  void load_run(DS& ds,
                const std::string& exp,
                unsigned run,
                int evt_per_read,
                int dgram_size) {
    typename DS::StreamConfig cfg;
    cfg.index_batch_size = evt_per_read;
    cfg.max_buffer_size = dgram_size;

    ds.load_source(cfg, exp, run);
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

    ds.load_source(cfg, path);
    ds.discover_metadata();
  }

  template <class T, std::size_t>
  using Repeat = T;

  template <class Range>
  struct ZipState {
    template <class DS, class... Groups>
    ZipState(DS& ds, const Groups&... groups)
      : range(ds, groups...)
    {}

    Range range;
    std::optional<typename Range::ConstIterator> it {};
    std::optional<typename Range::ConstIterator> end {};
  };

  template <class DS, class Group, std::size_t... Is>
  std::unique_ptr<pysbio::ZipIterator> make_zip(DS& ds,
                                                const std::vector<const Group*>& groups,
                                                bool batched,
                                                std::vector<py::object> keep_alive,
                                                std::index_sequence<Is...>) {
    auto make = [&](auto RangeTag) {
      using Range = typename decltype(RangeTag)::type;
      using State = ZipState<Range>;

      auto state { std::make_shared<State>(ds, *groups[Is]...) };

      auto next = [state, batched]() -> py::object {
        {
          // Fetching happens as the iterator moves
          py::gil_scoped_release release;
          if (!state->it) {
            state->it.emplace(state->range.begin());
            state->end.emplace(state->range.end());
          } else {
            ++(*state->it);
          }
        }

        if (*state->it == *state->end) {
          return py::none();
        }

        auto handle { **state->it };
        if constexpr (requires { handle.batch(); }) {
          return py::cast(pysbio::ZipBatch {
            static_cast<std::size_t>(handle.first()),
            static_cast<std::size_t>(handle.last()),
            handle.status()
          });
        } else {
          return
            py::cast(pysbio::ZipStep {
                static_cast<std::size_t>(handle.step()),
                handle.status()
              });
        }
      };

      const std::size_t num_fetches { state->range.num_fetches() };
      return std::make_unique<pysbio::ZipIterator>(std::move(next),
                                                   std::move(state),
                                                   num_fetches,
                                                   std::move(keep_alive));
    };

    if (batched) {
      return make(std::type_identity<sbio::ZippedBatchRange<DS, Repeat<Group, Is>...>> {});
    }

    return make(std::type_identity<sbio::ZippedGroupRange<DS, Repeat<Group, Is>...>> {});
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
    : m_exp(exp)
    , m_run(run)
  {
    auto make_for_format = [&](auto FormatTag) -> DataSourceV {
      using Format = typename decltype(FormatTag)::type;
      auto make_for_policy = [&](auto PolicyTag) -> DataSourceV {
        using Policy = typename decltype(PolicyTag)::type;

        ensure_parallel_environment<Policy>();
        DataSourceFor<Policy, Format> ds(parse_config<Policy>(exec_cfg));
        // TODO: Need to fix this for different constructors (Locators) -- CXITraits
        if constexpr (std::is_same_v<Format, sbio::CXITraits>) {
          load_file(ds, exp, evt_per_read, dgram_size);
        } else {
          load_run(ds, exp, run, evt_per_read, dgram_size);
        }

        return DataSourceV { std::move(ds) };
      };

      return with_policy(epolicy, make_for_policy);
    };

    m_ds = with_format(data_fmt, make_for_format);
  }

  PyDataSource::PyDataSource(DataSourceV ds,
                             std::string exp,
                             unsigned run,
                             std::shared_ptr<ContextWrapper> context)
    : m_ds(std::move(ds))
    , m_exp(std::move(exp))
    , m_run(run)
    , m_context(std::move(context))
  {}

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

  std::unique_ptr<ZipIterator> PyDataSource::zip(const std::vector<py::object>& groups,
                                                 bool batched) {
    if (groups.empty() || groups.size() > ZipIterator::MaxGroups) {
      throw std::invalid_argument("Can zip 1 to " + std::to_string(ZipIterator::MaxGroups) +
                                  " groups (given " + std::to_string(groups.size()) + ")");
    }

    auto make = [&](auto& ds) -> std::unique_ptr<ZipIterator> {
      using DS = std::remove_reference_t<decltype(ds)>;
      using Group = std::remove_cvref_t<decltype(ds.get_stream_group(""))>;

      std::vector<const Group*> zipped;
      for (const auto& obj : groups) {
        auto& wrapper { obj.cast<BrokerGroupWrapper&>() };
        auto* grp { std::get_if<Group>(&wrapper.grp) };
        if (grp == nullptr) {
          throw py::type_error("Can only zip groups of this DataSource!");
        }

        // The group's StreamBrokers must be this DataSource's
        const auto* first { ds.data_streams() };
        const auto* last { first + ds.num_data_streams() };
        for (std::size_t b = 0; b < grp->num_stream_brokers(); ++b) {
          const auto* broker { grp->stream_brokers()[b] };
          if (broker < first || broker >= last) {
            throw py::value_error("Can only zip groups of this DataSource!");
          }
        }

        zipped.push_back(grp);
      }

      switch (zipped.size()) {
      case 1: {
        return make_zip<DS, Group>(ds,
                                   zipped,
                                   batched,
                                   groups,
                                   std::make_index_sequence<1> {});
      }
      case 2: {
        return make_zip<DS, Group>(ds,
                                   zipped,
                                   batched,
                                   groups,
                                   std::make_index_sequence<2> {});
      }
      case 3: {
        return make_zip<DS, Group>(ds,
                                   zipped,
                                   batched,
                                   groups,
                                   std::make_index_sequence<3> {});
      }
      default: {
        return make_zip<DS, Group>(ds,
                                   zipped,
                                   batched,
                                   groups,
                                   std::make_index_sequence<4> {});
      }
      }
    };

    return std::visit(make, m_ds);
  }

  /// --- Iterators --- ///
  py::object StepIterator::next() {
    py::object value { m_next ? m_next() : py::none() };
    if (value.is_none()) {
      close();
      throw py::stop_iteration();
    }

    return value;
  }

  void StepIterator::close() {
    m_next = nullptr;
    if (m_range) {
      // Ending the iteration may wait on other units (e.g. other threads)
      if (Py_IsInitialized() && PyGILState_Check()) {
        py::gil_scoped_release release;
        m_range.reset();
      } else {
        m_range.reset();
      }
    }
  }

  py::object ZipIterator::next() {
    py::object value { m_next ? m_next() : py::none() };
    if (value.is_none()) {
      close();
      throw py::stop_iteration();
    }

    return value;
  }

  void ZipIterator::close() {
    m_next = nullptr;
    if (m_range) {
      if (Py_IsInitialized() && PyGILState_Check()) {
        py::gil_scoped_release release;
        m_range.reset();
      } else {
        m_range.reset();
      }
    }
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
        std::make_unique<sbio::Context<sbio::SyncPOSIXIO, Policy>>(parse_config<Policy>(exec_cfg));
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

        // TODO: Need to fix this for different constructors (Locators) -- CXITraits
        if constexpr (std::is_same_v<Format, sbio::CXITraits>) {
          load_file(ds, exp, evt_per_read, dgram_size);
        } else {
          load_run(ds, exp, run, evt_per_read, dgram_size);
        }

        return DataSourceV { std::move(ds) };
      };

      return std::visit(make, self->m_context);
    };

    // Load before handing `exp` over (argument evaluation order is unspecified)
    DataSourceV ds { with_format(data_fmt, make_for_format) };

    return PyDataSource(std::move(ds), std::move(exp), run, self);
  }
} // namespace pysbio
