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

#include "mem/copy.h"
#include "mem/move.h"
#include "mem/mref.h"

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
/// A `Clone` type may also provide a `clone_from(const T& source)` method
/// to have `sus::mem::clone_into()` perform copy-assignment from `source`, in
/// order to reuse its resources and avoid allocations.
///
/// It is not valid to be `Copy` and also have a `clone()` method, as it becomes
/// amgiuous. One outcome of this is a container type should only implement a
/// clone() method if the type within is (Clone && !Copy), and should implement
/// copy constructors if the type within is Copy.
//
template <class T>
concept Clone = (Copy<T> || (__private::HasCloneMethod<T> && Move<T>)) &&
                !(__private::HasCloneMethod<T> && Copy<T>);

/// A concept to verify that a Clone type has a specialization of `clone_from()`
/// in order to optimize `::sus::clone_into()`. Mostly for testing types to
/// ensure they're doing what you think they're doing.
///
/// TODO: Should we make this into/from name consistent...??
template <class T>
concept CloneFrom = Clone<T> && __private::HasCloneFromMethod<T>;

template <Clone T>
inline constexpr auto clone(const T& source) noexcept {
  if constexpr (Copy<T>) {
    return source;
  } else {
    return source.clone();
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
template <Clone T>
inline constexpr void clone_into(T& dest, const T& source) noexcept {
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
