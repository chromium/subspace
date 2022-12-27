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

#include "num/__private/signed_integer_macros.h"

namespace sus::num {

// TODO: from_str_radix(). Need Result type and Errors.

// TODO: div_ceil() and div_floor()? Lots of discussion still on
// https://github.com/rust-lang/rust/issues/88581 for signed types.

// TODO: Split apart the declarations and the definitions? Then they can be in
// u32_defn.h and u32_impl.h, allowing most of the library to just use
// u32_defn.h which will keep some headers smaller. But then the combined
// headers are larger, is that worse?

/// A 32-bit signed integer.
struct i32 final {
  _sus__signed_impl(i32, /*PrimitiveT=*/int32_t, /*UnsignedT=*/u32);
};

/// An 8-bit signed integer.
struct i8 final {
  _sus__signed_impl(i8, /*PrimitiveT=*/int8_t, /*UnsignedT=*/u8);
};

/// A 16-bit signed integer.
struct i16 final {
  _sus__signed_impl(i16, /*PrimitiveT=*/int16_t, /*UnsignedT=*/u16);
};

/// A 64-bit signed integer.
struct i64 final {
  _sus__signed_impl(i64, /*PrimitiveT=*/int64_t, /*UnsignedT=*/u64);
};

/// A pointer-sized signed integer.
struct isize final {
  _sus__signed_impl(
      isize,
      /*PrimitiveT=*/::sus::num::__private::ptr_type<>::signed_type,
      /*UnsignedT=*/usize);

  /** Converts to its primitive value explicitly. */
  constexpr inline explicit operator decltype(primitive_value)() noexcept {
    return primitive_value;
  }
};

}  // namespace sus::num

namespace std {
_sus__signed_hash_equal_to(::sus::num::i8);
_sus__signed_hash_equal_to(::sus::num::i16);
_sus__signed_hash_equal_to(::sus::num::i32);
_sus__signed_hash_equal_to(::sus::num::i64);
_sus__signed_hash_equal_to(::sus::num::isize);
}  // namespace std

_sus__integer_literal(i8, ::sus::num::i8);
_sus__integer_literal(i16, ::sus::num::i16);
_sus__integer_literal(i32, ::sus::num::i32);
_sus__integer_literal(i64, ::sus::num::i64);
_sus__integer_literal(isize, ::sus::num::isize);

// Promote signed integer types into the `sus` namespace.
namespace sus {
using sus::num::i16;
using sus::num::i32;
using sus::num::i64;
using sus::num::i8;
using sus::num::isize;
}  // namespace sus
