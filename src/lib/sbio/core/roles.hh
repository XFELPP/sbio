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

#ifndef SBIO_CORE_ROLES_HH
#define SBIO_CORE_ROLES_HH

#ifdef __CUDACC__

#include <cuda/std/concepts>
#include <cuda/std/cstddef>
#include <cuda/std/cstdint>
#include <cuda/std/type_traits>

namespace hd_std = cuda::std;

#ifndef SBIO_HD
#define SBIO_HD __host__ __device__
#endif

#else

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace hd_std = std;

#ifndef SBIO_HD
#define SBIO_HD
#endif

#endif

namespace sbio {
  namespace roles {
    namespace impl {
      struct RoleTag {};

      struct VariantTag {};
    } // namespace impl

    struct Control  : public impl::RoleTag {};

    struct Verify   : public impl::RoleTag {};

    struct Metadata : public impl::RoleTag {};

    struct Data     : public impl::RoleTag {};

    struct Index    : public impl::RoleTag {};

    struct Table    : public impl::RoleTag {};
  } // namespace roles

  template <typename T, typename U>
  concept IsSameUnderlying = hd_std::same_as<hd_std::decay_t<T>, hd_std::decay_t<U>>;

  template <typename T, typename U>
  concept IsConvertibleToUnderlying = hd_std::convertible_to<
    hd_std::decay_t<T>, hd_std::decay_t<U>
  >;

  template <typename T, typename... U>
  concept IsInList = ((IsSameUnderlying<T, U> || IsConvertibleToUnderlying<T, U>) || ... );

  template <typename... Ts>
  struct type_list {
  private:
    // helpers for doing the public indexing implementations below
    template <typename Target, hd_std::size_t Index, typename Head, typename... Tail>
    struct index_of_impl {
      static constexpr hd_std::ptrdiff_t value =
        (IsSameUnderlying<Target, Head> || IsConvertibleToUnderlying<Target, Head>)
        ? static_cast<hd_std::ptrdiff_t>(Index)
        : index_of_impl<Target, Index + 1, Tail...>::value;
    };

    template <typename Target, hd_std::size_t Index, typename Head>
    struct index_of_impl<Target, Index, Head> {
      static constexpr hd_std::ptrdiff_t value =
        (IsSameUnderlying<Target, Head> || IsConvertibleToUnderlying<Target, Head>)
        ? static_cast<hd_std::ptrdiff_t>(Index)
        : -1;
    };

  public:
    /**
     * Returns the length of the type list, i.e., the number of types.
     *
     * @returns The length of the type list.
     */
    SBIO_HD static constexpr hd_std::size_t size() { return sizeof...(Ts); }

    template <typename Target>
    static constexpr hd_std::ptrdiff_t index_of = (sizeof...(Ts) == 0)
      ? -1
      : index_of_impl<Target, 0, Ts...>::value;

    /**
     * Evaluates true if the type U is in the type list.
     */
    template <typename U>
    static constexpr bool accepts = (sizeof...(Ts) == 0) || IsInList<U, Ts...>;
  };

  template <typename R>
  concept IsRole = hd_std::is_base_of_v<roles::impl::RoleTag, R>;

  template <typename... Ts>
  concept IsRolesList = (sizeof...(Ts) > 0) && (IsRole<Ts> && ...);

  // StreamVariant inherits from type_list and VariantTag
  template <typename... Ts>
  requires IsRolesList<Ts...>
  struct StreamVariant : public type_list<Ts...>, public roles::impl::VariantTag {
  public:
    static constexpr bool has_data { StreamVariant::template accepts<roles::Data> };

  };

  template <typename V>
  concept IsStreamVariant = hd_std::is_base_of_v<roles::impl::VariantTag, V>;

  template <typename... Ts>
  concept ListOfStreamVariants = (sizeof...(Ts) > 0) && (IsStreamVariant<Ts> && ...);

  template <typename... SVs>
  requires ListOfStreamVariants<SVs...>
  struct StreamSet : public type_list<SVs...> {
  private:
    template <typename Role, hd_std::size_t Index, typename Head, typename... Tail>
    struct role_index_impl {
      static constexpr hd_std::ptrdiff_t value = Head::template accepts<Role>()
        ? Index
        : role_index_impl<Role, Index + 1, Tail...>::value;
    };

    template <typename Role, hd_std::size_t Index, typename Head>
    struct role_index_impl<Role, Index, Head> {
      static constexpr hd_std::ptrdiff_t value =
        Head::template accepts<Role>() ? static_cast<hd_std::ptrdiff_t>(Index) : -1;
    };

  public:
    template <typename Role>
    static constexpr hd_std::ptrdiff_t index_of_role = (sizeof...(SVs) == 0)
      ? -1
      : role_index_impl<Role, 0, SVs...>::value;
  };

  template <typename S>
  struct is_stream_set : hd_std::false_type {};

  template <typename... SVs>
  struct is_stream_set<StreamSet<SVs...>> : hd_std::true_type {};

  template <typename S>
  concept IsStreamSet = is_stream_set<S>::value;
} // namespace sbio
#endif // SBIO_CORE_ROLES_HH
