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

#include "num/__private/unsigned_integer_macros.h"

namespace sus::num {

// TODO: from_str_radix(). Need Result type and Errors.

// TODO: Split apart the declarations and the definitions? Then they can be in
// u32_defn.h and u32_impl.h, allowing most of the library to just use
// u32_defn.h which will keep some headers smaller. But then the combined
// headers are larger, is that worse?

/// A 32-bit unsigned integer.
struct u32 final {
  _sus__unsigned_impl(u32, /*PrimitiveT=*/uint32_t, /*SignedT=*/i32);

  /** Construction from the underlying primitive type.
   */
  template <std::same_as<decltype(primitive_value)>
                P>  // Prevent implicit conversions.
  constexpr inline u32(P val) noexcept : primitive_value(val) {}
};

/// An 8-bit unsigned integer.
struct u8 final {
  _sus__unsigned_impl(u8, /*PrimitiveT=*/uint8_t, /*SignedT=*/i8);

  /** Construction from the underlying primitive type.
   */
  template <std::same_as<decltype(primitive_value)>
                P>  // Prevent implicit conversions.
  constexpr inline u8(P val) noexcept : primitive_value(val) {}
};

/// A 16-bit unsigned integer.
struct u16 final {
  _sus__unsigned_impl(u16, /*PrimitiveT=*/uint16_t, /*SignedT=*/i16);

  /** Construction from the underlying primitive type.
   */
  template <std::same_as<decltype(primitive_value)>
                P>  // Prevent implicit conversions.
  constexpr inline u16(P val) noexcept : primitive_value(val) {}
};

/// A 64-bit unsigned integer.
struct u64 final {
  _sus__unsigned_impl(u64, /*PrimitiveT=*/uint64_t,
                      /*SignedT=*/i64);

  /** Construction from the underlying primitive type.
   */
  template <std::same_as<decltype(primitive_value)>
                P>  // Prevent implicit conversions.
  constexpr inline u64(P val) noexcept : primitive_value(val) {}
};

/// A pointer-sized unsigned integer.
struct usize final {
  _sus__unsigned_impl(
      usize,
      /*PrimitiveT=*/::sus::num::__private::ptr_type<>::unsigned_type,
      /*SignedT=*/isize);

  /** Construction from an unsigned literal. */
  constexpr inline usize(uint8_t val) noexcept
      : primitive_value(static_cast<decltype(primitive_value)>(val)) {}
  /** Construction from an unsigned literal. */
  constexpr inline usize(uint16_t val) noexcept
      : primitive_value(static_cast<decltype(primitive_value)>(val)) {}
  /** Construction from an unsigned literal. */
  constexpr inline usize(uint32_t val) noexcept
      : primitive_value(static_cast<decltype(primitive_value)>(val)) {}
  /** Construction from an unsigned literal. */
  constexpr inline usize(uint64_t val)
      : primitive_value(static_cast<decltype(primitive_value)>(val)) {
    if (std::is_constant_evaluated()) {
      if (val > uint64_t{MAX_PRIMITIVE}) [[unlikely]]
        throw "usize construction from literal is out of bounds";
    } else {
      check(val <= uint64_t{MAX_PRIMITIVE});
    }
  }

  /** Construction from size_t which can differ from sized integer types. */
  template <std::same_as<size_t> T>
  constexpr inline usize(T val) noexcept
    requires(sizeof(T) == sizeof(uint32_t) &&
             !std::same_as<T, decltype(primitive_value)>)
  : usize(uint32_t{val}) {}
  template <std::same_as<size_t> T>
  constexpr inline usize(size_t val)
    requires(sizeof(T) == sizeof(uint64_t) &&
             !std::same_as<T, decltype(primitive_value)>)
  : usize(uint64_t{val}) {}

  /** Converts to its primitive value explicitly. */
  constexpr inline explicit operator decltype(
      primitive_value)() const noexcept {
    return primitive_value;
  }
  /** Converts to size_t explicitly, which can differ from sized integer types.
   */
  template <std::same_as<size_t> T>
  constexpr inline explicit operator T() const noexcept
    requires(sizeof(size_t) >= sizeof(uint64_t) &&
             !std::same_as<size_t, decltype(primitive_value)>)
  {
    return size_t{primitive_value};
  }
};

}  // namespace sus::num

_sus__integer_literal(u8, ::sus::num::u8);
_sus__integer_literal(u16, ::sus::num::u16);
_sus__integer_literal(u32, ::sus::num::u32);
_sus__integer_literal(u64, ::sus::num::u64);
_sus__integer_literal(usize, ::sus::num::usize);

// Promote unsigned integer types into the top-level namespace.
using sus::num::u16;
using sus::num::u32;
using sus::num::u64;
using sus::num::u8;
using sus::num::usize;
