// Copyright 2023 Google LLC
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

#include "subspace/marker/unsafe.h"
#include "subspace/num/integer_concepts.h"
#include "subspace/num/unsigned_integer.h"

namespace sus::option {
template <class T>
class Option;
}

namespace sus::iter::__private {

template <::sus::num::Integer T>
constexpr T step_forward(T l) noexcept {
  // SAFETY: All `Integer` can hold `1`.
  return l + T::from_unchecked(::sus::marker::unsafe_fn, 1);
}
template <::sus::num::Integer T>
constexpr T step_backward(T l) noexcept {
  // SAFETY: All `Integer` can hold `1`.
  return l - T::from_unchecked(::sus::marker::unsafe_fn, 1);
}
template <::sus::num::Integer T>
constexpr Option<T> step_forward_checked(T l) noexcept {
  // SAFETY: All `Integer` can hold `1`.
  return l.checked_add(T::from_unchecked(::sus::marker::unsafe_fn, 1));
}
template <::sus::num::Integer T>
constexpr Option<T> step_backward_checked(T l) noexcept {
  // SAFETY: All `Integer` can hold `1`.
  return l.checked_sub(T::from_unchecked(::sus::marker::unsafe_fn, 1));
}
template <::sus::num::Integer T>
constexpr T step_forward_by(T l, ::sus::num::usize steps) noexcept {
  return l + T::from(steps);
}
template <::sus::num::Integer T>
constexpr T step_backward_by(T l, ::sus::num::usize steps) noexcept {
  return l - T::from(steps);
}
template <::sus::num::Integer T>
constexpr Option<T> step_forward_by_checked(T l,
                                            ::sus::num::usize steps) noexcept {
  return T::try_from(steps).ok().and_then(
      [&l](T steps) { return l.checked_add(::sus::marker::unsafe_fn, steps); });
}
template <::sus::num::Integer T>
constexpr Option<T> step_backward_by_checked(T l,
                                             ::sus::num::usize steps) noexcept {
  return T::try_from(steps).ok().and_then(
      [&l](T steps) { return l.checked_sub(::sus::marker::unsafe_fn, steps); });
}
template <::sus::num::Integer T>
constexpr Option<::sus::num::usize> steps_between(const T& l,
                                                  const T& r) noexcept {
  return r.checked_sub(l).and_then(
      [](T steps) { return ::sus::num::usize::try_from(steps).ok(); });
}

/// Objects that have a notion of successor and predecessor operations.
///
/// The successor operations move towards values that compare greater. The
/// predecessor operations moves toward values that compare lesser.
template <class T>
concept Step = requires(const T& t, ::sus::num::usize n) {
  // Required methods.
  { ::sus::iter::__private::step_forward(t) } -> std::same_as<T>;
  { ::sus::iter::__private::step_backward(t) } -> std::same_as<T>;
  {
    ::sus::iter::__private::step_forward_checked(t)
  } -> std::same_as<::sus::option::Option<T>>;
  {
    ::sus::iter::__private::step_backward_checked(t)
  } -> std::same_as<::sus::option::Option<T>>;
  { ::sus::iter::__private::step_forward_by(t, n) } -> std::same_as<T>;
  { ::sus::iter::__private::step_backward_by(t, n) } -> std::same_as<T>;
  {
    ::sus::iter::__private::step_forward_by_checked(t, n)
  } -> std::same_as<::sus::option::Option<T>>;
  {
    ::sus::iter::__private::step_backward_by_checked(t, n)
  } -> std::same_as<::sus::option::Option<T>>;
  {
    ::sus::iter::__private::steps_between(t, t)
  } -> std::same_as<::sus::option::Option<::sus::num::usize>>;
};

}  // namespace sus::iter::__private
