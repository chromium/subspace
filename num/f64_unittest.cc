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

#include <math.h>

#include "num/float.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

#define F64_NEAR(a, b, c) \
  EXPECT_NEAR(a.primitive_value, b.primitive_value, c.primitive_value);

namespace {

TEST(f64, Traits) {
  static_assert(sus::num::Neg<f64>);
  static_assert(sus::num::Add<f64, f64>);
  static_assert(sus::num::AddAssign<f64, f64>);
  static_assert(sus::num::Sub<f64, f64>);
  static_assert(sus::num::SubAssign<f64, f64>);
  static_assert(sus::num::Mul<f64, f64>);
  static_assert(sus::num::MulAssign<f64, f64>);
  static_assert(sus::num::Div<f64, f64>);
  static_assert(sus::num::DivAssign<f64, f64>);
  static_assert(sus::num::Rem<f64, f64>);
  static_assert(sus::num::RemAssign<f64, f64>);
  static_assert(!sus::num::BitAnd<f64, f64>);
  static_assert(!sus::num::BitAndAssign<f64, f64>);
  static_assert(!sus::num::BitOr<f64, f64>);
  static_assert(!sus::num::BitOrAssign<f64, f64>);
  static_assert(!sus::num::BitXor<f64, f64>);
  static_assert(!sus::num::BitXorAssign<f64, f64>);
  static_assert(!sus::num::BitNot<f64>);
  static_assert(!sus::num::Shl<f64>);
  static_assert(!sus::num::ShlAssign<f64>);
  static_assert(!sus::num::Shr<f64>);
  static_assert(!sus::num::ShrAssign<f64>);

  static_assert(!sus::num::Ord<f64, f64>);
  static_assert(!sus::num::WeakOrd<f64, f64>);
  static_assert(sus::num::PartialOrd<f64, f64>);
  static_assert(1_f64 >= 1_f64);
  static_assert(2_f64 > 1_f64);
  static_assert(1_f64 <= 1_f64);
  static_assert(1_f64 < 2_f64);
  static_assert(1_f64 <= f64::TODO_NAN());
  static_assert(sus::num::Eq<f64, f64>);
  static_assert(1_f64 == 1_f64);
  static_assert(!(1_f64 == 2_f64));
  static_assert(1_f64 != 2_f64);
  static_assert(!(1_f64 != 1_f64));
  static_assert(f64::TODO_NAN() != f64::TODO_NAN());

  // Verify constexpr.
  constexpr f64 c = 1_f64 + 2_f64 - 3_f64 * 4_f64 / 5_f64 % 6_f64;
  constexpr std::partial_ordering o = 2_f64 <=> 3_f64;
}

TEST(f64, Consts) {
  {
    constexpr auto min = f64::MIN();
    static_assert(std::same_as<decltype(min), const f64>);
    EXPECT_EQ(min.primitive_value, -DBL_MAX);
    constexpr auto max = f64::MAX();
    static_assert(std::same_as<decltype(max), const f64>);
    EXPECT_EQ(max.primitive_value, DBL_MAX);
  }
  {
    constexpr auto min = f64::MIN_PRIMITIVE;
    static_assert(std::same_as<decltype(min), const double>);
    EXPECT_EQ(min, -DBL_MAX);
    constexpr auto max = f64::MAX_PRIMITIVE;
    static_assert(std::same_as<decltype(max), const double>);
    EXPECT_EQ(max, DBL_MAX);
  }

  EXPECT_EQ(f64::RADIX(), 2_u32);
  EXPECT_EQ(f64::MANTISSA_DIGITS(), 53_u32);
  EXPECT_EQ(f64::DIGITS(), 15_u32);
  EXPECT_EQ(f64::EPSILON(), f64(DBL_EPSILON));
  EXPECT_EQ(f64::MIN(), f64(-DBL_MAX));
  EXPECT_EQ(f64::MAX(), f64(DBL_MAX));
  EXPECT_EQ(f64::MIN_POSITIVE(), DBL_MIN);
  EXPECT_EQ(f64::MIN_EXP(), -1021_i32);
  EXPECT_EQ(f64::MAX_EXP(), 1024_i32);
  EXPECT_EQ(f64::MIN_10_EXP(), -307_i32);
  EXPECT_EQ(f64::MAX_10_EXP(), 308_i32);
  EXPECT_TRUE(isnan(f64::TODO_NAN().primitive_value));
  EXPECT_TRUE(isinf(f64::TODO_INFINITY().primitive_value));
  EXPECT_GT(f64::TODO_INFINITY(), 0_f64);
  EXPECT_TRUE(isinf(f64::NEG_INFINITY().primitive_value));
  EXPECT_LT(f64::NEG_INFINITY(), 0_f64);

  EXPECT_EQ(f64::consts::E(), 2.71828182845904523536028747135266250_f64);
  EXPECT_EQ(f64::consts::FRAC_1_PI(),
            0.318309886183790671537767526745028724_f64);
  EXPECT_EQ(f64::consts::FRAC_1_SQRT_2(),
            0.707106781186547524400844362104849039_f64);
  EXPECT_EQ(f64::consts::FRAC_2_PI(),
            0.636619772367581343075535053490057448_f64);
  EXPECT_EQ(f64::consts::FRAC_2_SQRT_PI(),
            1.12837916709551257389615890312154517_f64);
  EXPECT_EQ(f64::consts::FRAC_PI_2(),
            1.57079632679489661923132169163975144_f64);
  EXPECT_EQ(f64::consts::FRAC_PI_3(),
            1.04719755119659774615421446109316763_f64);
  EXPECT_EQ(f64::consts::FRAC_PI_4(),
            0.785398163397448309615660845819875721_f64);
  EXPECT_EQ(f64::consts::FRAC_PI_6(),
            0.52359877559829887307710723054658381_f64);
  EXPECT_EQ(f64::consts::FRAC_PI_8(),
            0.39269908169872415480783042290993786_f64);
  EXPECT_EQ(f64::consts::LN_2(), 0.693147180559945309417232121458176568_f64);
  EXPECT_EQ(f64::consts::LN_10(), 2.30258509299404568401799145468436421_f64);
  EXPECT_EQ(f64::consts::LOG2_10(), 3.32192809488736234787031942948939018_f64);
  EXPECT_EQ(f64::consts::LOG2_E(), 1.44269504088896340735992468100189214_f64);
  EXPECT_EQ(f64::consts::LOG10_2(), 0.301029995663981195213738894724493027_f64);
  EXPECT_EQ(f64::consts::LOG10_E(), 0.434294481903251827651128918916605082_f64);
  EXPECT_EQ(f64::consts::PI(), 3.14159265358979323846264338327950288_f64);
  EXPECT_EQ(f64::consts::SQRT_2(), 1.41421356237309504880168872420969808_f64);
  EXPECT_EQ(f64::consts::TAU(), 6.28318530717958647692528676655900577_f64);
}

TEST(f64, Literals) {
  static_assert((0._f64).primitive_value == 0.);
  static_assert((0.0_f64).primitive_value == 0.);
  static_assert((1.2345678912345_f64).primitive_value == 1.2345678912345);
  static_assert((-1.2345678912345_f64).primitive_value == -1.2345678912345);

  // Whole numbers.
  static_assert((0_f64).primitive_value == 0.);
  static_assert((1_f64).primitive_value == 1.);
  static_assert((-5_f64).primitive_value == -5.);
}

TEST(f64, ConstructPrimitive) {
  auto a = f64();
  EXPECT_EQ(a.primitive_value, 0.);

  f64 b;
  EXPECT_EQ(b.primitive_value, 0.);

  auto c = f64(1.2);
  EXPECT_EQ(c.primitive_value, 1.2);
}

TEST(f64, AssignPrimitive) {
  auto a = f64();
  EXPECT_EQ(a.primitive_value, 0.f);
  a = 1.2;
  EXPECT_EQ(a.primitive_value, 1.2);
}

TEST(f64, Negate) {
  constexpr auto a = -(0.345_f64);
  EXPECT_EQ(a, f64(-0.345));

  auto b = 0.345_f64;
  EXPECT_EQ(-b, f64(-0.345));
}

TEST(f64, BinaryOperators) {
  {
    constexpr auto a = 1_f64 + 0.345_f64;
    EXPECT_EQ(a, f64(1.345));

    auto b = 1_f64;
    b += 0.345_f64;
    EXPECT_EQ(b, f64(1.345));
  }
  {
    constexpr auto a = 1_f64 - 0.345_f64;
    EXPECT_EQ(a, f64(0.655));

    auto b = 1_f64;
    b -= 0.345_f64;
    EXPECT_EQ(b, f64(0.655));
  }
  {
    constexpr auto a = 2_f64 * 0.345_f64;
    EXPECT_EQ(a, f64(0.690));

    auto b = 2_f64;
    b *= 0.345_f64;
    EXPECT_EQ(b, f64(0.690));
  }
  {
    constexpr auto a = 0.690_f64 / 2_f64;
    EXPECT_EQ(a, f64(0.345));

    auto b = 0.690_f64;
    b /= 2_f64;
    EXPECT_EQ(b, f64(0.345));
  }
  {
    constexpr auto a = 2.345_f64 % 2_f64;
    F64_NEAR(a, f64(0.345), 0.00001_f64);

    constexpr auto b = 2.4_f64 % 1.1_f64;
    F64_NEAR(b, f64(0.2), 0.00001_f64);

    auto c = 2.345_f64;
    c %= 2_f64;
    F64_NEAR(c, f64(0.345), 0.00001_f64);

    auto d = 2.4_f64;
    d %= 1.1_f64;
    F64_NEAR(d, f64(0.2), 0.00001_f64);
  }
}

TEST(f64, Abs) {
  auto a = (-0.345_f64).abs();
  EXPECT_EQ(a, 0.345_f64);

  auto b = 0.345_f64;
  EXPECT_EQ(b.abs(), 0.345_f64);
}

TEST(f64, TotalCmp) {
  // TODO: Use classify on f64.

  const auto quiet_nan = std::bit_cast<f64>(uint64_t{0x7ff8000000000000});
  const auto signaling_nan = std::bit_cast<f64>(uint64_t{0x7ff0000000000001});
  EXPECT_EQ(::fpclassify(quiet_nan.primitive_value), FP_NAN);
  EXPECT_EQ(::fpclassify(signaling_nan.primitive_value), FP_NAN);
  EXPECT_TRUE(
      sus::num::__private::float_is_nan_quiet(quiet_nan.primitive_value));
  EXPECT_FALSE(
      sus::num::__private::float_is_nan_quiet(signaling_nan.primitive_value));

  const auto quiet_nan2 = std::bit_cast<f64>(uint64_t{0x7ff8000000000001});
  const auto signaling_nan2 = std::bit_cast<f64>(uint64_t{0x7ff0000000000002});
  EXPECT_EQ(::fpclassify(quiet_nan2.primitive_value), FP_NAN);
  EXPECT_EQ(::fpclassify(signaling_nan2.primitive_value), FP_NAN);
  EXPECT_TRUE(
      sus::num::__private::float_is_nan_quiet(quiet_nan2.primitive_value));
  EXPECT_FALSE(
      sus::num::__private::float_is_nan_quiet(signaling_nan2.primitive_value));

  const auto neg_quiet_nan = std::bit_cast<f64>(uint64_t{0xfff8000000000000});
  const auto neg_signaling_nan = std::bit_cast<f64>(uint64_t{0xfff0000000000001});
  EXPECT_EQ(::fpclassify(neg_quiet_nan.primitive_value), FP_NAN);
  EXPECT_EQ(::fpclassify(neg_signaling_nan.primitive_value), FP_NAN);
  EXPECT_TRUE(
      sus::num::__private::float_is_nan_quiet(neg_quiet_nan.primitive_value));
  EXPECT_FALSE(sus::num::__private::float_is_nan_quiet(
      neg_signaling_nan.primitive_value));

  const auto neg_quiet_nan2 = std::bit_cast<f64>(uint64_t{0xfff8000000000001});
  const auto neg_signaling_nan2 = std::bit_cast<f64>(uint64_t{0xfff0000000000002});
  EXPECT_EQ(::fpclassify(neg_quiet_nan2.primitive_value), FP_NAN);
  EXPECT_EQ(::fpclassify(neg_signaling_nan2.primitive_value), FP_NAN);
  EXPECT_TRUE(
      sus::num::__private::float_is_nan_quiet(neg_quiet_nan2.primitive_value));
  EXPECT_FALSE(sus::num::__private::float_is_nan_quiet(
      neg_signaling_nan2.primitive_value));

  const auto inf = f64::TODO_INFINITY();
  const auto neg_inf = f64::NEG_INFINITY();
  EXPECT_EQ(::fpclassify(inf.primitive_value), FP_INFINITE);
  EXPECT_EQ(::fpclassify(neg_inf.primitive_value), FP_INFINITE);

  const auto norm1 = 123_f64;
  const auto norm2 = 234_f64;
  EXPECT_EQ(::fpclassify(norm1.primitive_value), FP_NORMAL);
  EXPECT_EQ(::fpclassify(norm2.primitive_value), FP_NORMAL);
  constexpr auto neg_norm1 = -123_f64;
  constexpr auto neg_norm2 = -234_f64;
  EXPECT_EQ(::fpclassify(neg_norm1.primitive_value), FP_NORMAL);
  EXPECT_EQ(::fpclassify(neg_norm2.primitive_value), FP_NORMAL);

  const auto subnorm1 =
      f64(std::numeric_limits<decltype(f64::primitive_value)>::denorm_min());
  const auto subnorm2 = subnorm1 * 2_f64;
  EXPECT_NE(subnorm1.primitive_value, subnorm2.primitive_value);
  EXPECT_EQ(::fpclassify(subnorm1.primitive_value), FP_SUBNORMAL);
  EXPECT_EQ(::fpclassify(subnorm2.primitive_value), FP_SUBNORMAL);
  const auto neg_subnorm1 = -subnorm1;
  const auto neg_subnorm2 = -subnorm2;
  EXPECT_EQ(::fpclassify(neg_subnorm1.primitive_value), FP_SUBNORMAL);
  EXPECT_EQ(::fpclassify(neg_subnorm2.primitive_value), FP_SUBNORMAL);

  const auto zero = 0_f64;
  const auto neg_zero = -0_f64;
  EXPECT_EQ(zero, neg_zero);

  EXPECT_EQ(neg_quiet_nan.total_cmp(neg_quiet_nan2),
            std::strong_ordering::greater);

  EXPECT_EQ(neg_quiet_nan.total_cmp(neg_quiet_nan),
            std::strong_ordering::equal);
  EXPECT_EQ(neg_quiet_nan.total_cmp(neg_signaling_nan),
            std::strong_ordering::less);
  EXPECT_EQ(neg_quiet_nan.total_cmp(neg_norm1), std::strong_ordering::less);
  EXPECT_EQ(neg_quiet_nan.total_cmp(neg_subnorm1), std::strong_ordering::less);
  EXPECT_EQ(neg_quiet_nan.total_cmp(neg_zero), std::strong_ordering::less);
  EXPECT_EQ(neg_quiet_nan.total_cmp(zero), std::strong_ordering::less);
  EXPECT_EQ(neg_quiet_nan.total_cmp(subnorm1), std::strong_ordering::less);
  EXPECT_EQ(neg_quiet_nan.total_cmp(norm1), std::strong_ordering::less);
  EXPECT_EQ(neg_quiet_nan.total_cmp(signaling_nan), std::strong_ordering::less);
  EXPECT_EQ(neg_quiet_nan.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(neg_signaling_nan.total_cmp(neg_signaling_nan2),
            std::strong_ordering::greater);

  EXPECT_EQ(neg_signaling_nan.total_cmp(neg_quiet_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(neg_signaling_nan.total_cmp(neg_signaling_nan),
            std::strong_ordering::equal);
  EXPECT_EQ(neg_signaling_nan.total_cmp(neg_norm1), std::strong_ordering::less);
  EXPECT_EQ(neg_signaling_nan.total_cmp(neg_subnorm1),
            std::strong_ordering::less);
  EXPECT_EQ(neg_signaling_nan.total_cmp(neg_zero), std::strong_ordering::less);
  EXPECT_EQ(neg_signaling_nan.total_cmp(zero), std::strong_ordering::less);
  EXPECT_EQ(neg_signaling_nan.total_cmp(subnorm1), std::strong_ordering::less);
  EXPECT_EQ(neg_signaling_nan.total_cmp(norm1), std::strong_ordering::less);
  EXPECT_EQ(neg_signaling_nan.total_cmp(signaling_nan),
            std::strong_ordering::less);
  EXPECT_EQ(neg_signaling_nan.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(neg_inf.total_cmp(neg_quiet_nan), std::strong_ordering::greater);
  EXPECT_EQ(neg_inf.total_cmp(neg_signaling_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(neg_inf.total_cmp(neg_inf), std::strong_ordering::equal);
  EXPECT_EQ(neg_inf.total_cmp(neg_norm1), std::strong_ordering::less);
  EXPECT_EQ(neg_inf.total_cmp(neg_subnorm1), std::strong_ordering::less);
  EXPECT_EQ(neg_inf.total_cmp(neg_zero), std::strong_ordering::less);
  EXPECT_EQ(neg_inf.total_cmp(zero), std::strong_ordering::less);
  EXPECT_EQ(neg_inf.total_cmp(subnorm1), std::strong_ordering::less);
  EXPECT_EQ(neg_inf.total_cmp(norm1), std::strong_ordering::less);
  EXPECT_EQ(neg_inf.total_cmp(inf), std::strong_ordering::less);
  EXPECT_EQ(neg_inf.total_cmp(signaling_nan), std::strong_ordering::less);
  EXPECT_EQ(neg_inf.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(neg_norm1.total_cmp(neg_norm2), std::strong_ordering::greater);

  EXPECT_EQ(neg_norm1.total_cmp(neg_quiet_nan), std::strong_ordering::greater);
  EXPECT_EQ(neg_norm1.total_cmp(neg_signaling_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(neg_norm1.total_cmp(neg_inf), std::strong_ordering::greater);
  EXPECT_EQ(neg_norm1.total_cmp(neg_norm1), std::strong_ordering::equal);
  EXPECT_EQ(neg_norm1.total_cmp(neg_subnorm1), std::strong_ordering::less);
  EXPECT_EQ(neg_norm1.total_cmp(neg_zero), std::strong_ordering::less);
  EXPECT_EQ(neg_norm1.total_cmp(zero), std::strong_ordering::less);
  EXPECT_EQ(neg_norm1.total_cmp(subnorm1), std::strong_ordering::less);
  EXPECT_EQ(neg_norm1.total_cmp(norm1), std::strong_ordering::less);
  EXPECT_EQ(neg_norm1.total_cmp(inf), std::strong_ordering::less);
  EXPECT_EQ(neg_norm1.total_cmp(signaling_nan), std::strong_ordering::less);
  EXPECT_EQ(neg_norm1.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(neg_subnorm1.total_cmp(neg_subnorm2),
            std::strong_ordering::greater);

  EXPECT_EQ(neg_subnorm1.total_cmp(neg_quiet_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(neg_subnorm1.total_cmp(neg_signaling_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(neg_subnorm1.total_cmp(neg_inf), std::strong_ordering::greater);
  EXPECT_EQ(neg_subnorm1.total_cmp(neg_norm1), std::strong_ordering::greater);
  EXPECT_EQ(neg_subnorm1.total_cmp(neg_subnorm1), std::strong_ordering::equal);
  EXPECT_EQ(neg_subnorm1.total_cmp(neg_zero), std::strong_ordering::less);
  EXPECT_EQ(neg_subnorm1.total_cmp(zero), std::strong_ordering::less);
  EXPECT_EQ(neg_subnorm1.total_cmp(subnorm1), std::strong_ordering::less);
  EXPECT_EQ(neg_subnorm1.total_cmp(norm1), std::strong_ordering::less);
  EXPECT_EQ(neg_subnorm1.total_cmp(inf), std::strong_ordering::less);
  EXPECT_EQ(neg_subnorm1.total_cmp(signaling_nan), std::strong_ordering::less);
  EXPECT_EQ(neg_subnorm1.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(neg_zero.total_cmp(neg_quiet_nan), std::strong_ordering::greater);
  EXPECT_EQ(neg_zero.total_cmp(neg_signaling_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(neg_zero.total_cmp(neg_inf), std::strong_ordering::greater);
  EXPECT_EQ(neg_zero.total_cmp(neg_norm1), std::strong_ordering::greater);
  EXPECT_EQ(neg_zero.total_cmp(neg_subnorm1), std::strong_ordering::greater);
  EXPECT_EQ(neg_zero.total_cmp(neg_zero), std::strong_ordering::equal);
  EXPECT_EQ(neg_zero.total_cmp(zero), std::strong_ordering::less);
  EXPECT_EQ(neg_zero.total_cmp(subnorm1), std::strong_ordering::less);
  EXPECT_EQ(neg_zero.total_cmp(norm1), std::strong_ordering::less);
  EXPECT_EQ(neg_zero.total_cmp(inf), std::strong_ordering::less);
  EXPECT_EQ(neg_zero.total_cmp(signaling_nan), std::strong_ordering::less);
  EXPECT_EQ(neg_zero.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(subnorm1.total_cmp(subnorm2), std::strong_ordering::less);

  EXPECT_EQ(subnorm1.total_cmp(neg_quiet_nan), std::strong_ordering::greater);
  EXPECT_EQ(subnorm1.total_cmp(neg_signaling_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(subnorm1.total_cmp(neg_inf), std::strong_ordering::greater);
  EXPECT_EQ(subnorm1.total_cmp(neg_norm1), std::strong_ordering::greater);
  EXPECT_EQ(subnorm1.total_cmp(neg_subnorm1), std::strong_ordering::greater);
  EXPECT_EQ(subnorm1.total_cmp(neg_zero), std::strong_ordering::greater);
  EXPECT_EQ(subnorm1.total_cmp(zero), std::strong_ordering::greater);
  EXPECT_EQ(subnorm1.total_cmp(subnorm1), std::strong_ordering::equal);
  EXPECT_EQ(subnorm1.total_cmp(norm1), std::strong_ordering::less);
  EXPECT_EQ(subnorm1.total_cmp(inf), std::strong_ordering::less);
  EXPECT_EQ(subnorm1.total_cmp(signaling_nan), std::strong_ordering::less);
  EXPECT_EQ(subnorm1.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(norm1.total_cmp(norm2), std::strong_ordering::less);

  EXPECT_EQ(norm1.total_cmp(neg_quiet_nan), std::strong_ordering::greater);
  EXPECT_EQ(norm1.total_cmp(neg_signaling_nan), std::strong_ordering::greater);
  EXPECT_EQ(norm1.total_cmp(neg_inf), std::strong_ordering::greater);
  EXPECT_EQ(norm1.total_cmp(neg_norm1), std::strong_ordering::greater);
  EXPECT_EQ(norm1.total_cmp(neg_subnorm1), std::strong_ordering::greater);
  EXPECT_EQ(norm1.total_cmp(neg_zero), std::strong_ordering::greater);
  EXPECT_EQ(norm1.total_cmp(zero), std::strong_ordering::greater);
  EXPECT_EQ(norm1.total_cmp(subnorm1), std::strong_ordering::greater);
  EXPECT_EQ(norm1.total_cmp(norm1), std::strong_ordering::equal);
  EXPECT_EQ(norm1.total_cmp(inf), std::strong_ordering::less);
  EXPECT_EQ(norm1.total_cmp(signaling_nan), std::strong_ordering::less);
  EXPECT_EQ(norm1.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(inf.total_cmp(neg_quiet_nan), std::strong_ordering::greater);
  EXPECT_EQ(inf.total_cmp(neg_signaling_nan), std::strong_ordering::greater);
  EXPECT_EQ(inf.total_cmp(neg_inf), std::strong_ordering::greater);
  EXPECT_EQ(inf.total_cmp(neg_norm1), std::strong_ordering::greater);
  EXPECT_EQ(inf.total_cmp(neg_subnorm1), std::strong_ordering::greater);
  EXPECT_EQ(inf.total_cmp(neg_zero), std::strong_ordering::greater);
  EXPECT_EQ(inf.total_cmp(zero), std::strong_ordering::greater);
  EXPECT_EQ(inf.total_cmp(subnorm1), std::strong_ordering::greater);
  EXPECT_EQ(inf.total_cmp(norm1), std::strong_ordering::greater);
  EXPECT_EQ(inf.total_cmp(inf), std::strong_ordering::equal);
  EXPECT_EQ(inf.total_cmp(signaling_nan), std::strong_ordering::less);
  EXPECT_EQ(inf.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(signaling_nan.total_cmp(signaling_nan2),
            std::strong_ordering::less);

  EXPECT_EQ(signaling_nan.total_cmp(neg_quiet_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(signaling_nan.total_cmp(neg_signaling_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(signaling_nan.total_cmp(neg_norm1), std::strong_ordering::greater);
  EXPECT_EQ(signaling_nan.total_cmp(neg_subnorm1),
            std::strong_ordering::greater);
  EXPECT_EQ(signaling_nan.total_cmp(neg_zero), std::strong_ordering::greater);
  EXPECT_EQ(signaling_nan.total_cmp(zero), std::strong_ordering::greater);
  EXPECT_EQ(signaling_nan.total_cmp(subnorm1), std::strong_ordering::greater);
  EXPECT_EQ(signaling_nan.total_cmp(norm1), std::strong_ordering::greater);
  EXPECT_EQ(signaling_nan.total_cmp(signaling_nan),
            std::strong_ordering::equal);
  EXPECT_EQ(signaling_nan.total_cmp(quiet_nan), std::strong_ordering::less);

  EXPECT_EQ(quiet_nan.total_cmp(quiet_nan2), std::strong_ordering::less);

  EXPECT_EQ(quiet_nan.total_cmp(neg_quiet_nan), std::strong_ordering::greater);
  EXPECT_EQ(quiet_nan.total_cmp(neg_signaling_nan),
            std::strong_ordering::greater);
  EXPECT_EQ(quiet_nan.total_cmp(neg_norm1), std::strong_ordering::greater);
  EXPECT_EQ(quiet_nan.total_cmp(neg_subnorm1), std::strong_ordering::greater);
  EXPECT_EQ(quiet_nan.total_cmp(neg_zero), std::strong_ordering::greater);
  EXPECT_EQ(quiet_nan.total_cmp(zero), std::strong_ordering::greater);
  EXPECT_EQ(quiet_nan.total_cmp(subnorm1), std::strong_ordering::greater);
  EXPECT_EQ(quiet_nan.total_cmp(norm1), std::strong_ordering::greater);
  EXPECT_EQ(quiet_nan.total_cmp(signaling_nan), std::strong_ordering::greater);
  EXPECT_EQ(quiet_nan.total_cmp(quiet_nan), std::strong_ordering::equal);
}

}  // namespace
