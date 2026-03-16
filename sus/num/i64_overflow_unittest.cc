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

#ifdef TEST_MODULE
import sus;
#else
#include "sus/prelude.h"
#endif

#include "googletest/include/gtest/gtest.h"
#include "sus/test/ensure_use.h"

namespace {

using sus::test::ensure_use;

TEST(i64OverflowDeathTest, Abs) { EXPECT_EQ(i64::MIN.abs(), i64::MIN); }

TEST(i64Overflow, AddOverflow) {
  EXPECT_EQ(i64::MAX + 1_i64, i64::MIN);
  EXPECT_EQ(i64::MIN + -1_i64, i64::MAX);

  EXPECT_EQ(1_i32 + i64::MAX, i64::MIN);
  EXPECT_EQ(i64::MAX + 1_i32, i64::MIN);

  auto i = i64::MAX;
  i += 1_i64;
  EXPECT_EQ(i, i64::MIN);
}

// Division overflow still panics.
TEST(i64OverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i64::MAX / 0_i64;
        ensure_use(&x);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = i64::MIN / -1_i64;
        ensure_use(&x);
      },
      "attempt to divide with overflow");

  auto x = i64::MIN;
  EXPECT_DEATH(x /= 0_i64, "attempt to divide by zero");
  EXPECT_DEATH(x /= -1_i64, "attempt to divide with overflow");
#endif
}

TEST(i64OverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i64::MAX.overflowing_div(0_i64);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(i64OverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i64::MAX.saturating_div(0_i64);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(i64OverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i64::MAX.wrapping_div(0_i64);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(i64Overflow, MulOverflow) {
  EXPECT_EQ(i64::MAX * 2_i64, -2);
  EXPECT_EQ(i64::MAX * -2_i64, 2);

  auto i = i64::MAX;
  i *= 2_i64;
  EXPECT_EQ(i, -2);
}

TEST(i64OverflowDeathTest, NegOverflow) {  //
  EXPECT_EQ(-i64::MIN, i64::MIN);
}

TEST(i64OverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i64::MAX % 0_i64;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = i64::MIN % -1_i64;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");

  auto x = i64::MIN;
  EXPECT_DEATH(x %= 0_i64, "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(x %= -1_i64, "attempt to calculate the remainder with overflow");
#endif
}

TEST(i64OverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i64::MAX.overflowing_rem(0_i64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i64OverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i64::MAX.wrapping_rem(0_i64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i64OverflowDeathTest, ShlOverflow) { EXPECT_EQ(1_i64 << 65_u32, 2); }

TEST(i64OverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(i64::MAX >> 65_u32, i64::MAX >> 1_u32);
}

TEST(i64Overflow, SubOverflow) {
  EXPECT_EQ(i64::MIN - 1_i64, i64::MAX);
  EXPECT_EQ(i64::MAX - -1_i64, i64::MIN);

  EXPECT_EQ(1_i32 - -i64::MAX, i64::MIN);
  EXPECT_EQ(i64::MIN - 1_i32, i64::MAX);

  auto i = u32::MIN;
  i -= 1_u32;
  EXPECT_EQ(i, u32::MAX);
}

TEST(i64OverflowDeathTest, PowOverflow) { EXPECT_EQ((i64::MAX).pow(2_u32), 1); }

TEST(i64OverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i64).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i64).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(i64OverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i64).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i64).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(i64OverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_i64).log(10_i64);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_i64).log(0_i64);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_i64).log(10_i64);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_i64).log(-2_i64);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}
TEST(i64OverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i64).div_euclid(0_i64);
        EXPECT_EQ(x, i64::MIN);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = (i64::MIN).div_euclid(-1_i64);
        EXPECT_EQ(x, i64::MIN);
      },
      "attempt to divide with overflow");
#endif
}

TEST(i64OverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i64).overflowing_div_euclid(0_i64);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(i64OverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i64).wrapping_div_euclid(0_i64);
        EXPECT_EQ(x, i64::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(i64OverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i64).rem_euclid(0_i64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = (i64::MIN).rem_euclid(-1_i64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");
#endif
}

TEST(i64OverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i64).overflowing_rem_euclid(0_i64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(i64OverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_i64).wrapping_rem_euclid(0_i64);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

}  // namespace
