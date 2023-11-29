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

TEST(usizeOverflow, AddOverflow) {
  EXPECT_EQ(usize::MAX + 1_usize, usize::MIN);

  EXPECT_EQ(1_u16 + usize::MAX, usize::MIN);
  EXPECT_EQ(usize::MAX + 1_u16, usize::MIN);

  auto i = usize::MAX;
  i += 1_usize;
  EXPECT_EQ(i, usize::MIN);
}

// Division overflow still panics.
TEST(usizeOverflowDeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = usize::MAX / 0_usize;
        ensure_use(&x);
      },
      "");

  auto x = usize::MIN;
  EXPECT_DEATH(x /= 0_usize, "");
#endif
}

TEST(usizeOverflowDeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = usize::MAX.overflowing_div(0_usize);
        ensure_use(&x);
      },
      "");

#endif
}

TEST(usizeOverflowDeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = usize::MAX.saturating_div(0_usize);
        ensure_use(&x);
      },
      "");

#endif
}

TEST(usizeOverflowDeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = usize::MAX.wrapping_div(0_usize);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(usizeOverflow, MulOverflow) {
  EXPECT_EQ(usize::MAX * 2_usize, usize::MAX - 1_usize);

  auto i = usize::MAX;
  i *= 2_usize;
  EXPECT_EQ(i, usize::MAX - 1_usize);
}

TEST(usizeOverflowDeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = usize::MAX % 0_usize;
        ensure_use(&x);
      },
      "");

  auto x = usize::MIN;
  EXPECT_DEATH(x %= 0_usize, "");
#endif
}

TEST(usizeOverflowDeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = usize::MAX.overflowing_rem(0_usize);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(usizeOverflowDeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = usize::MAX.wrapping_rem(0_usize);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(usizeOverflowDeathTest, ShlOverflow) {
  EXPECT_EQ(1_usize << 65_usize, 2_usize);
}

TEST(usizeOverflowDeathTest, ShrOverflow) {
  EXPECT_EQ(usize::MAX >> 65_usize, usize::MAX >> 1_usize);
}

TEST(usizeOverflow, SubOverflow) {
  EXPECT_EQ(usize::MIN - 1_usize, usize::MAX);

  EXPECT_EQ(1_u16 - 2_usize, usize::MAX);
  EXPECT_EQ(usize::MIN - 1_u16, usize::MAX);

  auto i = usize::MIN;
  i -= 1_usize;
  EXPECT_EQ(i, usize::MAX);
}

TEST(usizeOverflowDeathTest, PowOverflow) {
  EXPECT_EQ((usize::MAX).pow(2_u32), 1u);
}

TEST(usizeOverflowDeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_usize).log2();
        ensure_use(&x);
      },
      "");
#endif
}

TEST(usizeOverflowDeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_usize).log10();
        ensure_use(&x);
      },
      "");
#endif
}

TEST(usizeOverflowDeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (0_usize).log(10_usize);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = (2_usize).log(0_usize);
        ensure_use(&x);
      },
      "");
#endif
}
TEST(usizeOverflowDeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_usize).div_euclid(0_usize);
        EXPECT_EQ(x, usize::MIN);
      },
      "");
#endif
}

TEST(usizeOverflowDeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_usize).overflowing_div_euclid(0_usize);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(usizeOverflowDeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_usize).wrapping_div_euclid(0_usize);
        EXPECT_EQ(x, usize::MIN);
      },
      "");
#endif
}

TEST(usizeOverflowDeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_usize).rem_euclid(0_usize);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(usizeOverflowDeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_usize).overflowing_rem_euclid(0_usize);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(usizeOverflowDeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = (7_usize).wrapping_rem_euclid(0_usize);
        ensure_use(&x);
      },
      "");
#endif
}

}  // namespace