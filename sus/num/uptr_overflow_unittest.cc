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

TEST(uptrOverflow, AddOverflow) {
  EXPECT_EQ(uptr::MAX_BIT_PATTERN + 1_u32, uptr::MIN);

  EXPECT_EQ(1_u16 + uptr::MAX_BIT_PATTERN, uptr::MIN);
  EXPECT_EQ(uptr::MAX_BIT_PATTERN + 1_u16, uptr::MIN);

  auto i = uptr::MAX_BIT_PATTERN;
  i += 1_u32;
  EXPECT_EQ(i, uptr::MIN);
}

// Division overflow still panics.
TEST(uptrOverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = uptr::MAX_BIT_PATTERN / 0_u32;
        ensure_use(&x);
      },
      "attempt to divide by zero");

  auto x = uptr::MIN;
  EXPECT_DEATH(x /= 0_u32, "attempt to divide by zero");
#endif
}

TEST(uptrOverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = uptr::MAX_BIT_PATTERN.overflowing_div(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(uptrOverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = uptr::MAX_BIT_PATTERN.saturating_div(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");

#endif
}

TEST(uptrOverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = uptr::MAX_BIT_PATTERN.wrapping_div(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(uptrOverflow, MulOverflow) {
  EXPECT_EQ(uptr::MAX_BIT_PATTERN * 2_u32, uptr::MAX_BIT_PATTERN - 1_u32);

  auto i = uptr::MAX_BIT_PATTERN;
  i *= 2_u32;
  EXPECT_EQ(i, uptr::MAX_BIT_PATTERN - 1_u32);
}

TEST(uptrOverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = uptr::MAX_BIT_PATTERN % 0_u32;
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");

  auto x = uptr::MIN;
  EXPECT_DEATH(x %= 0_u32,
               "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(uptrOverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = uptr::MAX_BIT_PATTERN.overflowing_rem(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(uptrOverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = uptr::MAX_BIT_PATTERN.wrapping_rem(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(uptrOverflowDeathTest, ShlOverflow) {
  EXPECT_EQ(uptr().with_addr(1_usize) << 65_u32, 2_u32);
}

TEST(uptrOverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(uptr::MAX_BIT_PATTERN >> 65_u32, uptr::MAX_BIT_PATTERN >> 1_u32);
}

TEST(uptrOverflow, SubOverflow) {
  EXPECT_EQ(uptr::MIN - 1_u32, uptr::MAX_BIT_PATTERN);

  EXPECT_EQ(1_u16 - uptr().with_addr(2_u32), uptr::MAX_BIT_PATTERN);
  EXPECT_EQ(uptr::MIN - 1_u16, uptr::MAX_BIT_PATTERN);

  auto i = uptr::MIN;
  i -= 1_u32;
  EXPECT_EQ(i, uptr::MAX_BIT_PATTERN);
}

TEST(uptrOverflowDeathTest, PowOverflow) {
  EXPECT_EQ((uptr::MAX_BIT_PATTERN).pow(2_u32), 1u);
}

TEST(uptrOverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (uptr()).log2();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(uptrOverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (uptr()).log10();
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}

TEST(uptrOverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (uptr()).log(10_u32);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
  EXPECT_DEATH(
      {
        auto x = (uptr().with_addr(2_usize)).log(0_u32);
        ensure_use(&x);
      },
      "argument of integer logarithm must be positive");
#endif
}
TEST(uptrOverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (uptr().with_addr(7_usize)).div_euclid(0_u32);
        EXPECT_EQ(x, uptr::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(uptrOverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (uptr().with_addr(7_usize)).overflowing_div_euclid(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(uptrOverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (uptr().with_addr(7_usize)).wrapping_div_euclid(0_u32);
        EXPECT_EQ(x, uptr::MIN);
      },
      "attempt to divide by zero");
#endif
}

TEST(uptrOverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (uptr().with_addr(7_usize)).rem_euclid(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(uptrOverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (uptr().with_addr(7_usize)).overflowing_rem_euclid(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(uptrOverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (uptr().with_addr(7_usize)).wrapping_rem_euclid(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif
}

TEST(uptrOverflowDeathTest, DivCeilDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = uptr().with_addr(0_usize).div_ceil(0_u32);
        ensure_use(&x);
      },
      "attempt to divide by zero");
  EXPECT_DEATH(
      {
        auto x = uptr::MAX_BIT_PATTERN.div_ceil(0_u64);
        ensure_use(&x);
      },
      "attempt to divide by zero");
#endif
}

TEST(uptrOverflowDeathTest, NextMultipleOfDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = uptr().with_addr(0_usize).next_multiple_of(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
  EXPECT_DEATH(
      {
        auto x = uptr::MAX_BIT_PATTERN.next_multiple_of(0_u32);
        ensure_use(&x);
      },
      "attempt to calculate the remainder with a divisor of zero");
#endif

  // Overflow occurs but is not checked.
  EXPECT_EQ(uptr::MAX_BIT_PATTERN.next_multiple_of(2_u32), 0u);
  EXPECT_EQ(uptr::MAX_BIT_PATTERN.next_multiple_of(3_u32),
            uptr::MAX_BIT_PATTERN);
  EXPECT_EQ(uptr::MAX_BIT_PATTERN.next_multiple_of(4_u32), 0u);
  EXPECT_EQ(uptr::MAX_BIT_PATTERN.next_multiple_of(5_u32),
            uptr::MAX_BIT_PATTERN);
}

}  // namespace
