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

  /**
   * @brief A simple string wrapper for use as non-type template param in named args.
   *
   * FixedString's are used as NTTP to allow ergonomic, and type safe, named arguments
   * for DataRequest objects.
   *
   * @tparam N The length of the string.
   */
  template <hd_std::size_t N>
  struct FixedString {
    char buf[N] {};

    SBIO_HD constexpr FixedString(const char (&s)[N]) {
      for (hd_std::size_t i = 0; i < N; ++i) {
        buf[i] = s[i];
      }
    }

    template <hd_std::size_t M>
    SBIO_HD constexpr bool operator==(const FixedString<M>& other) const {
      // Strings of different lengths compare not equal...
      if constexpr (N != M) {
        return false;
      } else {
        for (hd_std::size_t i = 0; i < N; ++i) {
          if (buf[i] != other.buf[i]) {
            return false;
          }
        }

        return true;
      }
    }

    template <hd_std::size_t M>
    SBIO_HD constexpr bool operator!=(const FixedString<M>& other) const {
      return !(*this == other);
    }
  };

  /**
   * @brief The set of additional format-specific named arguments to use for DataRequest.
   */
  template <FixedString... Keys>
  struct RequestFieldSchema {
    static constexpr hd_std::size_t count { sizeof...(Keys) };

    template <FixedString Key>
    SBIO_HD static constexpr bool contains() { return ( (Keys == Key) || ... ); }
  };

  namespace impl {
    template <FixedString Key, FixedString First, FixedString... Rest>
    SBIO_HD constexpr hd_std::size_t get_key_index_impl(hd_std::size_t idx = 0) {
      if constexpr (Key == First) {
        return idx;
      } else {
        static_assert(sizeof...(Rest) > 0, "Key not found in format RequestFieldSchema!");

        return get_key_index_impl<Key, Rest...>(idx + 1);
      }
    }

    template <FixedString Key, FixedString... Keys>
    SBIO_HD constexpr hd_std::size_t get_key_index() {
      return get_key_index_impl<Key, Keys...>();
    }
  } // namespace impl

  /**
   * @brief A mechanism to attach a name/keyword to an argument.
   *
   * Classes and structs with constructors accepting NamedArg instances, and in
   * general multiple using variadic templates, can have various associated utilities
   * to make it more ergonomic to use them. This provides a semblance of keyword
   * arguments in standard C++.
   *
   * Currently, the main recommendation is to provide an associated literal to the
   * class argument, allow it to be used in any position in the constructor and with
   * the attached literal functioning as a keyword. E.g.:
   &
   * @code{.cpp}
   * namespace literals {
   *   constexpr auto operator ""_alg(const char* str, hd_std::size_t) {
   *     return NamedArg<"alg"> { str }
   *   }
   * } // namespace literals
   *
   * // Now, constructors accepting these NamedArg (like DataRequest) have kwargs
   * DataRequest("value_of_alg"_alg);
   * @endcode
   */
  template <FixedString Key>
  struct NamedArg {
    static constexpr auto key_name { Key };

    const char* value { nullptr };
  };

  // --- Concept helpers for argument/key conversion --- //
  template <typename T>
  concept IsStringLike = hd_std::is_convertible_v<T, const char*>;

  template <typename T>
  concept IsNamedArg = requires {
    hd_std::remove_cvref_t<T>::key_name;
  };

  template <hd_std::size_t ExpectedCount, typename... Args>
  concept ValidPositionalArgs = (ExpectedCount > 0) &&
    (sizeof...(Args) == ExpectedCount) &&
    ( IsStringLike<Args> && ... );

  template <hd_std::size_t ExpectedCount, typename... Args>
  concept ValidNamedArgs = (ExpectedCount > 0) &&
    (sizeof...(Args) == ExpectedCount) &&
    ( IsNamedArg<Args> && ... );

  // --- Generic DataRequest --- //

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
   * The additional format-specific fields are set using a sysem of `NamedArg` params.
   *
   * The additional request fields can be filled using the `set<NAME>` routine.
   * The group_name and group_type can be filled using standard C-string operations.
   *
   * The additional fields can be retrieved using the `get<NAME>` syntax, additionally,
   * a data format may implement overloads for specific literals which can be used.
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
  template <FixedString... Keys>
  struct SBIO_API DataRequest<RequestFieldSchema<Keys...>> {
    char group_name[SBIO_MAX_NAME_SIZE] {};
    char group_type[SBIO_MAX_NAME_SIZE] {};

    hd_std::size_t segment_number { 0 };

    char field_values[sizeof...(Keys) > 0 ? sizeof...(Keys) : 1][SBIO_MAX_NAME_SIZE] {};

    SBIO_HD DataRequest() = default;

    SBIO_HD DataRequest(const char* name, const char* type) {
      safe_strncpy(group_name, name, SBIO_MAX_NAME_SIZE);
      safe_strncpy(group_type, type, SBIO_MAX_NAME_SIZE);
    }

    template <typename... Args>
    requires ValidPositionalArgs<sizeof...(Keys), Args...>
    SBIO_HD DataRequest(const char* name, const char* type, Args... args) {
      safe_strncpy(group_name, name, SBIO_MAX_NAME_SIZE);
      safe_strncpy(group_type, type, SBIO_MAX_NAME_SIZE);

#ifndef __CUDA_ARCH__
      std::string_view arg_views[] = { std::string_view(args)... };
      for (hd_std::size_t i = 0; i < sizeof...(Keys); ++i) {
        if (arg_views[i].data()) {
          safe_strncpy(field_values[i], arg_views[i].data(), SBIO_MAX_NAME_SIZE);
        }
      }
#else
      const char* arg_ptrs[] = { static_cast<const char*>(args)... };
      for (hd_std::size_t i = 0; i < sizeof...(Keys); ++i) {
        if (arg_ptrs[i]) {
          safe_strncpy(field_values[i], arg_ptrs[i], SBIO_MAX_NAME_SIZE);
        }
      }
#endif // __CUDA_ARCH__
    }

    template <typename... NamedArgs>
    requires ValidNamedArgs<sizeof...(Keys), NamedArgs...>
    SBIO_HD DataRequest(const char* name, const char* type, NamedArgs... args) {
      safe_strncpy(group_name, name, SBIO_MAX_NAME_SIZE);
      safe_strncpy(group_type, type, SBIO_MAX_NAME_SIZE);

      auto bind_arg = [this](auto named_arg) {
        using ArgType = decltype(named_arg);

        constexpr hd_std::size_t idx { impl::get_key_index<ArgType::key_name, Keys...>() };
        if (named_arg.value) {
          safe_strncpy(field_values[idx], named_arg.value, SBIO_MAX_NAME_SIZE);
        }
      };

      ( bind_arg(args), ... );
    }

    /**
     * Retrieve the value of a named argument.
     *
     * @code{.cpp}
     * req.get<"field">();
     * @endcode
     *
     * @tparam The argument to retrieve.
     * @returns The value of the named argument.
     */
    template <FixedString Key>
    SBIO_HD inline const char* get() const {
      constexpr hd_std::size_t idx { impl::get_key_index<Key, Keys...>() };

      static_assert(idx < sizeof...(Keys), "Invalid key for data format!");

      return field_values[idx];
    }

    /**
     * Set the value of a named argument.
     *
     * @code{.cpp}
     * req.set<"field">("raw");
     * @endcode
     *
     * @tparam The argument to set.
     */
    template <FixedString Key>
    SBIO_HD inline void set(const char* val) {
      constexpr hd_std::size_t idx { impl::get_key_index<Key, Keys...>() };
      if (val) {
        safe_strncpy(field_values[idx], val, SBIO_MAX_NAME_SIZE);
      } else {
        field_values[idx][0] = '\0';
      }
    }
  };
} // namespace sbio

#endif // SBIO_CORE_REQUEST_HH
