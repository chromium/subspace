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

#include "sus/iter/once.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/prelude.h"

namespace {

TEST(Once, Example) {
  auto o = sus::iter::once<u16>(sus::some(3_u16));
  sus::check(o.next().unwrap() == 3_u16);
}

TEST(Once, Empty) {
  auto o = sus::iter::once<u16>(sus::none());
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
  EXPECT_EQ(o.exact_size_hint(), 0u);
  EXPECT_EQ(o.next(), sus::none());
}

TEST(Once, Next) {
  auto o = sus::iter::once<u16>(sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(1u, sus::some(1u)));
  EXPECT_EQ(o.exact_size_hint(), 1u);
  EXPECT_EQ(o.next(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
  EXPECT_EQ(o.exact_size_hint(), 0u);
  EXPECT_EQ(o.next(), sus::none());
}

TEST(Once, NextBack) {
  auto o = sus::iter::once<u16>(sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(1u, sus::some(1u)));
  EXPECT_EQ(o.exact_size_hint(), 1u);
  EXPECT_EQ(o.next_back(), sus::some(3_u16));
  EXPECT_EQ(o.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
  EXPECT_EQ(o.exact_size_hint(), 0u);
  EXPECT_EQ(o.next_back(), sus::none());
}

}  // namespace
