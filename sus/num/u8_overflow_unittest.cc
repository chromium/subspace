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

TEST(u8Overflow, AddOverflow) { EXPECT_EQ(u8::MAX + 1_u8, u8::MIN); }

// Division overflow still panics.
TEST(u8OverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u8::MAX / 0_u8;
        ensure_use(&x);
      },
      "");

  auto x = u8::MIN;
  EXPECT_DEATH(x /= 0_u8, "");
#endif
}

TEST(u8OverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u8::MAX.overflowing_div(0_u8);
        ensure_use(&x);
      },
      "");

#endif
}

TEST(u8OverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u8::MAX.saturating_div(0_u8);
        ensure_use(&x);
      },
      "");

#endif
}

TEST(u8OverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u8::MAX.wrapping_div(0_u8);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(u8Overflow, MulOverflow) { EXPECT_EQ(u8::MAX * 2_u8, u8::MAX - 1_u8); }

TEST(u8OverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u8::MAX % 0_u8;
        ensure_use(&x);
      },
      "");

  auto x = u8::MIN;
  EXPECT_DEATH(x %= 0_u8, "");
#endif
}

TEST(u8OverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u8::MAX.overflowing_rem(0_u8);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(u8OverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u8::MAX.wrapping_rem(0_u8);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(u8OverflowDeathTest, ShlOverflow) { EXPECT_EQ(1_u8 << 33_u32, 2_u8); }

TEST(u8OverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(u8::MAX >> 33_u32, u8::MAX >> 1_u32);
}

TEST(u8Overflow, SubOverflow) { EXPECT_EQ(u8::MIN - 1_u8, u8::MAX); }

TEST(u8OverflowDeathTest, PowOverflow) {
  EXPECT_EQ((u8::MAX).pow(2_u32), 1_u8);
}

TEST(u8OverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u8).log2();
        ensure_use(&x);
      },
      "");
#endif
}

TEST(u8OverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u8).log10();
        ensure_use(&x);
      },
      "");
#endif
}

TEST(u8OverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_u8).log(10_u8);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = (2_u8).log(0_u8);
        ensure_use(&x);
      },
      "");
#endif
}
TEST(u8OverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u8).div_euclid(0_u8);
        EXPECT_EQ(x, u8::MIN);
      },
      "");
#endif
}

TEST(u8OverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u8).overflowing_div_euclid(0_u8);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(u8OverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u8).wrapping_div_euclid(0_u8);
        EXPECT_EQ(x, u8::MIN);
      },
      "");
#endif
}

TEST(u8OverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u8).rem_euclid(0_u8);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(u8OverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u8).overflowing_rem_euclid(0_u8);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(u8OverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_u8).wrapping_rem_euclid(0_u8);
        ensure_use(&x);
      },
      "");
#endif
}

}  // namespace
