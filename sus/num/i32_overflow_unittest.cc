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

TEST(i32OverflowDeathTest, Abs) { EXPECT_EQ(i32::MIN.abs(), i32::MIN); }

TEST(i32Overflow, AddOverflow) {
  EXPECT_EQ(i32::MAX + 1_i32, i32::MIN);
  EXPECT_EQ(i32::MIN + -1_i32, i32::MAX);

  EXPECT_EQ(1_i16 + i32::MAX, i32::MIN);
  EXPECT_EQ(i32::MAX + 1_i16, i32::MIN);

  auto i = i32::MAX;
  i += 1_i32;
  EXPECT_EQ(i, i32::MIN);
}

// Division overflow still panics.
TEST(i32OverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i32::MAX / 0_i32;
        ensure_use(&x);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = i32::MIN / -1_i32;
        ensure_use(&x);
      },
      "attempt to divide with overflow");

  auto x = i32::MIN;
  EXPECT_DEATH(x /= 0_i32, "attempt to divide by zero");
  EXPECT_DEATH(x /= -1_i32, "attempt to divide with overflow");
#endif
}

TEST(i32OverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i32::MAX.overflowing_div(0_i32);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(i32OverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i32::MAX.saturating_div(0_i32);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(i32OverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i32::MAX.wrapping_div(0_i32);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(i32Overflow, MulOverflow) {
  EXPECT_EQ(i32::MAX * 2_i32, -2);
  EXPECT_EQ(i32::MAX * -2_i32, 2);

  auto i = i32::MAX;
  i *= 2_i32;
  EXPECT_EQ(i, -2);
}

TEST(i32OverflowDeathTest, NegOverflow) {  //
  EXPECT_EQ(-i32::MIN, i32::MIN);
}

TEST(i32OverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i32::MAX % 0_i32;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = i32::MIN % -1_i32;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");

  auto x = i32::MIN;
  EXPECT_DEATH(x %= 0_i32,
               "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(x %= -1_i32, "attempt to calculate the remainder with overflow");
#endif
}

TEST(i32OverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i32::MAX.overflowing_rem(0_i32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i32OverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i32::MAX.wrapping_rem(0_i32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i32OverflowDeathTest, ShlOverflow) { EXPECT_EQ(1_i32 << 33_u32, 2); }

TEST(i32OverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(i32::MAX >> 33_u32, i32::MAX >> 1_u32);
}

TEST(i32Overflow, SubOverflow) {
  EXPECT_EQ(i32::MIN - 1_i32, i32::MAX);
  EXPECT_EQ(i32::MAX - -1_i32, i32::MIN);

  EXPECT_EQ(1_i16 - -i32::MAX, i32::MIN);
  EXPECT_EQ(i32::MIN - 1_i16, i32::MAX);

  auto i = u32::MIN;
  i -= 1_u32;
  EXPECT_EQ(i, u32::MAX);
}

TEST(i32OverflowDeathTest, PowOverflow) { EXPECT_EQ((i32::MAX).pow(2_u32), 1); }

TEST(i32OverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i32).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i32).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(i32OverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i32).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i32).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(i32OverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i32).log(10_i32);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_i32).log(0_i32);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i32).log(10_i32);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_i32).log(-2_i32);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}
TEST(i32OverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i32).div_euclid(0_i32);
        EXPECT_EQ(x, i32::MIN);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = (i32::MIN).div_euclid(-1_i32);
        EXPECT_EQ(x, i32::MIN);
      },
      "attempt to divide with overflow");
#endif
}

TEST(i32OverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i32).overflowing_div_euclid(0_i32);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(i32OverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i32).wrapping_div_euclid(0_i32);
        EXPECT_EQ(x, i32::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(i32OverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i32).rem_euclid(0_i32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = (i32::MIN).rem_euclid(-1_i32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");
#endif
}

TEST(i32OverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i32).overflowing_rem_euclid(0_i32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i32OverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i32).wrapping_rem_euclid(0_i32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

}  // namespace
