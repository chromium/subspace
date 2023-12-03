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

TEST(i16OverflowDeathTest, Abs) { EXPECT_EQ(i16::MIN.abs(), i16::MIN); }

TEST(i16Overflow, AddOverflow) {
  EXPECT_EQ(i16::MAX + 1_i16, i16::MIN);
  EXPECT_EQ(i16::MIN + -1_i16, i16::MAX);

  EXPECT_EQ(1_i8 + i16::MAX, i16::MIN);
  EXPECT_EQ(i16::MAX + 1_i8, i16::MIN);

  auto i = i16::MAX;
  i += 1_i16;
  EXPECT_EQ(i, i16::MIN);
}

// Division overflow still panics.
TEST(i16OverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i16::MAX / 0_i16;
        ensure_use(&x);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = i16::MIN / -1_i16;
        ensure_use(&x);
      },
      "attempt to divide with overflow");

  auto x = i16::MIN;
  EXPECT_DEATH(x /= 0_i16, "attempt to divide by zero");
  EXPECT_DEATH(x /= -1_i16, "attempt to divide with overflow");
#endif
}

TEST(i16OverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i16::MAX.overflowing_div(0_i16);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(i16OverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i16::MAX.saturating_div(0_i16);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(i16OverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i16::MAX.wrapping_div(0_i16);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(i16Overflow, MulOverflow) {
  EXPECT_EQ(i16::MAX * 2_i16, -2);
  EXPECT_EQ(i16::MAX * -2_i16, 2);

  auto i = i16::MAX;
  i *= 2_i16;
  EXPECT_EQ(i, -2);
}

TEST(i16OverflowDeathTest, NegOverflow) {  //
  EXPECT_EQ(-i16::MIN, i16::MIN);
}

TEST(i16OverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i16::MAX % 0_i16;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = i16::MIN % -1_i16;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");

  auto x = i16::MIN;
  EXPECT_DEATH(x %= 0_i16, "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(x %= -1_i16, "attempt to calculate the remainder with overflow");
#endif
}

TEST(i16OverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i16::MAX.overflowing_rem(0_i16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i16OverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i16::MAX.wrapping_rem(0_i16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i16OverflowDeathTest, ShlOverflow) { EXPECT_EQ(1_i16 << 33_u32, 2); }

TEST(i16OverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(i16::MAX >> 33_u32, i16::MAX >> 1_u32);
}

TEST(i16Overflow, SubOverflow) {
  EXPECT_EQ(i16::MIN - 1_i16, i16::MAX);
  EXPECT_EQ(i16::MAX - -1_i16, i16::MIN);

  EXPECT_EQ(1_i8 - -i16::MAX, i16::MIN);
  EXPECT_EQ(i16::MIN - 1_i8, i16::MAX);

  auto i = u32::MIN;
  i -= 1_u32;
  EXPECT_EQ(i, u32::MAX);
}

TEST(i16OverflowDeathTest, PowOverflow) { EXPECT_EQ((i16::MAX).pow(2_u32), 1); }

TEST(i16OverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i16).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i16).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(i16OverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i16).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i16).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(i16OverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i16).log(10_i16);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_i16).log(0_i16);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i16).log(10_i16);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_i16).log(-2_i16);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}
TEST(i16OverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i16).div_euclid(0_i16);
        EXPECT_EQ(x, i16::MIN);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = (i16::MIN).div_euclid(-1_i16);
        EXPECT_EQ(x, i16::MIN);
      },
      "attempt to divide with overflow");
#endif
}

TEST(i16OverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i16).overflowing_div_euclid(0_i16);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(i16OverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i16).wrapping_div_euclid(0_i16);
        EXPECT_EQ(x, i16::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(i16OverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i16).rem_euclid(0_i16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = (i16::MIN).rem_euclid(-1_i16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");
#endif
}

TEST(i16OverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i16).overflowing_rem_euclid(0_i16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i16OverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i16).wrapping_rem_euclid(0_i16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

}  // namespace
