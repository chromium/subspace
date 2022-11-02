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
#include "mem/mref.h"

namespace sus::mem {

template <class T>
struct never_value_field {
 private:
  static constexpr bool has_field_helper(...) { return false; }
  static constexpr bool has_field_helper(int)
    requires(
        std::same_as<bool, decltype(std::declval<T>()
                                        .SusUnsafeNeverValueIsConstructed())>)
  {
    return true;
  };

 public:
  static constexpr bool has_field = has_field_helper(0);

  static constexpr sus_always_inline bool is_constructed(
      ::sus::marker::UnsafeFnMarker, const T& t) noexcept
    requires(has_field)
  {
    return t.SusUnsafeNeverValueIsConstructed();
  }
  static constexpr sus_always_inline void set_never_value(
      ::sus::marker::UnsafeFnMarker, Mref<T&> t) noexcept
    requires(has_field)
  {
    t.inner().SusUnsafeNeverValueSetNeverValue();
  }
};

}  // namespace sus::mem

/// Mark a class field as never being a specific value, often a zero, after a
/// constructor has run and bef ore the destructor has completed. This allows
/// querying if a class is constructed in a memory location, since the class is
/// constructed iff the value of the field is not the never-value.
#define sus_class_never_value_field(unsafe_fn, T, name, never_value)           \
  static_assert(                                                               \
      std::same_as<decltype(unsafe_fn), const ::sus::marker::UnsafeFnMarker>); \
  template <class SusOuterClassTypeForNeverValueField>                         \
  friend struct ::sus::mem::never_value_field;                         \
  constexpr sus_always_inline bool SusUnsafeNeverValueIsConstructed()          \
      const noexcept {                                                         \
    static_assert(std::same_as<decltype(*this), const T&>);                    \
    return name != never_value;                                                \
  }                                                                            \
  constexpr sus_always_inline void                                             \
  SusUnsafeNeverValueSetNeverValue() noexcept {                                \
    name = never_value;                                                        \
  }                                                                            \
  static_assert(true)
