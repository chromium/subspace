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

#include "assertions/check.h"
#include "assertions/endian.h"
#include "num/__private/int_log10.h"
#include "num/__private/intrinsics.h"
#include "num/__private/literals.h"
#include "num/__private/ptr_type.h"
#include "num/integer_concepts.h"
#include "option/option.h"

namespace sus::containers {
template <class T, size_t N>
  requires(N <= PTRDIFF_MAX)
class Array;
}

namespace sus::num {
struct u8;
}

namespace sus::tuple {
template <class T, class... Ts>
class Tuple;
}

#define _sus__unsigned_impl(T, PrimitiveT, SignedT) \
  _sus__unsigned_storage(PrimitiveT);               \
  _sus__unsigned_constants(T, PrimitiveT);          \
  _sus__unsigned_construct(T, PrimitiveT);          \
  _sus__unsigned_from(T, PrimitiveT);               \
  _sus__unsigned_integer_comparison(T);             \
  _sus__unsigned_unary_ops(T);                      \
  _sus__unsigned_binary_logic_ops(T);               \
  _sus__unsigned_binary_bit_ops(T);                 \
  _sus__unsigned_mutable_logic_ops(T);              \
  _sus__unsigned_mutable_bit_ops(T);                \
  _sus__unsigned_abs(T);                            \
  _sus__unsigned_add(T, SignedT);                   \
  _sus__unsigned_div(T);                            \
  _sus__unsigned_mul(T);                            \
  _sus__unsigned_neg(T, PrimitiveT);                \
  _sus__unsigned_rem(T);                            \
  _sus__unsigned_euclid(T);                         \
  _sus__unsigned_shift(T);                          \
  _sus__unsigned_sub(T);                            \
  _sus__unsigned_bits(T);                           \
  _sus__unsigned_pow(T);                            \
  _sus__unsigned_log(T);                            \
  _sus__unsigned_power_of_two(T, PrimitiveT);       \
  _sus__unsigned_endian(T, PrimitiveT, sizeof(PrimitiveT))

#define _sus__unsigned_storage(PrimitiveT)                                    \
  /** The inner primitive value, in case it needs to be unwrapped from the    \
   * type. Avoid using this member except to convert when a consumer requires \
   * it.                                                                      \
   */                                                                         \
  PrimitiveT primitive_value { 0u }

#define _sus__unsigned_constants(T, PrimitiveT)                             \
  static constexpr auto MIN_PRIMITIVE = __private::min_value<PrimitiveT>(); \
  static constexpr auto MAX_PRIMITIVE = __private::max_value<PrimitiveT>(); \
  static constexpr inline T MIN() noexcept { return MIN_PRIMITIVE; }        \
  static constexpr inline T MAX() noexcept { return MAX_PRIMITIVE; }        \
  static constexpr inline u32 BITS() noexcept {                             \
    return __private::num_bits<PrimitiveT>();                               \
  }                                                                         \
  static_assert(true)

#define _sus__unsigned_construct(T, PrimitiveT)                                \
  /** Default constructor, which sets the integer to 0.                        \
   *                                                                           \
   * The trivial copy and move constructors are implicitly declared, as is the \
   * trivial destructor.                                                       \
   */                                                                          \
  constexpr inline T() noexcept = default;                                     \
                                                                               \
  /** Assignment from the underlying primitive type.                           \
   */                                                                          \
  template <std::same_as<PrimitiveT> P> /* Prevent implicit conversions. */    \
  constexpr inline void operator=(P v) noexcept {                              \
    primitive_value = v;                                                       \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_from(T, PrimitiveT)                                     \
  /** Constructs a ##T## from a signed integer type (i8, i16, i32, etc).       \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   */                                                                          \
  template <Signed S>                                                          \
  static constexpr T from(S s) noexcept {                                      \
    ::sus::check(s.primitive_value >= 0);                                      \
    constexpr auto umax = __private::into_unsigned(S::MAX_PRIMITIVE);          \
    if constexpr (MAX_PRIMITIVE < umax)                                        \
      ::sus::check(__private::into_unsigned(s.primitive_value) <=              \
                   MAX_PRIMITIVE);                                             \
    return T(static_cast<PrimitiveT>(s.primitive_value));                      \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from an unsigned integer type (u8, u16, u32, etc).    \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   */                                                                          \
  template <Unsigned U>                                                        \
  static constexpr T from(U u) noexcept {                                      \
    if constexpr (MAX_PRIMITIVE < U::MAX_PRIMITIVE)                            \
      ::sus::check(u.primitive_value <= MAX_PRIMITIVE);                        \
    return T(static_cast<PrimitiveT>(u.primitive_value));                      \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from a signed primitive integer type (int, long,      \
   * etc).                                                                     \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   */                                                                          \
  template <SignedPrimitiveInteger S>                                          \
  static constexpr T from(S s) {                                               \
    ::sus::check(s >= 0);                                                      \
    constexpr auto umax = __private::into_unsigned(__private::max_value<S>()); \
    if constexpr (MAX_PRIMITIVE < umax)                                        \
      ::sus::check(__private::into_unsigned(s) <= MAX_PRIMITIVE);              \
    return T(static_cast<PrimitiveT>(s));                                      \
  }                                                                            \
                                                                               \
  /** Constructs a ##T## from an unsigned primitive integer type (unsigned     \
   * int, unsigned long, etc).                                                 \
   *                                                                           \
   * # Panics                                                                  \
   * The function will panic if the input value is out of range for ##T##.     \
   */                                                                          \
  template <UnsignedPrimitiveInteger U>                                        \
  static constexpr T from(U u) {                                               \
    if constexpr (MAX_PRIMITIVE < __private::max_value<U>())                   \
      ::sus::check(u <= MAX_PRIMITIVE);                                        \
    return T(static_cast<PrimitiveT>(u));                                      \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_integer_comparison(T)                                  \
  /** sus::concepts::Eq<##T##> trait. */                                      \
  friend constexpr inline bool operator==(const T& l, const T& r) noexcept {  \
    return (l.primitive_value <=> r.primitive_value) == 0;                    \
  }                                                                           \
  /** sus::concepts::Ord<##T##> trait. */                                     \
  friend constexpr inline auto operator<=>(const T& l, const T& r) noexcept { \
    return l.primitive_value <=> r.primitive_value;                           \
  }                                                                           \
  static_assert(true)

#define _sus__unsigned_unary_ops(T)                      \
  /** sus::concepts::Neg trait intentionally omitted. */ \
  /** sus::concepts::BitNot trait. */                    \
  constexpr inline T operator~() const& noexcept {       \
    return __private::unchecked_not(primitive_value);    \
  }                                                      \
  static_assert(true)

#define _sus__unsigned_binary_logic_ops(T)                                  \
  /** sus::concepts::Add<##T##> trait. */                                   \
  friend constexpr inline T operator+(const T& l, const T& r) noexcept {    \
    const auto out =                                                        \
        __private::add_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::concepts::Sub<##T##> trait. */                                   \
  friend constexpr inline T operator-(const T& l, const T& r) noexcept {    \
    const auto out =                                                        \
        __private::sub_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::concepts::Mul<##T##> trait. */                                   \
  friend constexpr inline T operator*(const T& l, const T& r) noexcept {    \
    const auto out =                                                        \
        __private::mul_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::concepts::Div<##T##> trait. */                                   \
  friend constexpr inline T operator/(const T& l, const T& r) noexcept {    \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r.primitive_value != 0u);                                  \
    return __private::unchecked_div(l.primitive_value, r.primitive_value);  \
  }                                                                         \
  /** sus::concepts::Rem<##T##> trait. */                                   \
  friend constexpr inline T operator%(const T& l, const T& r) noexcept {    \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r.primitive_value != 0u);                                  \
    return __private::unchecked_rem(l.primitive_value, r.primitive_value);  \
  }                                                                         \
  static_assert(true)

#define _sus__unsigned_binary_bit_ops(T)                                    \
  /** sus::concepts::BitAnd<##T##> trait. */                                \
  friend constexpr inline T operator&(const T& l, const T& r) noexcept {    \
    return __private::unchecked_and(l.primitive_value, r.primitive_value);  \
  }                                                                         \
  /** sus::concepts::BitOr<##T##> trait. */                                 \
  friend constexpr inline T operator|(const T& l, const T& r) noexcept {    \
    return __private::unchecked_or(l.primitive_value, r.primitive_value);   \
  }                                                                         \
  /** sus::concepts::BitXor<##T##> trait. */                                \
  friend constexpr inline T operator^(const T& l, const T& r) noexcept {    \
    return __private::unchecked_xor(l.primitive_value, r.primitive_value);  \
  }                                                                         \
  /** sus::concepts::Shl trait. */                                          \
  friend constexpr inline T operator<<(const T& l, const u32& r) noexcept { \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r < BITS());                                               \
    return __private::unchecked_shl(l.primitive_value, r.primitive_value);  \
  }                                                                         \
  /** sus::concepts::Shr trait. */                                          \
  friend constexpr inline T operator>>(const T& l, const u32& r) noexcept { \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r < BITS());                                               \
    return __private::unchecked_shr(l.primitive_value, r.primitive_value);  \
  }                                                                         \
  static_assert(true)

#define _sus__unsigned_mutable_logic_ops(T)                               \
  /** sus::concepts::AddAssign<##T##> trait. */                           \
  constexpr inline void operator+=(T r)& noexcept {                       \
    const auto out =                                                      \
        __private::add_with_overflow(primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(!out.overflow);                                          \
    primitive_value = out.value;                                          \
  }                                                                       \
  /** sus::concepts::SubAssign<##T##> trait. */                           \
  constexpr inline void operator-=(T r)& noexcept {                       \
    const auto out =                                                      \
        __private::sub_with_overflow(primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(!out.overflow);                                          \
    primitive_value = out.value;                                          \
  }                                                                       \
  /** sus::concepts::MulAssign<##T##> trait. */                           \
  constexpr inline void operator*=(T r)& noexcept {                       \
    const auto out =                                                      \
        __private::mul_with_overflow(primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(!out.overflow);                                          \
    primitive_value = out.value;                                          \
  }                                                                       \
  /** sus::concepts::DivAssign<##T##> trait. */                           \
  constexpr inline void operator/=(T r)& noexcept {                       \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(r.primitive_value != 0u);                                \
    primitive_value /= r.primitive_value;                                 \
  }                                                                       \
  /** sus::concepts::RemAssign<##T##> trait. */                           \
  constexpr inline void operator%=(T r)& noexcept {                       \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(r.primitive_value != 0u);                                \
    primitive_value %= r.primitive_value;                                 \
  }                                                                       \
  static_assert(true)

#define _sus__unsigned_mutable_bit_ops(T)                     \
  /** sus::concepts::BitAndAssign<##T##> trait. */            \
  constexpr inline void operator&=(T r)& noexcept {           \
    primitive_value &= r.primitive_value;                     \
  }                                                           \
  /** sus::concepts::BitOrAssign<##T##> trait. */             \
  constexpr inline void operator|=(T r)& noexcept {           \
    primitive_value |= r.primitive_value;                     \
  }                                                           \
  /** sus::concepts::BitXorAssign<##T##> trait. */            \
  constexpr inline void operator^=(T r)& noexcept {           \
    primitive_value ^= r.primitive_value;                     \
  }                                                           \
  /** sus::concepts::ShlAssign trait. */                      \
  constexpr inline void operator<<=(const u32& r)& noexcept { \
    /* TODO: Allow opting out of all overflow checks? */      \
    ::sus::check(r < BITS());                                 \
    primitive_value <<= r.primitive_value;                    \
  }                                                           \
  /** sus::concepts::ShrAssign trait. */                      \
  constexpr inline void operator>>=(const u32& r)& noexcept { \
    /* TODO: Allow opting out of all overflow checks? */      \
    ::sus::check(r < BITS());                                 \
    primitive_value >>= r.primitive_value;                    \
  }                                                           \
  static_assert(true)

#define _sus__unsigned_abs(T)                                              \
  /** Computes the absolute difference between self and other.             \
   */                                                                      \
  constexpr T abs_diff(const T& r) const& noexcept {                       \
    if (primitive_value >= r.primitive_value)                              \
      return __private::unchecked_sub(primitive_value, r.primitive_value); \
    else                                                                   \
      return __private::unchecked_sub(r.primitive_value, primitive_value); \
  }                                                                        \
  static_assert(true)

#define _sus__unsigned_add(T, SignedT)                                         \
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
  template <std::same_as<SignedT> S>                                           \
  constexpr Option<T> checked_add_signed(const S& rhs) const& noexcept {       \
    const auto out = __private::add_with_overflow_signed(primitive_value,      \
                                                         rhs.primitive_value); \
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
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
  constexpr Tuple overflowing_add(const T& rhs) const& noexcept {              \
    const auto out =                                                           \
        __private::add_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple::with(out.value, out.overflow);                               \
  }                                                                            \
                                                                               \
  /** Calculates self + rhs with an unsigned rhs                               \
   *                                                                           \
   * Returns a tuple of the addition along with a boolean indicating whether   \
   * an arithmetic overflow would occur. If an overflow would have occurred    \
   * then the wrapped value is returned.                                       \
   */                                                                          \
  template <std::same_as<SignedT> S,                                           \
            std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
  constexpr Tuple overflowing_add_signed(const S& rhs) const& noexcept {       \
    const auto r = __private::add_with_overflow_signed(primitive_value,        \
                                                       rhs.primitive_value);   \
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
  template <std::same_as<SignedT> S>                                           \
  constexpr T saturating_add_signed(const S& rhs) const& noexcept {            \
    const auto r = __private::add_with_overflow_signed(primitive_value,        \
                                                       rhs.primitive_value);   \
    if (!r.overflow) [[likely]]                                                \
      return r.value;                                                          \
    else {                                                                     \
      /* TODO: Can this be done without a branch? If it's complex or uses      \
       * compiler stuff, move into intrinsics. */                              \
      if (rhs.primitive_value >= 0)                                            \
        return MAX();                                                          \
      else                                                                     \
        return MIN();                                                          \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Unchecked integer addition. Computes self + rhs, assuming overflow       \
   * cannot occur.                                                             \
   *                                                                           \
   * # Safety                                                                  \
   * This function is allowed to result in undefined behavior when `self + rhs \
   * > ##T##::MAX` or `self + rhs < ##T##::MIN`, i.e. when `checked_add()`     \
   * would return None.                                                        \
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
  template <std::same_as<SignedT> S>                                           \
  constexpr T wrapping_add_signed(const S& rhs) const& noexcept {              \
    return __private::add_with_overflow_signed(primitive_value,                \
                                               rhs.primitive_value)            \
        .value;                                                                \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_div(T)                                                  \
  /** Checked integer division. Computes self / rhs, returning None if `rhs == \
   * 0`.                                                                       \
   */                                                                          \
  constexpr Option<T> checked_div(const T& rhs) const& noexcept {              \
    if (rhs.primitive_value != 0u) [[likely]]                                  \
      return Option<T>::some(                                                  \
          __private::unchecked_div(primitive_value, rhs.primitive_value));     \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Calculates the divisor when self is divided by rhs.                      \
   *                                                                           \
   * Returns a tuple of the divisor along with a boolean indicating whether an \
   *arithmetic overflow would occur. Note that for unsigned integers overflow  \
   *never occurs, so the second value is always false.                         \
   *                                                                           \
   * #Panics                                                                   \
   *This function will panic if rhs is 0.                                      \
   */                                                                          \
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
  constexpr Tuple overflowing_div(const T& rhs) const& noexcept {              \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0u);                                   \
    return Tuple::with(                                                        \
        __private::unchecked_div(primitive_value, rhs.primitive_value),        \
        false);                                                                \
  }                                                                            \
                                                                               \
  /** Saturating integer division. Computes self / rhs, saturating at the      \
   numeric bounds instead of overflowing.                                      \
   *                                                                           \
   * #Panics                                                                   \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  constexpr T saturating_div(const T& rhs) const& noexcept {                   \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0u);                                   \
    return __private::unchecked_div(primitive_value, rhs.primitive_value);     \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) division. Computes self / rhs. Wrapped division on    \
   * unsigned types is just normal division. There's no way wrapping could     \
   * ever happen. This function exists, so that all operations are accounted   \
   * for in the wrapping operations.                                           \
   *                                                                           \
   * #Panics                                                                   \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  constexpr T wrapping_div(const T& rhs) const& noexcept {                     \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0u);                                   \
    return __private::unchecked_div(primitive_value, rhs.primitive_value);     \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_mul(T)                                                  \
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
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
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
   * This function is allowed to result in undefined behavior when `self * rhs \
   * > ##T##::MAX` or `self * rhs < ##T##::MIN`, i.e. when `checked_mul()`     \
   * would return None.                                                        \
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

#define _sus__unsigned_neg(T, PrimitiveT)                                      \
  /** Checked negation. Computes -self, returning None unless `self == 0`.     \
   *                                                                           \
   * Note that negating any positive integer will overflow.                    \
   */                                                                          \
  constexpr Option<T> checked_neg() const& noexcept {                          \
    if (primitive_value == 0u)                                                 \
      return Option<T>::some(T(PrimitiveT{0u}));                               \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Negates self in an overflowing fashion.                                  \
   *                                                                           \
   * Returns `~self + 1` using wrapping operations to return the value that    \
   * represents the negation of this unsigned value. Note that for positive    \
   * unsigned values overflow always occurs, but negating 0 does not overflow. \
   */                                                                          \
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
  constexpr Tuple overflowing_neg() const& noexcept {                          \
    return Tuple::with((~(*this)).wrapping_add(T(PrimitiveT{1u})),             \
                       primitive_value != 0u);                                 \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) negation. Computes `-self`, wrapping around at the    \
    boundary of the type.                                                      \
   *                                                                           \
   * Since unsigned types do not have negative equivalents all applications of \
   * this function will wrap (except for -0). For values smaller than the      \
   * corresponding signed type's maximum the result is the same as casting     \
   * the corresponding signed value. Any larger values are equivalent to       \
   * `MAX + 1 - (val - MAX - 1)` where MAX is the corresponding signed type's  \
   * maximum.                                                                  \
   */                                                                          \
  constexpr T wrapping_neg() const& noexcept {                                 \
    return (T(PrimitiveT{0u})).wrapping_sub(*this);                            \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_rem(T)                                                  \
  /** Checked integer remainder. Computes `self % rhs`, returning None if `rhs \
   * == 0`.                                                                    \
   */                                                                          \
  constexpr Option<T> checked_rem(const T& rhs) const& noexcept {              \
    if (rhs.primitive_value != 0u) [[likely]]                                  \
      return Option<T>::some(                                                  \
          __private::unchecked_rem(primitive_value, rhs.primitive_value));     \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Calculates the remainder when self is divided by rhs.                    \
   *                                                                           \
   * Returns a tuple of the remainder after dividing along with a boolean      \
   * indicating whether an arithmetic overflow would occur. Note that for      \
   * unsigned integers overflow never occurs, so the second value is always    \
   * false.                                                                    \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
  constexpr Tuple overflowing_rem(const T& rhs) const& noexcept {              \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0u);                                   \
    return Tuple::with(                                                        \
        __private::unchecked_rem(primitive_value, rhs.primitive_value),        \
        false);                                                                \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) remainder. Computes self % rhs. Wrapped remainder     \
   * calculation on unsigned types is just the regular remainder calculation.  \
   *                                                                           \
   * There's no way wrapping could ever happen. This function exists, so that  \
   * all operations are accounted for in the wrapping operations.              \
   *                                                                           \
   * # Panics                                                                  \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  constexpr T wrapping_rem(const T& rhs) const& noexcept {                     \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs.primitive_value != 0u);                                   \
    return __private::unchecked_rem(primitive_value, rhs.primitive_value);     \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_euclid(T)                                                \
  /** Performs Euclidean division.                                              \
   *                                                                            \
   * Since, for the positive integers, all common definitions of division are   \
   * equal, this is exactly equal to self / rhs.                                \
   *                                                                            \
   * # Panics                                                                   \
   * This function will panic if rhs is 0.                                      \
   */                                                                           \
  constexpr T div_euclid(const T& rhs) const& noexcept {                        \
    /* TODO: Allow opting out of all overflow checks? */                        \
    ::sus::check(rhs.primitive_value != 0u);                                    \
    return __private::unchecked_div(primitive_value, rhs.primitive_value);      \
  }                                                                             \
                                                                                \
  /** Checked Euclidean division. Computes self.div_euclid(rhs), returning      \
   * None if rhs == 0.                                                          \
   */                                                                           \
  constexpr Option<T> checked_div_euclid(const T& rhs) const& noexcept {        \
    if (rhs.primitive_value == 0u) [[unlikely]] {                               \
      return Option<T>::none();                                                 \
    } else {                                                                    \
      return Option<T>::some(                                                   \
          __private::unchecked_div(primitive_value, rhs.primitive_value));      \
    }                                                                           \
  }                                                                             \
                                                                                \
  /** Calculates the quotient of Euclidean division self.div_euclid(rhs).       \
   *                                                                            \
   * Returns a tuple of the divisor along with a boolean indicating whether an  \
   * arithmetic overflow would occur. Note that for unsigned integers overflow  \
   * never occurs, so the second value is always false. Since, for the          \
   * positive integers, all common definitions of division are equal, this is   \
   * exactly equal to self.overflowing_div(rhs).                                \
   *                                                                            \
   * # Panics                                                                   \
   * This function will panic if rhs is 0.                                      \
   */                                                                           \
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                  \
                ::sus::tuple::Tuple<T, bool>>                                   \
  constexpr Tuple overflowing_div_euclid(const T& rhs) const& noexcept {        \
    /* TODO: Allow opting out of all overflow checks? */                        \
    ::sus::check(rhs.primitive_value != 0u);                                    \
    return Tuple::with(                                                         \
        __private::unchecked_div(primitive_value, rhs.primitive_value),         \
        false);                                                                 \
  }                                                                             \
                                                                                \
  /** Wrapping Euclidean division. Computes self.div_euclid(rhs). Wrapped       \
   * division on unsigned types is just normal division.                        \
   *                                                                            \
   * There's no way wrapping could ever happen. This function exists so that    \
   * all operations are accounted for in the wrapping operations. Since, for    \
   * the positive integers, all common definitions of division are equal, this  \
   * is exactly equal to self.wrapping_div(rhs).                                \
   *                                                                            \
   * # Panics                                                                   \
   * This function will panic if rhs is 0.                                      \
   */                                                                           \
  constexpr T wrapping_div_euclid(const T& rhs) const& noexcept {               \
    /* TODO: Allow opting out of all overflow checks? */                        \
    ::sus::check(rhs.primitive_value != 0u);                                    \
    return __private::unchecked_div(primitive_value, rhs.primitive_value);      \
  }                                                                             \
                                                                                \
  /** Calculates the least remainder of self (mod rhs).                         \
   *                                                                            \
   * Since, for the positive integers, all common definitions of division are   \
   * equal, this is exactly equal to self % rhs. \                              \
   *                                                                            \
   * # Panics                                                                   \
   * This function will panic if rhs is 0.                                      \
   */                                                                           \
  constexpr T rem_euclid(const T& rhs) const& noexcept {                        \
    /* TODO: Allow opting out of all overflow checks? */                        \
    ::sus::check(rhs.primitive_value != 0u);                                    \
    return __private::unchecked_rem(primitive_value, rhs.primitive_value);      \
  }                                                                             \
                                                                                \
  /** Checked Euclidean modulo. Computes self.rem_euclid(rhs), returning None   \
   * if rhs == 0.                                                               \
   */                                                                           \
  constexpr Option<T> checked_rem_euclid(const T& rhs) const& noexcept {        \
    if (rhs.primitive_value == 0u) [[unlikely]] {                               \
      return Option<T>::none();                                                 \
    } else {                                                                    \
      return Option<T>::some(                                                   \
          __private::unchecked_rem(primitive_value, rhs.primitive_value));      \
    }                                                                           \
  }                                                                             \
                                                                                \
  /** Calculates the remainder self.rem_euclid(rhs) as if by Euclidean          \
   * division.                                                                  \
   *                                                                            \
   * Returns a tuple of the modulo after dividing along with a boolean          \
   * indicating whether an arithmetic overflow would occur. Note that for       \
   * unsigned integers overflow never occurs, so the second value is always     \
   * false. Since, for the positive integers, all common definitions of         \
   * division are equal, this operation is exactly equal to                     \
   * self.overflowing_rem(rhs).                                                 \
   *                                                                            \
   * # Panics                                                                   \
   * This function will panic if rhs is 0.                                      \
   */                                                                           \
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                  \
                ::sus::tuple::Tuple<T, bool>>                                   \
  constexpr Tuple overflowing_rem_euclid(const T& rhs) const& noexcept {        \
    /* TODO: Allow opting out of all overflow checks? */                        \
    ::sus::check(rhs.primitive_value != 0u);                                    \
    return Tuple::with(                                                         \
        __private::unchecked_rem(primitive_value, rhs.primitive_value),         \
        false);                                                                 \
  }                                                                             \
                                                                                \
  /** Wrapping Euclidean modulo. Computes self.rem_euclid(rhs). Wrapped modulo  \
   * calculation on unsigned types is just the regular remainder calculation.   \
   *                                                                            \
   * There’s no way wrapping could ever happen. This function exists, so that \
   * all operations are accounted for in the wrapping operations. Since, for    \
   * the positive integers, all common definitions of division are equal, this  \
   * is exactly equal to self.wrapping_rem(rhs). \                              \
   *                                                                            \
   * # Panics                                                                   \
   * This function will panic if rhs is 0.                                      \
   */                                                                           \
  constexpr T wrapping_rem_euclid(const T& rhs) const& noexcept {               \
    /* TODO: Allow opting out of all overflow checks? */                        \
    ::sus::check(rhs.primitive_value != 0u);                                    \
    return __private::unchecked_rem(primitive_value, rhs.primitive_value);      \
  }                                                                             \
  static_assert(true)

#define _sus__unsigned_shift(T)                                                \
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
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
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
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
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

#define _sus__unsigned_sub(T)                                                  \
  /** Checked integer subtraction. Computes self - rhs, returning None if      \
   * overflow occurred.                                                        \
   */                                                                          \
  constexpr Option<T> checked_sub(const T& rhs) const& {                       \
    const auto out =                                                           \
        __private::sub_with_overflow(primitive_value, rhs.primitive_value);    \
    if (!out.overflow) [[likely]]                                              \
      return Option<T>::some(out.value);                                       \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Calculates self - rhs                                                    \
   *                                                                           \
   * Returns a tuple of the subtraction along with a boolean indicating        \
   * whether an arithmetic overflow would occur. If an overflow would have     \
   * occurred then the wrapped value is returned.                              \
   */                                                                          \
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
  constexpr Tuple overflowing_sub(const T& rhs) const& noexcept {              \
    const auto out =                                                           \
        __private::sub_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple::with(out.value, out.overflow);                               \
  }                                                                            \
                                                                               \
  /** Saturating integer subtraction. Computes self - rhs, saturating at the   \
   * numeric bounds instead of overflowing.                                    \
   */                                                                          \
  constexpr T saturating_sub(const T& rhs) const& {                            \
    return __private::saturating_sub(primitive_value, rhs.primitive_value);    \
  }                                                                            \
                                                                               \
  /** Unchecked integer subtraction. Computes self - rhs, assuming overflow    \
   * cannot occur.                                                             \
   *                                                                           \
   * # Safety                                                                  \
   * This function is allowed to result in undefined behavior when `self - rhs \
   * > ##T##::MAX` or `self - rhs < ##T##::MIN`, i.e. when `checked_sub()`     \
   * would return None.                                                        \
   */                                                                          \
  constexpr T unchecked_sub(::sus::marker::UnsafeFnMarker, const T& rhs)       \
      const& {                                                                 \
    return __private::unchecked_sub(primitive_value, rhs.primitive_value);     \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) subtraction. Computes self - rhs, wrapping around at  \
   * the boundary of the type.                                                 \
   */                                                                          \
  constexpr T wrapping_sub(const T& rhs) const& {                              \
    return __private::wrapping_sub(primitive_value, rhs.primitive_value);      \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_bits(T)                                                 \
  /** Returns the number of ones in the binary representation of the current   \
   * value.                                                                    \
   */                                                                          \
  constexpr u32 count_ones() const& noexcept {                                 \
    return __private::count_ones(primitive_value);                             \
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
    return __private::leading_zeros(primitive_value);                          \
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
    return __private::trailing_zeros(primitive_value);                         \
  }                                                                            \
                                                                               \
  /** Reverses the order of bits in the integer. The least significant bit     \
   * becomes the most significant bit, second least-significant bit becomes    \
   * second most-significant bit, etc.                                         \
   */                                                                          \
  constexpr T reverse_bits() const& noexcept {                                 \
    return __private::reverse_bits(primitive_value);                           \
  }                                                                            \
                                                                               \
  /** Shifts the bits to the left by a specified amount, `n`, wrapping the     \
   * truncated bits to the end of the resulting integer.                       \
   *                                                                           \
   * Please note this isn't the same operation as the `<<` shifting operator!  \
   */                                                                          \
  constexpr T rotate_left(const u32& n) const& noexcept {                      \
    return __private::rotate_left(primitive_value, n.primitive_value);         \
  }                                                                            \
                                                                               \
  /** Shifts the bits to the right by a specified amount, n, wrapping the      \
   * truncated bits to the beginning of the resulting integer.                 \
   *                                                                           \
   * Please note this isn't the same operation as the >> shifting operator!    \
   */                                                                          \
  constexpr T rotate_right(const u32& n) const& noexcept {                     \
    return __private::rotate_right(primitive_value, n.primitive_value);        \
  }                                                                            \
                                                                               \
  /** Reverses the byte order of the integer.                                  \
   */                                                                          \
  constexpr T swap_bytes() const& noexcept {                                   \
    return __private::swap_bytes(primitive_value);                             \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_pow(T)                                                  \
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
  template <std::same_as<::sus::tuple::Tuple<T, bool>> Tuple =                 \
                ::sus::tuple::Tuple<T, bool>>                                  \
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

#define _sus__unsigned_log(T)                                                 \
  /** Returns the base 2 logarithm of the number, rounded down.               \
   *                                                                          \
   * Returns None if the number is zero.                                      \
   */                                                                         \
  constexpr Option<u32> checked_log2() const& {                               \
    if (primitive_value == 0u) [[unlikely]] {                                 \
      return Option<u32>::none();                                             \
    } else {                                                                  \
      uint32_t zeros =                                                        \
          __private::leading_zeros_nonzero(unsafe_fn, primitive_value);       \
      return Option<u32>::some(BITS() - u32(1u) - u32(zeros));                \
    }                                                                         \
  }                                                                           \
                                                                              \
  /** Returns the base 2 logarithm of the number, rounded down.               \
   *                                                                          \
   * # Panics                                                                 \
   * When the number is zero the function will panic. \                       \
   */                                                                         \
  constexpr u32 log2() const& {                                               \
    /* TODO: Allow opting out of all overflow checks? */                      \
    return checked_log2().unwrap();                                           \
  }                                                                           \
                                                                              \
  /** Returns the base 10 logarithm of the number, rounded down.              \
   *                                                                          \
   * Returns None if the number is zero.                                      \
   */                                                                         \
  constexpr Option<u32> checked_log10() const& {                              \
    if (primitive_value == 0u) [[unlikely]] {                                 \
      return Option<u32>::none();                                             \
    } else {                                                                  \
      return Option<u32>::some(__private::int_log10::T(primitive_value));     \
    }                                                                         \
  }                                                                           \
                                                                              \
  /** Returns the base 10 logarithm of the number, rounded down.              \
   *                                                                          \
   * # Panics                                                                 \
   * When the number is zero the function will panic.                         \
   */                                                                         \
  constexpr u32 log10() const& {                                              \
    /* TODO: Allow opting out of all overflow checks? */                      \
    return checked_log10().unwrap();                                          \
  }                                                                           \
                                                                              \
  /** Returns the logarithm of the number with respect to an arbitrary base,  \
   * rounded down.                                                            \
   *                                                                          \
   * Returns None if the number is zero, or if the base is not at least 2.    \
   *                                                                          \
   * This method might not be optimized owing to implementation details;      \
   * `checked_log2` can produce results more efficiently for base 2, and      \
   * `checked_log10` can produce results more efficiently for base 10.        \
   */                                                                         \
  constexpr Option<u32> checked_log(const T& base) const& noexcept {          \
    if (primitive_value == 0u || base.primitive_value <= 1u) [[unlikely]] {   \
      return Option<u32>::none();                                             \
    } else {                                                                  \
      auto n = uint32_t{0u};                                                  \
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
   * When the number is zero, or if the base is not at least 2, the function  \
   * will panic.                                                              \
   */                                                                         \
  constexpr u32 log(const T& base) const& noexcept {                          \
    return checked_log(base).unwrap();                                        \
  }                                                                           \
  static_assert(true)

#define _sus__unsigned_power_of_two(T, PrimitiveT)                            \
  /** Returns the smallest power of two greater than or equal to self.        \
   *                                                                          \
   * # Panics                                                                 \
   * The function panics when the return value overflows (i.e., `self > (1 << \
   * (N-1))` for type uN). */                                                 \
  constexpr T next_power_of_two() noexcept {                                  \
    const auto one_less =                                                     \
        __private::one_less_than_next_power_of_two(primitive_value);          \
    return T(one_less) + T(PrimitiveT{1u});                                   \
  }                                                                           \
                                                                              \
  /** Returns the smallest power of two greater than or equal to n.           \
   *                                                                          \
   * If the next power of two is greater than the type's maximum value, None  \
   * is returned, otherwise the power of two is wrapped in Some.              \
   */                                                                         \
  constexpr Option<T> checked_next_power_of_two() noexcept {                  \
    const auto one_less =                                                     \
        __private::one_less_than_next_power_of_two(primitive_value);          \
    return T(one_less).checked_add(T(PrimitiveT{1u}));                        \
  }                                                                           \
                                                                              \
  /** Returns the smallest power of two greater than or equal to n.           \
   *                                                                          \
   * If the next power of two is greater than the type's maximum value, the   \
   * return value is wrapped to 0.                                            \
   */                                                                         \
  constexpr T wrapping_next_power_of_two() noexcept {                         \
    const auto one_less =                                                     \
        __private::one_less_than_next_power_of_two(primitive_value);          \
    return T(one_less).wrapping_add(T(PrimitiveT{1u}));                       \
  }                                                                           \
  static_assert(true)

#define _sus__unsigned_endian(T, PrimitiveT, Bytes)                           \
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
  template <std::same_as<::sus::containers::Array<u8, Bytes>> Array =         \
                ::sus::containers::Array<u8, Bytes>>                          \
  constexpr Array to_be_bytes() const& noexcept {                             \
    return to_be().to_ne_bytes();                                             \
  }                                                                           \
                                                                              \
  /** Return the memory representation of this integer as a byte array in     \
   * little-endian byte order.                                                \
   */                                                                         \
  template <std::same_as<::sus::containers::Array<u8, Bytes>> Array =         \
                ::sus::containers::Array<u8, Bytes>>                          \
  constexpr Array to_le_bytes() const& noexcept {                             \
    return to_le().to_ne_bytes();                                             \
  }                                                                           \
                                                                              \
  /** Return the memory representation of this integer as a byte array in     \
   * native byte order.                                                       \
   *                                                                          \
   * As the target platform's native endianness is used, portable code should \
   * use `to_be_bytes()` or `to_le_bytes()`, as appropriate, instead.         \
   */                                                                         \
  template <std::same_as<::sus::containers::Array<u8, Bytes>> Array =         \
                ::sus::containers::Array<u8, Bytes>>                          \
  constexpr Array to_ne_bytes() const& noexcept {                             \
    auto bytes = Array::with_uninitialized(unsafe_fn);                        \
    if (std::is_constant_evaluated()) {                                       \
      auto uval = primitive_value;                                            \
      for (auto i = size_t{0}; i < sizeof(T); ++i) {                          \
        const auto last_byte = static_cast<uint8_t>(uval & 0xff);             \
        if (sus::assertions::is_little_endian())                              \
          bytes.get_mut(i) = last_byte;                                       \
        else                                                                  \
          bytes.get_mut(sizeof(T) - 1 - i) = last_byte;                       \
        uval >>= 8u;                                                          \
      }                                                                       \
    } else {                                                                  \
      memcpy(bytes.as_ptr_mut(), &primitive_value, sizeof(T));                \
    }                                                                         \
    return bytes;                                                             \
  }                                                                           \
                                                                              \
  /** Create an integer value from its representation as a byte array in big  \
   * endian.                                                                  \
   */                                                                         \
  template <std::same_as<::sus::containers::Array<u8, Bytes>> Array =         \
                ::sus::containers::Array<u8, Bytes>>                          \
  static constexpr T from_be_bytes(const Array& bytes) noexcept {             \
    return from_be(from_ne_bytes(bytes));                                     \
  }                                                                           \
                                                                              \
  /** Create an integer value from its representation as a byte array in      \
   * little endian.                                                           \
   */                                                                         \
  template <std::same_as<::sus::containers::Array<u8, Bytes>> Array =         \
                ::sus::containers::Array<u8, Bytes>>                          \
  static constexpr T from_le_bytes(const Array& bytes) noexcept {             \
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
  template <std::same_as<::sus::containers::Array<u8, Bytes>> Array =         \
                ::sus::containers::Array<u8, Bytes>>                          \
  static constexpr T from_ne_bytes(const Array& bytes) noexcept {             \
    PrimitiveT val;                                                           \
    if (std::is_constant_evaluated()) {                                       \
      val = 0u;                                                               \
      for (auto i = size_t{0}; i < sizeof(T); ++i) {                          \
        val |= bytes.get(i).primitive_value << (sizeof(T) - 1 - i);           \
      }                                                                       \
    } else {                                                                  \
      memcpy(&val, bytes.as_ptr(), sizeof(T));                                \
    }                                                                         \
    return val;                                                               \
  }                                                                           \
  static_assert(true)
