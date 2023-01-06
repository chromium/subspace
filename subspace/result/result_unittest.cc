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

#include "subspace/containers/array.h"
#include "subspace/iter/iterator.h"
#include "subspace/iter/once.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/move.h"
#include "subspace/num/types.h"
#include "subspace/prelude.h"
#include "googletest/include/gtest/gtest.h"

namespace {

using sus::result::Result;

struct TailPadding {
  i64 i;
  i32 j;
  // 4 bytes of tail padding, which the Result can use for its own state.
};
// MSVC doesn't take advantage of `[[no_unique_address]]`.
static_assert(sizeof(Result<i32, TailPadding>) ==
              sizeof(TailPadding) + sus_if_msvc_else(8, 0));

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

TEST(Option, OkHelpers) {
  auto a = Result<i32, u32>::with(2_i32);
  Result<i32, u32> a2 = sus::result::ok(2_i32);
  EXPECT_EQ(a, a2);

  auto i = 2_i32;
  auto c = Result<i32, u32>::with(i);
  Result<i32, u32> c2 = sus::result::ok(i);
  EXPECT_EQ(c, c2);

  const auto ci = 2_i32;
  const auto cc = Result<i32, u32>::with(ci);
  Result<i32, u32> cc2 = sus::result::ok(ci);
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
    auto marker = sus::result::ok(s);
    EXPECT_EQ(copies, 0);
    Result<S, u32> r = sus::move(marker);
    EXPECT_GE(copies, 1);
  }
}

TEST(Option, ErrHelpers) {
  auto a = Result<u32, i32>::with_err(2_i32);
  Result<u32, i32> a2 = sus::result::err(2_i32);
  EXPECT_EQ(a, a2);

  auto i = 2_i32;
  auto c = Result<u32, i32>::with_err(i);
  Result<u32, i32> c2 = sus::result::err(i);
  EXPECT_EQ(c, c2);

  const auto ci = 2_i32;
  const auto cc = Result<u32, i32>::with_err(ci);
  Result<u32, i32> cc2 = sus::result::err(ci);
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
    auto marker = sus::result::err(s);
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

template <class T>
struct CollectSum {
  sus_clang_bug_54050(CollectSum(T sum) : sum(sum){});

  static constexpr CollectSum from_iter(
      ::sus::iter::IteratorBase<T>&& iter) noexcept {
    T sum = T();
    for (const T& t : iter) sum += t;
    return CollectSum(sum);
  }

  T sum;
};

TEST(Result, FromIter) {
  enum class Error {
    OneError,
    TwoError,
  };

  auto no_errors =
      ::sus::Array<Result<usize, Error>, 5>::with_values(
          Result<usize, Error>::with(1u), Result<usize, Error>::with(2u),
          Result<usize, Error>::with(3u), Result<usize, Error>::with(4u),
          Result<usize, Error>::with(5u))
          .into_iter();

  auto no_errors_out =
      sus::move(no_errors).collect<Result<CollectSum<usize>, Error>>();
  EXPECT_EQ(no_errors_out, sus::result::Ok);
  EXPECT_EQ(sus::move(no_errors_out).unwrap().sum, 1u + 2u + 3u + 4u + 5u);

  auto with_error =
      ::sus::Array<Result<usize, Error>, 5>::with_values(
          Result<usize, Error>::with(1u), Result<usize, Error>::with(2u),
          Result<usize, Error>::with_err(Error::OneError),
          Result<usize, Error>::with(4u), Result<usize, Error>::with(5u))
          .into_iter();

  auto with_error_out =
      sus::move(with_error).collect<Result<CollectSum<usize>, Error>>();
  EXPECT_EQ(with_error_out, sus::result::Err);
  EXPECT_EQ(sus::move(with_error_out).unwrap_err(), Error::OneError);

  auto with_errors =
      ::sus::Array<Result<usize, Error>, 5>::with_values(
          Result<usize, Error>::with(1u), Result<usize, Error>::with(2u),
          Result<usize, Error>::with_err(Error::OneError),
          Result<usize, Error>::with(4u),
          Result<usize, Error>::with_err(Error::TwoError))
          .into_iter();

  auto with_errors_out =
      sus::move(with_errors).collect<Result<CollectSum<usize>, Error>>();
  EXPECT_EQ(with_errors_out, sus::result::Err);
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
  // Result is never Copy, but it is Clone if T is Clone.
  static_assert(!::sus::mem::Copy<Result<Copy, i32>>);
  static_assert(::sus::mem::Clone<Result<Copy, i32>>);
  static_assert(::sus::mem::CloneInto<Result<Copy, i32>>);
  static_assert(::sus::mem::Move<Result<Copy, i32>>);

  {
    const auto s = Result<Copy, i32>::with(Copy());
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Result<Copy, i32>>);
    EXPECT_EQ(s2, sus::result::Ok);
  }

  {
    const auto s = Result<Copy, i32>::with_err(2_i32);
    auto s2 = Result<Copy, i32>::with(Copy());
    sus::clone_into(mref(s2), s);
    EXPECT_EQ(s2, sus::result::Err);
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
    EXPECT_EQ(s2, sus::result::Ok);
    EXPECT_EQ(sus::move(s2).unwrap().i, 2_i32);
  }

  {
    const auto s = Result<Clone, i32>::with_err(2_i32);
    auto s2 = Result<Clone, i32>::with(Clone());
    sus::clone_into(mref(s2), s);
    EXPECT_EQ(s2, sus::result::Err);
  }
}

}  // namespace
