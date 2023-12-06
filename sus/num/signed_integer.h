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
struct [[sus_trivial_abi]] i32 final {
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
struct [[sus_trivial_abi]] i8 final {
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
struct [[sus_trivial_abi]] i16 final {
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
struct [[sus_trivial_abi]] i64 final {
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
struct [[sus_trivial_abi]] isize final {
#define _self isize
#define _primitive ::sus::num::__private::addr_type<>::signed_type
#define _unsigned usize
#include "sus/num/__private/signed_integer_methods.inc"
};
#define _self isize
#define _primitive ::sus::num::__private::addr_type<>::signed_type
#include "sus/num/__private/signed_integer_consts.inc"

/// Satisfies the [`Add`]($sus::num::Add) concept for pointers
/// (`T*`) with [`isize`]($sus::num::isize).
///
/// Adds a [`isize`]($sus::num::isize) to a pointer, returning the resulting
/// pointer.
///
/// #[doc.overloads=ptr.add.isize]
template <class T, Signed S>
  requires(std::constructible_from<isize, S>)
__sus_pure_const constexpr inline T* operator+(T* t, S offset) {
  return t + ptrdiff_t{offset};
}

/// Satisfies the [`AddAssign`]($sus::num::AddAssign) concept for pointers
/// (`T*`) with [`isize`]($sus::num::isize).
///
/// Adds a [`isize`]($sus::num::isize) to a referenced pointer, and returns
/// the input reference.
///
/// #[doc.overloads=ptr.add.isize]
template <class T>
constexpr inline T*& operator+=(T*& t, isize offset) {
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
__sus_pure_const constexpr inline T* operator-(T* t, isize offset) {
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
constexpr inline T*& operator-=(T*& t, isize offset) {
  t -= ptrdiff_t{offset};
  return t;
}

/// Satisfies the [`Shl`]($sus::num::Shl) concept for signed primitive integers
/// shifted by [`u64`]($sus::num::u64).
/// #[doc.overloads=signed.prim.<<u64]
template <class P, Integer U>
  requires((SignedPrimitiveInteger<P> || SignedPrimitiveEnum<P>) &&
           std::convertible_to<U, u64>)
[[nodiscard]] __sus_pure_const constexpr inline P operator<<(P l, U r) noexcept {
  // No UB checks on primitive types, since there's no promotion to a Subspace
  // return type?
  return l << u64(r).primitive_value;
}
/// #[doc.overloads=signed.prim.<<u64]
template <class P, Integer U>
  requires((SignedPrimitiveInteger<P> || SignedPrimitiveEnum<P>) &&
           !std::convertible_to<U, u64>)
constexpr inline P operator<<(P l, U r) noexcept = delete;

/// Satisfies the [`Shr`]($sus::num::Shr) concept for signed primitive integers
/// shifted by [`u64`]($sus::num::u64).
///
/// Performs sign extension, copying the sign bit to the right if its set.
/// #[doc.overloads=signed.prim.>>u64]
template <class P, Integer U>
  requires((SignedPrimitiveInteger<P> || SignedPrimitiveEnum<P>) &&
           std::convertible_to<U, u64>)
[[nodiscard]] __sus_pure_const constexpr inline P operator>>(P l, U r) noexcept {
  // No UB checks on primitive types, since there's no promotion to a Subspace
  // return type?
  return l >> u64(r).primitive_value;
}
/// #[doc.overloads=signed.prim.>>u64]
template <class P, Integer U>
  requires((SignedPrimitiveInteger<P> || SignedPrimitiveEnum<P>) &&
           !std::convertible_to<U, u64>)
constexpr inline P operator>>(P l, U r) noexcept = delete;

/// Satisfies the [`Shl`]($sus::num::Shl) concept for signed integers.
///
/// This operation supports shifting with primitive signed or unsigned integers
/// that convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// Thus the bound is `std::convertible_to` (implicit conversion) instead of
/// `sus::construct::From` (explicit conversion).
///
/// # Panics
/// This function will panic when `r` is not less than the number of bits in `l`
/// if overflow checks are enabled (they are by default) and will perform a
/// wrapping shift if overflow checks are disabled (not the default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
///
/// #[doc.overloads=signedint.<<]
[[nodiscard]] __sus_pure_const constexpr inline i8 operator<<(
    i8 l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shl_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift left with overflow");
    return out.value;
  } else {
    return l.wrapping_shl(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.<<]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline i8 operator<<(i8 l, U r) noexcept = delete;
/// Satisfies the [`Shr`]($sus::num::Shr) concept for signed integers.
///
/// Performs sign extension, copying the sign bit to the right if its set.
///
/// # Panics
/// This function will panic when `r` is not less than the number of bits in `l`
/// if overflow checks are enabled (they are by default) and will perform a
/// wrapping shift if overflow checks are disabled (not the default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
///
/// #[doc.overloads=signedint.>>]
[[nodiscard]] __sus_pure_const constexpr inline i8 operator>>(
    i8 l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shr_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift right with overflow");
    return out.value;
  } else {
    return l.wrapping_shr(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.>>]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline i8 operator>>(i8 l, U r) noexcept = delete;
/// #[doc.overloads=signedint.<<]
[[nodiscard]] __sus_pure_const constexpr inline i16 operator<<(
    i16 l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shl_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift left with overflow");
    return out.value;
  } else {
    return l.wrapping_shl(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.<<]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline i16 operator<<(i16 l, U r) noexcept = delete;
/// #[doc.overloads=signedint.>>]
[[nodiscard]] __sus_pure_const constexpr inline i16 operator>>(
    i16 l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shr_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift right with overflow");
    return out.value;
  } else {
    return l.wrapping_shr(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.>>]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline i16 operator>>(i16 l, U r) noexcept = delete;
/// #[doc.overloads=signedint.<<]
[[nodiscard]] __sus_pure_const constexpr inline i32 operator<<(
    i32 l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shl_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift left with overflow");
    return out.value;
  } else {
    return l.wrapping_shl(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.<<]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline i32 operator<<(i32 l, U r) noexcept = delete;
/// #[doc.overloads=signedint.>>]
[[nodiscard]] __sus_pure_const constexpr inline i32 operator>>(
    i32 l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shr_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift right with overflow");
    return out.value;
  } else {
    return l.wrapping_shr(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.>>]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline i32 operator>>(i32 l, U r) noexcept = delete;
/// #[doc.overloads=signedint.<<]
[[nodiscard]] __sus_pure_const constexpr inline i64 operator<<(
    i64 l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shl_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift left with overflow");
    return out.value;
  } else {
    return l.wrapping_shl(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.<<]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline i64 operator<<(i64 l, U r) noexcept = delete;
/// #[doc.overloads=signedint.>>]
[[nodiscard]] __sus_pure_const constexpr inline i64 operator>>(
    i64 l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shr_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift right with overflow");
    return out.value;
  } else {
    return l.wrapping_shr(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.>>]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline i64 operator>>(i64 l, U r) noexcept = delete;
/// #[doc.overloads=signedint.<<]
[[nodiscard]] __sus_pure_const constexpr inline isize operator<<(
    isize l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shl_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift left with overflow");
    return out.value;
  } else {
    return l.wrapping_shl(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.<<]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline isize operator<<(isize l, U r) noexcept = delete;
/// #[doc.overloads=signedint.>>]
[[nodiscard]] __sus_pure_const constexpr inline isize operator>>(
    isize l, std::convertible_to<u64> auto r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shr_with_overflow(l.primitive_value, u64(r).primitive_value);
    sus_check_with_message(!out.overflow,
                              "attempt to shift right with overflow");
    return out.value;
  } else {
    return l.wrapping_shr(u64(r).primitive_value);
  }
}
/// #[doc.overloads=signedint.>>]
template <class U>
  requires(!std::convertible_to<U, u64>)
constexpr inline isize operator>>(isize l, U r) noexcept = delete;

}  // namespace sus::num

/// For writing [`i8`]($sus::num::i8) literals.
_sus__integer_literal(i8, ::sus::num::i8);
/// For writing [`i16`]($sus::num::i16) literals.
_sus__integer_literal(i16, ::sus::num::i16);
/// For writing [`i32`]($sus::num::i32) literals.
_sus__integer_literal(i32, ::sus::num::i32);
/// For writing [`i64`]($sus::num::i64) literals.
_sus__integer_literal(i64, ::sus::num::i64);
/// For writing [`isize`]($sus::num::isize) literals.
_sus__integer_literal(isize, ::sus::num::isize);

// Promote signed integer types into the `sus` namespace.
namespace sus {
using sus::num::i16;
using sus::num::i32;
using sus::num::i64;
using sus::num::i8;
using sus::num::isize;
}  // namespace sus
