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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/marker/unsafe.h"
#include "sus/num/integer_concepts.h"
#include "sus/num/unsigned_integer.h"
#include "sus/option/option.h"

namespace sus::iter::__private {

template <::sus::num::IntegerNumeric T>
constexpr T step_max() noexcept {
  return T::MAX;
}
template <::sus::num::IntegerNumeric T>
constexpr T step_forward(T l) noexcept {
  // SAFETY: All `Integer` can hold `1`.
  return l + T::try_from(1).unwrap_unchecked(::sus::marker::unsafe_fn);
}
template <::sus::num::IntegerNumeric T>
constexpr T step_backward(T l) noexcept {
  // SAFETY: All `Integer` can hold `1`.
  return l - T::try_from(1).unwrap_unchecked(::sus::marker::unsafe_fn);
}
template <::sus::num::IntegerNumeric T>
constexpr ::sus::Option<::sus::num::usize> steps_between(const T& l,
                                                         const T& r) noexcept {
  return r.checked_sub(l).and_then(
      [](T steps) { return ::sus::num::usize::try_from(steps).ok(); });
}

template <::sus::num::IntegerPointer T>
constexpr T step_max() noexcept {
  // TODO: This is dumb, so maybe uptr::MAX should exist?
  // https://github.com/chromium/subspace/issues/238#issuecomment-1730736193
  return T::MAX_BIT_PATTERN;
}
template <::sus::num::IntegerPointer T>
constexpr T step_forward(T l) noexcept {
  return l + usize(1u);
}
template <::sus::num::IntegerPointer T>
constexpr T step_backward(T l) noexcept {
  return l - usize(1u);
}
template <::sus::num::IntegerPointer T>
constexpr ::sus::Option<::sus::num::usize> steps_between(const T& l,
                                                         const T& r) noexcept {
  return r.checked_sub(l).and_then(
      [](T steps) { return ::sus::num::usize::try_from(steps).ok(); });
}

template <::sus::num::PrimitiveInteger T>
constexpr T step_max() noexcept {
  return ::sus::num::__private::max_value<T>();
}

template <::sus::num::PrimitiveInteger T>
constexpr T step_forward(T l) noexcept {
  ::sus::check(l < ::sus::num::__private::max_value<T>());
  // SAFETY: All `PrimitiveInteger` can hold `1`.
  return l + T(1);
}
template <::sus::num::PrimitiveInteger T>
constexpr T step_backward(T l) noexcept {
  ::sus::check(l > ::sus::num::__private::min_value<T>());
  // SAFETY: All `PrimitiveInteger` can hold `1`.
  return l - T(1);
}
template <::sus::num::PrimitiveInteger T>
constexpr ::sus::Option<::sus::num::usize> steps_between(const T& l,
                                                         const T& r) noexcept {
  if (r >= l) {
    return ::sus::num::usize::try_from(r - l).ok();
  } else {
    return sus::none();
  }
}

/// Objects that have a notion of successor and predecessor operations.
///
/// The successor operations move towards values that compare greater. The
/// predecessor operations moves toward values that compare lesser.
template <class T>
concept Step = requires(const T& t, ::sus::num::usize n) {
  // Required methods.
  { ::sus::iter::__private::step_max<T>() } noexcept -> std::same_as<T>;
  { ::sus::iter::__private::step_forward(t) } noexcept -> std::same_as<T>;
  { ::sus::iter::__private::step_backward(t) } noexcept -> std::same_as<T>;
  /* These are part of Rust std, but are not used in C++ yet, so not required.
  {
    ::sus::iter::__private::step_forward_checked(t)
  } -> std::same_as<::sus::Option<T>>;
  {
    ::sus::iter::__private::step_backward_checked(t)
  } -> std::same_as<::sus::Option<T>>;
  { ::sus::iter::__private::step_forward_by(t, n) } -> std::same_as<T>;
  { ::sus::iter::__private::step_backward_by(t, n) } -> std::same_as<T>;
  {
    ::sus::iter::__private::step_forward_by_checked(t, n)
  } -> std::same_as<::sus::Option<T>>;
  {
    ::sus::iter::__private::step_backward_by_checked(t, n)
  } -> std::same_as<::sus::Option<T>>;
  */
  {
    ::sus::iter::__private::steps_between(t, t)
  } noexcept -> std::same_as<::sus::Option<::sus::num::usize>>;
};

}  // namespace sus::iter::__private
