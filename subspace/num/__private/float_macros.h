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
  static const T MIN;                                                          \
  /** Largest finite `##T##`. */                                               \
  static const T MAX;                                                          \
  /** The radix or base of the internal representation of `##T##`. */          \
  static constexpr u32 RADIX = __private::radix<PrimitiveT>();                 \
  /** Approximate number of significant digits in base 2. */                   \
  static constexpr u32 MANTISSA_DIGITS =                                       \
      __private::num_mantissa_digits<PrimitiveT>();                            \
  /** Approximate number of significant digits in base 10. */                  \
  static constexpr u32 DIGITS = __private::num_digits<PrimitiveT>();           \
  /** Machine epsilon value for `##T##`.                                       \
   *                                                                           \
   * This is the difference between `1.0` and the next larger representable    \
   * number.                                                                   \
   */                                                                          \
  static const T EPSILON;                                                      \
  /** Smallest positive normal `##T##` value. */                               \
  static const T MIN_POSITIVE;                                                 \
  /** One greater than the minimum possible normal power of 2 exponent. */     \
  static constexpr i32 MIN_EXP = __private::min_exp<PrimitiveT>();             \
  /** Maximum possible power of 2 exponent.                                    \
   */                                                                          \
  static constexpr i32 MAX_EXP = __private::max_exp<PrimitiveT>();             \
  /** Minimum possible normal power of 10 exponent. */                         \
  static constexpr i32 MIN_10_EXP = __private::min_10_exp<PrimitiveT>();       \
  /** Maximum possible power of 10 exponent. */                                \
  static constexpr i32 MAX_10_EXP = __private::max_10_exp<PrimitiveT>();       \
  /** Not a Number (NaN).                                                      \
   *                                                                           \
   * Note that IEEE-745 doesn't define just a single NaN value; a plethora of  \
   * bit patterns are considered to be NaN. Furthermore, the standard makes a  \
   * difference between a "signaling" and a "quiet" NaN, and allows inspecting \
   * its "payload" (the unspecified bits in the bit pattern). This constant    \
   * isn't guaranteed to equal to any specific NaN bitpattern, and the         \
   * stability of its representation over Subspace versions and target         \
   * platforms isn't guaranteed.                                               \
   *                                                                           \
   * This value is not constexpr because the value can differ in a constexpr   \
   * evaluation context from a runtime context, leading to bugs.               \
   */                                                                          \
  static const T NAN;                                                          \
  /** Infinity. */                                                             \
  static const T INFINITY;                                                     \
  /** Negative infinity. */                                                    \
  static const T NEG_INFINITY;                                                 \
  static_assert(true)

#define _sus__float_constants_out_of_line(T, PrimitiveT)                 \
  inline constexpr T T::MIN = T(T::MIN_PRIMITIVE);                       \
  inline constexpr T T::MAX = T(T::MAX_PRIMITIVE);                       \
  inline constexpr T T::EPSILON = T(__private::epsilon<PrimitiveT>());   \
  inline constexpr T T::MIN_POSITIVE =                                   \
      T(__private::min_positive_value<PrimitiveT>());                    \
  inline const T T::NAN = T(__private::nan<PrimitiveT>());               \
  inline constexpr T T::INFINITY = T(__private::infinity<PrimitiveT>()); \
  inline constexpr T T::NEG_INFINITY =                                   \
      T(__private::negative_infinity<PrimitiveT>());                     \
  static_assert(true)

#define _sus__float_construct(T, PrimitiveT)                                   \
  /** Default constructor, which sets the value to 0.                          \
   *                                                                           \
   * The trivial copy and move constructors are implicitly declared, as is the \
   * trivial destructor.                                                       \
   *                                                                           \
   * #[doc.overloads=ctor.default]                                             \
   */                                                                          \
  constexpr inline T() noexcept = default;                                     \
                                                                               \
  /** Construction from primitive types where no bits are lost.                \
   *                                                                           \
   * #[doc.overloads=ctor.from_primitive]                                      \
   */                                                                          \
  template <PrimitiveFloat P>                                                  \
    requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<PrimitiveT>())    \
  constexpr inline T(P v) noexcept : primitive_value(v) {}                     \
                                                                               \
  /** Assignment from primitive types where no bits are lost.                  \
   */                                                                          \
  template <PrimitiveFloat P>                                                  \
    requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<PrimitiveT>())    \
  constexpr inline T& operator=(P v) noexcept {                                \
    primitive_value = v;                                                       \
    return *this;                                                              \
  }                                                                            \
  static_assert(true)

#define _sus__float_to_primitive(T, PrimitiveT)                             \
  template <PrimitiveFloat U>                                               \
    requires(::sus::mem::size_of<U>() >= ::sus::mem::size_of<PrimitiveT>()) \
  sus_pure constexpr inline explicit operator U() const {                   \
    return primitive_value;                                                 \
  }                                                                         \
  static_assert(true)

#define _sus__float_comparison(T)                                              \
  /** sus::ops::Eq<##T##> trait.                                               \
   * #[doc.overloads=float##T##.eq] */                                         \
  friend sus_pure constexpr inline bool operator==(const T& l,                 \
                                                   const T& r) noexcept {      \
    return (l.primitive_value <=> r.primitive_value) == 0;                     \
  }                                                                            \
  /** sus::ops::PartialOrd<##T##> trait.                                       \
   * #[doc.overloads=float##T##.ord] */                                        \
  friend sus_pure constexpr inline auto operator<=>(const T& l,                \
                                                    const T& r) noexcept {     \
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
  sus_pure constexpr std::strong_ordering total_cmp(const T& rhs)              \
      const& noexcept {                                                        \
    return __private::float_strong_ordering(primitive_value,                   \
                                            rhs.primitive_value);              \
  }                                                                            \
  static_assert(true)

#define _sus__float_unary_ops(T)     \
  /** sus::num::Neg<##T##> trait. */ \
  sus_pure constexpr inline T operator-() const { return T(-primitive_value); }

#define _sus__float_binary_ops(T)                                        \
  /** sus::num::Add<##T##> trait.                                        \
   * #[doc.overloads=float##T##.+] */                                    \
  friend sus_pure constexpr inline T operator+(const T& l,               \
                                               const T& r) noexcept {    \
    return l.primitive_value + r.primitive_value;                        \
  }                                                                      \
  /** sus::num::Sub<##T##> trait.                                        \
   * #[doc.overloads=float##T##.-] */                                    \
  friend sus_pure constexpr inline T operator-(const T& l,               \
                                               const T& r) noexcept {    \
    return l.primitive_value - r.primitive_value;                        \
  }                                                                      \
  /** sus::num::Mul<##T##> trait.                                        \
   * #[doc.overloads=float##T##.*] */                                    \
  friend sus_pure constexpr inline T operator*(const T& l,               \
                                               const T& r) noexcept {    \
    return l.primitive_value * r.primitive_value;                        \
  }                                                                      \
  /** sus::num::Div<##T##> trait.                                        \
   * #[doc.overloads=float##T##./] */                                    \
  friend sus_pure constexpr inline T operator/(const T& l,               \
                                               const T& r) noexcept {    \
    return l.primitive_value / r.primitive_value;                        \
  }                                                                      \
  /** sus::num::Rem<##T##> trait.                                        \
   *                                                                     \
   * The remainder from the division of two floats.                      \
   *                                                                     \
   * The remainder has the same sign as the dividend and is computed as: \
   * `l - (l / r).trunc() * r`.                                          \
   *                                                                     \
   * #[doc.overloads=float##T##.%] */                                    \
  friend sus_pure constexpr inline T operator%(const T& l,               \
                                               const T& r) noexcept {    \
    const auto x = l.primitive_value;                                    \
    const auto y = r.primitive_value;                                    \
    return x - __private::truncate_float(x / y) * y;                     \
  }                                                                      \
  static_assert(true)

#define _sus__float_mutable_ops(T)                                       \
  /** sus::num::AddAssign<##T##> trait. */                               \
  constexpr inline void operator+=(T r)& noexcept {                      \
    primitive_value += r.primitive_value;                                \
  }                                                                      \
  /** sus::num::SubAssign<##T##> trait. */                               \
  constexpr inline void operator-=(T r)& noexcept {                      \
    primitive_value -= r.primitive_value;                                \
  }                                                                      \
  /** sus::num::MulAssign<##T##> trait. */                               \
  constexpr inline void operator*=(T r)& noexcept {                      \
    primitive_value *= r.primitive_value;                                \
  }                                                                      \
  /** sus::num::DivAssign<##T##> trait. */                               \
  constexpr inline void operator/=(T r)& noexcept {                      \
    primitive_value /= r.primitive_value;                                \
  }                                                                      \
  /** sus::num::RemAssign<##T##> trait.                                  \
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
  sus_pure inline T abs() const& noexcept {                 \
    return __private::into_float(                           \
        __private::into_unsigned_integer(primitive_value) & \
        ~__private::high_bit<PrimitiveT>());                \
  }                                                         \
  static_assert(true)

#define _sus__float_math(T, PrimitiveT)                                        \
  /** Computes the arccosine of a number. Return value is in radians in the    \
   * range [0, pi] or NaN if the number is outside the range [-1, 1].          \
   */                                                                          \
  sus_pure inline T acos() const& noexcept {                                   \
    if (primitive_value < PrimitiveT{-1} || primitive_value > PrimitiveT{1})   \
        [[unlikely]]                                                           \
      return NAN;                                                              \
    /* MSVC acos(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::acos(primitive_value));                   \
  }                                                                            \
  /** Inverse hyperbolic cosine function, or NaN if the number is less than    \
   * -1.                                                                       \
   */                                                                          \
  sus_pure inline T acosh() const& noexcept {                                  \
    if (primitive_value < PrimitiveT{-1}) [[unlikely]]                         \
      return NAN;                                                              \
    /* MSVC acosh(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::acosh(primitive_value));                  \
  }                                                                            \
  /** Computes the arcsine of a number. Return value is in radians in the      \
   * range [-pi/2, pi/2] or NaN if the number is outside the range [-1, 1].    \
   */                                                                          \
  sus_pure inline T asin() const& noexcept {                                   \
    if (primitive_value < PrimitiveT{-1} || primitive_value > PrimitiveT{1})   \
        [[unlikely]]                                                           \
      return NAN;                                                              \
    /* MSVC asin(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::asin(primitive_value));                   \
  }                                                                            \
  /** Inverse hyperbolic sine function.                                        \
   */                                                                          \
  sus_pure inline T asinh() const& noexcept {                                  \
    if (primitive_value < PrimitiveT{-1}) [[unlikely]]                         \
      return NAN;                                                              \
    /* MSVC asinh(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::asinh(primitive_value));                  \
  }                                                                            \
  /** Computes the arctangent of a number. Return value is in radians in the   \
   * range [-pi/2, pi/2];                                                      \
   */                                                                          \
  sus_pure inline T atan() const& noexcept {                                   \
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
  sus_pure inline T atan2(const T& other) const& noexcept {                    \
    /* MSVC atan2(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(                                            \
        ::atan2(primitive_value, other.primitive_value));                      \
  }                                                                            \
  /** Inverse hyperbolic tangent function.                                     \
   */                                                                          \
  sus_pure inline T atanh() const& noexcept {                                  \
    /* MSVC atanh(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::atanh(primitive_value));                  \
  }                                                                            \
  /** Returns the cube root of a number.                                       \
   */                                                                          \
  sus_pure inline T cbrt() const& noexcept {                                   \
    /* MSVC cbrt(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::cbrt(primitive_value));                   \
  }                                                                            \
  /** Returns the smallest integer greater than or equal to self.              \
   */                                                                          \
  sus_pure inline T ceil() const& noexcept {                                   \
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
  sus_pure inline T copysign(const T& sign) const& noexcept {                  \
    /* MSVC copysign(float) is returning a double for some reason. */          \
    return static_cast<PrimitiveT>(                                            \
        ::copysign(primitive_value, sign.primitive_value));                    \
  }                                                                            \
  /** Computes the cosine of a number (in radians).                            \
   */                                                                          \
  sus_pure inline T cos() const& noexcept {                                    \
    /* MSVC cos(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::cos(primitive_value));                    \
  }                                                                            \
  /** Hyperbolic cosine function.                                              \
   */                                                                          \
  sus_pure inline T cosh() const& noexcept {                                   \
    /* MSVC cosh(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::cosh(primitive_value));                   \
  }                                                                            \
  /** Returns `e^(self)`, (the exponential function).                          \
   */                                                                          \
  sus_pure inline T exp() const& noexcept {                                    \
    /* MSVC exp(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::exp(primitive_value));                    \
  }                                                                            \
  /** Returns `2^(self)`.                                                      \
   */                                                                          \
  sus_pure inline T exp2() const& noexcept {                                   \
    /* MSVC exp2(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::exp2(primitive_value));                   \
  }                                                                            \
  /** Returns `e^(self) - 1` in a way that is accurate even if the number is   \
   * close to zero.                                                            \
   */                                                                          \
  sus_pure inline T exp_m1() const& noexcept {                                 \
    /* MSVC expm1(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::expm1(primitive_value));                  \
  }                                                                            \
  /** Returns the largest integer less than or equal to self.                  \
   */                                                                          \
  sus_pure inline T floor() const& noexcept {                                  \
    /* MSVC floor(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::floor(primitive_value));                  \
  }                                                                            \
  /** Calculates the length of the hypotenuse of a right-angle triangle given  \
   * legs of length x and y.                                                   \
   */                                                                          \
  sus_pure inline T hypot(const T& other) const& noexcept {                    \
    /* MSVC hypot(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(                                            \
        ::hypot(primitive_value, other.primitive_value));                      \
  }                                                                            \
  /** Returns the natural logarithm of the number.                             \
   */                                                                          \
  sus_pure inline T ln() const& noexcept {                                     \
    /* MSVC log(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::log(primitive_value));                    \
  }                                                                            \
  /** Returns ln(1+n) (natural logarithm) more accurately than if the          \
   * operations were performed separately.                                     \
   */                                                                          \
  sus_pure inline T ln_1p() const& noexcept {                                  \
    /* MSVC log1p(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::log1p(primitive_value));                  \
  }                                                                            \
  /** Returns the base 10 logarithm of the number.                             \
   */                                                                          \
  sus_pure inline T log10() const& noexcept {                                  \
    /* MSVC log10(float) is returning a double for some reason. */             \
    return static_cast<PrimitiveT>(::log10(primitive_value));                  \
  }                                                                            \
  /** Returns the base 2 logarithm of the number.                              \
   */                                                                          \
  sus_pure inline T log2() const& noexcept {                                   \
    /* MSVC log2(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::log2(primitive_value));                   \
  }                                                                            \
  /** Returns the logarithm of the number with respect to an arbitrary base.   \
   *                                                                           \
   * The result might not be correctly rounded owing to implementation         \
   * details; self.log2() can produce more accurate results for base 2, and    \
   * self.log10() can produce more accurate results for base 10.               \
   */                                                                          \
  sus_pure inline T log(const T& base) const& noexcept {                       \
    return ln() / base.ln();                                                   \
  }                                                                            \
  /** Returns the maximum of the two numbers, ignoring NaN.                    \
   *                                                                           \
   * If one of the arguments is NaN, then the other argument is returned.      \
   */                                                                          \
  sus_pure inline T max(const T& other) const& noexcept {                      \
    /* MSVC fmax(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(                                            \
        ::fmax(primitive_value, other.primitive_value));                       \
  }                                                                            \
  /** Returns the minimum of the two numbers, ignoring NaN.                    \
   *                                                                           \
   * If one of the arguments is NaN, then the other argument is returned.      \
   */                                                                          \
  sus_pure inline T min(const T& other) const& noexcept {                      \
    /* MSVC fmin(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(                                            \
        ::fmin(primitive_value, other.primitive_value));                       \
  }                                                                            \
  /** Fused multiply-add. Computes `(self * a) + b` with only one rounding     \
   * error, yielding a more accurate result than an unfused multiply-add.      \
   *                                                                           \
   * Using mul_add may be more performant than an unfused multiply-add if the  \
   * target architecture has a dedicated fma CPU instruction. However, this is \
   * not always true, and will be heavily dependent on designing algorithms    \
   * with specific target hardware in mind.                                    \
   */                                                                          \
  sus_pure inline T mul_add(const T& a, const T& b) const& noexcept {          \
    /* MSVC fma(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(                                            \
        ::fma(primitive_value, a.primitive_value, b.primitive_value));         \
  }                                                                            \
  /** Raises a number to a floating point power.                               \
   */                                                                          \
  sus_pure inline T powf(const T& n) const& noexcept {                         \
    /* MSVC pow(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::pow(primitive_value, n.primitive_value)); \
  }                                                                            \
  /** Raises a number to an integer point power.                               \
   *                                                                           \
   * Using this function may be faster than using `powf()`. It might have a    \
   * different sequence of rounding operations than `powf()`, so the results   \
   * are not guaranteed to agree.                                              \
   */                                                                          \
  sus_pure inline T powi(const i32& n) const& noexcept {                       \
    /* MSVC pow(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(                                            \
        ::pow(primitive_value, int{n.primitive_value}));                       \
  }                                                                            \
  /** Takes the reciprocal (inverse) of a number, `1/x`.                       \
   */                                                                          \
  sus_pure inline T recip() const& noexcept {                                  \
    return PrimitiveT{1} / primitive_value;                                    \
  }                                                                            \
  /** Returns the nearest integer to itself, rounding half-way cases away from \
   * `0.0`.                                                                    \
   */                                                                          \
  sus_pure inline T round() const& noexcept {                                  \
    return __private::float_round(primitive_value);                            \
  }                                                                            \
  /** Returns a number that represents the sign of self.                       \
   *                                                                           \
   * - `1.0` if the number is positive, `+0.0` or `INFINITY`                   \
   * - `-1.0` if the number is negative, `-0.0` or `NEG_INFINITY`              \
   * - `NaN` if the number is `NaN`                                            \
   */                                                                          \
  sus_pure inline T signum() const& noexcept {                                 \
    return __private::float_signum(primitive_value);                           \
  }                                                                            \
  /** Computes the sine of a number (in radians).                              \
   */                                                                          \
  sus_pure inline T sin() const& noexcept {                                    \
    /* MSVC sin(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::sin(primitive_value));                    \
  }                                                                            \
  /** Hyperbolic sine function.                                                \
   */                                                                          \
  sus_pure inline T sinh() const& noexcept {                                   \
    /* MSVC sinh(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::sinh(primitive_value));                   \
  }                                                                            \
  /** Returns the square root of a number.                                     \
   *                                                                           \
   * Returns NaN if self is a negative number other than `-0.0`.               \
   */                                                                          \
  sus_pure inline T sqrt() const& noexcept {                                   \
    if (primitive_value < -PrimitiveT{0}) [[unlikely]]                         \
      return NAN;                                                              \
    /* MSVC sqrt(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::sqrt(primitive_value));                   \
  }                                                                            \
  /** Computes the tangent of a number (in radians).                           \
   */                                                                          \
  sus_pure inline T tan() const& noexcept {                                    \
    /* MSVC tan(float) is returning a double for some reason. */               \
    return static_cast<PrimitiveT>(::tan(primitive_value));                    \
  }                                                                            \
  /** Hyperbolic tangent function.                                             \
   */                                                                          \
  sus_pure inline T tanh() const& noexcept {                                   \
    /* MSVC tanh(float) is returning a double for some reason. */              \
    return static_cast<PrimitiveT>(::tanh(primitive_value));                   \
  }                                                                            \
  static_assert(true)

#define _sus__float_fract_trunc(T)                                          \
  /** Returns the fractional part of self.                                  \
   */                                                                       \
  sus_pure inline T fract() const& noexcept {                               \
    return primitive_value - __private::truncate_float(primitive_value);    \
  }                                                                         \
  /** Returns the integer part of self. This means that non-integer numbers \
   * are always truncated towards zero.                                     \
   */                                                                       \
  sus_pure inline T trunc() const& noexcept {                               \
    return __private::truncate_float(primitive_value);                      \
  }                                                                         \
  static_assert(true)

#define _sus__float_convert_to(T, PrimitiveT)                                 \
  /** Converts radians to degrees.                                            \
   */                                                                         \
  sus_pure inline T to_degrees() const& noexcept {                            \
    /* Use a constant for better precision. */                                \
    constexpr auto PIS_IN_180 =                                               \
        PrimitiveT{57.2957795130823208767981548141051703};                    \
    return primitive_value * PIS_IN_180;                                      \
  }                                                                           \
  /** Converts degrees to radians.                                            \
   */                                                                         \
  sus_pure inline T to_radians() const& noexcept {                            \
    return primitive_value * (consts::PI.primitive_value / PrimitiveT{180});  \
  }                                                                           \
  /** Rounds toward zero and converts to any primitive integer type, assuming \
   * that the value is finite and fits in that type.                          \
   */                                                                         \
  template <Integer I>                                                        \
  sus_pure constexpr inline I to_int_unchecked(::sus::marker::UnsafeFnMarker) \
      const& noexcept {                                                       \
    return static_cast<decltype(I::primitive_value)>(primitive_value);        \
  }                                                                           \
  static_assert(true)

#define _sus__float_bytes(T, UnsignedIntT)                                     \
  /** Raw transmutation from `##UnsignedIntT##`.                               \
   *                                                                           \
   * Note that this function is distinct from Into<##T##>, which attempts to   \
   * preserve the numeric value, and not the bitwise value.                    \
   *                                                                           \
   * # Examples                                                                \
   * ```                                                                       \
   * auto v = f32::from_bits(0x41480000);                                      \
   * sus::check!(v, 12.5);                                                     \
   * ```                                                                       \
   *                                                                           \
   * This function is not constexpr, as converting a NaN does not preserve the \
   * exact bits in a constexpr context.                                        \
   */                                                                          \
  sus_pure static T from_bits(const UnsignedIntT& v) noexcept {                \
    return std::bit_cast<T>(v);                                                \
  }                                                                            \
  /** Raw transmutation to ##UnsignedT##.                                      \
   *                                                                           \
   * Note that this function is distinct from Into<##UnsignedIntT##>, which    \
   * attempts to preserve the numeric value, and not the bitwise value.        \
   */                                                                          \
  sus_pure constexpr inline UnsignedIntT to_bits() const& noexcept {           \
    return std::bit_cast<decltype(UnsignedIntT::primitive_value)>(             \
        primitive_value);                                                      \
  }                                                                            \
  static_assert(true)

#define _sus__float_category(T)                                                \
  /** Returns the floating point category of the number.                       \
   *                                                                           \
   * If only one property is going to be tested, it is generally faster to use \
   * the specific predicate instead.                                           \
   */                                                                          \
  sus_pure constexpr inline FpCategory classify() const& noexcept {            \
    return __private::float_category(primitive_value);                         \
  }                                                                            \
  /** Returns true if this number is neither infinite nor NaN.                 \
   */                                                                          \
  sus_pure constexpr inline bool is_finite() const& noexcept {                 \
    return !__private::float_is_inf_or_nan(primitive_value);                   \
  }                                                                            \
  /** Returns true if this value is positive infinity or negative infinity,    \
   * and false otherwise.                                                      \
   */                                                                          \
  sus_pure constexpr inline bool is_infinite() const& noexcept {               \
    return __private::float_is_inf(primitive_value);                           \
  }                                                                            \
  /** Returns true if this value is NaN.                                       \
   */                                                                          \
  sus_pure constexpr inline bool is_nan() const& noexcept {                    \
    return __private::float_is_nan(primitive_value);                           \
  }                                                                            \
  /** Returns true if the number is neither zero, infinite, subnormal, or NaN. \
   */                                                                          \
  sus_pure constexpr inline bool is_normal() const& noexcept {                 \
    return __private::float_is_normal(primitive_value);                        \
  }                                                                            \
  /** Returns true if self has a negative sign, including -0.0, NaNs with      \
   * negative sign bit and negative infinity.                                  \
   *                                                                           \
   * Note that IEEE-745 doesn't assign any meaning to the sign bit in case of  \
   * a NaN                                                                     \
   */                                                                          \
  sus_pure constexpr inline bool is_sign_negative() const& noexcept {          \
    return __private::float_signbit(primitive_value);                          \
  }                                                                            \
  /** Returns true if self has a positive sign, including +0.0, NaNs with      \
   * positive sign bit and positive infinity.                                  \
   *                                                                           \
   * Note that IEEE-745 doesn't assign any meaning to the sign bit in case of  \
   * a NaN.                                                                    \
   */                                                                          \
  sus_pure constexpr inline bool is_sign_positive() const& noexcept {          \
    return !__private::float_signbit(primitive_value);                         \
  }                                                                            \
  /** Returns true if the number is subnormal.                                 \
   */                                                                          \
  sus_pure constexpr inline bool is_subnormal() const& noexcept {              \
    return !__private::float_is_zero(primitive_value) &&                       \
           __private::float_nonzero_is_subnormal(primitive_value);             \
  }                                                                            \
  static_assert(true)

#define _sus__float_clamp(T)                                                   \
  /** Restrict a value to a certain interval unless it is NaN.                 \
   *                                                                           \
   * Returns max if self is greater than max, and min if self is less than     \
   * min. Otherwise this returns self.                                         \
   *                                                                           \
   * Note that this function returns NaN if the initial value was NaN as well. \
   *                                                                           \
   * # Panics                                                                  \
   * Panics if min > max, min is NaN, or max is NaN.                           \
   */                                                                          \
  sus_pure constexpr inline T clamp(const T& min, const T& max)                \
      const& noexcept {                                                        \
    check(!min.is_nan() && !max.is_nan() &&                                    \
          min.primitive_value <= max.primitive_value);                         \
    /* SAFETY: We have verified that the min and max are not NaN and that      \
     * `min <= max`. */                                                        \
    return __private::float_clamp(::sus::marker::unsafe_fn, primitive_value,   \
                                  min.primitive_value, max.primitive_value);   \
  }

#define _sus__float_euclid(T, PrimitiveT)                                      \
  /** Calculates Euclidean division, the matching method for `rem_euclid`.     \
   *                                                                           \
   * This computes the integer `n` such that `self = n * rhs +                 \
   * self.rem_euclid(rhs)`. In other words, the result is `self / rhs` rounded \
   * to the integer `n` such that `self >= n * rhs`.                           \
   */                                                                          \
  sus_pure T div_euclid(const T& rhs) const& noexcept {                        \
    const auto q = (*this / rhs).trunc();                                      \
    if (*this % rhs < PrimitiveT{0}) {                                         \
      if (rhs > T{PrimitiveT{0}})                                              \
        return q - T{PrimitiveT{1}};                                           \
      else                                                                     \
        return q + T{PrimitiveT{1}};                                           \
    }                                                                          \
    return q;                                                                  \
  }                                                                            \
  /** Calculates the least nonnegative remainder of `self (mod rhs)`.          \
   *                                                                           \
   * In particular, the return value `r` satisfies `0.0 <= r < rhs.abs()` in   \
   * most cases. However, due to a floating point round-off error it can       \
   * result in `r == rhs.abs()`, violating the mathematical definition, if     \
   * `self` is much smaller than `rhs.abs()` in magnitude and `self < 0.0`.    \
   * This result is not an element of the function's codomain, but it is the   \
   * closest floating point number in the real numbers and thus fulfills the   \
   * property `self == self.div_euclid(rhs) * rhs + self.rem_euclid(rhs)`      \
   * approximately.                                                            \
   */                                                                          \
  sus_pure T rem_euclid(const T& rhs) const& noexcept {                        \
    const auto r = *this % rhs;                                                \
    if (r < T{PrimitiveT{0}})                                                  \
      return r + rhs.abs();                                                    \
    else                                                                       \
      return r;                                                                \
  }

#define _sus__float_endian(T, Bytes, UnsignedIntT)                             \
  /** Return the memory representation of this floating point number as a byte \
   * array in big-endian (network) byte order.                                 \
   */                                                                          \
  sus_pure constexpr ::sus::containers::Array<u8, Bytes> to_be_bytes()         \
      const& noexcept;                                                         \
  /** Return the memory representation of this floating point number as a byte \
   * array in little-endian byte order.                                        \
   */                                                                          \
  sus_pure constexpr ::sus::containers::Array<u8, Bytes> to_le_bytes()         \
      const& noexcept;                                                         \
  /** Return the memory representation of this floating point number as a byte \
   * array in native byte order.                                               \
   *                                                                           \
   * As the target platform's native endianness is used, portable code should  \
   * use `to_be_bytes()` or `to_le_bytes()`, as appropriate, instead.          \
   */                                                                          \
  sus_pure constexpr ::sus::containers::Array<u8, Bytes> to_ne_bytes()         \
      const& noexcept;                                                         \
  /** Create a floating point value from its representation as a byte array in \
   * big endian.                                                               \
   *                                                                           \
   * See `##T##::from_bits()` for why this function is not constexpr.          \
   */                                                                          \
  sus_pure static T from_be_bytes(                                             \
      const ::sus::containers::Array<u8, Bytes>& bytes) noexcept;              \
  /** Create a floating point value from its representation as a byte array in \
   * big endian.                                                               \
   *                                                                           \
   *  See `##T##::from_bits()` for why this function is not constexpr.         \
   */                                                                          \
  sus_pure static T from_le_bytes(                                             \
      const ::sus::containers::Array<u8, Bytes>& bytes) noexcept;              \
  /** Create a floating point value from its representation as a byte array in \
   * native endian.                                                            \
   *                                                                           \
   * As the target platform's native endianness is used, portable code likely  \
   * wants to use `from_be_bytes()` or `from_le_bytes()`, as appropriate       \
   * instead.                                                                  \
   *                                                                           \
   *  See `##T##::from_bits()` for why this function is not constexpr.         \
   */                                                                          \
  sus_pure static T from_ne_bytes(                                             \
      const ::sus::containers::Array<u8, Bytes>& bytes) noexcept;              \
  static_assert(true)

#define _sus__float_endian_out_of_line(T, Bytes, UnsignedIntT)            \
  sus_pure constexpr ::sus::containers::Array<u8, Bytes> T::to_be_bytes() \
      const& noexcept {                                                   \
    return to_bits().to_be_bytes();                                       \
  }                                                                       \
  sus_pure constexpr ::sus::containers::Array<u8, Bytes> T::to_le_bytes() \
      const& noexcept {                                                   \
    return to_bits().to_le_bytes();                                       \
  }                                                                       \
  sus_pure constexpr ::sus::containers::Array<u8, Bytes> T::to_ne_bytes() \
      const& noexcept {                                                   \
    return to_bits().to_ne_bytes();                                       \
  }                                                                       \
  sus_pure inline T T::from_be_bytes(                                     \
      const ::sus::containers::Array<u8, Bytes>& bytes) noexcept {        \
    return T::from_bits(UnsignedIntT::from_be_bytes(bytes));              \
  }                                                                       \
  sus_pure inline T T::from_le_bytes(                                     \
      const ::sus::containers::Array<u8, Bytes>& bytes) noexcept {        \
    return T::from_bits(UnsignedIntT::from_le_bytes(bytes));              \
  }                                                                       \
  sus_pure inline T T::from_ne_bytes(                                     \
      const ::sus::containers::Array<u8, Bytes>& bytes) noexcept {        \
    return T::from_bits(UnsignedIntT::from_ne_bytes(bytes));              \
  }                                                                       \
  static_assert(true)

#define _sus__float(T, PrimitiveT, UnsignedIntT) \
  _sus__float_storage(PrimitiveT);               \
  _sus__float_constants(T, PrimitiveT);          \
  _sus__float_construct(T, PrimitiveT);          \
  _sus__float_to_primitive(T, PrimitiveT);       \
  _sus__float_comparison(T);                     \
  _sus__float_unary_ops(T);                      \
  _sus__float_binary_ops(T);                     \
  _sus__float_mutable_ops(T);                    \
  _sus__float_abs(T, PrimitiveT);                \
  _sus__float_math(T, PrimitiveT);               \
  _sus__float_fract_trunc(T);                    \
  _sus__float_convert_to(T, PrimitiveT);         \
  _sus__float_bytes(T, UnsignedIntT);            \
  _sus__float_category(T);                       \
  _sus__float_clamp(T);                          \
  _sus__float_euclid(T, PrimitiveT);             \
  _sus__float_endian(T, ::sus::mem::size_of<PrimitiveT>(), UnsignedIntT)

#define _sus__float_out_of_line(T, PrimitiveT, UnsignedIntT)           \
  _sus__float_constants_out_of_line(T, PrimitiveT);                    \
  _sus__float_endian_out_of_line(T, ::sus::mem::size_of<PrimitiveT>(), \
                                 UnsignedIntT)

#define _sus__float_hash_equal_to(T)                                      \
  template <>                                                             \
  struct hash<T> {                                                        \
    sus_pure auto operator()(const T& u) const {                          \
      return std::hash<decltype(u.primitive_value)>()(u.primitive_value); \
    }                                                                     \
  };                                                                      \
  template <>                                                             \
  struct equal_to<T> {                                                    \
    sus_pure constexpr auto operator()(const T& l, const T& r) const {    \
      return l == r;                                                      \
    }                                                                     \
  };                                                                      \
  static_assert(true)
