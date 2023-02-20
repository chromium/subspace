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

#include "subspace/ops/range.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/prelude.h"

namespace {

// Range* satisifies RangeBounds.
static_assert(sus::ops::RangeBounds<sus::ops::Range<usize>, usize>);
static_assert(sus::ops::RangeBounds<sus::ops::RangeFrom<usize>, usize>);
static_assert(sus::ops::RangeBounds<sus::ops::RangeTo<usize>, usize>);
static_assert(sus::ops::RangeBounds<sus::ops::RangeFull<usize>, usize>);

// Range and RangeFrom satisify Iterator, and Range satisfies
// DoubleEndedIterator.
static_assert(sus::iter::Iterator<sus::ops::Range<usize>, usize>);
static_assert(sus::iter::DoubleEndedIterator<sus::ops::Range<usize>, usize>);
static_assert(sus::iter::Iterator<sus::ops::RangeFrom<usize>, usize>);
static_assert(
    !sus::iter::DoubleEndedIterator<sus::ops::RangeFrom<usize>, usize>);
static_assert(!sus::iter::Iterator<sus::ops::RangeTo<usize>, usize>);
static_assert(!sus::iter::Iterator<sus::ops::RangeFull<usize>, usize>);

static_assert(std::same_as<decltype(".."_r), sus::ops::RangeFull<usize>>);
static_assert(std::same_as<decltype("1.."_r), sus::ops::RangeFrom<usize>>);
static_assert(std::same_as<decltype("..2"_r), sus::ops::RangeTo<usize>>);
static_assert(std::same_as<decltype("1..2"_r), sus::ops::Range<usize>>);
static_assert(std::same_as<decltype("1..=2"_r), sus::ops::Range<usize>>);

// Start and end bounds.
// clang-format off
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

}  // namespace
