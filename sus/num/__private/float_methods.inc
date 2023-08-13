// Copyright 2023 Google LLC
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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"

///////////////////////////////////////////////////////////////////////////
//
// Declares (and defines) methods of floating point types
//
// TO USE THIS INC FILE:
//
// Include it into the body of your class.
//
// Define `_self` to the name of the floating point type.
// Define `_primitive` to the primitive it holds inside.
// Define `_unsigned` to the unsigned integer type of the same size.
///////////////////////////////////////////////////////////////////////////

/// The inner primitive value. Prefer to cast to the desired primitive type,
/// such as with `float{n}` for a numeric value `n`.
_primitive primitive_value = _primitive{0.0};

/// Smallest finite primitive value.
static const _primitive MIN_PRIMITIVE;
/// Largest finite primitive value.
static const _primitive MAX_PRIMITIVE;
/// Smallest finite `@doc.self`.
static const _self MIN;
/// Largest finite `@doc.self`.
static const _self MAX;
/// The radix or base of the internal representation of `@doc.self`.
static const u32 RADIX;
/// Approximate number of significant digits in base 2.
static const u32 MANTISSA_DIGITS;
/// Approximate number of significant digits in base 10.
static const u32 DIGITS;
/// Machine epsilon value for `@doc.self`.
///
/// This is the difference between `1.0` and the next larger representable
/// number.
static const _self EPSILON;
/// Smallest positive normal `@doc.self` value.
static const _self MIN_POSITIVE;
/// One greater than the minimum possible normal power of 2 exponent.
static const i32 MIN_EXP;
/// Maximum possible power of 2 exponent.
static const i32 MAX_EXP;
/// Minimum possible normal power of 10 exponent.
static const i32 MIN_10_EXP;
/// Maximum possible power of 10 exponent.
static const i32 MAX_10_EXP;
/// Not a Number (NaN).
///
/// Note that IEEE-745 doesn't define just a single NaN value; a plethora of
/// bit patterns are considered to be NaN. Furthermore, the standard makes a
/// difference between a "signaling" and a "quiet" NaN, and allows inspecting
/// its "payload" (the unspecified bits in the bit pattern). This constant
/// isn't guaranteed to equal to any specific NaN bitpattern, and the
/// stability of its representation over Subspace versions and target
/// platforms isn't guaranteed.
///
/// This value is not constexpr because the value can differ in a constexpr
/// evaluation context from a runtime context, leading to bugs.
static const _self NAN;
/// Infinity.
static const _self INFINITY;
/// Negative infinity.
static const _self NEG_INFINITY;

struct consts {
  /// Euler's number (e)
  static const _self E;
  /// 1/π
  static const _self FRAC_1_PI;
  /// 1/sqrt(2)
  static const _self FRAC_1_SQRT_2;
  /// 2/π
  static const _self FRAC_2_PI;
  /// 2/sqrt(π)
  static const _self FRAC_2_SQRT_PI;
  /// π/2
  static const _self FRAC_PI_2;
  /// π/3
  static const _self FRAC_PI_3;
  /// π/4
  static const _self FRAC_PI_4;
  /// π/6
  static const _self FRAC_PI_6;
  /// π/8
  static const _self FRAC_PI_8;
  /// ln(2)
  static const _self LN_2;
  /// ln(10)
  static const _self LN_10;
  /// log_2(10)
  static const _self LOG2_10;
  /// log_2(e)
  static const _self LOG2_E;
  /// log_10(2)
  static const _self LOG10_2;
  /// log_10(e)
  static const _self LOG10_E;
  /// Archimedes' constant (π)
  static const _self PI;
  /// sqrt(2)
  static const _self SQRT_2;
  /// The full circle constant (τ)
  static const _self TAU;
};

/// Default constructor, which sets the value to 0.
///
/// The trivial copy and move constructors are implicitly declared, as is the
/// trivial destructor.
///
/// #[doc.overloads=ctor.default]
constexpr inline _self() noexcept = default;

/// Construction from floating point types where no bits are lost.
///
/// #[doc.overloads=ctor.from_float]
template <Float F>
  requires(::sus::mem::size_of<F>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(F v) noexcept : primitive_value(v.primitive_value) {}

/// Construction from primitive types where no bits are lost.
///
/// #[doc.overloads=ctor.from_primitive]
template <PrimitiveFloat P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from floating point types where no bits are lost.
///
/// #[doc.overloads=from.float]
template <Float F>
  requires(::sus::mem::size_of<F>() <= ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(F f) noexcept {
  return _self(f);
}

/// Construction from primitive floating point types where no bits are lost.
///
/// #[doc.overloads=from.primitivefloat]
template <PrimitiveFloat F>
  requires(::sus::mem::size_of<F>() <= ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(F f) noexcept {
  return _self(f);
}

/// Assignment from floating point types where no bits are lost.
///
/// #[doc.overloads=assign.from_float]
template <Float F>
  requires(::sus::mem::size_of<F>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(F v) noexcept {
  primitive_value = v.primitive_value;
  return *this;
}

/// Assignment from primitive types where no bits are lost.
///
/// #[doc.overloads=assign.from_primitive]
template <PrimitiveFloat P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(P v) noexcept {
  primitive_value = v;
  return *this;
}

/// Constructs a @doc.self from an `Iterator` by computing the sum of all
/// elements in the iterator.
///
/// This method should rarely be called directly, as it is used to satisfy the
/// `sus::iter::Sum` concept so that `Iterator::sum()` can be called for
/// iterators over @doc.self.
static constexpr _self from_sum(
    ::sus::iter::Iterator<_self> auto&& it) noexcept {
  auto p = _self(_primitive{0u});
  for (_self i : ::sus::move(it)) p += i;
  return p;
}

/// Constructs a @doc.self from an `Iterator` by computing the product of all
/// elements in the iterator.
///
/// This method should rarely be called directly, as it is used to satisfy the
/// `sus::iter::Product` concept so that `Iterator::product()` can be called for
/// iterators over @doc.self.
static constexpr _self from_product(
    ::sus::iter::Iterator<_self> auto&& it) noexcept {
  auto p = _self(_primitive{1u});
  for (_self i : ::sus::move(it)) p *= i;
  return p;
}

/// Explicit conversion from the numeric type to the inner primitive type.
///
/// Typically this should be used in a curly-brace conversion statement such as
/// `float{3_f32}` which will ensure a compiler error if the conversion can lose
/// data. Lossy conversions can be done through the `sus::convert::Transmogrify`
/// concept, such as `sus::mog<float>(3_f32)`.
template <PrimitiveFloat U>
  requires(::sus::mem::size_of<U>() >= ::sus::mem::size_of<_primitive>())
sus_pure constexpr inline explicit operator U() const {
  return primitive_value;
}

/// sus::ops::Eq<T> trait for floating point numbers.
/// #[doc.overloads=float.eq.self]
[[nodiscard]] sus_pure friend constexpr inline bool operator==(
    _self l, _self r) noexcept {
  return (l.primitive_value <=> r.primitive_value) == 0;
}
/// sus::ops::Eq<T, U> trait for floating point numbers and primitive floating
/// point numbers.
/// #[doc.overloads=float.eq.prim]
[[nodiscard]] sus_pure friend constexpr inline bool operator==(
    _self l, PrimitiveFloat auto r) noexcept {
  return (l.primitive_value <=> r) == 0;
}
/// sus::ops::Eq<T, U> trait for heterogeneous floating point numbers.
/// #[doc.overloads=float.eq.float]
[[nodiscard]] sus_pure friend constexpr inline bool operator==(
    _self l, Float auto r) noexcept {
  return (l.primitive_value <=> r.primitive_value) == 0;
}

/// sus::ops::PartialOrd<T> trait for floating point numbers.
/// #[doc.overloads=float.ord.self]
[[nodiscard]] sus_pure friend constexpr inline auto operator<=>(
    _self l, _self r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}
/// sus::ops::PartialOrd<T, U> trait for floating point numbers and primitive
/// floating point numbers.
/// #[doc.overloads=float.ord.prim]
[[nodiscard]] sus_pure friend constexpr inline auto operator<=>(
    _self l, PrimitiveFloat auto r) noexcept {
  return l.primitive_value <=> r;
}
/// sus::ops::PartialOrd<T, U> trait for heterogeneous floating point numbers.
/// #[doc.overloads=float.ord.float]
[[nodiscard]] sus_pure friend constexpr inline auto operator<=>(
    _self l, Float auto r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}

/// Return the ordering between `*this` and `other`.
///
/// Unlike the standard partial comparison between floating point numbers,
/// this comparison always produces an ordering in accordance to the
/// totalOrder predicate as defined in the IEEE 754 (2008 revision) floating
/// point standard. The values are ordered in the following sequence:
///
/// negative quiet NaN
/// negative signaling NaN
/// negative infinity
/// negative numbers
/// negative subnormal numbers
/// negative zero
/// positive zero
/// positive subnormal numbers
/// positive numbers
/// positive infinity
/// positive signaling NaN
/// positive quiet NaN.
///
/// The ordering established by this function does not always agree with the
/// `PartialOrd` and `Eq` implementations of @doc.self. For example, they
/// consider negative and positive zero equal, while total_cmp doesn't.
///
/// The interpretation of the signaling NaN bit follows the definition in the
/// IEEE 754 standard, which may not match the interpretation by some of the
/// older, non-conformant (e.g. MIPS) hardware implementations.
sus_pure constexpr std::strong_ordering total_cmp(_self other) const& noexcept {
  return __private::float_strong_ordering(primitive_value,
                                          other.primitive_value);
}

/// sus::num::Neg<@doc.self> trait. */
sus_pure constexpr inline _self operator-() const {
  return _self(-primitive_value);
}

/// sus::num::Add<T> trait for floating point types. */
/// #[doc.overloads=float.+]
[[nodiscard]] sus_pure friend constexpr inline _self operator+(
    _self l, _self r) noexcept {
  return l.primitive_value + r.primitive_value;
}
/// sus::num::Sub<T> trait for floating point types. */
/// #[doc.overloads=float.-]
[[nodiscard]] sus_pure friend constexpr inline _self operator-(
    _self l, _self r) noexcept {
  return l.primitive_value - r.primitive_value;
}
/// sus::num::Mul<T> trait for floating point types. */
/// #[doc.overloads=float.*]
[[nodiscard]] sus_pure friend constexpr inline _self operator*(
    _self l, _self r) noexcept {
  return l.primitive_value * r.primitive_value;
}
/// sus::num::Div<T> trait for floating point types. */
/// #[doc.overloads=float./]
[[nodiscard]] sus_pure friend constexpr inline _self operator/(
    _self l, _self r) noexcept {
  return l.primitive_value / r.primitive_value;
}
/// sus::num::Rem<T> trait for floating point types. */
///
/// The remainder from the division of two floats.
///
/// The remainder has the same sign as the dividend and is computed as:
/// `l - (l / r).trunc() * r`.
///
/// #[doc.overloads=float.%]
[[nodiscard]] sus_pure friend constexpr inline _self operator%(
    _self l, _self r) noexcept {
  const auto x = l.primitive_value;
  const auto y = r.primitive_value;
  return x - __private::truncate_float(x / y) * y;
}

/// sus::num::AddAssign<@doc.self> trait. */
constexpr inline void operator+=(_self r) & noexcept {
  primitive_value += r.primitive_value;
}
/// sus::num::SubAssign<@doc.self> trait. */
constexpr inline void operator-=(_self r) & noexcept {
  primitive_value -= r.primitive_value;
}
/// sus::num::MulAssign<@doc.self> trait. */
constexpr inline void operator*=(_self r) & noexcept {
  primitive_value *= r.primitive_value;
}
/// sus::num::DivAssign<@doc.self> trait. */
constexpr inline void operator/=(_self r) & noexcept {
  primitive_value /= r.primitive_value;
}
/// sus::num::RemAssign<@doc.self> trait. */
///
/// Assigns the remainder from the division of two floats.
///
/// The remainder has the same sign as the dividend and is computed as:
/// `l - (l / r).trunc() * r`.
constexpr inline void operator%=(_self r) & noexcept {
  const auto x = primitive_value;
  const auto y = r.primitive_value;
  primitive_value = x - __private::truncate_float(x / y) * y;
}

// Computes the absolute value of itself.
sus_pure inline _self abs() const& noexcept {
  return __private::into_float(
      __private::into_unsigned_integer(primitive_value) &
      ~__private::high_bit<_primitive>());
}

/// Computes the arccosine of a number. Return value is in radians in the
/// range [0, pi] or NaN if the number is outside the range [-1, 1].
sus_pure inline _self acos() const& noexcept {
  if (primitive_value < _primitive{-1} || primitive_value > _primitive{1})
      [[unlikely]]
    return NAN;
  // MSVC acos(float) is returning a double for some reason.
  return static_cast<_primitive>(::acos(primitive_value));
}
/// Inverse hyperbolic cosine function, or NaN if the number is less than
/// -1.
sus_pure inline _self acosh() const& noexcept {
  if (primitive_value < _primitive{-1}) [[unlikely]]
    return NAN;
  // MSVC acosh(float) is returning a double for some reason.
  return static_cast<_primitive>(::acosh(primitive_value));
}
/// Computes the arcsine of a number. Return value is in radians in the
/// range [-pi/2, pi/2] or NaN if the number is outside the range [-1, 1].
sus_pure inline _self asin() const& noexcept {
  if (primitive_value < _primitive{-1} || primitive_value > _primitive{1})
      [[unlikely]]
    return NAN;
  // MSVC asin(float) is returning a double for some reason.
  return static_cast<_primitive>(::asin(primitive_value));
}
/// Inverse hyperbolic sine function.
sus_pure inline _self asinh() const& noexcept {
  if (primitive_value < _primitive{-1}) [[unlikely]]
    return NAN;
  // MSVC asinh(float) is returning a double for some reason.
  return static_cast<_primitive>(::asinh(primitive_value));
}
/// Computes the arctangent of a number. Return value is in radians in the
/// range [-pi/2, pi/2];
sus_pure inline _self atan() const& noexcept {
  // MSVC atan(float) is returning a double for some reason.
  return static_cast<_primitive>(::atan(primitive_value));
}
/// Computes the four quadrant arctangent of self (y) and other (x) in
/// radians.
///
/// - x = 0, y = 0: 0
/// - x >= 0: arctan(y/x) -> [-pi/2, pi/2]
/// - y >= 0: arctan(y/x) + pi -> (pi/2, pi]
/// - y < 0: arctan(y/x) - pi -> (-pi, -pi/2)
///
/// Returns NaN if both `self` and `other` are 0.
sus_pure inline _self atan2(_self other) const& noexcept {
  // MSVC atan2(float) is returning a double for some reason.
  return static_cast<_primitive>(
      ::atan2(primitive_value, other.primitive_value));
}
/// Inverse hyperbolic tangent function.
sus_pure inline _self atanh() const& noexcept {
  // MSVC atanh(float) is returning a double for some reason.
  return static_cast<_primitive>(::atanh(primitive_value));
}
/// Returns the cube root of a number.
sus_pure inline _self cbrt() const& noexcept {
  // MSVC cbrt(float) is returning a double for some reason.
  return static_cast<_primitive>(::cbrt(primitive_value));
}
/// Returns the smallest integer greater than or equal to self.
sus_pure inline _self ceil() const& noexcept {
  // MSVC ceil(float) is returning a double for some reason.
  return static_cast<_primitive>(::ceil(primitive_value));
}
/// Returns a number composed of the magnitude of self and the sign of sign.
///
/// Equal to self if the sign of self and sign are the same, otherwise equal
/// to -self. If self is a NaN, then a NaN with the sign bit of sign is
/// returned. Note, however, that conserving the sign bit on NaN across
/// arithmetical operations is not generally guaranteed.
sus_pure inline _self copysign(_self sign) const& noexcept {
  // MSVC copysign(float) is returning a double for some reason.
  return static_cast<_primitive>(
      ::copysign(primitive_value, sign.primitive_value));
}
/// Computes the cosine of a number (in radians).
sus_pure inline _self cos() const& noexcept {
  // MSVC cos(float) is returning a double for some reason.
  return static_cast<_primitive>(::cos(primitive_value));
}
/// Hyperbolic cosine function.
sus_pure inline _self cosh() const& noexcept {
  // MSVC cosh(float) is returning a double for some reason.
  return static_cast<_primitive>(::cosh(primitive_value));
}
/// Returns `e^(self)`, (the exponential function).
sus_pure inline _self exp() const& noexcept {
  // MSVC exp(float) is returning a double for some reason.
  return static_cast<_primitive>(::exp(primitive_value));
}
/// Returns `2^(self)`.
sus_pure inline _self exp2() const& noexcept {
  // MSVC exp2(float) is returning a double for some reason.
  return static_cast<_primitive>(::exp2(primitive_value));
}
/// Returns `e^(self) - 1` in a way that is accurate even if the number is
/// close to zero.
sus_pure inline _self exp_m1() const& noexcept {
  // MSVC expm1(float) is returning a double for some reason.
  return static_cast<_primitive>(::expm1(primitive_value));
}
/// Returns the largest integer less than or equal to self.
sus_pure inline _self floor() const& noexcept {
  // MSVC floor(float) is returning a double for some reason.
  return static_cast<_primitive>(::floor(primitive_value));
}
/// Calculates the length of the hypotenuse of a right-angle triangle given
/// legs of length x and y.
sus_pure inline _self hypot(_self other) const& noexcept {
  // MSVC hypot(float) is returning a double for some reason.
  return static_cast<_primitive>(
      ::hypot(primitive_value, other.primitive_value));
}
/// Returns the natural logarithm of the number.
sus_pure inline _self ln() const& noexcept {
  // MSVC log(float) is returning a double for some reason.
  return static_cast<_primitive>(::log(primitive_value));
}
/// Returns ln(1+n) (natural logarithm) more accurately than if the
/// operations were performed separately.
sus_pure inline _self ln_1p() const& noexcept {
  // MSVC log1p(float) is returning a double for some reason.
  return static_cast<_primitive>(::log1p(primitive_value));
}
/// Returns the base 10 logarithm of the number.
sus_pure inline _self log10() const& noexcept {
  // MSVC log10(float) is returning a double for some reason.
  return static_cast<_primitive>(::log10(primitive_value));
}
/// Returns the base 2 logarithm of the number.
sus_pure inline _self log2() const& noexcept {
  // MSVC log2(float) is returning a double for some reason.
  return static_cast<_primitive>(::log2(primitive_value));
}
/// Returns the logarithm of the number with respect to an arbitrary base.
///
/// The result might not be correctly rounded owing to implementation
/// details; self.log2() can produce more accurate results for base 2, and
/// self.log10() can produce more accurate results for base 10.
sus_pure inline _self log(_self base) const& noexcept {
  return ln() / base.ln();
}
/// Returns the maximum of the two numbers, ignoring NaN.
///
/// If one of the arguments is NaN, then the other argument is returned.
sus_pure inline _self max(_self other) const& noexcept {
  // MSVC fmax(float) is returning a double for some reason.
  return static_cast<_primitive>(
      ::fmax(primitive_value, other.primitive_value));
}
/// Returns the minimum of the two numbers, ignoring NaN.
///
/// If one of the arguments is NaN, then the other argument is returned.
sus_pure inline _self min(_self other) const& noexcept {
  // MSVC fmin(float) is returning a double for some reason.
  return static_cast<_primitive>(
      ::fmin(primitive_value, other.primitive_value));
}
/// Fused multiply-add. Computes `(self * a) + b` with only one rounding
/// error, yielding a more accurate result than an unfused multiply-add.
///
/// Using mul_add may be more performant than an unfused multiply-add if the
/// target architecture has a dedicated fma CPU instruction. However, this is
/// not always true, and will be heavily dependent on designing algorithms
/// with specific target hardware in mind.
sus_pure inline _self mul_add(_self a, _self b) const& noexcept {
  // MSVC fma(float) is returning a double for some reason.
  return static_cast<_primitive>(
      ::fma(primitive_value, a.primitive_value, b.primitive_value));
}

/// Returns the next representable value of the float type after `self` in the
/// direction of `toward`. If `self == toward`, `toward` is returned. If either
/// `self` or `toward` is NAN, NAN is returned.
///
/// This is implemented by the
/// [cmath](https://en.cppreference.com/w/c/numeric/math/nextafter) library, see
/// the documentation for details on errors.
//
// TODO: constexpr in C++23.
sus_pure inline _self next_toward(_self toward) const& noexcept {
  return __private::next_toward(primitive_value, toward.primitive_value);
}

/// Raises a number to a floating point power.
sus_pure inline _self powf(_self n) const& noexcept {
  // MSVC pow(float) is returning a double for some reason.
  return static_cast<_primitive>(::pow(primitive_value, n.primitive_value));
}
/// Raises a number to an integer point power.
///
/// Using this function may be faster than using `powf()`. It might have a
/// different sequence of rounding operations than `powf()`, so the results
/// are not guaranteed to agree.
sus_pure inline _self powi(i32 n) const& noexcept {
  // MSVC pow(float) is returning a double for some reason.
  return static_cast<_primitive>(
      ::pow(primitive_value, int{n.primitive_value}));
}
/// Takes the reciprocal (inverse) of a number, `1/x`.
sus_pure inline _self recip() const& noexcept {
  return _primitive{1} / primitive_value;
}
/// Returns the nearest integer to itself, rounding half-way cases away from
/// `0.0`.
sus_pure inline _self round() const& noexcept {
  return __private::float_round(primitive_value);
}
/// Returns a number that represents the sign of self.
///
/// - `1.0` if the number is positive, `+0.0` or `INFINITY`
/// - `-1.0` if the number is negative, `-0.0` or `NEG_INFINITY`
/// - `NaN` if the number is `NaN`
sus_pure inline _self signum() const& noexcept {
  return __private::float_signum(primitive_value);
}
/// Computes the sine of a number (in radians).
sus_pure inline _self sin() const& noexcept {
  // MSVC sin(float) is returning a double for some reason.
  return static_cast<_primitive>(::sin(primitive_value));
}
/// Hyperbolic sine function.
sus_pure inline _self sinh() const& noexcept {
  // MSVC sinh(float) is returning a double for some reason.
  return static_cast<_primitive>(::sinh(primitive_value));
}
/// Returns the square root of a number.
///
/// Returns NaN if self is a negative number other than `-0.0`.
sus_pure inline _self sqrt() const& noexcept {
  if (primitive_value < -_primitive{0}) [[unlikely]]
    return NAN;
  // MSVC sqrt(float) is returning a double for some reason.
  return static_cast<_primitive>(::sqrt(primitive_value));
}
/// Computes the tangent of a number (in radians).
sus_pure inline _self tan() const& noexcept {
  // MSVC tan(float) is returning a double for some reason.
  return static_cast<_primitive>(::tan(primitive_value));
}
/// Hyperbolic tangent function.
sus_pure inline _self tanh() const& noexcept {
  // MSVC tanh(float) is returning a double for some reason.
  return static_cast<_primitive>(::tanh(primitive_value));
}

// Returns the fractional part of self.
sus_pure inline _self fract() const& noexcept {
  return primitive_value - __private::truncate_float(primitive_value);
}
/// Returns the integer part of self. This means that non-integer numbers
/// are always truncated towards zero.
sus_pure inline _self trunc() const& noexcept {
  return __private::truncate_float(primitive_value);
}

/// Converts radians to degrees.
sus_pure inline _self to_degrees() const& noexcept {
  // Use a constant for better precision.
  constexpr auto PIS_IN_180 = _primitive{57.2957795130823208767981548141051703};
  return primitive_value * PIS_IN_180;
}
/// Converts degrees to radians.
sus_pure inline _self to_radians() const& noexcept {
  return primitive_value * (consts::PI.primitive_value / _primitive{180});
}
/// Rounds toward zero and converts to any primitive integer type, assuming
/// that the value is finite and fits in that type.
template <Integer I>
sus_pure constexpr inline I to_int_unchecked(
    ::sus::marker::UnsafeFnMarker) const& noexcept {
  return static_cast<decltype(I::primitive_value)>(primitive_value);
}

/// Raw transmutation from `##_unsigned##`.
///
/// This is identical to [`std::bit_cast<f32,
/// u32>`](https://en.cppreference.com/w/cpp/numeric/bit_cast), or
/// `std::bit_cast<f64, u64>`. It turns out this is incredibly portable, for two
/// reasons:
///
/// * Floats and Ints have the same endianness on all modern platforms.
/// * IEEE 754 very precisely specifies the bit layout of floats.
///
/// However there is one caveat: prior to the 2008 version of IEEE 754, how to
/// interpret the NaN signaling bit wasn’t actually specified. Most platforms
/// (notably x86 and ARM) picked the interpretation that was ultimately
/// standardized in 2008, but some didn’t (notably MIPS). As a result, all
/// signaling NaNs on MIPS are quiet NaNs on x86, and vice-versa.
///
/// Rather than trying to preserve signaling-ness cross-platform, this
/// implementation favors preserving the exact bits. This means that any
/// payloads encoded in NaNs will be preserved even if the result of this method
/// is sent over the network from an x86 machine to a MIPS one.
///
/// If the results of this method are only manipulated by the same architecture
/// that produced them, then there is no portability concern.
///
/// If the input isn’t NaN, then there is no portability concern.
///
/// If you don’t care about signalingness (very likely), then there is no
/// portability concern.
///
/// Note that this function is distinct from `Transmogrify` casting, which
/// attempts to preserve the *numeric* value, and not the bitwise value.
///
/// # Examples
/// ```
/// auto v = f32::from_bits(0x41480000);
/// sus::check!(v, 12.5);
/// ```
///
/// This function is not constexpr, as converting a NaN does not preserve the
/// exact bits in a constexpr context.
sus_pure static _self from_bits(_unsigned v) noexcept {
  return std::bit_cast<_self>(v);
}
/// Raw transmutation to ##UnsignedT##.
///
/// This is identical to [`std::bit_cast<u32,
/// f32>`](https://en.cppreference.com/w/cpp/numeric/bit_cast), or
/// `std::bit_cast<u64, f64>`.
///
/// See `from_bits()` for some discussion of the portability of this operation
/// (there are almost no issues).
///
/// Note that this function is distinct from `Transmogrify` casting, which
/// attempts to preserve the *numeric* value, and not the bitwise value.
sus_pure constexpr inline _unsigned to_bits() const& noexcept {
  return std::bit_cast<decltype(_unsigned::primitive_value)>(primitive_value);
}

/// Returns the floating point category of the number.
///
/// If only one property is going to be tested, it is generally faster to use
/// the specific predicate instead.
sus_pure constexpr inline FpCategory classify() const& noexcept {
  return __private::float_category(primitive_value);
}
/// Returns true if this number is neither infinite nor NaN.
sus_pure constexpr inline bool is_finite() const& noexcept {
  return !__private::float_is_inf_or_nan(primitive_value);
}
/// Returns true if this value is positive infinity or negative infinity,
/// and false otherwise.
sus_pure constexpr inline bool is_infinite() const& noexcept {
  return __private::float_is_inf(primitive_value);
}
/// Returns true if this value is NaN.
sus_pure constexpr inline bool is_nan() const& noexcept {
  return __private::float_is_nan(primitive_value);
}
/// Returns true if the number is neither zero, infinite, subnormal, or NaN.
sus_pure constexpr inline bool is_normal() const& noexcept {
  return __private::float_is_normal(primitive_value);
}
/// Returns true if self has a negative sign, including -0.0, NaNs with
/// negative sign bit and negative infinity.
///
/// Note that IEEE-745 doesn't assign any meaning to the sign bit in case of
/// a NaN
sus_pure constexpr inline bool is_sign_negative() const& noexcept {
  return __private::float_signbit(primitive_value);
}
/// Returns true if self has a positive sign, including +0.0, NaNs with
/// positive sign bit and positive infinity.
///
/// Note that IEEE-745 doesn't assign any meaning to the sign bit in case of
/// a NaN.
sus_pure constexpr inline bool is_sign_positive() const& noexcept {
  return !__private::float_signbit(primitive_value);
}
/// Returns true if the number is subnormal.
sus_pure constexpr inline bool is_subnormal() const& noexcept {
  return !__private::float_is_zero(primitive_value) &&
         __private::float_nonzero_is_subnormal(primitive_value);
}

/// Restrict a value to a certain interval unless it is NaN.
///
/// Returns max if self is greater than max, and min if self is less than
/// min. Otherwise this returns self.
///
/// Note that this function returns NaN if the initial value was NaN as well.
///
/// # Panics
/// Panics if min > max, min is NaN, or max is NaN.
sus_pure constexpr inline _self clamp(_self min, _self max) const& noexcept {
  check(!min.is_nan() && !max.is_nan() &&
        min.primitive_value <= max.primitive_value);
  // SAFETY: We have verified that the min and max are not NaN and that
  // `min <= max`.
  return __private::float_clamp(::sus::marker::unsafe_fn, primitive_value,
                                min.primitive_value, max.primitive_value);
}

/// Calculates Euclidean division, the matching method for `rem_euclid`.
///
/// This computes the integer `n` such that `self = n * rhs +
/// self.rem_euclid(rhs)`. In other words, the result is `self / rhs` rounded
/// to the integer `n` such that `self >= n * rhs`.
sus_pure _self div_euclid(_self rhs) const& noexcept {
  const auto q = (*this / rhs).trunc();
  if (*this % rhs < _primitive{0}) {
    if (rhs > _self{_primitive{0}})
      return q - _self{_primitive{1}};
    else
      return q + _self{_primitive{1}};
  }
  return q;
}
/// Calculates the least nonnegative remainder of `self (mod rhs)`.
///
/// In particular, the return value `r` satisfies `0.0 <= r < rhs.abs()` in
/// most cases. However, due to a floating point round-off error it can
/// result in `r == rhs.abs()`, violating the mathematical definition, if
/// `self` is much smaller than `rhs.abs()` in magnitude and `self < 0.0`.
/// This result is not an element of the function's codomain, but it is the
/// closest floating point number in the real numbers and thus fulfills the
/// property `self == self.div_euclid(rhs) * rhs + self.rem_euclid(rhs)`
/// approximately.
sus_pure _self rem_euclid(_self rhs) const& noexcept {
  const auto r = *this % rhs;
  if (r < _self{_primitive{0}})
    return r + rhs.abs();
  else
    return r;
}

/// Return the memory representation of this floating point number as a byte
/// array in big-endian (network) byte order.
sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
to_be_bytes() const& noexcept;
/// Return the memory representation of this floating point number as a byte
/// array in little-endian byte order.
sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
to_le_bytes() const& noexcept;
/// Return the memory representation of this floating point number as a byte
/// array in native byte order.
///
/// As the target platform's native endianness is used, portable code should
/// use `to_be_bytes()` or `to_le_bytes()`, as appropriate, instead.
sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
to_ne_bytes() const& noexcept;
/// Create a floating point value from its representation as a byte array in
/// big endian.
///
/// See `@doc.self::from_bits()` for why this function is not constexpr.
sus_pure static _self from_be_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;
/// Create a floating point value from its representation as a byte array in
/// big endian.
///
///  See `@doc.self::from_bits()` for why this function is not constexpr.
sus_pure static _self from_le_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;
/// Create a floating point value from its representation as a byte array in
/// native endian.
///
/// As the target platform's native endianness is used, portable code likely
/// wants to use `from_be_bytes()` or `from_le_bytes()`, as appropriate
/// instead.
///
///  See `@doc.self::from_bits()` for why this function is not constexpr.
sus_pure static _self from_ne_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

#undef _self
#undef _primitive
#undef _unsigned
