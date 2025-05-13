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

// IWYU pragma: private, include "sus/num/types.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <stddef.h>
#include <stdint.h>

#include <compare>
#include <functional>  // TODO: remove this but we need to hash things > size_t.

#include "fmt/format.h"
#include "sus/assertions/check.h"
#include "sus/iter/iterator_concept.h"
#include "sus/macros/pure.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/mem/size_of.h"
#include "sus/num/__private/check_integer_overflow.h"
#include "sus/num/__private/int_log10.h"
#include "sus/num/__private/intrinsics.h"
#include "sus/num/__private/literals.h"
#include "sus/num/__private/primitive_type.h"
#include "sus/num/integer_concepts.h"
#include "sus/num/try_from_int_error.h"
#include "sus/num/unsigned_integer.h"
#include "sus/num/unsigned_integer_consts.h"
#include "sus/option/option.h"
#include "sus/ptr/copy.h"
#include "sus/result/result.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus::num {

// TODO: from_str_radix(). Need Result type and Errors.

// TODO: div_ceil() and div_floor()? Lots of discussion still on
// https://github.com/rust-lang/rust/issues/88581 for signed types.

/// A 32-bit signed integer.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] i32 final {
#define _self i32
#define _primitive int32_t
#define _unsigned u32
#include "sus/num/__private/signed_integer_methods.inc"
};
#define _self i32
#define _primitive int32_t
#include "sus/num/__private/signed_integer_consts.inc"

/// An 8-bit signed integer.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] i8 final {
#define _self i8
#define _primitive int8_t
#define _unsigned u8
#include "sus/num/__private/signed_integer_methods.inc"
};
#define _self i8
#define _primitive int8_t
#include "sus/num/__private/signed_integer_consts.inc"

/// A 16-bit signed integer.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] i16 final {
#define _self i16
#define _primitive int16_t
#define _unsigned u16
#include "sus/num/__private/signed_integer_methods.inc"
};
#define _self i16
#define _primitive int16_t
#include "sus/num/__private/signed_integer_consts.inc"

/// A 64-bit signed integer.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] i64 final {
#define _self i64
#define _primitive int64_t
#define _unsigned u64
#include "sus/num/__private/signed_integer_methods.inc"
};
#define _self i64
#define _primitive int64_t
#include "sus/num/__private/signed_integer_consts.inc"

/// An address-sized signed integer.
///
/// This type is capable of holding any offset or distance in a single memory
/// allocation, since memory allocations are bounded at
/// [`isize::MAX`]($sus::num::isize::MAX).
///
/// Note that it is possible for a pointer to be larger than an address under
/// some architectures, with a pointer holding additional data such as
/// capabilities. See [CHERI](
/// https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-947.pdf) for an example. So
/// this type is not always the same size as a pointer.
///
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] isize final {
#define _self isize
#define _primitive ::sus::num::__private::addr_type<>::signed_type
#define _unsigned usize
#include "sus/num/__private/signed_integer_methods.inc"

  /// Satisfies the [`AddAssign`]($sus::num::AddAssign) concept for pointers
  /// (`T*`) with [`isize`]($sus::num::isize).
  ///
  /// Adds a [`isize`]($sus::num::isize) to a referenced pointer, and returns
  /// the input reference.
  ///
  /// #[doc.overloads=ptr.add.isize]
  template <class T>
  friend constexpr T*& operator+=(T*& t, isize offset) {
    t += ptrdiff_t{offset};
    return t;
  }

  /// Satisfies the [`Sub`]($sus::num::Sub) concept for pointers
  /// (`T*`) with [`isize`]($sus::num::isize).
  ///
  /// Subtracts a [`isize`]($sus::num::isize) from a pointer, returning the
  /// resulting pointer.
  ///
  /// #[doc.overloads=ptr.sub.isize]
  template <class T>
  __sus_pure_const friend constexpr T* operator-(T* t, isize offset) {
    return t - ptrdiff_t{offset};
  }

  /// Satisfies the [`SubAssign`]($sus::num::SubAssign) concept for pointers
  /// (`T*`) with [`isize`]($sus::num::isize).
  ///
  /// Subtracts a [`isize`]($sus::num::isize) from a referenced pointer, and
  /// returns the input reference.
  ///
  /// #[doc.overloads=ptr.sub.isize]
  template <class T>
  friend constexpr T*& operator-=(T*& t, isize offset) {
    t -= ptrdiff_t{offset};
    return t;
  }

  /// Satisfies the [`Add`]($sus::num::Add) concept for pointers
  /// (`T*`) with [`isize`]($sus::num::isize).
  ///
  /// Adds a [`isize`]($sus::num::isize) to a pointer, returning the resulting
  /// pointer.
  ///
  /// #[doc.overloads=ptr.add.isize]
  template <class T>
  __sus_pure_const friend constexpr T* operator+(T* t, isize offset) {
    return t + ptrdiff_t{offset};
  }
};
#define _self isize
#define _primitive ::sus::num::__private::addr_type<>::signed_type
#include "sus/num/__private/signed_integer_consts.inc"
}  // namespace sus::num

/// For writing [`i8`]($sus::num::i8) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`)
/// unless a literal suffix modifies them, such as with `_i8` which creates
/// an 8-bit value.
///
/// Values out of range for [`i8`]($sus::num::i8) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_i8 - (5_i8).abs();
/// sus_check(i == 118_i8);
/// ```
_sus__integer_literal(i8, ::sus::num::i8);
/// For writing [`i16`]($sus::num::i16) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`)
/// unless a literal suffix modifies them, such as with `_i16` which creates
/// an 16-bit value.
///
/// Values out of range for [`i16`]($sus::num::i16) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_i16 - (5_i16).abs();
/// sus_check(i == 118_i16);
/// ```
_sus__integer_literal(i16, ::sus::num::i16);
/// For writing [`i32`]($sus::num::i32) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`), unless
/// a literal suffix modifies them. The `_i32` suffix creates an 32-bit value
/// which is the same as not adding a suffix at all, except that it forces a
/// safe numeric type instead of a primitive value when this is needed (such as
/// for templates or member function access).
///
/// Values out of range for [`i32`]($sus::num::i32) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_i32 - (5_i32).abs();
/// sus_check(i == 118_i32);
/// ```
_sus__integer_literal(i32, ::sus::num::i32);
/// For writing [`i64`]($sus::num::i64) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`), unless
/// a literal suffix modifies them, such as with `_i64` which creates a 64-bit
/// value. On Windows, this is the same as the `l` suffix (which makes a
/// `long`) but is platform agnostic, and forces a safe numeric type instead of
/// a primitive value when this is needed (such as for templates or member
/// function access).
///
/// Values out of range for [`i64`]($sus::num::i64) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_i64 - (5_i64).abs();
/// sus_check(i == 118_i64);
/// ```
_sus__integer_literal(i64, ::sus::num::i64);
/// For writing [`isize`]($sus::num::isize) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`), unless
/// a literal suffix modifies them, such as with `_isize` which creates an
/// address-sized value. This is 32 bits for 32-bit targets and 64 bits for
/// 64-bit targets.
///
/// Values out of range for [`isize`]($sus::num::isize) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_isize - (5_isize).abs();
/// sus_check(i == 118_isize);
/// ```
_sus__integer_literal(isize, ::sus::num::isize);

// Promote signed integer types into the `sus` namespace.
namespace sus {
using sus::num::i16;
using sus::num::i32;
using sus::num::i64;
using sus::num::i8;
using sus::num::isize;
}  // namespace sus
