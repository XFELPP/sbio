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

#ifndef SBIO_UTIL_RC_HH
#define SBIO_UTIL_RC_HH

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
#include <utility>

namespace sbio {
  // --- Shared Pointer-Lite -- Very Rudimentary Reference Counter if Needed --- //
  // Tag dispatchers for overload variants with/without custom allocators/deleters
  /**
   * A small struct for tag dispatching a custom-allocator RC creation function.
   */
  struct use_rc_alloc_t {};
  /**
   * A small struct for tag dispatching a custom-deleter RC creation function.
   */
  struct use_rc_del_t {};
  /**
   * A small struct for tag dispatching a custom-allocator/deleter RC creation function.
   */
  struct use_rc_alloc_del_t {};

  /**
   * Tag to provide for custom allocator RC creation.
   */
  inline constexpr use_rc_alloc_t use_rc_alloc {};
  /**
   * Tag to provide for custom deleter RC creation.
   */
  inline constexpr use_rc_del_t use_rc_del {};
  /**
   * Tag to provide for custom allocator and deleter RC creation.
   */
  inline constexpr use_rc_alloc_del_t use_rc_alloc_del {};

  /**
   * Concept determining a valid RC Deleter.
   *
   * A Deleter must be callable with the pointer to T (to be freed) as the argument.
   *
   * @tparam D The Deleter type.
   * @tparam T The type the Deleter must be able to free.
   */
  template <typename D, typename T>
  concept RCDeleter = requires(D d, T* ptr) {
    { d(ptr) } -> std::same_as<void>;
  };

  /**
   * Concept determining a valid RC Allocator.
   *
   * An Allocator provides a static allocate function that constructs a type T.
   * It must be constructed in place at a provided destination, but beyond this
   * there are no restrictions. It will likely require accepting any arguments
   * T may expect for its constructors.
   *
   * @tparam A The Allocator type.
   * @tparam T The type the Allocator must create.
   * @tparam Args... Additional arguments for construction of T specifically.
   */
  template <typename A, typename T, typename... Args>
  concept RCAllocator = requires(std::size_t size, void* dest, Args&&... args) {
    { A::allocate(size, dest, std::forward<Args>(args)...) } -> std::same_as<void>;
  };

  /**
   * A simple reference counted container for an object.
   *
   * The RC class provides shared_ptr like semantics for a wrapped object. It is
   * provided as fully-device-friendly reference counted container which is not
   * necessarily guaranteed if using the STL shared_ptr. If you are positive that
   * your wrapped object will only ever be used from the host, you should use the
   * STL instead!
   *
   * By default the RC will allocate with new and free with delete. A custom
   * allocator and deleter may be provided as needed for the specific use-case.
   * Generally allocation and freeing will occur on the host-only, although the
   * object may be moved later onto device - this is dependent on the provided
   * allocator and deleter though.
   *
   * Copies of an RC container lead to increases in the reference count, while
   * moves do not. The moved-from object will be NULL'd.
   *
   * When a reference count reaches zero, on destruction of the final RC, the
   * wrapped object will be destructed.
   *
   * @tparam T The type of the wrapped object to be reference counted.
   * @tparam Allocator The type of the custom allocator if provided.
   * @tparam Deleter The type of the custom deleter if provided.
   */
  template <typename T, class Allocator = void, class Deleter = void>
  struct RC {
    /**
     * The control struct used by the RC for wrapping an object and tracking ref count.
     */
    struct RCObject {
      std::size_t ref_count;                       ///< The reference count of a wrapped object.
      T object;                                    ///< The wrapped object to be reference counted.
#ifdef _WIN32
      [[msvc::no_unique_address]] Deleter deleter; ///< Custom deleter if provided.
#else
      [[no_unique_address]] Deleter deleter;       ///< Custom deleter if provided.
#endif
    };

    /**
     * An RC can be default constructed and filled in (e.g. by a static func.)
     */
    SBIO_HD RC() = default;

    /**
     * Explicitly construct an RC from a prepared RCObject control struct.
     *
     * @param obj The control struct with wrapped object and ref count.
     */
    SBIO_HD explicit RC(RCObject* obj)
      : m_obj(obj)
    {}

    /**
     * Destruct the RC wrapper.
     *
     * If the RC contains an object, it will decrement the reference count to that
     * object. If the count reaches 0, then either the custom deleter will be used
     * on the wrapped object, or if one was not provided, then the standard delete
     * will be called.
     */
    SBIO_HD ~RC() {
      if (m_obj) {
        dec_reference();

        if (m_obj->ref_count == 0) {
          if constexpr (!std::is_void_v<Deleter>) {
            m_obj->deleter(&(m_obj->object));
            m_obj->deleter.~Deleter();
          }

          m_obj->object.~T();

          if constexpr (!std::is_void_v<Allocator>) {
            delete[] reinterpret_cast<std::uint8_t*>(m_obj);
          } else {
            delete m_obj;
          }
        }
      }
    }

    /**
     * Copy an RC-wrapped object, incrementing the reference count.
     *
     * @note If this RC previously held another object, it will have its
     *       reference count decremented.
     */
    SBIO_HD RC(const RC& other)
      : m_obj(other.m_obj)
    {
      if (m_obj) {
        inc_reference();
      }
    }

    /**
     * Copy an RC-wrapped object, incrementing the reference count.
     *
     * @note If this RC previously held another object, it will have its
     *       reference count decremented.
     */
    SBIO_HD RC& operator=(const RC& other) {
      // NOTE: Due to overloads of operator& need to use addressof
      if (this != std::addressof(other)) {
        this->~RC();

        m_obj = other.m_obj;
        if (m_obj) {
          inc_reference();
        }
      }

      return *this;
    }

    /**
     * Move an RC-wrapped object without incrementing the reference count.
     */
    SBIO_HD RC(RC&& other) noexcept
      : m_obj(other.m_obj)
    {
      other.m_obj = nullptr;
    }

    /**
     * Move an RC-wrapped object without incrementing the reference count.
     */
    SBIO_HD RC& operator=(RC&& other) noexcept {
      // NOTE: Due to overloads of operator& need to use addressof
      if (this != std::addressof(other)) {
        this->~RC();

        m_obj = other.m_obj;

        other.m_obj = nullptr;
      }

      return *this;
    }

    /**
     * Set the wrapped object's reference count to a specific value.
     *
     * @param count The new reference count to use.
     */
    SBIO_HD void set_count(std::size_t count) { m_obj->ref_count = count; }
    /**
     * Increment the wrapped object's reference count.
     */
    SBIO_HD void inc_reference() { m_obj->ref_count++; }
    /**
     * Decrement the wrapped object's reference count.
     */
    SBIO_HD void dec_reference() { m_obj->ref_count--; }

    /**
     * Overloaded dereference operator returns the wrapped object.
     *
     * @returns A reference to the wrapped object.
     */
    SBIO_HD T& operator*() { return m_obj->object; }
    /**
     * Overloaded dereference operator returns the wrapped object.
     *
     * @returns A const reference to the wrapped object.
     */
    SBIO_HD const T& operator*() const { return m_obj->object; }
    /**
     * Overloaded addressof operator returns a pointer to the wrapped object.
     *
     * @returns Pointer to the wrapped object.
     */
    SBIO_HD T* operator&() { return &(m_obj->object); }
    /**
     * Overloaded addressof operator returns a pointer to the wrapped object.
     *
     * @returns Const pointer to the wrapped object.
     */
    SBIO_HD const T* operator&() const { return &(m_obj->object); }

    /**
     * Create a new RC without custom allocator or deleter.
     *
     * @tparam Args... The type of the arguments for the wrapped type's constructor.
     * @param args... The arguments for the wrapped type's constructor.
     * @returns A new reference counted RC wrapper.
     */
    template <typename... Args>
    SBIO_HD static RC<T, Allocator, Deleter> make_rc(Args&&... args) {
      using RCT = RC<T, Allocator, Deleter>;
      using RCObjT = typename RCT::RCObject;

      RCObjT* obj = new RCObjT { 1, T(std::forward<Args>(args)...) };

      return RCT(obj);
    }

    /**
     * Create a new RC with a custom allocator but not deleter.
     *
     * @tparam Alloc The type of the custom Allocator class.
     * @tparam Args... The type of the arguments for the wrapped type's constructor.
     * @param use_rc_alloc_t Dispatch tag for using a custom allocator.
     * @param alloc The custom allocator.
     * @param args... The arguments for the wrapped type's constructor.
     * @returns A new reference counted RC wrapper.
     */
    template <typename Alloc, typename... Args>
    requires RCAllocator<Alloc, T, Args...>
    SBIO_HD static RC<T, Alloc, Deleter> make_rc(use_rc_alloc_t,
                                                 Alloc alloc,
                                                 Args&&... args) {
      using RCT = RC<T, Alloc, Deleter>;
      using RCObjT = typename RCT::RCObject;

      RCObjT* obj { nullptr };

      std::size_t bytes { sizeof(RCObjT) };
      std::uint8_t* buf { new std::uint8_t[bytes] };

      Alloc::allocate(sizeof(T),
                      buf + sizeof(std::size_t),
                      std::forward<Args>(args)...);
      obj = reinterpret_cast<RCObjT*>(buf);
      obj->ref_count = 1;

      return RCT(obj);
    }

    /**
     * Create a new RC with a custom deleter but not allocator.
     *
     * @tparam Del The type of the custom Deleter class.
     * @tparam Args... The type of the arguments for the wrapped type's constructor.
     * @param use_rc_del_t Dispatch tag for using a custom deleter.
     * @param del The custom deleter.
     * @param args... The arguments for the wrapped type's constructor.
     * @returns A new reference counted RC wrapper.
     */
    template <typename Del, typename... Args>
    requires RCDeleter<Del, T>
    SBIO_HD static RC<T, Allocator, Del> make_rc(use_rc_del_t,
                                                 Del del,
                                                 Args&&... args) {
      using RCT = RC<T, Allocator, Del>;
      using RCObjT = typename RCT::RCObject;

      RCObjT* obj { nullptr };

      obj = new RCObjT { 1, T(std::forward<Args>(args)...), del };

      return RCT(obj);
    }

    /**
     * Create a new RC with a custom allocator and deleter.
     *
     * @tparam Alloc The type of the custom Allocator class.
     * @tparam Del The type of the custom Deleter class.
     * @tparam Args... The type of the arguments for the wrapped type's constructor.
     * @param use_rc_alloc_del_t Dispatch tag for using a custom allocator and deleter.
     * @param alloc The custom allocator.
     * @param del The custom deleter.
     * @param args... The arguments for the wrapped type's constructor.
     * @returns A new reference counted RC wrapper.
     */
    template <typename Alloc, typename Del, typename... Args>
    requires RCAllocator<Alloc, T, Args...> && RCDeleter<Del, T>
    SBIO_HD static RC<T, Alloc, Del> make_rc(use_rc_alloc_del_t,
                                             Alloc alloc,
                                             Del del,
                                             Args&&... args) {
      using RCT = RC<T, Alloc, Del>;
      using RCObjT = typename RCT::RCObject;

      RCObjT* obj { nullptr };

      std::size_t bytes { sizeof(RCObjT) };
      std::uint8_t* buf { new std::uint8_t[bytes] };

      Alloc::allocate(sizeof(T),
                      buf + sizeof(std::size_t),
                      std::forward<Args>(args)...);
      obj = reinterpret_cast<RCObjT*>(buf);
      obj->ref_count = 1;

      new (&obj->deleter) Del(del);

      return RCT(obj);
    }

  private:
    RCObject* m_obj { nullptr }; ///< Wrapped container for the ref count and object
  };
} // namespace sbio
#endif // SBIO_UTIL_RC_HH
