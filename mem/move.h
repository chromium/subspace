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

#include "macros/always_inline.h"

namespace sus::mem {

/// Verify that an object of type `T`, or referred to by `T` if it's a
/// reference, is non-const.
template <class T>
concept NonConstObject = (!std::is_const_v<std::remove_reference_t<T>>);

/// Verify that `T` can be moved with `sus::move()` to construct another `T`.
///
/// This is similar to `std::is_move_constructible`, however it requires that
/// `T` is non-const. Otherwise, a copy would occur and `sus::move()` will fail
/// to compile.
template <class T>
concept Moveable = (NonConstObject<T> && std::is_move_constructible_v<T>);

/// Verify that `T` can be moved with `sus::move()` to assign to another `T`.
///
/// This is similar to `std::is_move_assignable`, however it requires that
/// `T` is non-const. Otherwise, a copy would occur and `sus::move()` will fail
/// to compile.
template <class T>
concept MoveableForAssign = (NonConstObject<T> && std::is_move_assignable_v<T>);

/// Cast `t` to an r-value reference so that it can be used to construct or be
/// assigned to another `T`.
///
/// `move()` requires that `t` can be moved from, so it requires that `t` is
/// non-const.
///
/// The `move()` call itself does nothing to `t`, as it is just a cast, similar
/// to `std::move()`. It enables an lvalue object to be used as an rvalue.
//
// TODO: Should this be `as_rvalue()`? Kinda technical. `as_...something...()`?
template <NonConstObject T>
sus_always_inline constexpr std::remove_reference_t<T>&& move(T&& t) noexcept {
  return static_cast<typename std::remove_reference_t<T>&&>(t);
}

}  // namespace sus::mem

namespace sus {
using ::sus::mem::move;
}
