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

  template <class DS_OR_SET, class... Groups>
  std::unique_ptr<ZipIterator> make_zip_iterator(DS_OR_SET& ds_or_set,
                                                 const std::vector<const BrokerGroupWrapper*>& wrappers,
                                                 bool batched,
                                                 const Groups&... groups) {
    auto make = [&] (auto RangeTag) -> std::unique_ptr<ZipIterator> {
      using Range = typename decltype(RangeTag)::type;
      using State = ZipState<Range>;

      auto state { std::make_shared<State>(ds_or_set, groups...) };

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
                                           std::move(wrappers));

    };

    if (batched) {
      return make(std::type_identity<sbio::ZippedBatchRange<DS_OR_SET, Groups...>> {});
    }

    return make(std::type_identity<sbio::ZippedGroupRange<DS_OR_SET, Groups...>> {});
  }

  template <
    bool IsSet,
    class DS_OR_SET,
    std::size_t GrpIdx,
    std::size_t NumGroups,
    class... Groups
  >
  std::unique_ptr<ZipIterator> unwrap_and_zip(DS_OR_SET& ds_or_set,
                                              const std::vector<const BrokerGroupWrapper*>& wrappers,
                                              bool batched,
                                              const Groups*... groups) {
    if constexpr (GrpIdx < NumGroups) {
      std::unique_ptr<ZipIterator> out;

      auto check_one = [&](auto* ds) {
        using DS = std::remove_cvref_t<decltype(*ds)>;
        using Group = std::remove_cvref_t<decltype(ds->get_stream_group(""))>;

        const auto* grp { std::get_if<Group>(&wrappers[GrpIdx]->grp) };
        if (is_datasource_group<DS, Group>(*ds,
                                           const_cast<BrokerGroupWrapper&>(*wrappers[GrpIdx]))) {
          out = unwrap_and_zip<IsSet, DS_OR_SET, GrpIdx + 1, NumGroups>(ds_or_set,
                                                                        wrappers,
                                                                        batched,
                                                                        groups...,
                                                                        grp);
        }
      };

      // Check the BrokerGroup belongs to the DataSource, or to one of the
      // DataSources if using a SourceSet
      if constexpr (IsSet) {
        auto check_one_all_ds = [&](auto*... ds) {
          ( check_one(ds), ... );
        };

        std::apply(check_one_all_ds, ds_or_set.sources());
      } else {
        check_one(&ds_or_set);
      }

      if (!out) {
        if constexpr (IsSet) {
          throw py::type_error("Can only zip BrokerGroup's of the SourceSet's DataSources!");
        } else {
          throw py::type_error("Can only zip BrokerGroup's from the DataSource!");
        }
      }

      return out;
    } else {
      return make_zip_iterator(ds_or_set, wrappers, batched, *groups...);
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

  std::unique_ptr<ZipIterator> make_zip(const PySource auto& py_src,
                                        const std::vector<const BrokerGroupWrapper*>& py_groups,
                                        bool batched) {
    if (py_groups.empty() || py_groups.size() > ZipIterator::MaxGroups) {
      throw std::invalid_argument("Can zip 1 to " + std::to_string(ZipIterator::MaxGroups) +
                                  " groups (given " + std::to_string(py_groups.size()) + ")");
    }

    using PySrcT = std::remove_cvref_t<decltype(py_src)>;
    constexpr bool IsSet { std::is_same_v<PySrcT, PySourceSet> ? true : false };

    auto dispatcher = [&](auto& src) -> std::unique_ptr<ZipIterator> {
      using Src = std::remove_reference_t<decltype(src)>;

      switch (py_groups.size()) {
      case 1: {
        return unwrap_and_zip<IsSet, Src, 0, 1>(src, py_groups, batched);
      }
      case 2: {
        return unwrap_and_zip<IsSet, Src, 0, 2>(src, py_groups, batched);
      }
      case 3: {
        return unwrap_and_zip<IsSet, Src, 0, 3>(src, py_groups, batched);
      }
      default: {
        return unwrap_and_zip<IsSet, Src, 0, 4>(src, py_groups, batched);
      }
      }
    };

    if constexpr (IsSet) {
      return std::visit(dispatcher, py_src.set());
    } else {
      return std::visit(dispatcher, py_src.ds());
    }
  }
} // namespace pysbio
