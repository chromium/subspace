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

// TODO: Overload all && functions with const& version if T is copyable? The
// latter copies T instead of moving it. This could lead to a lot of unintended
// copies if expensive types have copy constructors, which is common in
// preexisting C++ code since there's no concept of Clone there (which will TBD
// in this library). So it's not clear if this is the right thing to do
// actually, needs thought.

#include "result/result.h"

#include "mem/move.h"
#include "num/types.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::result::Result;

struct Error {};

TEST(Result, With) {
  constexpr auto i = 4_i32;
  {
    constexpr auto a = Result<i32, Error>::with(3_i32);
    constexpr auto b = Result<i32, Error>::with(i);
  }
  {
    auto j = 4_i32;
    auto a = Result<i32, Error>::with(3_i32);
    auto b = Result<i32, Error>::with(i);
    auto c = Result<i32, Error>::with(mref(j));
  }
}

TEST(Result, WithErr) {
  constexpr auto e = Error();
  {
    constexpr auto a = Result<i32, Error>::with_err(Error());
    constexpr auto b = Result<i32, Error>::with_err(e);
  }
  {
    auto f = Error();
    auto a = Result<i32, Error>::with_err(Error());
    auto b = Result<i32, Error>::with_err(e);
    auto c = Result<i32, Error>::with_err(mref(f));
  }
}

TEST(Result, IsOk) {
  constexpr bool a = Result<i32, Error>::with(3_i32).is_ok();
  EXPECT_TRUE(a);

  constexpr bool b = Result<i32, Error>::with_err(Error()).is_ok();
  EXPECT_FALSE(b);
}

TEST(Result, IsErr) {
  constexpr bool a = Result<i32, Error>::with(3_i32).is_err();
  EXPECT_FALSE(a);

  constexpr bool b = Result<i32, Error>::with_err(Error()).is_err();
  EXPECT_TRUE(b);
}

TEST(Result, Switch) {
  switch (Result<i32, Error>::with(3_i32)) {
    case sus::result::Ok: break;
    case sus::result::Err: ADD_FAILURE(); break;
  }

  switch (Result<i32, Error>::with_err(Error())) {
    case sus::result::Ok: ADD_FAILURE(); break;
    case sus::result::Err: break;
  }
}

TEST(Result, Ok) {
  static auto t_destructed = 0_usize;
  static auto e_destructed = 0_usize;
  struct T {
    ~T() { t_destructed += 1u; }
  };
  struct E {
    ~E() { e_destructed += 1u; }
  };

  {
    auto r = Result<T, E>::with(T());
    t_destructed = e_destructed = 0u;
    auto o = sus::move(r).ok();
    EXPECT_EQ(o, sus::Some);
    // We destroyed the T in Result<T, E>.
    EXPECT_GE(t_destructed, 1u);
    EXPECT_EQ(e_destructed, 0u);
  }

  {
    auto r = Result<T, E>::with_err(E());
    t_destructed = e_destructed = 0u;
    auto o = sus::move(r).ok();
    EXPECT_EQ(o, sus::None);
    // We destroyed the E in Result<T, E>.
    EXPECT_EQ(t_destructed, 0u);
    EXPECT_EQ(e_destructed, 1u);
  }
}

TEST(Result, Err) {
  static auto t_destructed = 0_usize;
  static auto e_destructed = 0_usize;
  struct T {
    ~T() { t_destructed += 1u; }
  };
  struct E {
    ~E() { e_destructed += 1u; }
  };

  {
    auto r = Result<T, E>::with_err(E());
    t_destructed = e_destructed = 0u;
    auto o = sus::move(r).err();
    EXPECT_EQ(o, sus::Some);
    // We destroyed the E in Result<T, E>.
    EXPECT_GE(e_destructed, 1u);
    EXPECT_EQ(t_destructed, 0u);
  }

  {
    auto r = Result<T, E>::with(T());
    t_destructed = e_destructed = 0u;
    auto o = sus::move(r).err();
    EXPECT_EQ(o, sus::None);
    // We destroyed the T in Result<T, E>.
    EXPECT_EQ(e_destructed, 0u);
    EXPECT_EQ(t_destructed, 1u);
  }
}

TEST(Result, Unwrap) {
  constexpr auto a = Result<i32, Error>::with(3_i32).unwrap();
  static_assert(std::same_as<decltype(a), const i32>);
  EXPECT_EQ(a, 3_i32);
}

TEST(ResultDeathTest, UnwrapWithErr) {
#if GTEST_HAS_DEATH_TEST
  auto r = Result<i32, Error>::with_err(Error());
  EXPECT_DEATH(sus::move(r).unwrap(), "");
#endif
}

TEST(Result, UnwrapErr) {
  constexpr auto a = Result<i32, Error>::with_err(Error()).unwrap_err();
  static_assert(std::same_as<decltype(a), const Error>);
}

TEST(ResultDeathTest, UnwrapErrWithOk) {
#if GTEST_HAS_DEATH_TEST
  auto r = Result<i32, Error>::with(3_i32);
  EXPECT_DEATH(sus::move(r).unwrap_err(), "");
#endif
}

TEST(Result, Copy) {
  auto r = Result<i32, i32>::with(1_i32);
  auto r2 = r;
  EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);
  r2 = r;
  EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);
}

TEST(Result, CopyAfterTrivialMove) {
  // Trivial copy won't catch use-after-move.
  auto r = Result<i32, i32>::with(1_i32);
  auto r3 = sus::move(r);
  auto r2 = r;
  EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);
}

struct NonTrivialCopy {
  NonTrivialCopy() {}
  NonTrivialCopy(const NonTrivialCopy&) {}
  NonTrivialCopy& operator=(const NonTrivialCopy&) { return *this; }
};

TEST(Result, CopyAfterNonTrivialMove) {
  auto r = Result<NonTrivialCopy, i32>::with(NonTrivialCopy());
  sus::move(r).unwrap();
  EXPECT_DEATH([[maybe_unused]] auto r2 = r, "");
}

TEST(Result, Move) {
  auto r = Result<i32, i32>::with(1_i32);
  auto r2 = sus::move(r);
  EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);
  r2 = Result<i32, i32>::with(2_i32);
  EXPECT_EQ(sus::move(r2).unwrap(), 2_i32);
}

TEST(Result, MoveAfterTrivialMove) {
  // Trivial move won't catch use-after-move.
  auto r = Result<i32, i32>::with(1_i32);
  auto r3 = sus::move(r);
  auto r2 = sus::move(r);
  EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);
}

struct NonTrivialMove {
  NonTrivialMove() {}
  NonTrivialMove(NonTrivialMove&&) {}
  NonTrivialMove& operator=(NonTrivialMove&&) { return *this; }
};

TEST(Result, MoveAfterNonTrivialMove) {
  auto r = Result<NonTrivialMove, i32>::with(NonTrivialMove());
  sus::move(r).unwrap();
  EXPECT_DEATH([[maybe_unused]] auto r2 = sus::move(r), "");
}

}  // namespace
