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

#include <concepts>

#include "subspace/macros/inline.h"
#include "subspace/mem/copy.h"
#include "subspace/mem/move.h"

namespace sus::mem {

namespace __private {

// clang-format off
template <class T>
concept HasCloneMethod = requires(const T& source) {
    { source.clone() } -> std::same_as<T>;
};

template <class T>
concept HasCloneFromMethod = requires(T& self, const T& source) {
    { self.clone_from(source) } -> std::same_as<void>;
};
// clang-format on

}  // namespace __private

/// A `Clone` type can make a new copy of itself.
///
/// A type `T` is `Clone` if one of the following holds:
/// * Is `Copy`. Meaning it has a copy constructor and assignment.
/// * Has a `clone() const -> T` method, and is `Move`.
///
/// This concept tests the object type of `T`, not a reference type `T&` or
/// `const T&`.
///
/// A `Clone` type may also provide a `clone_from(const T& source)` method
/// to have `sus::mem::clone_into()` perform copy-assignment from `source`, in
/// order to reuse its resources and avoid allocations. In this case the type is
/// also `CloneInto`.
///
/// It is not valid to be `Copy` and also have a `clone()` method, as it becomes
/// ambiguous. One outcome of this is a container type should only implement a
/// clone() method if the type within is (Clone && !Copy), and should implement
/// copy constructors if the type within is Copy.
template <class T>
concept Clone = (Copy<T> &&
                 !__private::HasCloneMethod<
                     std::remove_const_t<std::remove_reference_t<T>>>) ||
                (!Copy<T> && Move<T> &&
                 __private::HasCloneMethod<
                     std::remove_const_t<std::remove_reference_t<T>>>);

/// A `CloneOrRef` object or reference of type `T` can be cloned to construct a
/// new `T`.
///
/// This concept is used for templates that want to be generic over references,
/// that is templates that want to allow their template parameter to be a
/// reference and work with that reference as if it were an object itself. This
/// is uncommon outside of library implementations, and its usage should
/// typically be encapsulated inside a type that is `Clone`.
template <class T>
concept CloneOrRef = Clone<T> || std::is_reference_v<T>;

/// A concept to verify that a Clone type has a specialization of `clone_from()`
/// in order to optimize `::sus::clone_into()`. Mostly for testing types to
/// ensure they're doing what you think they're doing.
///
/// Evaluates to true if the type is `Copy` but it is not valid to be `Copy` and
/// also have a `clone_from()` method, as it becomes ambiguous.
///
/// TODO: Should we make this into/from name consistent...??
template <class T>
concept CloneInto =
    Clone<T> &&
    ((Copy<T> && !__private::HasCloneFromMethod<
                     std::remove_const_t<std::remove_reference_t<T>>>) ||
     (!Copy<T> && __private::HasCloneFromMethod<
                      std::remove_const_t<std::remove_reference_t<T>>>));

/// Clones the input either by copying or cloning. Returns a new object of type
/// `T`.
///
/// If `T` is a reference type, it will clone the underlying object.
template <Clone T>
[[nodiscard]] sus_always_inline constexpr std::decay_t<T> clone(
    const T& source) noexcept {
  if constexpr (Copy<T>) {
    return source;
  } else {
    return source.clone();
  }
}

/// Clones the input either by copying or cloning, producing an object of type
/// T.
///
/// If `T` is a reference type, it copies and returns the reference instead of
/// the underlying object.
template <CloneOrRef T>
[[nodiscard]] sus_always_inline constexpr T clone_or_forward(
    const std::remove_reference_t<T>& source) noexcept {
  if constexpr (std::is_reference_v<T>) {
    return source;
  } else {
    return ::sus::mem::clone(source);
  }
}

/// Performs copy-assignment from `source`.
///
/// Will perform a copy assignment if T is `Copy`. Otherwise will perform the
/// equivalent of `dest = source.clone()`.
///
/// The `Clone` type may provide an implementation of `clone_from(const T&
/// source)` to reuse its resources and avoid unnecessary allocations, which
/// will be preferred over calling `source.clone()`.
//
// TODO: Provide a concept to verify that this method exists.
template <Clone T>
sus_always_inline constexpr void clone_into(T& dest, const T& source) noexcept {
  if constexpr (Copy<T>) {
    dest = source;
  } else if constexpr (__private::HasCloneFromMethod<T>) {
    dest.clone_from(source);
  } else {
    dest = source.clone();
  }
}

}  // namespace sus::mem

// Promote `clone()` and `clone_into()` into the `sus` namespace.
namespace sus {
using ::sus::mem::clone;
using ::sus::mem::clone_into;
}  // namespace sus
