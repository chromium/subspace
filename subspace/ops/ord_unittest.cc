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

#include "subspace/ops/ord.h"

#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/num/types.h"
#include "subspace/prelude.h"
#include "googletest/include/gtest/gtest.h"

namespace {

using sus::ops::clamp;
using sus::ops::max;
using sus::ops::max_by;
using sus::ops::max_by_key;
using sus::ops::min;
using sus::ops::min_by;
using sus::ops::min_by_key;

struct Strong {
  sus_clang_bug_54040(Strong(i32 i, i32 id) : i(i), id(id){});

  i32 i;
  i32 id;

  friend std::strong_ordering operator<=>(const Strong& a, const Strong& b) {
    return a.i <=> b.i;
  }
};
static_assert(sus::ops::Ord<Strong>);

struct NoCmp {
  sus_clang_bug_54040(NoCmp(i32 i, i32 id) : i(i), id(id){});

  i32 i;
  i32 id;
};
static_assert(!sus::ops::Ord<NoCmp>);

TEST(Ord, Min) {
  auto low1 = Strong(1, 1);
  auto low2 = Strong(1, 2);
  auto high = Strong(3, 3);

  EXPECT_EQ(min(low1, high).id, 1_i32);
  EXPECT_EQ(min(high, low1).id, 1_i32);

  // On equal, the first is returned.
  EXPECT_EQ(min(low1, low2).id, 1_i32);
  EXPECT_EQ(min(low2, low1).id, 2_i32);
}

TEST(Ord, MinBy) {
  auto cmp = [](const NoCmp& a, const NoCmp& b) { return a.i <=> b.i; };

  auto low1 = NoCmp(1, 1);
  auto low2 = NoCmp(1, 2);
  auto high = NoCmp(3, 3);

  // NoCmp is not Ord, but the comparator returns a strong_ordering, so they can
  // be compared through it.
  EXPECT_EQ(min_by(low1, high, cmp).id, 1_i32);
  EXPECT_EQ(min_by(high, low1, cmp).id, 1_i32);

  // On equal, the first is returned.
  EXPECT_EQ(min_by(low1, low2, cmp).id, 1_i32);
  EXPECT_EQ(min_by(low2, low1, cmp).id, 2_i32);
}

TEST(Ord, MinByKey) {
  auto get_i = [](const NoCmp& a) { return a.i; };

  auto low1 = NoCmp(1, 1);
  auto low2 = NoCmp(1, 2);
  auto high = NoCmp(3, 3);

  // NoCmp is not Ord, but the key function returns a type that is Ord.
  EXPECT_EQ(min_by_key(low1, high, get_i).id, 1_i32);
  EXPECT_EQ(min_by_key(high, low1, get_i).id, 1_i32);

  // On equal, the first is returned.
  EXPECT_EQ(min_by_key(low1, low2, get_i).id, 1_i32);
  EXPECT_EQ(min_by_key(low2, low1, get_i).id, 2_i32);
}

TEST(Ord, Max) {
  auto low1 = Strong(1, 1);
  auto low2 = Strong(1, 2);
  auto high = Strong(3, 3);

  EXPECT_EQ(max(low1, high).id, 3_i32);
  EXPECT_EQ(max(high, low1).id, 3_i32);

  // On equal, the second is returned.
  EXPECT_EQ(max(low1, low2).id, 2_i32);
  EXPECT_EQ(max(low2, low1).id, 1_i32);
}

TEST(Ord, MaxBy) {
  auto cmp = [](const NoCmp& a, const NoCmp& b) { return a.i <=> b.i; };

  auto low1 = NoCmp(1, 1);
  auto low2 = NoCmp(1, 2);
  auto high = NoCmp(3, 3);

  // NoCmp is not Ord, but the comparator returns a strong_ordering, so they can
  // be compared through it.
  EXPECT_EQ(max_by(low1, high, cmp).id, 3_i32);
  EXPECT_EQ(max_by(high, low1, cmp).id, 3_i32);

  // On equal, the second is returned.
  EXPECT_EQ(max_by(low1, low2, cmp).id, 2_i32);
  EXPECT_EQ(max_by(low2, low1, cmp).id, 1_i32);
}

TEST(Ord, MaxByKey) {
  auto get_i = [](const NoCmp& a) { return a.i; };

  auto low1 = NoCmp(1, 1);
  auto low2 = NoCmp(1, 2);
  auto high = NoCmp(3, 3);

  // NoCmp is not Ord, but the key function returns a type that is Ord.
  EXPECT_EQ(max_by_key(low1, high, get_i).id, 3_i32);
  EXPECT_EQ(max_by_key(high, low1, get_i).id, 3_i32);

  // On equal, the second is returned.
  EXPECT_EQ(max_by_key(low1, low2, get_i).id, 2_i32);
  EXPECT_EQ(max_by_key(low2, low1, get_i).id, 1_i32);
}

}  // namespace
