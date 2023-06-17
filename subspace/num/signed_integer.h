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

#include "fmt/format.h"
#include "subspace/assertions/check.h"
#include "subspace/assertions/endian.h"
#include "subspace/macros/pure.h"
#include "subspace/mem/relocate.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/size_of.h"
#include "subspace/num/__private/int_log10.h"
#include "subspace/num/__private/intrinsics.h"
#include "subspace/num/__private/literals.h"
#include "subspace/num/integer_concepts.h"
#include "subspace/num/try_from_int_error.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/option/option.h"
#include "subspace/ptr/copy.h"
#include "subspace/result/result.h"
#include "subspace/string/__private/format_to_stream.h"

namespace sus::num {

// TODO: from_str_radix(). Need Result type and Errors.

// TODO: div_ceil() and div_floor()? Lots of discussion still on
// https://github.com/rust-lang/rust/issues/88581 for signed types.

// TODO: Split apart the declarations and the definitions? Then they can be in
// u32_defn.h and u32_impl.h, allowing most of the library to just use
// u32_defn.h which will keep some headers smaller. But then the combined
// headers are larger, is that worse?

/// A 32-bit signed integer.
struct sus_trivial_abi i32 final {
#define _self i32
#define _primitive int32_t
#define _unsigned u32
#include "subspace/num/__private/signed_integer_methods.inc"
};
#define _self i32
#define _primitive int32_t
#include "subspace/num/__private/signed_integer_consts.inc"

/// An 8-bit signed integer.
struct sus_trivial_abi i8 final {
#define _self i8
#define _primitive int8_t
#define _unsigned u8
#include "subspace/num/__private/signed_integer_methods.inc"
};
#define _self i8
#define _primitive int8_t
#include "subspace/num/__private/signed_integer_consts.inc"

/// A 16-bit signed integer.
struct sus_trivial_abi i16 final {
#define _self i16
#define _primitive int16_t
#define _unsigned u16
#include "subspace/num/__private/signed_integer_methods.inc"
};
#define _self i16
#define _primitive int16_t
#include "subspace/num/__private/signed_integer_consts.inc"

/// A 64-bit signed integer.
struct sus_trivial_abi i64 final {
#define _self i64
#define _primitive int64_t
#define _unsigned u64
#include "subspace/num/__private/signed_integer_methods.inc"
};
#define _self i64
#define _primitive int64_t
#include "subspace/num/__private/signed_integer_consts.inc"

/// An address-sized signed integer.
///
/// This type is capable of holding any offset or distance in a single memory
/// allocation, since memory allocations are bounded at `isize::MAX`.
///
/// Note that it is possible for a pointer to be larger than an address under
/// some architectures, with a pointer holding additional data such as
/// capabilities. See [CHERI](
/// https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-947.pdf) for an example. So
/// this type is not always the same size as a pointer.
struct sus_trivial_abi isize final {
#define _self isize
#define _primitive ::sus::num::__private::ptr_type<>::signed_type
#define _unsigned usize
#include "subspace/num/__private/signed_integer_methods.inc"
};
#define _self isize
#define _primitive ::sus::num::__private::ptr_type<>::signed_type
#include "subspace/num/__private/signed_integer_consts.inc"

/// Adds a `isize` to a pointer, returning the resulting pointer.
///
/// #[doc.overloads=ptr.add.isize]
template <class T>
sus_pure_const constexpr inline T* operator+(T* t, isize offset) {
  return t + ptrdiff_t{offset};
}

/// Adds a `isize` to a referenced pointer, and returns the input reference.
///
/// #[doc.overloads=ptr.add.isize]
template <class T>
constexpr inline T*& operator+=(T*& t, isize offset) {
  t += ptrdiff_t{offset};
  return t;
}

/// Subtracts a `isize` from a pointer, returning the resulting pointer.
///
/// #[doc.overloads=ptr.sub.isize]
template <class T>
sus_pure_const constexpr inline T* operator-(T* t, isize offset) {
  return t - ptrdiff_t{offset};
}

/// Subtracts a `isize` from a referenced pointer, and returns the input
/// reference.
///
/// #[doc.overloads=ptr.sub.isize]
template <class T>
constexpr inline T*& operator-=(T*& t, isize offset) {
  t -= ptrdiff_t{offset};
  return t;
}

}  // namespace sus::num

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
