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

#include <bit>
#include <limits>

#include "num/__private/intrinsics.h"
#include "num/float.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

#define F32_NEAR(a, b, c) \
  EXPECT_NEAR((a).primitive_value, (b).primitive_value, (c).primitive_value);

namespace {

using sus::num::FpCategory;

TEST(f32, Traits) {
  static_assert(sus::num::Neg<f32>);
  static_assert(sus::num::Add<f32, f32>);
  static_assert(sus::num::AddAssign<f32, f32>);
  static_assert(sus::num::Sub<f32, f32>);
  static_assert(sus::num::SubAssign<f32, f32>);
  static_assert(sus::num::Mul<f32, f32>);
  static_assert(sus::num::MulAssign<f32, f32>);
  static_assert(sus::num::Div<f32, f32>);
  static_assert(sus::num::DivAssign<f32, f32>);
  static_assert(sus::num::Rem<f32, f32>);
  static_assert(sus::num::RemAssign<f32, f32>);
  static_assert(!sus::num::BitAnd<f32, f32>);
  static_assert(!sus::num::BitAndAssign<f32, f32>);
  static_assert(!sus::num::BitOr<f32, f32>);
  static_assert(!sus::num::BitOrAssign<f32, f32>);
  static_assert(!sus::num::BitXor<f32, f32>);
  static_assert(!sus::num::BitXorAssign<f32, f32>);
  static_assert(!sus::num::BitNot<f32>);
  static_assert(!sus::num::Shl<f32>);
  static_assert(!sus::num::ShlAssign<f32>);
  static_assert(!sus::num::Shr<f32>);
  static_assert(!sus::num::ShrAssign<f32>);

  static_assert(!sus::num::Ord<f32, f32>);
  static_assert(!sus::num::WeakOrd<f32, f32>);
  static_assert(sus::num::PartialOrd<f32, f32>);
  static_assert(1_f32 >= 1_f32);
  static_assert(2_f32 > 1_f32);
  static_assert(1_f32 <= 1_f32);
  static_assert(1_f32 < 2_f32);
  static_assert(sus::num::Eq<f32, f32>);
  static_assert(1_f32 == 1_f32);
  static_assert(!(1_f32 == 2_f32));
  static_assert(1_f32 != 2_f32);
  static_assert(!(1_f32 != 1_f32));
  static_assert(f32::TODO_NAN() != f32::TODO_NAN());

  // Verify constexpr.
  constexpr f32 c = 1_f32 + 2_f32 - 3_f32 * 4_f32 / 5_f32 % 6_f32;
  constexpr std::partial_ordering o = 2_f32 <=> 3_f32;
}

TEST(f32, Consts) {
  {
    constexpr auto min = f32::MIN();
    static_assert(std::same_as<decltype(min), const f32>);
    EXPECT_EQ(min.primitive_value, -FLT_MAX);
    constexpr auto max = f32::MAX();
    static_assert(std::same_as<decltype(max), const f32>);
    EXPECT_EQ(max.primitive_value, FLT_MAX);
  }
  {
    constexpr auto min = f32::MIN_PRIMITIVE;
    static_assert(std::same_as<decltype(min), const float>);
    EXPECT_EQ(min, -FLT_MAX);
    constexpr auto max = f32::MAX_PRIMITIVE;
    static_assert(std::same_as<decltype(max), const float>);
    EXPECT_EQ(max, FLT_MAX);
  }

  EXPECT_EQ(f32::RADIX(), 2_u32);
  EXPECT_EQ(f32::MANTISSA_DIGITS(), 24_u32);
  EXPECT_EQ(f32::DIGITS(), 6_u32);
  EXPECT_EQ(f32::EPSILON(), f32(FLT_EPSILON));
  EXPECT_EQ(f32::MIN(), f32(-FLT_MAX));
  EXPECT_EQ(f32::MAX(), f32(FLT_MAX));
  EXPECT_EQ(f32::MIN_POSITIVE(), f32(FLT_MIN));
  EXPECT_EQ(f32::MIN_EXP(), -125_i32);
  EXPECT_EQ(f32::MAX_EXP(), 128_i32);
  EXPECT_EQ(f32::MIN_10_EXP(), -37_i32);
  EXPECT_EQ(f32::MAX_10_EXP(), 38_i32);
  EXPECT_TRUE(::isnan(f32::TODO_NAN().primitive_value));
  EXPECT_TRUE(::isinf(f32::TODO_INFINITY().primitive_value));
  EXPECT_GT(f32::TODO_INFINITY(), 0_f32);
  EXPECT_TRUE(::isinf(f32::NEG_INFINITY().primitive_value));
  EXPECT_LT(f32::NEG_INFINITY(), 0_f32);

  // Verify NaN in a constexpr context is the same as in a non-constexpr
  // context.
  constexpr auto n1 = f32::TODO_NAN().primitive_value;
  const auto n2 = f32::TODO_NAN().primitive_value;
  EXPECT_EQ(sus::num::__private::into_unsigned_integer(n1),
            sus::num::__private::into_unsigned_integer(n2));

  EXPECT_EQ(f32::consts::E(), 2.71828182845904523536028747135266250_f32);
  EXPECT_EQ(f32::consts::FRAC_1_PI(),
            0.318309886183790671537767526745028724_f32);
  EXPECT_EQ(f32::consts::FRAC_1_SQRT_2(),
            0.707106781186547524400844362104849039_f32);
  EXPECT_EQ(f32::consts::FRAC_2_PI(),
            0.636619772367581343075535053490057448_f32);
  EXPECT_EQ(f32::consts::FRAC_2_SQRT_PI(),
            1.12837916709551257389615890312154517_f32);
  EXPECT_EQ(f32::consts::FRAC_PI_2(),
            1.57079632679489661923132169163975144_f32);
  EXPECT_EQ(f32::consts::FRAC_PI_3(),
            1.04719755119659774615421446109316763_f32);
  EXPECT_EQ(f32::consts::FRAC_PI_4(),
            0.785398163397448309615660845819875721_f32);
  EXPECT_EQ(f32::consts::FRAC_PI_6(),
            0.52359877559829887307710723054658381_f32);
  EXPECT_EQ(f32::consts::FRAC_PI_8(),
            0.39269908169872415480783042290993786_f32);
  EXPECT_EQ(f32::consts::LN_2(), 0.693147180559945309417232121458176568_f32);
  EXPECT_EQ(f32::consts::LN_10(), 2.30258509299404568401799145468436421_f32);
  EXPECT_EQ(f32::consts::LOG2_10(), 3.32192809488736234787031942948939018_f32);
  EXPECT_EQ(f32::consts::LOG2_E(), 1.44269504088896340735992468100189214_f32);
  EXPECT_EQ(f32::consts::LOG10_2(), 0.301029995663981195213738894724493027_f32);
  EXPECT_EQ(f32::consts::LOG10_E(), 0.434294481903251827651128918916605082_f32);
  EXPECT_EQ(f32::consts::PI(), 3.14159265358979323846264338327950288_f32);
  EXPECT_EQ(f32::consts::SQRT_2(), 1.41421356237309504880168872420969808_f32);
  EXPECT_EQ(f32::consts::TAU(), 6.28318530717958647692528676655900577_f32);
}

TEST(f32, Literals) {
  static_assert((0._f32).primitive_value == 0.f);
  static_assert((0.0_f32).primitive_value == 0.f);
  static_assert((1.234_f32).primitive_value == 1.234f);
  static_assert((-1.234_f32).primitive_value == -1.234f);

  // Whole numbers.
  static_assert((0_f32).primitive_value == 0.f);
  static_assert((1_f32).primitive_value == 1.f);
  static_assert((-5_f32).primitive_value == -5.f);
}

TEST(f32, ConstructPrimitive) {
  auto a = f32();
  EXPECT_EQ(a.primitive_value, 0.f);

  f32 b;
  EXPECT_EQ(b.primitive_value, 0.f);

  auto c = f32(1.2f);
  EXPECT_EQ(c.primitive_value, 1.2f);
}

TEST(f32, AssignPrimitive) {
  auto a = f32();
  EXPECT_EQ(a.primitive_value, 0.f);
  a = 1.2f;
  EXPECT_EQ(a.primitive_value, 1.2f);
}

TEST(f32, Negate) {
  constexpr auto a = -(0.345_f32);
  EXPECT_EQ(a, f32(-0.345f));

  auto b = 0.345_f32;
  EXPECT_EQ(-b, f32(-0.345f));
}

TEST(f32, BinaryOperators) {
  {
    constexpr auto a = 1_f32 + 0.345_f32;
    EXPECT_EQ(a, f32(1.345f));

    auto b = 1_f32;
    b += 0.345_f32;
    EXPECT_EQ(b, f32(1.345f));
  }
  {
    constexpr auto a = 1_f32 - 0.345_f32;
    EXPECT_EQ(a, f32(0.655f));

    auto b = 1_f32;
    b -= 0.345_f32;
    EXPECT_EQ(b, f32(0.655f));
  }
  {
    constexpr auto a = 2_f32 * 0.345_f32;
    EXPECT_EQ(a, f32(0.690f));

    auto b = 2_f32;
    b *= 0.345_f32;
    EXPECT_EQ(b, f32(0.690f));
  }
  {
    constexpr auto a = 0.690_f32 / 2_f32;
    EXPECT_EQ(a, f32(0.345f));

    auto b = 0.690_f32;
    b /= 2_f32;
    EXPECT_EQ(b, f32(0.345f));
  }
  {
    constexpr auto a = 2.345_f32 % 2_f32;
    F32_NEAR(a, f32(0.345f), 0.00001_f32);

    constexpr auto b = 2.4_f32 % 1.1_f32;
    F32_NEAR(b, f32(0.2f), 0.00001_f32);

    auto c = 2.345_f32;
    c %= 2_f32;
    F32_NEAR(c, f32(0.345f), 0.00001_f32);

    auto d = 2.4_f32;
    d %= 1.1_f32;
    F32_NEAR(d, f32(0.2f), 0.00001_f32);
  }
}

TEST(f32, TotalCmp) {
  // TODO: Use classify on f32.

  const auto quiet_nan = std::bit_cast<f32>(uint32_t{0x7fc00000});
  const auto signaling_nan = std::bit_cast<f32>(uint32_t{0x7f800001});
  EXPECT_EQ(::fpclassify(quiet_nan.primitive_value), FP_NAN);
  EXPECT_EQ(::fpclassify(signaling_nan.primitive_value), FP_NAN);
  EXPECT_TRUE(
      sus::num::__private::float_is_nan_quiet(quiet_nan.primitive_value));
  EXPECT_FALSE(
      sus::num::__private::float_is_nan_quiet(signaling_nan.primitive_value));

  const auto quiet_nan2 = std::bit_cast<f32>(uint32_t{0x7fc00001});
  const auto signaling_nan2 = std::bit_cast<f32>(uint32_t{0x7f800002});
  EXPECT_EQ(::fpclassify(quiet_nan2.primitive_value), FP_NAN);
  EXPECT_EQ(::fpclassify(signaling_nan2.primitive_value), FP_NAN);
  EXPECT_TRUE(
      sus::num::__private::float_is_nan_quiet(quiet_nan2.primitive_value));
  EXPECT_FALSE(
      sus::num::__private::float_is_nan_quiet(signaling_nan2.primitive_value));

  const auto neg_quiet_nan = std::bit_cast<f32>(uint32_t{0xffc00000});
  const auto neg_signaling_nan = std::bit_cast<f32>(uint32_t{0xff800001});
  EXPECT_EQ(::fpclassify(neg_quiet_nan.primitive_value), FP_NAN);
  EXPECT_EQ(::fpclassify(neg_signaling_nan.primitive_value), FP_NAN);
  EXPECT_TRUE(
      sus::num::__private::float_is_nan_quiet(neg_quiet_nan.primitive_value));
  EXPECT_FALSE(sus::num::__private::float_is_nan_quiet(
      neg_signaling_nan.primitive_value));

  const auto neg_quiet_nan2 = std::bit_cast<f32>(uint32_t{0xffc00001});
  const auto neg_signaling_nan2 = std::bit_cast<f32>(uint32_t{0xff800002});
  EXPECT_EQ(::fpclassify(neg_quiet_nan2.primitive_value), FP_NAN);
  EXPECT_EQ(::fpclassify(neg_signaling_nan2.primitive_value), FP_NAN);
  EXPECT_TRUE(
      sus::num::__private::float_is_nan_quiet(neg_quiet_nan2.primitive_value));
  EXPECT_FALSE(sus::num::__private::float_is_nan_quiet(
      neg_signaling_nan2.primitive_value));

  const auto inf = f32::TODO_INFINITY();
  const auto neg_inf = f32::NEG_INFINITY();
  EXPECT_EQ(::fpclassify(inf.primitive_value), FP_INFINITE);
  EXPECT_EQ(::fpclassify(neg_inf.primitive_value), FP_INFINITE);

  const auto norm1 = 123_f32;
  const auto norm2 = 234_f32;
  EXPECT_EQ(::fpclassify(norm1.primitive_value), FP_NORMAL);
  EXPECT_EQ(::fpclassify(norm2.primitive_value), FP_NORMAL);
  constexpr auto neg_norm1 = -123_f32;
  constexpr auto neg_norm2 = -234_f32;
  EXPECT_EQ(::fpclassify(neg_norm1.primitive_value), FP_NORMAL);
  EXPECT_EQ(::fpclassify(neg_norm2.primitive_value), FP_NORMAL);

  const auto subnorm1 =
      f32(std::numeric_limits<decltype(f32::primitive_value)>::denorm_min());
  const auto subnorm2 = subnorm1 * 2_f32;
  EXPECT_NE(subnorm1.primitive_value, subnorm2.primitive_value);
  EXPECT_EQ(::fpclassify(subnorm1.primitive_value), FP_SUBNORMAL);
  EXPECT_EQ(::fpclassify(subnorm2.primitive_value), FP_SUBNORMAL);
  const auto neg_subnorm1 = -subnorm1;
  const auto neg_subnorm2 = -subnorm2;
  EXPECT_EQ(::fpclassify(neg_subnorm1.primitive_value), FP_SUBNORMAL);
  EXPECT_EQ(::fpclassify(neg_subnorm2.primitive_value), FP_SUBNORMAL);

  const auto zero = 0_f32;
  const auto neg_zero = -0_f32;
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

TEST(f32, Abs) {
  auto a = (-0.345_f32).abs();
  EXPECT_EQ(a, 0.345_f32);

  auto b = 0.345_f32;
  EXPECT_EQ(b.abs(), 0.345_f32);
}

TEST(f32, Acos) {
  auto a = (0.767_f32).acos();
  F32_NEAR(a, 0.696643798_f32, 0.0000001_f32);
  auto b = (1_f32).acos();
  F32_NEAR(b, 0_f32, 0.0000001_f32);
  auto c = (1.1_f32).acos();
  EXPECT_TRUE(::isnan(c.primitive_value));  // TODO: is_nan().
  auto d = (-1.1_f32).acos();
  EXPECT_TRUE(::isnan(d.primitive_value));  // TODO: is_nan().
}

TEST(f32, Acosh) {
  auto a = (2.5_f32).acosh();
  F32_NEAR(a, 1.566799236972411_f32, 0.0000001_f32);
  auto b = (1_f32).acosh();
  F32_NEAR(b, 0_f32, 0.0000001_f32);
  auto c = (0.9999999_f32).acosh();
  EXPECT_TRUE(::isnan(c.primitive_value));  // TODO: is_nan().
  auto d = (0_f32).acosh();
  EXPECT_TRUE(::isnan(d.primitive_value));  // TODO: is_nan().
  auto e = (-0.9999999_f32).acosh();
  EXPECT_TRUE(::isnan(e.primitive_value));  // TODO: is_nan().
}

TEST(f32, Asin) {
  auto a = (0.767_f32).asin();
  F32_NEAR(a, 0.874152528_f32, 0.0000001_f32);
  auto b = (0_f32).asin();
  F32_NEAR(b, 0_f32, 0.0000001_f32);
  auto c = (1.1_f32).asin();
  EXPECT_TRUE(::isnan(c.primitive_value));  // TODO: is_nan().
  auto d = (-1.1_f32).asin();
  EXPECT_TRUE(::isnan(d.primitive_value));  // TODO: is_nan().
}

TEST(f32, Asinh) {
  auto a = (2.5_f32).asinh();
  F32_NEAR(a, 1.6472311463711_f32, 0.0000001_f32);
  auto b = (0_f32).asinh();
  F32_NEAR(b, 0_f32, 0.0000001_f32);
  auto c = (0.9999999_f32).asinh();
  F32_NEAR(c, 0.88137351630886_f32, 0.0000001_f32);
}

TEST(f32, Atan) {
  auto a = (0.767_f32).atan();
  F32_NEAR(a, 0.654292628_f32, 0.0000001_f32);
  auto b = (0_f32).atan();
  F32_NEAR(b, 0_f32, 0.0000001_f32);
  auto c = (1.1_f32).atan();
  F32_NEAR(c, 0.832981267_f32, 0.0000001_f32);
  auto d = (-1.1_f32).atan();
  F32_NEAR(d, -0.832981267_f32, 0.0000001_f32);
}

TEST(f32, Atan2) {
  auto a = (0_f32).atan2(0_f32);
  F32_NEAR(a, 0_f32, 0.0000001_f32);
  auto b = (0.5_f32).atan2(1.2_f32);
  F32_NEAR(b, 0.39479112_f32, 0.0000001_f32);
  auto c = (-0.5_f32).atan2(1.2_f32);
  F32_NEAR(c, -0.39479112_f32, 0.0000001_f32);
  auto d = (-0.5_f32).atan2(-1.2_f32);
  F32_NEAR(d, 0.39479112_f32 - f32::consts::PI(), 0.0000001_f32);
  auto e = (0.5_f32).atan2(-1.2_f32);
  F32_NEAR(e, -0.39479112_f32 + f32::consts::PI(), 0.0000001_f32);
}

TEST(f32, Atanh) {
  auto a = (2.5_f32).atanh();
  EXPECT_TRUE(::isnan(a.primitive_value));  // TODO: is_nan().
  auto b = (0_f32).atanh();
  F32_NEAR(b, 0_f32, 0.0000001_f32);
  auto c = (0.75_f32).atanh();
  F32_NEAR(c, 0.97295507452766_f32, 0.0000001_f32);
  auto d = (1_f32).atanh();
  EXPECT_TRUE(::isinf(d.primitive_value));  // TODO: is_infinite().
}

TEST(f32, Cbrt) {
  auto a = (0.456_f32).cbrt();
  F32_NEAR(a, 0.76970022625_f32, 0.0000001_f32);
  auto b = (1_f32).cbrt();
  F32_NEAR(b, 1_f32, 0.0000001_f32);
  auto c = (-1_f32).cbrt();
  F32_NEAR(c, -1_f32, 0.0000001_f32);
}

TEST(f32, Ceil) {
  auto a = (0.456_f32).ceil();
  F32_NEAR(a, 1_f32, 0.0000001_f32);
  auto b = (-0.456_f32).ceil();
  EXPECT_EQ(b.total_cmp(-0_f32), std::strong_ordering::equal);
  auto c = (1.0001_f32).ceil();
  F32_NEAR(c, 2_f32, 0.0000001_f32);
}

TEST(f32, Copysign) {
  auto a = (0.456_f32).copysign(1_f32);
  EXPECT_EQ(a, 0.456_f32);
  auto b = (0.456_f32).copysign(-1_f32);
  EXPECT_EQ(b, -0.456_f32);
  auto c = f32::TODO_NAN().copysign(-1_f32);
  EXPECT_TRUE(::isnan(c.primitive_value));    // TODO: is_nan()
  EXPECT_TRUE(::signbit(c.primitive_value));  // TODO: is_positive()
  auto d = f32::TODO_NAN().copysign(1_f32);
  EXPECT_TRUE(::isnan(d.primitive_value));     // TODO: is_nan()
  EXPECT_TRUE(!::signbit(d.primitive_value));  // TODO: is_positive()
}

TEST(f32, Cos) {
  auto a = (0.767_f32).cos();
  F32_NEAR(a, 0.71999584159_f32, 0.0000001_f32);
  auto b = (1_f32).cos();
  F32_NEAR(b, 0.54030230586_f32, 0.0000001_f32);
  auto c = (4_f32).cos();
  F32_NEAR(c, -0.65364362086_f32, 0.0000001_f32);
}

TEST(f32, Cosh) {
  auto a = (0.767_f32).cosh();
  F32_NEAR(a, 1.30885042871_f32, 0.0000001_f32);
  auto b = (1_f32).cosh();
  F32_NEAR(b, 1.54308063482_f32, 0.0000001_f32);
  auto c = (4_f32).cosh();
  F32_NEAR(c, 27.308232836_f32, 0.0000001_f32);
}

TEST(f32, Exp) {
  auto a = (1_f32).exp();
  F32_NEAR(a, f32::consts::E(), 0.0000001_f32);
  auto b = (2.4_f32).exp();
  F32_NEAR(b, 11.0231763806_f32, 0.00001_f32);
}

TEST(f32, Exp2) {
  auto a = (1_f32).exp2();
  F32_NEAR(a, 2_f32, 0.0000001_f32);
  auto b = (2.4_f32).exp2();
  F32_NEAR(b, 5.27803164309_f32, 0.00001_f32);
}

TEST(f32, ExpM1) {
  auto a = (1_f32).exp_m1();
  F32_NEAR(a, f32::consts::E() - 1_f32, 0.00001_f32);
  auto b = (2.4_f32).exp_m1();
  F32_NEAR(b, 10.0231763806_f32, 0.00001_f32);
}

TEST(f32, Floor) {
  auto a = (0.456_f32).floor();
  EXPECT_EQ(a.total_cmp(0_f32), std::strong_ordering::equal);
  auto b = (-0.456_f32).floor();
  F32_NEAR(b, -1_f32, 0.0000001_f32);
  auto c = (1.0001_f32).floor();
  F32_NEAR(c, 1_f32, 0.0000001_f32);
}

TEST(f32, Hypot) {
  auto a = (0.456_f32).hypot(0.567_f32);
  F32_NEAR(a, 0.72761597013_f32, 0.0000001_f32);
}

TEST(f32, Ln) {
  auto a = (0.456_f32).ln();
  F32_NEAR(a, -0.78526246946_f32, 0.0000001_f32);
}

TEST(f32, Ln1p) {
  auto a = (0.456_f32).ln_1p();
  F32_NEAR(a, 0.37569294977_f32, 0.0000001_f32);
}

TEST(f32, Log10) {
  auto a = (0.456_f32).log10();
  F32_NEAR(a, -0.34103515733_f32, 0.0000001_f32);
}

TEST(f32, Log2) {
  auto a = (0.456_f32).log2();
  F32_NEAR(a, -1.1328942705_f32, 0.0000001_f32);
}

TEST(f32, Max) {
  auto a = (0.456_f32).max(-0.456_f32);
  EXPECT_EQ(a, 0.456_f32);
  auto b = (0.456_f32).max(0.457_f32);
  EXPECT_EQ(b, 0.457_f32);
  auto c = f32::TODO_NAN().max(0.457_f32);
  EXPECT_EQ(c, 0.457_f32);
  auto d = (0.456_f32).max(f32::TODO_NAN());
  EXPECT_EQ(d, 0.456_f32);
}

TEST(f32, Min) {
  auto a = (0.456_f32).min(-0.456_f32);
  EXPECT_EQ(a, -0.456_f32);
  auto b = (0.456_f32).min(0.457_f32);
  EXPECT_EQ(b, 0.456_f32);
  auto c = f32::TODO_NAN().min(0.457_f32);
  EXPECT_EQ(c, 0.457_f32);
  auto d = (0.456_f32).min(f32::TODO_NAN());
  EXPECT_EQ(d, 0.456_f32);
}

TEST(f32, MulAdd) {
  auto a = (0.456_f32).mul_add(2_f32, 3.1_f32);
  F32_NEAR(a, 0.456_f32 * 2_f32 + 3.1_f32, 0.0000001_f32);
}

TEST(f32, Powf) {
  auto a = (0.456_f32).powf(4.6_f32);
  F32_NEAR(a, 0.02699219956_f32, 0.0000001_f32);
}

TEST(f32, Powi) {
  auto a = (0.456_f32).powi(5_i32);
  F32_NEAR(a, 0.01971624532_f32, 0.0000001_f32);
}

TEST(f32, Recip) {
  auto a = (0.456_f32).recip();
  F32_NEAR(a, 2.19298245614_f32, 0.0000001_f32);
  auto b = f32::TODO_NAN().recip();
  EXPECT_TRUE(::isnan(b.primitive_value));  // TODO: is_nan()
}

TEST(f32, Round) {
  auto a = (0.456_f32).round();
  EXPECT_EQ(a.total_cmp(0_f32), std::strong_ordering::equal);
  auto b = (-0.456_f32).round();
  EXPECT_EQ(a.total_cmp(0_f32), std::strong_ordering::equal);
  auto c = (1.546_f32).round();
  F32_NEAR(c, 2_f32, 0.0000001_f32);
  auto d = (-1.546_f32).round();
  F32_NEAR(d, -2_f32, 0.0000001_f32);
}

TEST(f32, Signum) {
  EXPECT_EQ((0_f32).signum(), 1_f32);
  EXPECT_EQ((-0_f32).signum(), -1_f32);
  EXPECT_EQ((123_f32).signum(), 1_f32);
  EXPECT_EQ((-123_f32).signum(), -1_f32);
  EXPECT_EQ(f32::TODO_INFINITY().signum(), 1_f32);
  EXPECT_EQ(f32::NEG_INFINITY().signum(), -1_f32);
  EXPECT_TRUE(
      ::isnan(f32::TODO_NAN().signum().primitive_value));  // TODO: is_nan()
}

TEST(f32, Sin) {
  auto a = (0.767_f32).sin();
  F32_NEAR(a, 0.69397837724_f32, 0.0000001_f32);
  auto b = (1_f32).sin();
  F32_NEAR(b, 0.8414709848_f32, 0.0000001_f32);
  auto c = (4_f32).sin();
  F32_NEAR(c, -0.7568024953_f32, 0.0000001_f32);
}

TEST(f32, Sinh) {
  auto a = (0.767_f32).sinh();
  F32_NEAR(a, 0.84444623555_f32, 0.0000001_f32);
  auto b = (1_f32).sinh();
  F32_NEAR(b, 1.17520119364_f32, 0.0000001_f32);
  auto c = (4_f32).sinh();
  F32_NEAR(c, 27.2899171971_f32, 0.0000001_f32);
}

TEST(f32, Sqrt) {
  auto a = (4.68_f32).sqrt();
  F32_NEAR(a, 2.16333076528_f32, 0.0000001_f32);
}

TEST(f32, Tan) {
  auto a = (0.767_f32).tan();
  F32_NEAR(a, 0.96386442413_f32, 0.0000001_f32);
  auto b = (1_f32).tan();
  F32_NEAR(b, 1.55740772465_f32, 0.0000001_f32);
  auto c = (4_f32).tan();
  F32_NEAR(c, 1.15782128235_f32, 0.0000001_f32);
}

TEST(f32, Tanh) {
  auto a = (0.767_f32).tanh();
  F32_NEAR(a, 0.64518161665_f32, 0.0000001_f32);
  auto b = (1_f32).tanh();
  F32_NEAR(b, 0.76159415595_f32, 0.0000001_f32);
  auto c = (4_f32).tanh();
  F32_NEAR(c, 0.99932929973_f32, 0.0000001_f32);
}

TEST(f32, Fract) {
  auto a = (3.767_f32).fract();
  F32_NEAR(a, 0.767_f32, 0.0000001_f32);
  auto b = (1_f32).fract();
  F32_NEAR(b, 0_f32, 0.0000001_f32);
  auto c = (0.12345_f32).fract();
  F32_NEAR(c, 0.12345_f32, 0.0000001_f32);
  auto d = (-3.767_f32).fract();
  F32_NEAR(d, -0.767_f32, 0.0000001_f32);
}

TEST(f32, Trunc) {
  auto a = (3.767_f32).trunc();
  EXPECT_EQ(a, 3_f32);
  auto b = (1_f32).trunc();
  EXPECT_EQ(b, 1_f32);
  auto c = (0.12345_f32).trunc();
  EXPECT_EQ(c, 0_f32);
  auto d = (-3.767_f32).trunc();
  EXPECT_EQ(d, -3_f32);
}

TEST(f32, ToDegrees) {
  auto a = (3.4567_f32).to_degrees();
  F32_NEAR(a, 198.054321_f32, 0.0000001_f32);
}

TEST(f32, ToRadians) {
  auto a = (198.054321_f32).to_radians();
  F32_NEAR(a, 3.4567_f32, 0.0000001_f32);
}

TEST(f32, ToIntUnchecked) {
  auto a = (198.054321_f32).to_int_unchecked<u8>(unsafe_fn);
  EXPECT_EQ(a, 198_u8);
  auto b = (198.054321_f32).to_int_unchecked<u32>(unsafe_fn);
  EXPECT_EQ(b, 198_u32);
  auto c = (-108.054321_f32).to_int_unchecked<i8>(unsafe_fn);
  EXPECT_EQ(c, -108_i8);
}

TEST(f32, FromBits) {
  auto a = f32::from_bits(0x41480000_u32);
  EXPECT_EQ(a, 12.5_f32);
}

TEST(f32, ToBits) {
  auto a = (12.5_f32).to_bits();
  EXPECT_EQ(a, 0x41480000_u32);
}

TEST(f32, Classify) {
  EXPECT_EQ(f32::TODO_NAN().classify(), FpCategory::Nan);
  EXPECT_EQ(f32::TODO_INFINITY().classify(), FpCategory::Infinite);
  EXPECT_EQ(f32::NEG_INFINITY().classify(), FpCategory::Infinite);
  EXPECT_EQ((0_f32).classify(), FpCategory::Zero);
  EXPECT_EQ((-0_f32).classify(), FpCategory::Zero);
  EXPECT_EQ(
      f32(std::numeric_limits<decltype(f32::primitive_value)>::denorm_min())
          .classify(),
      FpCategory::Subnormal);
  EXPECT_EQ((123_f32).classify(), FpCategory::Normal);

  constexpr auto a = f32::TODO_NAN().classify();
  EXPECT_EQ(a, FpCategory::Nan);
  constexpr auto b = f32::TODO_INFINITY().classify();
  EXPECT_EQ(b, FpCategory::Infinite);
  constexpr auto c = f32::NEG_INFINITY().classify();
  EXPECT_EQ(c, FpCategory::Infinite);
  constexpr auto d = (0_f32).classify();
  EXPECT_EQ(d, FpCategory::Zero);
  constexpr auto e = (-0_f32).classify();
  EXPECT_EQ(e, FpCategory::Zero);
  constexpr auto f =
      f32(std::numeric_limits<decltype(f32::primitive_value)>::denorm_min())
          .classify();
  EXPECT_EQ(f, FpCategory::Subnormal);
  constexpr auto g = (123_f32).classify();
  EXPECT_EQ(g, FpCategory::Normal);
}

}  // namespace
