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

#ifndef SBIO_CORE_STORAGE_HH
#define SBIO_CORE_STORAGE_HH

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
  // --- TypeList helper: Used by FormatTraits to specify buffer requirements --- //
  template <typename... Ts>
  struct TypeList {
    static constexpr hd_std::size_t size = sizeof...(Ts);
  };

  template <typename T>
  struct is_type_list : hd_std::false_type {};

  template <typename... Ts>
  struct is_type_list<TypeList<Ts...>> : hd_std::true_type {};

  template <typename T>
  static constexpr bool is_type_list_v = is_type_list<T>::value;

  template <typename List>
  concept IsTypeList = is_type_list_v<List>;

  // --- Tags for roles that a buffer may play. --- //
  struct MetadataRole {};
  struct DataRole {};
  struct IndexRole {};
  struct CalibrationRole {};
  struct GroupRole {};
  struct TableRole {};

  // --- Tags for additional semantic hints as to what a buffer can do --- //
  struct Shareable {};      // Optimization: E.g., visible to MPI peers
  struct DeviceBound {};    // Requirement: E.g., only accessible to CUDA/HIP
  struct DoubleBuffered {}; // Strategy: E.g. requires ping-pong buffers

  /**
   * The MemorySpace an individual buffer lives in.
   *
   * Each buffer satisfying ValidBuffer (below) indicates the memory space that
   * it resides in. This allows querying, and making decisions if/when memory may
   * need to be copied or moved into different spaces.
   */
  enum class MemorySpace : hd_std::uint8_t {
    Host    = 0, ///< Resides in host memory
    Pinned  = 1, ///<
    Device  = 2, ///< Resides in device (GPU) memory
    Managed = 3  ///<
  };

  /**
   * Operating through StorageView's acquisition model, specify MemorySpace to use.
   *
   * A caller can specify a view of a buffer must be returned in the caller's native
   * memory space (GPU/CPU etc), or alternatively, that it can be left in the buffer's
   * native space. The latter option may at first seem not so useful (why would a caller
   * request access to a buffer it cannot read?) - however, callers in the sbio
   * infrastructure are often *orchestrating* other components. They can specify
   * acquisition, initiating a transaction, and proceed to direct how the buffer be
   * filled/read elsewhere, without actually reading it themselves. This allows for
   * major performance improvements where the alternative would require copies
   * back and forth between CPU and GPU memory (as an example).
   */
  enum class AcquireIntent : hd_std::uint8_t {
    CallerMemorySpace = 0, ///< On acquire, view should be in the caller's space
    BufferMemorySpace = 1  ///< On acquire, view can be left in the buffer's space
  };

  template <typename FTraits>
  struct AllocationRequest {
    // One entry for every descriptor in the BrokerBufferRequirements TypeList
    hd_std::size_t size_requests[FTraits::BrokerBufferRequirements::size] { 0 };
  };

  template <typename Role, typename BufferT>
  struct BufferMember {
    BufferT buffer;
  };

  template <
    typename Role,
    hd_std::size_t Id = 0,
    hd_std::size_t MinSize = 0,
    typename Hint = void
  > // Pass a tag above
  struct BufferDescriptor {
    using role = Role;
    using hint = Hint;
    static constexpr hd_std::size_t id = Id;
    static constexpr hd_std::size_t min_size = MinSize;
  };

  template <typename T>
  concept ValidBuffer = requires(T buf) {
    { buf.ptr() } -> hd_std::same_as<void*>;
    { buf.size() } -> hd_std::convertible_to<hd_std::size_t>;
    { buf.set_memory(hd_std::declval<void*>(), hd_std::declval<hd_std::size_t>()) };
    { T::memory_space() } -> hd_std::convertible_to<MemorySpace>;
  };

  // --- All buffers are combined together into a single Storage object --- //

  // Helpers to parse the buffer descriptors
  template <typename T>
  struct IsDescriptor : hd_std::false_type {};

  template <typename Role, hd_std::size_t Id, hd_std::size_t Size, typename H>
  struct IsDescriptor<BufferDescriptor<Role, Id, Size, H>> : hd_std::true_type {};

  template <typename Role, hd_std::size_t Id, typename List>
  struct FindDescriptor;

  template <typename Role, hd_std::size_t Id, typename Head, typename... Tail>
  struct FindDescriptor<Role, Id, TypeList<Head, Tail...>> {
    using type = hd_std::conditional_t<
      hd_std::is_same_v<typename Head::role, Role> && (Head::id == Id),
      Head,
      typename FindDescriptor<Role, Id, TypeList<Tail...>>::type
    >;
  };

  template <typename Role, hd_std::size_t Id>
  struct FindDescriptor<Role, Id, TypeList<>> {
    using type = void;
  };

  template <typename T>
  struct GetHint {
    using type = void;
  };

  template <typename Role, hd_std::size_t Id, hd_std::size_t MinSize, typename Hint>
  struct GetHint<BufferDescriptor<Role, Id, MinSize, Hint>> {
    using type = Hint;
  };

  template <typename List, typename Policy>
  struct Storage;

  template <typename... Roles, typename Policy>
  struct Storage<TypeList<Roles...>, Policy>
    : public BufferMember<Roles, typename Policy::template BufferTypeFor<Roles>>... {

    static_assert((ValidBuffer<typename Policy::template BufferTypeFor<Roles>> && ...),
                  "Execution Policy must provide types that satisfy the ValidBuffer concept!");
    /**
     * Compile-time buffer accessor.
     */
    template <typename RoleOrDescriptor, hd_std::size_t Id = 0>
    SBIO_HD auto& get() {
      using Descriptor = hd_std::conditional_t<
        IsDescriptor<RoleOrDescriptor>::value,
        RoleOrDescriptor,
        typename FindDescriptor<RoleOrDescriptor, Id, TypeList<Roles...>>::type
      >;

      using BufferT = typename Policy::template BufferTypeFor<Descriptor>;
      return static_cast<BufferMember<Descriptor, BufferT>*>(this)->buffer;
    }

    /**
     * An accessor that will iterate over members of a certain role.
     * This allows a caller to provide a function to be applied to all buffers of
     * that role, without having to know the number, or to iterate them itself.
     */
    template <class Role, class CBType>
    SBIO_HD void for_each_role(CBType&& callback) {
      auto& self = *this;

      // For the Roles pack, on expansion, if the individual Role matches the Role
      // provided to this function, run the callback. Otherwise, do nothing.
      auto run_callback = [&](auto DescTag) {
        using Descriptor = typename decltype(DescTag)::type;

        using BufRole = typename Descriptor::role;
        if constexpr (hd_std::is_same_v<BufRole, Role>) {
          auto& buf { self.template get<Descriptor>() };

          callback(buf);
        }
      };

      ( (run_callback(hd_std::type_identity<Roles>{})), ... );
    }
  };

  template <typename T>
  struct ExtractDescriptorsList;

  template <typename List, typename Policy>
  struct ExtractDescriptorsList<Storage<List, Policy>> {
    using type = List;
  };

  template <typename T>
  using ExtractDescriptorsListT = typename ExtractDescriptorsList<T>::type;

} // namespace sbio
#endif // SBIO_CORE_STORAGE_HH
