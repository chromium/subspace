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
#include "subspace/construct/default.h"
#include "subspace/macros/compiler.h"
#include "subspace/prelude.h"

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
  auto operator<=>(const NoDefault&) const noexcept {
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
// RangeFull has a 0 data size so it's not (0 is indistinguishable from the
// error case and doesn't need to be memcpy'd). But on clang, it is marked
// trivial_abi, so it can be seen as trivially relocatable there.
//
// However clang-cl (as seen via subdoc) does something weird here, and makes
// the data size into 1.
#if !(_MSC_VER && defined(__clang__))
static_assert(sus::mem::data_size_of<sus::ops::RangeFull<usize>>() == 0u);
static_assert(!sus::mem::relocate_by_memcpy<sus::ops::RangeFull<usize>>);
#endif

// The types produced by the various literal syntaxes.
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
