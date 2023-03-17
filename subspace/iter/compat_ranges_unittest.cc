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

#include "subspace/iter/compat_ranges.h"

#include <concepts>
#include <iterator>
#include <ranges>

#include "googletest/include/gtest/gtest.h"
#include "subspace/containers/vec.h"
#include "subspace/iter/empty.h"
#include "subspace/prelude.h"

namespace {

using CompatRange = sus::iter::IteratorRange<sus::iter::Empty<i32>>;

static_assert(std::ranges::range<CompatRange>);
static_assert(std::ranges::viewable_range<CompatRange>);
static_assert(std::ranges::input_range<CompatRange>);

TEST(CompatRanges, ViewableRange) {
  sus::Vec<i32> vec = sus::vec(1, 2, 3, 4, 5, 6);
  // all() requires a `std::ranges::viewable_range`.
  auto view = std::ranges::views::all(sus::move(vec).into_iter().range());
  i32 e = 1;
  for (i32 i : view) {
    EXPECT_EQ(e, i);
    e += 1;
  }
  EXPECT_EQ(e, 7);
}

TEST(CompatRanges, InputRange) {
  sus::Vec<i32> vec = sus::vec(1, 2, 3, 4, 5, 6);
  // filter() requires a `std::ranges::input_range`.
  auto filter = std::ranges::views::filter(sus::move(vec).into_iter().range(),
                                           [](const i32& i) { return i > 3; });
  i32 e = 4;
  for (i32 i : filter) {
    EXPECT_EQ(e, i);
    e += 1;
  }
  EXPECT_EQ(e, 7);
}

}  // namespace
