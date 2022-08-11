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
