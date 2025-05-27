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

#ifdef TEST_MODULE
import sus;
#else
#include "sus/cmp/ord.h"

#include "sus/num/types.h"
#include "sus/prelude.h"
#endif

#include "sus/macros/__private/compiler_bugs.h"
#include "googletest/include/gtest/gtest.h"

namespace {

using sus::cmp::clamp;
using sus::cmp::max;
using sus::cmp::max_by;
using sus::cmp::max_by_key;
using sus::cmp::min;
using sus::cmp::min_by;
using sus::cmp::min_by_key;

struct Strong {
  sus_clang_bug_54040(Strong(i32 i, i32 id) : i(i), id(id){});

  i32 i;
  i32 id;

  friend std::strong_ordering operator<=>(const Strong& a, const Strong& b) noexcept {
    return a.i <=> b.i;
  }
};
static_assert(sus::cmp::StrongOrd<Strong>);

struct NoCmp {
  sus_clang_bug_54040(NoCmp(i32 i, i32 id) : i(i), id(id){});

  i32 i;
  i32 id;
};
static_assert(!sus::cmp::StrongOrd<NoCmp>);

TEST(StrongOrd, Min) {
  auto low1 = Strong(1, 1);
  auto low2 = Strong(1, 2);
  auto high = Strong(3, 3);

  EXPECT_EQ(min(low1, high).id, 1_i32);
  EXPECT_EQ(min(high, low1).id, 1_i32);

  // On equal, the first is returned.
  EXPECT_EQ(min(low1, low2).id, 1_i32);
  EXPECT_EQ(min(low2, low1).id, 2_i32);
}

TEST(StrongOrd, MinBy) {
  auto cmp = [](const NoCmp& a, const NoCmp& b) { return a.i <=> b.i; };

  auto low1 = NoCmp(1, 1);
  auto low2 = NoCmp(1, 2);
  auto high = NoCmp(3, 3);

  // NoCmp is not StrongOrd, but the comparator returns a strong_ordering, so they can
  // be compared through it.
  EXPECT_EQ(min_by(low1, high, cmp).id, 1_i32);
  EXPECT_EQ(min_by(high, low1, cmp).id, 1_i32);

  // On equal, the first is returned.
  EXPECT_EQ(min_by(low1, low2, cmp).id, 1_i32);
  EXPECT_EQ(min_by(low2, low1, cmp).id, 2_i32);
}

TEST(StrongOrd, MinByKey) {
  auto get_i = [](const NoCmp& a) { return a.i; };

  auto low1 = NoCmp(1, 1);
  auto low2 = NoCmp(1, 2);
  auto high = NoCmp(3, 3);

  // NoCmp is not StrongOrd, but the key function returns a type that is StrongOrd.
  EXPECT_EQ(min_by_key(low1, high, get_i).id, 1_i32);
  EXPECT_EQ(min_by_key(high, low1, get_i).id, 1_i32);

  // On equal, the first is returned.
  EXPECT_EQ(min_by_key(low1, low2, get_i).id, 1_i32);
  EXPECT_EQ(min_by_key(low2, low1, get_i).id, 2_i32);
}

TEST(StrongOrd, Max) {
  auto low1 = Strong(1, 1);
  auto low2 = Strong(1, 2);
  auto high = Strong(3, 3);

  EXPECT_EQ(max(low1, high).id, 3_i32);
  EXPECT_EQ(max(high, low1).id, 3_i32);

  // On equal, the second is returned.
  EXPECT_EQ(max(low1, low2).id, 2_i32);
  EXPECT_EQ(max(low2, low1).id, 1_i32);
}

TEST(StrongOrd, MaxBy) {
  auto cmp = [](const NoCmp& a, const NoCmp& b) { return a.i <=> b.i; };

  auto low1 = NoCmp(1, 1);
  auto low2 = NoCmp(1, 2);
  auto high = NoCmp(3, 3);

  // NoCmp is not StrongOrd, but the comparator returns a strong_ordering, so they can
  // be compared through it.
  EXPECT_EQ(max_by(low1, high, cmp).id, 3_i32);
  EXPECT_EQ(max_by(high, low1, cmp).id, 3_i32);

  // On equal, the second is returned.
  EXPECT_EQ(max_by(low1, low2, cmp).id, 2_i32);
  EXPECT_EQ(max_by(low2, low1, cmp).id, 1_i32);
}

TEST(StrongOrd, MaxByKey) {
  auto get_i = [](const NoCmp& a) { return a.i; };

  auto low1 = NoCmp(1, 1);
  auto low2 = NoCmp(1, 2);
  auto high = NoCmp(3, 3);

  // NoCmp is not StrongOrd, but the key function returns a type that is StrongOrd.
  EXPECT_EQ(max_by_key(low1, high, get_i).id, 3_i32);
  EXPECT_EQ(max_by_key(high, low1, get_i).id, 3_i32);

  // On equal, the second is returned.
  EXPECT_EQ(max_by_key(low1, low2, get_i).id, 2_i32);
  EXPECT_EQ(max_by_key(low2, low1, get_i).id, 1_i32);
}

}  // namespace
