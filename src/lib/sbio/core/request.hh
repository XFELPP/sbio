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

#ifndef SBIO_CORE_REQUEST_HH
#define SBIO_CORE_REQUEST_HH

#include "sbio/export_macro.hh"
#include "sbio/util/parameters.hh"
#include "sbio/util/string.hh"

#include <ncarray/dtype.hh>
#include <ncarray/soarrays.hh>

#ifdef __CUDACC__

#include <cuda/std/array>
#include <cuda/std/cstddef>
#include <cuda/std/cstdint>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <array>
#include <cstddef>
#include <cstdint>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

#ifndef __CUDA_ARCH__
#include <string_view>
#endif

#ifndef SBIO_MAX_NAME_SIZE
#define SBIO_MAX_NAME_SIZE 256
#endif

namespace sbio {
  template <typename ReqSchemaType>
  struct DataRequest;

  /**
   * @brief Container used to specify retrieval of a specific piece of data.
   *
   * The DataRequest is used by BrokerGroups to pull data of a specific type.
   * At minimum, the request always contains a group_name and group_type.
   * Depending on the specific data-format, the request may be made up of additional
   * parameters that uniquely determine the data requested. E.g., a format may require
   * a single additional parameter (like a `field` name). Some other format may require
   * two (e.g. XTC2 uses an `alg` and a `field`).
   *
   * The additional format-specific fields are stored using FixedName structs, which are
   * simple device-safe, and constexpr, string like objects. The set of `NamedKeys`
   * can be accessed with compile-time template mechanisms, or fully at runtime using
   * familiar bracket access, as with a map.
   *
   * The additional request fields can be filled at compile using the `set<NAME>`
   * routine.
   * The group_name and group_type are also FixedName objects and can be set using
   * simple assignment. (req.group_name = "MyGroupNameOfInterest").
   *
   * The additional fields can be retrieved using the `get<NAME>` syntax.
   *
   * Additionally, a final affordance allows a request to be constructed using
   * arguments converted into a `NamedArg`. The recommendation is that a format
   * provide a literal operator overload that allows attaching a keyword to a string
   * literal, which then allows them to be passed to a constructor positionally
   * in any order. This emulates the behaviour of keyword arguments.
   *
   * For example, the XTC2 format uses `_alg` and `_field` literals. These can be
   * passed in any order to the DataRequest constructor. By proxy, this means the
   * variadic getters for the BrokerGroup also work this way.
   *
   * @code{.cpp}
   * DataRequest("raw"_alg, "raw"_field);
   * DataRequest("raw"_field, "raw"_alg); // Equivalent
   * // This means the BrokerGroup variadic get_data routines work this way as well:
   * // grp.get_data(step, "raw"_alg, "raw"_field);
   * // grp.get_data(step, "raw"_field, "raw"_alg); // Equivalent
   * @endcode
   *
   * This additional literal syntax is format-specific. Refer to the relevant docs.
   * The positional constructor is always available.
   *
   * @tparam Keys... The set of named schema fields. E.g. <"alg", "field">
   */
  template <FixedName... Keys>
  struct SBIO_API DataRequest<NamedKeys<Keys...>> {
    FixedName<> group_name;
    FixedName<> group_type;
    hd_std::size_t segment_number { 0 };

    using FieldKeys = NamedKeys<Keys...>;                     ///< Keyword names
    hd_std::array<FixedName<>, sizeof...(Keys)> field_values; ///< Values for each keyword FieldKey

    SBIO_HD DataRequest() = default;

    SBIO_HD DataRequest(const char* name, const char* type) {
      group_name = name;
      group_type = type;
    }

    /**
     * @brief Construct a request with name, type and *positional* FixedNames.
     *
     * The FixedName arguments must be passed in the appropriate order to match
     * the way the fields were defined by the format. E.g., a format that provided
     * <"alg", "field"> will then expect that the `alg` value come before the `field`
     * value in construction of the request.
     *
     * For use of a "keyword" argument like syntax, see the NamedArgs constructor.
     *
     * @tparam The types of the FixedName arguments
     * @param[in] name The BrokerGroup name.
     * @param[in] type The BrokerGroup type.
     * @param[in] args... The series of FixedName arguments in the order of the fields.
     */
    template <typename... Args>
    requires ValidPositionalArgs<sizeof...(Keys), Args...>
    SBIO_HD DataRequest(const char* name, const char* type, Args... args) {
      group_name = name;
      group_type = type;

      const char* field_ptrs[] = { static_cast<const char*>(args)... };
      for (hd_std::size_t i = 0; i < sizeof...(Keys); ++i) {
        if (field_ptrs[i]) {
          field_values[i] = field_ptrs[i];
        }
      }
    }

    /**
     * @brief Construct a request with name, type and NamedArg instances.
     *
     * By convention, NamedArgs can be created from literals where a literal operator
     * overload has been provided.
     *
     * @tparam The types of NamedArgs.
     * @param[in] name The BrokerGroup name.
     * @param[in] type The BrokerGroup type.
     * @param[in] args... The series of NamedArgs.
     */
    template <typename... NamedArgs>
    requires ValidNamedArgs<sizeof...(Keys), NamedArgs...>
    SBIO_HD DataRequest(const char* name, const char* type, NamedArgs... args) {
      group_name = name;
      group_type = type;

      auto bind_arg = [this](auto named_arg) {
        using ArgType = decltype(named_arg);

        constexpr hd_std::size_t idx { impl::get_key_index<ArgType::key_name, Keys...>() };
        if (named_arg.value) {
          field_values[idx] = named_arg.value;
        }
      };

      ( bind_arg(args), ... );
    }

    /**
     * Retrieve the value of a named argument - segment number cannot be accessed.
     *
     * The value of `key_name` may be "group_name", "group_type" or any of the
     * format-specific values that are provided. The segment number cannot be
     * retrieved from this function.
     *
     * @code{.cpp}
     * req["field"];
     * @endcode
     *
     * @note Unlike .get<""> this is NOT checked at compile.
     *
     * @tparam The FixedName type.
     * @param[in] key_name The name of the key to retrieve.
     * @returns The name of the field to retrieve.
     */
    template <typename FKey>
    SBIO_HD inline FixedName<>& operator[](const FKey& key_name) {
      if (key_name == "group_name") {
        return group_name;
      } else if (key_name == "group_type") {
        return group_type;
      } else {
        for (hd_std::size_t i = 0; i < sizeof...(Keys); ++i) {
          if (FieldKeys::keys[i] == key_name) {
            return field_values[i];
          }
        }

        static FixedName<> fallback{};
        return fallback; // Fallback option?
      }
    }

    /**
     * Retrieve the value of a named argument - segment number cannot be accessed.
     *
     * The value of `key_name` may be "group_name", "group_type" or any of the
     * format-specific values that are provided. The segment number cannot be
     * retrieved from this function.
     *
     * @code{.cpp}
     * req["field"];
     * @endcode
     *
     * @note Unlike .get<""> this is NOT checked at compile.
     *
     * @tparam The FixedName type.
     * @param[in] key_name The name of the key to retrieve.
     * @returns The name of the field to retrieve.
     */
    template <typename FKey>
    SBIO_HD inline const FixedName<>& operator[](const FKey& key_name) const {
      if (key_name == "group_name") {
        return group_name;
      } else if (key_name == "group_type") {
        return group_type;
      } else {
        for (hd_std::size_t i = 0; i < sizeof...(Keys); ++i) {
          if (FieldKeys::keys[i] == key_name) {
            return field_values[i];
          }
        }
      }
    }

    /**
     * Retrieve the value of a named argument.
     *
     * @code{.cpp}
     * req.get<"field">();
     * @endcode
     *
     * @note This is checked at compile.
     *
     * @tparam The argument to retrieve.
     * @returns The value of the named argument.
     */
    template <FixedName Key>
    SBIO_HD inline const FixedName<>& get() const {
      if constexpr (Key == "group_name") {
        return group_name;
      } else if constexpr (Key == "group_type") {
        return group_type;
      } else {
        constexpr hd_std::size_t idx { impl::get_key_index<Key, Keys...>() };

        static_assert(idx < sizeof...(Keys), "Invalid key for data format!");

        return field_values[idx];
      }
    }

    /**
     * Set the value of a named argument.
     *
     * @code{.cpp}
     * req.set<"field">("raw");
     * @endcode
     *
     * @note This is checked at compile.
     *
     * @tparam The argument to set.
     */
    template <FixedName Key>
    SBIO_HD inline void set(const char* val) {
      constexpr hd_std::size_t idx { impl::get_key_index<Key, Keys...>() };
      if (val) {
        field_values[idx] = val;
      } else {
        field_values[idx] = "\0";
      }
    }
  };
} // namespace sbio

#endif // SBIO_CORE_REQUEST_HH
