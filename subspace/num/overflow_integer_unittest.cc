// Copyright 2023 Google LLC
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

#include "subspace/num/overflow_integer.h"

#include <concepts>

#include "googletest/include/gtest/gtest.h"
#include "subspace/containers/array.h"
#include "subspace/num/signed_integer.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/prelude.h"

namespace {

using sus::num::OverflowInteger;

TEST(OverflowInteger, CopyCloneMove) {
  {
    static_assert(::sus::mem::Copy<OverflowInteger<i32>>);
    auto a = OverflowInteger<i32>::with(4);
    auto b = a;
    static_assert(std::same_as<decltype(a), decltype(b)>);
    EXPECT_EQ(a, b);
  }
  {
    static_assert(::sus::mem::Clone<OverflowInteger<i32>>);
    auto a = OverflowInteger<i32>::with(4);
    auto b = sus::clone(a);
    static_assert(std::same_as<decltype(a), decltype(b)>);
    EXPECT_EQ(a, b);
  }
  {
    static_assert(::sus::mem::Move<OverflowInteger<i32>>);
    auto a = OverflowInteger<i32>::with(4);
    auto b = sus::move(a);
    static_assert(std::same_as<decltype(a), decltype(b)>);
    EXPECT_EQ(a, b);
  }
}

TEST(OverflowInteger, Default) {
  static_assert(::sus::construct::Default<OverflowInteger<i32>>);
  EXPECT_EQ(OverflowInteger<i32>().unwrap(), 0);

  static_assert(::sus::construct::Default<OverflowInteger<u32>>);
  EXPECT_EQ(OverflowInteger<u32>().unwrap(), 0u);
}

TEST(OverflowInteger, With) {
  EXPECT_EQ(OverflowInteger<i32>::with(13_i8).unwrap(), 13);
  EXPECT_EQ(OverflowInteger<i32>::with(13_i16).unwrap(), 13);
  EXPECT_EQ(OverflowInteger<i32>::with(13_i32).unwrap(), 13);
  EXPECT_EQ(OverflowInteger<i32>::with(13_u8).unwrap(), 13);
  EXPECT_EQ(OverflowInteger<i32>::with(13_u16).unwrap(), 13);

  EXPECT_EQ(OverflowInteger<u32>::with(13_u8).unwrap(), 13u);
  EXPECT_EQ(OverflowInteger<u32>::with(13_u16).unwrap(), 13u);
  EXPECT_EQ(OverflowInteger<u32>::with(13_u32).unwrap(), 13u);
}

TEST(OverflowInteger, From) {
  static_assert(::sus::construct::Into<i8, OverflowInteger<i32>>);
  static_assert(::sus::construct::Into<i16, OverflowInteger<i32>>);
  static_assert(::sus::construct::Into<i32, OverflowInteger<i32>>);
  static_assert(::sus::construct::Into<i64, OverflowInteger<i32>>);
  static_assert(::sus::construct::Into<u8, OverflowInteger<i32>>);
  static_assert(::sus::construct::Into<u16, OverflowInteger<i32>>);
  static_assert(::sus::construct::Into<u32, OverflowInteger<i32>>);
  static_assert(::sus::construct::Into<u64, OverflowInteger<i32>>);

  EXPECT_EQ(OverflowInteger<i32>::from(13_u64).unwrap(), 13);
  EXPECT_EQ(OverflowInteger<i32>(sus::into(13_u64)).unwrap(), 13);
}

TEST(OverflowInteger, TryFrom) {
  static_assert(::sus::construct::TryFrom<OverflowInteger<i32>, u16>);
  static_assert(::sus::construct::TryFrom<OverflowInteger<i32>, i64>);

  EXPECT_EQ(OverflowInteger<i32>::try_from(13_u64).unwrap().unwrap(), 13);
  EXPECT_EQ(OverflowInteger<i32>::try_from(u64::MAX).unwrap_err(),
            sus::num::TryFromIntError::with_out_of_bounds());
}

TEST(OverflowInteger, FromProduct) {
  // To OverflowInteger with overflow.
  {
    auto a = sus::Array<i32, 2>::with(2, i32::MAX);
    decltype(auto) o =
        sus::move(a).into_iter().product<sus::num::OverflowInteger<i32>>();
    static_assert(std::same_as<decltype(o), sus::num::OverflowInteger<i32>>);
    EXPECT_EQ(o.to_option(), sus::None);
  }
  // To OverflowInteger without overflow.
  {
    auto a = sus::Array<i32, 2>::with(2, 4);
    decltype(auto) o =
        sus::move(a).into_iter().product<sus::num::OverflowInteger<i32>>();
    static_assert(std::same_as<decltype(o), sus::num::OverflowInteger<i32>>);
    EXPECT_EQ(o.to_option().unwrap(), 2 * 4);
  }
}

TEST(OverflowInteger, IsValid) {
  EXPECT_EQ((OverflowInteger<i32>::with(i32::MAX)).is_valid(), true);
  EXPECT_EQ((OverflowInteger<i32>::with(i32::MAX) + 1).is_valid(), false);
  EXPECT_EQ((OverflowInteger<i32>::with(i32::MIN)).is_valid(), true);
  EXPECT_EQ((OverflowInteger<i32>::with(i32::MIN) - 1).is_valid(), false);
}

TEST(OverflowInteger, IsOverflow) {
  EXPECT_EQ((OverflowInteger<i32>::with(i32::MAX)).is_overflow(), false);
  EXPECT_EQ((OverflowInteger<i32>::with(i32::MAX) + 1).is_overflow(), true);
  EXPECT_EQ((OverflowInteger<i32>::with(i32::MIN)).is_overflow(), false);
  EXPECT_EQ((OverflowInteger<i32>::with(i32::MIN) - 1).is_overflow(), true);
}

TEST(OverflowInteger, AsValue) {
  {
    auto lvalue = OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.as_value(), i32::MAX);
    static_assert(std::same_as<decltype(lvalue.as_value()), i32>);
    EXPECT_EQ(OverflowInteger<i32>::with(i32::MAX).as_value(), i32::MAX);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.as_value_unchecked(unsafe_fn), i32::MAX);
    EXPECT_EQ(OverflowInteger<i32>::with(i32::MAX).as_value(), i32::MAX);
    EXPECT_EQ(
        OverflowInteger<i32>::with(i32::MAX).as_value_unchecked(unsafe_fn),
        i32::MAX);
  }
}

TEST(OverflowIntegerDeathTest, AsValueOverflow) {
#if GTEST_HAS_DEATH_TEST
  auto o = OverflowInteger<i32>::with(i32::MAX) + 1;
  EXPECT_DEATH({ [[maybe_unused]] auto v = o.as_value(); }, "");
#endif
}

TEST(OverflowInteger, AsValueMut) {
  {
    auto lvalue = OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.as_value_mut(), i32::MAX);
    lvalue.as_value_mut() -= 1;
    static_assert(std::same_as<decltype(lvalue.as_value_mut()), i32&>);
    EXPECT_EQ(lvalue.as_value(), i32::MAX - 1);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.as_value_unchecked_mut(unsafe_fn), i32::MAX);
    lvalue.as_value_unchecked_mut(unsafe_fn) -= 1;
    EXPECT_EQ(lvalue.as_value(), i32::MAX - 1);
  }
}

TEST(OverflowIntegerDeathTest, AsValueMutOverflow) {
#if GTEST_HAS_DEATH_TEST
  auto o = OverflowInteger<i32>::with(i32::MAX) + 1;
  EXPECT_DEATH({ [[maybe_unused]] auto v = o.as_value_mut(); }, "");
#endif
}

TEST(OverflowInteger, Unwrap) {
  EXPECT_EQ(OverflowInteger<i32>::with(i32::MAX).unwrap(), i32::MAX);
  static_assert(
      std::same_as<decltype(OverflowInteger<i32>::with(i32::MAX).unwrap()),
                   i32>);
  EXPECT_EQ(OverflowInteger<i32>::with(i32::MAX).unwrap_unchecked(unsafe_fn),
            i32::MAX);
}

TEST(OverflowInteger, ToOption) {
  auto lvalue = OverflowInteger<i32>::with(i32::MAX);
  EXPECT_EQ(lvalue.to_option(), sus::some(i32::MAX));
  lvalue += 1;
  EXPECT_EQ(lvalue.to_option(), sus::none());

  EXPECT_EQ(OverflowInteger<i32>::with(i32::MAX).to_option(),
            sus::some(i32::MAX));
  EXPECT_EQ((OverflowInteger<i32>::with(i32::MAX) + 1).to_option(),
            sus::none());
}

TEST(OverflowInteger, MathAssignFromInt) {
  {
    auto lvalue = OverflowInteger<i32>::with(0);
    lvalue += 3;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 3);
    lvalue += i32::MAX;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(0);
    lvalue -= 3;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), -3);
    lvalue -= i32::MAX;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(2);
    lvalue *= 3;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 6);
    lvalue *= i32::MAX;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(8);
    lvalue /= 2;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 4);
    lvalue /= 0;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(6);
    lvalue %= 4;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 2);
    lvalue %= 0;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
}

TEST(OverflowInteger, MathAssignFromSelf) {
  {
    auto lvalue = OverflowInteger<i32>::with(0);
    lvalue += OverflowInteger<i32>::with(3);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 3);
    lvalue += OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(0);
    lvalue -= OverflowInteger<i32>::with(3);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), -3);
    lvalue -= OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(2);
    lvalue *= OverflowInteger<i32>::with(3);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 6);
    lvalue *= OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(8);
    lvalue /= OverflowInteger<i32>::with(2);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 4);
    lvalue /= OverflowInteger<i32>::with(0);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(6);
    lvalue %= OverflowInteger<i32>::with(4);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 2);
    lvalue %= OverflowInteger<i32>::with(0);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
}

TEST(OverflowInteger, MathIntSelf) {
  {
    auto lvalue = 1 + OverflowInteger<i32>::with(3);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 4);
    lvalue = lvalue.as_value() + OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = 1 - OverflowInteger<i32>::with(3);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), -2);
    lvalue = lvalue.as_value() - OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = 2 * OverflowInteger<i32>::with(3);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 6);
    lvalue = lvalue.as_value() * OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = 8 / OverflowInteger<i32>::with(2);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 4);
    lvalue = lvalue.as_value() / OverflowInteger<i32>::with(0);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = 6 % OverflowInteger<i32>::with(4);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 2);
    lvalue = lvalue.as_value() % OverflowInteger<i32>::with(0);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
}

TEST(OverflowInteger, MathSelfInt) {
  {
    auto lvalue = OverflowInteger<i32>::with(0) + 3;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 3);
    lvalue = lvalue + i32::MAX;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(0) - 3;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), -3);
    lvalue = lvalue - i32::MAX;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(2) * 3;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 6);
    lvalue = lvalue * i32::MAX;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(8) / 2;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 4);
    lvalue = lvalue / 0;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(6) % 4;
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 2);
    lvalue = lvalue % 0;
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
}

TEST(OverflowInteger, MathSelfSelf) {
  {
    auto lvalue = OverflowInteger<i32>::with(1) + OverflowInteger<i32>::with(3);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 4);
    lvalue = lvalue + OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(1) - OverflowInteger<i32>::with(3);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), -2);
    lvalue = lvalue - OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(2) * OverflowInteger<i32>::with(3);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 6);
    lvalue = lvalue * OverflowInteger<i32>::with(i32::MAX);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(8) / OverflowInteger<i32>::with(2);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 4);
    lvalue = lvalue / OverflowInteger<i32>::with(0);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
  {
    auto lvalue = OverflowInteger<i32>::with(6) % OverflowInteger<i32>::with(4);
    EXPECT_EQ(lvalue.is_overflow(), false);
    EXPECT_EQ(lvalue.as_value(), 2);
    lvalue = lvalue % OverflowInteger<i32>::with(0);
    EXPECT_EQ(lvalue.is_overflow(), true);
  }
}

TEST(OverflowInteger, Eq) {
  static_assert(::sus::ops::Eq<OverflowInteger<i32>, i32>);
  static_assert(::sus::ops::Eq<i32, OverflowInteger<i32>>);
  static_assert(::sus::ops::Eq<OverflowInteger<i32>>);

  EXPECT_EQ(OverflowInteger<i32>::with(5), 5_i32);
  EXPECT_EQ(5_i32, OverflowInteger<i32>::with(5));
  EXPECT_EQ(OverflowInteger<i32>::with(5), OverflowInteger<i32>::with(5));
  EXPECT_NE(OverflowInteger<i32>::with(5), 4_i32);
  EXPECT_NE(4_i32, OverflowInteger<i32>::with(5));
  EXPECT_NE(OverflowInteger<i32>::with(5), OverflowInteger<i32>::with(4));

  EXPECT_EQ(OverflowInteger<i32>::with(1) + i32::MAX,
            OverflowInteger<i32>::with(1) + i32::MAX);
  EXPECT_NE(OverflowInteger<i32>::with(5),
            OverflowInteger<i32>::with(1) + i32::MAX);
  EXPECT_NE(OverflowInteger<i32>::with(1) + i32::MAX,
            OverflowInteger<i32>::with(5));
}

TEST(OverflowInteger, Ord) {
  static_assert(::sus::ops::Ord<OverflowInteger<i32>>);
  static_assert(::sus::ops::Ord<OverflowInteger<i32>, i32>);
  static_assert(::sus::ops::Ord<i32, OverflowInteger<i32>>);

  EXPECT_EQ(OverflowInteger<i32>::with(5) <=> 4_i32,
            std::strong_ordering::greater);
  EXPECT_EQ(OverflowInteger<i32>::with(5) <=> 6_i32,
            std::strong_ordering::less);
  EXPECT_EQ(OverflowInteger<i32>::with(5) <=> 5_i32,
            std::strong_ordering::equivalent);

  EXPECT_EQ(6_i32 <=> OverflowInteger<i32>::with(5),
            std::strong_ordering::greater);
  EXPECT_EQ(4_i32 <=> OverflowInteger<i32>::with(5),
            std::strong_ordering::less);
  EXPECT_EQ(5_i32 <=> OverflowInteger<i32>::with(5),
            std::strong_ordering::equivalent);

  EXPECT_EQ(OverflowInteger<i32>::with(5) <=> OverflowInteger<i32>::with(4),
            std::strong_ordering::greater);
  EXPECT_EQ(OverflowInteger<i32>::with(5) <=> OverflowInteger<i32>::with(6),
            std::strong_ordering::less);
  EXPECT_EQ(OverflowInteger<i32>::with(5) <=> OverflowInteger<i32>::with(5),
            std::strong_ordering::equivalent);

  EXPECT_EQ(OverflowInteger<i32>::with(1) + i32::MAX <=>
                OverflowInteger<i32>::with(1) + i32::MAX,
            std::strong_ordering::equivalent);
  EXPECT_EQ(OverflowInteger<i32>::with(1) + i32::MAX <=> 0_i32,
            std::strong_ordering::greater);
  EXPECT_EQ(0_i32 <=> OverflowInteger<i32>::with(1) + i32::MAX,
            std::strong_ordering::less);
  EXPECT_EQ(OverflowInteger<i32>::with(1) + i32::MAX <=>
                OverflowInteger<i32>::with(0),
            std::strong_ordering::greater);
  EXPECT_EQ(OverflowInteger<i32>::with(0) <=>
                OverflowInteger<i32>::with(1) + i32::MAX,
            std::strong_ordering::less);
}

}  // namespace
