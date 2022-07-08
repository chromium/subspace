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

#include "mem/mref.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::Mref;

// Require conversion through mref().
static_assert(!std::is_convertible_v<Mref<int&>, int&>, "");
static_assert(
    std::is_convertible_v<Mref<int&>, decltype(mref(std::declval<int&>()))>,
    "");

// Require conversion through mref() even if you have an Mref already.
static_assert(!std::is_convertible_v<Mref<int&>, Mref<int&>&>, "");
static_assert(std::is_convertible_v<
                  Mref<int&>, decltype(mref(std::declval<Mref<int&>&>()))>,
              "");

void increment(Mref<int&> i) { ++i; }

TEST(Mref, Pass) {
  int i = 0;
  increment(mref(i));
  EXPECT_EQ(i, 1);
}

TEST(Mref, PassMref) {
  auto f = [](Mref<int&> i) { increment(mref(i)); };
  int i = 0;
  f(mref(i));
  EXPECT_EQ(i, 1);
}

TEST(Mref, Convertible) {
  int i = 3;
  Mref<int&> m = mref(i);
  int& j = m;
  j++;  // Increments `i` too.
  EXPECT_EQ(i, 4);
}

TEST(Mref, AssignConstRef) {
  int i = 3;
  Mref<int&> m = mref(i);
  int j = 4;
  m = j;
  EXPECT_EQ(i, 4);
}

TEST(Mref, AssignRvalueRef) {
  int i = 3;
  Mref<int&> m = mref(i);
  m = 4;
  EXPECT_EQ(i, 4);
}

}  // namespace
