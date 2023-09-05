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

#include <stddef.h>

#include <concepts>
#include <type_traits>

#include "sus/macros/inline.h"
#include "sus/macros/pure.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/forward.h"
#include "sus/mem/relocate.h"
#include "sus/ops/eq.h"

namespace sus::mem {

struct NeverValueConstructor {};

namespace __private {

/// A helper class that constructs and holds a NeverValueField type T.
///
/// Default-constructing NeverValueAccess will construct T with the never value
/// set.
///
/// The other constructors allow constructing the T from a parameter (typically
/// a const T& or T&&).
///
/// Provides methods to see if the T is in the never-value state or not, and to
/// set the never-value field to:
/// * the never-value, after trivial default construction.
/// * the destroy-value before destroying it from the never-value state.
///
/// A type which satisfies NeverValueField has a field which is never set to a
/// specific value during its lifetime under normal use. The type provides a
/// private way construct it with that never-value in the field as a signal to
/// say the type is not really constructed/present.
///
/// The destructor must be a no-op when being destroyed from the never-value
/// state. To help make that easier, a separate "destroy value" can be specified
/// which the never-value field is set to before calling the destructor so that
/// the destructor does not need to handle the never-value.
///
/// # Safety
///
/// To implement NeverValueAccess, a type must:
/// * Insert a call to the sus_class_never_value_field() macro inside its body,
///   which takes as parameters the NeverValue configuration of the type.
/// * Be (optionally-constexpr-) constructible from NeverValueConstructor, which
///   sets the NeverValue and leaves the type in a state that can be destructed
///   later as a no-op.
/// * The above constructor must be `private` to prevent incorrect access.
template <class T>
struct NeverValueAccess;

template <class T>
struct NeverValueAccess<T&> {
  static constexpr bool has_field = false;
};

template <class T>
struct NeverValueAccess {
  /// Whether the type `T` has a never-value field.
  static constexpr bool has_field = requires {
    std::declval<T&>()._sus_Unsafe_NeverValueIsConstructed(
        ::sus::marker::unsafe_fn);
  };

  constexpr NeverValueAccess() = default;

  template <class... U>
    requires(sizeof...(U) > 0u)
  constexpr NeverValueAccess(U&&... v) : t_(::sus::forward<U>(v)...) {}

  /// Checks if the never-value field is set to the never-value, returning false
  /// if it is.
  sus_pure constexpr sus_always_inline bool is_constructed() const noexcept
    requires(has_field)
  {
    return t_._sus_Unsafe_NeverValueIsConstructed(::sus::marker::unsafe_fn);
  }

  /// Sets the never-value field to the destroy-value.
  constexpr sus_always_inline void set_destroy_value(
      ::sus::marker::UnsafeFnMarker) noexcept
    requires(has_field)
  {
    t_._sus_Unsafe_NeverValueSetDestroyValue(::sus::marker::unsafe_fn);
  }

  sus_pure constexpr sus_always_inline const T& as_inner() const { return t_; }
  sus_pure constexpr sus_always_inline T& as_inner_mut() { return t_; }

 private:
  T t_{NeverValueConstructor()};

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(t_));
};

}  // namespace __private

/// A `NeverValueField` type has a field with a never-value.
///
/// Under normal use, that field in a `NeverValueField` object will never be
/// set to the never-value, which allows inspecting it to determine if the
/// object is "constructed".
///
/// Such types allow separate abnormal construction through the NeverValueField
/// machinery, where the never-value field is set to its never-value. The
/// object will not be used in that state except for calling the destructor, and
/// the field will be set to a special destroy-value before the destructor is
/// called.
template <class T>
concept NeverValueField = __private::NeverValueAccess<T>::has_field;

}  // namespace sus::mem

/// Mark a class field as never being a specific value, often a zero, after a
/// constructor has run and before the destructor has completed. This allows
/// querying if a class is constructed in a memory location, since the class is
/// constructed iff the value of the field is not the never-value.
///
/// The `never_value` will be placed in the named field after construction, and
/// the `destroy_value` will be placed in the named field just prior to
/// destruction. The latter is meant to help the destructor be a no-op when the
/// type is in a never-value state, if the never-value would be read in the
/// destructor.
///
/// The macro includes `private:` which changes the class definition visibility
/// to private.
#define sus_class_never_value_field(unsafe_fn, T, field_name, never_value,     \
                                    destroy_value)                             \
 private:                                                                      \
  static_assert(                                                               \
      std::same_as<decltype(unsafe_fn), const ::sus::marker::UnsafeFnMarker>); \
                                                                               \
  template <class>                                                             \
  friend struct ::sus::mem::__private::NeverValueAccess;                       \
                                                                               \
  sus_pure constexpr bool _sus_Unsafe_NeverValueIsConstructed(                 \
      ::sus::marker::UnsafeFnMarker) const noexcept {                          \
    static_assert(                                                             \
        std::is_assignable_v<decltype(field_name)&, decltype(never_value)>,    \
        "The `never_value` must be able to be assigned to the named field.");  \
    return !(field_name == never_value);                                       \
  }                                                                            \
  constexpr void _sus_Unsafe_NeverValueSetDestroyValue(                        \
      ::sus::marker::UnsafeFnMarker) noexcept {                                \
    static_assert(::sus::ops::Eq<decltype(field_name), decltype(never_value)>, \
                  "The `never_value` must be comparable to the named field."); \
    field_name = destroy_value;                                                \
  }                                                                            \
  static_assert(true)
