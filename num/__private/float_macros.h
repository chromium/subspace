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

#include <math.h>

#include <concepts>

#include "marker/unsafe.h"
#include "num/__private/float_ordering.h"
#include "num/__private/intrinsics.h"
#include "num/integer_concepts.h"
#include "num/signed_integer.h"
#include "num/unsigned_integer.h"

#define _sus__float_storage(PrimitiveT)                                       \
  /** The inner primitive value, in case it needs to be unwrapped from the    \
   * type. Avoid using this member except to convert when a consumer requires \
   * it.                                                                      \
   */                                                                         \
  PrimitiveT primitive_value = PrimitiveT{0.0};                               \
  static_assert(true)

#define _sus__float_constants(T, PrimitiveT)                                   \
  /** Smallest finite primitive value. */                                      \
  static constexpr auto MIN_PRIMITIVE = __private::min_value<PrimitiveT>();    \
  /** Largest finite primitive value. */                                       \
  static constexpr auto MAX_PRIMITIVE = __private::max_value<PrimitiveT>();    \
  /** Smallest finite `##T##`. */                                              \
  static constexpr T MIN() noexcept { return MIN_PRIMITIVE; }                  \
  /** Largest finite `##T##`. */                                               \
  static constexpr T MAX() noexcept { return MAX_PRIMITIVE; }                  \
  /** The radix or base of the internal representation of `##T##`. */          \
  static constexpr u32 RADIX() noexcept {                                      \
    return __private::radix<PrimitiveT>();                                     \
  }                                                                            \
  /** Approximate number of significant digits in base 2. */                   \
  static constexpr u32 MANTISSA_DIGITS() noexcept {                            \
    return __private::num_mantissa_digits<PrimitiveT>();                       \
  }                                                                            \
  /** Approximate number of significant digits in base 10. */                  \
  static constexpr u32 DIGITS() noexcept {                                     \
    return __private::num_digits<PrimitiveT>();                                \
  }                                                                            \
  /** Machine epsilon value for `##T##`.                                       \
   *                                                                           \
   * This is the difference between `1.0` and the next larger representable    \
   * number.                                                                   \
   */                                                                          \
  static constexpr T EPSILON() noexcept {                                      \
    return __private::epsilon<PrimitiveT>();                                   \
  }                                                                            \
  /** Smallest positive normal `##T##` value. */                               \
  static constexpr T MIN_POSITIVE() noexcept {                                 \
    return __private::min_positive_value<PrimitiveT>();                        \
  }                                                                            \
  /** One greater than the minimum possible normal power of 2 exponent. */     \
  static constexpr i32 MIN_EXP() noexcept {                                    \
    return __private::min_exp<PrimitiveT>();                                   \
  }                                                                            \
  /** Maximum possible power of 2 exponent.                                    \
   */                                                                          \
  static constexpr i32 MAX_EXP() noexcept {                                    \
    return __private::max_exp<PrimitiveT>();                                   \
  }                                                                            \
  /** Minimum possible normal power of 10 exponent. */                         \
  static constexpr i32 MIN_10_EXP() noexcept {                                 \
    return __private::min_10_exp<PrimitiveT>();                                \
  }                                                                            \
  /** Maximum possible power of 10 exponent. */                                \
  static constexpr i32 MAX_10_EXP() noexcept {                                 \
    return __private::max_10_exp<PrimitiveT>();                                \
  }                                                                            \
  /** Not a Number (NaN).                                                      \
   *                                                                           \
   * Note that IEEE-745 doesn't define just a single NaN value; a plethora of  \
   * bit patterns are considered to be NaN. Furthermore, the standard makes a  \
   * difference between a "signaling" and a "quiet" NaN, and allows inspecting \
   * its "payload" (the unspecified bits in the bit pattern). This constant    \
   * isn't guaranteed to equal to any specific NaN bitpattern, and the         \
   * stability of its representation over Subspace versions and target         \
   * platforms isn't guaranteed.                                               \
   */                                                                          \
  static constexpr T TODO_NAN() noexcept {                                     \
    return __private::nan<PrimitiveT>();                                       \
  }                                                                            \
  /** Infinity. */                                                             \
  static constexpr T TODO_INFINITY() noexcept {                                \
    return __private::infinity<PrimitiveT>();                                  \
  }                                                                            \
  /** Negative infinity. */                                                    \
  static constexpr T NEG_INFINITY() noexcept {                                 \
    return __private::negative_infinity<PrimitiveT>();                         \
  }                                                                            \
                                                                               \
  static_assert(true)

#define _sus__float_construct(T, PrimitiveT)                                   \
  /** Default constructor, which sets the value to 0.                          \
   *                                                                           \
   * The trivial copy and move constructors are implicitly declared, as is the \
   * trivial destructor.                                                       \
   */                                                                          \
  constexpr inline T() noexcept = default;                                     \
                                                                               \
  /** Construction from the underlying primitive type.                         \
   */                                                                          \
  template <std::same_as<PrimitiveT> P> /* Prevent implicit conversions. */    \
  constexpr inline T(P v) : primitive_value(v) {}                              \
                                                                               \
  /** Assignment from the underlying primitive type.                           \
   */                                                                          \
  template <std::same_as<PrimitiveT> P> /* Prevent implicit conversions. */    \
  constexpr inline void operator=(P v) noexcept {                              \
    primitive_value = v;                                                       \
  }                                                                            \
  static_assert(true)

#define _sus__float_comparison(T)                                              \
  /** sus::concepts::Eq<##T##> trait. */                                       \
  friend constexpr inline bool operator==(const T& l, const T& r) noexcept {   \
    return (l.primitive_value <=> r.primitive_value) == 0;                     \
  }                                                                            \
  /** sus::concepts::PartialOrd<##T##> trait. */                               \
  friend constexpr inline auto operator<=>(const T& l, const T& r) noexcept {  \
    return l.primitive_value <=> r.primitive_value;                            \
  }                                                                            \
  /** Return the ordering between self and other.                              \
   *                                                                           \
   * Unlike the standard partial comparison between floating point numbers,    \
   * this comparison always produces an ordering in accordance to the          \
   * totalOrder predicate as defined in the IEEE 754 (2008 revision) floating  \
   * point standard. The values are ordered in the following sequence:         \
   *                                                                           \
   * negative quiet NaN                                                        \
   * negative signaling NaN                                                    \
   * negative infinity                                                         \
   * negative numbers                                                          \
   * negative subnormal numbers                                                \
   * negative zero                                                             \
   * positive zero                                                             \
   * positive subnormal numbers                                                \
   * positive numbers                                                          \
   * positive infinity                                                         \
   * positive signaling NaN                                                    \
   * positive quiet NaN.                                                       \
   *                                                                           \
   * The ordering established by this function does not always agree with the  \
   * `PartialOrd` and `Eq` implementations of ##T##. For example, they         \
   * consider negative and positive zero equal, while total_cmp doesn't.       \
   *                                                                           \
   * The interpretation of the signaling NaN bit follows the definition in the \
   * IEEE 754 standard, which may not match the interpretation by some of the  \
   * older, non-conformant (e.g. MIPS) hardware implementations.               \
   */                                                                          \
  constexpr std::strong_ordering total_cmp(const T& rhs) const& noexcept {     \
    return __private::float_strong_ordering(primitive_value,                   \
                                            rhs.primitive_value);              \
  }                                                                            \
  static_assert(true)

#define _sus__float_unary_ops(T)     \
  /** sus::num::Neg<##T##> trait. */ \
  constexpr inline T operator-() const { return T(-primitive_value); }

#define _sus__float_binary_ops(T)                                        \
  /** sus::concepts::Add<##T##> trait. */                                \
  friend constexpr inline T operator+(const T& l, const T& r) noexcept { \
    return l.primitive_value + r.primitive_value;                        \
  }                                                                      \
  /** sus::concepts::Sub<##T##> trait. */                                \
  friend constexpr inline T operator-(const T& l, const T& r) noexcept { \
    return l.primitive_value - r.primitive_value;                        \
  }                                                                      \
  /** sus::concepts::Mul<##T##> trait. */                                \
  friend constexpr inline T operator*(const T& l, const T& r) noexcept { \
    return l.primitive_value * r.primitive_value;                        \
  }                                                                      \
  /** sus::concepts::Div<##T##> trait. */                                \
  friend constexpr inline T operator/(const T& l, const T& r) noexcept { \
    return l.primitive_value / r.primitive_value;                        \
  }                                                                      \
  /** sus::concepts::Rem<##T##> trait.                                   \
   *                                                                     \
   * The remainder from the division of two floats.                      \
   *                                                                     \
   * The remainder has the same sign as the dividend and is computed as: \
   * `l - (l / r).trunc() * r`.                                          \
   * */                                                                  \
  friend constexpr inline T operator%(const T& l, const T& r) noexcept { \
    const auto x = l.primitive_value;                                    \
    const auto y = r.primitive_value;                                    \
    return x - __private::truncate_float(x / y) * y;                     \
  }                                                                      \
  static_assert(true)

#define _sus__float_mutable_ops(T)                                       \
  /** sus::concepts::AddAssign<##T##> trait. */                          \
  constexpr inline void operator+=(T r)& noexcept {                      \
    primitive_value += r.primitive_value;                                \
  }                                                                      \
  /** sus::concepts::SubAssign<##T##> trait. */                          \
  constexpr inline void operator-=(T r)& noexcept {                      \
    primitive_value -= r.primitive_value;                                \
  }                                                                      \
  /** sus::concepts::MulAssign<##T##> trait. */                          \
  constexpr inline void operator*=(T r)& noexcept {                      \
    primitive_value *= r.primitive_value;                                \
  }                                                                      \
  /** sus::concepts::DivAssign<##T##> trait. */                          \
  constexpr inline void operator/=(T r)& noexcept {                      \
    primitive_value /= r.primitive_value;                                \
  }                                                                      \
  /** sus::concepts::RemAssign<##T##> trait.                             \
   *                                                                     \
   * Assigns the remainder from the division of two floats.              \
   *                                                                     \
   * The remainder has the same sign as the dividend and is computed as: \
   * `l - (l / r).trunc() * r`.                                          \
   */                                                                    \
  constexpr inline void operator%=(T r)& noexcept {                      \
    const auto x = primitive_value;                                      \
    const auto y = r.primitive_value;                                    \
    primitive_value = x - __private::truncate_float(x / y) * y;          \
  }                                                                      \
  static_assert(true)

#define _sus__float_abs(T, PrimitiveT)                      \
  /** Computes the absolute value of itself.                \
   */                                                       \
  inline T abs() const& noexcept {                          \
    return __private::into_float(                           \
        __private::into_unsigned_integer(primitive_value) & \
        ~__private::high_bit<PrimitiveT>());                \
  }                                                         \
  static_assert(true)

#define _sus__float_math(T, PrimitiveT)                                        \
  /** Computes the arccosine of a number. Return value is in radians in the    \
   * range [0, pi] or NaN if the number is outside the range [-1, 1].          \
   */                                                                          \
  inline T acos() const& noexcept {                                            \
    if (primitive_value < PrimitiveT{-1} || primitive_value > PrimitiveT{1})   \
        [[unlikely]]                                                           \
      return TODO_NAN();                                                       \
    /* MSVC acos(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::acos(primitive_value));                   \
  }                                                                            \
  /** Inverse hyperbolic cosine function, or NaN if the number is less than    \
   * -1.                                                                       \
   */                                                                          \
  inline T acosh() const& noexcept {                                           \
    if (primitive_value < PrimitiveT{-1}) [[unlikely]]                         \
      return TODO_NAN();                                                       \
    /* MSVC acosh(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::acosh(primitive_value));                  \
  }                                                                            \
  /** Computes the arcsine of a number. Return value is in radians in the      \
   * range [-pi/2, pi/2] or NaN if the number is outside the range [-1, 1].    \
   */                                                                          \
  inline T asin() const& noexcept {                                            \
    if (primitive_value < PrimitiveT{-1} || primitive_value > PrimitiveT{1})   \
        [[unlikely]]                                                           \
      return TODO_NAN();                                                       \
    /* MSVC asin(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::asin(primitive_value));                   \
  }                                                                            \
  /** Inverse hyperbolic sine function.                                        \
   */                                                                          \
  inline T asinh() const& noexcept {                                           \
    if (primitive_value < PrimitiveT{-1}) [[unlikely]]                         \
      return TODO_NAN();                                                       \
    /* MSVC asinh(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::asinh(primitive_value));                  \
  }                                                                            \
  /** Computes the arctangent of a number. Return value is in radians in the   \
   * range [-pi/2, pi/2];                                                      \
   */                                                                          \
  inline T atan() const& noexcept {                                            \
    /* MSVC atan(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::atan(primitive_value));                   \
  }                                                                            \
  /** Computes the four quadrant arctangent of self (y) and other (x) in       \
   * radians.                                                                  \
   *                                                                           \
   * - x = 0, y = 0: 0                                                         \
   * - x >= 0: arctan(y/x) -> [-pi/2, pi/2]                                    \
   * - y >= 0: arctan(y/x) + pi -> (pi/2, pi]                                  \
   * - y < 0: arctan(y/x) - pi -> (-pi, -pi/2)                                 \
   *                                                                           \
   * Returns NaN if both `self` and `other` are 0.                             \
   */                                                                          \
  inline T atan2(const T& other) const& noexcept {                             \
    if (primitive_value == PrimitiveT{0} &&                                    \
        other.primitive_value == PrimitiveT{0}) [[unlikely]]                   \
      return TODO_NAN();                                                       \
    /* MSVC atan2(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(                                            \
        ::atan2(primitive_value, other.primitive_value));                      \
  }                                                                            \
  /** Inverse hyperbolic tangent function.                                     \
   */                                                                          \
  inline T atanh() const& noexcept {                                           \
    /* MSVC atanh(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::atanh(primitive_value));                  \
  }                                                                            \
  /** Returns the cube root of a number.                                       \
   */                                                                          \
  inline T cbrt() const& noexcept {                                            \
    /* MSVC cbrt(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::cbrt(primitive_value));                   \
  }                                                                            \
  /** Returns the smallest integer greater than or equal to self.              \
   */                                                                          \
  inline T ceil() const& noexcept {                                            \
    /* MSVC ceil(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::ceil(primitive_value));                   \
  }                                                                            \
  /** Returns a number composed of the magnitude of self and the sign of sign. \
   *                                                                           \
   * Equal to self if the sign of self and sign are the same, otherwise equal  \
   * to -self. If self is a NaN, then a NaN with the sign bit of sign is       \
   * returned. Note, however, that conserving the sign bit on NaN across       \
   * arithmetical operations is not generally guaranteed.                      \
   */                                                                          \
  inline T copysign(const T& sign) const& noexcept {                           \
    /* MSVC copysign(float) is returning a double for some reason. */          \
    return static_cast<PrimitiveT>(                                            \
        ::copysign(primitive_value, sign.primitive_value));                    \
  }                                                                            \
  /** Computes the cosine of a number (in radians).                            \
   */                                                                          \
  inline T cos() const& noexcept {                                             \
    /* MSVC cos(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::cos(primitive_value));                    \
  }                                                                            \
  /** Hyperbolic cosine function.                                              \
   */                                                                          \
  inline T cosh() const& noexcept {                                            \
    /* MSVC cosh(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::cosh(primitive_value));                   \
  }                                                                            \
  /** Returns `e^(self)`, (the exponential function).                          \
   */                                                                          \
  inline T exp() const& noexcept {                                             \
    /* MSVC exp(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::exp(primitive_value));                    \
  }                                                                            \
  /** Returns `2^(self)`.                                                      \
   */                                                                          \
  inline T exp2() const& noexcept {                                            \
    /* MSVC exp2(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::exp2(primitive_value));                   \
  }                                                                            \
  /** Returns `e^(self) - 1` in a way that is accurate even if the number is   \
   * close to zero.                                                            \
   */                                                                          \
  inline T exp_m1() const& noexcept {                                          \
    /* MSVC expm1(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::expm1(primitive_value));                  \
  }                                                                            \
  /** Returns the largest integer less than or equal to self.                  \
   */                                                                          \
  inline T floor() const& noexcept {                                           \
    /* MSVC floor(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::floor(primitive_value));                  \
  }                                                                            \
  /** Calculates the length of the hypotenuse of a right-angle triangle given  \
   * legs of length x and y.                                                   \
   */                                                                          \
  inline T hypot(const T& other) const& noexcept {                             \
    /* MSVC hypot(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(                                            \
        ::hypot(primitive_value, other.primitive_value));                      \
  }                                                                            \
  /** Returns the natural logarithm of the number.                             \
   */                                                                          \
  inline T ln() const& noexcept {                                              \
    /* MSVC log(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::log(primitive_value));                    \
  }                                                                            \
  /** Returns ln(1+n) (natural logarithm) more accurately than if the          \
   * operations were performed separately.                                     \
   */                                                                          \
  inline T ln_1p() const& noexcept {                                           \
    /* MSVC log1p(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::log1p(primitive_value));                  \
  }                                                                            \
  /** Returns the base 10 logarithm of the number.                             \
   */                                                                          \
  inline T log10() const& noexcept {                                           \
    /* MSVC log10(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::log10(primitive_value));                  \
  }                                                                            \
  /** Returns the base 2 logarithm of the number.                              \
   */                                                                          \
  inline T log2() const& noexcept {                                            \
    /* MSVC log2(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::log2(primitive_value));                   \
  }                                                                            \
  /** Returns the maximum of the two numbers, ignoring NaN.                    \
   *                                                                           \
   * If one of the arguments is NaN, then the other argument is returned.      \
   */                                                                          \
  inline T max(const T& other) const& noexcept {                               \
    /* MSVC fmax(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(                                            \
        ::fmax(primitive_value, other.primitive_value));                       \
  }                                                                            \
  /** Returns the minimum of the two numbers, ignoring NaN.                    \
   *                                                                           \
   * If one of the arguments is NaN, then the other argument is returned.      \
   */                                                                          \
  inline T min(const T& other) const& noexcept {                               \
    /* MSVC fmin(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(                                            \
        ::fmin(primitive_value, other.primitive_value));                       \
  }                                                                            \
  /** Fused multiply-add. Computes `(self * a) + b` with only one rounding     \
   * error, yielding a more accurate result than an unfused multiply-add.      \
   *                                                                           \
   * Using mul_add may be more performant than an unfused multiply-add if the  \
   * target architecture has a dedicated fma CPU instruction. However, this is \
   * not always true, and will be heavily dependant on designing algorithms    \
   * with specific target hardware in mind.                                    \
   */                                                                          \
  inline T mul_add(const T& a, const T& b) const& noexcept {                   \
    /* MSVC fma(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(                                            \
        ::fma(primitive_value, a.primitive_value, b.primitive_value));         \
  }                                                                            \
  /** Raises a number to a floating point power.                               \
   */                                                                          \
  inline T powf(const T& n) const& noexcept {                                  \
    /* MSVC pow(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::pow(primitive_value, n.primitive_value)); \
  }                                                                            \
  /** Raises a number to an integer point power.                               \
   *                                                                           \
   * Using this function may be faster than using `powf()`. It might have a    \
   * different sequence of rounding operations than `powf()`, so the results   \
   * are not guaranteed to agree.                                              \
   */                                                                          \
  inline T powi(const i32& n) const& noexcept {                                \
    /* MSVC pow(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(                                            \
        ::pow(primitive_value, int{n.primitive_value}));                       \
  }                                                                            \
  /** Takes the reciprocal (inverse) of a number, `1/x`.                       \
   */                                                                          \
  inline T recip() const& noexcept { return PrimitiveT{1} / primitive_value; } \
  /** Returns the nearest integer to itself, rounding half-way cases away from \
   * `0.0`.                                                                    \
   */                                                                          \
  inline T round() const& noexcept {                                           \
    /* MSVC round(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::round(primitive_value));                  \
  }                                                                            \
  /** Returns a number that represents the sign of self.                       \
   *                                                                           \
   * - `1.0` if the number is positive, `+0.0` or `INFINITY`                   \
   * - `-1.0` if the number is negative, `-0.0` or `NEG_INFINITY`              \
   * - `NaN` if the number is `NaN`                                            \
   */                                                                          \
  inline T signum() const& noexcept {                                          \
    return __private::float_signum(primitive_value);                           \
  }                                                                            \
  /** Computes the sine of a number (in radians).                              \
   */                                                                          \
  inline T sin() const& noexcept {                                             \
    /* MSVC sin(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::sin(primitive_value));                    \
  }                                                                            \
  /** Hyperbolic sine function.                                                \
   */                                                                          \
  inline T sinh() const& noexcept {                                            \
    /* MSVC sinh(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::sinh(primitive_value));                   \
  }                                                                            \
  /** Returns the square root of a number.                                     \
   *                                                                           \
   * Returns NaN if self is a negative number other than `-0.0`.               \
   */                                                                          \
  inline T sqrt() const& noexcept {                                            \
    if (primitive_value < -PrimitiveT{0}) [[unlikely]]                         \
      return TODO_NAN();                                                       \
    /* MSVC sqrt(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::sqrt(primitive_value));                   \
  }                                                                            \
  /** Computes the tangent of a number (in radians).                           \
   */                                                                          \
  inline T tan() const& noexcept {                                             \
    /* MSVC tan(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::tan(primitive_value));                    \
  }                                                                            \
  /** Hyperbolic tangent function.                                             \
   */                                                                          \
  inline T tanh() const& noexcept {                                            \
    /* MSVC tanh(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::tanh(primitive_value));                   \
  }                                                                            \
  static_assert(true)

#define _sus__float_fract_trunc(T)                                          \
  /** Returns the fractional part of self.                                  \
   */                                                                       \
  inline T fract() const& noexcept {                                        \
    return primitive_value - __private::truncate_float(primitive_value);    \
  }                                                                         \
  /** Returns the integer part of self. This means that non-integer numbers \
   * are always truncated towards zero.                                     \
   */                                                                       \
  inline T truc() const& noexcept {                                         \
    return __private::truncate_float(primitive_value);                      \
  }                                                                         \
  static_assert(true)

#define _sus__float_convert_to(T, PrimitiveT)                                  \
  /** Converts radians to degrees.                                             \
   */                                                                          \
  inline T to_degrees() const& noexcept {                                      \
    /* Use a constant for better precision. */                                 \
    constexpr float PIS_IN_180 = 57.2957795130823208767981548141051703f;       \
    return primitive_value * PIS_IN_180;                                       \
  }                                                                            \
  /** Converts degrees to radians.                                             \
   */                                                                          \
  inline T to_radians() const& noexcept {                                      \
    return primitive_value * (consts::PI().primitive_value / PrimitiveT{180}); \
  }                                                                            \
  /** Rounds toward zero and converts to any primitive integer type, assuming  \
   * that the value is finite and fits in that type.                           \
   */                                                                          \
  template <Integer I>                                                         \
  inline I to_int_unchecked(::sus::marker::UnsafeFnMarker) const& noexcept {   \
    return static_cast<decltype(I::primitive_value)>(primitive_value);         \
  }                                                                            \
  static_assert(true)

#define _sus__float_bytes(T, UnsignedIntT)                                   \
  /** Raw transmutation from `##UnsignedIntT##`.                             \
   *                                                                         \
   * Note that this function is distinct from Into<##T##>, which attempts to \
   * preserve the numeric value, and not the bitwise value.                  \
   *                                                                         \
   * # Examples                                                              \
   * ```                                                                     \
   * auto v = f32::from_bits(0x41480000);                                    \
   * sus::check!(v, 12.5);                                                   \
   * ```                                                                     \
   */                                                                        \
  static T from_bits(const UnsignedIntT& v) noexcept {                       \
    return std::bit_cast<T>(v);                                              \
  }                                                                          \
  /** Raw transmutation to ##UnsignedT##.                                    \
   *                                                                         \
   * Note that this function is distinct from Into<##UnsignedIntT##>, which  \
   * attempts to preserve the numeric value, and not the bitwise value.      \
   */                                                                        \
  constexpr inline UnsignedIntT to_bits() const& noexcept {                  \
    return std::bit_cast<decltype(UnsignedIntT::primitive_value)>(           \
        primitive_value);                                                    \
  }                                                                          \
  static_assert(true)

// clamp
// classify, is_finite, is_infinite, is_nan, is_normal, is_sign_negative,
// is_sign_positive, is_subnormal
// div_euclid, rem_euclid
// from_be_bytes, from_le_bytes, from_ne_bytes
// to_be_bytes, to_le_bytes, to_ne_bytes
// log

#define _sus__float(T, PrimitiveT, UnsignedIntT) \
  _sus__float_storage(PrimitiveT);               \
  _sus__float_constants(T, PrimitiveT);          \
  _sus__float_construct(T, PrimitiveT);          \
  _sus__float_comparison(T);                     \
  _sus__float_unary_ops(T);                      \
  _sus__float_binary_ops(T);                     \
  _sus__float_mutable_ops(T);                    \
  _sus__float_abs(T, PrimitiveT);                \
  _sus__float_math(T, PrimitiveT);               \
  _sus__float_fract_trunc(T);                    \
  _sus__float_convert_to(T, PrimitiveT);         \
  _sus__float_bytes(T, UnsignedIntT);            \
  static_assert(true)
