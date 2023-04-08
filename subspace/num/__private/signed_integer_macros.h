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

#include "subspace/assertions/check.h"
#include "subspace/assertions/endian.h"
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

namespace sus::containers {
template <class T, size_t N>
  requires(N <= size_t{PTRDIFF_MAX})
class Array;
}

namespace sus::tuple_type {
template <class T, class... Ts>
class Tuple;
}

#define _sus__signed_impl(T, PrimitiveT, UnsignedT) \
  _sus__signed_storage(PrimitiveT);                 \
  _sus__signed_constants_defn(T, PrimitiveT);       \
  _sus__signed_construct(T, PrimitiveT);            \
  _sus__signed_from(T, PrimitiveT);                 \
  _sus__signed_to_primitive(T, PrimitiveT);         \
  _sus__signed_integer_comparison(T, PrimitiveT);   \
  _sus__signed_unary_ops(T);                        \
  _sus__signed_binary_logic_ops(T, PrimitiveT);     \
  _sus__signed_binary_bit_ops(T, PrimitiveT);       \
  _sus__signed_mutable_logic_ops(T);                \
  _sus__signed_mutable_bit_ops(T);                  \
  _sus__signed_abs(T, PrimitiveT, UnsignedT);       \
  _sus__signed_add(T, UnsignedT);                   \
  _sus__signed_div(T);                              \
  _sus__signed_mul(T);                              \
  _sus__signed_neg(T);                              \
  _sus__signed_rem(T, PrimitiveT);                  \
  _sus__signed_euclid(T, PrimitiveT);               \
  _sus__signed_shift(T);                            \
  _sus__signed_sub(T, PrimitiveT, UnsignedT);       \
  _sus__signed_bits(T);                             \
  _sus__signed_pow(T);                              \
  _sus__signed_log(T);                              \
  _sus__signed_endian(T, UnsignedT, ::sus::mem::size_of<PrimitiveT>())

#define _sus__signed_storage(PrimitiveT)                                      \
  /** The inner primitive value, in case it needs to be unwrapped from the    \
   * type. Avoid using this member except to convert when a consumer requires \
   * it.                                                                      \
   */                                                                         \
  PrimitiveT primitive_value { 0 }

#define _sus__signed_constants_defn(T, PrimitiveT)                          \
  /** The smallest value that can be represented by this integer type,      \
   * as a native C++ primitive. */                                          \
  static constexpr auto MIN_PRIMITIVE = __private::min_value<PrimitiveT>(); \
  /** The largest value that can be represented by this integer type,       \
   * as a native C++ primitive. */                                          \
  static constexpr auto MAX_PRIMITIVE = __private::max_value<PrimitiveT>(); \
  /** The smallest value that can be represented by this integer type. */   \
  static const T MIN;                                                       \
  /** The largest value that can be represented by this integer type. */    \
  static const T MAX;                                                       \
  /** The size of this integer type in bits. */                             \
  static const u32 BITS;                                                    \
  static_assert(true)

#define _sus__signed_constants_decl(T, PrimitiveT)                       \
  inline constexpr T T::MIN = T(T::MIN_PRIMITIVE);                       \
  inline constexpr T T::MAX = T(T::MAX_PRIMITIVE);                       \
  inline constexpr u32 T::BITS = u32(__private::num_bits<PrimitiveT>()); \
  static_assert(true)

#define _sus__signed_construct(T, PrimitiveT)                                  \
  /** Default constructor, which sets the integer to 0.                        \
   *                                                                           \
   * The trivial copy and move constructors are implicitly declared, as is the \
   * trivial destructor.                                                       \
   */                                                                          \
  constexpr inline T() noexcept = default;                                     \
                                                                               \
  /** Construction from signed primitive types where no bits are lost.         \
   *                                                                           \
   * #[doc.overloads=signedint.ctor.signedint]                                 \
   */                                                                          \
  template <SignedPrimitiveInteger P>                                          \
    requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<PrimitiveT>())    \
  constexpr inline T(P v) : primitive_value(v) {}                              \
                                                                               \
  /** Construction from signed enum types where no bits are lost.              \
   *                                                                           \
   * #[doc.overloads=signedint.ctor.signedenum]                                \
   */                                                                          \
  template <SignedPrimitiveEnum P>                                             \
    requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<PrimitiveT>())    \
  constexpr inline T(P v) : primitive_value(v) {}                              \
                                                                               \
  /** Construction from signed enum class types where no bits are lost.        \
   *                                                                           \
   * #[doc.overloads=signedint.ctor.signedenumclass]                           \
   */                                                                          \
  template <SignedPrimitiveEnumClass P>                                        \
    requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<PrimitiveT>())    \
  explicit constexpr inline T(P v)                                             \
      : primitive_value(static_cast<PrimitiveT>(v)) {}                         \
                                                                               \
  /** Construction from unsigned primitive types where no bits are lost.       \
   *                                                                           \
   * #[doc.overloads=signedint.ctor.unsignedint]                               \
   */                                                                          \
  template <UnsignedPrimitiveInteger P>                                        \
    requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<PrimitiveT>())     \
  constexpr inline T(P v) : primitive_value(v) {}                              \
                                                                               \
  /** Construction from unsigned enum types where no bits are lost.            \
   *                                                                           \
   * #[doc.overloads=signedint.ctor.unsignedenum]                              \
   */                                                                          \
  template <UnsignedPrimitiveEnum P>                                           \
    requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<PrimitiveT>())     \
  constexpr inline T(P v) : primitive_value(v) {}                              \
                                                                               \
  /** Construction from unsigned enum class types where no bits are lost.      \
   *                                                                           \
   * #[doc.overloads=signedint.ctor.unsignedenumclass]                         \
   */                                                                          \
  template <UnsignedPrimitiveEnumClass P>                                      \
    requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<PrimitiveT>())     \
  explicit constexpr inline T(P v)                                             \
      : primitive_value(static_cast<PrimitiveT>(v)) {}                         \
                                                                               \
  /** Assignment from signed primitive types where no bits are lost.           \
   *                                                                           \
   * #[doc.overloads=signedint.assign.signedint]                               \
   */                                                                          \
  template <SignedPrimitiveInteger P>                                          \
    requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<PrimitiveT>())    \
  constexpr inline T& operator=(P v) noexcept {                                \
    primitive_value = v;                                                       \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  /** Assignment from signed enum types where no bits are lost.                \
   *                                                                           \
   * #[doc.overloads=signedint.assign.signedenum]                              \
   */                                                                          \
  template <SignedPrimitiveEnum P>                                             \
    requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<PrimitiveT>())    \
  constexpr inline T& operator=(P v) noexcept {                                \
    primitive_value = v;                                                       \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  /** Assignment from unsigned primitive types where no bits are lost.         \
   *                                                                           \
   * #[doc.overloads=signedint.assign.unsignedint]                             \
   */                                                                          \
  template <UnsignedPrimitiveInteger P>                                        \
    requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<PrimitiveT>())     \
  constexpr inline T& operator=(P v) noexcept {                                \
    primitive_value = v;                                                       \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  /** Assignment from unsigned enum types where no bits are lost.              \
   *                                                                           \
   * #[doc.overloads=signedint.assign.unsignedenum]                            \
   */                                                                          \
  template <UnsignedPrimitiveEnum P>                                           \
    requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<PrimitiveT>())     \
  constexpr inline T& operator=(P v) noexcept {                                \
    primitive_value = v;                                                       \
    return *this;                                                              \
  }                                                                            \
  static_assert(true)

#define _sus__signed_from(T, PrimitiveT)                                       \
  /** Constructs a ##T## from a signed integer type (i8, i16, i32, etc).       \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   *                                                                           \
   * #[doc.overloads=0]                                                        \
   */                                                                          \
  template <Signed S>                                                          \
  static constexpr T from(S s) noexcept {                                      \
    if constexpr (MIN_PRIMITIVE > S::MIN_PRIMITIVE)                            \
      ::sus::check(s.primitive_value >= MIN_PRIMITIVE);                        \
    if constexpr (MAX_PRIMITIVE < S::MAX_PRIMITIVE)                            \
      ::sus::check(s.primitive_value <= MAX_PRIMITIVE);                        \
    return T(static_cast<PrimitiveT>(s.primitive_value));                      \
  }                                                                            \
                                                                               \
  /** Try to construct a ##T## from a signed integer type (i8, i16, i32, etc). \
   *                                                                           \
   * Returns an error if the source value is outside of the range of ##T##.    \
   *                                                                           \
   * #[doc.overloads=0]                                                        \
   */                                                                          \
  template <Signed S>                                                          \
  static constexpr ::sus::result::Result<T, ::sus::num::TryFromIntError>       \
  try_from(S s) noexcept {                                                     \
    using R = ::sus::result::Result<T, ::sus::num::TryFromIntError>;           \
    if constexpr (MIN_PRIMITIVE > S::MIN_PRIMITIVE) {                          \
      if (s.primitive_value < MIN_PRIMITIVE) {                                 \
        return R::with_err(::sus::num::TryFromIntError(                        \
            ::sus::num::TryFromIntError::Kind::OutOfBounds));                  \
      }                                                                        \
    }                                                                          \
    if constexpr (MAX_PRIMITIVE < S::MAX_PRIMITIVE) {                          \
      if (s.primitive_value > MAX_PRIMITIVE) {                                 \
        return R::with_err(::sus::num::TryFromIntError(                        \
            ::sus::num::TryFromIntError::Kind::OutOfBounds));                  \
      }                                                                        \
    }                                                                          \
    return R::with(T(static_cast<PrimitiveT>(s.primitive_value)));             \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from an unsigned integer type (u8, u16, u32, etc).    \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   *                                                                           \
   * #[doc.overloads=1]                                                        \
   */                                                                          \
  template <Unsigned U>                                                        \
  static constexpr T from(U u) noexcept {                                      \
    constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);             \
    if constexpr (umax < U::MAX_PRIMITIVE) {                                   \
      ::sus::check(u.primitive_value <= umax);                                 \
    }                                                                          \
    return T(static_cast<PrimitiveT>(u.primitive_value));                      \
  }                                                                            \
                                                                               \
  /** Try to construct a ##T## from an unsigned integer type (u8, u16, u32,    \
   * etc).                                                                     \
   *                                                                           \
   * Returns an error if the source value is outside of the range of ##T##.    \
   *                                                                           \
   * #[doc.overloads=1]                                                        \
   */                                                                          \
  template <Unsigned U>                                                        \
  static constexpr ::sus::result::Result<T, ::sus::num::TryFromIntError>       \
  try_from(U u) noexcept {                                                     \
    using R = ::sus::result::Result<T, ::sus::num::TryFromIntError>;           \
    constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);             \
    if constexpr (umax < U::MAX_PRIMITIVE) {                                   \
      if (u.primitive_value > umax) {                                          \
        return R::with_err(::sus::num::TryFromIntError(                        \
            ::sus::num::TryFromIntError::Kind::OutOfBounds));                  \
      }                                                                        \
    }                                                                          \
    return R::with(T(static_cast<PrimitiveT>(u.primitive_value)));             \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from a signed primitive integer type (int, long,      \
   * etc).                                                                     \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   *                                                                           \
   * #[doc.overloads=signedint.from.signedint]                                 \
   */                                                                          \
  template <SignedPrimitiveInteger S>                                          \
  static constexpr T from(S s) {                                               \
    if constexpr (MIN_PRIMITIVE > __private::min_value<S>())                   \
      ::sus::check(s >= MIN_PRIMITIVE);                                        \
    if constexpr (MAX_PRIMITIVE < __private::max_value<S>())                   \
      ::sus::check(s <= MAX_PRIMITIVE);                                        \
    return T(static_cast<PrimitiveT>(s));                                      \
  }                                                                            \
                                                                               \
  /** Tries to construct a ##T## from a signed primitive integer type (int,    \
   * long, etc).                                                               \
   *                                                                           \
   * Returns an error if the source value is outside of the range of ##T##.    \
   *                                                                           \
   * #[doc.overloads=signedint.tryfrom.signedint]                              \
   */                                                                          \
  template <SignedPrimitiveInteger S>                                          \
  static constexpr ::sus::result::Result<T, ::sus::num::TryFromIntError>       \
  try_from(S s) {                                                              \
    using R = ::sus::result::Result<T, ::sus::num::TryFromIntError>;           \
    if constexpr (MIN_PRIMITIVE > __private::min_value<S>()) {                 \
      if (s < MIN_PRIMITIVE) {                                                 \
        return R::with_err(::sus::num::TryFromIntError(                        \
            ::sus::num::TryFromIntError::Kind::OutOfBounds));                  \
      }                                                                        \
    }                                                                          \
    if constexpr (MAX_PRIMITIVE < __private::max_value<S>()) {                 \
      if (s > MAX_PRIMITIVE) {                                                 \
        return R::with_err(::sus::num::TryFromIntError(                        \
            ::sus::num::TryFromIntError::Kind::OutOfBounds));                  \
      }                                                                        \
    }                                                                          \
    return R::with(T(static_cast<PrimitiveT>(s)));                             \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from a signed enum type (or enum class).              \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   *                                                                           \
   * #[doc.overloads=signedint.from.signedenum]                                \
   */                                                                          \
  template <class S>                                                           \
    requires(SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>)            \
  static constexpr T from(S s) {                                               \
    using D = std::underlying_type_t<S>;                                       \
    if constexpr (MIN_PRIMITIVE > __private::min_value<D>())                   \
      ::sus::check(static_cast<D>(s) >= MIN_PRIMITIVE);                        \
    if constexpr (MAX_PRIMITIVE < __private::max_value<D>())                   \
      ::sus::check(static_cast<D>(s) <= MAX_PRIMITIVE);                        \
    return T(static_cast<PrimitiveT>(s));                                      \
  }                                                                            \
                                                                               \
  /** Tries to construct a ##T## from a signed enum type (or enum class).      \
   *                                                                           \
   * Returns an error if the source value is outside of the range of ##T##.    \
   *                                                                           \
   * #[doc.overloads=signedint.tryfrom.signedenum]                             \
   */                                                                          \
  template <class S>                                                           \
    requires(SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>)            \
  static constexpr ::sus::result::Result<T, ::sus::num::TryFromIntError>       \
  try_from(S s) {                                                              \
    using D = std::underlying_type_t<S>;                                       \
    using R = ::sus::result::Result<T, ::sus::num::TryFromIntError>;           \
    if constexpr (MIN_PRIMITIVE > __private::min_value<D>()) {                 \
      if (static_cast<D>(s) < MIN_PRIMITIVE) {                                 \
        return R::with_err(::sus::num::TryFromIntError(                        \
            ::sus::num::TryFromIntError::Kind::OutOfBounds));                  \
      }                                                                        \
    }                                                                          \
    if constexpr (MAX_PRIMITIVE < __private::max_value<D>()) {                 \
      if (static_cast<D>(s) > MAX_PRIMITIVE) {                                 \
        return R::with_err(::sus::num::TryFromIntError(                        \
            ::sus::num::TryFromIntError::Kind::OutOfBounds));                  \
      }                                                                        \
    }                                                                          \
    return R::with(T(static_cast<PrimitiveT>(s)));                             \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from an unsigned primitive integer type (unsigned     \
   * int, unsigned long, etc).                                                 \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   *                                                                           \
   * #[doc.overloads=signedint.from.unsignedint]                               \
   */                                                                          \
  template <UnsignedPrimitiveInteger U>                                        \
  static constexpr T from(U u) {                                               \
    constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);             \
    if constexpr (umax < __private::max_value<U>()) {                          \
      ::sus::check(u <= umax);                                                 \
    }                                                                          \
    return T(static_cast<PrimitiveT>(u));                                      \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from an unsigned primitive integer type (unsigned     \
   * int, unsigned long, etc).                                                 \
   *                                                                           \
   * Returns an error if the source value is outside of the range of ##T##.    \
   *                                                                           \
   * #[doc.overloads=signedint.tryfrom.unsignedint]                            \
   */                                                                          \
  template <UnsignedPrimitiveInteger U>                                        \
  static constexpr ::sus::result::Result<T, ::sus::num::TryFromIntError>       \
  try_from(U u) {                                                              \
    using R = ::sus::result::Result<T, ::sus::num::TryFromIntError>;           \
    constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);             \
    if constexpr (umax < __private::max_value<U>()) {                          \
      if (u > umax) {                                                          \
        return R::with_err(::sus::num::TryFromIntError(                        \
            ::sus::num::TryFromIntError::Kind::OutOfBounds));                  \
      }                                                                        \
    }                                                                          \
    return R::with(T(static_cast<PrimitiveT>(u)));                             \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from an unsigned enum type (or enum class).           \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   *                                                                           \
   * #[doc.overloads=signedint.from.unsignedint]                               \
   */                                                                          \
  template <class U>                                                           \
    requires(UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>)        \
  static constexpr T from(U u) {                                               \
    using D = std::underlying_type_t<U>;                                       \
    constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);             \
    if constexpr (umax < __private::max_value<D>()) {                          \
      ::sus::check(static_cast<D>(u) <= umax);                                 \
    }                                                                          \
    return T(static_cast<PrimitiveT>(u));                                      \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from an unsigned enum type (or enum class).           \
   *                                                                           \
   * Returns an error if the source value is outside of the range of ##T##.    \
   *                                                                           \
   * #[doc.overloads=signedint.tryfrom.unsignedint]                            \
   */                                                                          \
  template <class U>                                                           \
    requires(UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>)        \
  static constexpr ::sus::result::Result<T, ::sus::num::TryFromIntError>       \
  try_from(U u) {                                                              \
    using D = std::underlying_type_t<U>;                                       \
    using R = ::sus::result::Result<T, ::sus::num::TryFromIntError>;           \
    constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);             \
    if constexpr (umax < __private::max_value<D>()) {                          \
      if (static_cast<D>(u) > umax) {                                          \
        return R::with_err(::sus::num::TryFromIntError(                        \
            ::sus::num::TryFromIntError::Kind::OutOfBounds));                  \
      }                                                                        \
    }                                                                          \
    return R::with(T(static_cast<PrimitiveT>(u)));                             \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from a signed or unsigned integer type (i8, i16, u32, \
   * u64 etc).                                                                 \
   *                                                                           \
   * # Safety                                                                  \
   * If the input value is out of range for ##T##, the value will be           \
   * truncated, which may lead to application bugs and memory unsafety.        \
   *                                                                           \
   * #[doc.overloads=signedint.fromunchecked.int]                              \
   */                                                                          \
  template <Integer I>                                                         \
  static constexpr T from_unchecked(::sus::marker::UnsafeFnMarker,             \
                                    I i) noexcept {                            \
    return T(static_cast<PrimitiveT>(i.primitive_value));                      \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from a signed or unsigned integer primitive type      \
   * (int, long, unsigned int, etc).                                           \
   *                                                                           \
   * # Safety                                                                  \
   * If the input value is out of range for ##T##, the value will be           \
   * truncated, which may lead to application bugs and memory unsafety.        \
   *                                                                           \
   * #[doc.overloads=signedint.fromunchecked.primitive]                        \
   */                                                                          \
  template <PrimitiveInteger P>                                                \
  static constexpr T from_unchecked(::sus::marker::UnsafeFnMarker,             \
                                    P p) noexcept {                            \
    return T(static_cast<PrimitiveT>(p));                                      \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from a signed or unsigned enum type (or enum class).  \
   *                                                                           \
   * # Safety                                                                  \
   * If the input value is out of range for ##T##, the value will be           \
   * truncated, which may lead to application bugs and memory unsafety.        \
   *                                                                           \
   * #[doc.overloads=signedint.fromunchecked.enum]                             \
   */                                                                          \
  template <class P>                                                           \
    requires(PrimitiveEnum<P> || PrimitiveEnumClass<P>)                        \
  static constexpr T from_unchecked(::sus::marker::UnsafeFnMarker,             \
                                    P p) noexcept {                            \
    return T(static_cast<PrimitiveT>(p));                                      \
  }                                                                            \
  static_assert(true)

#define _sus__signed_to_primitive(T, PrimitiveT)                            \
  template <SignedPrimitiveInteger U>                                       \
    requires(::sus::mem::size_of<U>() >= ::sus::mem::size_of<PrimitiveT>()) \
  constexpr inline explicit operator U() const {                            \
    return primitive_value;                                                 \
  }                                                                         \
  static_assert(true)

#define _sus__signed_integer_comparison(T, PrimitiveT)                         \
  /** Returns true if the current value is positive and false if the number is \
   * zero or negative.                                                         \
   */                                                                          \
  constexpr bool is_negative() const& noexcept { return primitive_value < 0; } \
  /** Returns true if the current value is negative and false if the number is \
   * zero or positive.                                                         \
   */                                                                          \
  constexpr bool is_positive() const& noexcept { return primitive_value > 0; } \
                                                                               \
  /** Returns a number representing sign of the current value.                 \
   *                                                                           \
   * - 0 if the number is zero                                                 \
   * - 1 if the number is positive                                             \
   * - -1 if the number is negative                                            \
   */                                                                          \
  constexpr T signum() const& noexcept {                                       \
    if (primitive_value < 0)                                                   \
      return PrimitiveT{-1};                                                   \
    else                                                                       \
      return PrimitiveT{primitive_value != 0};                                 \
  }                                                                            \
                                                                               \
  /** sus::ops::Eq<##T##> trait.                                               \
   * #[doc.overloads=int.eq.self] */                                           \
  friend constexpr inline bool operator==(const T& l, const T& r) noexcept =   \
      default;                                                                 \
  /** sus::ops::Eq<##T##, UnsignedPrimitiveInteger> trait.                     \
   * #[doc.overloads=int.eq.signedprimitive] */                                \
  template <SignedPrimitiveInteger P>                                          \
  friend constexpr inline bool operator==(const T& l, const P& r) noexcept {   \
    return l.primitive_value == r;                                             \
  }                                                                            \
  /** sus::ops::Eq<##T##, Unsigned> trait.                                     \
   * #[doc.overloads=int.eq.signed] */                                         \
  template <Signed S>                                                          \
  friend constexpr inline bool operator==(const T& l, const S& r) noexcept {   \
    return l.primitive_value == r.primitive_value;                             \
  }                                                                            \
  /** sus::ops::Ord<##T##> trait.                                              \
   * #[doc.overloads=int.ord.self] */                                          \
  template <UnsignedPrimitiveInteger P>                                        \
  friend constexpr inline std::strong_ordering operator<=>(                    \
      const T& l, const T& r) noexcept {                                       \
    return l.primitive_value <=> r.primitive_value;                            \
  }                                                                            \
  /** sus::ops::Ord<##T##, SignedPrimitiveInteger> trait.                      \
   * #[doc.overloads=int.ord.signedprimitive] */                               \
  template <SignedPrimitiveInteger P>                                          \
  friend constexpr inline std::strong_ordering operator<=>(                    \
      const T& l, const P& r) noexcept {                                       \
    return l.primitive_value <=> r;                                            \
  }                                                                            \
  /** sus::ops::Ord<##T##, Signed> trait.                                      \
   * #[doc.overloads=int.ord.signed] */                                        \
  template <Signed S>                                                          \
  friend constexpr inline std::strong_ordering operator<=>(                    \
      const T& l, const S& r) noexcept {                                       \
    return l.primitive_value <=> r.primitive_value;                            \
  }                                                                            \
  static_assert(true)

#define _sus__signed_unary_ops(T)                                             \
  /** sus::num::Neg trait. */                                                 \
  constexpr inline T operator-() const& noexcept {                            \
    /* TODO: Allow opting out of all overflow checks? */                      \
    ::sus::check(primitive_value != MIN_PRIMITIVE);                           \
    return __private::unchecked_neg(primitive_value);                         \
  }                                                                           \
  /** sus::num::BitNot trait. */                                              \
  constexpr inline T operator~() const& noexcept {                            \
    return __private::into_signed(                                            \
        __private::unchecked_not(__private::into_unsigned(primitive_value))); \
  }                                                                           \
  static_assert(true)

#define _sus__signed_binary_logic_ops(T, PrimitiveT)                        \
  /** sus::num::Add<##T##> trait.                                           \
   * #[doc.overloads=int##T##.+] */                                         \
  friend constexpr inline T operator+(const T& l, const T& r) noexcept {    \
    const auto out =                                                        \
        __private::add_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::num::Sub<##T##> trait.                                           \
   * #[doc.overloads=int##T##.-] */                                         \
  friend constexpr inline T operator-(const T& l, const T& r) noexcept {    \
    const auto out =                                                        \
        __private::sub_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::num::Mul<##T##> trait.                                           \
   * #[doc.overloads=int##T##.*] */                                         \
  friend constexpr inline T operator*(const T& l, const T& r) noexcept {    \
    const auto out =                                                        \
        __private::mul_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::num::Div<##T##> trait.                                           \
   * #[doc.overloads=int##T##./] */                                         \
  friend constexpr inline T operator/(const T& l, const T& r) noexcept {    \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r.primitive_value != 0);                                   \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(l.primitive_value != MIN_PRIMITIVE ||                      \
                 r.primitive_value != -1);                                  \
    return static_cast<PrimitiveT>(l.primitive_value / r.primitive_value);  \
  }                                                                         \
  /** sus::num::Rem<##T##> trait.                                           \
   * #[doc.overloads=int##T##.%] */                                         \
  friend constexpr inline T operator%(const T& l, const T& r) noexcept {    \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r.primitive_value != 0);                                   \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(l.primitive_value != MIN_PRIMITIVE ||                      \
                 r.primitive_value != -1);                                  \
    return static_cast<PrimitiveT>(l.primitive_value % r.primitive_value);  \
  }                                                                         \
  static_assert(true)

#define _sus__signed_binary_bit_ops(T, PrimitiveT)                          \
  /** sus::num::BitAnd<##T##> trait.                                        \
   * #[doc.overloads=int##T##.&] */                                         \
  friend constexpr inline T operator&(const T& l, const T& r) noexcept {    \
    return static_cast<PrimitiveT>(l.primitive_value & r.primitive_value);  \
  }                                                                         \
  /** sus::num::BitOr<##T##> trait.                                         \
   * #[doc.overloads=int##T##.|] */                                         \
  friend constexpr inline T operator|(const T& l, const T& r) noexcept {    \
    return static_cast<PrimitiveT>(l.primitive_value | r.primitive_value);  \
  }                                                                         \
  /** sus::num::BitXor<##T##> trait.                                        \
   * #[doc.overloads=int##T##.^] */                                         \
  friend constexpr inline T operator^(const T& l, const T& r) noexcept {    \
    return static_cast<PrimitiveT>(l.primitive_value ^ r.primitive_value);  \
  }                                                                         \
  /** sus::num::Shl trait.                                                  \
   * #[doc.overloads=int##T##.<<] */                                        \
  friend constexpr inline T operator<<(const T& l, const u32& r) noexcept { \
    const auto out =                                                        \
        __private::shl_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::num::Shr trait.                                                  \
   * #[doc.overloads=int##T##.>>] */                                        \
  friend constexpr inline T operator>>(const T& l, const u32& r) noexcept { \
    const auto out =                                                        \
        __private::shr_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  static_assert(true)

#define _sus__signed_mutable_logic_ops(T)                                      \
  /** sus::num::AddAssign<##T##> trait. */                                     \
  constexpr inline void operator+=(T r)& noexcept {                            \
    const auto out =                                                           \
        __private::add_with_overflow(primitive_value, r.primitive_value);      \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(!out.overflow);                                               \
    primitive_value = out.value;                                               \
  }                                                                            \
  /** sus::num::SubAssign<##T##> trait. */                                     \
  constexpr inline void operator-=(T r)& noexcept {                            \
    const auto out =                                                           \
        __private::sub_with_overflow(primitive_value, r.primitive_value);      \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(!out.overflow);                                               \
    primitive_value = out.value;                                               \
  }                                                                            \
  /** sus::num::MulAssign<##T##> trait. */                                     \
  constexpr inline void operator*=(T r)& noexcept {                            \
    const auto out =                                                           \
        __private::mul_with_overflow(primitive_value, r.primitive_value);      \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(!out.overflow);                                               \
    primitive_value = out.value;                                               \
  }                                                                            \
  /** sus::num::DivAssign<##T##> trait. */                                     \
  constexpr inline void operator/=(T r)& noexcept {                            \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(r.primitive_value != 0);                                      \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(primitive_value != MIN_PRIMITIVE || r.primitive_value != -1); \
    primitive_value /= r.primitive_value;                                      \
  }                                                                            \
  /** sus::num::RemAssign<##T##> trait. */                                     \
  constexpr inline void operator%=(T r)& noexcept {                            \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(r.primitive_value != 0);                                      \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(primitive_value != MIN_PRIMITIVE || r.primitive_value != -1); \
    primitive_value %= r.primitive_value;                                      \
  }                                                                            \
  static_assert(true)

#define _sus__signed_mutable_bit_ops(T)                                   \
  /** sus::num::BitAndAssign<##T##> trait. */                             \
  constexpr inline void operator&=(T r)& noexcept {                       \
    primitive_value &= r.primitive_value;                                 \
  }                                                                       \
  /** sus::num::BitOrAssign<##T##> trait. */                              \
  constexpr inline void operator|=(T r)& noexcept {                       \
    primitive_value |= r.primitive_value;                                 \
  }                                                                       \
  /** sus::num::BitXorAssign<##T##> trait. */                             \
  constexpr inline void operator^=(T r)& noexcept {                       \
    primitive_value ^= r.primitive_value;                                 \
  }                                                                       \
  /** sus::num::ShlAssign trait. */                                       \
  constexpr inline void operator<<=(const u32& r)& noexcept {             \
    const auto out =                                                      \
        __private::shl_with_overflow(primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(!out.overflow);                                          \
    primitive_value = out.value;                                          \
  }                                                                       \
  /** sus::num::ShrAssign trait. */                                       \
  constexpr inline void operator>>=(const u32& r)& noexcept {             \
    const auto out =                                                      \
        __private::shr_with_overflow(primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(!out.overflow);                                          \
    primitive_value = out.value;                                          \
  }                                                                       \
  static_assert(true)

#define _sus__signed_abs(T, PrimitiveT, UnsignedT)                            \
  /** Computes the absolute value of itself.                                  \
   *                                                                          \
   * The absolute value of ##T##::MIN cannot be represented as an ##T##, and  \
   * attempting to calculate it will panic.                                   \
   */                                                                         \
  constexpr inline T abs() const& noexcept {                                  \
    /* TODO: Allow opting out of all overflow checks? */                      \
    ::sus::check(primitive_value != MIN_PRIMITIVE);                           \
    if (primitive_value >= 0)                                                 \
      return primitive_value;                                                 \
    else                                                                      \
      return __private::unchecked_neg(primitive_value);                       \
  }                                                                           \
                                                                              \
  /** Checked absolute value. Computes `abs()`, returning None if the current \
   * value is MIN.                                                            \
   */                                                                         \
  constexpr Option<T> checked_abs() const& noexcept {                         \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return Option<T>::some(abs());                                          \
    else                                                                      \
      return Option<T>::none();                                               \
  }                                                                           \
                                                                              \
  /** Computes the absolute value of self.                                    \
   *                                                                          \
   * Returns a tuple of the absolute version of self along with a boolean     \
   * indicating whether an overflow happened. If self is the minimum value    \
   * (e.g., ##T##::MIN for values of type ##T##), then the minimum value will \
   * be returned again and true will be returned for an overflow happening.   \
   */                                                                         \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>         \
  constexpr Tuple overflowing_abs() const& noexcept {                         \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return Tuple::with(abs(), false);                                       \
    else                                                                      \
      return Tuple::with(MIN, true);                                          \
  }                                                                           \
                                                                              \
  /** Saturating absolute value. Computes `abs()`, returning MAX if the       \
   *  current value is MIN instead of overflowing.                            \
   */                                                                         \
  constexpr T saturating_abs() const& noexcept {                              \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return abs();                                                           \
    else                                                                      \
      return MAX;                                                             \
  }                                                                           \
                                                                              \
  /** Computes the absolute value of self without any wrapping or panicking.  \
   */                                                                         \
  constexpr UnsignedT unsigned_abs() const& noexcept {                        \
    if (primitive_value >= 0) {                                               \
      return __private::into_unsigned(primitive_value);                       \
    } else {                                                                  \
      const auto neg_plus_one =                                               \
          __private::unchecked_add(primitive_value, PrimitiveT{1});           \
      const auto pos_minus_one =                                              \
          __private::into_unsigned(__private::unchecked_neg(neg_plus_one));   \
      return __private::unchecked_add(pos_minus_one,                          \
                                      decltype(pos_minus_one){1});            \
    }                                                                         \
  }                                                                           \
                                                                              \
  /** Wrapping (modular) absolute value. Computes `this->abs()`, wrapping     \
   * around at the boundary of the type.                                      \
   *                                                                          \
   * The only case where such wrapping can occur is when one takes the        \
   * absolute value of the negative minimal value for the type; this is a     \
   * positive value that is too large to represent in the type. In such a     \
   * case, this function returns MIN itself.                                  \
   */                                                                         \
  constexpr T wrapping_abs() const& noexcept {                                \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return abs();                                                           \
    else                                                                      \
      return MIN;                                                             \
  }                                                                           \
                                                                              \
  /** Computes the absolute difference between self and other.                \
   *                                                                          \
   * This function always returns the correct answer without overflow or      \
   * panics by returning an unsigned integer.                                 \
   */                                                                         \
  constexpr UnsignedT abs_diff(const T& r) const& noexcept {                  \
    if (primitive_value >= r.primitive_value) {                               \
      return __private::sub_with_unsigned_positive_result(primitive_value,    \
                                                          r.primitive_value); \
    } else {                                                                  \
      return __private::sub_with_unsigned_positive_result(r.primitive_value,  \
                                                          primitive_value);   \
    }                                                                         \
  }                                                                           \
  static_assert(true)

#define _sus__signed_add(T, UnsignedT)                                         \
  /** Checked integer addition. Computes self + rhs, returning None if         \
   * overflow occurred.                                                        \
   */                                                                          \
  constexpr Option<T> checked_add(const T& rhs) const& noexcept {              \
    const auto out =                                                           \
        __private::add_with_overflow(primitive_value, rhs.primitive_value);    \
    if (!out.overflow) [[likely]]                                              \
      return Option<T>::some(out.value);                                       \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Checked integer addition with an unsigned rhs. Computes self + rhs,      \
   * returning None if overflow occurred.                                      \
   */                                                                          \
  constexpr Option<T> checked_add_unsigned(const UnsignedT& rhs)               \
      const& noexcept {                                                        \
    const auto out = __private::add_with_overflow_unsigned(                    \
        primitive_value, rhs.primitive_value);                                 \
    if (!out.overflow) [[likely]]                                              \
      return Option<T>::some(out.value);                                       \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Calculates self + rhs                                                    \
   *                                                                           \
   * Returns a tuple of the addition along with a boolean indicating whether   \
   * an arithmetic overflow would occur. If an overflow would have occurred    \
   * then the wrapped value is returned.                                       \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_add(const T& rhs) const& noexcept {              \
    const auto r =                                                             \
        __private::add_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple::with(r.value, r.overflow);                                   \
  }                                                                            \
                                                                               \
  /** Calculates self + rhs with an unsigned rhs                               \
   *                                                                           \
   * Returns a tuple of the addition along with a boolean indicating whether   \
   * an arithmetic overflow would occur. If an overflow would have occurred    \
   * then the wrapped value is returned.                                       \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_add_unsigned(const UnsignedT& rhs)               \
      const& noexcept {                                                        \
    const auto r = __private::add_with_overflow_unsigned(primitive_value,      \
                                                         rhs.primitive_value); \
    return Tuple::with(r.value, r.overflow);                                   \
  }                                                                            \
                                                                               \
  /** Saturating integer addition. Computes self + rhs, saturating at the      \
   * numeric bounds instead of overflowing.                                    \
   */                                                                          \
  constexpr T saturating_add(const T& rhs) const& noexcept {                   \
    return __private::saturating_add(primitive_value, rhs.primitive_value);    \
  }                                                                            \
                                                                               \
  /** Saturating integer addition with an unsigned rhs. Computes self + rhs,   \
   * saturating at the numeric bounds instead of overflowing.                  \
   */                                                                          \
  constexpr T saturating_add_unsigned(const UnsignedT& rhs) const& noexcept {  \
    const auto r = __private::add_with_overflow_unsigned(primitive_value,      \
                                                         rhs.primitive_value); \
    if (!r.overflow) [[likely]]                                                \
      return r.value;                                                          \
    else                                                                       \
      return MAX;                                                              \
  }                                                                            \
                                                                               \
  /** Unchecked integer addition. Computes self + rhs, assuming overflow       \
   * cannot occur.                                                             \
   *                                                                           \
   * # Safety                                                                  \
   * This results in undefined behavior when self + rhs > ##T##::MAX or self   \
   * + rhs < ##T##::MIN, i.e. when checked_add() would return None.            \
   */                                                                          \
  inline constexpr T unchecked_add(::sus::marker::UnsafeFnMarker,              \
                                   const T& rhs) const& noexcept {             \
    return __private::unchecked_add(primitive_value, rhs.primitive_value);     \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) addition. Computes self + rhs, wrapping around at the \
   * boundary of the type.                                                     \
   */                                                                          \
  constexpr T wrapping_add(const T& rhs) const& noexcept {                     \
    return __private::wrapping_add(primitive_value, rhs.primitive_value);      \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) addition with an unsigned rhs. Computes self + rhs,   \
   * wrapping around at the boundary of the type.                              \
   */                                                                          \
  constexpr T wrapping_add_unsigned(const UnsignedT& rhs) const& noexcept {    \
    return __private::add_with_overflow_unsigned(primitive_value,              \
                                                 rhs.primitive_value)          \
        .value;                                                                \
  }                                                                            \
  static_assert(true)

#define _sus__signed_div(T)                                                    \
  /** Checked integer division. Computes self / rhs, returning None if rhs ==  \
   * 0 or the division results in overflow.                                    \
   */                                                                          \
  constexpr Option<T> checked_div(const T& rhs) const& noexcept {              \
    if (__private::div_overflows(primitive_value, rhs.primitive_value))        \
        [[unlikely]]                                                           \
      return Option<T>::none();                                                \
    else                                                                       \
      return Option<T>::some(                                                  \
          __private::unchecked_div(primitive_value, rhs.primitive_value));     \
  }                                                                            \
                                                                               \
  /** Calculates the divisor when self is divided by rhs.                      \
   *                                                                           \
   * Returns a tuple of the divisor along with a boolean indicating whether an \
   * arithmetic overflow would occur. If an overflow would occur then self is  \
   * returned.                                                                 \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_div(const T& rhs) const& noexcept {              \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0);                                    \
    if (__private::div_overflows_nonzero(                                      \
            ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value))   \
        [[unlikely]] {                                                         \
      return Tuple::with(MIN, true);                                           \
    } else {                                                                   \
      return Tuple::with(                                                      \
          __private::unchecked_div(primitive_value, rhs.primitive_value),      \
          false);                                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Saturating integer division. Computes self / rhs, saturating at the      \
   * numeric bounds instead of overflowing.                                    \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  constexpr T saturating_div(const T& rhs) const& noexcept {                   \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0);                                    \
    if (__private::div_overflows_nonzero(                                      \
            ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value))   \
        [[unlikely]] {                                                         \
      /* Only overflows in the case of -MIN / -1, which gives MAX + 1,         \
       saturated to MAX. */                                                    \
      return MAX;                                                              \
    } else {                                                                   \
      return __private::unchecked_div(primitive_value, rhs.primitive_value);   \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) division. Computes self / rhs, wrapping around at the \
   * boundary of the type.                                                     \
   *                                                                           \
   * The only case where such wrapping can occur is when one divides MIN / -1  \
   * on a signed type (where MIN is the negative minimal value for the type);  \
   * this is equivalent to -MIN, a positive value that is too large to         \
   * represent in the type. In such a case, this function returns MIN itself.  \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  constexpr T wrapping_div(const T& rhs) const& noexcept {                     \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0);                                    \
    if (__private::div_overflows_nonzero(                                      \
            ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value))   \
        [[unlikely]] {                                                         \
      /* Only overflows in the case of -MIN / -1, which gives MAX + 1,         \
       that wraps around to MIN. */                                            \
      return MIN;                                                              \
    } else {                                                                   \
      return __private::unchecked_div(primitive_value, rhs.primitive_value);   \
    }                                                                          \
  }                                                                            \
  static_assert(true)

#define _sus__signed_mul(T)                                                    \
  /** Checked integer multiplication. Computes self * rhs, returning None if   \
   * overflow occurred.                                                        \
   */                                                                          \
  constexpr Option<T> checked_mul(const T& rhs) const& noexcept {              \
    const auto out =                                                           \
        __private::mul_with_overflow(primitive_value, rhs.primitive_value);    \
    if (!out.overflow) [[likely]]                                              \
      return Option<T>::some(out.value);                                       \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Calculates the multiplication of self and rhs.                           \
   *                                                                           \
   * Returns a tuple of the multiplication along with a boolean indicating     \
   * whether an arithmetic overflow would occur. If an overflow would have     \
   * occurred then the wrapped value is returned.                              \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_mul(const T& rhs) const& noexcept {              \
    const auto out =                                                           \
        __private::mul_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple::with(out.value, out.overflow);                               \
  }                                                                            \
                                                                               \
  /** Saturating integer multiplication. Computes self * rhs, saturating at    \
   * the numeric bounds instead of overflowing.                                \
   */                                                                          \
  constexpr T saturating_mul(const T& rhs) const& noexcept {                   \
    return __private::saturating_mul(primitive_value, rhs.primitive_value);    \
  }                                                                            \
                                                                               \
  /** Unchecked integer multiplication. Computes self * rhs, assuming overflow \
   * cannot occur.                                                             \
   *                                                                           \
   * # Safety                                                                  \
   * This results in undefined behavior when `self * rhs > ##T##::MAX` or      \
   * `self                                                                     \
   * * rhs < ##T##::MIN`, i.e. when `checked_mul()` would return None.         \
   */                                                                          \
  constexpr inline T unchecked_mul(::sus::marker::UnsafeFnMarker,              \
                                   const T& rhs) const& noexcept {             \
    return __private::unchecked_mul(primitive_value, rhs.primitive_value);     \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) multiplication. Computes self * rhs, wrapping around  \
   * at the boundary of the type.                                              \
   */                                                                          \
  constexpr T wrapping_mul(const T& rhs) const& noexcept {                     \
    return __private::wrapping_mul(primitive_value, rhs.primitive_value);      \
  }                                                                            \
  static_assert(true)

#define _sus__signed_neg(T)                                                   \
  /** Checked negation. Computes -self, returning None if self == MIN.        \
   */                                                                         \
  constexpr Option<T> checked_neg() const& noexcept {                         \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return Option<T>::some(__private::unchecked_neg(primitive_value));      \
    else                                                                      \
      return Option<T>::none();                                               \
  }                                                                           \
                                                                              \
  /** Negates self, overflowing if this is equal to the minimum value.        \
   *                                                                          \
   * Returns a tuple of the negated version of self along with a boolean      \
   * indicating whether an overflow happened. If self is the minimum value    \
   * (e.g., ##T##::MIN for values of type ##T##), then the minimum value will \
   * be returned again and true will be returned for an overflow happening.   \
   */                                                                         \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>         \
  constexpr Tuple overflowing_neg() const& noexcept {                         \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return Tuple::with(__private::unchecked_neg(primitive_value), false);   \
    else                                                                      \
      return Tuple::with(MIN, true);                                          \
  }                                                                           \
                                                                              \
  /** Saturating integer negation. Computes -self, returning MAX if self ==   \
   * MIN instead of overflowing.                                              \
   */                                                                         \
  constexpr T saturating_neg() const& noexcept {                              \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return __private::unchecked_neg(primitive_value);                       \
    else                                                                      \
      return MAX;                                                             \
  }                                                                           \
                                                                              \
  /** Wrapping (modular) negation. Computes -self, wrapping around at the     \
   * boundary of the type.                                                    \
   *                                                                          \
   * The only case where such wrapping can occur is when one negates MIN on   \
   * a signed type (where MIN is the negative minimal value for the type);    \
   * this is a positive value that is too large to represent in the type. In  \
   * such a case, this function returns MIN itself.                           \
   */                                                                         \
  constexpr T wrapping_neg() const& noexcept {                                \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return __private::unchecked_neg(primitive_value);                       \
    else                                                                      \
      return MIN;                                                             \
  }                                                                           \
  static_assert(true)

#define _sus__signed_rem(T, PrimitiveT)                                        \
  /** Checked integer remainder. Computes self % rhs, returning None if rhs == \
   * 0 or the division results in overflow.                                    \
   */                                                                          \
  constexpr Option<T> checked_rem(const T& rhs) const& noexcept {              \
    if (__private::div_overflows(primitive_value, rhs.primitive_value))        \
        [[unlikely]]                                                           \
      return Option<T>::none();                                                \
    else                                                                       \
      return Option<T>::some(                                                  \
          static_cast<PrimitiveT>(primitive_value % rhs.primitive_value));     \
  }                                                                            \
                                                                               \
  /** Calculates the remainder when self is divided by rhs.                    \
   *                                                                           \
   * Returns a tuple of the remainder after dividing along with a boolean      \
   * indicating whether an arithmetic overflow would occur. If an overflow     \
   * would occur then 0 is returned.                                           \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_rem(const T& rhs) const& noexcept {              \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0);                                    \
    if (__private::div_overflows_nonzero(                                      \
            ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value))   \
        [[unlikely]] {                                                         \
      return Tuple::with(PrimitiveT{0}, true);                                 \
    } else {                                                                   \
      return Tuple::with(                                                      \
          static_cast<PrimitiveT>(primitive_value % rhs.primitive_value),      \
          false);                                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) remainder. Computes self % rhs, wrapping around at    \
   * the boundary of the type.                                                 \
   *                                                                           \
   * Such wrap-around never actually occurs mathematically; implementation     \
   * artifacts make x % y invalid for MIN / -1 on a signed type (where MIN     \
   * is the negative minimal value). In such a case, this function returns 0.  \
   */                                                                          \
  constexpr T wrapping_rem(const T& rhs) const& noexcept {                     \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0);                                    \
    if (__private::div_overflows_nonzero(                                      \
            ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value))   \
        [[likely]] {                                                           \
      return PrimitiveT{0};                                                    \
    } else {                                                                   \
      return static_cast<PrimitiveT>(primitive_value % rhs.primitive_value);   \
    }                                                                          \
  }                                                                            \
  static_assert(true)

#define _sus__signed_euclid(T, PrimitiveT)                                     \
  /** Calculates the quotient of Euclidean division of self by rhs.            \
   *                                                                           \
   * This computes the integer q such that self = q * rhs + r, with r =        \
   * self.rem_euclid(rhs) and 0 <= r < abs(rhs).                               \
   *                                                                           \
   * In other words, the result is self / rhs rounded to the integer q such    \
   * that self >= q * rhs. If self > 0, this is equal to round towards zero    \
   * (the default in Rust); if self < 0, this is equal to round towards +/-    \
   * infinity.                                                                 \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0 or the division results in overflow. \
   */                                                                          \
  constexpr T div_euclid(const T& rhs) const& noexcept {                       \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(                                                              \
        !__private::div_overflows(primitive_value, rhs.primitive_value));      \
    return __private::div_euclid(::sus::marker::unsafe_fn, primitive_value,    \
                                 rhs.primitive_value);                         \
  }                                                                            \
                                                                               \
  /** Checked Euclidean division. Computes self.div_euclid(rhs), returning     \
   * None if rhs == 0 or the division results in overflow.                     \
   */                                                                          \
  constexpr Option<T> checked_div_euclid(const T& rhs) const& noexcept {       \
    if (__private::div_overflows(primitive_value, rhs.primitive_value))        \
        [[unlikely]] {                                                         \
      return Option<T>::none();                                                \
    } else {                                                                   \
      return Option<T>::some(__private::div_euclid(                            \
          ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value));    \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Calculates the quotient of Euclidean division self.div_euclid(rhs).      \
   *                                                                           \
   * Returns a tuple of the divisor along with a boolean indicating whether an \
   * arithmetic overflow would occur. If an overflow would occur then self is  \
   * returned.                                                                 \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_div_euclid(const T& rhs) const& noexcept {       \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0);                                    \
    if (__private::div_overflows_nonzero(                                      \
            ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value))   \
        [[unlikely]] {                                                         \
      return Tuple::with(MIN, true);                                           \
    } else {                                                                   \
      return Tuple::with(                                                      \
          __private::div_euclid(::sus::marker::unsafe_fn, primitive_value,     \
                                rhs.primitive_value),                          \
          false);                                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Wrapping Euclidean division. Computes self.div_euclid(rhs), wrapping     \
   * around at the boundary of the type.                                       \
   *                                                                           \
   * Wrapping will only occur in MIN / -1 on a signed type (where MIN is the   \
   * negative minimal value for the type). This is equivalent to -MIN, a       \
   * positive value that is too large to represent in the type. In this case,  \
   * this method returns MIN itself.                                           \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  constexpr T wrapping_div_euclid(const T& rhs) const& noexcept {              \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0);                                    \
    if (__private::div_overflows_nonzero(                                      \
            ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value))   \
        [[unlikely]] {                                                         \
      return MIN;                                                              \
    } else {                                                                   \
      return __private::div_euclid(::sus::marker::unsafe_fn, primitive_value,  \
                                   rhs.primitive_value);                       \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Calculates the least nonnegative remainder of self (mod rhs).            \
   *                                                                           \
   * This is done as if by the Euclidean division algorithm – given r =      \
   * self.rem_euclid(rhs), self = rhs * self.div_euclid(rhs) + r, and 0 <= r < \
   * abs(rhs).                                                                 \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0 or the division results in overflow. \
   */                                                                          \
  constexpr T rem_euclid(const T& rhs) const& noexcept {                       \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(                                                              \
        !__private::div_overflows(primitive_value, rhs.primitive_value));      \
    return __private::rem_euclid(::sus::marker::unsafe_fn, primitive_value,    \
                                 rhs.primitive_value);                         \
  }                                                                            \
                                                                               \
  /** Checked Euclidean remainder. Computes self.rem_euclid(rhs), returning    \
   * None if rhs == 0 or the division results in overflow.                     \
   */                                                                          \
  constexpr Option<T> checked_rem_euclid(const T& rhs) const& noexcept {       \
    if (__private::div_overflows(primitive_value, rhs.primitive_value))        \
        [[unlikely]] {                                                         \
      return Option<T>::none();                                                \
    } else {                                                                   \
      return Option<T>::some(__private::rem_euclid(                            \
          ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value));    \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Overflowing Euclidean remainder. Calculates self.rem_euclid(rhs).        \
   *                                                                           \
   * Returns a tuple of the remainder after dividing along with a boolean      \
   * indicating whether an arithmetic overflow would occur. If an overflow     \
   * would occur then 0 is returned.                                           \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_rem_euclid(const T& rhs) const& noexcept {       \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0);                                    \
    if (__private::div_overflows_nonzero(                                      \
            ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value))   \
        [[unlikely]] {                                                         \
      return Tuple::with(PrimitiveT{0}, true);                                 \
    } else {                                                                   \
      return Tuple::with(                                                      \
          __private::rem_euclid(::sus::marker::unsafe_fn, primitive_value,     \
                                rhs.primitive_value),                          \
          false);                                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Wrapping Euclidean remainder. Computes self.rem_euclid(rhs), wrapping    \
   * around at the boundary of the type.                                       \
   *                                                                           \
   * Wrapping will only occur in MIN % -1 on a signed type (where MIN is the   \
   * negative minimal value for the type). In this case, this method returns   \
   * 0.                                                                        \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  constexpr T wrapping_rem_euclid(const T& rhs) const& noexcept {              \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0);                                    \
    if (__private::div_overflows_nonzero(                                      \
            ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value))   \
        [[unlikely]] {                                                         \
      return PrimitiveT{0};                                                    \
    } else {                                                                   \
      return __private::rem_euclid(::sus::marker::unsafe_fn, primitive_value,  \
                                   rhs.primitive_value);                       \
    }                                                                          \
  }                                                                            \
  static_assert(true)

#define _sus__signed_shift(T)                                                  \
  /** Checked shift left. Computes `*this << rhs`, returning None if rhs is    \
   * larger than or equal to the number of bits in self.                       \
   */                                                                          \
  constexpr Option<T> checked_shl(const u32& rhs) const& noexcept {            \
    const auto out =                                                           \
        __private::shl_with_overflow(primitive_value, rhs.primitive_value);    \
    if (!out.overflow) [[likely]]                                              \
      return Option<T>::some(out.value);                                       \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Shifts self left by rhs bits.                                            \
   *                                                                           \
   * Returns a tuple of the shifted version of self along with a boolean       \
   * indicating whether the shift value was larger than or equal to the number \
   * of bits. If the shift value is too large, then value is masked (N-1)      \
   * where N is the number of bits, and this value is then used to perform the \
   * shift.                                                                    \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_shl(const u32& rhs) const& noexcept {            \
    const auto out =                                                           \
        __private::shl_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple::with(out.value, out.overflow);                               \
  }                                                                            \
                                                                               \
  /** Panic-free bitwise shift-left; yields `*this << mask(rhs)`, where mask   \
   * removes any high-order bits of `rhs` that would cause the shift to exceed \
   * the bitwidth of the type.                                                 \
   *                                                                           \
   * Note that this is not the same as a rotate-left; the RHS of a wrapping    \
   * shift-left is restricted to the range of the type, rather than the bits   \
   * shifted out of the LHS being returned to the other end. The primitive     \
   * integer types all implement a rotate_left function, which may be what you \
   * want instead.                                                             \
   */                                                                          \
  constexpr T wrapping_shl(const u32& rhs) const& noexcept {                   \
    return __private::shl_with_overflow(primitive_value, rhs.primitive_value)  \
        .value;                                                                \
  }                                                                            \
                                                                               \
  /** Checked shift right. Computes `*this >> rhs`, returning None if rhs is   \
   * larger than or equal to the number of bits in self.                       \
   */                                                                          \
  constexpr Option<T> checked_shr(const u32& rhs) const& noexcept {            \
    const auto out =                                                           \
        __private::shr_with_overflow(primitive_value, rhs.primitive_value);    \
    if (!out.overflow) [[likely]]                                              \
      return Option<T>::some(out.value);                                       \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Shifts self right by rhs bits.                                           \
   *                                                                           \
   * Returns a tuple of the shifted version of self along with a boolean       \
   * indicating whether the shift value was larger than or equal to the number \
   * of bits. If the shift value is too large, then value is masked (N-1)      \
   * where N is the number of bits, and this value is then used to perform the \
   * shift.                                                                    \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_shr(const u32& rhs) const& noexcept {            \
    const auto out =                                                           \
        __private::shr_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple::with(out.value, out.overflow);                               \
  }                                                                            \
                                                                               \
  /** Panic-free bitwise shift-right; yields `*this >> mask(rhs)`, where mask  \
   * removes any high-order bits of `rhs` that would cause the shift to exceed \
   * the bitwidth of the type.                                                 \
   *                                                                           \
   * Note that this is not the same as a rotate-right; the RHS of a wrapping   \
   * shift-right is restricted to the range of the type, rather than the bits  \
   * shifted out of the LHS being returned to the other end. The primitive     \
   * integer types all implement a rotate_right function, which may be what    \
   * you want instead.                                                         \
   */                                                                          \
  constexpr T wrapping_shr(const u32& rhs) const& noexcept {                   \
    return __private::shr_with_overflow(primitive_value, rhs.primitive_value)  \
        .value;                                                                \
  }                                                                            \
  static_assert(true)

#define _sus__signed_sub(T, PrimitiveT, UnsignedT)                            \
  /** Checked integer subtraction. Computes self - rhs, returning None if     \
   * overflow occurred.                                                       \
   */                                                                         \
  constexpr Option<T> checked_sub(const T& rhs) const& {                      \
    const auto out =                                                          \
        __private::sub_with_overflow(primitive_value, rhs.primitive_value);   \
    if (!out.overflow) [[likely]]                                             \
      return Option<T>::some(out.value);                                      \
    else                                                                      \
      return Option<T>::none();                                               \
  }                                                                           \
                                                                              \
  /** Checked integer subtraction with an unsigned rhs. Computes self - rhs,  \
   * returning None if overflow occurred.                                     \
   */                                                                         \
  constexpr Option<T> checked_sub_unsigned(const UnsignedT& rhs) const& {     \
    const auto out = __private::sub_with_overflow_unsigned(                   \
        primitive_value, rhs.primitive_value);                                \
    if (!out.overflow) [[likely]]                                             \
      return Option<T>::some(out.value);                                      \
    else                                                                      \
      return Option<T>::none();                                               \
  }                                                                           \
                                                                              \
  /** Calculates self - rhs                                                   \
   *                                                                          \
   * Returns a tuple of the subtraction along with a boolean indicating       \
   * whether an arithmetic overflow would occur. If an overflow would have    \
   * occurred then the wrapped value is returned.                             \
   */                                                                         \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>         \
  constexpr Tuple overflowing_sub(const T& rhs) const& noexcept {             \
    const auto out =                                                          \
        __private::sub_with_overflow(primitive_value, rhs.primitive_value);   \
    return Tuple::with(out.value, out.overflow);                              \
  }                                                                           \
                                                                              \
  /** Calculates self - rhs with an unsigned rhs.                             \
   *                                                                          \
   * Returns a tuple of the subtraction along with a boolean indicating       \
   * whether an arithmetic overflow would occur. If an overflow would have    \
   * occurred then the wrapped value is returned.                             \
   */                                                                         \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>         \
  constexpr Tuple overflowing_sub_unsigned(const UnsignedT& rhs)              \
      const& noexcept {                                                       \
    const auto out = __private::sub_with_overflow_unsigned(                   \
        primitive_value, rhs.primitive_value);                                \
    return Tuple::with(out.value, out.overflow);                              \
  }                                                                           \
                                                                              \
  /** Saturating integer subtraction. Computes self - rhs, saturating at the  \
   * numeric bounds instead of overflowing.                                   \
   */                                                                         \
  constexpr T saturating_sub(const T& rhs) const& {                           \
    return __private::saturating_sub(primitive_value, rhs.primitive_value);   \
  }                                                                           \
                                                                              \
  /** Saturating integer subtraction with an unsigned rhs. Computes self -    \
   * rhs, saturating at the numeric bounds instead of overflowing.            \
   */                                                                         \
  constexpr T saturating_sub_unsigned(const UnsignedT& rhs) const& {          \
    const auto out = __private::sub_with_overflow_unsigned(                   \
        primitive_value, rhs.primitive_value);                                \
    if (!out.overflow) [[likely]]                                             \
      return out.value;                                                       \
    else                                                                      \
      return MIN;                                                             \
  }                                                                           \
                                                                              \
  /** Unchecked integer subtraction. Computes self - rhs, assuming overflow   \
   * cannot occur.                                                            \
   */                                                                         \
  constexpr T unchecked_sub(::sus::marker::UnsafeFnMarker, const T& rhs)      \
      const& {                                                                \
    return static_cast<PrimitiveT>(primitive_value - rhs.primitive_value);    \
  }                                                                           \
                                                                              \
  /** Wrapping (modular) subtraction. Computes self - rhs, wrapping around at \
   * the boundary of the type.                                                \
   */                                                                         \
  constexpr T wrapping_sub(const T& rhs) const& {                             \
    return __private::wrapping_sub(primitive_value, rhs.primitive_value);     \
  }                                                                           \
                                                                              \
  /** Wrapping (modular) subtraction with an unsigned rhs. Computes self -    \
   * rhs, wrapping around at the boundary of the type.                        \
   */                                                                         \
  constexpr T wrapping_sub_unsigned(const UnsignedT& rhs) const& {            \
    return __private::sub_with_overflow_unsigned(primitive_value,             \
                                                 rhs.primitive_value)         \
        .value;                                                               \
  }                                                                           \
  static_assert(true)

#define _sus__signed_bits(T)                                                   \
  /** Returns the number of ones in the binary representation of the current   \
   * value.                                                                    \
   */                                                                          \
  constexpr u32 count_ones() const& noexcept {                                 \
    return __private::count_ones(__private::into_unsigned(primitive_value));   \
  }                                                                            \
                                                                               \
  /** Returns the number of zeros in the binary representation of the current  \
   * value.                                                                    \
   */                                                                          \
  constexpr u32 count_zeros() const& noexcept {                                \
    return (~(*this)).count_ones();                                            \
  }                                                                            \
                                                                               \
  /** Returns the number of leading ones in the binary representation of the   \
   * current value.                                                            \
   */                                                                          \
  constexpr u32 leading_ones() const& noexcept {                               \
    return (~(*this)).leading_zeros();                                         \
  }                                                                            \
                                                                               \
  /** Returns the number of leading zeros in the binary representation of the  \
   * current value.                                                            \
   */                                                                          \
  constexpr u32 leading_zeros() const& noexcept {                              \
    return __private::leading_zeros(                                           \
        __private::into_unsigned(primitive_value));                            \
  }                                                                            \
                                                                               \
  /** Returns the number of trailing ones in the binary representation of the  \
   * current value.                                                            \
   */                                                                          \
  constexpr u32 trailing_ones() const& noexcept {                              \
    return (~(*this)).trailing_zeros();                                        \
  }                                                                            \
                                                                               \
  /** Returns the number of trailing zeros in the binary representation of the \
   * current value.                                                            \
   */                                                                          \
  constexpr u32 trailing_zeros() const& noexcept {                             \
    return __private::trailing_zeros(                                          \
        __private::into_unsigned(primitive_value));                            \
  }                                                                            \
                                                                               \
  /** Reverses the order of bits in the integer. The least significant bit     \
   * becomes the most significant bit, second least-significant bit becomes    \
   * second most-significant bit, etc.                                         \
   */                                                                          \
  constexpr T reverse_bits() const& noexcept {                                 \
    return __private::into_signed(                                             \
        __private::reverse_bits(__private::into_unsigned(primitive_value)));   \
  }                                                                            \
                                                                               \
  /** Shifts the bits to the left by a specified amount, `n`, wrapping the     \
   * truncated bits to the end of the resulting integer.                       \
   *                                                                           \
   * Please note this isn't the same operation as the `<<` shifting operator!  \
   */                                                                          \
  constexpr T rotate_left(const u32& n) const& noexcept {                      \
    return __private::into_signed(__private::rotate_left(                      \
        __private::into_unsigned(primitive_value), n.primitive_value));        \
  }                                                                            \
                                                                               \
  /** Shifts the bits to the right by a specified amount, n, wrapping the      \
   * truncated bits to the beginning of the resulting integer.                 \
   *                                                                           \
   * Please note this isn't the same operation as the >> shifting operator!    \
   */                                                                          \
  constexpr T rotate_right(const u32& n) const& noexcept {                     \
    return __private::into_signed(__private::rotate_right(                     \
        __private::into_unsigned(primitive_value), n.primitive_value));        \
  }                                                                            \
                                                                               \
  /** Reverses the byte order of the integer.                                  \
   */                                                                          \
  constexpr T swap_bytes() const& noexcept {                                   \
    return __private::into_signed(                                             \
        __private::swap_bytes(__private::into_unsigned(primitive_value)));     \
  }                                                                            \
  static_assert(true)

#define _sus__signed_pow(T)                                                    \
  /**  Raises self to the power of `exp`, using exponentiation by squaring. */ \
  constexpr inline T pow(const u32& rhs) const& noexcept {                     \
    const auto out =                                                           \
        __private::pow_with_overflow(primitive_value, rhs.primitive_value);    \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(!out.overflow);                                               \
    return out.value;                                                          \
  }                                                                            \
                                                                               \
  /** Checked exponentiation. Computes `##T##::pow(exp)`, returning None if    \
   * overflow occurred.                                                        \
   */                                                                          \
  constexpr Option<T> checked_pow(const u32& rhs) const& noexcept {            \
    const auto out =                                                           \
        __private::pow_with_overflow(primitive_value, rhs.primitive_value);    \
    /* TODO: Allow opting out of all overflow checks? */                       \
    if (!out.overflow) [[likely]]                                              \
      return Option<T>::some(out.value);                                       \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Raises self to the power of `exp`, using exponentiation by squaring.     \
   *                                                                           \
   * Returns a tuple of the exponentiation along with a bool indicating        \
   * whether an overflow happened.                                             \
   */                                                                          \
  template <int&..., class Tuple = ::sus::tuple_type::Tuple<T, bool>>          \
  constexpr Tuple overflowing_pow(const u32& exp) const& noexcept {            \
    const auto out =                                                           \
        __private::pow_with_overflow(primitive_value, exp.primitive_value);    \
    return Tuple::with(out.value, out.overflow);                               \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) exponentiation. Computes self.pow(exp), wrapping      \
   * around at the boundary of the type.                                       \
   */                                                                          \
  constexpr T wrapping_pow(const u32& exp) const& noexcept {                   \
    return __private::wrapping_pow(primitive_value, exp.primitive_value);      \
  }                                                                            \
  static_assert(true)

#define _sus__signed_log(T)                                                   \
  /** Returns the base 2 logarithm of the number, rounded down.               \
   *                                                                          \
   * Returns None if the number is negative or zero.                          \
   */                                                                         \
  constexpr Option<u32> checked_log2() const& {                               \
    if (primitive_value <= 0) [[unlikely]] {                                  \
      return Option<u32>::none();                                             \
    } else {                                                                  \
      uint32_t zeros = __private::leading_zeros_nonzero(                      \
          ::sus::marker::unsafe_fn,                                           \
          __private::into_unsigned(primitive_value));                         \
      return Option<u32>::some(BITS - 1_u32 - u32(zeros));                    \
    }                                                                         \
  }                                                                           \
                                                                              \
  /** Returns the base 2 logarithm of the number, rounded down.               \
   *                                                                          \
   * # Panics                                                                 \
   * When the number is zero or negative the function will panic.             \
   */                                                                         \
  constexpr u32 log2() const& {                                               \
    /* TODO: Allow opting out of all overflow checks? */                      \
    return checked_log2().unwrap();                                           \
  }                                                                           \
                                                                              \
  /** Returns the base 10 logarithm of the number, rounded down.              \
   *                                                                          \
   * Returns None if the number is negative or zero.                          \
   */                                                                         \
  constexpr Option<u32> checked_log10() const& {                              \
    if (primitive_value <= 0) [[unlikely]] {                                  \
      return Option<u32>::none();                                             \
    } else {                                                                  \
      return Option<u32>::some(__private::int_log10::T(primitive_value));     \
    }                                                                         \
  }                                                                           \
                                                                              \
  /** Returns the base 10 logarithm of the number, rounded down.              \
   *                                                                          \
   * # Panics                                                                 \
   * When the number is zero or negative the function will panic.             \
   */                                                                         \
  constexpr u32 log10() const& {                                              \
    /* TODO: Allow opting out of all overflow checks? */                      \
    return checked_log10().unwrap();                                          \
  }                                                                           \
                                                                              \
  /** Returns the logarithm of the number with respect to an arbitrary base,  \
   * rounded down.                                                            \
   *                                                                          \
   * Returns None if the number is negative or zero, or if the base is not at \
   * least 2.                                                                 \
   *                                                                          \
   * This method might not be optimized owing to implementation details;      \
   * `checked_log2` can produce results more efficiently for base 2, and      \
   * `checked_log10` can produce results more efficiently for base 10.        \
   */                                                                         \
  constexpr Option<u32> checked_log(const T& base) const& noexcept {          \
    if (primitive_value <= 0 || base.primitive_value <= 1) [[unlikely]] {     \
      return Option<u32>::none();                                             \
    } else {                                                                  \
      auto n = uint32_t{0};                                                   \
      auto r = primitive_value;                                               \
      const auto b = base.primitive_value;                                    \
      while (r >= b) {                                                        \
        r /= b;                                                               \
        n += 1u;                                                              \
      }                                                                       \
      return Option<u32>::some(n);                                            \
    }                                                                         \
  }                                                                           \
                                                                              \
  /** Returns the logarithm of the number with respect to an arbitrary base,  \
   * rounded down.                                                            \
   *                                                                          \
   * This method might not be optimized owing to implementation details; log2 \
   * can produce results more efficiently for base 2, and log10 can produce   \
   * results more efficiently for base 10.                                    \
   *                                                                          \
   * # Panics                                                                 \
   * When the number is negative, zero, or if the base is not at least 2.     \
   */                                                                         \
  constexpr u32 log(const T& base) const& noexcept {                          \
    return checked_log(base).unwrap();                                        \
  }                                                                           \
  static_assert(true)

#define _sus__signed_endian(T, UnsignedT, Bytes)                              \
  /** Converts an integer from big endian to the target's endianness.         \
   *                                                                          \
   * On big endian this is a no-op. On little endian the bytes are swapped.   \
   */                                                                         \
  static constexpr T from_be(const T& x) noexcept {                           \
    if (::sus::assertions::is_big_endian())                                   \
      return x;                                                               \
    else                                                                      \
      return x.swap_bytes();                                                  \
  }                                                                           \
                                                                              \
  /** Converts an integer from little endian to the target's endianness.      \
   *                                                                          \
   * On little endian this is a no-op. On big endian the bytes are swapped.   \
   */                                                                         \
  static constexpr T from_le(const T& x) noexcept {                           \
    if (::sus::assertions::is_little_endian())                                \
      return x;                                                               \
    else                                                                      \
      return x.swap_bytes();                                                  \
  }                                                                           \
                                                                              \
  /** Converts self to big endian from the target's endianness.               \
   *                                                                          \
   * On big endian this is a no-op. On little endian the bytes are swapped.   \
   */                                                                         \
  constexpr T to_be() const& noexcept {                                       \
    if (::sus::assertions::is_big_endian())                                   \
      return *this;                                                           \
    else                                                                      \
      return swap_bytes();                                                    \
  }                                                                           \
                                                                              \
  /** Converts self to little endian from the target's endianness.            \
   *                                                                          \
   * On little endian this is a no-op. On big endian the bytes are swapped.   \
   */                                                                         \
  constexpr T to_le() const& noexcept {                                       \
    if (::sus::assertions::is_little_endian())                                \
      return *this;                                                           \
    else                                                                      \
      return swap_bytes();                                                    \
  }                                                                           \
                                                                              \
  /** Return the memory representation of this integer as a byte array in     \
   * big-endian (network) byte order.                                         \
   */                                                                         \
  template <int&..., class Array = ::sus::containers::Array<u8, Bytes>>       \
  constexpr Array to_be_bytes() const& noexcept {                             \
    return to_be().to_ne_bytes sus_clang_bug_58835(<Array>)();                \
  }                                                                           \
                                                                              \
  /** Return the memory representation of this integer as a byte array in     \
   * little-endian byte order.                                                \
   */                                                                         \
  template <int&..., class Array = ::sus::containers::Array<u8, Bytes>>       \
  constexpr Array to_le_bytes() const& noexcept {                             \
    return to_le().to_ne_bytes sus_clang_bug_58835(<Array>)();                \
  }                                                                           \
                                                                              \
  /** Return the memory representation of this integer as a byte array in     \
   * native byte order.                                                       \
   *                                                                          \
   * As the target platform's native endianness is used, portable code should \
   * use `to_be_bytes()` or `to_le_bytes()`, as appropriate, instead.         \
   */                                                                         \
  template <sus_clang_bug_58835_else(int&..., ) class Array =                 \
                ::sus::containers::Array<u8, Bytes>>                          \
  constexpr Array to_ne_bytes() const& noexcept {                             \
    auto bytes = Array::with_uninitialized(::sus::marker::unsafe_fn);         \
    if (std::is_constant_evaluated()) {                                       \
      auto uval = __private::into_unsigned(primitive_value);                  \
      for (auto i = size_t{0}; i < Bytes; ++i) {                              \
        const auto last_byte = static_cast<uint8_t>(uval & 0xff);             \
        if (sus::assertions::is_little_endian())                              \
          bytes[i] = last_byte;                                               \
        else                                                                  \
          bytes[Bytes - 1 - i] = last_byte;                                   \
        /* If T is one byte, this shift would be UB. But it's also not needed \
           since the loop will not run again. */                              \
        if constexpr (Bytes > 1) uval >>= 8u;                                 \
      }                                                                       \
      return bytes;                                                           \
    } else {                                                                  \
      ::sus::ptr::copy_nonoverlapping(                                        \
          ::sus::marker::unsafe_fn,                                           \
          reinterpret_cast<const char*>(&primitive_value),                    \
          reinterpret_cast<char*>(bytes.as_mut_ptr()), Bytes);                \
      return bytes;                                                           \
    }                                                                         \
  }                                                                           \
                                                                              \
  /** Create an integer value from its representation as a byte array in big  \
   * endian.                                                                  \
   */                                                                         \
  template <int&..., class Array = ::sus::containers::Array<u8, Bytes>,       \
            std::convertible_to<Array> U>                                     \
  static constexpr T from_be_bytes(const U& bytes) noexcept {                 \
    return from_be(from_ne_bytes(bytes));                                     \
  }                                                                           \
                                                                              \
  /** Create an integer value from its representation as a byte array in      \
   * little endian.                                                           \
   */                                                                         \
  template <int&..., class Array = ::sus::containers::Array<u8, Bytes>,       \
            std::convertible_to<Array> U>                                     \
  static constexpr T from_le_bytes(const U& bytes) noexcept {                 \
    return from_le(from_ne_bytes(bytes));                                     \
  }                                                                           \
                                                                              \
  /** Create an integer value from its memory representation as a byte array  \
   * in native endianness.                                                    \
   *                                                                          \
   * As the target platform's native endianness is used, portable code likely \
   * wants to use `from_be_bytes()` or `from_le_bytes()`, as appropriate      \
   * instead.                                                                 \
   */                                                                         \
  template <int&..., class Array = ::sus::containers::Array<u8, Bytes>,       \
            std::convertible_to<Array> U>                                     \
  static constexpr T from_ne_bytes(const U& bytes) noexcept {                 \
    using Unsigned = decltype(__private::into_unsigned(primitive_value));     \
    Unsigned val;                                                             \
    if (std::is_constant_evaluated()) {                                       \
      val = Unsigned{0};                                                      \
      for (auto i = size_t{0}; i < Bytes; ++i) {                              \
        val |= bytes[i].primitive_value << (Bytes - size_t{1} - i);           \
      }                                                                       \
    } else {                                                                  \
      ::sus::ptr::copy_nonoverlapping(                                        \
          ::sus::marker::unsafe_fn,                                           \
          reinterpret_cast<const char*>(bytes.as_ptr()),                      \
          reinterpret_cast<char*>(&val), Bytes);                              \
    }                                                                         \
    return __private::into_signed(val);                                       \
  }                                                                           \
  static_assert(true)

#define _sus__signed_hash_equal_to(Type)                                   \
  template <>                                                              \
  struct hash<Type> {                                                      \
    size_t operator()(const Type& u) const { return u.primitive_value; }   \
  };                                                                       \
  template <>                                                              \
  struct equal_to<Type> {                                                  \
    auto operator()(const Type& l, const Type& r) const { return l == r; } \
  };                                                                       \
  static_assert(true)
