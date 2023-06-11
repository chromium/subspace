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

#include "subspace/result/result.h"

#include <sstream>

#include "fmt/std.h"
#include "googletest/include/gtest/gtest.h"
#include "subspace/containers/array.h"
#include "subspace/iter/iterator.h"
#include "subspace/iter/once.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/move.h"
#include "subspace/num/types.h"
#include "subspace/prelude.h"
#include "subspace/test/behaviour_types.h"

namespace {

using sus::Result;
using sus::test::NotTriviallyRelocatableCopyableOrMoveable;
using sus::test::TriviallyCopyable;
using sus::test::TriviallyMoveableAndRelocatable;

struct TailPadding {
  i64 i;
  i32 j;
  // 4 bytes of tail padding, which the Result can use for its own state.
};
// MSVC doesn't take advantage of `[[no_unique_address]]`.
static_assert(sizeof(Result<i32, TailPadding>) ==
              sizeof(TailPadding) + sus_if_msvc_else(8, 0));

struct Error {};

static_assert(::sus::mem::Copy<Result<int, int>>);
static_assert(::sus::mem::Move<Result<int, int>>);
static_assert(sus::mem::Copy<Result<i32, i32>>);
static_assert(sus::mem::Move<Result<i32, i32>>);
static_assert(sus::mem::Clone<Result<i32, i32>>);

static_assert(!sus::mem::Copy<Result<i32, TriviallyMoveableAndRelocatable>>);
static_assert(sus::mem::Move<Result<i32, TriviallyMoveableAndRelocatable>>);
static_assert(!sus::mem::Clone<Result<i32, TriviallyMoveableAndRelocatable>>);

static_assert(!sus::mem::Copy<Result<TriviallyMoveableAndRelocatable, i32>>);
static_assert(sus::mem::Move<Result<TriviallyMoveableAndRelocatable, i32>>);
static_assert(!sus::mem::Clone<Result<TriviallyMoveableAndRelocatable, i32>>);

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
    auto c = Result<i32, Error>::with(j);
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

TEST(Result, OkHelpers) {
  auto a = Result<i32, u32>::with(2_i32);
  Result<i32, u32> a2 = sus::ok(2_i32);
  EXPECT_EQ(a, a2);

  auto i = 2_i32;
  auto c = Result<i32, u32>::with(i);
  Result<i32, u32> c2 = sus::ok(i);
  EXPECT_EQ(c, c2);

  const auto ci = 2_i32;
  const auto cc = Result<i32, u32>::with(ci);
  Result<i32, u32> cc2 = sus::ok(ci);
  EXPECT_EQ(cc, cc2);

  // Verify no copies happen in the marker.
  {
    static i32 copies;
    struct S {
      S() {}
      S(const S&) { copies += 1; }
      S& operator=(const S&) {
        copies += 1;
        return *this;
      }
    };
    copies = 0;
    S s;
    auto marker = sus::ok(s);
    EXPECT_EQ(copies, 0);
    Result<S, u32> r = sus::move(marker);
    EXPECT_GE(copies, 1);
  }

  // In place explicit construction.
  {
    auto r = sus::ok(2_i32).construct<u32>();
    static_assert(std::same_as<decltype(r), Result<i32, u32>>);
    EXPECT_EQ(sus::move(r).unwrap(), 2_i32);
  }
}

TEST(Result, ErrHelpers) {
  auto a = Result<u32, i32>::with_err(2_i32);
  Result<u32, i32> a2 = sus::err(2_i32);
  EXPECT_EQ(a, a2);

  auto i = 2_i32;
  auto c = Result<u32, i32>::with_err(i);
  Result<u32, i32> c2 = sus::err(i);
  EXPECT_EQ(c, c2);

  const auto ci = 2_i32;
  const auto cc = Result<u32, i32>::with_err(ci);
  Result<u32, i32> cc2 = sus::err(ci);
  EXPECT_EQ(cc, cc2);

  // Verify no copies happen in the marker.
  {
    static i32 copies;
    struct S {
      S() {}
      S(const S&) { copies += 1; }
      S& operator=(const S&) {
        copies += 1;
        return *this;
      }
    };
    copies = 0;
    S s;
    auto marker = sus::err(s);
    EXPECT_EQ(copies, 0);
    Result<u32, S> r = sus::move(marker);
    EXPECT_GE(copies, 1);
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
    case sus::Ok: break;
    case sus::Err: ADD_FAILURE(); break;
  }

  switch (Result<i32, Error>::with_err(Error())) {
    case sus::Ok: ADD_FAILURE(); break;
    case sus::Err: break;
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

  Result<void, Error>::with().unwrap();  // Returns void, doesn't panic.
  static_assert(
      std::same_as<decltype(Result<void, Error>::with().unwrap()), void>);
}

TEST(ResultDeathTest, UnwrapWithErr) {
#if GTEST_HAS_DEATH_TEST
  auto r = Result<i32, Error>::with_err(Error());
  EXPECT_DEATH(sus::move(r).unwrap(), "");

  auto r2 = Result<void, Error>::with_err(Error());
  EXPECT_DEATH(sus::move(r2).unwrap(), "");
#endif
}

TEST(Result, UnwrapErr) {
  constexpr auto a = Result<i32, Error>::with_err(Error()).unwrap_err();
  static_assert(std::same_as<decltype(a), const Error>);

  constexpr auto b = Result<void, Error>::with_err(Error()).unwrap_err();
  static_assert(std::same_as<decltype(b), const Error>);
}

TEST(ResultDeathTest, UnwrapErrWithOk) {
#if GTEST_HAS_DEATH_TEST
  auto r = Result<i32, Error>::with(3_i32);
  EXPECT_DEATH(sus::move(r).unwrap_err(), "");

  auto r2 = Result<void, Error>::with();
  EXPECT_DEATH(sus::move(r).unwrap_err(), "");
#endif
}

TEST(Result, UnwrapOrElse) {
  constexpr auto a = Result<i32, Error>::with(3_i32).unwrap_or_else(
      [](Error) { return 4_i32; });
  static_assert(std::same_as<decltype(a), const i32>);
  EXPECT_EQ(a, 3_i32);

  constexpr auto b = Result<i32, Error>::with_err(Error()).unwrap_or_else(
      [](Error) constexpr { return 4_i32; });
  EXPECT_EQ(b, 4_i32);

  Result<void, Error>::with().unwrap_or_else(
      [](Error) {});  // Returns void, doesn't panic.
  static_assert(
      std::same_as<decltype(Result<void, Error>::with().unwrap_or_else(
                       [](Error) {})),
                   void>);

  Result<void, Error>::with_err(Error()).unwrap_or_else(
      [](Error) {});  // Returns void, doesn't panic.
}

TEST(Result, Move) {
  auto r = Result<i32, i32>::with(1_i32);
  auto r2 = sus::move(r);
  EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);
  r2 = Result<i32, i32>::with(2_i32);
  EXPECT_EQ(sus::move(r2).unwrap(), 2_i32);

  auto rv = Result<void, i32>::with();
  auto rv2 = sus::move(rv);
  EXPECT_TRUE(rv2.is_ok());
}

TEST(Result, MoveAfterTrivialMove) {
  // Trivial move won't catch use-after-move.
  auto r = Result<i32, i32>::with(1_i32);
  auto r3 = sus::move(r);
  auto r2 = sus::move(r);
  EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);

  auto rv = Result<void, i32>::with();
  auto rv3 = sus::move(rv);
  auto rv2 = sus::move(rv);
  EXPECT_TRUE(rv2.is_ok());
}

TEST(Result, AssignAfterTrivialMove) {
  // Trivial move won't catch use-after-move.
  auto r = Result<i32, i32>::with(1_i32);
  auto r3 = sus::move(r);
  r = sus::move(r3);
  EXPECT_EQ(sus::move(r).unwrap(), 1_i32);

  auto rv = Result<void, i32>::with();
  auto rv3 = sus::move(rv);
  rv = sus::move(rv3);
  EXPECT_TRUE(rv.is_ok());
}

struct NonTrivialMove {
  NonTrivialMove(i32 i) : i(i) {}
  NonTrivialMove(NonTrivialMove&& o) : i(o.i) {}
  NonTrivialMove& operator=(NonTrivialMove&& o) {
    i = o.i;
    return *this;
  }

  i32 i;
};

TEST(ResultDeathTest, MoveAfterNonTrivialMove) {
  auto r = Result<NonTrivialMove, i32>::with(NonTrivialMove(1));
  sus::move(r).unwrap();
  EXPECT_DEATH([[maybe_unused]] auto r2 = sus::move(r), "");
}

TEST(Result, AssignAfterNonTrivialMove) {
  auto r = Result<NonTrivialMove, i32>::with(NonTrivialMove(1));
  auto r3 = sus::move(r);
  r = sus::move(r3);
  EXPECT_EQ(sus::move(r).unwrap().i, 1_i32);
}

TEST(Result, MoveSelfAssign) {
  auto r = Result<TriviallyCopyable, i32>::with(TriviallyCopyable(1));
  r = sus::move(r);
  EXPECT_EQ(sus::move(r).unwrap().i, 1);

  auto rv = Result<void, i32>::with();
  rv = sus::move(rv);
  EXPECT_TRUE(rv.is_ok());

  auto s = Result<NotTriviallyRelocatableCopyableOrMoveable, i32>::with(
      NotTriviallyRelocatableCopyableOrMoveable(1));
  s = sus::move(s);
  EXPECT_EQ(sus::move(s).unwrap().i, 1);

  auto e = Result<i32, TriviallyCopyable>::with_err(TriviallyCopyable(1));
  e = sus::move(e);
  EXPECT_EQ(sus::move(e).unwrap_err().i, 1);

  auto f = Result<i32, NotTriviallyRelocatableCopyableOrMoveable>::with_err(
      NotTriviallyRelocatableCopyableOrMoveable(1));
  f = sus::move(f);
  EXPECT_EQ(sus::move(f).unwrap_err().i, 1);
}

TEST(Result, CopySelfAssign) {
  auto r = Result<TriviallyCopyable, i32>::with(TriviallyCopyable(1));
  r = r;
  EXPECT_EQ(sus::move(r).unwrap().i, 1);

  auto rv = Result<void, i32>::with();
  rv = rv;
  EXPECT_TRUE(rv.is_ok());

  auto s = Result<NotTriviallyRelocatableCopyableOrMoveable, i32>::with(
      NotTriviallyRelocatableCopyableOrMoveable(1));
  s = s;
  EXPECT_EQ(sus::move(s).unwrap().i, 1);

  auto e = Result<i32, TriviallyCopyable>::with_err(TriviallyCopyable(1));
  e = e;
  EXPECT_EQ(sus::move(e).unwrap_err().i, 1);

  auto f = Result<i32, NotTriviallyRelocatableCopyableOrMoveable>::with_err(
      NotTriviallyRelocatableCopyableOrMoveable(1));
  f = f;
  EXPECT_EQ(sus::move(f).unwrap_err().i, 1);
}

TEST(Result, CloneIntoSelfAssign) {
  auto r = Result<TriviallyCopyable, i32>::with(TriviallyCopyable(1));
  sus::clone_into(r, r);
  EXPECT_EQ(sus::move(r).unwrap().i, 1);

  auto v = Result<void, i32>::with();
  sus::clone_into(v, v);
  EXPECT_TRUE(v.is_ok());

  auto s = Result<NotTriviallyRelocatableCopyableOrMoveable, i32>::with(
      NotTriviallyRelocatableCopyableOrMoveable(1));
  sus::clone_into(s, s);
  EXPECT_EQ(sus::move(s).unwrap().i, 1);

  auto e = Result<i32, TriviallyCopyable>::with_err(TriviallyCopyable(1));
  sus::clone_into(e, e);
  EXPECT_EQ(sus::move(e).unwrap_err().i, 1);

  auto f = Result<i32, NotTriviallyRelocatableCopyableOrMoveable>::with_err(
      NotTriviallyRelocatableCopyableOrMoveable(1));
  sus::clone_into(f, f);
  EXPECT_EQ(sus::move(f).unwrap_err().i, 1);
}

TEST(Result, Iter) {
  auto x = Result<i32, u8>::with_err(2_u8);
  for ([[maybe_unused]] auto i : x.iter()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Result<i32, u8>::with(-4_i32);
  for (auto& i : y.iter()) {
    static_assert(std::is_same_v<decltype(i), const i32&>, "");
    EXPECT_EQ(i, -4_i32);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

TEST(Result, IterMut) {
  auto x = Result<i32, u8>::with_err(2_u8);
  for ([[maybe_unused]] auto i : x.iter_mut()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Result<i32, u8>::with(-3_i32);
  for (auto& i : y.iter_mut()) {
    static_assert(std::is_same_v<decltype(i), i32&>, "");
    EXPECT_EQ(i, -3_i32);
    i += 1;
    ++count;
  }
  EXPECT_EQ(sus::move(y).unwrap(), -2_i32);
}

struct MoveOnly {
  explicit MoveOnly(int i) : i(i) {}
  MoveOnly(MoveOnly&& o) : i(o.i) {}
  void operator=(MoveOnly&& o) { i = o.i; }

  int i;
};

TEST(Result, IntoIter) {
  auto x = Result<i32, u8>::with_err(2_u8);
  for ([[maybe_unused]] auto i : x.iter_mut()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Result<MoveOnly, u8>::with(MoveOnly(-3));
  for (auto m : sus::move(y).into_iter()) {
    static_assert(std::is_same_v<decltype(m), MoveOnly>, "");
    EXPECT_EQ(m.i, -3);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

TEST(Result, ImplicitIter) {
  auto x = Result<i32, u8>::with_err(2_u8);
  for ([[maybe_unused]] auto i : x) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Result<MoveOnly, u8>::with(MoveOnly(-3));
  for (const auto& m : y) {
    static_assert(std::is_same_v<decltype(m), const MoveOnly&>, "");
    EXPECT_EQ(m.i, -3);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

template <class T>
struct CollectSum {
  sus_clang_bug_54050(CollectSum(T sum) : sum(sum){});

  static constexpr CollectSum from_iter(
      sus::iter::IntoIterator<T> auto iter) noexcept {
    T sum = T();
    for (T t : sus::move(iter).into_iter()) sum += t;
    return CollectSum(sum);
  }

  T sum;
};
static_assert(sus::iter::FromIterator<CollectSum<usize>, usize>);

TEST(Result, FromIter) {
  enum class Error {
    OneError,
    TwoError,
  };

  auto no_errors =
      sus::Array<Result<usize, Error>, 5>::with_values(
          Result<usize, Error>::with(1u), Result<usize, Error>::with(2u),
          Result<usize, Error>::with(3u), Result<usize, Error>::with(4u),
          Result<usize, Error>::with(5u))
          .into_iter();

  auto no_errors_out =
      sus::move(no_errors).collect<Result<CollectSum<usize>, Error>>();
  EXPECT_EQ(no_errors_out, sus::Ok);
  EXPECT_EQ(sus::move(no_errors_out).unwrap().sum, 1u + 2u + 3u + 4u + 5u);

  auto with_error =
      sus::Array<Result<usize, Error>, 5>::with_values(
          Result<usize, Error>::with(1u), Result<usize, Error>::with(2u),
          Result<usize, Error>::with_err(Error::OneError),
          Result<usize, Error>::with(4u), Result<usize, Error>::with(5u))
          .into_iter();

  auto with_error_out =
      sus::move(with_error).collect<Result<CollectSum<usize>, Error>>();
  EXPECT_EQ(with_error_out, sus::Err);
  EXPECT_EQ(sus::move(with_error_out).unwrap_err(), Error::OneError);

  auto with_errors =
      sus::Array<Result<usize, Error>, 5>::with_values(
          Result<usize, Error>::with(1u), Result<usize, Error>::with(2u),
          Result<usize, Error>::with_err(Error::OneError),
          Result<usize, Error>::with(4u),
          Result<usize, Error>::with_err(Error::TwoError))
          .into_iter();

  auto with_errors_out =
      sus::move(with_errors).collect<Result<CollectSum<usize>, Error>>();
  EXPECT_EQ(with_errors_out, sus::Err);
  EXPECT_EQ(sus::move(with_errors_out).unwrap_err(), Error::OneError);
}

TEST(Result, Clone) {
  struct Copy {
    Copy() {}
    Copy(const Copy& o) : i(o.i + 1_i32) {}
    Copy& operator=(const Copy&) = default;
    i32 i = 1_i32;
  };

  static_assert(::sus::mem::Copy<Copy>);
  static_assert(::sus::mem::Clone<Copy>);
  static_assert(::sus::mem::CloneInto<Copy>);
  static_assert(::sus::mem::Move<Copy>);
  static_assert(::sus::mem::Copy<Result<Copy, i32>>);
  static_assert(::sus::mem::Clone<Result<Copy, i32>>);
  static_assert(::sus::mem::CloneInto<Result<Copy, i32>>);
  static_assert(::sus::mem::Move<Result<Copy, i32>>);

  {
    const auto s = Result<Copy, i32>::with(Copy());
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Result<Copy, i32>>);
    EXPECT_EQ(s2, sus::Ok);
  }

  {
    const auto s = Result<Copy, i32>::with_err(2_i32);
    auto s2 = Result<Copy, i32>::with(Copy());
    sus::clone_into(mref(s2), s);
    EXPECT_EQ(s2, sus::Err);
  }

  struct Clone {
    Clone() {}
    Clone clone() const {
      auto c = Clone();
      c.i = i + 1_i32;
      return c;
    }

    Clone(Clone&&) = default;
    Clone& operator=(Clone&&) = default;

    i32 i = 1_i32;
  };

  static_assert(!::sus::mem::Copy<Clone>);
  static_assert(::sus::mem::Clone<Clone>);
  static_assert(!::sus::mem::CloneInto<Clone>);
  static_assert(::sus::mem::Move<Clone>);
  static_assert(!::sus::mem::Copy<Result<Clone, i32>>);
  static_assert(::sus::mem::Clone<Result<Clone, i32>>);
  static_assert(::sus::mem::CloneInto<Result<Clone, i32>>);
  static_assert(::sus::mem::Move<Result<Clone, i32>>);

  {
    const auto s = Result<Clone, i32>::with(Clone());
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Result<Clone, i32>>);
    EXPECT_EQ(s2, sus::Ok);
    EXPECT_EQ(sus::move(s2).unwrap().i, 2_i32);
  }

  {
    const auto s = Result<Clone, i32>::with_err(2_i32);
    auto s2 = Result<Clone, i32>::with(Clone());
    sus::clone_into(mref(s2), s);
    EXPECT_EQ(s2, sus::Err);
  }

  {
    const auto v = Result<void, i32>::with();
    auto v2 = sus::clone(v);
    EXPECT_TRUE(v2.is_ok());
  }

  {
    const auto v = Result<void, i32>::with_err(2);
    auto v2 = Result<void, i32>::with();
    sus::clone_into(v2, v);
    EXPECT_EQ(sus::move(v2).unwrap_err(), 2);
  }
}

TEST(Result, Eq) {
  struct NotEq {};
  static_assert(!sus::ops::Eq<NotEq>);

  static_assert(::sus::ops::Eq<Result<i32, i32>, Result<i32, i32>>);
  static_assert(::sus::ops::Eq<Result<i32, i32>, Result<i64, i8>>);
  static_assert(!::sus::ops::Eq<Result<i32, NotEq>, Result<i32, NotEq>>);
  static_assert(!::sus::ops::Eq<Result<NotEq, i32>, Result<NotEq, i32>>);
  static_assert(!::sus::ops::Eq<Result<NotEq, NotEq>, Result<NotEq, NotEq>>);

  EXPECT_EQ((Result<i32, i32>::with(1)), (Result<i32, i32>::with(1)));
  EXPECT_NE((Result<i32, i32>::with(1)), (Result<i32, i32>::with(2)));
  EXPECT_NE((Result<i32, i32>::with(1)), (Result<i32, i32>::with_err(1)));
  EXPECT_NE((Result<i32, i32>::with_err(1)), (Result<i32, i32>::with(1)));
  EXPECT_EQ((Result<i32, i32>::with_err(1)), (Result<i32, i32>::with_err(1)));

  EXPECT_EQ((Result<f32, i32>::with(1.f)), (Result<f32, i32>::with(1.f)));
  EXPECT_EQ((Result<f32, i32>::with(0.f)), (Result<f32, i32>::with(-0.f)));

  EXPECT_NE((Result<f32, i32>::with(f32::NAN)),
            (Result<f32, i32>::with(f32::NAN)));
  EXPECT_EQ((Result<i32, f32>::with_err(1.f)),
            (Result<i32, f32>::with_err(1.f)));
  EXPECT_EQ((Result<i32, f32>::with_err(0.f)),
            (Result<i32, f32>::with_err(-0.f)));
  EXPECT_NE((Result<i32, f32>::with_err(f32::NAN)),
            (Result<i32, f32>::with_err(f32::NAN)));

  // Comparison with marker types. EXPECT_EQ also converts it to a const
  // reference, so this tests that comparison from a const marker works (if the
  // inner type is copyable).
  EXPECT_EQ((Result<i32, i32>::with(1)), sus::ok(1));
  EXPECT_EQ((Result<i32, i32>::with_err(1)), sus::err(1));
}

TEST(Result, Ord) {
  static_assert(::sus::ops::Ord<Result<i32, i32>>);

  static_assert(Result<i32, i32>::with(1) < Result<i32, i32>::with(2));
  static_assert(Result<i32, i32>::with(3) > Result<i32, i32>::with(2));
  static_assert(Result<i32, i32>::with_err(1) < Result<i32, i32>::with_err(2));
  static_assert(Result<i32, i32>::with_err(3) > Result<i32, i32>::with_err(2));

  static_assert(Result<i32, i32>::with(1) > Result<i32, i32>::with_err(2));
  static_assert(Result<i32, i32>::with_err(1) < Result<i32, i32>::with(2));
}

TEST(Result, StrongOrder) {
  static_assert(std::strong_order(Result<i32, i32>::with(12),
                                  Result<i32, i32>::with(12)) ==
                std::strong_ordering::equal);
  static_assert(std::strong_order(Result<i32, i32>::with(12),
                                  Result<i32, i32>::with(13)) ==
                std::strong_ordering::less);
  static_assert(std::strong_order(Result<i32, i32>::with(12),
                                  Result<i32, i32>::with(11)) ==
                std::strong_ordering::greater);

  static_assert(std::strong_order(Result<i32, i32>::with_err(12),
                                  Result<i32, i32>::with_err(12)) ==
                std::strong_ordering::equal);
  static_assert(std::strong_order(Result<i32, i32>::with_err(12),
                                  Result<i32, i32>::with_err(13)) ==
                std::strong_ordering::less);
  static_assert(std::strong_order(Result<i32, i32>::with_err(12),
                                  Result<i32, i32>::with_err(11)) ==
                std::strong_ordering::greater);

  static_assert(std::strong_order(Result<i32, i32>::with(12),
                                  Result<i32, i32>::with_err(12)) ==
                std::strong_ordering::greater);
  static_assert(std::strong_order(Result<i32, i32>::with_err(12),
                                  Result<i32, i32>::with(12)) ==
                std::strong_ordering::less);
}

struct Weak {
  constexpr auto operator==(const Weak& o) const& noexcept {
    return a == o.a && b == o.b;
  }
  constexpr auto operator<=>(const Weak& o) const& noexcept {
    if (a == o.a) return std::weak_ordering::equivalent;
    if (a < o.a) return std::weak_ordering::less;
    return std::weak_ordering::greater;
  }

  constexpr Weak(int a, int b) : a(a), b(b) {}
  int a;
  int b;
};

TEST(Result, WeakOrder) {
  static_assert(!::sus::ops::Ord<Result<Weak, i32>>);
  static_assert(!::sus::ops::Ord<Result<i32, Weak>>);
  static_assert(!::sus::ops::Ord<Result<Weak, Weak>>);

  static_assert(::sus::ops::WeakOrd<Result<Weak, i32>>);
  static_assert(::sus::ops::WeakOrd<Result<i32, Weak>>);
  static_assert(::sus::ops::WeakOrd<Result<Weak, Weak>>);

  static_assert(std::weak_order(Result<Weak, i32>::with(Weak(1, 2)),
                                Result<Weak, i32>::with(Weak(1, 2))) ==
                std::weak_ordering::equivalent);
  static_assert(std::weak_order(Result<Weak, i32>::with(Weak(1, 2)),
                                Result<Weak, i32>::with(Weak(1, 3))) ==
                std::weak_ordering::equivalent);
  static_assert(std::weak_order(Result<Weak, i32>::with(Weak(1, 2)),
                                Result<Weak, i32>::with(Weak(2, 3))) ==
                std::weak_ordering::less);
  static_assert(std::weak_order(Result<Weak, i32>::with(Weak(2, 2)),
                                Result<Weak, i32>::with(Weak(1, 3))) ==
                std::weak_ordering::greater);
}

TEST(Result, PartialOrder) {
  static_assert(!::sus::ops::Ord<Result<f32, i8>>);
  static_assert(!::sus::ops::Ord<Result<i8, f32>>);
  static_assert(!::sus::ops::Ord<Result<f32, f32>>);

  static_assert(!::sus::ops::WeakOrd<Result<f32, i8>>);
  static_assert(!::sus::ops::WeakOrd<Result<i8, f32>>);
  static_assert(!::sus::ops::WeakOrd<Result<f32, f32>>);

  static_assert(::sus::ops::PartialOrd<Result<f32, i8>>);
  static_assert(::sus::ops::PartialOrd<Result<i8, f32>>);
  static_assert(::sus::ops::PartialOrd<Result<f32, f32>>);

  static_assert(std::partial_order(Result<f32, i8>::with(0.0f),
                                   Result<f32, i8>::with(-0.0f)) ==
                std::partial_ordering::equivalent);
  static_assert(std::partial_order(Result<f32, i8>::with(1.0f),
                                   Result<f32, i8>::with(-0.0f)) ==
                std::partial_ordering::greater);
  static_assert(std::partial_order(Result<f32, i8>::with(0.0f),
                                   Result<f32, i8>::with(1.0f)) ==
                std::partial_ordering::less);
  EXPECT_EQ(std::partial_order(Result<f32, i8>::with(f32::NAN),
                               Result<f32, i8>::with(f32::NAN)),
            std::partial_ordering::unordered);
}

TEST(Result, NoOrder) {
  struct NotCmp {};
  static_assert(!::sus::ops::PartialOrd<NotCmp>);
  static_assert(!::sus::ops::PartialOrd<Result<NotCmp, i8>>);
}

TEST(Result, UnwrapOrElse_BasicUsageExample) {
  enum class ECode { ItsHappening = -1 };
  auto conv = [](ECode e) { return static_cast<i32>(e); };
  auto ok = sus::Result<i32, ECode>::with(2);
  sus::check(sus::move(ok).unwrap_or_else(conv) == 2);
  auto err = sus::Result<i32, ECode>::with_err(ECode::ItsHappening);
  sus::check(sus::move(err).unwrap_or_else(conv) == -1);
}

TEST(Result, fmt) {
  static_assert(fmt::is_formattable<::sus::Result<i32, i32>, char>::value);
  EXPECT_EQ(fmt::format("{}", sus::Result<i32, i32>::with(12345)), "Ok(12345)");
  EXPECT_EQ(fmt::format("{:06}", sus::Result<i32, i32>::with(12345)),
            "Ok(012345)");  // The format string is for the Ok value.
  EXPECT_EQ(fmt::format("{}", sus::Result<i32, i32>::with_err(4321)),
            "Err(4321)");
  EXPECT_EQ(fmt::format("{:06}", sus::Result<i32, i32>::with_err(4321)),
            "Err(4321)");  // The format string is for the Ok value.
  EXPECT_EQ(
      fmt::format("{}", sus::Result<std::string_view, i32>::with("12345")),
      "Ok(12345)");
  EXPECT_EQ(
      fmt::format("{}", sus::Result<i32, std::string_view>::with_err("4321")),
      "Err(4321)");

  struct NoFormat {
    i32 a = 0x16ae3cf2;
  };
  static_assert(!fmt::is_formattable<NoFormat, char>::value);
  static_assert(fmt::is_formattable<Result<NoFormat, i32>, char>::value);
  static_assert(fmt::is_formattable<Result<i32, NoFormat>, char>::value);
  static_assert(fmt::is_formattable<Result<NoFormat, NoFormat>, char>::value);

  EXPECT_EQ(fmt::format("{}", sus::Result<i32, NoFormat>::with(12345)),
            "Ok(12345)");
  EXPECT_EQ(fmt::format("{}", sus::Result<i32, NoFormat>::with_err(NoFormat())),
            "Err(f2-3c-ae-16)");
  EXPECT_EQ(fmt::format("{}", sus::Result<NoFormat, i32>::with(NoFormat())),
            "Ok(f2-3c-ae-16)");
  EXPECT_EQ(fmt::format("{}", sus::Result<NoFormat, i32>::with_err(12345)),
            "Err(12345)");
}

TEST(Result, Stream) {
  std::stringstream s;
  s << sus::Result<i32, i32>::with(12345) << " "
    << sus::Result<i32, i32>::with_err(-76543);
  EXPECT_EQ(s.str(), "Ok(12345) Err(-76543)");
}

TEST(Result, GTest) {
  EXPECT_EQ(testing::PrintToString(sus::Result<i32, i32>::with(12345)),
            "Ok(12345)");
}

}  // namespace
