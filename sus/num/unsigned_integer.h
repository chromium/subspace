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

#include <bit>
#include <compare>
#include <functional>  // TODO: remove this but we need to hash things > size_t.

#include "fmt/format.h"
#include "sus/assertions/check.h"
#include "sus/iter/iterator_concept.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/macros/pure.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/mem/size_of.h"
#include "sus/num/__private/check_integer_overflow.h"
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

/// A 64-bit unsigned integer.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] u64 final {
#define _self u64
#define _pointer false
#define _pointer_sized
#define _primitive uint64_t
#define _signed i64
#include "sus/num/__private/unsigned_integer_methods.inc"
};

/// A 32-bit unsigned integer.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] u32 final {
#define _self u32
#define _pointer false
#define _pointer_sized
#define _primitive uint32_t
#define _signed i32
#include "sus/num/__private/unsigned_integer_methods.inc"
};

/// An 8-bit unsigned integer.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] u8 final {
#define _self u8
#define _pointer false
#define _pointer_sized
#define _primitive uint8_t
#define _signed i8
#include "sus/num/__private/unsigned_integer_methods.inc"
};

/// A 16-bit unsigned integer.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] u16 final {
#define _self u16
#define _pointer false
#define _pointer_sized
#define _primitive uint16_t
#define _signed i16
#include "sus/num/__private/unsigned_integer_methods.inc"
};

/// An address-sized unsigned integer.
///
/// This type is capable of holding any offset or (positive) distance in a
/// single memory allocation, as allocations are bounded at
/// [`isize::MAX`]($sus::num::isize::MAX). It can represent any absolute
/// address in a linear address system.
///
/// Note that it is possible for a pointer to be larger than an address under
/// some architectures, with a pointer holding additional data such as
/// capabilities. See [CHERI](
/// https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-947.pdf) for an example. So
/// this type is not always the same size as a pointer and should not be used to
/// hold a pointer value without acknowledging that it is only the address part
/// of the pointer.
///
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] usize final {
#define _self usize
#define _pointer false
#define _pointer_sized
#define _primitive \
  ::sus::num::__private::ptr_type<::sus::mem::size_of<size_t>()>::unsigned_type
#define _signed isize
#include "sus/num/__private/unsigned_integer_methods.inc"

  /// Satisfies the [`AddAssign`]($sus::num::AddAssign) concept for pointers
  /// (`T*`) with [`usize`]($sus::num::usize).
  ///
  /// Adds a [`usize`]($sus::num::usize) to a referenced pointer, and returns the
  /// input reference.
  ///
  /// #[doc.overloads=ptr.add.usize]
  template <class T>
  constexpr friend T*& operator+=(T*& t, usize offset) {
    t += size_t{offset};
    return t;
  }

  /// Satisfies the [`Sub`]($sus::num::Sub) concept for pointers
  /// (`T*`) with [`usize`]($sus::num::usize).
  ///
  /// Subtracts a [`usize`]($sus::num::usize) from a pointer, returning the
  /// resulting pointer.
  ///
  /// #[doc.overloads=ptr.sub.usize]
  template <class T>
  __sus_pure_const constexpr friend T* operator-(T* t, usize offset) {
    return t - size_t{offset};
  }

  /// Satisfies the [`SubAssign`]($sus::num::SubAssign) concept for pointers
  /// (`T*`) with [`usize`]($sus::num::usize).
  ///
  /// Subtracts a [`usize`]($sus::num::usize) from a referenced pointer, and
  /// returns the input
  /// reference.
  ///
  /// #[doc.overloads=ptr.sub.usize]
  template <class T>
  constexpr friend T*& operator-=(T*& t, usize offset) {
    t -= size_t{offset};
    return t;
  }
};

/// A pointer-sized unsigned integer.
///
/// This type is capable of holding a pointer, and is convertible to and from
/// pointers. It is typically the same size as [`usize`]($sus::num::usize) but it can be larger when
/// pointers include additional bits that the address.
///
/// # Constructing a [`uptr`]($sus::num::uptr)
///
/// See [`with_addr`]($sus::num::uptr::with_addr) for constructing
/// [`uptr`]($sus::num::uptr) with an address from another
/// [`uptr`]($sus::num::uptr).
///
/// If pointers contain additional metadata beyond an address, the
/// [`with_addr`]($sus::num::uptr::with_addr) method copies the metadata from
/// the original `uptr` to the newly produced
/// [`uptr`]($sus::num::uptr). Otherwise, constructing a
/// [`uptr`]($sus::num::uptr) from an integer can produce a pointer with
/// invalid (empty) metadata and dereferencing such a pointer would be invalid.
///
/// To explicitly construct a [`uptr`]($sus::num::uptr) with empty metadata,
/// use `uptr().with_addr(address)`.
///
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] uptr final {
#define _self uptr
#define _pointer true
#define _pointer_sized ::sus::num::__private::ptr_type<>::pointer_sized_type
#define _primitive                 \
  ::sus::num::__private::ptr_type< \
      ::sus::mem::size_of<uintptr_t>()>::unsigned_type
#include "sus/num/__private/unsigned_integer_methods.inc"
};

}  // namespace sus::num

/// For writing [`u8`]($sus::num::u8) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`) and
/// signed values, unless a literal suffix modifies them, such as with `_u8`
/// which creates an unsigned 8-bit value.
///
/// Values out of range for [`u8`]($sus::num::u8) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_u8 - (5_u8).abs();
/// sus_check(i == 118_u8);
/// ```
_sus__integer_literal(u8, ::sus::num::u8);
/// For writing [`u16`]($sus::num::u16) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`) and
/// signed values, unless a literal suffix modifies them, such as with `_u16`
/// which creates an unsigned 16-bit value.
///
/// Values out of range for [`u16`]($sus::num::u16) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_u16 - (5_u16).abs();
/// sus_check(i == 118_u16);
/// ```
_sus__integer_literal(u16, ::sus::num::u16);
/// For writing [`u32`]($sus::num::u32) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`) and
/// signed values, unless a literal suffix modifies them, such as with `_u32`
/// which creates an unsigned 32-bit value. This is the same as the `u` suffix
/// except that it forces a safe numeric type instead of a primitive value when
/// this is needed (such as for templates or member function access).
///
/// Values out of range for [`u32`]($sus::num::u32) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_u32 - (5_u32).abs();
/// sus_check(i == 118_u32);
/// ```
_sus__integer_literal(u32, ::sus::num::u32);
/// For writing [`u64`]($sus::num::u64) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`) and
/// signed values, unless a literal suffix modifies them, such as with `_u64`
/// which creates an unsigned 64-bit value. On Windows, this is the same as
/// the `ul` suffix (which makes an `unsigned long`) but is platform agnostic,
/// and forces a safe numeric type instead of a primitive value when this is
/// needed (such as for templates or member function access).
///
/// Values out of range for [`u64`]($sus::num::u64) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_u64 - (5_u64).abs();
/// sus_check(i == 118_u64);
/// ```
_sus__integer_literal(u64, ::sus::num::u64);
/// For writing [`usize`]($sus::num::usize) literals.
///
/// Un-qualified integer literals are 32 bits large (the size of `int`) and
/// signed values, unless a literal suffix modifies them, such as with `_usize`
/// which creates an unsigned address-sized value. This is 32 bits for 32-bit
/// targets and 64 bits for 64-bit targets.
///
/// Values out of range for [`usize`]($sus::num::usize) will fail to compile.
///
/// # Examples
/// ```
/// auto i = 123_usize - (5_usize).abs();
/// sus_check(i == 118_usize);
/// ```
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
