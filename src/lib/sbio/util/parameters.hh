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

#ifndef SBIO_UTIL_PARAMETERS_HH
#define SBIO_UTIL_PARAMETERS_HH

#include <ncarray/dtype.hh>

#ifdef __CUDACC__

#include <cuda/std/array>
#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/cstring>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif // __CUDACC__

#ifndef __CUDA_ARCH__
// Provide converting constructors/casts/etc for host-only
#include <string>
#include <string_view>
#endif

#ifndef SBIO_MAX_NAME_SIZE
#define SBIO_MAX_NAME_SIZE 256
#endif

namespace sbio {
  template <hd_std::size_t N = SBIO_MAX_NAME_SIZE>
  struct FixedName {
    hd_std::array<char, N> storage {};

    SBIO_HD constexpr FixedName() = default;

    // Move/copy (same thing) for same size
    SBIO_HD constexpr FixedName(const FixedName&) = default;
    SBIO_HD constexpr FixedName(FixedName&&) noexcept = default;
    SBIO_HD constexpr FixedName& operator=(const FixedName&) = default;
    SBIO_HD constexpr FixedName& operator=(FixedName&&) noexcept = default;

    // Converting between sizes
    template <hd_std::size_t M>
    SBIO_HD constexpr FixedName(const FixedName<M>& other) {
      hd_std::size_t len { M < N ? M : N };
      for (hd_std::size_t i = 0; i < len; ++i) {
        storage[i] = other.storage[i];
      }
    }

    template <hd_std::size_t M>
    SBIO_HD constexpr FixedName(FixedName<M>&& other) noexcept {
      hd_std::size_t len { M < N ? M : N };
      for (hd_std::size_t i = 0; i < len; ++i) {
        storage[i] = other.storage[i];
      }
    }

    template <hd_std::size_t M>
    SBIO_HD constexpr FixedName& operator=(FixedName<M>&& other) noexcept {
      hd_std::size_t len { M < N ? M : N };
      for (hd_std::size_t i = 0; i < len; ++i) {
        storage[i] = other.storage[i];
      }
      if constexpr (N > M) {
        storage[M] = '\0';
      }
      return *this;
    }

    // -- Construct from C-string
    SBIO_HD constexpr FixedName(const char* str) {
      if (str) {
        for (hd_std::size_t i = 0; i < N - 1 && str[i] != '\0'; ++i) {
          storage[i] = str[i];
        }
      }
    }

    SBIO_HD inline FixedName& operator=(const char* str) {
      if (str) {
        for (hd_std::size_t i = 0; i < N - 1 && str[i] != '\0'; ++i) {
          storage[i] = str[i];
        }
      }

      return *this;
    }

#ifndef __CUDA_ARCH__
    // --- Construct/cast to string_view --- //
    constexpr FixedName(std::string_view sv) {
      std::size_t len { std::min(sv.size(), N - 1) };
      std::copy_n(sv.data(), len, storage.begin());
    }

    constexpr FixedName& operator=(std::string_view sv) {
      std::size_t len { std::min(sv.size(), N - 1) };
      std::copy_n(sv.data(), len, storage.begin());

      return *this;
    }

    constexpr operator std::string_view() const {
      return { storage.data() };
    }

    // --- Cast to string --- //

    constexpr operator std::string() const {
      return { storage.data() };
    }
#endif

    SBIO_HD constexpr const char* c_str() const { return storage.data(); }

    // --- General Comparisons --- //
    SBIO_HD constexpr auto operator<=>(const FixedName& other) const = default;
    SBIO_HD constexpr bool operator==(const FixedName& other) const = default;
    SBIO_HD constexpr bool operator!=(const FixedName& other) const = default;
    SBIO_HD constexpr bool operator<(const FixedName& other) const = default;
    SBIO_HD constexpr bool operator>(const FixedName& other) const = default;

#ifndef __CUDA_ARCH__
    // --- Stringy Comparisons --- //
    constexpr bool operator==(std::string_view sv) const {
      return (sv == storage.data());
    }
    constexpr bool operator!=(std::string_view sv) const {
      return !(*this == sv);
    }
    constexpr bool operator<(std::string_view sv) const {
      return (sv < storage.data());
    }
    constexpr bool operator>(std::string_view sv) const {
      return (sv > storage.data());
    }
#endif

    // --- C-string comparison --- //
    SBIO_HD inline auto operator==(const char* str) const {
      if (str) {
        return hd_std::strcmp(storage.data(), str) == 0 ? true : false;
      }
      return false;
    }
  };

  namespace impl {
    template <FixedName Key, FixedName First, FixedName... Rest>
    SBIO_HD constexpr hd_std::size_t get_key_index_impl(hd_std::size_t idx = 0) {
      if constexpr (Key == First) {
        return idx;
      } else {
        static_assert(sizeof...(Rest) > 0, "Key not found in format RequestFieldSchema!");

        return get_key_index_impl<Key, Rest...>(idx + 1);
      }
    }

    template <FixedName Key, FixedName... Keys>
    SBIO_HD constexpr hd_std::size_t get_key_index() {
      return get_key_index_impl<Key, Keys...>();
    }
  } // namespace impl

  /**
   * @brief A wrapping container for a set of named keys.
   *
   * Series of FixedNames are used as the "keyword" names for data-class parameters.
   * A set of these can be defined using the NamedKeys. Underlying storage container
   * is provided by an array. These constructs are constexpr, and device safe.
   *
   * @tparam Keys The set of FixedName that serve as keywords.
   */
  template <FixedName... Keys>
  struct NamedKeys {
    static constexpr hd_std::size_t count { sizeof...(Keys) };

    static constexpr hd_std::array<FixedName<>, count> keys = { Keys... };

    template <FixedName Key>
    SBIO_HD static constexpr bool contains() {
      return ( (Keys == Key) || ... );
    }

    template <FixedName Key>
    SBIO_HD static constexpr std::size_t index_of() {
      return impl::get_key_index<Key, Keys...>();
    }
  };

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
  template <FixedName Key>
  struct NamedArg {
    static constexpr auto key_name { Key };

    const char* value { nullptr };
  };

  // --- Concept helpers for argument/key conversion --- //
  template <typename T>
  concept IsStringLike =
    requires(T str) { str.c_str(); } || hd_std::is_convertible_v<T, const char*>;

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


  // --- Named pointer to members --- //
  /**
   * @brief A small wrapper to attach a name to a member attribute.
   *
   * While the system of NamedKeys and NamedArgs works will for certain keyword
   * arguments, the types of values that they can support are limited. The
   * NamedMember adds a mechanism to attach a keyword to any member of a class
   * or struct. A container of these NamedMembers can then be used to iterate
   * over the member fields with some sort of semantic checks.
   *
   * @tparam Class The type of the overall class/struct.
   * @tparam T The type of the member attribute being named.
   */
  template <typename Class, typename T>
  struct NamedMember {
    const char* name;  ///< Provided parameter name
    T Class::* ptr;    ///< Pointer to member where the parameter value is stored
  };

  /**
   * @brief Construct a NamedMember.
   *
   * @returns A NamedMember.
   */
  template <typename Class, typename T>
  SBIO_HD SBIO_API constexpr NamedMember<Class, T>
  make_named_member(const char* name, T Class::* ptr) {
    return { name, ptr };
  }

} // namespace sbio

#endif // SBIO_UTIL_PARAMETERS_HH
