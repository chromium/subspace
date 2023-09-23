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

#include "sus/result/result.h"

#include <sstream>

#include "fmt/std.h"
#include "googletest/include/gtest/gtest.h"
#include "sus/collections/array.h"
#include "sus/iter/iterator.h"
#include "sus/iter/once.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/mem/move.h"
#include "sus/num/types.h"
#include "sus/prelude.h"
#include "sus/test/behaviour_types.h"
#include "sus/test/no_copy_move.h"

using sus::result::OkVoid;
using sus::result::Result;
using namespace sus::test;

namespace sus::test::result {
template <class T>
struct CollectSum {
  T sum;
};
}  // namespace sus::test::result

using namespace sus::test::result;

template <class T>
struct sus::iter::FromIteratorImpl<sus::test::result::CollectSum<T>> {
  static constexpr sus::test::result::CollectSum<T> from_iter(
      sus::iter::IntoIterator<T> auto iter) noexcept {
    T sum = T();
    for (T t : sus::move(iter).into_iter()) sum += t;
    return sus::test::result::CollectSum<T>(sum);
  }
};
static_assert(
    sus::iter::FromIterator<sus::test::result::CollectSum<usize>, usize>);

namespace {
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

static_assert(std::constructible_from<Result<i32, Error>, i32>);
static_assert(std::constructible_from<Result<i32, Error>, const i32>);
static_assert(std::constructible_from<Result<i32, Error>, i32&>);
static_assert(std::constructible_from<Result<i32, Error>, i32&&>);
static_assert(std::constructible_from<Result<i32, Error>, const i32&>);

static_assert(!std::constructible_from<Result<i32&, Error>, i32>);
static_assert(!std::constructible_from<Result<i32&, Error>, const i32>);
static_assert(std::constructible_from<Result<i32&, Error>, i32&>);
static_assert(!std::constructible_from<Result<i32&, Error>, const i32&>);

static_assert(std::constructible_from<Result<const i32&, Error>, i32>);
static_assert(std::constructible_from<Result<const i32&, Error>, const i32&>);
static_assert(std::constructible_from<Result<const i32&, Error>, i32&>);
static_assert(std::constructible_from<Result<const i32&, Error>, i32&&>);
static_assert(std::constructible_from<Result<const i32&, Error>, const i32&&>);

// No conversion to a temporary.
static_assert(std::constructible_from<Result<i32, Error>, i16>);
static_assert(std::constructible_from<Result<i32, Error>, const i16&>);
static_assert(std::constructible_from<Result<i32, Error>, i16&>);
static_assert(std::constructible_from<Result<i32, Error>, i16&&>);
static_assert(std::constructible_from<Result<i32, Error>, const i16&&>);
static_assert(!std::constructible_from<Result<const i32&, Error>, i16>);
static_assert(!std::constructible_from<Result<const i32&, Error>, const i16&>);
static_assert(!std::constructible_from<Result<const i32&, Error>, i16&>);
static_assert(!std::constructible_from<Result<const i32&, Error>, i16&&>);
static_assert(!std::constructible_from<Result<const i32&, Error>, const i16&&>);

TEST(Result, Construct) {
  {
    using T = DefaultConstructible;
    auto x = Result<T, i32>(T());
    auto y = Result<T, i32>::with_err(1);
    auto t = T();
    auto z = Result<T, i32>(t);
  }
  {
    using T = NotDefaultConstructible;
    auto x = Result<T, i32>(T(1));
    auto y = Result<T, i32>::with_err(1);
    auto t = T(1);
    auto z = Result<T, i32>(t);
  }
  {
    using T = TriviallyCopyable;
    auto x = Result<T, i32>(T(1));
    auto y = Result<T, i32>::with_err(1);
    auto t = T(1);
    auto z = Result<T, i32>(t);
  }
  {
    using T = TriviallyMoveableAndRelocatable;
    auto x = Result<T, i32>(T(1));
    auto y = Result<T, i32>::with_err(1);
    // Not copyable.
    // auto t = T(1);
    // auto z = Result<T, i32>(t);
  }
  {
    using T = TriviallyCopyableNotDestructible;
    auto x = Result<T, i32>(T(1));
    auto y = Result<T, i32>::with_err(1);
    auto t = T(1);
    auto z = Result<T, i32>(t);
  }
  {
    using T = TriviallyMoveableNotDestructible;
    auto x = Result<T, i32>(T(1));
    auto y = Result<T, i32>::with_err(1);
    // Not copyable.
    // auto t = T(1);
    // auto z = Result<T, i32>(t);
  }
  {
    using T = NotTriviallyRelocatableCopyableOrMoveable;
    auto x = Result<T, i32>(T(1));
    auto y = Result<T, i32>::with_err(1);
    // Not copyable.
    // auto t = T(1);
    // auto z = Result<T, i32>(t);
  }
  {
    using T = TrivialAbiRelocatable;
    auto x = Result<T, i32>(T(1));
    auto y = Result<T, i32>::with_err(1);
    // Not copyable.
    // auto t = T(1);
    // auto z = Result<T, i32>(t);
  }
  {
    using T = const NoCopyMove&;
    auto i = NoCopyMove();
    auto x = Result<T, i32>(static_cast<T>(i));
    auto y = Result<T, i32>::with_err(1);
    T t = i;
    auto z = Result<T, i32>(t);
  }
  {
    using T = NoCopyMove&;
    auto i = NoCopyMove();
    auto x = Result<T, i32>(static_cast<T>(i));
    auto y = Result<T, i32>::with_err(1);
    T t = i;
    auto z = Result<T, i32>(t);
  }
}

TEST(Result, Destructor) {
  static auto t_destructed = 0_usize;
  static auto e_destructed = 0_usize;
  struct T {
    ~T() { t_destructed += 1u; }
  };
  struct E {
    ~E() { e_destructed += 1u; }
  };

  // Verify non-trivial destructors are run, whether the other T/E type is
  // trivial or void or reference or non-trivial.
  {
    auto r = Result<T, E>(T());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(t_destructed, 1_usize);
  EXPECT_EQ(e_destructed, 0_usize);
  {
    auto r = Result<T, E>::with_err(E());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(t_destructed, 0_usize);
  EXPECT_EQ(e_destructed, 1_usize);
  {
    auto r = Result<T, int>(T());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(t_destructed, 1_usize);
  {
    auto r = Result<T, int>::with_err(2);
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(t_destructed, 0_usize);
  {
    auto r = Result<int, E>(2);
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 0_usize);
  {
    auto r = Result<int, E>::with_err(E());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 1_usize);

  auto m = NoCopyMove();

  {
    auto r = Result<NoCopyMove&, E>(m);
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 0_usize);
  {
    auto r = Result<NoCopyMove&, E>::with_err(E());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 1_usize);
  { auto r = Result<NoCopyMove&, int>(m); }
  { auto r = Result<NoCopyMove&, int>::with_err(2); }

  {
    auto r = Result<void, E>(OkVoid());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 0_usize);
  {
    auto r = Result<void, E>::with_err(E());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 1_usize);
  { auto r = Result<void, int>(OkVoid()); }
  { auto r = Result<void, int>::with_err(2); }
}

TEST(Result, With) {
  constexpr auto i = 4_i32;
  {
    constexpr auto a = Result<i32, Error>(3_i32);
    constexpr auto b = Result<i32, Error>(i);
  }
  {
    auto j = 4_i32;
    auto a = Result<i32, Error>(3_i32);
    auto b = Result<i32, Error>(i);
    auto c = Result<i32, Error>(j);
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
    auto c = Result<i32, Error>::with_err(f);
  }
}

TEST(Result, OkHelpers) {
  auto a = Result<i32, u32>(2_i32);
  Result<i32, u32> a2 = sus::ok(2_i32);
  EXPECT_EQ(a, a2);

  auto i = 2_i32;
  auto c = Result<i32, u32>(i);
  Result<i32, u32> c2 = sus::ok(i);
  EXPECT_EQ(c, c2);

  const auto ci = 2_i32;
  const auto cc = Result<i32, u32>(ci);
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

  // References.
  {
    const auto m = NoCopyMove();
    decltype(auto) u =
        sus::ok(m).construct<const NoCopyMove&, Error>().unwrap();
    static_assert(std::same_as<decltype(u), const NoCopyMove&>);
    EXPECT_EQ(&u, &m);
  }

  {
    auto m = NoCopyMove();
    decltype(auto) u =
        sus::ok(m).construct<const NoCopyMove&, Error>().unwrap();
    static_assert(std::same_as<decltype(u), const NoCopyMove&>);
    EXPECT_EQ(&u, &m);
  }

  {
    auto m = NoCopyMove();
    decltype(auto) u = sus::ok(m).construct<NoCopyMove&, Error>().unwrap();
    static_assert(std::same_as<decltype(u), NoCopyMove&>);
    EXPECT_EQ(&u, &m);
  }

  // Void Ok types.
  Result<void, i32> r = sus::ok();
  EXPECT_TRUE(r.is_ok());
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

  // References.
  {
    decltype(auto) u =
        sus::err(2_i32).construct<const NoCopyMove&, i32>().unwrap_err();
    static_assert(std::same_as<decltype(u), i32>);
    EXPECT_EQ(u, 2);
  }

  {
    decltype(auto) u =
        sus::err(2_i32).construct<NoCopyMove&, i32>().unwrap_err();
    static_assert(std::same_as<decltype(u), i32>);
    EXPECT_EQ(u, 2);
  }
}

TEST(Result, IsOk) {
  constexpr bool a = Result<i32, Error>(3_i32).is_ok();
  EXPECT_TRUE(a);

  constexpr bool b = Result<i32, Error>::with_err(Error()).is_ok();
  EXPECT_FALSE(b);

  auto m = NoCopyMove();
  constexpr bool c = Result<NoCopyMove&, Error>(m).is_ok();
  EXPECT_TRUE(c);
}

TEST(Result, IsErr) {
  constexpr bool a = Result<i32, Error>(3_i32).is_err();
  EXPECT_FALSE(a);

  constexpr bool b = Result<i32, Error>::with_err(Error()).is_err();
  EXPECT_TRUE(b);

  constexpr bool c = Result<NoCopyMove&, Error>::with_err(Error()).is_err();
  EXPECT_TRUE(c);
}

TEST(Result, Switch) {
  switch (Result<i32, Error>(3_i32)) {
    case sus::Ok: break;
    case sus::Err: ADD_FAILURE(); break;
  }

  switch (Result<i32, Error>::with_err(Error())) {
    case sus::Ok: ADD_FAILURE(); break;
    case sus::Err: break;
  }

  auto m = NoCopyMove();
  switch (Result<NoCopyMove&, Error>(m)) {
    case sus::Ok: break;
    case sus::Err: ADD_FAILURE(); break;
  }

  switch (Result<NoCopyMove&, Error>::with_err(Error())) {
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
    auto r = Result<T, E>(T());
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

  auto m = NoCopyMove();
  {
    auto r = Result<NoCopyMove&, E>(m);
    e_destructed = 0u;
    auto o = sus::move(r).ok();
    static_assert(std::same_as<decltype(o), sus::Option<NoCopyMove&>>);
    EXPECT_EQ(&o.as_value(), &m);
    EXPECT_EQ(e_destructed, 0u);
  }
  {
    auto r = Result<const NoCopyMove&, E>(m);
    e_destructed = 0u;
    auto o = sus::move(r).ok();
    static_assert(std::same_as<decltype(o), sus::Option<const NoCopyMove&>>);
    EXPECT_EQ(&o.as_value(), &m);
    EXPECT_EQ(e_destructed, 0u);
  }
  {
    auto r = Result<NoCopyMove&, E>::with_err(E());
    e_destructed = 0u;
    auto o = sus::move(r).ok();
    static_assert(std::same_as<decltype(o), sus::Option<NoCopyMove&>>);
    // We destroyed the E in Result<T, E>.
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
    auto r = Result<T, E>(T());
    t_destructed = e_destructed = 0u;
    auto o = sus::move(r).err();
    EXPECT_EQ(o, sus::None);
    // We destroyed the T in Result<T, E>.
    EXPECT_EQ(e_destructed, 0u);
    EXPECT_EQ(t_destructed, 1u);
  }
}

TEST(Result, Unwrap) {
  constexpr auto a = Result<i32, Error>(3_i32).unwrap();
  static_assert(std::same_as<decltype(a), const i32>);
  EXPECT_EQ(a, 3_i32);

  Result<void, Error>(OkVoid()).unwrap();  // Returns void, doesn't panic.
  static_assert(
      std::same_as<decltype(Result<void, Error>(OkVoid()).unwrap()), void>);

  auto m = NoCopyMove();
  decltype(auto) u = Result<NoCopyMove&, Error>(m).unwrap();
  static_assert(std::same_as<decltype(u), NoCopyMove&>);
  EXPECT_EQ(&u, &m);

  decltype(auto) cu = Result<const NoCopyMove&, Error>(m).unwrap();
  static_assert(std::same_as<decltype(cu), const NoCopyMove&>);
  EXPECT_EQ(&cu, &m);
}

TEST(Result, Expect) {
  constexpr auto a = Result<i32, Error>(3_i32).expect("hello");
  static_assert(std::same_as<decltype(a), const i32>);
  EXPECT_EQ(a, 3_i32);

  Result<void, Error>(OkVoid()).expect(
      "hello");  // Returns void, doesn't panic.
  static_assert(
      std::same_as<decltype(Result<void, Error>(OkVoid()).expect("hello")),
                   void>);

  auto m = NoCopyMove();
  decltype(auto) u = Result<NoCopyMove&, Error>(m).expect("hello");
  static_assert(std::same_as<decltype(u), NoCopyMove&>);
  EXPECT_EQ(&u, &m);

  decltype(auto) cu = Result<const NoCopyMove&, Error>(m).expect("hello");
  static_assert(std::same_as<decltype(cu), const NoCopyMove&>);
  EXPECT_EQ(&cu, &m);
}

TEST(ResultDeathTest, Unwrap) {
#if GTEST_HAS_DEATH_TEST
  {
    auto r = Result<i32, Error>::with_err(Error());
    EXPECT_DEATH(r.as_value(), "PANIC! at 'Result has error state'");
    EXPECT_DEATH(r.as_value_mut(), "PANIC! at 'Result has error state'");
    EXPECT_DEATH(sus::move(r).unwrap(), "PANIC! at 'Result has error state'");
  }
  {
    auto r = Result<i32, u32>::with_err(3u);
    EXPECT_DEATH(r.as_value(), "PANIC! at '3'");
    EXPECT_DEATH(r.as_value_mut(), "PANIC! at '3'");
    EXPECT_DEATH(sus::move(r).unwrap(), "PANIC! at '3'");
  }
#endif
}

TEST(ResultDeathTest, UnwrapErr) {
#if GTEST_HAS_DEATH_TEST
  struct Unprintable {};
  {
    auto r = Result<Unprintable, Error>(Unprintable());
    EXPECT_DEATH(r.as_err(), "PANIC! at 'Result has ok state'");
    // TODO: EXPECT_DEATH(r.as_err_mut(), "PANIC! at 'Result has ok state'");
    EXPECT_DEATH(sus::move(r).unwrap_err(), "PANIC! at 'Result has ok state'");
  }
  {
    auto r = Result<i32, Error>(2);
    EXPECT_DEATH(r.as_err(), "PANIC! at '2'");
    // TODO: EXPECT_DEATH(r.as_err_mut(), "PANIC! at '2'");
    EXPECT_DEATH(sus::move(r).unwrap_err(), "PANIC! at '2'");
  }
#endif
}  // namespace

TEST(ResultDeathTest, Expect) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((Result<i32, Error>::with_err(Error()).expect("hello")),
               "PANIC! at 'hello'");
  EXPECT_DEATH((Result<i32, u32>::with_err(3u).expect("hello")),
               "PANIC! at 'hello: 3'");
#endif
}

TEST(Result, UnwrapOrDefault) {
  {
    constexpr auto a = Result<i32, Error>(3_i32).unwrap_or_default();
    static_assert(std::same_as<decltype(a), const i32>);
    EXPECT_EQ(a, 3_i32);

    constexpr auto d =
        Result<i32, Error>::with_err(Error()).unwrap_or_default();
    static_assert(std::same_as<decltype(d), const i32>);
    EXPECT_EQ(d, 0_i32);
  }
  {
    // Returns void, doesn't panic.
    Result<void, Error>(OkVoid()).unwrap_or_default();
    static_assert(
        std::same_as<
            decltype(Result<void, Error>(OkVoid()).unwrap_or_default()), void>);
    // Returns void, doesn't panic.
    Result<void, Error>::with_err(Error()).unwrap_or_default();
    static_assert(
        std::same_as<
            decltype(Result<void, Error>(OkVoid()).unwrap_or_default()), void>);
  }
}

TEST(Result, UnwrapUnchecked) {
  constexpr auto a = Result<i32, Error>(3).unwrap_unchecked(unsafe_fn);
  static_assert(std::same_as<decltype(a), const i32>);
  EXPECT_EQ(a, 3_i32);

  // Returns void, doesn't panic.
  Result<void, Error>(OkVoid()).unwrap_unchecked(unsafe_fn);
  static_assert(
      std::same_as<decltype(Result<void, Error>(OkVoid()).unwrap_unchecked(
                       unsafe_fn)),
                   void>);

  auto m = NoCopyMove();
  decltype(auto) u = Result<NoCopyMove&, Error>(m).unwrap_unchecked(unsafe_fn);
  static_assert(std::same_as<decltype(u), NoCopyMove&>);
  EXPECT_EQ(&u, &m);

  decltype(auto) cu =
      Result<const NoCopyMove&, Error>(m).unwrap_unchecked(unsafe_fn);
  static_assert(std::same_as<decltype(cu), const NoCopyMove&>);
  EXPECT_EQ(&cu, &m);
}

TEST(Result, DestroyAfterUnwrap) {
  static i32 destroyed = 0;
  struct S {
    S() = default;
    S(S&&) = default;
    S& operator=(S&&) = default;
    ~S() { destroyed += 1; }
  };

  // Verify an unwrapped Result doesn't destroy a value that was already
  // unwrapped/destroyed.

  i32 counted_destroyed;
  {
    auto r = Result<S, Error>(S());
    sus::move(r).unwrap();
    counted_destroyed = destroyed;
  }
  EXPECT_EQ(destroyed, counted_destroyed);

  {
    auto r = Result<S, Error>(S());
    sus::move(r).unwrap_or_default();
    counted_destroyed = destroyed;
  }
  EXPECT_EQ(destroyed, counted_destroyed);

  {
    auto r = Result<S, Error>(S());
    sus::move(r).unwrap_unchecked(unsafe_fn);
    counted_destroyed = destroyed;
  }
  EXPECT_EQ(destroyed, counted_destroyed);

  {
    auto r = Result<void, S>::with_err(S());
    sus::move(r).unwrap_err();
    counted_destroyed = destroyed;
  }
  EXPECT_EQ(destroyed, counted_destroyed);

  {
    auto r = Result<void, S>::with_err(S());
    sus::move(r).unwrap_err_unchecked(unsafe_fn);
    counted_destroyed = destroyed;
  }
  EXPECT_EQ(destroyed, counted_destroyed);
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
  auto r = Result<i32, Error>(3_i32);
  EXPECT_DEATH(sus::move(r).unwrap_err(), "");

  auto r2 = Result<void, Error>(OkVoid());
  EXPECT_DEATH(sus::move(r).unwrap_err(), "");
#endif
}

TEST(Result, UnwrapOrElse) {
  constexpr auto a =
      Result<i32, Error>(3_i32).unwrap_or_else([](Error) { return 4_i32; });
  static_assert(std::same_as<decltype(a), const i32>);
  EXPECT_EQ(a, 3_i32);

  constexpr auto b = Result<i32, Error>::with_err(Error()).unwrap_or_else(
      [](Error) constexpr { return 4_i32; });
  EXPECT_EQ(b, 4_i32);

  Result<void, Error>(OkVoid()).unwrap_or_else(
      [](Error) {});  // Returns void, doesn't panic.
  static_assert(
      std::same_as<decltype(Result<void, Error>(OkVoid()).unwrap_or_else(
                       [](Error) {})),
                   void>);

  Result<void, Error>::with_err(Error()).unwrap_or_else(
      [](Error) {});  // Returns void, doesn't panic.
}

TEST(Result, Copy) {
  // This type has a user defined copy constructor, which deletes the implicit
  // copy constructor in Option.
  auto static copied = 0_usize;
  struct Type {
    explicit Type() noexcept = default;
    Type(const Type&) noexcept { copied += 1u; }
    Type& operator=(const Type&) noexcept { return copied += 1u, *this; }

    constexpr bool operator==(const Type& rhs) const& noexcept {
      return this == &rhs;
    }
  };

  copied = 0_usize;
  {
    auto x = Result<Type, i32>(Type());
    EXPECT_EQ(copied, 1u);
    auto y = x;
    EXPECT_EQ(copied, 2u);
    EXPECT_EQ(x, sus::Ok);
    EXPECT_EQ(y, sus::Ok);
  }
  copied = 0_usize;
  {
    auto x = Result<Type, i32>::with_err(2);
    EXPECT_EQ(copied, 0u);
    auto y = x;
    EXPECT_EQ(copied, 0u);
    EXPECT_EQ(x, sus::Err);
    EXPECT_EQ(y, sus::Err);
  }
  copied = 0_usize;
  {
    auto x = Result<i32, Type>(2);
    EXPECT_EQ(copied, 0u);
    auto y = x;
    EXPECT_EQ(copied, 0u);
    EXPECT_EQ(x, sus::Ok);
    EXPECT_EQ(y, sus::Ok);
  }
  copied = 0_usize;
  {
    auto x = Result<i32, Type>::with_err(Type());
    EXPECT_EQ(copied, 1u);
    auto y = x;
    EXPECT_EQ(copied, 2u);
    EXPECT_EQ(x, sus::Err);
    EXPECT_EQ(y, sus::Err);
  }

  copied = 0_usize;
  {
    auto rv = Result<void, Type>(OkVoid());
    EXPECT_EQ(copied, 0u);
    auto rv2 = rv;
    EXPECT_EQ(copied, 0u);
    EXPECT_EQ(rv, rv2);
    EXPECT_EQ(rv, sus::Ok);
    EXPECT_EQ(rv2, sus::Ok);
  }
  {
    copied = 0_usize;
    auto rv = Result<void, Type>::with_err(Type());
    EXPECT_EQ(copied, 1u);
    auto rv2 = rv;
    EXPECT_EQ(copied, 2u);
    EXPECT_EQ(rv, sus::Err);
    EXPECT_EQ(rv2, sus::Err);
  }
  {
    copied = 0_usize;
    auto rv = Result<void, Type>(OkVoid());
    auto rv2 = Result<void, Type>::with_err(Type());
    EXPECT_EQ(copied, 1u);
    rv = rv2;
    EXPECT_EQ(copied, 2u);
    EXPECT_EQ(rv.is_err(), true);
    EXPECT_EQ(rv2.is_err(), true);
  }
  {
    copied = 0_usize;
    auto rv = Result<void, Type>(OkVoid());
    auto rv2 = Result<void, Type>::with_err(Type());
    EXPECT_EQ(copied, 1u);
    rv2 = rv;
    EXPECT_EQ(rv.is_ok(), true);
    EXPECT_EQ(rv2.is_ok(), true);
  }

  auto m = NoCopyMove();

  {
    auto z = Result<NoCopyMove&, int>(m);
    auto zz = z;
    EXPECT_EQ(&z.as_value(), &m);
    EXPECT_EQ(&zz.as_value(), &m);
  }
  {
    auto z = Result<NoCopyMove&, int>::with_err(2);
    auto zz = z;
    EXPECT_EQ(z.as_err(), 2);
    EXPECT_EQ(zz.as_err(), 2);
  }
  {
    auto z = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>(m);
    auto zz = z;
    EXPECT_EQ(&z.as_value(), &m);
    EXPECT_EQ(&zz.as_value(), &m);
  }
  {
    auto z = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    auto zz = z;
    EXPECT_EQ(z.as_err().i, 2);
    EXPECT_EQ(zz.as_err().i, 2);
  }
  {
    auto z = Result<NoCopyMove&, int>(m);
    auto zz = Result<NoCopyMove&, int>::with_err(2);
    z = zz;
    EXPECT_EQ(z.as_err(), 2);
    EXPECT_EQ(zz.as_err(), 2);
  }
  {
    auto z = Result<NoCopyMove&, int>(m);
    auto zz = Result<NoCopyMove&, int>::with_err(2);
    zz = z;
    EXPECT_EQ(&z.as_value(), &m);
    EXPECT_EQ(&zz.as_value(), &m);
  }
  {
    auto z = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>(m);
    auto zz = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    z = zz;
    EXPECT_EQ(z.as_err().i, 2);
    EXPECT_EQ(zz.as_err().i, 2);
  }
  {
    auto z = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>(m);
    auto zz = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    zz = z;
    EXPECT_EQ(&z.as_value(), &m);
    EXPECT_EQ(&zz.as_value(), &m);
  }

  // Constexpr copy construct.
  constexpr auto r1 = []() constexpr {
    auto r = sus::Result<NotTriviallyRelocatableCopyableOrMoveable, u32>(
        NotTriviallyRelocatableCopyableOrMoveable(5));
    auto s = r;
    return ::sus::move(s).unwrap();
  }();
  static_assert(r1.i == 5);
  // Constexpr copy assign.
  constexpr auto r2 = []() constexpr {
    auto r = sus::Result<NotTriviallyRelocatableCopyableOrMoveable, u32>(
        NotTriviallyRelocatableCopyableOrMoveable(5));
    auto s = sus::Result<NotTriviallyRelocatableCopyableOrMoveable, u32>(
        NotTriviallyRelocatableCopyableOrMoveable(6));
    r = s;
    return ::sus::move(r).unwrap();
  }();
  static_assert(r2.i == 6);
}

TEST(Result, Move) {
  // This type has a user defined move constructor, which deletes the implicit
  // move constructor in Option.
  struct Type {
    Type() = default;
    Type(Type&&) = default;
    Type& operator=(Type&&) = default;
  };
  auto x = Result<Type, i32>(Type());
  auto y = sus::move(x);
  EXPECT_EQ(y, sus::Ok);
  x = sus::move(y);
  EXPECT_EQ(x, sus::Ok);

  struct MoveableLvalue {
    MoveableLvalue(int i) : i(i) {}
    MoveableLvalue(const MoveableLvalue& m) : i(m.i) {}
    void operator=(const MoveableLvalue& m) { i = m.i; }
    MoveableLvalue(MoveableLvalue&& m) : i(m.i) { m.i = 0; }
    void operator=(MoveableLvalue&& m) {
      i = m.i;
      m.i = 0;
    }
    int i;
  };
  MoveableLvalue lvalue(2);
  auto a = Result<MoveableLvalue, i32>(lvalue);
  EXPECT_EQ(a.as_value().i, 2);
  EXPECT_EQ(lvalue.i, 2);

  auto b = Result<MoveableLvalue, i32>(sus::move(lvalue));
  EXPECT_EQ(b.as_value().i, 2);
  EXPECT_EQ(lvalue.i, 0);

  {
    auto z = Result<void, int>(OkVoid());
    auto zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
    z = sus::move(zz);
    EXPECT_EQ(z.is_ok(), true);
  }
  {
    auto z = Result<void, NotTriviallyRelocatableCopyableOrMoveable>(OkVoid());
    auto zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
    z = sus::move(zz);
    EXPECT_EQ(z.is_ok(), true);
  }
  {
    auto z = Result<void, int>(OkVoid());
    auto zz = Result<void, int>::with_err(2);
    z = sus::move(zz);
    EXPECT_EQ(z.as_err(), 2);
  }
  {
    auto z = Result<void, int>(OkVoid());
    auto zz = Result<void, int>::with_err(2);
    zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
  }
  {
    auto z = Result<void, NotTriviallyRelocatableCopyableOrMoveable>(OkVoid());
    auto zz = Result<void, NotTriviallyRelocatableCopyableOrMoveable>::with_err(
        NotTriviallyRelocatableCopyableOrMoveable(2));
    z = sus::move(zz);
    EXPECT_EQ(z.as_err().i, 2);
  }
  {
    auto z = Result<void, NotTriviallyRelocatableCopyableOrMoveable>(OkVoid());
    auto zz = Result<void, NotTriviallyRelocatableCopyableOrMoveable>::with_err(
        NotTriviallyRelocatableCopyableOrMoveable(2));
    zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
  }

  {
    auto m = NoCopyMove();
    auto z = Result<NoCopyMove&, int>(m);
    auto zz = sus::move(z);
    EXPECT_EQ(&zz.as_value(), &m);
    z = sus::move(zz);
    EXPECT_EQ(&z.as_value(), &m);
  }
  {
    auto m = NoCopyMove();
    auto z = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>(m);
    auto zz = sus::move(z);
    EXPECT_EQ(&zz.as_value(), &m);
    z = sus::move(zz);
    EXPECT_EQ(&z.as_value(), &m);
  }
  {
    auto m = NoCopyMove();
    auto z = Result<NoCopyMove&, int>(m);
    auto zz = Result<NoCopyMove&, int>::with_err(2);
    z = sus::move(zz);
    EXPECT_EQ(z.as_err(), 2);
  }
  {
    auto m = NoCopyMove();
    auto z = Result<NoCopyMove&, int>(m);
    auto zz = Result<NoCopyMove&, int>::with_err(2);
    zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
  }
  {
    auto m = NoCopyMove();
    auto z = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>(m);
    auto zz = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    z = sus::move(zz);
    EXPECT_EQ(z.as_err().i, 2);
  }
  {
    auto m = NoCopyMove();
    auto z = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>(m);
    auto zz = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
  }

  // Constexpr move construct.
  constexpr auto r1 = []() constexpr {
    auto r = sus::Result<NotTriviallyRelocatableCopyableOrMoveable, u32>(
        NotTriviallyRelocatableCopyableOrMoveable(5));
    auto s = sus::move(r);
    return ::sus::move(s).unwrap();
  }();
  static_assert(r1.i == 5);
  // Constexpr move assign.
  constexpr auto r2 = []() constexpr {
    auto r = sus::Result<NotTriviallyRelocatableCopyableOrMoveable, u32>(
        NotTriviallyRelocatableCopyableOrMoveable(5));
    auto s = sus::Result<NotTriviallyRelocatableCopyableOrMoveable, u32>(
        NotTriviallyRelocatableCopyableOrMoveable(6));
    r = sus::move(s);
    return ::sus::move(r).unwrap();
  }();
  static_assert(r2.i == 6);
}

TEST(Result, MoveAfterTrivialMove) {
  {
    auto r = Result<i32, i32>(1_i32);
    auto r3 = sus::move(r);
    auto r2 = sus::move(r3);
    EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);
  }
  {
    auto r = Result<i32, i32>(1_i32);
    auto r3 = sus::move(r);
    auto r2 = sus::move(r3);
    EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);
  }
  {
    auto r = Result<i32, i32>::with_err(2_i32);
    auto r3 = sus::move(r);
    auto r2 = sus::move(r3);
    EXPECT_EQ(sus::move(r2).unwrap_err(), 2_i32);
  }
  {
    auto rv = Result<void, i32>(OkVoid());
    auto rv3 = sus::move(rv);
    auto rv2 = sus::move(rv3);
    EXPECT_TRUE(rv2.is_ok());
  }

  auto m = NoCopyMove();
  {
    auto rv = Result<NoCopyMove&, i32>(m);
    auto rv3 = sus::move(rv);
    auto rv2 = sus::move(rv3);
    EXPECT_EQ(&rv2.as_value(), &m);
  }
}

TEST(Result, AssignAfterTrivialMove) {
  {
    auto r = Result<i32, i32>(1_i32);
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap(), 1_i32);
  }
  {
    auto r = Result<i32, i32>(1_i32);
    auto r3 = sus::move(r);
    r = Result<i32, i32>::with_err(1_i32);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap(), 1_i32);
  }

  {
    auto r = Result<i32, i32>::with_err(2_i32);
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err(), 2_i32);
  }
  {
    auto r = Result<i32, i32>::with_err(2_i32);
    auto r3 = sus::move(r);
    r = Result<i32, i32>(2_i32);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err(), 2_i32);
  }

  {
    auto rv = Result<void, i32>(OkVoid());
    auto rv3 = sus::move(rv);
    rv = sus::move(rv3);
    EXPECT_TRUE(rv.is_ok());
  }
  {
    auto rv = Result<void, i32>(OkVoid());
    auto rv3 = sus::move(rv);
    rv = Result<void, i32>::with_err(2);
    rv = sus::move(rv3);
    EXPECT_TRUE(rv.is_ok());
  }

  auto m = NoCopyMove();
  {
    auto rv = Result<NoCopyMove&, i32>(m);
    auto rv3 = sus::move(rv);
    rv = sus::move(rv3);
    EXPECT_EQ(&rv.as_value(), &m);
  }
  {
    auto rv = Result<NoCopyMove&, i32>(m);
    auto rv3 = sus::move(rv);
    rv = Result<NoCopyMove&, i32>::with_err(2);
    rv = sus::move(rv3);
    EXPECT_EQ(&rv.as_value(), &m);
  }
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
  {
    auto r = Result<NonTrivialMove, i32>(NonTrivialMove(1));
    sus::move(r).unwrap();
    EXPECT_DEATH([[maybe_unused]] auto r2 = sus::move(r), "");
  }
  {
    auto r = Result<i32, NonTrivialMove>(1);
    sus::move(r).unwrap();
    EXPECT_DEATH([[maybe_unused]] auto r2 = sus::move(r), "");
  }
  {
    auto r = Result<void, NonTrivialMove>(OkVoid());
    sus::move(r).unwrap();
    EXPECT_DEATH([[maybe_unused]] auto r2 = sus::move(r), "");
  }

  auto m = NoCopyMove();
  {
    auto rv = Result<NoCopyMove&, NonTrivialMove>(m);
    auto rv3 = sus::move(rv);
    rv = sus::move(rv3);
    EXPECT_EQ(&rv.as_value(), &m);
  }
}

TEST(Result, AssignAfterNonTrivialMove) {
  {
    auto r = Result<NonTrivialMove, i32>(NonTrivialMove(1));
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap().i, 1_i32);
  }
  {
    auto r = Result<NonTrivialMove, i32>(NonTrivialMove(1));
    auto r3 = sus::move(r);
    r = Result<NonTrivialMove, i32>::with_err(2);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap().i, 1_i32);
  }

  {
    auto r = Result<NonTrivialMove, i32>::with_err(2);
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err(), 2_i32);
  }
  {
    auto r = Result<NonTrivialMove, i32>::with_err(2);
    auto r3 = sus::move(r);
    r = Result<NonTrivialMove, i32>(NonTrivialMove(1));
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err(), 2_i32);
  }

  {
    auto r = Result<i32, NonTrivialMove>(1);
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap(), 1_i32);
  }
  {
    auto r = Result<i32, NonTrivialMove>(1);
    auto r3 = sus::move(r);
    r = Result<i32, NonTrivialMove>::with_err(NonTrivialMove(2));
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap(), 1_i32);
  }

  {
    auto r = Result<i32, NonTrivialMove>::with_err(NonTrivialMove(2));
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err().i, 2_i32);
  }
  {
    auto r = Result<i32, NonTrivialMove>::with_err(NonTrivialMove(2));
    auto r3 = sus::move(r);
    r = Result<i32, NonTrivialMove>(1);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err().i, 2_i32);
  }

  {
    auto r = Result<void, NonTrivialMove>(OkVoid());
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(r, sus::Ok);
  }
  {
    auto r = Result<void, NonTrivialMove>(OkVoid());
    auto r3 = sus::move(r);
    r = Result<void, NonTrivialMove>::with_err(NonTrivialMove(2));
    r = sus::move(r3);
    EXPECT_EQ(r, sus::Ok);
  }

  {
    auto r = Result<void, NonTrivialMove>::with_err(NonTrivialMove(2));
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err().i, 2_i32);
  }
  {
    auto r = Result<void, NonTrivialMove>::with_err(NonTrivialMove(2));
    auto r3 = sus::move(r);
    r = Result<void, NonTrivialMove>(OkVoid());
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err().i, 2_i32);
  }

  auto m = NoCopyMove();
  {
    auto r = Result<NoCopyMove&, NonTrivialMove>(m);
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(&r.as_value(), &m);
  }
  {
    auto r = Result<NoCopyMove&, NonTrivialMove>(m);
    auto r3 = sus::move(r);
    r = Result<NoCopyMove&, NonTrivialMove>::with_err(NonTrivialMove(1));
    r = sus::move(r3);
    EXPECT_EQ(&r.as_value(), &m);
  }

  {
    auto r = Result<NoCopyMove&, NonTrivialMove>::with_err(NonTrivialMove(2));
    auto r3 = sus::move(r);
    r = Result<NoCopyMove&, NonTrivialMove>(m);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err().i, 2_i32);
  }

  auto m2 = NoCopyMove();
  {
    auto r = Result<NoCopyMove&, NonTrivialMove>(m);
    auto r2 = sus::move(r);
    r = Result<NoCopyMove&, NonTrivialMove>(m2);
    EXPECT_EQ(&r.as_value(), &m2);
    EXPECT_EQ(&r2.as_value(), &m);
  }
}

TEST(Result, MoveSelfAssign) {
  auto r = Result<TriviallyCopyable, i32>(TriviallyCopyable(1));
  r = sus::move(r);
  EXPECT_EQ(sus::move(r).unwrap().i, 1);

  auto rv = Result<void, i32>(OkVoid());
  rv = sus::move(rv);
  EXPECT_TRUE(rv.is_ok());

  auto s = Result<NotTriviallyRelocatableCopyableOrMoveable, i32>(
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

  auto m = NoCopyMove();
  auto rm = Result<NoCopyMove&, i32>(m);
  rm = sus::move(rm);
  EXPECT_EQ(&rm.as_value(), &m);
}

TEST(Result, CopySelfAssign) {
  auto r = Result<TriviallyCopyable, i32>(TriviallyCopyable(1));
  r = r;
  EXPECT_EQ(sus::move(r).unwrap().i, 1);

  auto rv = Result<void, i32>(OkVoid());
  rv = rv;
  EXPECT_TRUE(rv.is_ok());

  auto s = Result<NotTriviallyRelocatableCopyableOrMoveable, i32>(
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

  auto m = NoCopyMove();
  auto rm = Result<NoCopyMove&, i32>(m);
  rm = rm;
  EXPECT_EQ(&rm.as_value(), &m);
}

TEST(Result, CloneIntoSelfAssign) {
  auto r = Result<TriviallyCopyable, i32>(TriviallyCopyable(1));
  sus::clone_into(r, r);
  EXPECT_EQ(sus::move(r).unwrap().i, 1);

  auto v = Result<void, i32>(OkVoid());
  sus::clone_into(v, v);
  EXPECT_TRUE(v.is_ok());

  auto s = Result<NotTriviallyRelocatableCopyableOrMoveable, i32>(
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

  auto m = NoCopyMove();
  auto rm = Result<NoCopyMove&, i32>(m);
  sus::clone_into(rm, rm);
  EXPECT_EQ(&rm.as_value(), &m);
}

TEST(Result, Iter) {
  auto x = Result<i32, u8>::with_err(2_u8);
  for ([[maybe_unused]] auto i : x.iter()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Result<i32, u8>(-4_i32);
  for (auto& i : y.iter()) {
    static_assert(std::same_as<decltype(i), const i32&>);
    EXPECT_EQ(i, -4_i32);
    ++count;
  }
  EXPECT_EQ(count, 1);

  auto m = NoCopyMove();
  {
    auto err = Result<NoCopyMove&, u8>::with_err(2_u8);
    for ([[maybe_unused]] auto& i : err.iter()) {
      ADD_FAILURE();
    }
  }
  {
    auto ok = Result<NoCopyMove&, u8>(m);
    auto it = ok.iter();
    static_assert(
        std::same_as<decltype(it.next()), sus::Option<const NoCopyMove&>>);
    EXPECT_EQ(&it.next().unwrap(), &m);
    EXPECT_EQ(it.next(), sus::None);
  }

  // A reference type can be iterated as an rvalue.
  {
    for ([[maybe_unused]] auto& i :
         Result<NoCopyMove&, u8>::with_err(2_u8).iter()) {
      ADD_FAILURE();
    }
  }
  {
    auto it = Result<NoCopyMove&, u8>(m).iter();
    EXPECT_EQ(&it.next().unwrap(), &m);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(Result, IterMut) {
  auto x = Result<i32, u8>::with_err(2_u8);
  for ([[maybe_unused]] auto i : x.iter_mut()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Result<i32, u8>(-3_i32);
  for (auto& i : y.iter_mut()) {
    static_assert(std::same_as<decltype(i), i32&>, "");
    EXPECT_EQ(i, -3_i32);
    i += 1;
    ++count;
  }
  EXPECT_EQ(sus::move(y).unwrap(), -2_i32);

  auto m = NoCopyMove();
  {
    auto err = Result<NoCopyMove&, u8>::with_err(2_u8);
    for ([[maybe_unused]] auto& i : err.iter_mut()) {
      ADD_FAILURE();
    }
  }
  {
    auto ok = Result<NoCopyMove&, u8>(m);
    auto it = ok.iter_mut();
    static_assert(std::same_as<decltype(it.next()), sus::Option<NoCopyMove&>>);
    EXPECT_EQ(&it.next().unwrap(), &m);
    EXPECT_EQ(it.next(), sus::None);
  }

  // A reference type can be iterated as an rvalue.
  {
    for ([[maybe_unused]] auto& i :
         Result<NoCopyMove&, u8>::with_err(2_u8).iter_mut()) {
      ADD_FAILURE();
    }
  }
  {
    auto it = Result<NoCopyMove&, u8>(m).iter_mut();
    EXPECT_EQ(&it.next().unwrap(), &m);
    EXPECT_EQ(it.next(), sus::None);
  }
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
  auto y = Result<MoveOnly, u8>(MoveOnly(-3));
  for (auto m : sus::move(y).into_iter()) {
    static_assert(std::same_as<decltype(m), MoveOnly>, "");
    EXPECT_EQ(m.i, -3);
    ++count;
  }
  EXPECT_EQ(count, 1);

  auto m = NoCopyMove();
  {
    auto err = Result<NoCopyMove&, u8>::with_err(2_u8);
    for ([[maybe_unused]] auto& i : sus::move(err).into_iter()) {
      ADD_FAILURE();
    }
  }
  {
    auto ok = Result<NoCopyMove&, u8>(m);
    auto it = sus::move(ok).into_iter();
    static_assert(std::same_as<decltype(it.next()), sus::Option<NoCopyMove&>>);
    EXPECT_EQ(&it.next().unwrap(), &m);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(Result, ImplicitIter) {
  auto x = Result<i32, u8>::with_err(2_u8);
  for ([[maybe_unused]] auto i : x) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Result<MoveOnly, u8>(MoveOnly(-3));
  for (const auto& m : y) {
    static_assert(std::same_as<decltype(m), const MoveOnly&>, "");
    EXPECT_EQ(m.i, -3);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

TEST(Result, FromIter) {
  enum class Error {
    OneError,
    TwoError,
  };

  auto no_errors = sus::Array<Result<usize, Error>, 5>(
                       Result<usize, Error>(1u), Result<usize, Error>(2u),
                       Result<usize, Error>(3u), Result<usize, Error>(4u),
                       Result<usize, Error>(5u))
                       .into_iter();

  auto no_errors_out =
      sus::move(no_errors).collect<Result<CollectSum<usize>, Error>>();
  EXPECT_EQ(no_errors_out, sus::Ok);
  EXPECT_EQ(sus::move(no_errors_out).unwrap().sum, 1u + 2u + 3u + 4u + 5u);

  auto with_error = sus::Array<Result<usize, Error>, 5>(
                        Result<usize, Error>(1u), Result<usize, Error>(2u),
                        Result<usize, Error>::with_err(Error::OneError),
                        Result<usize, Error>(4u), Result<usize, Error>(5u))
                        .into_iter();

  auto with_error_out =
      sus::move(with_error).collect<Result<CollectSum<usize>, Error>>();
  EXPECT_EQ(with_error_out, sus::Err);
  EXPECT_EQ(sus::move(with_error_out).unwrap_err(), Error::OneError);

  auto with_errors = sus::Array<Result<usize, Error>, 5>(
                         Result<usize, Error>(1u), Result<usize, Error>(2u),
                         Result<usize, Error>::with_err(Error::OneError),
                         Result<usize, Error>(4u),
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
  static_assert(::sus::mem::CloneFrom<Copy>);
  static_assert(::sus::mem::Move<Copy>);
  static_assert(::sus::mem::Copy<Result<Copy, i32>>);
  static_assert(::sus::mem::Clone<Result<Copy, i32>>);
  static_assert(::sus::mem::CloneFrom<Result<Copy, i32>>);
  static_assert(::sus::mem::Move<Result<Copy, i32>>);

  {
    const auto s = Result<Copy, i32>(Copy());
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Result<Copy, i32>>);
    EXPECT_EQ(s2, sus::Ok);
  }

  {
    const auto s = Result<Copy, i32>::with_err(2_i32);
    auto s2 = Result<Copy, i32>(Copy());
    sus::clone_into(s2, s);
    EXPECT_EQ(s2, sus::Err);
  }

  struct Clone {
    Clone(i32 i) : i(i) {}
    Clone clone() const { return Clone(i + 1_i32); }

    Clone(Clone&& o) = default;
    Clone& operator=(Clone&& o) = default;

    i32 i = 1_i32;
  };

  static_assert(!::sus::mem::Copy<Clone>);
  static_assert(::sus::mem::Clone<Clone>);
  static_assert(!::sus::mem::CloneFrom<Clone>);
  static_assert(::sus::mem::Move<Clone>);
  static_assert(!::sus::mem::Copy<Result<Clone, i32>>);
  static_assert(::sus::mem::Clone<Result<Clone, i32>>);
  static_assert(::sus::mem::CloneFrom<Result<Clone, i32>>);
  static_assert(::sus::mem::Move<Result<Clone, i32>>);

  {
    const auto s = Result<Clone, i32>(Clone(1));
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Result<Clone, i32>>);
    EXPECT_EQ(s.as_value().i, 1_i32);
    EXPECT_EQ(s2.as_value().i, 2_i32);
  }
  {
    const auto s = Result<Clone, i32>::with_err(2);
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Result<Clone, i32>>);
    EXPECT_EQ(s.as_err(), 2_i32);
    EXPECT_EQ(s2.as_err(), 2_i32);
  }
  {
    const auto s = Result<Clone, i32>(Clone(1));
    auto s2 = Result<Clone, i32>(Clone(4));
    sus::clone_into(s2, s);
    EXPECT_EQ(s.as_value().i, 1_i32);
    EXPECT_EQ(s2.as_value().i, 2_i32);
  }
  {
    const auto s = Result<Clone, i32>(Clone(1));
    auto s2 = Result<Clone, i32>::with_err(2);
    sus::clone_into(s2, s);
    EXPECT_EQ(s.as_value().i, 1_i32);
    EXPECT_EQ(s2.as_value().i, 2_i32);
  }
  {
    const auto s = Result<Clone, i32>::with_err(2);
    auto s2 = Result<Clone, i32>(Clone(1));
    sus::clone_into(s2, s);
    EXPECT_EQ(s.as_err(), 2_i32);
    EXPECT_EQ(s2.as_err(), 2_i32);
  }

  {
    const auto v = Result<void, Clone>(OkVoid());
    auto v2 = sus::clone(v);
    static_assert(std::same_as<decltype(v2), Result<void, Clone>>);
    EXPECT_TRUE(v.is_ok());
    EXPECT_TRUE(v2.is_ok());
  }
  {
    const auto v = Result<void, Clone>::with_err(Clone(1));
    auto v2 = sus::clone(v);
    static_assert(std::same_as<decltype(v2), Result<void, Clone>>);
    EXPECT_EQ(v.as_err().i, 1);
    EXPECT_EQ(v2.as_err().i, 2);
  }
  {
    const auto v = Result<void, Clone>(OkVoid());
    auto v2 = Result<void, Clone>(OkVoid());
    sus::clone_into(v2, v);
    EXPECT_TRUE(v.is_ok());
    EXPECT_TRUE(v2.is_ok());
  }
  {
    const auto v = Result<void, Clone>(OkVoid());
    auto v2 = Result<void, Clone>::with_err(Clone(2));
    sus::clone_into(v2, v);
    EXPECT_TRUE(v.is_ok());
    EXPECT_TRUE(v2.is_ok());
  }
  {
    const auto v = Result<void, Clone>::with_err(Clone(2));
    auto v2 = Result<void, Clone>(OkVoid());
    sus::clone_into(v2, v);
    EXPECT_EQ(v.as_err().i, 2);
    EXPECT_EQ(v2.as_err().i, 3);
  }

  auto m = NoCopyMove();
  {
    const auto v = Result<NoCopyMove&, i32>(m);
    auto v2 = sus::clone(v);
    static_assert(std::same_as<decltype(v2), Result<NoCopyMove&, i32>>);
    EXPECT_EQ(&v.as_value(), &m);
    EXPECT_EQ(&v2.as_value(), &m);
  }
  {
    const auto v = Result<NoCopyMove&, i32>::with_err(2);
    auto v2 = sus::clone(v);
    static_assert(std::same_as<decltype(v2), Result<NoCopyMove&, i32>>);
    EXPECT_EQ(v.as_err(), 2);
    EXPECT_EQ(v2.as_err(), 2);
  }
  {
    const auto v = Result<NoCopyMove&, i32>(m);
    auto v2 = Result<NoCopyMove&, i32>(m);
    sus::clone_into(v2, v);
    EXPECT_EQ(&v.as_value(), &m);
    EXPECT_EQ(&v2.as_value(), &m);
  }
  {
    const auto v = Result<NoCopyMove&, i32>(m);
    auto v2 = Result<NoCopyMove&, i32>::with_err(2);
    sus::clone_into(v2, v);
    EXPECT_EQ(&v.as_value(), &m);
    EXPECT_EQ(&v2.as_value(), &m);
  }
  {
    const auto v = Result<NoCopyMove&, i32>::with_err(2);
    auto v2 = Result<NoCopyMove&, i32>(m);
    sus::clone_into(v2, v);
    EXPECT_EQ(v.as_err(), 2);
    EXPECT_EQ(v2.as_err(), 2);
  }
}

TEST(Result, Eq) {
  struct NotEq {};
  static_assert(!sus::cmp::Eq<NotEq>);

  static_assert(::sus::cmp::Eq<Result<i32, i32>, Result<i32, i32>>);
  static_assert(::sus::cmp::Eq<Result<i32, i32>, Result<i64, i8>>);
  static_assert(!::sus::cmp::Eq<Result<i32, NotEq>, Result<i32, NotEq>>);
  static_assert(!::sus::cmp::Eq<Result<NotEq, i32>, Result<NotEq, i32>>);
  static_assert(!::sus::cmp::Eq<Result<NotEq, NotEq>, Result<NotEq, NotEq>>);

  EXPECT_EQ((Result<i32, i32>(1)), (Result<i32, i32>(1)));
  EXPECT_NE((Result<i32, i32>(1)), (Result<i32, i32>(2)));
  EXPECT_NE((Result<i32, i32>(1)), (Result<i32, i32>::with_err(1)));
  EXPECT_NE((Result<i32, i32>::with_err(1)), (Result<i32, i32>(1)));
  EXPECT_EQ((Result<i32, i32>::with_err(1)), (Result<i32, i32>::with_err(1)));

  EXPECT_EQ((Result<f32, i32>(1.f)), (Result<f32, i32>(1.f)));
  EXPECT_EQ((Result<f32, i32>(0.f)), (Result<f32, i32>(-0.f)));

  EXPECT_NE((Result<f32, i32>(f32::NAN)), (Result<f32, i32>(f32::NAN)));
  EXPECT_EQ((Result<i32, f32>::with_err(1.f)),
            (Result<i32, f32>::with_err(1.f)));
  EXPECT_EQ((Result<i32, f32>::with_err(0.f)),
            (Result<i32, f32>::with_err(-0.f)));
  EXPECT_NE((Result<i32, f32>::with_err(f32::NAN)),
            (Result<i32, f32>::with_err(f32::NAN)));

  // Comparison with marker types. EXPECT_EQ also converts it to a const
  // reference, so this tests that comparison from a const marker works (if the
  // inner type is copyable).
  EXPECT_EQ((Result<i32, i32>(1)), sus::ok(1));
  EXPECT_EQ((Result<i32, i32>::with_err(1)), sus::err(1));

  auto m = NoCopyMove();
  auto m2 = NoCopyMove();
  EXPECT_EQ((Result<NoCopyMove&, i32>(m)), (Result<NoCopyMove&, i32>(m)));
  EXPECT_NE((Result<NoCopyMove&, i32>(m)), (Result<NoCopyMove&, i32>(m2)));
  EXPECT_EQ((Result<NoCopyMove&, i32>::with_err(1)),
            (Result<NoCopyMove&, i32>::with_err(1)));
  EXPECT_NE((Result<NoCopyMove&, i32>::with_err(1)),
            (Result<NoCopyMove&, i32>::with_err(2)));
}

TEST(Result, StrongOrd) {
  static_assert(::sus::cmp::StrongOrd<Result<i32, i32>>);

  static_assert(Result<i32, i32>(1) < Result<i32, i32>(2));
  static_assert(Result<i32, i32>(3) > Result<i32, i32>(2));
  static_assert(Result<i32, i32>::with_err(1) < Result<i32, i32>::with_err(2));
  static_assert(Result<i32, i32>::with_err(3) > Result<i32, i32>::with_err(2));

  static_assert(Result<i32, i32>(1) > Result<i32, i32>::with_err(2));
  static_assert(Result<i32, i32>::with_err(1) < Result<i32, i32>(2));

  NoCopyMove m[2];
  EXPECT_LE((Result<NoCopyMove&, i32>(m[0])), (Result<NoCopyMove&, i32>(m[0])));
  EXPECT_LT((Result<NoCopyMove&, i32>(m[0])), (Result<NoCopyMove&, i32>(m[1])));
  EXPECT_LE((Result<NoCopyMove&, i32>::with_err(1)),
            (Result<NoCopyMove&, i32>::with_err(1)));
  EXPECT_LT((Result<NoCopyMove&, i32>::with_err(1)),
            (Result<NoCopyMove&, i32>::with_err(2)));
}

TEST(Result, StrongOrder) {
  static_assert(std::strong_order(Result<i32, i32>(12), Result<i32, i32>(12)) ==
                std::strong_ordering::equal);
  static_assert(std::strong_order(Result<i32, i32>(12), Result<i32, i32>(13)) ==
                std::strong_ordering::less);
  static_assert(std::strong_order(Result<i32, i32>(12), Result<i32, i32>(11)) ==
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

  static_assert(
      std::strong_order(Result<i32, i32>(12), Result<i32, i32>::with_err(12)) ==
      std::strong_ordering::greater);
  static_assert(
      std::strong_order(Result<i32, i32>::with_err(12), Result<i32, i32>(12)) ==
      std::strong_ordering::less);

  auto m = NoCopyMove();
  static_assert(std::strong_order(Result<NoCopyMove&, i32>(m),
                                  Result<NoCopyMove&, i32>::with_err(12)) ==
                std::strong_ordering::greater);
  static_assert(std::strong_order(Result<NoCopyMove&, i32>::with_err(12),
                                  Result<NoCopyMove&, i32>(m)) ==
                std::strong_ordering::less);
}

struct Weak {
  constexpr bool operator==(const Weak& o) const& noexcept {
    return a == o.a && b == o.b;
  }
  constexpr std::weak_ordering operator<=>(const Weak& o) const& noexcept {
    if (a == o.a) return std::weak_ordering::equivalent;
    if (a < o.a) return std::weak_ordering::less;
    return std::weak_ordering::greater;
  }

  constexpr Weak(int a, int b) : a(a), b(b) {}
  int a;
  int b;
};

TEST(Result, WeakOrder) {
  static_assert(!::sus::cmp::StrongOrd<Result<Weak, i32>>);
  static_assert(!::sus::cmp::StrongOrd<Result<i32, Weak>>);
  static_assert(!::sus::cmp::StrongOrd<Result<Weak, Weak>>);

  static_assert(::sus::cmp::Ord<Result<Weak, i32>>);
  static_assert(::sus::cmp::Ord<Result<i32, Weak>>);
  static_assert(::sus::cmp::Ord<Result<Weak, Weak>>);

  static_assert(std::weak_order(Result<Weak, i32>(Weak(1, 2)),
                                Result<Weak, i32>(Weak(1, 2))) ==
                std::weak_ordering::equivalent);
  static_assert(std::weak_order(Result<Weak, i32>(Weak(1, 2)),
                                Result<Weak, i32>(Weak(1, 3))) ==
                std::weak_ordering::equivalent);
  static_assert(std::weak_order(Result<Weak, i32>(Weak(1, 2)),
                                Result<Weak, i32>(Weak(2, 3))) ==
                std::weak_ordering::less);
  static_assert(std::weak_order(Result<Weak, i32>(Weak(2, 2)),
                                Result<Weak, i32>(Weak(1, 3))) ==
                std::weak_ordering::greater);
}

TEST(Result, PartialOrder) {
  static_assert(!::sus::cmp::StrongOrd<Result<f32, i8>>);
  static_assert(!::sus::cmp::StrongOrd<Result<i8, f32>>);
  static_assert(!::sus::cmp::StrongOrd<Result<f32, f32>>);

  static_assert(!::sus::cmp::Ord<Result<f32, i8>>);
  static_assert(!::sus::cmp::Ord<Result<i8, f32>>);
  static_assert(!::sus::cmp::Ord<Result<f32, f32>>);

  static_assert(::sus::cmp::PartialOrd<Result<f32, i8>>);
  static_assert(::sus::cmp::PartialOrd<Result<i8, f32>>);
  static_assert(::sus::cmp::PartialOrd<Result<f32, f32>>);

  static_assert(
      std::partial_order(Result<f32, i8>(0.0f), Result<f32, i8>(-0.0f)) ==
      std::partial_ordering::equivalent);
  static_assert(
      std::partial_order(Result<f32, i8>(1.0f), Result<f32, i8>(-0.0f)) ==
      std::partial_ordering::greater);
  static_assert(
      std::partial_order(Result<f32, i8>(0.0f), Result<f32, i8>(1.0f)) ==
      std::partial_ordering::less);
  EXPECT_EQ(
      std::partial_order(Result<f32, i8>(f32::NAN), Result<f32, i8>(f32::NAN)),
      std::partial_ordering::unordered);
}

TEST(Result, NoOrder) {
  struct NotCmp {};
  static_assert(!::sus::cmp::PartialOrd<NotCmp>);
  static_assert(!::sus::cmp::PartialOrd<Result<NotCmp, i8>>);
}

TEST(Result, UnwrapOrElse_BasicUsageExample) {
  enum class ECode { ItsHappening = -1 };
  auto conv = [](ECode e) { return static_cast<i32>(e); };
  auto ok = sus::Result<i32, ECode>(2);
  sus::check(sus::move(ok).unwrap_or_else(conv) == 2);
  auto err = sus::Result<i32, ECode>::with_err(ECode::ItsHappening);
  sus::check(sus::move(err).unwrap_or_else(conv) == -1);
}

TEST(Result, fmt) {
  static_assert(fmt::is_formattable<::sus::Result<i32, i32>, char>::value);
  EXPECT_EQ(fmt::format("{}", sus::Result<i32, i32>(12345)), "Ok(12345)");
  EXPECT_EQ(fmt::format("{:06}", sus::Result<i32, i32>(12345)),
            "Ok(012345)");  // The format string is for the Ok value.
  EXPECT_EQ(fmt::format("{}", sus::Result<i32, i32>::with_err(4321)),
            "Err(4321)");
  EXPECT_EQ(fmt::format("{:06}", sus::Result<i32, i32>::with_err(4321)),
            "Err(4321)");  // The format string is for the Ok value.
  EXPECT_EQ(fmt::format("{}", sus::Result<std::string_view, i32>("12345")),
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

  EXPECT_EQ(fmt::format("{}", sus::Result<i32, NoFormat>(12345)), "Ok(12345)");
  EXPECT_EQ(fmt::format("{}", sus::Result<i32, NoFormat>::with_err(NoFormat())),
            "Err(f2-3c-ae-16)");
  EXPECT_EQ(fmt::format("{}", sus::Result<NoFormat, i32>(NoFormat())),
            "Ok(f2-3c-ae-16)");
  EXPECT_EQ(fmt::format("{}", sus::Result<NoFormat, i32>::with_err(12345)),
            "Err(12345)");

  EXPECT_EQ(fmt::format("{}", sus::Result<void, i32>(OkVoid())), "Ok(<void>)");
  EXPECT_EQ(fmt::format("{}", sus::Result<void, i32>::with_err(12345)),
            "Err(12345)");
}

TEST(Result, Stream) {
  std::stringstream s;
  s << sus::Result<i32, i32>(12345) << " "
    << sus::Result<i32, i32>::with_err(-76543);
  EXPECT_EQ(s.str(), "Ok(12345) Err(-76543)");
}

TEST(Result, GTest) {
  EXPECT_EQ(testing::PrintToString(sus::Result<i32, i32>(12345)), "Ok(12345)");
}

TEST(Result, FromProduct) {
  enum E { ERROR };
  static_assert(sus::iter::Product<Result<i32, E>, Result<i32, E>>);

  // With a None.
  {
    auto a =
        sus::Array<Result<i32, E>, 3>(sus::ok(2), sus::err(ERROR), sus::ok(4));
    decltype(auto) o = sus::move(a).into_iter().product();
    static_assert(std::same_as<decltype(o), sus::Result<i32, E>>);
    EXPECT_EQ(o.as_err(), ERROR);
  }
  // Without a None.
  {
    auto a = sus::Array<Result<i32, E>, 3>(sus::ok(2), sus::ok(3), sus::ok(4));
    decltype(auto) o = sus::move(a).into_iter().product();
    static_assert(std::same_as<decltype(o), sus::Result<i32, E>>);
    EXPECT_EQ(o.as_value(), 2 * 3 * 4);
  }
}

TEST(Result, FromSum) {
  enum E { ERROR };
  static_assert(sus::iter::Sum<Result<i32, E>, Result<i32, E>>);

  // With a None.
  {
    auto a =
        sus::Array<Result<i32, E>, 3>(sus::ok(2), sus::err(ERROR), sus::ok(4));
    decltype(auto) o = sus::move(a).into_iter().sum();
    static_assert(std::same_as<decltype(o), sus::Result<i32, E>>);
    EXPECT_EQ(o.as_err(), ERROR);
  }
  // Without a None.
  {
    auto a = sus::Array<Result<i32, E>, 3>(sus::ok(2), sus::ok(3), sus::ok(4));
    decltype(auto) o = sus::move(a).into_iter().sum();
    static_assert(std::same_as<decltype(o), sus::Result<i32, E>>);
    EXPECT_EQ(o.as_value(), 2 + 3 + 4);
  }
}

TEST(Result, Try) {
  static_assert(sus::ops::Try<Result<i32, u32>>);
  EXPECT_EQ((sus::ops::TryImpl<Result<i32, u32>>::is_success(sus::ok(1_i32))),
            true);
  EXPECT_EQ((sus::ops::TryImpl<Result<i32, u32>>::is_success(sus::err(2_u32))),
            false);
}

template <class E>
concept AsValueRvalue = requires {
  { Result<i32, E>(2_i32).as_value() };
};

TEST(Result, AsValue) {
  auto x = Result<i32, u8>(2_i32);
  static_assert(std::same_as<decltype(x.as_value()), const i32&>);
  EXPECT_EQ(x.as_value(), 2);

  static_assert(!AsValueRvalue<u8>);

  auto y = Result<i32, u8>(2_i32);
  static_assert(std::same_as<decltype(y.as_value_mut()), i32&>);
  EXPECT_EQ(y.as_value_mut(), 2);
}

}  // namespace
