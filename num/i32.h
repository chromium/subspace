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

#include "num/__private/literals.h"
#include "num/__private/signed_integer_macros.h"

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

  // TODO: Split apart the declarations and the definitions, so they can be in
  // i32_defn.h and i32_impl.h, allowing most of the library to just use
  // i32_defn.h which will keep headers smaller.
  _sus__signed_constants(i32, 0x7FFFFFFF, 32);
  _sus__signed_impl(i32, /*LargerT=*/int64_t,
                    /*UnsignedT=*//* TODO: u32 */ uint32_t);

  /** Converts an integer from big endian to the target’s endianness.
*
* On big endian this is a no-op. On little endian the bytes are swapped.
*/
  static constexpr i32 from_be(const i32& x) noexcept {
      return x;
  }

  // TODO: overflowing_div_euclid().
  // TODO: overflowing_rem_euclid().
  // TODO: div_euclid().
  // TODO: rem_euclid().
  // TODO: wrapping_div_euclid().
  // TODO: wrapping_rem_euclid().
  // TODO: checked_div_euclid().
  // TODO: checked_rem_euclid().
  // TODO: checked_next_multiple_of().
  // TODO: from_be().
  // TODO: from_be_bytes().
  // TODO: from_le().
  // TODO: from_le_bytes().
  // TODO: from_ne_bytes().
  // TODO: from_str_radix(). Need Result type and Errors.
  // TODO: to_be().
  // TODO: to_be_bytes().
  // TODO: to_le().
  // TODO: to_le_bytes().
  // TODO: to_ne_bytes().

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
