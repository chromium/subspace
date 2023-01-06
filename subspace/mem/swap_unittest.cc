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

#include "subspace/mem/swap.h"

#include <type_traits>

#include "googletest/include/gtest/gtest.h"
#include "subspace/macros/builtin.h"
#include "subspace/mem/relocate.h"

namespace sus::mem {
namespace {

TEST(Swap, ConstexprTrivialRelocate) {
  using T = int;
  static_assert(relocate_one_by_memcpy<T>, "");

  auto i = []() constexpr {
    T i(2);
    T j(5);
    ::sus::mem::swap(mref(i), mref(j));
    return i;
  };
  auto j = []() constexpr {
    T i(2);
    T j(5);
    ::sus::mem::swap(mref(i), mref(j));
    return j;
  };
  static_assert(i() == T(5), "");
  static_assert(j() == T(2), "");
}

TEST(Swap, ConstexprTrivialAbi) {
  struct [[sus_trivial_abi]] S {
    constexpr S(int n) : num(n) {}
    constexpr S(S&& other) : num(other.num), moves(other.moves + 1) {}
    constexpr void operator=(S&& other) { num = other.num; }
    int num;
    int moves = 0;
  };
  // This means `S` is only "trivially relocatable" if achieved through
  // [[sus_trivial_abi]].
  static_assert(!std::is_trivially_move_constructible_v<S>, "");
  static_assert(
      relocate_one_by_memcpy<S> == __has_extension(trivially_relocatable), "");

  auto i = []() constexpr {
    S i(2);
    S j(5);
    ::sus::mem::swap(mref(i), mref(j));
    return i;
  };
  auto j = []() constexpr {
    S i(2);
    S j(5);
    ::sus::mem::swap(mref(i), mref(j));
    return j;
  };
  static_assert(i().num == 5, "");
  static_assert(j().num == 2, "");
  // The swap was done by move operations, since memcpy is not constexpr.
  static_assert(i().moves == 1, "");
  static_assert(j().moves == 1, "");
}

TEST(Swap, ConstexprNonTrivial) {
  struct S {
    constexpr S(int n) : num(n) {}
    constexpr S(S&& other) : num(other.num), moves(other.moves + 1) {}
    constexpr void operator=(S&& other) { num = other.num; }
    int num;
    int moves = 0;
  };
  static_assert(!relocate_one_by_memcpy<S>, "");

  auto i = []() constexpr {
    S i(2);
    S j(5);
    ::sus::mem::swap(mref(i), mref(j));
    return i;
  };
  auto j = []() constexpr {
    S i(2);
    S j(5);
    ::sus::mem::swap(mref(i), mref(j));
    return j;
  };
  static_assert(i().num == 5, "");
  static_assert(j().num == 2, "");
  // The swap was done by move operations, since memcpy is not constexpr.
  static_assert(i().moves == 1, "");
  static_assert(j().moves == 1, "");
}

TEST(Swap, TrivialRelocate) {
  using T = int;
  static_assert(relocate_one_by_memcpy<T>, "");

  T i(2);
  T j(5);
  ::sus::mem::swap(mref(i), mref(j));
  EXPECT_EQ(i, T(5));
  EXPECT_EQ(j, T(2));
}

TEST(Swap, TrivialAbi) {
  struct [[sus_trivial_abi]] S {
    constexpr S(int n) : num(n) {}
    constexpr S(S&& other) : num(other.num), moves(other.moves + 1) {}
    constexpr void operator=(S&& other) {
      num = other.num;
      moves = other.moves + 1;
    }
    int num;
    int moves = 0;
  };
  // This means `S` is only "trivially relocatable" if achieved through
  // [[sus_trivial_abi]].
  static_assert(!std::is_trivially_move_constructible_v<S>, "");
  static_assert(
      relocate_one_by_memcpy<S> == __has_extension(trivially_relocatable), "");

  S i(2);
  S j(5);
  ::sus::mem::swap(mref(i), mref(j));
  EXPECT_EQ(i.num, 5);
  EXPECT_EQ(j.num, 2);
#if __has_extension(trivially_relocatable)
  // The swap was done by memcpy.
  EXPECT_EQ(i.moves, 0);
#else
  // The swap was done by move operations.
  EXPECT_GE(i.moves, 1);
#endif
}

TEST(Swap, NonTrivial) {
  struct S {
    constexpr S(int n) : num(n) {}
    constexpr S(S&& other) : num(other.num), moves(other.moves + 1) {}
    void operator=(S&& other) {
      num = other.num;
      moves = other.moves + 1;
    }
    int num;
    int moves = 0;
  };
  static_assert(!relocate_one_by_memcpy<S>, "");

  S i(2);
  S j(5);
  ::sus::mem::swap(mref(i), mref(j));
  EXPECT_EQ(i.num, 5);
  EXPECT_EQ(j.num, 2);
  // The swap was done by move operations.
  EXPECT_GE(i.moves, 1);
  EXPECT_GE(j.moves, 1);
}

}  // namespace
}  // namespace sus::mem
