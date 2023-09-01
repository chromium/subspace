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

#include "sus/assertions/check.h"
#include "sus/macros/nonnull.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/addressof.h"
#include "sus/mem/never_value.h"
#include "sus/mem/relocate.h"
#include "sus/ops/eq.h"
#include "sus/ops/ord.h"
#include "sus/option/option.h"
#include "sus/ptr/subclass.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus::ptr {

/// A pointer wrapper which holds a never-null pointer.
///
/// A NonNull can not be implicitly created from an array, as that would throw
/// away the length information. Explicitly cast to a pointer to use NonNull
/// with an array.
///
/// The NonNull type is trivially copyable and moveable.
///
/// TODO: Make a NonNullArray type? https://godbolt.org/z/3vW3xsz5h
template <class T>
  requires(!std::is_reference_v<T>)
class [[sus_trivial_abi]] NonNull {
 public:
  /// Constructs a `NonNull<T>` from a reference to `T`.
  explicit constexpr NonNull(T& t) noexcept : ptr_(::sus::mem::addressof(t)) {}

  /// Constructs a `NonNull<T>` from a pointer to `T`.
  ///
  /// Does not implicitly convert from an array. The caller must explicitly
  /// convert it to a pointer to throw away the length of the array.
  ///
  /// # Panics
  /// The method will panic if the pointer `t` is null.
  template <::sus::ptr::SameOrSubclassOf<T*> U>
  static constexpr inline ::sus::option::Option<NonNull> with_ptr(
      U t) noexcept {
    if (t != nullptr) [[likely]]
      return ::sus::option::Option<NonNull>(NonNull(*t));
    else
      return ::sus::option::Option<NonNull>();
  }

  template <class U, size_t N>
  static constexpr inline ::sus::option::Option<NonNull> with_ptr(U (&t)[N]) =
      delete;

  /// Constructs a `NonNull<T>` from a pointer to `T`.
  ///
  /// Does not implicitly convert from an array. The caller must explicitly
  /// convert it to a pointer to throw away the length of the array.
  ///
  /// # Safety
  /// This method must not be called with a null pointer, or Undefined Behaviour
  /// results.
  template <::sus::ptr::SameOrSubclassOf<T*> U>
  static constexpr inline sus_nonnull_fn NonNull
  with_ptr_unchecked(::sus::marker::UnsafeFnMarker,
                     sus_nonnull_arg U sus_nonnull_var t) noexcept {
    return NonNull(*t);
  }

  template <class U, size_t N>
  static constexpr inline NonNull with_ptr_unchecked(U (&t)[N]) = delete;

  /// Satisfies the [`sus::construct::From<NonNull<T>,
  /// T&>`]($sus::construct::From) concept.
  static constexpr inline NonNull from(T& t) noexcept { return NonNull(t); }

  template <class U, size_t N>
  static constexpr inline NonNull from(U (&t)[N]) = delete;

  /// `NonNull<T>` is `Copy`, so this is the copy constructor.
  /// #[doc.overloads=ctor.copy]
  NonNull(const NonNull&) = default;
  /// `NonNull<T>` is `Copy`, so this is the copy assignment operator.
  /// #[doc.overloads=assign.copy]
  NonNull& operator=(const NonNull&) = default;

  /// Gives access to the object pointed to by NonNull.
  ///
  /// Mutable access is only given is NonNull is not const and the pointer
  /// within is not const.
  const T* operator->() const noexcept { return ptr_; }
  T* operator->()
    requires(!std::is_const_v<T>)
  {
    return ptr_;
  }

  /// Returns a const reference to the pointee.
  constexpr inline const T& as_ref() const noexcept { return *ptr_; }

  /// Returns a mutable reference to the pointee.
  ///
  /// This method is only callable when the pointer is not const.
  constexpr inline T& as_mut() noexcept
    requires(!std::is_const_v<T>)
  {
    return *ptr_;
  }

  /// Returns a const pointer to the pointee.
  constexpr inline const T* as_ptr() const noexcept { return ptr_; }

  /// Returns a mutable pointer to the pointee.
  ///
  /// This method is only callable when the pointer is not const.
  constexpr inline T* as_mut_ptr() noexcept
    requires(!std::is_const_v<T>)
  {
    return ptr_;
  }

  /// Cast the pointer of type `T` in `NonNull<T>` to a pointer of type `U` and
  /// return a `NonNull<U>`.
  ///
  /// This requires that `T*` is a subclass of `U*`. To perform a
  /// downcast, like static_cast<U*> allows, use `downcast()`.
  template <class U>
    requires ::sus::ptr::SameOrSubclassOf<T*, U*>
  NonNull<U> cast() const noexcept {
    return NonNull<U>::with_ptr_unchecked(::sus::marker::unsafe_fn,
                                          static_cast<U*>(ptr_));
  }

  /// Cast the pointer of type `T` in `NonNull<T>` to a pointer of type `U` and
  /// return a `NonNull<U>`.
  ///
  /// # Safety
  /// The pointee must be a `U*` or this results in Undefined Behaviour.
  template <class U>
  NonNull<U> downcast(::sus::marker::UnsafeFnMarker) const noexcept {
    return NonNull<U>::with_ptr_unchecked(::sus::marker::unsafe_fn,
                                          static_cast<U*>(ptr_));
  }

 private:
  T* ptr_;

  // Declare that this type can always be trivially relocated for library
  // optimizations.
  sus_class_trivially_relocatable_unchecked(::sus::marker::unsafe_fn);
  // Declare that the `ptr_` field is never set to `nullptr` for library
  // optimizations.
  sus_class_never_value_field(::sus::marker::unsafe_fn, NonNull, ptr_, nullptr,
                              nullptr);
  // For the NeverValueField.
  explicit constexpr NonNull(::sus::mem::NeverValueConstructor) noexcept
      : ptr_(nullptr) {}
};

/// Satisfies the [`Eq<NonNull<T>, NonNull<U>>`]($sus::ops::Eq) concept if the
/// pointers are comparable and thus satisfy `Eq` as well.
template <class T, class U>
  requires(::sus::ops::Eq<const T*, const U*>)
constexpr inline bool operator==(const NonNull<T>& l,
                                 const NonNull<U>& r) noexcept {
  return l.as_ptr() == r.as_ptr();
}

/// Satisfies the [`StrongOrd<NonNull<T>>`]($sus::ops::StrongOrd) concept if the
/// pointers are comparable and thus satisfy `StrongOrd` as well.
template <class T, class U>
  requires(::sus::ops::StrongOrd<const T*, const U*>)
constexpr inline std::strong_ordering operator<=>(
    const NonNull<T>& l, const NonNull<U>& r) noexcept {
  return l.as_ptr() <=> r.as_ptr();
}

}  // namespace sus::ptr

// fmt support.
template <class T, class Char>
struct fmt::formatter<::sus::ptr::NonNull<T>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::ptr::NonNull<T>& t,
                        FormatContext& ctx) const {
    return underlying_.format(t.as_ptr(), ctx);
  }

 private:
  formatter<void*, Char> underlying_;
};

// Stream support.
sus__format_to_stream(sus::ptr, NonNull, T);
