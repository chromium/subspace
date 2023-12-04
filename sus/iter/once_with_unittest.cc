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

#include "sus/iter/once_with.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/prelude.h"

namespace {

TEST(OnceWith, Example) {
  auto ow = sus::iter::once_with<u16>([]() { return 3_u16; });
  sus_check(ow.next().unwrap() == 3_u16);
}

TEST(OnceWith, Next) {
  auto ow = sus::iter::once_with<u16>([]() { return 3_u16; });
  EXPECT_EQ(ow.size_hint(), sus::iter::SizeHint(1u, sus::some(1u)));
  EXPECT_EQ(ow.exact_size_hint(), 1u);
  EXPECT_EQ(ow.next(), sus::some(3_u16));
  EXPECT_EQ(ow.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
  EXPECT_EQ(ow.exact_size_hint(), 0u);
  EXPECT_EQ(ow.next(), sus::none());
}

TEST(OnceWith, NextBack) {
  auto ow = sus::iter::once_with<u16>([]() { return 3_u16; });
  EXPECT_EQ(ow.size_hint(), sus::iter::SizeHint(1u, sus::some(1u)));
  EXPECT_EQ(ow.exact_size_hint(), 1u);
  EXPECT_EQ(ow.next_back(), sus::some(3_u16));
  EXPECT_EQ(ow.size_hint(), sus::iter::SizeHint(0u, sus::some(0u)));
  EXPECT_EQ(ow.exact_size_hint(), 0u);
  EXPECT_EQ(ow.next_back(), sus::none());
}

// constexpr, and verifies that the return type can be converted to the Item
// type.
static_assert(sus::iter::once_with<i32>([] { return 5; }).sum() == 5);

}  // namespace
