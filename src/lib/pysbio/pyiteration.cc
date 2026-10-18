/*
 * sbio - Stream Broker IO
 *
 * Copyright (C) 2025-2026 Gabriel Dorlhiac
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pysbio/pyiteration.hh"

#include "pysbio/pybroker_group.hh"
#include "pysbio/pydatasource.hh"
#include "pysbio/pysourceset.hh"

#include <pybind11/pybind11.h>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

namespace py = pybind11;

namespace {
  using pysbio::BrokerGroupWrapper;
  using pysbio::ZipIterator;

  template <class DS, class Group>
  bool is_datasource_group(const DS& ds, BrokerGroupWrapper& wrapper) {
    auto* grp { std::get_if<Group>(&wrapper.grp) };
    if (grp == nullptr) {
      return false;
    }

    // Also check the StreamBrokers
    const auto* first { ds.data_streams() };
    const auto* last { first + ds.num_data_streams() };
    for (std::size_t b_idx = 0; b_idx < grp->num_stream_brokers(); ++b_idx) {
      const auto* broker { grp->stream_brokers()[b_idx] };
      if (broker < first || broker >= last) {
        return false;
      }
    }

    return true;
  }

  template <class Range>
  struct ZipState {
    template <class DS_OR_SET, class... Groups>
    ZipState(DS_OR_SET& ds_or_set, const Groups&... groups)
      : range(ds_or_set, groups...)
    {}

    Range range;
    std::optional<typename Range::ConstIterator> it {};
    std::optional<typename Range::ConstIterator> end {};
  };

  template <class Set, std::size_t GrpIdx, std::size_t NumGroups, class... Groups>
  std::unique_ptr<ZipIterator> unwrap_and_make_zip(Set& set,
                                                   const std::vector<const BrokerGroupWrapper*>& wrappers,
                                                   bool batched,
                                                   std::vector<py::object> keep_alive,
                                                   const Groups*... groups) {

    auto make_zip = [&](const Groups&... grps) -> std::unique_ptr<ZipIterator> {
      auto make = [&] (auto RangeTag) -> std::unique_ptr<ZipIterator> {
        using Range = typename decltype(RangeTag)::type;
        using State = ZipState<Range>;

        auto state { std::make_shared<State>(set, grps...) };

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
            pysbio::ZipBatch zbatch {
              static_cast<std::size_t>(handle.first()),
              static_cast<std::size_t>(handle.last()),
              handle.status()
            };

            return py::cast(zbatch);
          } else {
            pysbio::ZipStep zstep {
              static_cast<std::size_t>(handle.step()),
              handle.status()
            };

            return py::cast(zstep);
          }
        };

        const std::size_t num_fetches { state->range.num_fetches() };
        return std::make_unique<ZipIterator>(std::move(next),
                                             std::move(state),
                                             num_fetches,
                                             std::move(keep_alive));

      };

      if (batched) {
        return make(std::type_identity<sbio::ZippedBatchRange<Set, Groups...>> {});
      }

      return make(std::type_identity<sbio::ZippedGroupRange<Set, Groups...>> {});
    };

    if constexpr (GrpIdx < NumGroups) {
      std::unique_ptr<ZipIterator> out;

      auto unwrap = [&](auto* ds) {
        using Group = std::remove_cvref_t<decltype(ds->get_stream_group(""))>;

        const auto* grp { std::get_if<Group>(&wrappers[GrpIdx]->grp) };
        out = unwrap_and_make_zip<Set, GrpIdx + 1, NumGroups>(const_cast<Set&>(set),
                                                              wrappers,
                                                              batched,
                                                              keep_alive,
                                                              groups...,
                                                              grp);
      };

      if constexpr (requires { set.sources(); }) {
        auto unwrap_all = [&](auto*... ds) {
          ( unwrap(ds), ... );
        };

        std::apply(unwrap_all, set.sources());
      } else {
        unwrap(&set);
      }

      return out;
    } else {
      return make_zip(*groups...);
    }
  }
} // anonymous namespace

namespace pysbio {
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

  std::unique_ptr<ZipIterator> make_zip(PyDataSource& py_ds,
                                        const std::vector<py::object>& groups,
                                        bool batched) {
    if (groups.empty() || groups.size() > ZipIterator::MaxGroups) {
      throw std::invalid_argument("Can zip 1 to " + std::to_string(ZipIterator::MaxGroups) +
                                  " groups (given " + std::to_string(groups.size()) + ")");
    }

    std::vector<const BrokerGroupWrapper*> zipped;
    auto dispatcher = [&](auto& ds) -> std::unique_ptr<ZipIterator> {
      using DS = std::remove_cvref_t<decltype(ds)>;
      using Group = std::remove_cvref_t<decltype(ds.get_stream_group(""))>;

      for (const auto& obj : groups) {
        auto& wrapper { obj.cast<BrokerGroupWrapper&>() };

        if (!is_datasource_group<DS, Group>(ds, wrapper)) {
          throw py::type_error("Can only zip BrokerGroup's of the SourceSet's DataSources!");
        }

        zipped.push_back(&wrapper); // Use BrokerGroupWrapper since type-erased
      }

      switch (zipped.size()) {
      case 1: {
        return unwrap_and_make_zip<DS, 0, 1>(ds,
                                             zipped,
                                             batched,
                                             groups);
      }
      case 2: {
        return unwrap_and_make_zip<DS, 0, 2>(ds,
                                             zipped,
                                             batched,
                                             groups);
      }
      case 3: {
        return unwrap_and_make_zip<DS, 0, 3>(ds,
                                             zipped,
                                             batched,
                                             groups);
      }
      default: {
        return unwrap_and_make_zip<DS, 0, 4>(ds,
                                             zipped,
                                             batched,
                                             groups);
      }
      }
    };

    return std::visit(dispatcher, py_ds.ds());
  }

  std::unique_ptr<ZipIterator> make_zip(PySourceSet& py_set,
                                        const std::vector<py::object>& groups,
                                        bool batched) {
    if (groups.empty() || groups.size() > ZipIterator::MaxGroups) {
      throw std::invalid_argument("Can zip 1 to " + std::to_string(ZipIterator::MaxGroups) +
                                  " groups (given " + std::to_string(groups.size()) + ")");
    }

    std::vector<const BrokerGroupWrapper*> zipped;
    auto dispatcher = [&](auto& set) -> std::unique_ptr<ZipIterator> {
      using Set = std::remove_reference_t<decltype(set)>;

      for (const auto& obj : groups) {
        auto& wrapper { obj.cast<BrokerGroupWrapper&>() };

        std::size_t i { 0 };
        std::size_t ds_grp_idx { 0 };

        auto set_idx_if_grp = [&](auto* ds) {
          using DS = std::remove_cvref_t<decltype(*ds)>;
          using Group = std::remove_cvref_t<decltype(ds->get_stream_group(""))>;
          if (is_datasource_group<DS, Group>(*ds, wrapper)) {
            ds_grp_idx = i + 1;
          }

          i++;
        };

        auto check_datasources = [&](auto*... ds) {
          ( set_idx_if_grp(ds), ... );
        };
        std::apply(check_datasources, set.sources());

        if (ds_grp_idx == 0) {
          throw py::type_error("Can only zip BrokerGroup's of the SourceSet's DataSources!");
        }

        zipped.push_back(&wrapper); // Use BrokerGroupWrapper since type-erased
      }

      switch (zipped.size()) {
      case 1: {
        return unwrap_and_make_zip<Set, 0, 1>(set,
                                              zipped,
                                              batched,
                                              groups);
      }
      case 2: {
        return unwrap_and_make_zip<Set, 0, 2>(set,
                                              zipped,
                                              batched,
                                              groups);
      }
      case 3: {
        return unwrap_and_make_zip<Set, 0, 3>(set,
                                              zipped,
                                              batched,
                                              groups);
      }
      default: {
        return unwrap_and_make_zip<Set, 0, 4>(set,
                                              zipped,
                                              batched,
                                              groups);
      }
      }
    };

    return std::visit(dispatcher, py_set.set());
  }

} // namespace pysbio
