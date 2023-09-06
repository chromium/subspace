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

#include "sus/ops/range.h"

#include <sstream>

#include "googletest/include/gtest/gtest.h"
#include "sus/construct/default.h"
#include "sus/macros/compiler.h"
#include "sus/prelude.h"

namespace {

using sus::ops::Range;
using sus::ops::RangeFrom;
using sus::ops::RangeFull;
using sus::ops::RangeTo;

// Range* satisifies RangeBounds.
static_assert(sus::ops::RangeBounds<sus::ops::Range<usize>, usize>);
static_assert(sus::ops::RangeBounds<sus::ops::RangeFrom<usize>, usize>);
static_assert(sus::ops::RangeBounds<sus::ops::RangeTo<usize>, usize>);
static_assert(sus::ops::RangeBounds<sus::ops::RangeFull<usize>, usize>);

struct NoDefault {
  NoDefault(i32) {}
  std::strong_ordering operator<=>(const NoDefault&) const noexcept {
    return std::strong_ordering::equal;
  }
};

// Range*<T> is Default if T is. RangeFull is always Default as it has no T
// field.
static_assert(sus::construct::Default<sus::ops::Range<usize>>);
static_assert(sus::construct::Default<sus::ops::RangeFrom<usize>>);
static_assert(sus::construct::Default<sus::ops::RangeTo<usize>>);
static_assert(sus::construct::Default<sus::ops::RangeFull<usize>>);
static_assert(!sus::construct::Default<sus::ops::Range<NoDefault>>);
static_assert(!sus::construct::Default<sus::ops::RangeFrom<NoDefault>>);
static_assert(!sus::construct::Default<sus::ops::RangeTo<NoDefault>>);
static_assert(sus::construct::Default<sus::ops::RangeFull<NoDefault>>);

// Range and RangeFrom on integer types satisify Iterator, and Range satisfies
// DoubleEndedIterator.
static_assert(sus::iter::Iterator<sus::ops::Range<usize>, usize>);
static_assert(sus::iter::DoubleEndedIterator<sus::ops::Range<usize>, usize>);
static_assert(sus::iter::Iterator<sus::ops::RangeFrom<usize>, usize>);
static_assert(
    !sus::iter::DoubleEndedIterator<sus::ops::RangeFrom<usize>, usize>);
static_assert(!sus::iter::Iterator<sus::ops::RangeTo<usize>, usize>);
static_assert(!sus::iter::Iterator<sus::ops::RangeFull<usize>, usize>);

// Other types in Range don't let it be an Iterator.
static_assert(!sus::iter::Iterator<sus::ops::Range<NoDefault>, NoDefault>);
static_assert(!sus::iter::Iterator<sus::ops::RangeFrom<NoDefault>, NoDefault>);

// Range types are trivially relocatable if the inner type is.
static_assert(sus::mem::relocate_by_memcpy<sus::ops::Range<usize>>);
static_assert(sus::mem::relocate_by_memcpy<sus::ops::RangeFrom<usize>>);
static_assert(sus::mem::relocate_by_memcpy<sus::ops::RangeTo<usize>>);
static_assert(sus::mem::relocate_by_memcpy<sus::ops::RangeFull<usize>>);

// The types produced by the various literal syntaxes.
static_assert(std::same_as<decltype(".."_r), sus::ops::RangeFull<usize>>);
static_assert(std::same_as<decltype("1.."_r), sus::ops::RangeFrom<usize>>);
static_assert(std::same_as<decltype("..2"_r), sus::ops::RangeTo<usize>>);
static_assert(std::same_as<decltype("1..2"_r), sus::ops::Range<usize>>);
static_assert(std::same_as<decltype("1..=2"_r), sus::ops::Range<usize>>);
// Signed.
static_assert(std::same_as<decltype(".."_rs), sus::ops::RangeFull<isize>>);
static_assert(std::same_as<decltype("1.."_rs), sus::ops::RangeFrom<isize>>);
static_assert(std::same_as<decltype("..2"_rs), sus::ops::RangeTo<isize>>);
static_assert(std::same_as<decltype("1..2"_rs), sus::ops::Range<isize>>);
static_assert(std::same_as<decltype("1..=2"_rs), sus::ops::Range<isize>>);

// clang-format off

// Start and end bounds.
static_assert([]() constexpr {auto r = ".."_r; return r.start_bound().is_none(); }());
static_assert([]() constexpr { auto r = ".."_r; return r.end_bound().is_none(); }());
static_assert([]() constexpr { auto r = "3.."_r; return r.start_bound().unwrap() == 3_usize; }());
static_assert([]() constexpr { auto r = "3.."_r; return r.end_bound().is_none(); }());
static_assert([]() constexpr { auto r = "..3"_r; return r.start_bound().is_none(); }());
static_assert([]() constexpr { auto r = "..3"_r; return r.end_bound().unwrap() == 3_usize; }());
static_assert([]() constexpr { auto r = "..=3"_r; return r.start_bound().is_none(); }());
static_assert([]() constexpr { auto r = "..=3"_r; return r.end_bound().unwrap() == 4_usize; }());
static_assert([]() constexpr { auto r = "3..8"_r; return r.start_bound().unwrap() == 3_usize; }());
static_assert([]() constexpr { auto r = "3..8"_r; return r.end_bound().unwrap() == 8_usize; }());
static_assert([]() constexpr { auto r = "3..=8"_r; return r.start_bound().unwrap() == 3_usize; }());
static_assert([]() constexpr { auto r = "3..=8"_r; return r.end_bound().unwrap() == 9_usize; }());
// Signed
static_assert([]() constexpr {auto r = ".."_rs; return r.start_bound().is_none(); }());
static_assert([]() constexpr { auto r = ".."_rs; return r.end_bound().is_none(); }());
static_assert([]() constexpr { auto r = "3.."_rs; return r.start_bound().unwrap() == 3_isize; }());
static_assert([]() constexpr { auto r = "3.."_rs; return r.end_bound().is_none(); }());
static_assert([]() constexpr { auto r = "-3.."_rs; return r.start_bound().unwrap() == -3_isize; }());
static_assert([]() constexpr { auto r = "-3.."_rs; return r.end_bound().is_none(); }());
static_assert([]() constexpr { auto r = "..3"_rs; return r.start_bound().is_none(); }());
static_assert([]() constexpr { auto r = "..3"_rs; return r.end_bound().unwrap() == 3_isize; }());
static_assert([]() constexpr { auto r = "..-3"_rs; return r.start_bound().is_none(); }());
static_assert([]() constexpr { auto r = "..-3"_rs; return r.end_bound().unwrap() == -3_isize; }());
static_assert([]() constexpr { auto r = "..=3"_rs; return r.start_bound().is_none(); }());
static_assert([]() constexpr { auto r = "..=3"_rs; return r.end_bound().unwrap() == 4_isize; }());
static_assert([]() constexpr { auto r = "..=-3"_rs; return r.start_bound().is_none(); }());
static_assert([]() constexpr { auto r = "..=-3"_rs; return r.end_bound().unwrap() == -2_isize; }());
static_assert([]() constexpr { auto r = "-8..3"_rs; return r.start_bound().unwrap() == -8_isize; }());
static_assert([]() constexpr { auto r = "-8..3"_rs; return r.end_bound().unwrap() == 3_isize; }());
static_assert([]() constexpr { auto r = "-8..-3"_rs; return r.start_bound().unwrap() == -8_isize; }());
static_assert([]() constexpr { auto r = "-8..-3"_rs; return r.end_bound().unwrap() == -3_isize; }());
static_assert([]() constexpr { auto r = "-8..=-3"_rs; return r.start_bound().unwrap() == -8_isize; }());
static_assert([]() constexpr { auto r = "-8..=-3"_rs; return r.end_bound().unwrap() == -2_isize; }());
static_assert([]() constexpr { auto r = "-8..=-1"_rs; return r.end_bound().unwrap() == 0_isize; }());
static_assert([]() constexpr { auto r = "-8..=-0"_rs; return r.end_bound().unwrap() == 1_isize; }());
static_assert([]() constexpr { auto r = "-8..=0"_rs; return r.end_bound().unwrap() == 1_isize; }());

// clang-format on

// Parsing of numbers.
static_assert([]() constexpr {
  auto r = "345678..876543"_r;
  return r.start_bound().unwrap() == 345678_usize;
}());
static_assert([]() constexpr {
  auto r = "345678..876543"_r;
  return r.end_bound().unwrap() == 876543_usize;
}());
static_assert([]() constexpr {
  auto r = "3'4'5'6..87'654'3"_r;
  return r.start_bound().unwrap() == 3456_usize;
}());
static_assert([]() constexpr {
  auto r = "3'4'5'6..87'654'3"_r;
  return r.end_bound().unwrap() == 876543_usize;
}());
// Signed.
static_assert([]() constexpr {
  auto r = "345678..876543"_rs;
  return r.start_bound().unwrap() == 345678_isize;
}());
static_assert([]() constexpr {
  auto r = "345678..876543"_rs;
  return r.end_bound().unwrap() == 876543_isize;
}());
static_assert([]() constexpr {
  auto r = "-3'4'5'6..87'654'3"_rs;
  return r.start_bound().unwrap() == -3456_isize;
}());
static_assert([]() constexpr {
  auto r = "3'4'5'6..-87'654'3"_rs;
  return r.end_bound().unwrap() == -876543_isize;
}());

// None of these compile.
// TODO: No-compile tests of some sort? Can't test this with a concept.
// auto x2 = "..'2"_r;
// auto x2 = "..2'"_r;
// auto x3 = "'1..'2"_r;
// auto x4 = "1..2'"_r;
// auto x5 = "'1.."_r;
// auto x6 = "'1..2"_r;
// auto x7 = "1'.."_r;
// auto x8 = "1'..2"_r;
// auto x9 = "1''2..'"_r;
// auto x10 = "1''2..3'"_r;
// auto x11 = "'1..=2"_r;
// auto x12 = "1'..=2"_r;
// auto x13 = "1..='2"_r;
// auto x14 = "1..=2'"_r;

// start_at() and end_at().
static_assert([]() constexpr {
  auto r = "1..5"_r.start_at(8u);
  return r == Range(8_usize, 5_usize);
}());
static_assert([]() constexpr {
  auto r = "1..5"_r.start_at(8u);
  return r == Range(8_usize, 5_usize);
}());
static_assert([]() constexpr {
  auto r = "1..5"_r.end_at(8u);
  return r == Range(1_usize, 8_usize);
}());
static_assert([]() constexpr {
  auto r = "1..5"_r.start_at(8u).end_at(9u);
  return r == Range(8_usize, 9_usize);
}());
static_assert([]() constexpr {
  auto r = "1.."_r.start_at(8u);
  return r == RangeFrom(8_usize);
}());
static_assert([]() constexpr {
  auto r = "1.."_r.end_at(8u);
  return r == Range(1_usize, 8_usize);
}());
static_assert([]() constexpr {
  auto r = "1..5"_r.start_at(8u).end_at(9u);
  return r == Range(8_usize, 9_usize);
}());
static_assert([]() constexpr {
  auto r = "..5"_r.start_at(8u);
  return r == Range(8_usize, 5_usize);
}());
static_assert([]() constexpr {
  auto r = "..5"_r.end_at(8u);
  return r == RangeTo(8_usize);
}());
static_assert([]() constexpr {
  auto r = "..5"_r.start_at(8u).end_at(9u);
  return r == Range(8_usize, 9_usize);
}());
static_assert([]() constexpr {
  auto r = ".."_r.start_at(8u);
  return r == RangeFrom(8_usize);
}());
static_assert([]() constexpr {
  auto r = ".."_r.end_at(8u);
  return r == RangeTo(8_usize);
}());
static_assert([]() constexpr {
  auto r = ".."_r.start_at(8u).end_at(9u);
  return r == Range(8_usize, 9_usize);
}());

static_assert(std::same_as<decltype(sus::ops::range(1_i32, 2_i32)),
                           sus::ops::Range<i32>>);
static_assert(std::same_as<decltype(sus::ops::range_from(1_i32)),
                           sus::ops::RangeFrom<i32>>);
static_assert(
    std::same_as<decltype(sus::ops::range_to(2_i32)), sus::ops::RangeTo<i32>>);

TEST(Range, Iter) {
  auto it = "1..5"_r;
  EXPECT_EQ(it.next().unwrap(), 1u);
  EXPECT_EQ(it.next().unwrap(), 2u);
  EXPECT_EQ(it.next().unwrap(), 3u);
  EXPECT_EQ(it.next().unwrap(), 4u);
  EXPECT_EQ(it.next(), sus::None);

  auto it2 = "3.."_r;
  EXPECT_EQ(it2.next().unwrap(), 3u);
  EXPECT_EQ(it2.next().unwrap(), 4u);
  EXPECT_EQ(it2.next().unwrap(), 5u);
  EXPECT_EQ(it2.next().unwrap(), 6u);
  EXPECT_EQ(it2.next().unwrap(), 7u);
  // Never ends..
}

TEST(Range, RangeForIterator) {
  sus::Vec<usize> v;
  for (auto&& i : "1..5"_r) {
    static_assert(std::same_as<decltype(i), usize&&>);
    v.push(i);
  }
  EXPECT_EQ(v[0u], 1u);
  EXPECT_EQ(v[1u], 2u);
  EXPECT_EQ(v[2u], 3u);
  EXPECT_EQ(v[3u], 4u);
  EXPECT_EQ(v.len(), 4u);

  v.clear();

  for (auto&& i : "1.."_r) {
    static_assert(std::same_as<decltype(i), usize&&>);
    v.push(i);
    if (i == 3u) break;
  }
  EXPECT_EQ(v[0u], 1u);
  EXPECT_EQ(v[1u], 2u);
  EXPECT_EQ(v[2u], 3u);
  EXPECT_EQ(v.len(), 3u);
}

TEST(Range, StructuredBindings) {
  auto [a, b] = "1..5"_r;
  EXPECT_EQ(a, 1u);
  EXPECT_EQ(b, 5u);
}

TEST(Range, Functions) {
  static_assert(sus::ops::range(2_i32, 5_i32).into_iter().sum() == 2 + 3 + 4);
  static_assert(sus::ops::range_from(2_i32).into_iter().take(3u).sum() ==
                2 + 3 + 4);
  static_assert(sus::ops::range_to(5_i32).start_at(2).into_iter().sum() ==
                2 + 3 + 4);
}

TEST(Range, Primitives) {
  int sum = 0;
  for (int i : sus::ops::range(2, 5)) {
    sum += i;
  }
  EXPECT_EQ(sum, 2 + 3 + 4);
}

TEST(Range, fmt) {
  static_assert(fmt::is_formattable<sus::ops::Range<i32>, char>::value);
  static_assert(fmt::is_formattable<sus::ops::RangeFrom<i32>, char>::value);
  static_assert(fmt::is_formattable<sus::ops::RangeTo<i32>, char>::value);
  static_assert(fmt::is_formattable<sus::ops::RangeFull<i32>, char>::value);

  EXPECT_EQ(fmt::format("{}", sus::ops::Range<i32>(1, 5)), "1..5");
  EXPECT_EQ(fmt::format("{:02}", sus::ops::Range<i32>(1, 5)), "01..05");
  EXPECT_EQ(fmt::format("{}", sus::ops::RangeFrom<i32>(1)), "1..");
  EXPECT_EQ(fmt::format("{:02}", sus::ops::RangeFrom<i32>(1)), "01..");
  EXPECT_EQ(fmt::format("{}", sus::ops::RangeTo<i32>(5)), "..5");
  EXPECT_EQ(fmt::format("{:02}", sus::ops::RangeTo<i32>(5)), "..05");
  EXPECT_EQ(fmt::format("{}", sus::ops::RangeFull<i32>()), "..");
  EXPECT_EQ(fmt::format("{:02}", sus::ops::RangeFull<i32>()), "..");

  struct NoFormat {
    i32 a = 0x16ae3cf2;
    std::strong_ordering operator<=>(const NoFormat& rhs) const noexcept {
      return a <=> rhs.a;
    }
  };
  static_assert(sus::ops::StrongOrd<NoFormat>);
  static_assert(!fmt::is_formattable<NoFormat, char>::value);
  static_assert(fmt::is_formattable<sus::ops::Range<NoFormat>, char>::value);
  static_assert(
      fmt::is_formattable<sus::ops::RangeFrom<NoFormat>, char>::value);
  static_assert(fmt::is_formattable<sus::ops::RangeTo<NoFormat>, char>::value);
  static_assert(
      fmt::is_formattable<sus::ops::RangeFull<NoFormat>, char>::value);

  EXPECT_EQ(fmt::format("{}", sus::ops::Range<NoFormat>(NoFormat(0xf00d),
                                                        NoFormat(0xbeef))),
            "0d-f0-00-00..ef-be-00-00");
  EXPECT_EQ(fmt::format("{}", sus::ops::RangeFrom<NoFormat>(NoFormat(0xf00d))),
            "0d-f0-00-00..");
  EXPECT_EQ(fmt::format("{}", sus::ops::RangeTo<NoFormat>(NoFormat(0xbeef))),
            "..ef-be-00-00");
}

TEST(Range, Stream) {
  {
    std::stringstream s;
    s << sus::ops::Range<i32>(1, 5) << " " << sus::ops::RangeFrom<i32>(1) << " "
      << sus::ops::RangeTo<i32>(5) << " " << sus::ops::RangeFull<i32>();
    EXPECT_EQ(s.str(), "1..5 1.. ..5 ..");
  }
}

TEST(Range, GTest) {
  EXPECT_EQ(testing::PrintToString(sus::ops::Range<i32>(1, 5)), "1..5");
  EXPECT_EQ(testing::PrintToString(sus::ops::RangeFrom<i32>(1)), "1..");
  EXPECT_EQ(testing::PrintToString(sus::ops::RangeTo<i32>(5)), "..5");
  EXPECT_EQ(testing::PrintToString(sus::ops::RangeFull<i32>()), "..");
}

}  // namespace
