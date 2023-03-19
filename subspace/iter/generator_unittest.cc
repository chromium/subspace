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

#include "subspace/iter/generator.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/iter/iterator.h"
#include "subspace/prelude.h"
#include "subspace/test/no_copy_move.h"

namespace {

using sus::iter::Generator;
using sus::test::NoCopyMove;

TEST(IterGenerator, Iterator) {
  {
    auto x = []() -> Generator<i32> {
      co_yield 1;
      co_yield 3;
      co_yield 5;
      co_yield 7;
    };
    sus::iter::Iterator<i32> auto it = x();
    static_assert(sus::iter::Iterator<decltype(it), i32>);
    EXPECT_EQ(it.next().unwrap(), 1);
    EXPECT_EQ(it.next().unwrap(), 3);
    EXPECT_EQ(it.next().unwrap(), 5);
    EXPECT_EQ(it.next().unwrap(), 7);
    EXPECT_EQ(it.next(), sus::None);
  }

  {
    NoCopyMove n1, n2, n3;
    auto x = [&]() -> Generator<NoCopyMove&> {
      co_yield n3;
      co_yield n1;
      co_yield n2;
    };
    sus::iter::Iterator<NoCopyMove&> auto it = x();
    static_assert(sus::iter::Iterator<decltype(it), NoCopyMove&>);
    EXPECT_EQ(&it.next().unwrap(), &n3);
    EXPECT_EQ(&it.next().unwrap(), &n1);
    EXPECT_EQ(&it.next().unwrap(), &n2);
    EXPECT_EQ(it.next(), sus::None);
  }

  {
    NoCopyMove n1, n2, n3;
    auto x = [&]() -> Generator<const NoCopyMove&> {
      co_yield n3;
      co_yield n1;
      co_yield n2;
    };
    sus::iter::Iterator<const NoCopyMove&> auto it = x();
    static_assert(sus::iter::Iterator<decltype(it), const NoCopyMove&>);
    EXPECT_EQ(&it.next().unwrap(), &n3);
    EXPECT_EQ(&it.next().unwrap(), &n1);
    EXPECT_EQ(&it.next().unwrap(), &n2);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(IterGenerator, ForLoop) {
  auto x = []() -> Generator<i32> {
    co_yield 1;
    co_yield 2;
    co_yield 3;
    co_yield 4;
  };

  i32 e = 1;
  for (i32 i : x()) {
    EXPECT_EQ(e, i);
    e += 1;
  }
  EXPECT_EQ(e, 5);
}

TEST(IterGenerator, Nested) {
  auto y = []() -> Generator<i32> {
    co_yield 3;
    co_yield 4;
  };
  auto x = [y = std::move(y)]() -> Generator<i32> {
    co_yield 1;
    co_yield 2;
    for (auto i : y()) co_yield i;
  };
  i32 e = 1;
  for (i32 i : x()) {
    EXPECT_EQ(e, i);
    e += 1;
  }
  EXPECT_EQ(e, 5);
}

TEST(IterGenerator, ComposeFromGenerator) {
  auto x = []() -> Generator<i32> {
    co_yield 1;
    co_yield 2;
    co_yield 3;
    co_yield 4;
  };

  // Generator is trivially relocatable, so no need to box() it.
  sus::iter::Iterator<i32> auto it =
      x().filter([](const i32& a) { return a > 1 && a < 4; });
  EXPECT_EQ(it.next().unwrap(), 2);
  EXPECT_EQ(it.next().unwrap(), 3);
  EXPECT_EQ(it.next(), sus::None);
}

TEST(IterGenerator, ComposeIntoGenerator) {
  auto x = [](sus::iter::Iterator<i32> auto it) -> Generator<i32> {
    for (i32 i : it) {
      if (i > 1 && i < 4) co_yield i;
    }
  };

  sus::iter::Iterator<i32> auto it =
      sus::vec(1, 2, 3, 4).construct<i32>().into_iter().generate(x);
  EXPECT_EQ(it.next().unwrap(), 2);
  EXPECT_EQ(it.next().unwrap(), 3);
  EXPECT_EQ(it.next(), sus::None);
}

}  // namespace
