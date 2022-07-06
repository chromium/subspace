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

#pragma once

#include "tuple/tuple.h"

#include <concepts>

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::tuple::Tuple;

TEST(Tuple, With) {
  auto t1 = Tuple<int>::with(2);
  auto t2 = Tuple<int, float>::with(2, 3.f);
  auto t3 = Tuple<int, float, int>::with(2, 3.f, 4);

  [[maybe_unused]] constexpr auto c = Tuple<int, float>::with(2, 3.f);
}

TEST(Tuple, Get) {
  auto t1 = Tuple<int>::with(2);
  EXPECT_EQ(t1.get<0>(), 2);
  static_assert(std::same_as<const int&, decltype(t1.get<0>())>);

  const auto t2 = Tuple<int, float>::with(2, 3.f);
  EXPECT_EQ(t2.get<0>(), 2);
  static_assert(std::same_as<const int&, decltype(t2.get<0>())>);
  EXPECT_EQ(t2.get<1>(), 3.f);
  static_assert(std::same_as<const float&, decltype(t2.get<1>())>);

  auto t3 = Tuple<int, float, int>::with(2, 3.f, 4);
  EXPECT_EQ(t3.get<0>(), 2);
  static_assert(std::same_as<const int&, decltype(t3.get<0>())>);
  EXPECT_EQ(t3.get<1>(), 3.f);
  static_assert(std::same_as<const float&, decltype(t3.get<1>())>);
  EXPECT_EQ(t3.get<2>(), 4);
  static_assert(std::same_as<const int&, decltype(t3.get<2>())>);

  constexpr auto c = Tuple<int, float>::with(2, 3.f);
  [[maybe_unused]] constexpr auto& c0 = c.get<0>();
  [[maybe_unused]] constexpr auto& c1 = c.get<1>();
}

TEST(Tuple, GetMut) {
  auto t1 = Tuple<int>::with(2);
  EXPECT_EQ(t1.get_mut<0>(), 2);
  t1.get_mut<0>() += 1;
  EXPECT_EQ(t1.get_mut<0>(), 3);
  static_assert(std::same_as<int&, decltype(t1.get_mut<0>())>);

  auto t2 = Tuple<int, float>::with(2, 3.f);
  EXPECT_EQ(t2.get_mut<0>(), 2);
  t2.get_mut<0>() += 1;
  EXPECT_EQ(t2.get_mut<0>(), 3);
  static_assert(std::same_as<int&, decltype(t2.get_mut<0>())>);
  EXPECT_EQ(t2.get_mut<1>(), 3.f);
  t2.get_mut<1>() += 1.f;
  EXPECT_EQ(t2.get_mut<1>(), 4.f);
  static_assert(std::same_as<float&, decltype(t2.get_mut<1>())>);

  auto t3 = Tuple<int, float, int>::with(2, 3.f, 4);
  EXPECT_EQ(t3.get_mut<0>(), 2);
  t3.get_mut<0>() += 1;
  EXPECT_EQ(t3.get_mut<0>(), 3);
  static_assert(std::same_as<int&, decltype(t3.get_mut<0>())>);
  EXPECT_EQ(t3.get_mut<1>(), 3.f);
  t3.get_mut<1>() += 1.f;
  EXPECT_EQ(t3.get_mut<1>(), 4.f);
  static_assert(std::same_as<float&, decltype(t3.get_mut<1>())>);
  EXPECT_EQ(t3.get_mut<2>(), 4);
  t3.get_mut<2>() += 1;
  EXPECT_EQ(t3.get_mut<2>(), 5);
  static_assert(std::same_as<int&, decltype(t3.get_mut<2>())>);
}

TEST(Tuple, Unwrap) {
  {
    auto make = []() { return Tuple<int>::with(2); };
    EXPECT_EQ(make().unwrap<0>(), 2);
    static_assert(std::same_as<int, decltype(make().unwrap<0>())>);
  }
  {
    auto make = []() { return Tuple<int, float>::with(2, 3.f); };
    EXPECT_EQ(make().unwrap<0>(), 2);
    static_assert(std::same_as<int, decltype(make().unwrap<0>())>);
    EXPECT_EQ(make().unwrap<1>(), 3.f);
    static_assert(std::same_as<float, decltype(make().unwrap<1>())>);
  }
  {
    auto make = []() { return Tuple<int, float, int>::with(2, 3.f, 4); };
    EXPECT_EQ(make().unwrap<0>(), 2);
    static_assert(std::same_as<int, decltype(make().unwrap<0>())>);
    EXPECT_EQ(make().unwrap<1>(), 3.f);
    static_assert(std::same_as<float, decltype(make().unwrap<1>())>);
    EXPECT_EQ(make().unwrap<2>(), 4);
    static_assert(std::same_as<int, decltype(make().unwrap<2>())>);
  }

  static int counter = 0;
  struct MoveCounter {
    MoveCounter() = default;
    MoveCounter(MoveCounter&&) { counter++; }
  };
  auto m = Tuple<MoveCounter>::with(MoveCounter()).unwrap<0>();
  EXPECT_EQ(counter, 2);  // One move into the Tuple, and one move out.

  [[maybe_unused]] constexpr auto c0 =
      Tuple<int, float>::with(2, 3.f).unwrap<0>();
  [[maybe_unused]] constexpr auto c1 =
      Tuple<int, float>::with(2, 3.f).unwrap<1>();
}

TEST(TupleDeathTest, Moved) {
#if GTEST_HAS_DEATH_TEST
  auto m1 = Tuple<int>::with(1);
  static_cast<decltype(m1)&&>(m1).unwrap<0>();
  EXPECT_DEATH(m1.get<0>(), "");

  auto m2 = Tuple<int>::with(1);
  static_cast<decltype(m2)&&>(m2).unwrap<0>();
  EXPECT_DEATH(m2.get_mut<0>(), "");

  auto m3 = Tuple<int>::with(1);
  static_cast<decltype(m3)&&>(m3).unwrap<0>();
  EXPECT_DEATH(static_cast<decltype(m3)&&>(m3).unwrap<0>(), "");
#endif
}

}  // namespace
