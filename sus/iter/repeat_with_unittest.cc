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

#include "sus/iter/repeat_with.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/prelude.h"

namespace {

TEST(RepeatWith, Example) {
  auto r = sus::iter::repeat_with<u16>([] { return 3_u16; });
  sus_check(r.next().unwrap() == 3_u16);
  sus_check(r.next().unwrap() == 3_u16);
  sus_check(r.next().unwrap() == 3_u16);
}

TEST(RepeatWith, Next) {
  auto o = sus::iter::repeat_with<u16>([] { return 3_u16; });
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next(), sus::some(3_u16));
}

TEST(RepeatWith, NextBack) {
  auto o = sus::iter::repeat_with<u16>([] { return 3_u16; });
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next_back(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next_back(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next_back(), sus::some(3_u16));
}

// constexpr, and verifies that the return type can be converted to the Item
// type.
static_assert(sus::iter::repeat_with<i32>([] { return 3; }).take(4u).sum() ==
              3 * 4);

}  // namespace
