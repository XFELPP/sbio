#ifndef SBIO_CORE_STORAGE_HH
#define SBIO_CORE_STORAGE_HH

#ifndef SBIO_HD
#ifdef __CUDACC__
#define SBIO_HD __host__ __device__
#else
#define SBIO_HD
#endif
#endif

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace sbio {
  // --- TypeList helper: Used by FormatTraits to specify buffer requirements --- //
  template <typename... Ts>
  struct TypeList {
    static constexpr std::size_t size = sizeof...(Ts);
  };

  // --- Tags for roles that a buffer may play. --- //
  struct MetadataRole {};
  struct DataRole {};
  struct IndexRole {};
  struct CalibrationRole {};

  // --- Tags for additional semantic hints as to what a buffer can do --- //
  struct Shareable {};      // Optimization: E.g., visible to MPI peers
  struct DeviceBound {};    // Requirement: E.g., only accessible to CUDA/HIP
  struct DoubleBuffered {}; // Strategy: E.g. requires ping-pong buffers

  template <typename FTraits>
  struct AllocationRequest {
    // One entry for every descriptor in the BufferRequirements TypeList
    std::size_t size_requests[FTraits::BufferRequirements::size] { 0 };
  };

  template <typename Role, typename BufferT>
  struct BufferMember {
    BufferT buffer;
  };

  template <
    typename Role,
    std::size_t Id = 0,
    std::size_t MinSize = 0,
    typename Hint = void
  > // Pass a tag above
  struct BufferDescriptor {
    using role = Role;
    static constexpr std::size_t id = Id;
    static constexpr std::size_t min_size = MinSize;
  };

  // --- Specific types of buffers --- //

  struct HostBuffer {
    SBIO_HD inline void* ptr() const {
      return m_ptr;
    }

    SBIO_HD std::size_t size() const {
      return m_size;
    }

    SBIO_HD inline void set_memory(void* ptr, std::size_t size) {
      m_ptr = ptr;
      m_size = size;
    }

  private:
    void* m_ptr;
    std::size_t m_size;
  };

  template <typename T>
  concept ValidBuffer = requires(T buf) {
    { buf.ptr() } -> std::same_as<void*>;
    { buf.size() } -> std::convertible_to<std::size_t>;
    { buf.set_memory(std::declval<void*>(), std::declval<std::size_t>()) };
  };

  // --- All buffers are combined together into a single Storage object --- //

  // Helpers to parse the buffer descriptors
  template <typename T>
  struct IsDescriptor : std::false_type {};

  template <typename Role, std::size_t Id, std::size_t Size, typename H>
  struct IsDescriptor<BufferDescriptor<Role, Id, Size, H>> : std::true_type {};

  template <typename Role, std::size_t Id, typename List>
  struct FindDescriptor;

  template <typename Role, std::size_t Id, typename Head, typename... Tail>
  struct FindDescriptor<Role, Id, TypeList<Head, Tail...>> {
    using type = std::conditional_t<
      std::is_same_v<typename Head::role, Role> && (Head::id == Id),
      Head,
      typename FindDescriptor<Role, Id, TypeList<Tail...>>::type
    >;
  };

  template <typename Role, std::size_t Id>
  struct FindDescriptor<Role, Id, TypeList<>> {
    using type = void;
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
    template <typename RoleOrDescriptor, std::size_t Id = 0>
    SBIO_HD auto& get() {
      using Descriptor =
        std::conditional_t<IsDescriptor<RoleOrDescriptor>::value,
                           RoleOrDescriptor,
                           typename FindDescriptor<RoleOrDescriptor, Id, TypeList<Roles...>>::type>;

      using BufferT = typename Policy::template BufferTypeFor<Descriptor>;
      return static_cast<BufferMember<Descriptor, BufferT>*>(this)->buffer;
    }

    /**
     * An accessor that will iterate over members of a certain role.
     * This allows a caller to provide a function to be applied to all buffers of
     * that role, without having to know the number, or to iterate them itself.
     */
    template <class Role, class CBType>
    SBIO_HD void for_each_role(this auto&& self, CBType&& callback) {
      // For the Roles pack, on expansion, if the individual Role matches the Role
      // provided to this function, run the callback. Otherwise, do nothing.
      ( (std::is_same_v<class Roles::role, Role> ? callback(self.template get<Roles>()) : void()), ... );
    }
  };
} // namespace sbio
#endif // SBIO_CORE_STORAGE_HH
