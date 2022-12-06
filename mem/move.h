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

/// A `Move` type can be moved-from to construct a new object and can be
/// assigned to by move.
///
/// A type satisfies `Move` by implementing a move constructor and assignment
/// operator.
///
/// A type that is `Copy` is also `Move`. However the type can opt out by
/// explicitly deleteing the move constructor and assignment operator. This is
/// not recommended, unless deleting the copy operations too, as it tends to
/// break things that want to move-or-fallback-to-copy.
///
/// As a special case, types that can not be assigned to at all, by copy or
/// move, can still satisfy Move by being able to construct by move. This is
/// requires for types like lambdas.
///
/// # Example
/// ```
/// struct S {
///   S() = default;
///   S(S&&) = default;
///   S& operator=(S&&) = default;
/// };
/// static_assert(sus::mem::Move<S>);
template <class T>
concept Move = std::is_move_constructible_v<T> &&
               (std::is_move_assignable_v<T> || !std::is_copy_assignable_v<T>);

/// Cast `t` to an r-value reference so that it can be used to construct or be
/// assigned to another `T`.
///
/// `move()` requires that `t` can be moved from, so it requires that `t` is
/// non-const. This differs from `std::move()`.
///
/// The `move()` call itself does nothing to `t`, as it is just a cast, similar
/// to `std::move()`. It enables an lvalue (a named object) to be used as an
/// rvalue.
//
// TODO: Should this be `as_rvalue()`? Kinda technical. `as_...something...()`?
template <Move T>
  requires(!std::is_const_v<std::remove_reference_t<T>>)
[[nodiscard]] sus_always_inline constexpr auto&& move(T&& t) noexcept {
  return static_cast<typename std::remove_reference_t<T>&&>(t);
}

/// Like move(), but if the object being moved is a reference, the reference
/// will be copied, even if it is const.
///
/// A copy of `T` does not occur in either case. Either `T` is moved, or a
/// reference-to-`T` is copied.
template <class T>
  requires(std::is_reference_v<T> || (Move<T> && !std::is_const_v<T>))
[[nodiscard]] sus_always_inline
    constexpr auto&& move_or_copy_ref(T&& t) noexcept {
  return static_cast<typename std::remove_reference_t<T>&&>(t);
}

}  // namespace sus::mem

namespace sus {
using ::sus::mem::move;
}  // namespace sus
