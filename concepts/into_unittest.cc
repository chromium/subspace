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

#include "concepts/into.h"

#include "mem/forward.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

using sus::concepts::from::From;
using sus::concepts::into::Into;
using sus::concepts::into::__private::IntoRef;

namespace {

struct S {
  int val = 5;
};

struct FromInt {
  static auto from(int) { return FromInt(); }
};

static_assert(Into<int, FromInt>);
static_assert(!Into<S, FromInt>);

template <class To, class From>
concept can_into = requires(From&& from) {
                     {
                       [](To) {}(sus::into(sus::forward<From>(from)))
                     };
                   };
// into() accepts const or mutable rvalues.
static_assert(!can_into<FromInt, const int&>);
static_assert(!can_into<FromInt, int&>);
static_assert(can_into<FromInt, int&&>);
static_assert(can_into<FromInt, const int&&>);

template <class To, class From>
concept can_move_into = requires(From&& from) {
                          {
                            [](To) {}(sus::move_into(sus::forward<From>(from)))
                          };
                        };
// move_into() accepts mutable lvalue or rvalue references.
static_assert(!can_move_into<FromInt, const int&>);
static_assert(can_move_into<FromInt, int&>);
static_assert(can_move_into<FromInt, int&&>);
static_assert(!can_move_into<FromInt, const int&&>);

struct Counter {
  Counter(int& copies, int& moves) : copies(copies), moves(moves) {}
  Counter(const Counter& c) : copies(c.copies), moves(c.moves) { copies += 1; }
  Counter(Counter&& c) : copies(c.copies), moves(c.moves) { moves += 1; }

  static auto from(Counter&& c) -> Counter {
    return Counter(static_cast<Counter&&>(c));
  }

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
  static auto from(int i) { return FromThings(i); }
  static auto from(S s) { return FromThings(s.val); }
  int got_value;
};

TEST(Into, Concept) {
  // F takes anything that FromThings can be constructed from.
  auto f = []<Into<FromThings> T>(T&& t) -> FromThings {
    return sus::move_into(t);
  };
  EXPECT_EQ(f(int(2)).got_value, 2);
  EXPECT_EQ(f(S(3)).got_value, 3);
}

}  // namespace
