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

#include "sus/construct/into.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/mem/forward.h"
#include "sus/prelude.h"
#include "sus/test/behaviour_types.h"

using sus::construct::From;
using sus::construct::Into;
using sus::construct::__private::IntoRef;
using namespace sus::test;

namespace {

struct S {
  sus_clang_bug_54040(constexpr inline S(int val) : val(val){});
  int val = 5;
};

struct FromInt {
  static auto from(int) { return FromInt(); }
};

static_assert(Into<int, FromInt>);
static_assert(!Into<S, FromInt>);

struct FromStuff {
  template <class T>
  static auto from(T) {
    return FromStuff();
  }
};

static_assert(Into<int, FromStuff>);
static_assert(Into<S, FromStuff>);

TEST(F, F) { FromStuff f = sus::into(TriviallyMoveableNotDestructible(2)); }

template <class To, class From>
concept CanMoveInto = requires(From&& from) {
  {
    [](To) {}(sus::move_into(sus::forward<From>(from)))
  };
};
// move_into() accepts mutable lvalue or rvalue references.
static_assert(!CanMoveInto<FromInt, const int&>);
static_assert(CanMoveInto<FromInt, int&>);
static_assert(CanMoveInto<FromInt, int&&>);
static_assert(!CanMoveInto<FromInt, const int&&>);

struct Counter {
  Counter(int& copies, int& moves) : copies(copies), moves(moves) {}
  Counter(const Counter& c) : copies(c.copies), moves(c.moves) { copies += 1; }
  Counter(Counter&& c) : copies(c.copies), moves(c.moves) { moves += 1; }

  int& copies;
  int& moves;
};

TEST(Into, Into) {
  int copies = 0;
  int moves = 0;

  [](Counter) {}(sus::into(Counter(copies, moves)));
  EXPECT_EQ(copies, 0);
  EXPECT_EQ(moves, 1);

  auto from = Counter(copies, moves);
  [](Counter) {}(sus::into(static_cast<Counter&&>(from)));
  EXPECT_EQ(copies, 0);
  EXPECT_EQ(moves, 2);
}

TEST(Into, MoveInto) {
  int copies = 0;
  int moves = 0;

  auto from = Counter(copies, moves);
  [](Counter) {}(sus::move_into(from));
  EXPECT_EQ(copies, 0);
  EXPECT_EQ(moves, 1);

  auto from2 = Counter(copies, moves);
  [](Counter) {}(sus::move_into(static_cast<Counter&&>(from2)));
  EXPECT_EQ(copies, 0);
  EXPECT_EQ(moves, 2);

  [](Counter) {}(sus::move_into(Counter(copies, moves)));
  EXPECT_EQ(copies, 0);
  EXPECT_EQ(moves, 3);
}

struct FromThings {
  sus_clang_bug_54040(constexpr inline FromThings(int i) : got_value(i){});
  static auto from(int i) { return FromThings(i); }
  static auto from(S s) { return FromThings(s.val); }
  int got_value;
};

TEST(Into, Concept) {
  // F takes anything that FromThings can be constructed from.
  auto f = []<Into<FromThings> T>(T&& t) -> FromThings {
    return sus::move_into(t);
  };
  EXPECT_EQ(FromThings(1).got_value, 1);
  EXPECT_EQ(f(int(2)).got_value, 2);
  EXPECT_EQ(f(S(3)).got_value, 3);
}

TEST(TryInto, Example) {
  auto valid = sus::try_into<u8>(123_i32).unwrap_or_default();
  sus_check(valid == 123u);
  auto invalid = sus::try_into<u8>(-1_i32).unwrap_or_default();
  sus_check(invalid == 0u);
}

TEST(Into, Ref) {
  struct S {
    S() {}
    S(const S&&) {}
    S(S&&) {}
  };
  S s;

  // S&.
  sus::Option<S&> m = sus::into(s);
  EXPECT_EQ(&m.as_value(), &s);

  // S& to const S&.
  sus::Option<const S&> m2 = sus::into(s);
  EXPECT_EQ(&m2.as_value(), &s);

  // const S&.
  sus::Option<const S&> c = sus::into(s);
  EXPECT_EQ(&c.as_value(), &s);
}

}  // namespace
