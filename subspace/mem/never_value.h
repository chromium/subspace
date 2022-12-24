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

#include "macros/always_inline.h"
#include "marker/unsafe.h"

namespace sus::mem {

namespace __private {

template <class T, bool HasField>
struct never_value_access_helper;

template <class T>
struct never_value_access_helper<T, false> {
  static constexpr bool has_field = false;
  using OverlayType = char;

  static constexpr bool is_constructed(const OverlayType&) noexcept {
    return false;
  }
  static constexpr void set_never_value(OverlayType&) noexcept {}
};

template <class T>
struct never_value_access_helper<T, true> {
  static constexpr bool has_field = true;
  using OverlayType = T::template SusUnsafeNeverValueOverlay<T>::type;

  static sus_always_inline constexpr bool is_constructed(
      const OverlayType& t) noexcept {
    return t.SusUnsafeNeverValueIsConstructed();
  }
  static sus_always_inline constexpr void set_never_value(
      OverlayType& t) noexcept {
    t.SusUnsafeNeverValueSetNeverValue();
  }
};

template <class NeverType, NeverType never_value, class FieldType, size_t N>
struct SusUnsafeNeverValueOverlayImpl;

template <class NeverType, NeverType never_value, class FieldType, size_t N>
struct SusUnsafeNeverValueOverlayImpl {
  char padding[N];
  FieldType never_value_field;
  constexpr inline void SusUnsafeNeverValueSetNeverValue() noexcept {
    never_value_field = never_value;
  }
  constexpr inline bool SusUnsafeNeverValueIsConstructed() const noexcept {
    return never_value_field != never_value;
  }
};

template <class NeverType, NeverType never_value, class FieldType>
struct SusUnsafeNeverValueOverlayImpl<NeverType, never_value, FieldType, 0> {
  FieldType never_value_field;
  constexpr inline void SusUnsafeNeverValueSetNeverValue() noexcept {
    never_value_field = never_value;
  }
  constexpr inline bool SusUnsafeNeverValueIsConstructed() const noexcept {
    return never_value_field != never_value;
  }
};

}  // namespace __private

/// A trait to inspect if a type `T` has a field with a never-value. For such a
/// type, it is possible to tell if the type is constructed in a memory
/// location, by storing the never-value through `set_never_value()` in the
/// memory location before it is constructed and/or after it is destroyed.
///
/// This allows a flag to check for a class being constructed without an
/// additional boolean flag.
template <class T>
struct never_value_access {
  /// Whether the type `T` has a never-value field.
  static constexpr bool has_field =
      requires { T::template SusUnsafeNeverValueOverlay<T>::exists; };

  /// A type with a common initial sequence with `T` up to and including the
  /// never-value field, so that it can be used to read and write the
  /// never-value field in a union (though reading an inactive union field is
  /// invalid in a constant expression in C++20).
  using OverlayType =
      typename __private::never_value_access_helper<T, has_field>::OverlayType;

  /// Returns whether there is a type `T` constructed at the memory location
  /// `t`, where the OverlayType `t` has the same address as a type `T` in a
  /// union.
  ///
  /// # Safety
  /// This will only produce a correct answer if the memory was previous set to
  /// the never-value through `set_never_value()` before construction of the
  /// type `T`.
  static constexpr sus_always_inline bool is_constructed(
      ::sus::marker::UnsafeFnMarker, const OverlayType& t) noexcept
    requires(has_field)
  {
    return __private::never_value_access_helper<T, has_field>::is_constructed(
        t);
  }
  /// Sets a field in the memory location `t` to a value that is never set
  /// during the lifetime of `T`, where the OverlayType `t` has the same address
  /// as a type `T` in a union.
  ///
  /// # Safety
  /// This must never be called while there is an object of type `T` constructed
  /// at the given memory location. It must be called only before a constructor
  /// is run, or after a destructor is run.
  static constexpr sus_always_inline void set_never_value(
      ::sus::marker::UnsafeFnMarker, OverlayType& t) noexcept
    requires(has_field)
  {
    return __private::never_value_access_helper<T, has_field>::set_never_value(
        t);
  }
};

/// A `NeverValueField` type has a field with a never-value.
///
/// When not constructed, that field in a `NeverValueField` object can be set to
/// the never-value in order to see if/when the object is constructed, as the
/// never-value would be changed by construction, and would never occur during
/// the lifetime of the object.
template <class T>
concept NeverValueField = never_value_access<T>::has_field;

}  // namespace sus::mem

/// Mark a class field as never being a specific value, often a zero, after a
/// constructor has run and bef ore the destructor has completed. This allows
/// querying if a class is constructed in a memory location, since the class is
/// constructed iff the value of the field is not the never-value.
///
/// This has no effect if the type `T` is not a standard-layout type, as the
/// field is only accessible in that case. Therefore it is advised to verify
/// `NeverValueField<T>` where you expect it to be true.
///
/// The macro includes `private:` which changes the class definition visiblity
/// to private.
#define sus_class_never_value_field(unsafe_fn, T, field_name, never_value)     \
 private:                                                                      \
  static_assert(                                                               \
      std::same_as<decltype(unsafe_fn), const ::sus::marker::UnsafeFnMarker>); \
  static_assert(                                                               \
      std::is_assignable_v<decltype(field_name)&, decltype(never_value)>,      \
      "The `never_value` must be able to be assigned to the named field.");    \
                                                                               \
  /* For the inclined, this is because otherwise using the Overlay type in     \
  the Option's internal union, after the destruction of the type T, would      \
  require placement new of the Overlay type which is not a constant            \
  expression. */                                                               \
  static_assert(std::is_trivially_constructible_v<decltype(field_name)>,       \
                "The `never_value` field must be trivially constructible or "  \
                "else Option<T> couldn't be constexpr.");                      \
                                                                               \
  template <class>                                                             \
  friend struct ::sus::mem::never_value_access;                                \
  template <class, bool>                                                       \
  friend struct ::sus::mem::__private::never_value_access_helper;              \
                                                                               \
  template <class SusUnsafeNeverValueOuter,                                    \
            bool SusUnsafeNeverValueStandardLayout =                           \
                std::is_standard_layout_v<SusUnsafeNeverValueOuter>>           \
  struct SusUnsafeNeverValueOverlay;                                           \
                                                                               \
  template <class SusUnsafeNeverValueOuter>                                    \
  struct SusUnsafeNeverValueOverlay<SusUnsafeNeverValueOuter, false> {};       \
                                                                               \
  /* The NeverValue trait is only provided when `##T##` is a standard-layout   \
   * type, since:                                                              \
   * - offsetof() is only valid in a standard-layout type.                     \
   * - Option can only access the never-value field outside the lifetime of    \
   *   `##T##` if `##T##` is a standard-layout type.                           \
   */                                                                          \
  template <class SusUnsafeNeverValueOuter>                                    \
  struct SusUnsafeNeverValueOverlay<SusUnsafeNeverValueOuter, true> {          \
    static constexpr bool exists = true;                                       \
                                                                               \
    using type = ::sus::mem::__private::SusUnsafeNeverValueOverlayImpl<        \
        decltype(never_value), never_value, decltype(field_name),              \
        offsetof(SusUnsafeNeverValueOuter, field_name)>;                       \
  };                                                                           \
  static_assert(true)
