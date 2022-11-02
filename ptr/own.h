// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <type_traits>

#include "assertions/builtin.h"
#include "assertions/check.h"
#include "mem/forward.h"
#include "mem/relocate.h"
#include "mem/replace.h"
#include "num/float_concepts.h"
#include "num/integer_concepts.h"
#include "num/unsigned_integer.h"
#include "ptr/__private/in_use.h"

namespace sus::ptr {

namespace __private {

// Pointer is set to a value in this mask when moved from or in use through
// operator->.
static constexpr auto kInUsePtr = 0x00000001_usize;
// Compare these bits to 0 to see if pointer is not moved-from or in use.
static constexpr auto kValidMask = 0xfffffff0_usize;

}  // namespace __private

template <class T>
class [[sus_trivial_abi]] Own final {
  sus_class_trivial_relocatable_value(unsafe_fn, true);

  static_assert(!std::is_reference_v<T>,
                "own<T> must hold a concrete object and not a reference");
  static_assert(!std::is_pointer_v<std::decay_t<T>>,
                "own<T> must hold a concrete object and not a pointer");

  static constexpr bool T_is_primitive =
      ::sus::num::Integer<T> || ::sus::num::PrimitiveInteger<T> ||
      ::sus::num::FloatingPoint<T> || ::sus::num::PrimitiveFloatingPoint<T> ||
      std::is_enum_v<T>;

 public:
  template <class... Args>
  static inline Own with(Args&&... args) noexcept {
    return Own(kConstructArgs, forward<Args>(args)...);
  }

  /// Conversion from T to Own<T>.
  ///
  /// The conversion allocates on the heap and moves t from the stack into it.
  static constexpr Own from(T t) {
    // Sets `o` as moved from.
    return Own(kConstructArgs, static_cast<T&&>(t));
  }

  // Conversion from Own<T> to Own<const T>.
  static constexpr Own from(Own<std::remove_const_t<T>>&& o)
    requires(std::is_const_v<T>)
  {
    check(o.is_not_in_use_or_moved_from());
    // Sets `o` as moved from.
    return Own(kConstructPointer, ::sus::mem::replace_ptr(mref(o.t_), nullptr));
  }

  /// Constructs an `Own<T>` from a raw pointer `T*`.
  ///
  /// After calling this function, the raw pointer is owned by the resulting
  /// `Own<T>`. Specifically, the `Own<T>` destructor will call the destructor
  /// of `T` and free the allocated memory.
  ///
  /// # Safety
  ///
  /// This function is unsafe because improper use may lead to memory problems.
  /// For example, a double-free may occur if the function is called twice on
  /// the same raw pointer.
  static inline Own from_raw(::sus::marker::UnsafeFnMarker, T* raw) noexcept {
    return Own(kConstructPointer, raw);
  }

  /// Construct an `Own<T>` with the default constructor for the type `T`.
  ///
  /// The type `T` must be #MakeDefault, and will be constructed through that
  /// trait.
  static inline Own with_default() noexcept
    requires(::sus::construct::MakeDefault<T>)
  {
    return Own(kConstructDefault);
  }

  ~Own() noexcept {
    if (is_not_moved_from()) {
      check(is_not_in_use());
      delete t_;
    }
  }

  Own(const Own& o) = delete;
  Own& operator=(const Own& o) = delete;

  constexpr Own(Own&& o)
      // Sets `o` as moved from.
      : t_(::sus::mem::replace_ptr(mref(o.t_), nullptr)) {
    check(is_not_in_use_or_moved_from());
  }

  constexpr Own& operator=(Own&& o) {
    check(o.is_not_in_use_or_moved_from());
    check(is_not_in_use());
    if (is_not_moved_from()) {
      delete t_;
    }
    // Sets `o` as moved from.
    t_ = ::sus::mem::replace_ptr(mref(o.t_), nullptr);
    return *this;
  }

  constexpr auto operator*() const noexcept
    requires(T_is_primitive)
  {
    check(is_not_in_use_or_moved_from());
    return *t_;
  }

  constexpr const auto operator->() const noexcept {
#if __has_builtin(__builtin_prefetch)
    __builtin_prefetch(t_);
#endif
    check(is_not_in_use_or_moved_from());
    // `t_` will be put back and the class can't be used in the meantime. So
    // this method acts like const externally, but needs to mutate `t_` to
    // provide safety guarnatees.
    Own& this_mut = const_cast<Own<T>&>(*this);
    return __private::InUse<T>(
        *mem::replace_ptr(mref(this_mut.t_), in_use_ptr()), this_mut);
  }

  auto operator->() noexcept {
#if __has_builtin(__builtin_prefetch)
    __builtin_prefetch(t_);
#endif
    check(is_not_in_use_or_moved_from());
    return __private::InUse<T>(*mem::replace_ptr(mref(t_), in_use_ptr()),
                               *this);
  }

  void drop() && noexcept {
    check(is_not_in_use_or_moved_from());
    delete t_;
    t_ = nullptr;  // Is moved from.
  }

  // Upcasting from Own<Subclass> to Own<Baseclass>.
  template <class U>
    requires(std::is_convertible_v<T*, U*>)
  constexpr Own<U> cast_to() && noexcept {
#if __has_builtin(__builtin_prefetch)
    __builtin_prefetch(t_);
#endif
    check(is_not_in_use_or_moved_from());
    return Own<U>(Own<U>::kConstructPointer,
                  static_cast<U*>(mem::replace_ptr(mref(t_), nullptr)));
  }

  // TODO: downcast, downcast_unchecked

  Own clone() const noexcept {
    check(is_not_in_use_or_moved_from());
    return Own(kConstructCloning, *t_);
  }

  // TODO: ptr

  constexpr bool ptr_equal(const Own<T>& o) const noexcept {
    check(is_not_in_use_or_moved_from());
    check(o.is_not_in_use_or_moved_from());
    return t_ == o.t_;
  }

  /// This copies-from the stored T, not from the Own<T>. No deallocation
  /// occurs.
  constexpr T to_copy() const noexcept {
    check(is_not_in_use_or_moved_from());
    return *t_;
  }

  /// This copies into the stored T. No allocation occurs.
  template <class U>
    requires(std::is_assignable_v<T&, U>)
  constexpr void copy_from(const U& u) noexcept {
    check(is_not_in_use_or_moved_from());
    *t_ = u;
  }

  /// This moves-from the stored T, not from the Own<T>. No deallocation occurs.
  ///
  /// The Own<T> remains valid though the T inside will be in a moved-from state
  /// and should be reinitialized by copy_to() or move_to() before use.constexpr
  T to_move() noexcept {
    check(is_not_in_use_or_moved_from());
    return static_cast<T&&>(*t_);
  }

  /// This moves into the stored T. No allocation occurs.
  template <class U>
    requires(std::is_assignable_v<T&, U &&> &&
             (std::is_rvalue_reference_v<U> || !std::is_reference_v<U>) &&
             !std::is_const_v<U>)
  constexpr void move_from(U&& u) noexcept {
    check(is_not_in_use_or_moved_from());
    *t_ = static_cast<U&&>(u);
  }

  ///  Copy or move-assigns to the underlying T. No allocation occurs.
  ///
  /// Prefer copy_to() or move_to(), as this is for use from templated code
  /// which has a forwarding reference to T.
  template <class U>
    requires(std::is_assignable_v<T&, U &&>)
  constexpr void forward_from(U&& u) noexcept {
    check(is_not_in_use_or_moved_from());
    *t_ = forward<U&&>(u);
  }

  T* into_raw(::sus::marker::UnsafeFnMarker) && noexcept {
    check(is_not_in_use_or_moved_from());
    return mem::replace_ptr(mref(t_), nullptr);  // Is moved from.
  }

  const T& as_ref(::sus::marker::UnsafeFnMarker) const& noexcept {
    check(is_not_in_use_or_moved_from());
    return *t_;
  }
  const T& as_ref(::sus::marker::UnsafeFnMarker) && = delete;

  const T& as_mut(::sus::marker::UnsafeFnMarker) & noexcept {
    check(is_not_in_use_or_moved_from());
    return *t_;
  }

 private:
  friend class __private::InUse<T>;  // To remove the in-use flag when done.
  template <class U>
  friend class Own;

  enum ConstructArgs { kConstructArgs };
  template <class... Args>
  explicit inline Own(ConstructArgs, Args&&... args) noexcept
      // In use during the constructor.
      : t_(in_use_ptr()) {
    t_ = new T(forward<Args>(args)...);
  }

  enum ConstructDefault { kConstructDefault };
  explicit inline Own(ConstructDefault) noexcept
      // In use during the constructor.
      : t_(in_use_ptr()) {
    t_ = ::sus::construct::alloc_make_default<T>();
  }
  enum ConstructPointer { kConstructPointer };
  explicit inline Own(ConstructPointer, T* t) noexcept : t_(t) {}
  enum ConstructCloning { kConstructCloning };
  explicit inline Own(ConstructCloning, const T& t) noexcept
      // In use during the constructor.
      : t_(in_use_ptr()) {
    t_ = new T(t);  // TODO: Use a clone concept.
  }

  constexpr sus_always_inline bool is_not_in_use() const noexcept {
    return usize::from_ptr(unsafe_fn, t_) != __private::kInUsePtr;
  }
  constexpr sus_always_inline bool is_not_moved_from() const noexcept {
    return bool(t_);
  }
  constexpr sus_always_inline bool is_not_in_use_or_moved_from()
      const noexcept {
    return (usize::from_ptr(unsafe_fn, t_) & __private::kValidMask) != 0_usize;
  }
  static constexpr sus_always_inline T* in_use_ptr() noexcept {
    return __private::kInUsePtr.to_ptr<T*>(unsafe_fn);
  }

  /// The object owned by the Own<T>, or one of:
  /// - nullptr: Indicates the Own<T> is moved-from.
  /// - kInUsePtr: Indicates a method on T is being run.
  T* t_;
};

/// sus::num::Eq<Own<U>> trait.
template <class T, class U>
  requires(::sus::num::Eq<T, U>)
constexpr inline bool operator==(const Own<T>& l, const Own<U>& r) noexcept {
  return l.as_ref(unsafe_fn) == r.as_ref(unsafe_fn);
}

/// sus::num::Ord<Own<U>> trait.
/// sus::num::WeakOrd<Option<U>> trait.
/// sus::num::PartialOrd<Option<U>> trait.
template <class T, class U>
  requires(::sus::num::PartialOrd<T, U>)
constexpr inline auto operator<=>(const Own<T>& l, const Own<U>& r) noexcept {
  return l.as_ref(unsafe_fn) <=> r.as_ref(unsafe_fn);
}

}  // namespace sus::ptr

// Promote `ptr::Own` into the `sus` namespace.
namespace sus {
using ::sus::ptr::Own;
}
