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

#include "sus/iter/repeat.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/prelude.h"

namespace {

TEST(Repeat, Example) {
  auto r = sus::iter::repeat<u16>(3_u16);
  sus_check(r.next().unwrap() == 3_u16);
  sus_check(r.next().unwrap() == 3_u16);
  sus_check(r.next().unwrap() == 3_u16);
}

TEST(Repeat, Next) {
  auto o = sus::iter::repeat<u16>(3_u16);
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next(), sus::some(3_u16));
}

TEST(Repeat, NextBack) {
  auto o = sus::iter::repeat<u16>(3_u16);
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next_back(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next_back(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(usize::MAX, sus::none()));
  EXPECT_EQ(o.next_back(), sus::some(3_u16));
}

}  // namespace
