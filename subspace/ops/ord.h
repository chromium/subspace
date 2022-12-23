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

#include <compare>
#include <concepts>

#include "assertions/check.h"
#include "fn/callable.h"

// TODO: PartialOrd comes with: lt, le, ge, gt.
// TODO: Eq comes with eq, ne.
// TODO: Where to put these functions??

namespace sus::ops {

using ::sus::fn::callable::CallableReturns;
using ::sus::fn::callable::CallableWith;

/// Concept for types that form a total order (aka `std::strong_ordering`).
template <class T, class U = T>
concept Ord = requires(const T& lhs, const U& rhs) {
                { lhs <=> rhs } -> std::same_as<std::strong_ordering>;
              };

/// Concept for types that form a weak ordering (aka `std::weak_ordering`).
///
/// This will be true if the types have a total ordering as well, which is
/// stronger than a weak ordering. To determine if a weak ordering is the
/// strongest type of ordering between the types, use `ExclusiveWeakOrd`.
template <class T, class U = T>
concept WeakOrd = Ord<T, U> || requires(const T& lhs, const U& rhs) {
                                 {
                                   lhs <=> rhs
                                   } -> std::same_as<std::weak_ordering>;
                               };

/// Concept for types that form a partial ordering (aka
/// `std::partial_ordering`).
///
/// This will be true if the types have a weak r total ordering as well, which
/// is stronger than a partial ordering. To determine if a partial ordering is
/// the strongest type of ordering between the types, use `ExclusivePartialOrd`.
template <class T, class U = T>
concept PartialOrd =
    WeakOrd<T, U> || Ord<T, U> || requires(const T& lhs, const U& rhs) {
                                    {
                                      lhs <=> rhs
                                      } -> std::same_as<std::partial_ordering>;
                                  };

/// Concept for types that have a total ordering (aka `std::strong_ordering`).
///
/// This is an alias for Ord, but exists as a set with `ExclusiveWeakOrd` and
/// `ExclusivePartialOrd`.
template <class T, class U = T>
concept ExclusiveOrd = Ord<T, U>;

/// Determines if the types `Lhs` and `Rhs` have a weak ordering (aka
/// `std::weak_ordering`), and that this is the strongest ordering that exists
/// between the types.
template <class T, class U = T>
concept ExclusiveWeakOrd = (!Ord<T, U> && WeakOrd<T, U>);

/// Determines if the types `Lhs` and `Rhs` have a partial ordering (aka
/// `std::partial_ordering`), and that this is the strongest ordering that
/// exists between the types.
template <class T, class U>
concept ExclusivePartialOrd = (!WeakOrd<T, U> && PartialOrd<T, U>);

/// Compares and returns the minimum of two values.
///
/// Returns the first argument if the comparison determines them to be equal.
///
/// By default this receives and returns objects by value. To receive and return
/// references, specify the type parameter, such as:
/// `sus::ops::min<i32&>(a, b)`. Note that if either input is a temporary object
/// this can return a reference to an object past its lifetime.
template <class T>
  requires(Ord<T>)
constexpr T min(T a, T b) noexcept {
  return a > b ? b : a;
}

/// Compares and returns the minimum of two values with respect to the specified
/// comparison function.
///
/// Returns the first argument if the comparison determines them to be equal.
///
/// By default this receives and returns objects by value. To receive and return
/// references, specify the type parameter, such as:
/// `sus::ops::min_by<i32&>(a, b, c)`. Note that if either input is a temporary
/// object this can return a reference to an object past its lifetime.
template <class T, CallableReturns<std::strong_ordering, const T&, const T&> F>
constexpr T min_by(T a, T b, F compare) noexcept {
  return compare(a, b) == std::strong_ordering::greater ? b : a;
}

/// Returns the element that gives the minimum value from the specified
/// function.
///
/// Returns the first argument if the comparison determines them to be equal.
///
/// By default this receives and returns objects by value. To receive and return
/// references, specify the type parameter, such as:
/// `sus::ops::min_by_key<i32&>(a, b, k)`. Note that if either input is a
/// temporary object this can return a reference to an object past its lifetime.
template <class T, CallableWith<const T&> F, int&...,
          class K = std::invoke_result_t<F, const T&>>
  requires(Ord<K>)
constexpr T min_by_key(T a, T b, F f) noexcept {
  return f(a) > f(b) ? b : a;
}

/// Compares and returns the maximum of two values.
///
/// Returns the second argument if the comparison determines them to be equal.
///
/// By default this receives and returns objects by value. To receive and return
/// references, specify the type parameter, such as:
/// `sus::ops::max<i32&>(a, b)`. Note that if either input is a temporary object
/// this can return a reference to an object past its lifetime.
template <class T>
  requires(Ord<T>)
constexpr T max(T a, T b) noexcept {
  return a > b ? a : b;
}

/// Compares and returns the maximum of two values with respect to the specified
/// comparison function.
///
/// Returns the second argument if the comparison determines them to be equal.
///
/// By default this receives and returns objects by value. To receive and return
/// references, specify the type parameter, such as:
/// `sus::ops::max_by<i32&>(a, b, c)`. Note that if either input is a temporary
/// object this can return a reference to an object past its lifetime.
template <class T, CallableReturns<std::strong_ordering, const T&, const T&> F>
constexpr T max_by(T a, T b, F compare) noexcept {
  return compare(a, b) == std::strong_ordering::greater ? a : b;
}

/// Returns the element that gives the maximum value from the specified
/// function.
///
/// Returns the second argument if the comparison determines them to be equal.
///
/// By default this receives and returns objects by value. To receive and return
/// references, specify the type parameter, such as:
/// `sus::ops::max_by_key<i32&>(a, b, k)`. Note that if either input is a
/// temporary object this can return a reference to an object past its lifetime.
template <class T, CallableWith<const T&> F, int&...,
          class K = std::invoke_result_t<F, const T&>>
  requires(Ord<K>)
constexpr T max_by_key(T a, T b, F f) noexcept {
  return f(a) > f(b) ? a : b;
}

/// Restrict a value to a certain interval.
///
/// Returns `max` if `v` is greater than `max`, and `min` if `v` is less than
/// `min`. Otherwise this returns `v`.
///
/// By default this receives and returns objects by value. To receive and return
/// references, specify the type parameter, such as:
/// `sus::ops::clamp<i32&>(a, min, max)`. Note that if any input is a temporary
/// object this can return a reference to an object past its lifetime.
///
/// # Panics
/// Panics if `min > max`.
template <class T>
  requires(Ord<T>)
constexpr T clamp(T v, T min, T max) noexcept {
  ::sus::check(min <= max);
  return v < min ? min : (v > max ? max : v);
}

}  // namespace sus::ops
