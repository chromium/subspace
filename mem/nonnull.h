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

#include "assertions/check.h"
#include "macros/nonnull.h"
#include "marker/unsafe.h"
#include "mem/addressof.h"
#include "mem/never_value.h"
#include "mem/relocate.h"
#include "option/option.h"

namespace sus::mem {

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
struct [[sus_trivial_abi]] NonNull {
  /// Constructs a `NonNull<T>` from a reference to `T`.
  static constexpr inline NonNull with(T& t) { return NonNull(addressof(t)); }

  /// Constructs a `NonNull<T>` from a pointer to `T`.
  ///
  /// Does not implicitly convert from an array. Explicitly convert it to a
  /// pointer to throw away the length of the array.
  ///
  /// # Panics
  /// The method will panic if the pointer `t` is null.
  template <class U>
    requires(std::is_pointer_v<U> && std::is_convertible_v<U, T*>)
  static constexpr inline ::sus::option::Option<NonNull> with_ptr(U t) {
    if (t) [[likely]]
      return ::sus::option::Option<NonNull<T>>::some(NonNull(t));
    else
      return ::sus::option::Option<NonNull<T>>::none();
  }

  template <class U, size_t N>
  static constexpr inline ::sus::option::Option<NonNull> with_ptr(U (&t)[N]) =
      delete;

  /// Constructs a `NonNull<T>` from a pointer to `T`.
  ///
  /// Does not implicitly convert from an array. Explicitly convert it to a
  /// pointer to throw away the length of the array.
  ///
  /// # Safety
  /// This method must not be called with a null pointer, or Undefined Behaviour
  /// results.
  template <class U>
    requires(std::is_pointer_v<U> && std::is_convertible_v<U, T*>)
  static constexpr inline NonNull
      with_ptr_unchecked(::sus::marker::UnsafeFnMarker,
                         sus_assertions_nonnull_arg U t)
          sus_assertions_nonnull_fn {
    return NonNull(t);
  }

  template <class U, size_t N>
  static constexpr inline NonNull with_ptr_unchecked(U (&t)[N]) = delete;

  /// sus::construct::From<NonNull<T>, T&> and
  /// sus::construct::From<NonNull<const T>, T&> traits.
  static constexpr inline NonNull from(T& t) { return with(t); }

  /// sus::construct::From<NonNull<T>, T*> and
  /// sus::construct::From<NonNull<const T>, T*> traits.
  ///
  /// Does not implicitly convert from an array. Explicitly convert it to a
  /// pointer to throw away the length of the array.
  ///
  /// # Panics
  /// The method will panic if the pointer `t` is null.
  template <class U>
    requires(std::is_pointer_v<U> && std::is_convertible_v<U, T*>)
  static constexpr inline NonNull from(U t) {
    check(t);
    return NonNull(t);
  }

  template <class U, size_t N>
  static constexpr inline NonNull from(U (&t)[N]) = delete;

  /// NonNull<T> is copyable, so this is the copy constructor.
  NonNull(const NonNull&) = default;
  /// NonNull<T> is copyable, so this is the copy assignment operator.
  NonNull& operator=(const NonNull&) = default;

  /// Returns a const reference to the pointee.
  constexpr inline const T& as_ref() const { return *ptr_; }

  /// Returns a mutable reference to the pointee.
  ///
  /// This method is only callable when the pointer is not const.
  constexpr inline T& as_mut()
    requires(!std::is_const_v<T>)
  {
    return *ptr_;
  }

  /// Returns a const pointer to the pointee.
  constexpr inline const T* as_ptr() const { return ptr_; }

  /// Returns a mutable pointer to the pointee.
  ///
  /// This method is only callable when the pointer is not const.
  constexpr inline T* as_ptr_mut()
    requires(!std::is_const_v<T>)
  {
    return ptr_;
  }

  /// Cast the pointer of type `T` in NonNull<T> to a pointer of type `U` and
  /// return a `NonNull<U>`.
  ///
  /// This requires that `T*` is implicitly convertible to `U*`. To perform a
  /// static_cast<U*>, use `downcast()`.
  template <class U>
    requires(std::is_convertible_v<T*, U*>)
  NonNull<U> cast() const {
    return NonNull<U>::with_ptr_unchecked(unsafe_fn, static_cast<U*>(ptr_));
  }

  /// Cast the pointer of type `T` in NonNull<T> to a pointer of type `U` and
  /// return a `NonNull<U>`.
  ///
  /// # Safety
  /// The pointee must be a `U*` or this results in Undefined Behaviour.
  template <class U>
  NonNull<U> downcast(::sus::marker::UnsafeFnMarker) const {
    return NonNull<U>::with_ptr_unchecked(unsafe_fn, static_cast<U*>(ptr_));
  }

 private:
  explicit constexpr inline NonNull(sus_assertions_nonnull_arg T* t)
      sus_assertions_nonnull_fn : ptr_(t) {}

  T* ptr_;

  // Declare that this type can always be trivially relocated for library
  // optimizations.
  sus_class_trivial_relocatable(unsafe_fn);
  // Declare that the `ptr_` field is never set to `nullptr` for library
  // optimizations.
  sus_class_never_value_field(unsafe_fn, NonNull, ptr_, nullptr);
};

}  // namespace sus::mem
