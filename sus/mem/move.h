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

// IWYU pragma: private, include "sus/prelude.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <type_traits>

#include "sus/macros/inline.h"
#include "sus/macros/pure.h"

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
/// # Example
/// ```
/// struct S {
///   S() = default;
///   S(S&&) = default;
///   S& operator=(S&&) = default;
/// };
/// static_assert(sus::mem::Move<S>);
/// ```
template <class T>
concept Move =
    std::is_move_constructible_v<
        std::remove_const_t<std::remove_reference_t<T>>> &&
    std::is_move_assignable_v<std::remove_const_t<std::remove_reference_t<T>>>;

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

/// Matches types which are [`MoveOrRef`]($sus::mem::MoveOrRef) or are `void`.
///
/// A helper for genertic types which can hold void as a type.
template <class T>
concept MoveOrRefOrVoid = MoveOrRef<T> || std::is_void_v<T>;

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
template <class T>
  requires(!std::is_const_v<std::remove_reference_t<T>>)
sus_pure_const sus_always_inline constexpr decltype(auto) move(T&& t) noexcept {
  return static_cast<std::remove_reference_t<T>&&>(t);
}

/// A concept that can be used to constrain a universal reference parameter to
/// ensure the caller provides something that was moved from, akin to receiving
/// by value. This avoids inadvertantly moving out of an lvalue in the caller.
///
/// Always invoke IsMoveRef with the `decltype` of the argument being tested to
/// ensure the correct type is tested, such as `IsMoveRef<decltype(arg)>`.
///
/// In the common case, when you want to receive a parameter that will be moved,
/// it should be received by value. However, library implementors sometimes with
/// to receive an *rvalue reference*. That is a reference to an rvalue which can
/// be moved from. This is unfortunately spelled the same as a universal
/// reference, so it will also bind to an lvalue in the caller, and moving from
/// the reference would move from the caller's lvalue.
///
/// To receive an rvalue reference (and not a universal reference), constrain
/// the input universal reference `T&& t` by the `IsMoveRef` concept with
/// `requires(IsMoveRef<decltype(t)>)`. When this is satisfied, the input will
/// need to be an rvalue. In this case, `sus::move()` will always perform a move
/// from an rvalue and will not move out of an lvalue in the caller.
///
/// `sus::forward<T>` can also be used with universal references, but when
/// constructing a value type `T` it will implicitly copy from const
/// references.
template <class T>
concept IsMoveRef = std::is_rvalue_reference_v<T> &&
                    !std::is_const_v<std::remove_reference_t<T>>;

}  // namespace sus::mem

namespace sus {
using ::sus::mem::move;
}  // namespace sus
