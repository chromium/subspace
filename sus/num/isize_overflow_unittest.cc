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

TEST(isizeOverflowDeathTest, Abs) { EXPECT_EQ(isize::MIN.abs(), isize::MIN); }

TEST(isizeOverflow, AddOverflow) {
  EXPECT_EQ(isize::MAX + 1_isize, isize::MIN);
  EXPECT_EQ(isize::MIN + -1_isize, isize::MAX);

  EXPECT_EQ(1_i16 + isize::MAX, isize::MIN);
  EXPECT_EQ(isize::MAX + 1_i16, isize::MIN);

  auto i = isize::MAX;
  i += 1_isize;
  EXPECT_EQ(i, isize::MIN);
}

// Division overflow still panics.
TEST(isizeOverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = isize::MAX / 0_isize;
        ensure_use(&x);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = isize::MIN / -1_isize;
        ensure_use(&x);
      },
      "attempt to divide with overflow");

  auto x = isize::MIN;
  EXPECT_DEATH(x /= 0_isize, "attempt to divide by zero");
  EXPECT_DEATH(x /= -1_isize, "attempt to divide with overflow");
#endif
}

TEST(isizeOverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = isize::MAX.overflowing_div(0_isize);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(isizeOverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = isize::MAX.saturating_div(0_isize);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(isizeOverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = isize::MAX.wrapping_div(0_isize);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(isizeOverflow, MulOverflow) {
  EXPECT_EQ(isize::MAX * 2_isize, -2);
  EXPECT_EQ(isize::MAX * -2_isize, 2);

  auto i = isize::MAX;
  i *= 2_isize;
  EXPECT_EQ(i, -2);
}

TEST(isizeOverflowDeathTest, NegOverflow) {  //
  EXPECT_EQ(-isize::MIN, isize::MIN);
}

TEST(isizeOverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = isize::MAX % 0_isize;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = isize::MIN % -1_isize;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");

  auto x = isize::MIN;
  EXPECT_DEATH(x %= 0_isize, "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(x %= -1_isize, "attempt to calculate the remainder with overflow");
#endif
}

TEST(isizeOverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = isize::MAX.overflowing_rem(0_isize);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(isizeOverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = isize::MAX.wrapping_rem(0_isize);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(isizeOverflowDeathTest, ShlOverflow) { EXPECT_EQ(1_isize << 65_u32, 2); }

TEST(isizeOverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(isize::MAX >> 65_u32, isize::MAX >> 1_u32);
}

TEST(isizeOverflow, SubOverflow) {
  EXPECT_EQ(isize::MIN - 1_isize, isize::MAX);
  EXPECT_EQ(isize::MAX - -1_isize, isize::MIN);

  EXPECT_EQ(1_i16 - -isize::MAX, isize::MIN);
  EXPECT_EQ(isize::MIN - 1_i16, isize::MAX);

  auto i = u32::MIN;
  i -= 1_u32;
  EXPECT_EQ(i, u32::MAX);
}

TEST(isizeOverflowDeathTest, PowOverflow) { EXPECT_EQ((isize::MAX).pow(2_u32), 1); }

TEST(isizeOverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_isize).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_isize).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(isizeOverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_isize).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_isize).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(isizeOverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_isize).log(10_isize);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_isize).log(0_isize);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (-1_isize).log(10_isize);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (2_isize).log(-2_isize);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}
TEST(isizeOverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_isize).div_euclid(0_isize);
        EXPECT_EQ(x, isize::MIN);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = (isize::MIN).div_euclid(-1_isize);
        EXPECT_EQ(x, isize::MIN);
      },
      "attempt to divide with overflow");
#endif
}

TEST(isizeOverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_isize).overflowing_div_euclid(0_isize);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(isizeOverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_isize).wrapping_div_euclid(0_isize);
        EXPECT_EQ(x, isize::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(isizeOverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_isize).rem_euclid(0_isize);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = (isize::MIN).rem_euclid(-1_isize);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with overflow");
#endif
}

TEST(isizeOverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_isize).overflowing_rem_euclid(0_isize);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(isizeOverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_isize).wrapping_rem_euclid(0_isize);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

}  // namespace
