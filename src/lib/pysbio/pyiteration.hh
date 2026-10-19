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

#ifndef PYSBIO_PYITERATION_HH
#define PYSBIO_PYITERATION_HH

#include "pysbio/pybroker_group.hh"

#include "sbio/core/iteration.hh"

#include <pybind11/pybind11.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

namespace py = pybind11;

namespace pysbio {
  /**
   * A wrapper over sbio ranges for step, or batch (multi-step), iteration.
   *
   * The iterator can be used with either DataSources or SourceSets.
   */
  class StepIterator {
  public:
    template <class DS_OR_SET>
    StepIterator(DS_OR_SET& ds_or_set, bool batched, std::size_t batch_size) {
      using StepIdx = typename DS_OR_SET::StepIdxType;
      constexpr StepIdx Exhausted { DS_OR_SET::DataFormat::ExhaustedSentinel };

      using StepRange = typename DS_OR_SET::StepRange;
      using BatchRange = typename DS_OR_SET::BatchRange;

      if (batched) {
        m_range = std::make_shared<BatchRange>(ds_or_set, batch_size);
        m_next = [&ds_or_set, batch_size]() -> py::object {
          sbio::StepBatch<StepIdx> batch {};
          {
            py::gil_scoped_release release;
            batch = ds_or_set.next_batch(batch_size);
          }

          if (batch.first == Exhausted) {
            return py::none();
          }

          return py::cast(sbio::StepBatch<std::size_t> { batch.first, batch.last });
        };
      } else {
        m_range = std::make_shared<StepRange>(ds_or_set);
        m_next = [&ds_or_set]() -> py::object {
          StepIdx step {};
          {
            py::gil_scoped_release release;
            step = ds_or_set.next();
          }

          if (step == Exhausted) {
            return py::none();
          }

          return py::int_(static_cast<std::size_t>(step));
        };
      }
    }

    // No copy
    StepIterator(const StepIterator&) = delete;
    StepIterator& operator=(const StepIterator&) = delete;

    ~StepIterator() { close(); }

    py::object next();

    void close();

  private:
    std::function<py::object()> m_next {};
    std::shared_ptr<void> m_range {};
  };

  struct ZipStep {
    std::size_t step;
    sbio::IOStatus status;
  };

  struct ZipBatch {
    std::size_t first;
    std::size_t last;
    sbio::IOStatus status;
  };

  class ZipIterator {
  public:
    static constexpr std::size_t MaxGroups { 4 };

    ZipIterator(std::function<py::object()> next,
                std::shared_ptr<void> range,
                std::size_t num_fetches,
                const std::vector<const BrokerGroupWrapper*>& wrappers)
      : m_next(std::move(next))
      , m_range(std::move(range))
      , m_num_fetches(num_fetches)
      , m_keep_alive(std::move(wrappers))
    {}

    ZipIterator(const ZipIterator&) = delete;
    ZipIterator& operator=(const ZipIterator&) = delete;

    ~ZipIterator() { close(); }

    py::object next();
    void close();

    std::size_t num_fetches() const { return m_num_fetches; }

  private:
    std::function<py::object()> m_next {};
    std::shared_ptr<void> m_range {};        ///< Keep the range alive
    std::size_t m_num_fetches { 0 };
    std::vector<const BrokerGroupWrapper*> m_keep_alive {};
  };

  class PyDataSource;
  class PySourceSet;

  template <class PySrc>
  concept PySource =
    std::is_same_v<PySrc, PyDataSource> || std::is_same_v<PySrc, PySourceSet>;

  std::unique_ptr<ZipIterator> make_zip(const PySource auto& py_src,
                                        const std::vector<const BrokerGroupWrapper*>& py_groups,
                                        bool batched);
} // namespace pysbio

#endif // PYSBIO_PYITERATION_HH
