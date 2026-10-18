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

#ifndef PYSBIO_PYDATASOURCE_HH
#define PYSBIO_PYDATASOURCE_HH

#include "pysbio/execution/pyexecution.hh"
#include "pysbio/formats/pyformat_traits.hh"
#include "pysbio/io/pyio.hh"
#include "pysbio/policies.hh"

#include "sbio/core/context.hh"
#include "sbio/core/datasource.hh"
#include "sbio/execution/serial.hh"
#ifdef SBIO_HAS_MPI
#include "sbio/execution/mpi.hh"
#include "sbio/execution/mpi_threaded.hh"
#endif
#include "sbio/execution/threaded.hh"
#include "sbio/io/posix.hh"
#ifdef SBIO_HAS_XTC1
#include "sbio/formats/xtc1/xtc1_traits.hh"
#endif
#ifdef SBIO_HAS_XTC2
#include "sbio/formats/xtc2/xtc2_traits.hh"
#endif

#include <ncarray/ncarrays.hh>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdlib>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <variant>

namespace py = pybind11;

namespace pysbio {
  /**
   * The DataSource for any supported data format/execution policy (see policies.hh)
   */
  using DataSourceV = VariantOver<DataSourceFor>;

  struct BrokerGroupWrapper;
  class ContextWrapper;

  class StepIterator {
  public:
    template <class DS_OR_SET>
    StepIterator(DS_OR_SET& ds_or_set, bool batched, std::size_t batch_size);

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
                std::vector<py::object> keep_alive)
      : m_next(std::move(next))
      , m_range(std::move(range))
      , m_num_fetches(num_fetches)
      , m_keep_alive(std::move(keep_alive))
    {}

    ZipIterator(const ZipIterator&) = delete;
    ZipIterator& operator=(const ZipIterator&) = delete;

    ~ZipIterator() { close(); }

    py::object next();
    void close();

    std::size_t num_fetches() const { return m_num_fetches; }

  private:
    std::function<py::object()> m_next {};
    std::shared_ptr<void> m_range {};
    std::size_t m_num_fetches { 0 };
    std::vector<py::object> m_keep_alive {}; ///< The zipped groups
  };

  class PyDataSource {
  public:
    PyDataSource(std::string exp,
                 unsigned run,
                 int evt_per_read,
                 int dgram_size,
                 pysbio::FTraits data_fmt = pysbio::FTraits::XTC2,
                 pysbio::ExecutionPolicy epolicy = pysbio::ExecutionPolicy::MPI,
                 py::dict exec_cfg = py::dict(),
                 pysbio::IOPolicy io_policy = pysbio::IOPolicy::SyncPOSIX);

    // Setup to read 1 file (generally, HDF5, but XTC1/XTC2 work too)
    PyDataSource(std::string path,
                 int index_batch_size,
                 int max_buffer_size,
                 pysbio::FTraits data_fmt = pysbio::FTraits::CXI,
                 pysbio::ExecutionPolicy epolicy = pysbio::ExecutionPolicy::MPI,
                 py::dict exec_cfg = py::dict(),
                 pysbio::IOPolicy io_policy = pysbio::IOPolicy::SyncPOSIX);

    PyDataSource(DataSourceV ds,
                 std::string exp,
                 unsigned run,
                 std::shared_ptr<ContextWrapper> context);

    const std::set<std::string>& available_groups() const {
      return m_available_groups;
    }

    py::object group(py::module& m, const char* name);

    std::size_t next();

    std::unique_ptr<StepIterator> steps(bool batched, std::size_t batch_size);

    std::unique_ptr<ZipIterator> zip(const std::vector<py::object>& groups,
                                     bool batched);

    // TODO: Switch to making this const after the ConstIterator works when
    //       DataSource::next is properly restructured to support it.
    DataSourceV& ds() { return m_ds; }

  private:
    void enumerate_group_names();

    DataSourceV m_ds;

    std::set<std::string> m_available_groups;

    std::shared_ptr<ContextWrapper> m_context {};
  };

  class ContextWrapper {
  public:
    ContextWrapper(pysbio::ExecutionPolicy epolicy,
                   py::dict exec_cfg = py::dict(),
                   pysbio::IOPolicy io_policy = pysbio::IOPolicy::SyncPOSIX);

    bool ok() const;
    std::string error() const;

    std::string env(const std::string& key, const std::string& fallback) const;

    bool has_env(const std::string& key) const;

    void set_env(const std::string& key, const std::string& value);

    pysbio::ExecutionPolicy policy() const { return m_epolicy; }

    static PyDataSource data_source(const std::shared_ptr<ContextWrapper>& self,
                                    std::string exp,
                                    unsigned run,
                                    int evt_per_read,
                                    int dgram_size,
                                    pysbio::FTraits data_fmt);

  private:
    template <class EPolicy>
    using ContextPtr = std::unique_ptr<sbio::Context<sbio::SyncPOSIXIO, EPolicy>>;

    template <class Policies>
    struct ContextVariant;

    template <class... Ps>
    struct ContextVariant<TypeList<Ps...>> {
      using type = std::variant<ContextPtr<Ps>...>;
    };

    typename ContextVariant<ExecutionPolicies>::type m_context;
    pysbio::ExecutionPolicy m_epolicy;
   };

  template <class DS_OR_SET>
  StepIterator::StepIterator(DS_OR_SET& ds_or_set,
                             bool batched,
                             std::size_t batch_size) {
    using StepIdx = typename DS_OR_SET::StepIdxType;
    constexpr StepIdx Exhausted { DS_OR_SET::DataFormat::ExhaustedSentinel };

    if (batched) {
      m_range = std::make_shared<typename DS_OR_SET::BatchRange>(ds_or_set, batch_size);
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
      m_range = std::make_shared<typename DS_OR_SET::StepRange>(ds_or_set);
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
} // namespace pysbio

#endif // PYSBIO_PYDATASOURCE_HH
