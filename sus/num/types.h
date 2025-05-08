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

// IWYU pragma: private, include "sus/prelude.h"
// IWYU pragma: friend "sus/.*"
#pragma once

namespace sus {

/// Safe integer (e.g. [`i32`]($sus::num::i32)) and floating point
/// (e.g. [`f32`]($sus::num::f32)) numerics, and numeric concepts.
///
/// This namespace contains safe integer types and floating point types.
///
/// Safe numeric types:
/// * Signed integers: [`i8`]($sus::num::i8), [`i16`]($sus::num::i16),
///   [`i32`]($sus::num::i32), [`i64`]($sus::num::i64),
///   [`isize`]($sus::num::isize).
/// * Unsigned integers: [`u8`]($sus::num::u8), [`u16`]($sus::num::u16),
///   [`u32`]($sus::num::u32), [`u64`]($sus::num::u64),
///   [`usize`]($sus::num::usize), [`uptr`]($sus::num::uptr).
/// * Floating point: [`f32`]($sus::num::f32), [`f64`]($sus::num::f64).
/// * Portability helper: [`CInt`]
///
/// Additionally, there are Concepts that match against safe numerics, C++
/// primitive types, and operations with numeric types.
///
/// The Subspace library numeric types can interoperate with primitive C++
/// types, but are safer than primitive C++ types and eliminate many classes of
/// bugs that often lead to security vulnerabilities:
/// * Integer overflow is not allowed by default (see [Overflow behaviour](
///   #overflow-behaviour)), and will [`panic`]($sus::panic) to terminate the
///   program.
///   Intentional overflow can be achieved through methods like
///   [`wrapping_add`]($sus::num::i32::wrapping_add) or
///   [`saturating_mul`]($sus::num::i32::saturating_mul). The
///   [`OverflowInteger`]($sus::num::OverflowInteger) type can be used for a
///   series of potentially-overflowing operations and unwraps to an integer
///   value if-and-only-if no overflow has occured.
/// * Integers and floats convert implicitly into each other or into primitive
///   types *only* when no data can be lost, otherwise conversions do not
///   compile. To convert fallibly and observe data loss, use the
///   [`TryFrom`]($sus::construct::TryFrom) concept methods, such as
///   `u32::try_from(3_i32)`. To do casting conversions with truncation, use
///   [`Cast`]($sus::construct::Cast).
/// * No integer promotion. Math on 8-bit and 16-bit integers will not change
///   their type, unlike primitive types which convert to (signed) int on any
///   math operation.
/// * No Undefined Behaviour in conversions. Conversions between all numeric
///   types, and between them and primitive types is well-defined for all
///   possible values, unlike conversions between primitive integer and
///   floating point types which can result in Undefined Behaviour.
///
/// The numeric types also come with builtin methods to perform common
/// operations, such as [`abs`]($sus::num::i32::abs),
/// [`pow`]($sus::num::i32::pow), [`log10`]($sus::num::i32::log10), or
/// [`leading_ones`]($sus::num::i32::leading_ones).
///
/// # Overflow behaviour
///
/// The default build configuration will panic on integer overflow in arithmetic
/// operations (`+`, `-`, `*`, `/`, etc). These checks can be disabled by
/// defining `SUS_CHECK_INTEGER_OVERFLOW` to false during compilation. Both
/// signed and unsigned integers will then overflow by performing wrapping
/// operations. There is no Undefined Behaviour with signed or unsigned integers
/// unless going through the unchecked operations explicitly, such as
/// [`unchecked_add`]($sus::num::i32::unchecked_add).
///
/// Division by zero, or overflow in integer division will panic regardless of
/// whether overflow checks are enabled.
///
/// # Conversions
///
/// To explicitly invoke a lossless conversion, use
/// [`From`]($sus::construct::From). Use [`Into`]($sus::construct::Into) to
/// constrain inputs in generic code, and [`sus::into()`]($sus::construct::into)
/// to type-deduce for conversions. Some lossless conversions are also allowed
/// to happen implicitly, though explicit conversion is better.
///
/// To convert and handle the case where data is lost, use
/// [`TryFrom`]($sus::construct::TryFrom), or
/// [`TryInto`]($sus::construct::TryInto) in generic code. Using
/// `T::try_from(U).unwrap()` is a quick way to convert and find out if the
/// value was out of range, or to terminate on malicious inputs. Or
/// `T::try_from(U).unwrap_or_default()` to convert to the input value or else
/// to zero.
///
/// To convert with truncation/loss of data, like `static_cast`, use
/// [`sus::cast<T>()`]($sus::construct::cast). It can convert between
/// integers, floats, and enums, for both safe numerics and primitives. See
/// [Casting numeric types](
/// $sus::construct::Cast#casting-numeric-types) for the rules of
/// conversion through [`cast`]($sus::construct::cast).
namespace num {}

}  // namespace sus

// IWYU pragma: begin_exports
#include "sus/num/cast.h"
#include "sus/num/float.h"
#include "sus/num/float_impl.h"
#include "sus/num/signed_integer.h"
#include "sus/num/signed_integer_impl.h"
#include "sus/num/try_from_int_error_impl.h"
#include "sus/num/unsigned_integer.h"
#include "sus/num/unsigned_integer_impl.h"
// IWYU pragma: end_exports
