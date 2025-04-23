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

#ifdef TEST_MODULE
import sus;

#include "sus/assertions/check.h"
#else
#include "sus/iter/compat_ranges.h"

#include "sus/collections/vec.h"
#include "sus/iter/empty.h"
#include "sus/iter/iterator.h"
#include "sus/prelude.h"
#endif

#include <algorithm>
#include <concepts>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "googletest/include/gtest/gtest.h"

namespace {

using CompatRange = sus::iter::IteratorRange<sus::iter::Empty<i32>>;
static_assert(std::ranges::range<CompatRange>);
static_assert(std::ranges::viewable_range<CompatRange>);
static_assert(std::ranges::input_range<CompatRange>);
static_assert(std::ranges::output_range<CompatRange, i32>);

using CompatRangeConstRef =
    sus::iter::IteratorRange<sus::iter::Empty<const i32&>>;
static_assert(std::ranges::range<CompatRangeConstRef>);
static_assert(std::ranges::viewable_range<CompatRangeConstRef>);
static_assert(std::ranges::input_range<CompatRangeConstRef>);
static_assert(!std::ranges::output_range<CompatRangeConstRef, i32>);

using CompatRangeMutRef = sus::iter::IteratorRange<sus::iter::Empty<i32&>>;
static_assert(std::ranges::range<CompatRangeMutRef>);
static_assert(std::ranges::viewable_range<CompatRangeMutRef>);
static_assert(std::ranges::input_range<CompatRangeMutRef>);
static_assert(std::ranges::output_range<CompatRangeMutRef, i32>);

TEST(CompatRanges, ViewableRange) {
  auto vec = sus::Vec<i32>(1, 2, 3, 4, 5, 6);

  // all() requires a `std::ranges::viewable_range`.
  auto view = std::ranges::views::all(sus::move(vec).into_iter().range());
  i32 e = 1;
  for (i32 i : view) {
    EXPECT_EQ(e, i);
    e += 1;
  }
  EXPECT_EQ(e, 7);
}

TEST(CompatRanges, ViewableRangeConstRefs) {
  auto vec = sus::Vec<i32>(1, 2, 3, 4, 5, 6);

  // all() requires a `std::ranges::viewable_range`.
  auto view = std::ranges::views::all(vec.iter().range());
  i32 e = 1;
  for (const i32& i : view) {
    EXPECT_EQ(e, i);
    e += 1;
  }
  EXPECT_EQ(e, 7);
}

TEST(CompatRanges, ViewableRangeMutRefs) {
  auto vec = sus::Vec<i32>(1, 2, 3, 4, 5, 6);

  // all() requires a `std::ranges::viewable_range`.
  auto view = std::ranges::views::all(vec.iter_mut().range());
  for (i32& i : view) {
    i += 1;
  }

  EXPECT_EQ(vec, sus::Vec<i32>(2, 3, 4, 5, 6, 7));
}

TEST(CompatRanges, InputRange) {
  auto vec = sus::Vec<i32>(1, 2, 3, 4, 5, 6);

  // max() requires a `std::ranges::input_range`.
  auto max = std::ranges::max(sus::move(vec).into_iter().range());
  EXPECT_EQ(max, 6);

  // max() requires a `std::ranges::input_range`.
  static_assert(
      std::ranges::max(sus::Vec<i32>(1, 2, 4, 3).into_iter().range()) == 4);
}

TEST(CompatRanges, InputRangeConstRefs) {
  auto vec = sus::Vec<i32>(1, 2, 3, 6, 4, 5);

  // max() requires a `std::ranges::input_range`.
  auto max = std::ranges::max(vec.iter().range());
  EXPECT_EQ(max, 6);

  // max() requires a `std::ranges::input_range`.
  static_assert([]() {
    auto vec = sus::Vec<i32>(1, 2, 4, 3);
    return std::ranges::max(vec.iter().range());
  }() == 4);
}

TEST(CompatRanges, InputRangeMutRefs) {
  auto vec = sus::Vec<i32>(1, 2, 3, 6, 4, 5);

  // max() requires a `std::ranges::input_range`.
  auto max = std::ranges::max(vec.iter_mut().range());
  EXPECT_EQ(max, 6);

  // max() requires a `std::ranges::input_range`.
  static_assert([]() {
    auto vec = sus::Vec<i32>(1, 2, 4, 3);
    return std::ranges::max(vec.iter_mut().range());
  }() == 4);
}

TEST(CompatRanges, FromRange) {
  {
    auto v = std::vector<i32>({1, 2, 3});
    auto it = sus::iter::from_range(v);
    static_assert(sus::iter::Iterator<decltype(it), i32&>);
    static_assert(sus::iter::DoubleEndedIterator<decltype(it), i32&>);
    static_assert(sus::iter::ExactSizeIterator<decltype(it), i32&>);
    static_assert(sus::iter::TrustedLen<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
    static_assert(!sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::TriviallyRelocatable<decltype(it)>);
  }

  // Mutable use of vector.
  {
    auto v = std::vector<i32>({1, 2, 3});
    sus::iter::Iterator<i32&> auto it = sus::iter::from_range(v);
    static_assert(sus::iter::DoubleEndedIterator<decltype(it), i32&>);
    static_assert(sus::iter::ExactSizeIterator<decltype(it), i32&>);
    static_assert(sus::iter::TrustedLen<decltype(it)>);

    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(3u, sus::some(3u)));
    EXPECT_EQ(it.exact_size_hint(), 3u);

    EXPECT_EQ(&it.next().unwrap(), &v[0]);
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(2u, sus::some(2u)));
    EXPECT_EQ(it.exact_size_hint(), 2u);

    EXPECT_EQ(&it.next().unwrap(), &v[1]);
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(1u, sus::some(1u)));
    EXPECT_EQ(it.exact_size_hint(), 1u);

    EXPECT_EQ(&it.next().unwrap(), &v[2]);
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
    EXPECT_EQ(it.exact_size_hint(), 0u);

    EXPECT_EQ(it.next(), sus::none());
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
    EXPECT_EQ(it.exact_size_hint(), 0u);
  }
  // Const use of vector.
  {
    const auto v = std::vector<i32>({1, 2, 3});
    sus::iter::Iterator<const i32&> auto it = sus::iter::from_range(v);
    static_assert(sus::iter::DoubleEndedIterator<decltype(it), const i32&>);
    static_assert(sus::iter::ExactSizeIterator<decltype(it), const i32&>);
    static_assert(sus::iter::TrustedLen<decltype(it)>);

    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(3u, sus::some(3u)));
    EXPECT_EQ(it.exact_size_hint(), 3u);

    EXPECT_EQ(&it.next().unwrap(), &v[0]);
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(2u, sus::some(2u)));
    EXPECT_EQ(it.exact_size_hint(), 2u);

    EXPECT_EQ(&it.next().unwrap(), &v[1]);
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(1u, sus::some(1u)));
    EXPECT_EQ(it.exact_size_hint(), 1u);

    EXPECT_EQ(&it.next().unwrap(), &v[2]);
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
    EXPECT_EQ(it.exact_size_hint(), 0u);

    EXPECT_EQ(it.next(), sus::none());
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
    EXPECT_EQ(it.exact_size_hint(), 0u);
  }
  // next_back().
  {
    const auto v = std::vector<i32>({1, 2, 3});
    sus::iter::Iterator<const i32&> auto it = sus::iter::from_range(v);

    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(3u, sus::some(3u)));
    EXPECT_EQ(it.exact_size_hint(), 3u);

    EXPECT_EQ(&it.next_back().unwrap(), &v[2]);
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(2u, sus::some(2u)));
    EXPECT_EQ(it.exact_size_hint(), 2u);

    EXPECT_EQ(&it.next().unwrap(), &v[0]);
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(1u, sus::some(1u)));
    EXPECT_EQ(it.exact_size_hint(), 1u);

    EXPECT_EQ(&it.next_back().unwrap(), &v[1]);
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
    EXPECT_EQ(it.exact_size_hint(), 0u);

    EXPECT_EQ(it.next_back(), sus::none());
    EXPECT_EQ(it.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
    EXPECT_EQ(it.exact_size_hint(), 0u);
  }
  // rvalues are *not* moved from the range, the range is not actually moved
  // from. Otherwise the range which may not be owning could cause the values to
  // be moved from an unexpected place. Ranges do not express or understand
  // ownership, and moving from a range is done on a per-element basis instead.
  {
    auto v = std::vector<i32>({1, 2, 3});
    sus::iter::Iterator<i32&> auto it = sus::iter::from_range(sus::move(v));
    EXPECT_EQ(sus::move(it).copied().sum(), 1 + 2 + 3);

    EXPECT_EQ(sus::iter::from_range(std::vector<i32>({1, 2, 3})).copied().sum(),
              1 + 2 + 3);
  }
}

TEST(CompatRanges, FromRange_Example) {
  {
    // An input_iterator by reference.
    const auto v = std::vector<i32>({1, 2, 3});
    sus_check(sus::iter::from_range(v).copied().sum() == 1 + 2 + 3);
  }
  {
    // An input_iterator by value.
    auto v = std::vector<i32>({1, 2, 3});
    sus_check(sus::iter::from_range(v).moved(unsafe_fn).sum() == 1 + 2 + 3);
    v.clear();
  }
}

TEST(CompatRanges, Tr) {
  auto tr = [](std::string_view from, std::string_view to, std::string& s) {
    sus::iter::from_range(s).for_each([&](char& b) {
      if (auto found = sus::iter::from_range(from)
                           .zip(sus::iter::from_range(to))
                           .find([&](auto from_to) {
                             auto [f, t] = from_to;
                             return f == b;
                           });
          found.is_some()) {
        auto [f, t] = *found;
        b = t;
      }
    });
  };
  auto s = std::string("abracadabra");
  tr("abc", "ABC", s);
  EXPECT_EQ(s, "ABrACAdABrA");
}

static_assert(
    sus::iter::Iterator<
        decltype(sus::iter::from_range(std::vector<i32>()).moved(unsafe_fn)),
        i32>);
static_assert(
    sus::iter::DoubleEndedIterator<
        decltype(sus::iter::from_range(std::vector<i32>()).moved(unsafe_fn)),
        i32>);
static_assert(
    sus::iter::ExactSizeIterator<
        decltype(sus::iter::from_range(std::vector<i32>()).moved(unsafe_fn)),
        i32>);
static_assert(
    sus::iter::TrustedLen<
        decltype(sus::iter::from_range(std::vector<i32>()).moved(unsafe_fn))>);

TEST(CompatRanges, Moved) {
  struct Movable {
    Movable(i32 i) : i(i) {}
    Movable(Movable&& m) : i(m.i) { m.i += 1; }
    Movable& operator=(Movable&& m) { return i = m.i, m.i += 1, *this; }
    i32 i;

    constexpr bool operator==(const Movable& rhs) const noexcept = default;
  };

  // Move from references.
  {
    auto moving = std::vector<Movable>();
    moving.emplace_back(10);
    moving.emplace_back(20);

    auto it = sus::iter::from_range(moving).moved(unsafe_fn);
    static_assert(std::same_as<Movable, decltype(it.next().unwrap())>);
    EXPECT_EQ(it.size_hint().lower, 2u);
    EXPECT_EQ(it.size_hint().upper.unwrap(), 2u);
    EXPECT_EQ(it.exact_size_hint(), 2u);

    EXPECT_EQ(moving[0].i, 10);
    EXPECT_EQ(moving[1].i, 20);
    EXPECT_EQ(it.next().unwrap().i, 10);
    EXPECT_EQ(moving[0].i, 11);
    EXPECT_EQ(moving[1].i, 20);
    EXPECT_EQ(it.next().unwrap().i, 20);
    EXPECT_EQ(moving[0].i, 11);
    EXPECT_EQ(moving[1].i, 21);
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(moving[0].i, 11);
    EXPECT_EQ(moving[1].i, 21);
  }

  static_assert(sus::iter::from_range(std::vector<usize>({1u, 2u}))
                    .moved(unsafe_fn)
                    .sum() == 1u + 2u);
}

}  // namespace
