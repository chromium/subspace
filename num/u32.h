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

#include <concepts>

#include "concepts/from.h"
#include "concepts/into.h"
#include "num/__private/literals.h"
#include "num/__private/unsigned_integer_macros.h"
#include "num/integer_concepts.h"

namespace sus::num {

/// A 32-bit signed integer.
struct u32 {
 private:
  /// The underlying primitive type.
  using primitive_type = uint32_t;

 public:
  /// Default constructor, which sets the integer to 0.
  ///
  /// The trivial copy and move constructors are implicitly declared, as is the
  /// trivial destructor.
  constexpr inline u32() noexcept = default;

  /// Construction from the underlying primitive type.
  template <class T>
    requires(std::same_as<T, primitive_type>)  // Prevent implicit conversions.
  constexpr inline u32(T val) noexcept : primitive_value(val) {}

  /// Assignment from the underlying primitive type.
  template <class T>
    requires(std::same_as<T, primitive_type>)  // Prevent implicit conversions.
  constexpr inline void operator=(T v) noexcept {
    primitive_value = v;
  }

  template <class T>
    requires(std::same_as<T, size_t>)  // Prevent implicit conversions.
  static constexpr u32 from(T v) {
    ::sus::check(v >= T{MIN_PRIMITIVE});
    ::sus::check(v <= T{MAX_PRIMITIVE});
    return u32(static_cast<primitive_type>(v));
  }

  // TODO: Split apart the declarations and the definitions, so they can be in
  // u32_defn.h and u32_impl.h, allowing most of the library to just use
  // u32_defn.h which will keep headers smaller.
  _sus__unsigned_constants(u32, 0xFFFFFFFF);
  _sus__unsigned_impl(u32, sizeof(primitive_type), /*SignedT=*/i32,
                      /*LargerT=*/uint64_t);

  // TODO: overflowing_div_euclid().
  // TODO: overflowing_rem_euclid().
  // TODO: div_euclid().
  // TODO: rem_euclid().
  // TODO: wrapping_div_euclid().
  // TODO: wrapping_rem_euclid().
  // TODO: checked_div_euclid().
  // TODO: checked_rem_euclid().
  // TODO: checked_next_multiple_of().
  // TODO: from_str_radix(). Need Result type and Errors.
  // TODO: next_power_of_two().
  // TODO: checked_add_signed() and friends?

  /// The inner primitive value, in case it needs to be unwrapped from the
  /// type. Avoid using this member except to convert when a consumer
  /// requires it.
  primitive_type primitive_value = 0;
};

}  // namespace sus::num

// Promote u32 into the top-level namespace.
using sus::num::u32;

// clang-format off
template <char... C>
  requires requires {
    { ::sus::num::__private::BuildInteger<
        decltype(u32::primitive_value), u32::MAX_PRIMITIVE, C...>::value
    } -> std::same_as<const decltype(u32::primitive_value)&>;
  }
u32 inline constexpr operator"" _u32() noexcept {
  using Builder = ::sus::num::__private::BuildInteger<
    decltype(u32::primitive_value), u32::MAX_PRIMITIVE, C...>;
  return u32(Builder::value);
}
// clang-format on
