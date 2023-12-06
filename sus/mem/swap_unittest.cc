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

#include "sus/mem/swap.h"

#include <type_traits>

#include "googletest/include/gtest/gtest.h"
#include "sus/collections/array.h"
#include "sus/macros/builtin.h"
#include "sus/mem/relocate.h"
#include "sus/ops/range_literals.h"
#include "sus/prelude.h"

namespace {

TEST(Swap, ConstexprTrivialRelocate) {
  using T = int;
  static_assert(sus::mem::relocate_by_memcpy<T>, "");

  auto i = []() constexpr {
    T i(2);
    T j(5);
    sus::mem::swap(i, j);
    return i;
  };
  auto j = []() constexpr {
    T i(2);
    T j(5);
    sus::mem::swap(i, j);
    return j;
  };
  static_assert(i() == T(5), "");
  static_assert(j() == T(2), "");
}

TEST(Swap, ConstexprTrivialAbi) {
  struct [[_sus_trivial_abi]] S {
    constexpr S(int n) : num(n) {}
    constexpr S(S&& other) : num(other.num), moves(other.moves + 1) {}
    constexpr void operator=(S&& other) { num = other.num; }
    int num;
    int moves = 0;
  };
  // This means `S` is only "trivially relocatable" if achieved through
  // [[_sus_trivial_abi]].
  static_assert(!std::is_trivially_move_constructible_v<S>, "");
  static_assert(
      sus::mem::relocate_by_memcpy<S> == __has_extension(trivially_relocatable),
      "");

  auto i = []() constexpr {
    S i(2);
    S j(5);
    sus::mem::swap(i, j);
    return i;
  };
  auto j = []() constexpr {
    S i(2);
    S j(5);
    sus::mem::swap(i, j);
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
  static_assert(!sus::mem::relocate_by_memcpy<S>, "");

  auto i = []() constexpr {
    S i(2);
    S j(5);
    sus::mem::swap(i, j);
    return i;
  };
  auto j = []() constexpr {
    S i(2);
    S j(5);
    sus::mem::swap(i, j);
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
  static_assert(sus::mem::relocate_by_memcpy<T>, "");

  T i(2);
  T j(5);
  sus::mem::swap(i, j);
  EXPECT_EQ(i, T(5));
  EXPECT_EQ(j, T(2));
}

TEST(Swap, TrivialAbi) {
  struct [[_sus_trivial_abi]] S {
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
  // [[_sus_trivial_abi]].
  static_assert(!std::is_trivially_move_constructible_v<S>, "");
  static_assert(
      sus::mem::relocate_by_memcpy<S> == __has_extension(trivially_relocatable),
      "");

  S i(2);
  S j(5);
  sus::mem::swap(i, j);
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
  static_assert(!sus::mem::relocate_by_memcpy<S>, "");

  S i(2);
  S j(5);
  sus::mem::swap(i, j);
  EXPECT_EQ(i.num, 5);
  EXPECT_EQ(j.num, 2);
  // The swap was done by move operations.
  EXPECT_GE(i.moves, 1);
  EXPECT_GE(j.moves, 1);
}

struct Trivial {
  explicit Trivial(sus::Array<i32, 100> i) : num(sus::move(i)) {}
  Trivial(Trivial&&) { moves += 1u; }
  Trivial& operator=(Trivial&&) {
    moves += 1u;
    return *this;
  }
  sus::Array<i32, 100> num;

  static usize moves;

  sus_class_trivially_relocatable_unchecked(unsafe_fn);
};

usize Trivial::moves;

TEST(Swap, Alias) {
  static usize moves;
  struct S {
    explicit S(i32 i) : num(i) {}
    S(S&&) { moves += 1u; }
    S& operator=(S&&) {
      moves += 1u;
      return *this;
    }
    i32 num;
  };

  S i(2);
  sus::mem::swap(i, i);
  EXPECT_EQ(moves, 0u);

  Trivial::moves = 0u;
  Trivial t(sus::Array<i32, 100>::with_initializer(
      [i = 0_i32]() mutable { return sus::mem::replace(i, i + 1); }));
  sus::mem::swap(t, t);
  for (usize j : "0..100"_r) {
    EXPECT_EQ(t.num[j], i32::try_from(j).unwrap());
  }
  EXPECT_EQ(Trivial::moves, 0u);
}

TEST(Swap, NoAliasUnchecked) {
  static usize moves;
  struct S {
    explicit S(i32 i) : num(i) {}
    S(S&&) { moves += 1u; }
    S& operator=(S&&) {
      moves += 1u;
      return *this;
    }
    i32 num;
  };

  S i1(2);
  S i2(3);
  sus::mem::swap_nonoverlapping(unsafe_fn, i1, i2);
  EXPECT_EQ(moves, 3u);

  Trivial::moves = 0u;
  Trivial t1(sus::Array<i32, 100>::with_initializer(
      [i = 0_i32]() mutable { return sus::mem::replace(i, i + 1); }));
  Trivial t2(sus::Array<i32, 100>::with_initializer(
      [i = 10_i32]() mutable { return sus::mem::replace(i, i + 1); }));
  sus::mem::swap_nonoverlapping(unsafe_fn, t1, t2);
  for (usize j : "0..100"_r) {
    EXPECT_EQ(t1.num[j], i32::try_from(j).unwrap() + 10);
    EXPECT_EQ(t2.num[j], i32::try_from(j).unwrap());
  }
  EXPECT_EQ(Trivial::moves, 0u);
}

}  // namespace
