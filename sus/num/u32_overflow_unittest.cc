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

TEST(u32Overflow, AddOverflow) {
  EXPECT_EQ(u32::MAX + 1_u32, u32::MIN);

  EXPECT_EQ(1_u16 + u32::MAX, u32::MIN);
  EXPECT_EQ(u32::MAX + 1_u16, u32::MIN);

  auto i = u32::MAX;
  i += 1_u32;
  EXPECT_EQ(i, u32::MIN);
}

// Division overflow still panics.
TEST(u32OverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u32::MAX / 0_u32;
        ensure_use(&x);
      },
      "attempt to divide by zero");

  auto x = u32::MIN;
  EXPECT_DEATH(x /= 0_u32, "attempt to divide by zero");
#endif
}

TEST(u32OverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u32::MAX.overflowing_div(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(u32OverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u32::MAX.saturating_div(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(u32OverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u32::MAX.wrapping_div(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(u32Overflow, MulOverflow) {
  EXPECT_EQ(u32::MAX * 2_u32, u32::MAX - 1_u32);

  auto i = u32::MAX;
  i *= 2_u32;
  EXPECT_EQ(i, u32::MAX - 1_u32);
}

TEST(u32OverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u32::MAX % 0_u32;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");

  auto x = u32::MIN;
  EXPECT_DEATH(x %= 0_u32,
               "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u32OverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u32::MAX.overflowing_rem(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u32OverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u32::MAX.wrapping_rem(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u32OverflowDeathTest, ShlOverflow) { EXPECT_EQ(1_u32 << 33_u32, 2_u32); }

TEST(u32OverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(u32::MAX >> 33_u32, u32::MAX >> 1_u32);
}

TEST(u32Overflow, SubOverflow) {
  EXPECT_EQ(u32::MIN - 1_u32, u32::MAX);

  EXPECT_EQ(1_u16 - 2_u32, u32::MAX);
  EXPECT_EQ(u32::MIN - 1_u16, u32::MAX);

  auto i = u32::MIN;
  i -= 1_u32;
  EXPECT_EQ(i, u32::MAX);
}

TEST(u32OverflowDeathTest, PowOverflow) {
  EXPECT_EQ((u32::MAX).pow(2_u32), 1u);
}

TEST(u32OverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u32).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(u32OverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u32).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(u32OverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u32).log(10_u32);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_u32).log(0_u32);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}
TEST(u32OverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u32).div_euclid(0_u32);
        EXPECT_EQ(x, u32::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(u32OverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u32).overflowing_div_euclid(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(u32OverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u32).wrapping_div_euclid(0_u32);
        EXPECT_EQ(x, u32::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(u32OverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u32).rem_euclid(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u32OverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u32).overflowing_rem_euclid(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u32OverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u32).wrapping_rem_euclid(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u32Overlow, NextPowerOfTwoOutOfBounds) {
  EXPECT_EQ((u32::MAX - 1_u32).next_power_of_two(), 0_u32);
  EXPECT_EQ(u32::MAX.next_power_of_two(), 0_u32);
}

TEST(u32OverflowDeathTest, DivCeilDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u32).div_ceil(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = u32::MAX.div_ceil(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(u32OverflowDeathTest, NextMultipleOfDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u32).next_multiple_of(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = u32::MAX.next_multiple_of(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif

  // Overflow occurs but is not checked.
  EXPECT_EQ(u32::MAX.next_multiple_of(2_u32), 0u);
  EXPECT_EQ(u32::MAX.next_multiple_of(3_u32), u32::MAX);
  EXPECT_EQ(u32::MAX.next_multiple_of(4_u32), 0u);
  EXPECT_EQ(u32::MAX.next_multiple_of(5_u32), u32::MAX);
}

}  // namespace
