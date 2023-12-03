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

TEST(u64Overflow, AddOverflow) {
  EXPECT_EQ(u64::MAX + 1_u64, u64::MIN);

  EXPECT_EQ(1_u32 + u64::MAX, u64::MIN);
  EXPECT_EQ(u64::MAX + 1_u32, u64::MIN);

  auto i = u64::MAX;
  i += 1_u64;
  EXPECT_EQ(i, u64::MIN);
}

// Division overflow still panics.
TEST(u64OverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u64::MAX / 0_u64;
        ensure_use(&x);
      },
      "attempt to divide by zero");

  auto x = u64::MIN;
  EXPECT_DEATH(x /= 0_u64, "attempt to divide by zero");
#endif
}

TEST(u64OverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u64::MAX.overflowing_div(0_u64);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(u64OverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u64::MAX.saturating_div(0_u64);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(u64OverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u64::MAX.wrapping_div(0_u64);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(u64Overflow, MulOverflow) {
  EXPECT_EQ(u64::MAX * 2_u64, u64::MAX - 1_u64);

  auto i = u64::MAX;
  i *= 2_u64;
  EXPECT_EQ(i, u64::MAX - 1_u64);
}

TEST(u64OverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u64::MAX % 0_u64;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");

  auto x = u64::MIN;
  EXPECT_DEATH(x %= 0_u64,
               "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u64OverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u64::MAX.overflowing_rem(0_u64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u64OverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u64::MAX.wrapping_rem(0_u64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u64OverflowDeathTest, ShlOverflow) { EXPECT_EQ(1_u64 << 65_u64, 2_u64); }

TEST(u64OverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(u64::MAX >> 65_u64, u64::MAX >> 1_u64);
}

TEST(u64Overflow, SubOverflow) {
  EXPECT_EQ(u64::MIN - 1_u64, u64::MAX);

  EXPECT_EQ(1_u32 - 2_u64, u64::MAX);
  EXPECT_EQ(u64::MIN - 1_u32, u64::MAX);

  auto i = u64::MIN;
  i -= 1_u64;
  EXPECT_EQ(i, u64::MAX);
}

TEST(u64OverflowDeathTest, PowOverflow) {
  EXPECT_EQ((u64::MAX).pow(2_u32), 1u);
}

TEST(u64OverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u64).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(u64OverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u64).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(u64OverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u64).log(10_u64);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_u64).log(0_u64);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}
TEST(u64OverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u64).div_euclid(0_u64);
        EXPECT_EQ(x, u64::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(u64OverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u64).overflowing_div_euclid(0_u64);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(u64OverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u64).wrapping_div_euclid(0_u64);
        EXPECT_EQ(x, u64::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(u64OverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u64).rem_euclid(0_u64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u64OverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u64).overflowing_rem_euclid(0_u64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u64OverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u64).wrapping_rem_euclid(0_u64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(u64OverflowDeathTest, DivCeilDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u64).div_ceil(0_u64);
        ensure_use(&x);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = u64::MAX.div_ceil(0_u64);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

}  // namespace
