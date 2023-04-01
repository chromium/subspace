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

#include "subspace/mem/replace.h"

#include <type_traits>

#include "googletest/include/gtest/gtest.h"
#include "subspace/macros/builtin.h"
#include "subspace/mem/relocate.h"
#include "subspace/prelude.h"

using sus::mem::replace;
using sus::mem::replace_ptr;

namespace {

TEST(Replace, ConstexprTrivialCopy) {
  using T = int;
  static_assert(std::is_trivially_copyable_v<T>, "");

  auto i = []() constexpr {
    T i(2);
    [[maybe_unused]] T j(::sus::mem::replace(mref(i), T(5)));
    return i;
  };
  auto j = []() constexpr {
    T i(2);
    T j(::sus::mem::replace(mref(i), T(5)));
    return j;
  };
  static_assert(i() == T(5), "");
  static_assert(j() == T(2), "");
}

TEST(Replace, ConstexprNonTrivial) {
  struct S {
    constexpr S(int n) : num(n) {}
    constexpr S(S&& other) : num(other.num), assigns(other.assigns) {}
    constexpr void operator=(S&& other) {
      num = other.num;
      assigns = other.assigns + 1;
    }
    int num;
    int assigns = 0;
  };
  static_assert(!std::is_trivially_copyable_v<S>, "");

  auto i = []() constexpr {
    S i(2);
    S j(::sus::mem::replace(mref(i), S(5)));
    return i;
  };
  auto j = []() constexpr {
    S i(2);
    S j(::sus::mem::replace(mref(i), S(5)));
    return j;
  };
  static_assert(i().num == 5, "");
  static_assert(j().num == 2, "");
  // The replace was done by move operations, since memcpy is not constexpr.
  static_assert(i().assigns == 1, "");
}

TEST(Replace, TrivialCopy) {
  using T = int;
  static_assert(std::is_trivially_copyable_v<T>, "");

  T i(2);
  T j(::sus::mem::replace(mref(i), T(5)));
  EXPECT_EQ(i, T(5));
  EXPECT_EQ(j, T(2));

  T lvalue(6);

  T k(::sus::mem::replace(mref(i), lvalue));
  EXPECT_EQ(i, T(6));
  EXPECT_EQ(k, T(5));
}

TEST(Replace, NonTrivial) {
  struct S {
    constexpr S(int n) : num(n) {}
    constexpr S(const S&) : num(999), assigns(999) {}
    constexpr S(S&& other) : num(other.num), assigns(other.assigns) {}
    constexpr S& operator=(const S& other) {
      num = other.num;
      assigns = other.assigns + 1;
      return *this;
    }
    constexpr S& operator=(S&& other) {
      num = other.num;
      assigns = other.assigns + 1;
      return *this;
    }
    int num;
    int assigns = 0;
  };
  static_assert(!std::is_trivially_copyable_v<S>, "");

  S i(2);
  S j(::sus::mem::replace(mref(i), S(5)));
  EXPECT_EQ(i.num, 5);
  EXPECT_EQ(j.num, 2);
  // The replace was done by move operations.
  EXPECT_EQ(1, i.assigns);

  S lvalue(6);

  i.assigns = 0;
  S k(::sus::mem::replace(mref(i), lvalue));
  EXPECT_EQ(i.num, 6);
  EXPECT_EQ(k.num, 5);
  // The replace was done by move operations.
  EXPECT_EQ(1, i.assigns);
}

TEST(Replace, Convertible) {
  static i32 converted = 0;

  struct Target {};
  struct [[sus_trivial_abi]] Copyable {
    Copyable() = default;
    Copyable(const Copyable&) = default;
    Copyable& operator=(const Copyable&) = default;
    operator Target() const {
      converted += 1;
      return Target();
    }
  };
  struct [[sus_trivial_abi]] MoveableCopyConvert {
    MoveableCopyConvert() = default;
    MoveableCopyConvert(MoveableCopyConvert&&) = default;
    MoveableCopyConvert& operator=(MoveableCopyConvert&&) = default;
    // Const conversion.
    operator Target() const& {
      converted += 1;
      return Target();
    }
  };
  struct [[sus_trivial_abi]] MoveableMoveConvert {
    MoveableMoveConvert() = default;
    MoveableMoveConvert(MoveableMoveConvert&&) = default;
    MoveableMoveConvert& operator=(MoveableMoveConvert&&) = default;
    // Move conversion.
    operator Target() && {
      converted += 1;
      return Target();
    }
  };

  Target target;

  // Tests the replace(T, const U&) path, ensuring we don't memcpy() from a
  // different type.
  converted = 0;
  [[maybe_unused]] auto a = ::sus::mem::replace(mref(target), Copyable());
  EXPECT_EQ(converted, 1);

  // Tests the replace(T, U&&) path, ensuring we don't memcpy() from a
  // different type, and that it successfully can use a const conversion
  // operator.
  converted = 0;
  [[maybe_unused]] auto b =
      ::sus::mem::replace(mref(target), MoveableCopyConvert());
  EXPECT_EQ(converted, 1);

  // Tests the replace(T, U&&) path, ensuring we don't memcpy() from a
  // different type, and that it successfully can use am rvalue conversion
  // operator.
  converted = 0;
  [[maybe_unused]] auto c =
      ::sus::mem::replace(mref(target), MoveableMoveConvert());
  EXPECT_EQ(converted, 1);
}

TEST(Replace, ConstPtr) {
  int i1 = 1, i2 = 2;
  const int* p1 = &i1;
  const int* p2 = &i2;
  const int* o = replace(mref(p1), p2);
  EXPECT_EQ(o, &i1);
  EXPECT_EQ(p1, &i2);
  EXPECT_EQ(p2, &i2);

  o = replace(mref(p1), nullptr);
  EXPECT_EQ(o, &i2);
  EXPECT_EQ(p1, nullptr);
}

TEST(Replace, MutPtr) {
  int i1 = 1, i2 = 2;
  int* p1 = &i1;
  int* p2 = &i2;
  int* o = replace(mref(p1), p2);
  EXPECT_EQ(o, &i1);
  EXPECT_EQ(p1, &i2);
  EXPECT_EQ(p2, &i2);

  o = replace(mref(p1), nullptr);
  EXPECT_EQ(o, &i2);
  EXPECT_EQ(p1, nullptr);
}

TEST(ReplaceDeathTest, Aliasing) {
  int i1 = 1;
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH([[maybe_unused]] auto x = replace(mref(i1), i1), "");
#endif

  struct S {
    int i;
    operator const int&() const { return i; }
  };
  auto s = S(0);
#if GTEST_HAS_DEATH_TEST
  // `s` converts to an `int` with the same address a `s.i`.
  EXPECT_DEATH([[maybe_unused]] auto x = replace(mref(s.i), s), "");
#endif
}

}  // namespace
