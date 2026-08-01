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

#ifndef PYSBIO_CORE_EXECUTION_HH
#define PYSBIO_CORE_EXECUTION_HH

#include "sbio/core/execution.hh"
#include "sbio/core/io.hh"
#include "sbio/core/storage.hh"

#include "sbio/execution/serial.hh"

#include <ncarray/ncarrays.hh>
#include <ncarray/soarrays.hh>
#include <pybind11/pybind11.h>

#include <cstdint>
#include <memory>
#include <type_traits>

namespace py = pybind11;

namespace pysbio {
  enum class StorageRole : std::uint16_t {
    UNKNOWN  = 0,
    Metadata = 1,
    Data     = 2,
    Index    = 3,
    Group    = 4,
    Table    = 5
  };

  enum class StorageHint : std::uint16_t {
    NONE           = 0,
    Shareable      = 1,
    DeviceBound    = 2,
    DoubleBuffered = 3
  };

  /**
   * A minimal trampoline class for allowing Python Execution policy subclassing.
   *
   * The PyExecution class templates on a specific Execution policy to use a base
   * and fallback for methods and mechanisms not overrided in Python. This also
   * avoids the need and added complexity of requiring Python developers to also
   * implement Storage class overrides.
   *
   * @tparam BasePolicy The base Execution policy to override from.
   */
  template <class BasePolicy = sbio::SerialExecution>
  class PyExecution : public sbio::Execution<PyExecution<BasePolicy>> {
  public:
    using Config = typename BasePolicy::Config;
    static constexpr auto ParallelSupport = BasePolicy::ParallelSupport;

    template <typename Descriptor>
    using BufferTypeFor = typename BasePolicy::template BufferTypeFor<Descriptor>;

    PyExecution() { s_active_instance = this; }

    virtual ~PyExecution() {
      if (s_active_instance == this) {
        s_active_instance = nullptr;
      }
    }

    static void set_active_instance(PyExecution<BasePolicy>* inst) { s_active_instance = inst; }
    static PyExecution<BasePolicy>* get_active_instance() { return s_active_instance; }

    template <sbio::IsTypeList Requirements, class IO, class FTraits>
    requires sbio::FormatTraits<FTraits, IO, PyExecution<BasePolicy>>
    static auto allocate_storage_impl(sbio::AllocationRequest<FTraits>& request) {
      if constexpr (requires {
          BasePolicy::template allocate_storage<Requirements, IO, FTraits>(request);
        }) {
        auto base_storage = BasePolicy::template allocate_storage<Requirements, IO, FTraits>(request);
        return convert_storage(Requirements{}, std::move(base_storage));
      } else {
        auto base_storage = BasePolicy::template allocate_storage<IO, FTraits>(request);
        return convert_storage(Requirements{}, std::move(base_storage));
      }
    }

    template <class ConfigT>
    static void configure_impl(const ConfigT& config) {
      BasePolicy::configure(config);
    }

    static bool should_index_impl() {
      py::gil_scoped_acquire acquire;

      if (auto instance = get_active_instance()) {
        py::function fn = py::get_override(instance, "should_index");
        if (fn) {
          return fn().cast<bool>();
        }
      }

      return BasePolicy::should_index();
    }

    template <class BrokerType, class FTraits>
    static void on_discovery_impl(BrokerType& broker,
                                  typename FTraits::MetadataInventory& inv,
                                  sbio::IOStatus status) {
      {
        py::gil_scoped_acquire acquire;
        if (auto instance = get_active_instance())  {
          py::function fn = py::get_override(instance, "on_discovery");

          if (fn) {
            fn(py::cast(&broker, py::return_value_policy::reference),
               py::cast(&inv, py::return_value_policy::reference),
               status);
          }
        }
      }

      return BasePolicy::template on_discovery<BrokerType, FTraits>(broker,
                                                                    inv,
                                                                    status);
    }

    template <typename FTraits>
    static bool should_process_impl(typename FTraits::StepIdxType& step_idx) {
      {
        py::gil_scoped_acquire acquire;
        if (auto instance = get_active_instance()) {
          py::function fn = py::get_override(instance, "should_process");

          if (fn) {
            return fn(static_cast<std::size_t>(step_idx)).template cast<bool>();
          }
        }
      }

      return BasePolicy::template should_process<FTraits>(step_idx);
    }

    template <class Role, class StorageT>
    static void pre_update_impl(StorageT& storage) {
      {
        py::gil_scoped_acquire acquire;
        if (auto instance = get_active_instance()) {
          py::function fn = py::get_override(instance, "pre_update");

          if (fn) {
            auto role { get_storage_role<Role>() };
            fn(role, py::cast(&storage, py::return_value_policy::reference));
          }
        }
      }

      BasePolicy::template pre_update<Role, StorageT>(storage);
    }

    template <class Role, class StorageT, class SyncT>
    static void post_update_impl(StorageT& storage,
                                 SyncT&& sync_vars,
                                 sbio::IOStatus status) {
      {
        py::gil_scoped_acquire acquire;
        if (auto instance = get_active_instance()) {
          py::function fn = py::get_override(instance, "post_update");

          if (fn) {
            py::list py_sync_list;
            auto append_to_list = [&](auto& var) {
              py_sync_list.append(py::cast(var));
            };
            sync_vars.for_each(append_to_list);

            auto role { get_storage_role<Role>() };
            fn(role, py_sync_list, status);
          }
        }
      }

      BasePolicy::template post_update<Role, StorageT, SyncT>(storage,
                                                              std::forward<SyncT>(sync_vars),
                                                              status);
    }

    template <class FTraits, class IndexTrigger>
    static typename FTraits::StepIdxType next_impl(typename FTraits::StepIdxType& max_capacity,
                                                   IndexTrigger&& trigger) {
      {
        py::gil_scoped_acquire acquire;
        if (auto instance = get_active_instance()) {
          py::function fn = py::get_override(instance, "next");

          if (fn) {
            py::function py_trig = py::cpp_function([&]() { return trigger(); });
            return fn(static_cast<std::size_t>(max_capacity),
                      py_trig).template cast<typename FTraits::StepIdxType>();
          }
        }
      }

      return BasePolicy::template next<FTraits>(max_capacity, std::forward<IndexTrigger>(trigger));
    }

    template <class FTraits, class FetchCBType, class GetCBType>
    static sbio::IOStatus get_data_impl(typename FTraits::StepIdxType step_idx,
                                        FetchCBType&& unit_fetcher,
                                        std::size_t num_fetches,
                                        GetCBType&& unit_get_data,
                                        std::size_t num_accesses) {
      {
        py::gil_scoped_acquire acquire;
        if (auto instance = get_active_instance()) {
          py::function fn = py::get_override(instance, "get_data");

          if (fn) {
            py::function py_fetcher =
              py::cpp_function([&](std::size_t i) { return unit_fetcher(i); });

            py::function py_getter =
              py::cpp_function([&](std::size_t i) { unit_get_data(i); });

            return fn(step_idx,
                      py_fetcher,
                      num_fetches,
                      py_getter,
                      num_accesses).template cast<sbio::IOStatus>();
          }
        }
      }

      return BasePolicy::template get_data<FTraits>(step_idx,
                                                    std::forward<FetchCBType>(unit_fetcher),
                                                    num_fetches,
                                                    std::forward<GetCBType>(unit_get_data),
                                                    num_accesses);
    }

    template <class FTraits, class FetchCBType, class GetCBType>
    static sbio::IOStatus
    get_data_steps_impl(const hd_std::initializer_list<typename FTraits::StepIdxType>& steps,
                        FetchCBType&& unit_fetcher,
                        std::size_t num_fetches,
                        GetCBType&& unit_get_data,
                        std::size_t num_accesses) {
      {
        py::gil_scoped_acquire acquire;
        if (auto instance = get_active_instance()) {
          py::function fn = py::get_override(instance, "get_data_steps");

          if (fn) {
            py::function py_fetcher =
              py::cpp_function([&](std::size_t i) { return unit_fetcher(i); });

            py::function py_getter =
              py::cpp_function([&](std::size_t i, std::size_t cnt) { unit_get_data(i, cnt); });

            return fn(steps,
                      py_fetcher,
                      num_fetches,
                      py_getter,
                      num_accesses).template cast<sbio::IOStatus>();
          }
        }
      }

      return BasePolicy::template get_data_steps<FTraits>(steps,
                                                          std::forward<FetchCBType>(unit_fetcher),
                                                          num_fetches,
                                                          std::forward<GetCBType>(unit_get_data),
                                                          num_accesses);
    }

  private:
    template <typename Role>
    static StorageRole get_storage_role() {
      if constexpr (std::is_same_v<Role, sbio::MetadataRole>) {
        return StorageRole::Metadata;
      } else if constexpr (std::is_same_v<Role, sbio::DataRole>) {
        return StorageRole::Data;
      } else if constexpr (std::is_same_v<Role, sbio::IndexRole>) {
        return StorageRole::Index;
      } else if constexpr (std::is_same_v<Role, sbio::GroupRole>) {
        return StorageRole::Group;
      } else if constexpr (std::is_same_v<Role, sbio::TableRole>) {
        return StorageRole::Table;
      } else {
        return StorageRole::UNKNOWN;
      }
    }

    template <typename Hint>
    static StorageHint get_storage_hint() {
      if constexpr (std::is_same_v<Hint, sbio::Shareable>) {
        return StorageHint::Shareable;
      } else if constexpr (std::is_same_v<Hint, sbio::DeviceBound>) {
        return StorageHint::DeviceBound;
      } else if constexpr (std::is_same_v<Hint, sbio::DoubleBuffered>) {
        return StorageHint::DoubleBuffered;
      } else {
        return StorageHint::NONE;
      }
    }

    template <typename... Descriptors, typename BaseStorageT>
    static auto convert_storage(sbio::TypeList<Descriptors...>,
                                BaseStorageT&& base_storage) {
      sbio::Storage<sbio::TypeList<Descriptors...>, PyExecution<BasePolicy>> target_storage;

      // We offload the allocation of Storage to BasePolicy, but this results in a
      // template mismatch (Storage was created with <BasePolicy> but we need to return
      // Storage created with <PyExecution>) -- Just move the storage buffers over.
      auto move_base_to_py = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;
        target_storage.template get<Descriptor>() =
          std::move(base_storage.template get<Descriptor>());
      };

      ( (move_base_to_py(std::type_identity<Descriptors>{})), ... );

      return target_storage;
    }

    // Make thread local in case there is a threaded Execution policy.
    static inline thread_local PyExecution<BasePolicy>* s_active_instance { nullptr };
  };
} // namespace pysbio

#endif // PYSBIO_CORE_EXECUTION_HH
