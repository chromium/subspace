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
#include "subspace/num/integer_concepts.h"
#include "subspace/num/try_from_int_error.h"
#include "subspace/option/option.h"
#include "subspace/result/result.h"

namespace sus::containers {
template <class T, size_t N>
  requires(N <= size_t{PTRDIFF_MAX})
class Array;
}

namespace sus::num {

struct u8;

// TODO: from_str_radix(). Need Result typ`e and Errors.

/// A 32-bit unsigned integer.
struct u32 final {
#define _self u32
#define _primitive uint32_t
#define _signed i32
#include "subspace/num/__private/unsigned_integer_methods.inc"
};

/// An 8-bit unsigned integer.
struct u8 final {
#define _self u8
#define _primitive uint8_t
#define _signed i8
#include "subspace/num/__private/unsigned_integer_methods.inc"
};


/// A 16-bit unsigned integer.
struct u16 final {
#define _self u16
#define _primitive uint16_t
#define _signed i16
#include "subspace/num/__private/unsigned_integer_methods.inc"
};

/// A 64-bit unsigned integer.
struct u64 final {
#define _self u64
#define _primitive uint64_t
#define _signed i64
#include "subspace/num/__private/unsigned_integer_methods.inc"
};

/// A pointer-sized unsigned integer.
struct usize final {
#define _self usize
#define _primitive ::sus::num::__private::ptr_type<>::unsigned_type
#define _signed isize
#include "subspace/num/__private/unsigned_integer_methods.inc"
};

/// Adds a `usize` to a pointer, returning the resulting pointer.
///
/// #[doc.overloads=ptr.add.usize]
template <class T>
sus_pure_const constexpr inline T* operator+(T* t, usize offset) {
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
sus_pure_const constexpr inline T* operator-(T* t, usize offset) {
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
