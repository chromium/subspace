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

TEST(u16Overflow, AddOverflow) {
  EXPECT_EQ(u16::MAX + 1_u16, u16::MIN);

  EXPECT_EQ(1_u8 + u16::MAX, u16::MIN);
  EXPECT_EQ(u16::MAX + 1_u8, u16::MIN);

  auto i = u16::MAX;
  i += 1_u16;
  EXPECT_EQ(i, u16::MIN);
}

// Division overflow still panics.
TEST(u16OverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u16::MAX / 0_u16;
        ensure_use(&x);
      },
      "attempt to divide by zero");

  auto x = u16::MIN;
  EXPECT_DEATH(x /= 0_u16, "attempt to divide by zero");
#endif
}

TEST(u16OverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u16::MAX.overflowing_div(0_u16);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(u16OverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u16::MAX.saturating_div(0_u16);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(u16OverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u16::MAX.wrapping_div(0_u16);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(u16Overflow, MulOverflow) {
  EXPECT_EQ(u16::MAX * 2_u16, u16::MAX - 1_u16);

  auto i = u16::MAX;
  i *= 2_u16;
  EXPECT_EQ(i, u16::MAX - 1_u16);
}

TEST(u16OverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u16::MAX % 0_u16;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");

  auto x = u16::MIN;
  EXPECT_DEATH(x %= 0_u16,
               "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u16OverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u16::MAX.overflowing_rem(0_u16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u16OverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u16::MAX.wrapping_rem(0_u16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u16OverflowDeathTest, ShlOverflow) { EXPECT_EQ(1_u16 << 33_u16, 2_u16); }

TEST(u16OverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(u16::MAX >> 33_u16, u16::MAX >> 1_u16);
}

TEST(u16Overflow, SubOverflow) {
  EXPECT_EQ(u16::MIN - 1_u16, u16::MAX);

  EXPECT_EQ(1_u8 - 2_u16, u16::MAX);
  EXPECT_EQ(u16::MIN - 1_u8, u16::MAX);

  auto i = u16::MIN;
  i -= 1_u16;
  EXPECT_EQ(i, u16::MAX);
}

TEST(u16OverflowDeathTest, PowOverflow) {
  EXPECT_EQ((u16::MAX).pow(2_u16), 1u);
}

TEST(u16OverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u16).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(u16OverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u16).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(u16OverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u16).log(10_u16);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_u16).log(0_u16);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}
TEST(u16OverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u16).div_euclid(0_u16);
        EXPECT_EQ(x, u16::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(u16OverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u16).overflowing_div_euclid(0_u16);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(u16OverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u16).wrapping_div_euclid(0_u16);
        EXPECT_EQ(x, u16::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(u16OverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u16).rem_euclid(0_u16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u16OverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u16).overflowing_rem_euclid(0_u16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u16OverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u16).wrapping_rem_euclid(0_u16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u16OverflowDeathTest, DivCeilDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u16).div_ceil(0_u16);
        ensure_use(&x);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = u16::MAX.div_ceil(0_u16);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(u16OverflowDeathTest, NextMultipleOfDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u16).next_multiple_of(0_u16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = u16::MAX.next_multiple_of(0_u16);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif

  // Overflow occurs but is not checked.
  EXPECT_EQ(u16::MAX.next_multiple_of(2_u16), 0u);
  EXPECT_EQ(u16::MAX.next_multiple_of(3_u16), u16::MAX);
  EXPECT_EQ(u16::MAX.next_multiple_of(4_u16), 0u);
  EXPECT_EQ(u16::MAX.next_multiple_of(5_u16), u16::MAX);
}

}  // namespace
