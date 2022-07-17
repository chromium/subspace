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
#include "num/integer_concepts.h"

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
  _sus__signed_constants(i32, 0x7FFFFFFF);
  _sus__signed_impl(i32, sizeof(primitive_type), /*LargerT=*/int64_t,
                    /*UnsignedT=*//* TODO: u32 */ uint32_t,
                    /*UnsignedSusT=*/u32);

  // TODO: from_str_radix(). Need Result type and Errors.

  // TODO: div_ceil() and div_floor()? Lots of discussion still on
  // https://github.com/rust-lang/rust/issues/88581 for signed types.

  /// The inner primitive value, in case it needs to be unwrapped from the type.
  /// Avoid using this member except to convert when a consumer requires it.
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
