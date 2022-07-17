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

#include <compare>

#include "assertions/check.h"
#include "assertions/endian.h"
#include "containers/array.h"
#include "num/__private/int_log10.h"
#include "num/__private/intrinsics.h"
#include "option/option.h"
#include "tuple/tuple.h"

#define _sus__unsigned_constants(T, Max)                             \
  static constexpr auto MIN_PRIMITIVE =                              \
      __private::min_value<primitive_type>();                        \
  static constexpr auto MAX_PRIMITIVE =                              \
      __private::max_value<primitive_type>();                        \
  static constexpr inline T MIN() noexcept { return MIN_PRIMITIVE; } \
  static constexpr inline T MAX() noexcept { return MAX_PRIMITIVE; } \
  static constexpr inline u32 BITS() noexcept {                      \
    return __private::num_bits<primitive_type>();                    \
  }                                                                  \
  static_assert(true)

#define _sus__unsigned_impl(T, Bytes, SignedT, LargerT) \
  _sus__unsigned_from(T);                               \
  _sus__unsigned_integer_comparison(T);                 \
  _sus__unsigned_unary_ops(T);                          \
  _sus__unsigned_binary_logic_ops(T);                   \
  _sus__unsigned_binary_bit_ops(T);                     \
  _sus__unsigned_mutable_logic_ops(T);                  \
  _sus__unsigned_mutable_bit_ops(T);                    \
  _sus__unsigned_abs(T);                                \
  _sus__unsigned_add(T, SignedT);                       \
  _sus__unsigned_div(T);                                \
  _sus__unsigned_mul(T, LargerT);                       \
  _sus__unsigned_neg(T);                                \
  _sus__unsigned_rem(T);                                \
  _sus__unsigned_shift(T);                              \
  _sus__unsigned_sub(T);                                \
  _sus__unsigned_bits(T);                               \
  _sus__unsigned_pow(T);                                \
  _sus__unsigned_log(T);                                \
  _sus__unsigned_power_of_two(T);                       \
  _sus__unsigned_endian(T, Bytes)

#define _sus__unsigned_from(T)                                              \
  /** Constructs a ##T## from a signed integer type (i8, i16, i32, etc).    \
   *                                                                        \
   * # Panics                                                               \
   * The function will panic if the input value is out of range for ##T##.  \
   */                                                                       \
  template <Signed S>                                                       \
  static constexpr u32 from(S s) noexcept {                                 \
    ::sus::check(s.primitive_value >= 0);                                   \
    constexpr auto umax = static_cast<primitive_type>(S::MAX_PRIMITIVE);    \
    if constexpr (MAX_PRIMITIVE < umax)                                     \
      ::sus::check(static_cast<primitive_type>(s.primitive_value) <=        \
                   MAX_PRIMITIVE);                                          \
    return u32(static_cast<primitive_type>(s.primitive_value));             \
  }                                                                         \
                                                                            \
  /** Constructs a ##T## from an unsigned integer type (u8, u16, u32, etc). \
   *                                                                        \
   * # Panics                                                               \
   * The function will panic if the input value is out of range for ##T##.  \
   */                                                                       \
  template <Unsigned U>                                                     \
  static constexpr u32 from(U u) noexcept {                                 \
    if constexpr (MIN_PRIMITIVE > U::MIN_PRIMITIVE)                         \
      ::sus::check(u.primitive_value >= MIN_PRIMITIVE);                     \
    if constexpr (MAX_PRIMITIVE < U::MAX_PRIMITIVE)                         \
      ::sus::check(u.primitive_value <= MAX_PRIMITIVE);                     \
    return u32(static_cast<primitive_type>(u.primitive_value));             \
  }                                                                         \
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

#define _sus__unsigned_unary_ops(T)                                           \
  /** sus::concepts::Neg trait intentionally omitted. */                      \
  /** sus::concepts::BitNot trait. */                                         \
  constexpr inline T operator~() const& noexcept { return ~primitive_value; } \
  static_assert(true)

#define _sus__unsigned_binary_logic_ops(T)                                  \
  /** sus::concepts::Add<##T##> trait. */                                   \
  friend constexpr inline T operator+(const T& l, const T& r) noexcept {    \
    auto out =                                                              \
        __private::add_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::concepts::Sub<##T##> trait. */                                   \
  friend constexpr inline T operator-(const T& l, const T& r) noexcept {    \
    auto out =                                                              \
        __private::sub_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::concepts::Mul<##T##> trait. */                                   \
  friend constexpr inline T operator*(const T& l, const T& r) noexcept {    \
    auto out =                                                              \
        __private::mul_with_overflow(l.primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(!out.overflow);                                            \
    return out.value;                                                       \
  }                                                                         \
  /** sus::concepts::Div<##T##> trait. */                                   \
  friend constexpr inline T operator/(const T& l, const T& r) noexcept {    \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r.primitive_value != primitive_type{0});                   \
    return l.primitive_value / r.primitive_value;                           \
  }                                                                         \
  /** sus::concepts::Rem<##T##> trait. */                                   \
  friend constexpr inline T operator%(const T& l, const T& r) noexcept {    \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r.primitive_value != primitive_type{0});                   \
    return l.primitive_value % r.primitive_value;                           \
  }                                                                         \
  static_assert(true)

#define _sus__unsigned_binary_bit_ops(T)                                    \
  /** sus::concepts::BitAnd<##T##> trait. */                                \
  friend constexpr inline T operator&(const T& l, const T& r) noexcept {    \
    return l.primitive_value & r.primitive_value;                           \
  }                                                                         \
  /** sus::concepts::BitOr<##T##> trait. */                                 \
  friend constexpr inline T operator|(const T& l, const T& r) noexcept {    \
    return l.primitive_value | r.primitive_value;                           \
  }                                                                         \
  /** sus::concepts::BitXor<##T##> trait. */                                \
  friend constexpr inline T operator^(const T& l, const T& r) noexcept {    \
    return l.primitive_value ^ r.primitive_value;                           \
  }                                                                         \
  /** sus::concepts::Shl trait. */                                          \
  friend constexpr inline T operator<<(const T& l, const u32& r) noexcept { \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r < BITS());                                               \
    return l.primitive_value << r.primitive_value;                          \
  }                                                                         \
  /** sus::concepts::Shr trait. */                                          \
  friend constexpr inline T operator>>(const T& l, const u32& r) noexcept { \
    /* TODO: Allow opting out of all overflow checks? */                    \
    ::sus::check(r < BITS());                                               \
    return l.primitive_value >> r.primitive_value;                          \
  }                                                                         \
  static_assert(true)

#define _sus__unsigned_mutable_logic_ops(T)                               \
  /** sus::concepts::AddAssign<##T##> trait. */                           \
  constexpr inline void operator+=(T r)& noexcept {                       \
    auto out =                                                            \
        __private::add_with_overflow(primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(!out.overflow);                                          \
    primitive_value = out.value;                                          \
  }                                                                       \
  /** sus::concepts::SubAssign<##T##> trait. */                           \
  constexpr inline void operator-=(T r)& noexcept {                       \
    auto out =                                                            \
        __private::sub_with_overflow(primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(!out.overflow);                                          \
    primitive_value = out.value;                                          \
  }                                                                       \
  /** sus::concepts::MulAssign<##T##> trait. */                           \
  constexpr inline void operator*=(T r)& noexcept {                       \
    auto out =                                                            \
        __private::mul_with_overflow(primitive_value, r.primitive_value); \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(!out.overflow);                                          \
    primitive_value = out.value;                                          \
  }                                                                       \
  /** sus::concepts::DivAssign<##T##> trait. */                           \
  constexpr inline void operator/=(T r)& noexcept {                       \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(r.primitive_value != primitive_type{0});                 \
    primitive_value /= r.primitive_value;                                 \
  }                                                                       \
  /** sus::concepts::RemAssign<##T##> trait. */                           \
  constexpr inline void operator%=(T r)& noexcept {                       \
    /* TODO: Allow opting out of all overflow checks? */                  \
    ::sus::check(r.primitive_value != primitive_type{0});                 \
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

#define _sus__unsigned_abs(T)                                  \
  /** Computes the absolute difference between self and other. \
   */                                                          \
  constexpr T abs_diff(const T& r) const& noexcept {           \
    if (primitive_value >= r.primitive_value)                  \
      return primitive_value - r.primitive_value;              \
    else                                                       \
      return r.primitive_value - primitive_value;              \
  }                                                            \
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
  constexpr Tuple<T, bool> overflowing_add(const T& rhs) const& noexcept {     \
    const auto out =                                                           \
        __private::add_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple<T, bool>::with(out.value, out.overflow);                      \
  }                                                                            \
                                                                               \
  /** Calculates self + rhs with an unsigned rhs                               \
   *                                                                           \
   * Returns a tuple of the addition along with a boolean indicating whether   \
   * an arithmetic overflow would occur. If an overflow would have occurred    \
   * then the wrapped value is returned.                                       \
   */                                                                          \
  template <std::same_as<SignedT> S>                                           \
  constexpr Tuple<T, bool> overflowing_add_signed(const S& rhs)                \
      const& noexcept {                                                        \
    const auto r = __private::add_with_overflow_signed(primitive_value,        \
                                                       rhs.primitive_value);   \
    return Tuple<T, bool>::with(r.value, r.overflow);                          \
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
    return primitive_value + rhs.primitive_value;                              \
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
    if (rhs.primitive_value != primitive_type{0u}) [[likely]]                  \
      return Option<T>::some(primitive_value / rhs.primitive_value);           \
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
  constexpr Tuple<T, bool> overflowing_div(const T& rhs) const& noexcept {     \
    /* TODO: Allow opting out of all overflow checks? */                       \
    ::sus::check(rhs != primitive_type{0u});                                   \
    return Tuple<T, bool>::with(primitive_value / rhs.primitive_value, false); \
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
    ::sus::check(rhs != primitive_type{0u});                                   \
    return primitive_value / rhs.primitive_value;                              \
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
    ::sus::check(rhs != primitive_type{0u});                                   \
    return primitive_value / rhs.primitive_value;                              \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_mul(T, LargerT)                                         \
  /** Checked integer multiplication. Computes self * rhs, returning None if   \
   * overflow occurred.                                                        \
   */                                                                          \
  constexpr Option<T> checked_mul(const T& rhs) const& noexcept {              \
    auto out =                                                                 \
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
  constexpr Tuple<T, bool> overflowing_mul(const T& rhs) const& noexcept {     \
    auto r =                                                                   \
        __private::mul_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple<T, bool>::with(r.value, r.overflow);                          \
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
    return primitive_value * rhs.primitive_value;                              \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) multiplication. Computes self * rhs, wrapping around  \
   * at the boundary of the type.                                              \
   */                                                                          \
  constexpr T wrapping_mul(const T& rhs) const& noexcept {                     \
    return __private::wrapping_mul(primitive_value, rhs.primitive_value);      \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_neg(T)                                                  \
  /** Checked negation. Computes -self, returning None unless `self == 0`.     \
   *                                                                           \
   * Note that negating any positive integer will overflow.                    \
   */                                                                          \
  constexpr Option<T> checked_neg() const& noexcept {                          \
    if (primitive_value == primitive_type{0u})                                 \
      return Option<T>::some(T(0u));                                           \
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
  constexpr Tuple<T, bool> overflowing_neg() const& noexcept {                 \
    return Tuple<T, bool>::with((~(*this)).wrapping_add(T(1u)),                \
                                primitive_value != primitive_type{0});         \
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
    return (T(0u)).wrapping_sub(*this);                                        \
  }                                                                            \
  static_assert(true)

#define _sus__unsigned_rem(T)                                                   \
  /** Checked integer remainder. Computes `self % rhs`, returning None if `rhs  \
   * == 0`.                                                                     \
   */                                                                           \
  constexpr Option<T> checked_rem(const T& rhs) const& noexcept {               \
    if (rhs.primitive_value != primitive_type{0u}) [[likely]]                   \
      return Option<T>::some(primitive_value % rhs.primitive_value);            \
    else                                                                        \
      return Option<T>::none();                                                 \
  }                                                                             \
                                                                                \
  /** Calculates the remainder when self is divided by rhs.                     \
   *                                                                            \
   * Returns a tuple of the remainder after dividing along with a boolean       \
   * indicating whether an arithmetic overflow would occur. Note that for       \
   * unsigned integers overflow never occurs, so the second value is always     \
   * false.                                                                     \
   *                                                                            \
   * # Panics                                                                   \
   * This function will panic if rhs is 0.                                      \
   */                                                                           \
  constexpr Tuple<T, bool> overflowing_rem(const T& rhs) const& noexcept {      \
    /* TODO: Allow opting out of all overflow checks? */                        \
    ::sus::check(rhs != primitive_type{0u});                                    \
    return Tuple<T, bool>::with(primitive_value % rhs.primitive_value, false);  \
  }                                                                             \
                                                                                \
  /** Wrapping (modular) remainder. Computes self % rhs. Wrapped remainder      \
   * calculation on unsigned types is just the regular remainder calculation.   \
   * There’s no way wrapping could ever happen. This function exists, so that \
   * all operations are accounted for in the wrapping operations.               \
   *                                                                            \
   * # Panics                                                                   \
   * This function will panic if rhs is 0.                                      \
   */                                                                           \
  constexpr T wrapping_rem(const T& rhs) const& noexcept {                      \
    /* TODO: Allow opting out of all overflow checks? */                        \
    ::sus::check(rhs != primitive_type{0u});                                    \
    return primitive_value % rhs.primitive_value;                               \
  }                                                                             \
  static_assert(true)

#define _sus__unsigned_shift(T)                                                \
  /** Checked shift left. Computes `*this << rhs`, returning None if rhs is    \
   * larger than or equal to the number of bits in self.                       \
   */                                                                          \
  constexpr Option<T> checked_shl(const u32& rhs) const& noexcept {            \
    auto out =                                                                 \
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
  constexpr Tuple<T, bool> overflowing_shl(const u32& rhs) const& noexcept {   \
    auto r =                                                                   \
        __private::shl_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple<T, bool>::with(r.value, r.overflow);                          \
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
    auto out =                                                                 \
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
  constexpr Tuple<T, bool> overflowing_shr(const u32& rhs) const& noexcept {   \
    auto r =                                                                   \
        __private::shr_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple<T, bool>::with(r.value, r.overflow);                          \
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
    auto out =                                                                 \
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
  constexpr Tuple<T, bool> overflowing_sub(const T& rhs) const& noexcept {     \
    auto r =                                                                   \
        __private::sub_with_overflow(primitive_value, rhs.primitive_value);    \
    return Tuple<T, bool>::with(r.value, r.overflow);                          \
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
    return primitive_value - rhs.primitive_value;                              \
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
  constexpr /* TODO:u32 */ uint32_t leading_ones() const& noexcept {           \
    return (~(*this)).leading_zeros();                                         \
  }                                                                            \
                                                                               \
  /** Returns the number of leading zeros in the binary representation of the  \
   * current value.                                                            \
   */                                                                          \
  constexpr /* TODO:u32 */ uint32_t leading_zeros() const& noexcept {          \
    return __private::leading_zeros(primitive_value);                          \
  }                                                                            \
                                                                               \
  /** Returns the number of trailing ones in the binary representation of the  \
   * current value.                                                            \
   */                                                                          \
  constexpr /* TODO:u32 */ uint32_t trailing_ones() const& noexcept {          \
    return (~(*this)).trailing_zeros();                                        \
  }                                                                            \
                                                                               \
  /** Returns the number of trailing zeros in the binary representation of the \
   * current value.                                                            \
   */                                                                          \
  constexpr /* TODO:u32 */ uint32_t trailing_zeros() const& noexcept {         \
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
    auto out =                                                                 \
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
    auto out =                                                                 \
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
  constexpr Tuple<T, bool> overflowing_pow(const u32& exp) const& noexcept {   \
    auto r =                                                                   \
        __private::pow_with_overflow(primitive_value, exp.primitive_value);    \
    return Tuple<T, bool>::with(r.value, r.overflow);                          \
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
    if (primitive_value == primitive_type{0}) [[unlikely]] {                  \
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
    if (primitive_value == primitive_type{0}) [[unlikely]] {                  \
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
    if (primitive_value == primitive_type{0} ||                               \
        base.primitive_value <= primitive_type{1}) [[unlikely]] {             \
      return Option<u32>::none();                                             \
    } else {                                                                  \
      auto n = u32(0u);                                                       \
      auto r = primitive_value;                                               \
      const auto b = base.primitive_value;                                    \
      while (r >= b) {                                                        \
        r /= b;                                                               \
        n += u32(1u);                                                         \
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

#define _sus__unsigned_power_of_two(T)                                        \
  /** Returns the smallest power of two greater than or equal to self.        \
   *                                                                          \
   * # Panics                                                                 \
   * The function panics when the return value overflows (i.e., `self > (1 << \
   * (N-1))` for type uN). */                                                 \
  constexpr T next_power_of_two() noexcept {                                  \
    const primitive_type one_less =                                           \
        __private::one_less_than_next_power_of_two(primitive_value);          \
    return T(one_less) + T(1u);                                               \
  }                                                                           \
                                                                              \
  /** Returns the smallest power of two greater than or equal to n.           \
   *                                                                          \
   * If the next power of two is greater than the type's maximum value, None  \
   * is returned, otherwise the power of two is wrapped in Some.              \
   */                                                                         \
  constexpr Option<T> checked_next_power_of_two() noexcept {                  \
    const primitive_type one_less =                                           \
        __private::one_less_than_next_power_of_two(primitive_value);          \
    return T(one_less).checked_add(T(1u));                                    \
  }                                                                           \
                                                                              \
  /** Returns the smallest power of two greater than or equal to n.           \
   *                                                                          \
   * If the next power of two is greater than the type's maximum value, the   \
   * return value is wrapped to 0.                                            \
   */                                                                         \
  constexpr T wrapping_next_power_of_two() noexcept {                         \
    const primitive_type one_less =                                           \
        __private::one_less_than_next_power_of_two(primitive_value);          \
    return T(one_less).wrapping_add(T(1u));                                   \
  }                                                                           \
  static_assert(true)

#define _sus__unsigned_endian(T, Bytes)                                       \
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
  constexpr ::sus::Array</* TODO: u8 */ uint8_t, Bytes> to_be_bytes()         \
      const& noexcept {                                                       \
    return to_be().to_ne_bytes();                                             \
  }                                                                           \
                                                                              \
  /** Return the memory representation of this integer as a byte array in     \
   * little-endian byte order.                                                \
   */                                                                         \
  constexpr ::sus::Array</* TODO: u8 */ uint8_t, Bytes> to_le_bytes()         \
      const& noexcept {                                                       \
    return to_le().to_ne_bytes();                                             \
  }                                                                           \
                                                                              \
  /** Return the memory representation of this integer as a byte array in     \
   * native byte order.                                                       \
   *                                                                          \
   * As the target platform's native endianness is used, portable code should \
   * use `to_be_bytes()` or `to_le_bytes()`, as appropriate, instead.         \
   */                                                                         \
  constexpr ::sus::Array</* TODO: u8 */ uint8_t, Bytes> to_ne_bytes()         \
      const& noexcept {                                                       \
    auto bytes =                                                              \
        ::sus::Array</* TODO: u8 */ uint8_t, sizeof(T)>::with_uninitialized(  \
            unsafe_fn);                                                       \
    if (std::is_constant_evaluated()) {                                       \
      auto uval = primitive_value;                                            \
      for (auto i = size_t{0}; i < sizeof(T); ++i) {                          \
        if (sus::assertions::is_little_endian())                              \
          bytes.get_mut(i) = uval & 0xff;                                     \
        else                                                                  \
          bytes.get_mut(sizeof(T) - 1 - i) = uval & 0xff;                     \
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
  static constexpr T from_be_bytes(                                           \
      const ::sus::Array</*TODO: u8*/ uint8_t, Bytes>& bytes) noexcept {      \
    return from_be(from_ne_bytes(bytes));                                     \
  }                                                                           \
                                                                              \
  /** Create an integer value from its representation as a byte array in      \
   * little endian.                                                           \
   */                                                                         \
  static constexpr T from_le_bytes(                                           \
      const ::sus::Array</*TODO: u8*/ uint8_t, Bytes>& bytes) noexcept {      \
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
  static constexpr T from_ne_bytes(                                           \
      const ::sus::Array</*TODO: u8*/ uint8_t, Bytes>& bytes) noexcept {      \
    primitive_type val;                                                       \
    if (std::is_constant_evaluated()) {                                       \
      val = primitive_type{0};                                                \
      for (auto i = size_t{0}; i < sizeof(T); ++i) {                          \
        val |= bytes.get(i) << (sizeof(T) - 1 - i);                           \
      }                                                                       \
    } else {                                                                  \
      memcpy(&val, bytes.as_ptr(), sizeof(T));                                \
    }                                                                         \
    return val;                                                               \
  }                                                                           \
  static_assert(true)
