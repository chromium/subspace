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

#include <concepts>

#include "num/__private/intrinsics.h"
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

#define _sus__float_comparison(T)                                             \
  /** sus::concepts::Eq<##T##> trait. */                                      \
  friend constexpr inline bool operator==(const T& l, const T& r) noexcept {  \
    return (l.primitive_value <=> r.primitive_value) == 0;                    \
  }                                                                           \
  /** sus::concepts::Ord<##T##> trait. */                                     \
  friend constexpr inline auto operator<=>(const T& l, const T& r) noexcept { \
    return l.primitive_value <=> r.primitive_value;                           \
  }                                                                           \
  static_assert(true)

#define _sus__float_negate(T) \
  constexpr inline T operator-() const { return T(-primitive_value); }

#define _sus__float(T, PrimitiveT)      \
  _sus__float_storage(PrimitiveT);      \
  _sus__float_constants(T, PrimitiveT); \
  _sus__float_construct(T, PrimitiveT); \
  _sus__float_comparison(T);            \
  _sus__float_negate(T);                \
                                        \
  static_assert(true)
