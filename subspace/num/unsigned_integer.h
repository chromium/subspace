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

#include <stddef.h>
#include <stdint.h>

#include <compare>
#include <functional>  // TODO: remove this but we need to hash things > size_t.

#include "subspace/assertions/check.h"
#include "subspace/assertions/endian.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/macros/pure.h"
#include "subspace/mem/size_of.h"
#include "subspace/num/__private/int_log10.h"
#include "subspace/num/__private/intrinsics.h"
#include "subspace/num/__private/literals.h"
#include "subspace/num/__private/ptr_type.h"
#include "subspace/num/__private/unsigned_integer_macros.h"
#include "subspace/num/integer_concepts.h"
#include "subspace/num/try_from_int_error.h"
#include "subspace/option/option.h"
#include "subspace/result/result.h"

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
_sus__unsigned_constants_decl(u32, /*PrimitiveT=*/uint32_t);

/// An 8-bit unsigned integer.
struct u8 final {
  _sus__unsigned_impl(u8, /*PrimitiveT=*/uint8_t, /*SignedT=*/i8);
};
_sus__unsigned_constants_decl(u8, /*PrimitiveT=*/uint8_t);

/// A 16-bit unsigned integer.
struct u16 final {
  _sus__unsigned_impl(u16, /*PrimitiveT=*/uint16_t, /*SignedT=*/i16);
};
_sus__unsigned_constants_decl(u16, /*PrimitiveT=*/uint16_t);

/// A 64-bit unsigned integer.
struct u64 final {
  _sus__unsigned_impl(u64, /*PrimitiveT=*/uint64_t,
                      /*SignedT=*/i64);
};
_sus__unsigned_constants_decl(u64, /*PrimitiveT=*/uint64_t);

/// A pointer-sized unsigned integer.
struct usize final {
  _sus__unsigned_impl(
      usize,
      /*PrimitiveT=*/::sus::num::__private::ptr_type<>::unsigned_type,
      /*SignedT=*/isize);
};
_sus__unsigned_constants_decl(
    usize, /*PrimitiveT=*/::sus::num::__private::ptr_type<>::unsigned_type);

/// Adds a `usize` to a pointer, returning the resulting pointer.
///
/// #[doc.overloads=ptr.add.usize]
template <class T>
[[nodiscard]] sus_pure_const constexpr inline T* operator+(T* t, usize offset) {
  return t + size_t{offset};
}

/// Adds a `usize` to a referenced pointer, and returns the input reference.
///
/// #[doc.overloads=ptr.add.usize]
template <class T>
constexpr inline T*& operator+=(T*& t, usize offset) {
  t += size_t{offset};
  return t;
}

/// Subtracts a `usize` from a pointer, returning the resulting pointer.
///
/// #[doc.overloads=ptr.sub.usize]
template <class T>
[[nodiscard]] sus_pure_const constexpr inline T* operator-(T* t, usize offset) {
  return t - size_t{offset};
}

/// Subtracts a `usize` from a referenced pointer, and returns the input
/// reference.
///
/// #[doc.overloads=ptr.sub.usize]
template <class T>
constexpr inline T*& operator-=(T*& t, usize offset) {
  t -= size_t{offset};
  return t;
}

}  // namespace sus::num

namespace std {
_sus__unsigned_hash_equal_to(::sus::num::u8);
_sus__unsigned_hash_equal_to(::sus::num::u16);
_sus__unsigned_hash_equal_to(::sus::num::u32);
_sus__unsigned_hash_equal_to(::sus::num::u64);
_sus__unsigned_hash_equal_to(::sus::num::usize);
}  // namespace std

_sus__integer_literal(u8, ::sus::num::u8);
_sus__integer_literal(u16, ::sus::num::u16);
_sus__integer_literal(u32, ::sus::num::u32);
_sus__integer_literal(u64, ::sus::num::u64);
_sus__integer_literal(usize, ::sus::num::usize);

// Promote unsigned integer types into the `sus` namespace.
namespace sus {
using sus::num::u16;
using sus::num::u32;
using sus::num::u64;
using sus::num::u8;
using sus::num::usize;
}  // namespace sus
