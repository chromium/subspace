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

#define F32_NEAR(a, b, c) \
  EXPECT_NEAR(a.primitive_value, b.primitive_value, c.primitive_value);

namespace {

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
  static_assert(1_f32 <= f32::TODO_NAN());
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

}  // namespace
