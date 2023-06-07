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

#include "subspace/macros/inline.h"
#include "subspace/macros/pure.h"

namespace sus::mem {

/// A `Move` type can be moved-from to construct a new object of the same type
/// and can be assigned to by move.
///
/// A type satisfies `Move` by implementing a move constructor and assignment
/// operator.
///
/// This concept tests the object type of `T`, not a reference type `T&` or
/// `const T&`.
///
/// A type that is `Copy` is also `Move`. However the type can opt out by
/// explicitly deleting the move constructor and assignment operator. This is
/// not recommended, unless deleting the copy operations too, as it tends to
/// break things that want to move-or-fallback-to-copy.
///
/// Types that can not be assigned to at all, by copy or move, can still satisfy
/// Move by being able to construct by move. This is required for types like
/// lambdas, or types with const fields.
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
concept Move = std::is_move_constructible_v<
                   std::remove_const_t<std::remove_reference_t<T>>> &&
               (std::is_move_assignable_v<
                    std::remove_const_t<std::remove_reference_t<T>>>/* ||
                !std::is_copy_assignable_v<
                    std::remove_const_t<std::remove_reference_t<T>>>*/);

/// A `MoveOrRef` object or reference of type `T` can be moved to construct a
/// new `T`.
///
/// This concept is used for templates that want to be generic over references,
/// that is templates that want to allow their template parameter to be a
/// reference and work with that reference as if it were an object itself. This
/// is uncommon outside of library implementations, and its usage should
/// typically be encapsulated inside a type that is `Move`.
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
sus_pure_const sus_always_inline constexpr auto&& move(T&& t) noexcept {
  return static_cast<std::remove_reference_t<T>&&>(t);
}

}  // namespace sus::mem

namespace sus {
using ::sus::mem::move;
}  // namespace sus
