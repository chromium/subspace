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

#include "marker/unsafe.h"

namespace sus::mem::layout {

namespace __private {

template <class T>
struct layout_nonzero_tag final {
  static constexpr bool has_field(...) { return false; }
  static constexpr bool has_field(int)
    requires(std::is_same_v<
             decltype(std::declval<T>().SusUnsafeNonZeroIsNonZero()), bool>)
  {
    return true;
  };

  static constexpr bool is_non_zero(const T* t)
  {
    return t->SusUnsafeNonZeroIsNonZero();
  };
  static constexpr void set_zero(T* t)
  {
    t->SusUnsafeNonZeroSetZero();
  };
};

}  // namespace __private

template <class T>
struct nonzero_field {
  static constexpr bool has_field =
      __private::layout_nonzero_tag<T>::has_field(0);

  static constexpr bool is_non_zero(::sus::marker::UnsafeFnMarker,
                                    const T* t) noexcept
    requires(has_field)
  {
    return __private::layout_nonzero_tag<T>::is_non_zero(t);
  }
  static constexpr void set_zero(::sus::marker::UnsafeFnMarker, T* t) noexcept
    requires(has_field)
  {
    __private::layout_nonzero_tag<T>::set_zero(t);
  }
};

}  // namespace sus::mem::layout

/// Mark a class field as never being zero (after a constructor has run, until
/// the destructor has completed).
#define sus_class_nonzero_field(unsafe_fn, T, name)                   \
  static_assert(std::is_same_v<decltype(unsafe_fn),                   \
                               const ::sus::marker::UnsafeFnMarker>); \
  template <class SusOuterClassTypeForNonZeroField>                   \
  friend struct ::sus::mem::layout::__private::layout_nonzero_tag;    \
  constexpr inline bool SusUnsafeNonZeroIsNonZero() const noexcept {  \
    static_assert(std::same_as<decltype(*this), const T&>);           \
    return static_cast<bool>(name);                                   \
  }                                                                   \
  constexpr inline void SusUnsafeNonZeroSetZero() noexcept {          \
    name = static_cast<decltype(name)>(0u);                           \
  }                                                                   \
  static_assert(true)
