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

/// A `Move` type can be moved-from to construct a new object of the same type
/// and can be assigned to by move.
///
/// A type satisfies `Move` by implementing a move constructor and assignment
/// operator.
///
/// Const (non-reference) types are not `Move` as they can't be assigned to.
/// References are always `Move`, even if const, as a reference can always be
/// constructed from a reference.
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
concept Move = std::is_move_constructible_v<std::decay_t<T>> &&
               (std::is_move_assignable_v<std::decay_t<T>> ||
                !std::is_copy_assignable_v<std::decay_t<T>>);

/// A `MoveOrRef` object or reference of type `T` can be moved to construct a
/// new `T`.
///
/// This concept is used for templates that want to be generic over references,
/// that is templates that want to allow their template parameter to be a
/// reference and work with that reference as if it were an object itself. This
/// is uncommon outside of library implementations, and its usage should
/// typically be encapsuated inside a type that is `Move`.
template <class T>
concept MoveOrRef = Move<T> || std::is_reference_v<T>;

/// Cast `t` to an r-value reference so that it can be used to construct or be
/// assigned to a (non-reference) object of type `T`.
///
/// `move()` requires that `t` can be moved from, so it requires that `t` is
/// non-const. This differs from `std::move()`.
///
/// The `move()` call itself does nothing to `t`, as it is just a cast, similar
/// to `std::move()`. It enables an lvalue (a named object) to be used as an
/// rvalue. The function does not require `T` to be `Move`, in order to call
/// rvalue-qualified methods on `T` even if it is not `Move`.
//
// TODO: Should this be `as_rvalue()`? Kinda technical. `as_...something...()`?
template <class T>
  requires(!std::is_const_v<std::remove_reference_t<T>>)
[[nodiscard]] sus_always_inline constexpr decltype(auto) move(T&& t) noexcept {
  return static_cast<std::decay_t<T>&&>(t);
}

/// Moves-from x if x is a non-reference type, and copies the reference if x is
/// a reference type.
///
/// Like move(), this function may not be called with a const non-reference
/// value.
///
/// This requires a macro to implement as we need to determine the type of the
/// expression itself, not a function parameter at which point an lvalue becomes
/// an lvalue-reference.
///
/// NOTE: An expression of type `structure.field` will be a reference even if
/// the `field` is not a reference. To avoid this and actually get the type of
/// the field, you would require a method on structure that returns
/// `sus_move_preserve_ref(field)` with a return type of `decltype(auto)`. See
/// Tuple::into_inner() for an example, where it calls a method instead of
/// directly returning the field.
//
// Implemented with a lambda in order to static_assert things inside an
// expression.
#define sus_move_preserve_ref(x)                                     \
  []<class Y>(Y&& y) -> decltype(auto) {                             \
    /*static_assert(MoveOrRef<Y>);                                */ \
    static_assert(std::is_reference_v<Y> ||                          \
                  !std::is_const_v<std::remove_reference_t<Y>>);     \
    return static_cast<Y&&>(y);                                      \
  }(static_cast<decltype(x)&&>(x))

}  // namespace sus::mem

namespace sus {
using ::sus::mem::move;
}  // namespace sus
