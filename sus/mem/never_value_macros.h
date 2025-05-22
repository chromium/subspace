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

/// Mark a class field as never being a specific value, often a zero, after a
/// constructor has run and before the destructor has completed. This allows
/// querying if a class is constructed in a memory location, since the class is
/// constructed iff the value of the field is not the never-value.
///
/// The named field can be compared to the `never_value` to determine if the
/// object is constructed. The field must be set to the `destroy_value` just
/// prior to destruction. The latter is meant to help the destructor be a no-op
/// when the type is in a never-value state, if the never-value would be read in
/// the destructor.
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
  template <class>                                                             \
  friend struct ::sus::mem::__private::NeverValueChecker;                      \
                                                                               \
  _sus_pure constexpr bool _sus_Unsafe_NeverValueIsConstructed(                \
      ::sus::marker::UnsafeFnMarker) const noexcept {                          \
    static_assert(                                                             \
        std::is_assignable_v<decltype(field_name)&, decltype(never_value)>,    \
        "The `never_value` must be able to be assigned to the named field.");  \
    return !(field_name == never_value);                                       \
  }                                                                            \
  constexpr void _sus_Unsafe_NeverValueSetDestroyValue(                        \
      ::sus::marker::UnsafeFnMarker) noexcept {                                \
    static_assert(::sus::cmp::Eq<decltype(field_name), decltype(never_value)>, \
                  "The `never_value` must be comparable to the named field."); \
    field_name = destroy_value;                                                \
  }                                                                            \
  static_assert(true)
