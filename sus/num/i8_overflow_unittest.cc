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

#include "googletest/include/gtest/gtest.h"
#include "sus/prelude.h"
#include "sus/test/ensure_use.h"

namespace {

using sus::test::ensure_use;

TEST(i8OverflowDeathTest, Abs) { EXPECT_EQ(i8::MIN.abs(), i8::MIN); }

TEST(i8Overflow, AddOverflow) {
  EXPECT_EQ(i8::MAX + 1_i8, i8::MIN);
  EXPECT_EQ(i8::MIN + -1_i8, i8::MAX);
}

// Division overflow still panics.
TEST(i8OverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i8::MAX / 0_i8;
        ensure_use(&x);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = i8::MIN / -1_i8;
        ensure_use(&x);
      },
      "attempt to divide with overflow");

  auto x = i8::MIN;
  EXPECT_DEATH(x /= 0_i8, "attempt to divide by zero");
  EXPECT_DEATH(x /= -1_i8, "attempt to divide with overflow");
#endif
}

TEST(i8OverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i8::MAX.overflowing_div(0_i8);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(i8OverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i8::MAX.saturating_div(0_i8);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(i8OverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i8::MAX.wrapping_div(0_i8);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(i8Overflow, MulOverflow) {
  EXPECT_EQ(i8::MAX * 2_i8, -2_i8);
  EXPECT_EQ(i8::MAX * -2_i8, 2_i8);
}

TEST(i8OverflowDeathTest, NegOverflow) {  //
  EXPECT_EQ(-i8::MIN, i8::MIN);
}

TEST(i8OverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i8::MAX % 0_i8;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = i8::MIN % -1_i8;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");

  auto x = i8::MIN;
  EXPECT_DEATH(x %= 0_i8, "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(x %= -1_i8, "attempt to calculate the remainder with overflow");
#endif
}

TEST(i8OverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i8::MAX.overflowing_rem(0_i8);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i8OverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i8::MAX.wrapping_rem(0_i8);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i8OverflowDeathTest, ShlOverflow) { EXPECT_EQ(1_i8 << 33_u32, 2_i8); }

TEST(i8OverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(i8::MAX >> 33_u32, i8::MAX >> 1_u32);
}

TEST(i8Overflow, SubOverflow) {
  EXPECT_EQ(i8::MIN - 1_i8, i8::MAX);
  EXPECT_EQ(i8::MAX - -1_i8, i8::MIN);
}

TEST(i8OverflowDeathTest, PowOverflow) {
  EXPECT_EQ((i8::MAX).pow(2_u32), 1_i8);
}

TEST(i8OverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i8).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i8).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(i8OverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i8).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i8).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(i8OverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i8).log(10_i8);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_i8).log(0_i8);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i8).log(10_i8);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_i8).log(-2_i8);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}
TEST(i8OverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i8).div_euclid(0_i8);
        EXPECT_EQ(x, i8::MIN);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = (i8::MIN).div_euclid(-1_i8);
        EXPECT_EQ(x, i8::MIN);
      },
      "attempt to divide with overflow");
#endif
}

TEST(i8OverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i8).overflowing_div_euclid(0_i8);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(i8OverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i8).wrapping_div_euclid(0_i8);
        EXPECT_EQ(x, i8::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(i8OverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i8).rem_euclid(0_i8);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = (i8::MIN).rem_euclid(-1_i8);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");
#endif
}

TEST(i8OverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i8).overflowing_rem_euclid(0_i8);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i8OverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i8).wrapping_rem_euclid(0_i8);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

}  // namespace
