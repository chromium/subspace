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
#include "sus/assertions/check.h"
#include "sus/assertions/endian.h"
#include "sus/iter/iterator_concept.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/macros/pure.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/mem/size_of.h"
#include "sus/num/__private/int_log10.h"
#include "sus/num/__private/intrinsics.h"
#include "sus/num/__private/literals.h"
#include "sus/num/__private/primitive_type.h"
#include "sus/num/float_concepts.h"
#include "sus/num/integer_concepts.h"
#include "sus/num/try_from_int_error.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus::num {

// TODO: from_str_radix(). Need Result typ`e and Errors.

/// A 32-bit unsigned integer.
///
/// # Conversions
///
/// To convert to and from integer values, use `sus::into()` when
/// `sus::construct::Into<From, To>` is satisfied between the two types for
/// lossless conversion. Otherwise use `sus::try_into()` when
/// `sus::construct::TryInto<From, To>` is satisfied to convert and handle cases
/// where the value can not be represented in the target type.
///
/// Use use `sus::mog<T>()` to do a lossy type coercion (like
/// `static_cast<T>()`) between integer and floating point types, or C++
/// primitive integers, floating point, or enums. When converting to a larger
/// signed integer type, the value will be sign-extended.
struct [[sus_trivial_abi]] u32 final {
#define _self u32
#define _pointer 0
#define _primitive uint32_t
#define _signed i32
#include "sus/num/__private/unsigned_integer_methods.inc"
};
#define _self u32
#define _pointer 0
#define _primitive uint32_t
#include "sus/num/__private/unsigned_integer_consts.inc"

/// An 8-bit unsigned integer.
///
/// # Conversions
///
/// To convert to and from integer values, use `sus::into()` when
/// `sus::construct::Into<From, To>` is satisfied between the two types for
/// lossless conversion. Otherwise use `sus::try_into()` when
/// `sus::construct::TryInto<From, To>` is satisfied to convert and handle cases
/// where the value can not be represented in the target type.
///
/// Use use `sus::mog<T>()` to do a lossy type coercion (like
/// `static_cast<T>()`) between integer and floating point types, or C++
/// primitive integers, floating point, or enums. When converting to a larger
/// signed integer type, the value will be sign-extended.
struct [[sus_trivial_abi]] u8 final {
#define _self u8
#define _pointer 0
#define _primitive uint8_t
#define _signed i8
#include "sus/num/__private/unsigned_integer_methods.inc"
};
#define _self u8
#define _pointer 0
#define _primitive uint8_t
#include "sus/num/__private/unsigned_integer_consts.inc"

/// A 16-bit unsigned integer.
///
/// # Conversions
///
/// To convert to and from integer values, use `sus::into()` when
/// `sus::construct::Into<From, To>` is satisfied between the two types for
/// lossless conversion. Otherwise use `sus::try_into()` when
/// `sus::construct::TryInto<From, To>` is satisfied to convert and handle cases
/// where the value can not be represented in the target type.
///
/// Use use `sus::mog<T>()` to do a lossy type coercion (like
/// `static_cast<T>()`) between integer and floating point types, or C++
/// primitive integers, floating point, or enums. When converting to a larger
/// signed integer type, the value will be sign-extended.
struct [[sus_trivial_abi]] u16 final {
#define _self u16
#define _pointer 0
#define _primitive uint16_t
#define _signed i16
#include "sus/num/__private/unsigned_integer_methods.inc"
};
#define _self u16
#define _pointer 0
#define _primitive uint16_t
#include "sus/num/__private/unsigned_integer_consts.inc"

/// A 64-bit unsigned integer.
///
/// # Conversions
///
/// To convert to and from integer values, use `sus::into()` when
/// `sus::construct::Into<From, To>` is satisfied between the two types for
/// lossless conversion. Otherwise use `sus::try_into()` when
/// `sus::construct::TryInto<From, To>` is satisfied to convert and handle cases
/// where the value can not be represented in the target type.
///
/// Use use `sus::mog<T>()` to do a lossy type coercion (like
/// `static_cast<T>()`) between integer and floating point types, or C++
/// primitive integers, floating point, or enums. When converting to a larger
/// signed integer type, the value will be sign-extended.
struct [[sus_trivial_abi]] u64 final {
#define _self u64
#define _pointer 0
#define _primitive uint64_t
#define _signed i64
#include "sus/num/__private/unsigned_integer_methods.inc"
};
#define _self u64
#define _pointer 0
#define _primitive uint64_t
#include "sus/num/__private/unsigned_integer_consts.inc"

/// An address-sized unsigned integer.
///
/// This type is capable of holding any offset or (positive) distance in a
/// single memory allocation, as allocations are bounded at `isize::MAX`. It can
/// represent any absolute address in a linear address system.
///
/// Note that it is possible for a pointer to be larger than an address under
/// some architectures, with a pointer holding additional data such as
/// capabilities. See [CHERI](
/// https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-947.pdf) for an example. So
/// this type is not always the same size as a pointer and should not be used to
/// hold a pointer value without acknowledging that it is only the address part
/// of the pointer.
///
/// # Conversions
///
/// To convert to and from integer values, use `sus::into()` when
/// `sus::construct::Into<From, To>` is satisfied between the two types for
/// lossless conversion. Otherwise use `sus::try_into()` when
/// `sus::construct::TryInto<From, To>` is satisfied to convert and handle cases
/// where the value can not be represented in the target type.
///
/// Use use `sus::mog<T>()` to do a lossy type coercion (like
/// `static_cast<T>()`) between integer and floating point types, or C++
/// primitive integers, floating point, or enums. When converting to a larger
/// signed integer type, the value will be sign-extended.
struct [[sus_trivial_abi]] usize final {
#define _self usize
#define _pointer 0
#define _primitive ::sus::num::__private::addr_type<>::unsigned_type
#define _signed isize
#include "sus/num/__private/unsigned_integer_methods.inc"
};
#define _self usize
#define _pointer 0
#define _primitive ::sus::num::__private::addr_type<>::unsigned_type
#include "sus/num/__private/unsigned_integer_consts.inc"

/// A pointer-sized unsigned integer.
///
/// This type is capable of holding a pointer, and is convertible to and from
/// pointers. It is typically the same size as `usize` but it can be larger when
/// pointers include additional bits that the address.
///
/// # Constructing a `uptr`
///
/// See `with_addr` for constructing `uptr` with an address from another `uptr`.
///
/// If pointers contain additional metadata beyond an address, the `with_addr`
/// method copies the metadata from the original `uptr` to the newly produced
/// `uptr.` Otherwise, constructing a uptr() from an integer can produce a
/// pointer with invalid (empty) metadata and dereferencing such a pointer would
/// be invalid.
///
/// To explicitly construct a `uptr` with empty metadata, use
/// `uptr().with_addr(address)`.
///
/// # Conversions
///
/// To convert to and from integer values, use `sus::into()` when
/// `sus::construct::Into<From, To>` is satisfied between the two types for
/// lossless conversion. Otherwise use `sus::try_into()` when
/// `sus::construct::TryInto<From, To>` is satisfied to convert and handle cases
/// where the value can not be represented in the target type.
///
/// Use use `sus::mog<T>()` to do a lossy type coercion (like
/// `static_cast<T>()`) between integer and floating point types, or C++
/// primitive integers, floating point, or enums. When converting to a larger
/// signed integer type, the value will be sign-extended.
struct [[sus_trivial_abi]] uptr final {
#define _self uptr
#define _pointer 1
#define _primitive ::sus::num::__private::ptr_type<>::unsigned_type
#include "sus/num/__private/unsigned_integer_methods.inc"
};
#define _self uptr
#define _pointer 1
#define _primitive ::sus::num::__private::ptr_type<>::unsigned_type
#include "sus/num/__private/unsigned_integer_consts.inc"

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
using sus::num::uptr;
using sus::num::usize;
}  // namespace sus
