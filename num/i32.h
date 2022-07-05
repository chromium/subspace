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

#include <stdint.h>

#include <compare>
#include <concepts>

#include "assertions/check.h"
#include "marker/unsafe.h"
#include "num/__private/literals.h"
#include "num/__private/signed_integer_macros.h"
#include "num/__private/unsigned_integer_macros.h"  // TODO: Remove this, include u32 instead.
#include "option/option.h"

namespace sus::num {

/// A 32-bit signed integer.
struct i32 {
 private:
  /// The underlying primitive type.
  using primitive_type = int32_t;

 public:
  /// Default constructor, which sets the integer to 0.
  ///
  /// The trivial copy and move constructors are implicitly declared, as is the
  /// trivial destructor.
  constexpr inline i32() noexcept = default;

  /// Construction from the underlying primitive type.
  template <class T>
    requires(std::same_as<T, primitive_type>)  // Prevent implicit conversions.
  constexpr inline i32(T val) noexcept : primitive_value(val) {}

  /// Assignment from the underlying primitive type.
  template <class T>
    requires(std::same_as<T, primitive_type>)  // Prevent implicit conversions.
  constexpr inline void operator=(T v) noexcept {
    primitive_value = v;
  }

  _sus__signed_constants(i32, 0x7FFFFFFF, 32);
  _sus__signed_impl(i32, /*LargerT=*/int64_t, /*UnsignedT=*/uint32_t);

  // TODO: overflowing_abs(). Need a tuple type.
  // TODO: overflowing_add(). Need a tuple type.
  // TODO: overflowing_div(). Need a tuple type.
  // TODO: overflowing_div_euclid(). Need a tuple type.
  // TODO: overflowing_mul(). Need a tuple type.
  // TODO: overflowing_neg(). Need a tuple type.
  // TODO: overflowing_rem(). Need a tuple type.
  // TODO: overflowing_rem_euclid(). Need a tuple type.
  // TODO: overflowing_shl(). Need a tuple type.
  // TODO: overflowing_shr(). Need a tuple type.
  // TODO: overflowing_sub(). Need a tuple type.
  // TODO: overflowing_sub_unsigned(). Need a tuple type.
  // TODO: div_euclid().
  // TODO: wrapping_div_euclid().
  // TODO: wrapping_rem_euclid().
  // TODO: checked_div_euclid().
  // TODO: checked_log().
  // TODO: checked_log10().
  // TODO: checked_log2().
  // TODO: checked_next_multiple_of().
  // TODO: checked_pow().
  // TODO: checked_rem_euclid().
  // TODO: checked_shr().
  // TODO: from_be().
  // TODO: from_be_bytes().
  // TODO: from_le().
  // TODO: from_le_bytes().
  // TODO: from_ne_bytes().
  // TODO: from_str_radix(). Need Result type and Errors.

  /// The inner primitive value, in case it needs to be unwrapped from the
  /// type. Avoid using this member except to convert when a consumer
  /// requires it.
  primitive_type primitive_value = 0;
};

}  // namespace sus::num

// Promote i32 into the top-level namespace.
using sus::num::i32;

// clang-format off
template <char... C>
  requires requires {
    { ::sus::num::__private::BuildInteger<
        decltype(i32::primitive_value), i32::MAX_PRIMITIVE, C...>::value
    } -> std::same_as<const decltype(i32::primitive_value)&>;
  }
i32 inline constexpr operator"" _i32() noexcept {
  using Builder = ::sus::num::__private::BuildInteger<
    decltype(i32::primitive_value), i32::MAX_PRIMITIVE, C...>;
  return i32(Builder::value);
}
// clang-format on
