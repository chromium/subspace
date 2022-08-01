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
};

/// An 8-bit unsigned integer.
struct u8 final {
  _sus__unsigned_impl(u8, /*PrimitiveT=*/uint8_t, /*SignedT=*/i8);
};

/// A 16-bit unsigned integer.
struct u16 final {
  _sus__unsigned_impl(u16, /*PrimitiveT=*/uint16_t, /*SignedT=*/i16);
};

/// A 64-bit unsigned integer.
struct u64 final {
  _sus__unsigned_impl(u64, /*PrimitiveT=*/uint64_t,
                      /*SignedT=*/i64);
};

/// A pointer-sized unsigned integer.
struct usize final {
  _sus__unsigned_impl(
      usize,
      /*PrimitiveT=*/::sus::num::__private::ptr_type<>::unsigned_type,
      /*SignedT=*/isize);
};

}  // namespace sus::num

_sus__unsigned_literal(u8, ::sus::num::u8, /*PrimitiveT=*/uint8_t);
_sus__unsigned_literal(u16, ::sus::num::u16, /*PrimitiveT=*/uint16_t);
_sus__unsigned_literal(u32, ::sus::num::u32, /*PrimitiveT=*/uint32_t);
_sus__unsigned_literal(u64, ::sus::num::u64, /*PrimitiveT=*/uint64_t);
_sus__unsigned_literal(
    usize, ::sus::num::usize,
    /*PrimitiveT=*/::sus::num::__private::ptr_type<>::unsigned_type);

// Promote unsigned integer types into the top-level namespace.
using sus::num::u16;
using sus::num::u32;
using sus::num::u64;
using sus::num::u8;
using sus::num::usize;
