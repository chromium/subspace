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

#include "sus/macros/inline.h"
#include "sus/macros/pure.h"

namespace sus::mem {

/// Move from non-reference values but pass through and preserve references.
///
/// Typically, passing an rvalue reference will convert it to an lvalue. Using
/// `sus::forward<T>(t)` on an rvalue reference `T&& t` will preserve the rvalue
/// nature of the reference. Other reference types are also forwarded unchanged.
///
/// The type argument must be provided, and should be the `T` from a `T&&`
/// typename in order to properly preserve the reference type of the object of
/// type `T&&`.
///
/// # Universal references and moves
///
/// In the common case, when you want to receive a parameter that will be moved,
/// it should be received by value. However, library implementors sometimes with
/// to receive an *rvalue reference*. If you find yourself needing to
/// [`move()`]($sus::mem::move) from a universal reference instead of
/// [`forward()`]($sus::mem::forward), such as to construct a
/// value type `T` from a universal reference `T&&` without introducing a copy,
/// use [`IsMoveRef`]($sus::mem::IsMoveRef) to constrain the universal reference
/// to be an rvalue, and use [`move()`]($sus::mem::move) instead of
/// [`forward()`]($sus::mem::forward).
template <class T>
sus_pure_const _sus_always_inline constexpr T&& forward(
    std::remove_reference_t<T>& t) noexcept {
  return static_cast<T&&>(t);
}

template <class T>
sus_pure_const _sus_always_inline constexpr T&& forward(
    std::remove_reference_t<T>&& t) noexcept {
  static_assert(!std::is_lvalue_reference_v<T>,
                "Can not convert an rvalue to an lvalue with forward().");
  return static_cast<T&&>(t);
}

}  // namespace sus::mem

namespace sus {
using ::sus::mem::forward;
}  // namespace sus
