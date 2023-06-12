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
#include "subspace/test/no_copy_move.h"

namespace {

using sus::Result;
using namespace sus::test;

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

TEST(Option, Construct) {
  {
    using T = DefaultConstructible;
    auto x = Result<T, i32>::with(T());
    auto y = Result<T, i32>::with_err(1);
    auto t = T();
    auto z = Result<T, i32>::with(t);
  }
  {
    using T = NotDefaultConstructible;
    auto x = Result<T, i32>::with(T(1));
    auto y = Result<T, i32>::with_err(1);
    auto t = T(1);
    auto z = Result<T, i32>::with(t);
  }
  {
    using T = TriviallyCopyable;
    auto x = Result<T, i32>::with(T(1));
    auto y = Result<T, i32>::with_err(1);
    auto t = T(1);
    auto z = Result<T, i32>::with(t);
  }
  {
    using T = TriviallyMoveableAndRelocatable;
    auto x = Result<T, i32>::with(T(1));
    auto y = Result<T, i32>::with_err(1);
    // Not copyable.
    // auto t = T(1);
    // auto z = Result<T, i32>::with(t);
  }
  {
    using T = TriviallyCopyableNotDestructible;
    auto x = Result<T, i32>::with(T(1));
    auto y = Result<T, i32>::with_err(1);
    auto t = T(1);
    auto z = Result<T, i32>::with(t);
  }
  {
    using T = TriviallyMoveableNotDestructible;
    auto x = Result<T, i32>::with(T(1));
    auto y = Result<T, i32>::with_err(1);
    // Not copyable.
    // auto t = T(1);
    // auto z = Result<T, i32>::with(t);
  }
  {
    using T = NotTriviallyRelocatableCopyableOrMoveable;
    auto x = Result<T, i32>::with(T(1));
    auto y = Result<T, i32>::with_err(1);
    // Not copyable.
    // auto t = T(1);
    // auto z = Result<T, i32>::with(t);
  }
  {
    using T = TrivialAbiRelocatable;
    auto x = Result<T, i32>::with(T(1));
    auto y = Result<T, i32>::with_err(1);
    // Not copyable.
    // auto t = T(1);
    // auto z = Result<T, i32>::with(t);
  }
  {
    using T = const NoCopyMove&;
    auto i = NoCopyMove();
    auto x = Result<T, i32>::with(static_cast<T>(i));
    auto y = Result<T, i32>::with_err(1);
    T t = i;
    auto z = Result<T, i32>::with(t);
  }
  {
    using T = NoCopyMove&;
    auto i = NoCopyMove();
    auto x = Result<T, i32>::with(mref(static_cast<T>(i)));
    auto y = Result<T, i32>::with_err(1);
    T t = i;
    auto z = Result<T, i32>::with(mref(t));
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
    auto r = Result<T, E>::with(T());
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
    auto r = Result<T, int>::with(T());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(t_destructed, 1_usize);
  {
    auto r = Result<T, int>::with_err(2);
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(t_destructed, 0_usize);
  {
    auto r = Result<int, E>::with(2);
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
    auto r = Result<NoCopyMove&, E>::with(m);
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 0_usize);
  {
    auto r = Result<NoCopyMove&, E>::with_err(E());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 1_usize);
  { auto r = Result<NoCopyMove&, int>::with(m); }
  { auto r = Result<NoCopyMove&, int>::with_err(2); }

  {
    auto r = Result<void, E>::with();
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 0_usize);
  {
    auto r = Result<void, E>::with_err(E());
    t_destructed = e_destructed = 0_usize;
  }
  EXPECT_EQ(e_destructed, 1_usize);
  { auto r = Result<void, int>::with(); }
  { auto r = Result<void, int>::with_err(2); }
}

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
  constexpr bool a = Result<i32, Error>::with(3_i32).is_ok();
  EXPECT_TRUE(a);

  constexpr bool b = Result<i32, Error>::with_err(Error()).is_ok();
  EXPECT_FALSE(b);

  auto m = NoCopyMove();
  constexpr bool c = Result<NoCopyMove&, Error>::with(m).is_ok();
  EXPECT_TRUE(c);
}

TEST(Result, IsErr) {
  constexpr bool a = Result<i32, Error>::with(3_i32).is_err();
  EXPECT_FALSE(a);

  constexpr bool b = Result<i32, Error>::with_err(Error()).is_err();
  EXPECT_TRUE(b);

  constexpr bool c = Result<NoCopyMove&, Error>::with_err(Error()).is_err();
  EXPECT_TRUE(c);
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

  auto m = NoCopyMove();
  switch (Result<NoCopyMove&, Error>::with(m)) {
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

  auto m = NoCopyMove();
  {
    auto r = Result<NoCopyMove&, E>::with(m);
    e_destructed = 0u;
    auto o = sus::move(r).ok();
    static_assert(std::same_as<decltype(o), sus::Option<NoCopyMove&>>);
    EXPECT_EQ(&o.as_value(), &m);
    EXPECT_EQ(e_destructed, 0u);
  }
  {
    auto r = Result<const NoCopyMove&, E>::with(m);
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

  auto m = NoCopyMove();
  decltype(auto) u = Result<NoCopyMove&, Error>::with(m).unwrap();
  static_assert(std::same_as<decltype(u), NoCopyMove&>);
  EXPECT_EQ(&u, &m);

  decltype(auto) cu = Result<const NoCopyMove&, Error>::with(m).unwrap();
  static_assert(std::same_as<decltype(cu), const NoCopyMove&>);
  EXPECT_EQ(&cu, &m);
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

TEST(Result, Copy) {
  // This type has a user defined copy constructor, which deletes the implicit
  // copy constructor in Option.
  auto static copied = 0_usize;
  struct Type {
    explicit Type() noexcept = default;
    Type(const Type&) noexcept { copied += 1; }
    Type& operator=(const Type&) noexcept { return copied += 1, *this; }

    constexpr bool operator==(const Type& rhs) const& noexcept {
      return this == &rhs;
    }
  };

  copied = 0_usize;
  {
    auto x = Result<Type, i32>::with(Type());
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
    auto x = Result<i32, Type>::with(2);
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
    auto rv = Result<void, Type>::with();
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
    auto rv = Result<void, Type>::with();
    auto rv2 = Result<void, Type>::with_err(Type());
    EXPECT_EQ(copied, 1u);
    rv = rv2;
    EXPECT_EQ(copied, 2u);
    EXPECT_EQ(rv.is_err(), true);
    EXPECT_EQ(rv2.is_err(), true);
  }
  {
    copied = 0_usize;
    auto rv = Result<void, Type>::with();
    auto rv2 = Result<void, Type>::with_err(Type());
    EXPECT_EQ(copied, 1u);
    rv2 = rv;
    EXPECT_EQ(rv.is_ok(), true);
    EXPECT_EQ(rv2.is_ok(), true);
  }

  auto m = NoCopyMove();

  {
    auto z = Result<NoCopyMove&, int>::with(m);
    auto zz = z;
    EXPECT_EQ(&z.as_ok(), &m);
    EXPECT_EQ(&zz.as_ok(), &m);
  }
  {
    auto z = Result<NoCopyMove&, int>::with_err(2);
    auto zz = z;
    EXPECT_EQ(z.as_err(), 2);
    EXPECT_EQ(zz.as_err(), 2);
  }
  {
    auto z =
        Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::with(m);
    auto zz = z;
    EXPECT_EQ(&z.as_ok(), &m);
    EXPECT_EQ(&zz.as_ok(), &m);
  }
  {
    auto z = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    auto zz = z;
    EXPECT_EQ(z.as_err().i, 2);
    EXPECT_EQ(zz.as_err().i, 2);
  }
  {
    auto z = Result<NoCopyMove&, int>::with(m);
    auto zz = Result<NoCopyMove&, int>::with_err(2);
    z = zz;
    EXPECT_EQ(z.as_err(), 2);
    EXPECT_EQ(zz.as_err(), 2);
  }
  {
    auto z = Result<NoCopyMove&, int>::with(m);
    auto zz = Result<NoCopyMove&, int>::with_err(2);
    zz = z;
    EXPECT_EQ(&z.as_ok(), &m);
    EXPECT_EQ(&zz.as_ok(), &m);
  }
  {
    auto z =
        Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::with(m);
    auto zz = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    z = zz;
    EXPECT_EQ(z.as_err().i, 2);
    EXPECT_EQ(zz.as_err().i, 2);
  }
  {
    auto z =
        Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::with(m);
    auto zz = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    zz = z;
    EXPECT_EQ(&z.as_ok(), &m);
    EXPECT_EQ(&zz.as_ok(), &m);
  }
}

TEST(Result, Move) {
  // This type has a user defined move constructor, which deletes the implicit
  // move constructor in Option.
  struct Type {
    Type() = default;
    Type(Type&&) = default;
    Type& operator=(Type&&) = default;
  };
  auto x = Result<Type, i32>::with(Type());
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
  auto a = Result<MoveableLvalue, i32>::with(lvalue);
  EXPECT_EQ(a.as_ok().i, 2);
  EXPECT_EQ(lvalue.i, 2);

  auto b = Result<MoveableLvalue, i32>::with(sus::move(lvalue));
  EXPECT_EQ(b.as_ok().i, 2);
  EXPECT_EQ(lvalue.i, 0);

  {
    auto z = Result<void, int>::with();
    auto zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
    z = sus::move(zz);
    EXPECT_EQ(z.is_ok(), true);
  }
  {
    auto z = Result<void, NotTriviallyRelocatableCopyableOrMoveable>::with();
    auto zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
    z = sus::move(zz);
    EXPECT_EQ(z.is_ok(), true);
  }
  {
    auto z = Result<void, int>::with();
    auto zz = Result<void, int>::with_err(2);
    z = sus::move(zz);
    EXPECT_EQ(z.as_err(), 2);
  }
  {
    auto z = Result<void, int>::with();
    auto zz = Result<void, int>::with_err(2);
    zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
  }
  {
    auto z = Result<void, NotTriviallyRelocatableCopyableOrMoveable>::with();
    auto zz = Result<void, NotTriviallyRelocatableCopyableOrMoveable>::with_err(
        NotTriviallyRelocatableCopyableOrMoveable(2));
    z = sus::move(zz);
    EXPECT_EQ(z.as_err().i, 2);
  }
  {
    auto z = Result<void, NotTriviallyRelocatableCopyableOrMoveable>::with();
    auto zz = Result<void, NotTriviallyRelocatableCopyableOrMoveable>::with_err(
        NotTriviallyRelocatableCopyableOrMoveable(2));
    zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
  }

  {
    auto m = NoCopyMove();
    auto z = Result<NoCopyMove&, int>::with(m);
    auto zz = sus::move(z);
    EXPECT_EQ(&zz.as_ok(), &m);
    z = sus::move(zz);
    EXPECT_EQ(&z.as_ok(), &m);
  }
  {
    auto m = NoCopyMove();
    auto z =
        Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::with(m);
    auto zz = sus::move(z);
    EXPECT_EQ(&zz.as_ok(), &m);
    z = sus::move(zz);
    EXPECT_EQ(&z.as_ok(), &m);
  }
  {
    auto m = NoCopyMove();
    auto z = Result<NoCopyMove&, int>::with(m);
    auto zz = Result<NoCopyMove&, int>::with_err(2);
    z = sus::move(zz);
    EXPECT_EQ(z.as_err(), 2);
  }
  {
    auto m = NoCopyMove();
    auto z = Result<NoCopyMove&, int>::with(m);
    auto zz = Result<NoCopyMove&, int>::with_err(2);
    zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
  }
  {
    auto m = NoCopyMove();
    auto z =
        Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::with(m);
    auto zz = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    z = sus::move(zz);
    EXPECT_EQ(z.as_err().i, 2);
  }
  {
    auto m = NoCopyMove();
    auto z =
        Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::with(m);
    auto zz = Result<NoCopyMove&, NotTriviallyRelocatableCopyableOrMoveable>::
        with_err(NotTriviallyRelocatableCopyableOrMoveable(2));
    zz = sus::move(z);
    EXPECT_EQ(zz.is_ok(), true);
  }
}

TEST(Result, MoveAfterTrivialMove) {
  {
    auto r = Result<i32, i32>::with(1_i32);
    auto r3 = sus::move(r);
    auto r2 = sus::move(r3);
    EXPECT_EQ(sus::move(r2).unwrap(), 1_i32);
  }
  {
    auto r = Result<i32, i32>::with(1_i32);
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
    auto rv = Result<void, i32>::with();
    auto rv3 = sus::move(rv);
    auto rv2 = sus::move(rv3);
    EXPECT_TRUE(rv2.is_ok());
  }

  auto m = NoCopyMove();
  {
    auto rv = Result<NoCopyMove&, i32>::with(m);
    auto rv3 = sus::move(rv);
    auto rv2 = sus::move(rv3);
    EXPECT_EQ(&rv2.as_ok(), &m);
  }
}

TEST(Result, AssignAfterTrivialMove) {
  {
    auto r = Result<i32, i32>::with(1_i32);
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap(), 1_i32);
  }
  {
    auto r = Result<i32, i32>::with(1_i32);
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
    r = Result<i32, i32>::with(2_i32);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err(), 2_i32);
  }

  {
    auto rv = Result<void, i32>::with();
    auto rv3 = sus::move(rv);
    rv = sus::move(rv3);
    EXPECT_TRUE(rv.is_ok());
  }
  {
    auto rv = Result<void, i32>::with();
    auto rv3 = sus::move(rv);
    rv = Result<void, i32>::with_err(2);
    rv = sus::move(rv3);
    EXPECT_TRUE(rv.is_ok());
  }

  auto m = NoCopyMove();
  {
    auto rv = Result<NoCopyMove&, i32>::with(m);
    auto rv3 = sus::move(rv);
    rv = sus::move(rv3);
    EXPECT_EQ(&rv.as_ok(), &m);
  }
  {
    auto rv = Result<NoCopyMove&, i32>::with(m);
    auto rv3 = sus::move(rv);
    rv = Result<NoCopyMove&, i32>::with_err(2);
    rv = sus::move(rv3);
    EXPECT_EQ(&rv.as_ok(), &m);
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
    auto r = Result<NonTrivialMove, i32>::with(NonTrivialMove(1));
    sus::move(r).unwrap();
    EXPECT_DEATH([[maybe_unused]] auto r2 = sus::move(r), "");
  }
  {
    auto r = Result<i32, NonTrivialMove>::with(1);
    sus::move(r).unwrap();
    EXPECT_DEATH([[maybe_unused]] auto r2 = sus::move(r), "");
  }
  {
    auto r = Result<void, NonTrivialMove>::with();
    sus::move(r).unwrap();
    EXPECT_DEATH([[maybe_unused]] auto r2 = sus::move(r), "");
  }

  auto m = NoCopyMove();
  {
    auto rv = Result<NoCopyMove&, NonTrivialMove>::with(m);
    auto rv3 = sus::move(rv);
    rv = sus::move(rv3);
    EXPECT_EQ(&rv.as_ok(), &m);
  }
}

TEST(Result, AssignAfterNonTrivialMove) {
  {
    auto r = Result<NonTrivialMove, i32>::with(NonTrivialMove(1));
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap().i, 1_i32);
  }
  {
    auto r = Result<NonTrivialMove, i32>::with(NonTrivialMove(1));
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
    r = Result<NonTrivialMove, i32>::with(NonTrivialMove(1));
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err(), 2_i32);
  }

  {
    auto r = Result<i32, NonTrivialMove>::with(1);
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap(), 1_i32);
  }
  {
    auto r = Result<i32, NonTrivialMove>::with(1);
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
    r = Result<i32, NonTrivialMove>::with(1);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err().i, 2_i32);
  }

  {
    auto r = Result<void, NonTrivialMove>::with();
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(r, sus::Ok);
  }
  {
    auto r = Result<void, NonTrivialMove>::with();
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
    r = Result<void, NonTrivialMove>::with();
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err().i, 2_i32);
  }

  auto m = NoCopyMove();
  {
    auto r = Result<NoCopyMove&, NonTrivialMove>::with(m);
    auto r3 = sus::move(r);
    r = sus::move(r3);
    EXPECT_EQ(&r.as_ok(), &m);
  }
  {
    auto r = Result<NoCopyMove&, NonTrivialMove>::with(m);
    auto r3 = sus::move(r);
    r = Result<NoCopyMove&, NonTrivialMove>::with_err(NonTrivialMove(1));
    r = sus::move(r3);
    EXPECT_EQ(&r.as_ok(), &m);
  }

  {
    auto r = Result<NoCopyMove&, NonTrivialMove>::with_err(NonTrivialMove(2));
    auto r3 = sus::move(r);
    r = Result<NoCopyMove&, NonTrivialMove>::with(m);
    r = sus::move(r3);
    EXPECT_EQ(sus::move(r).unwrap_err().i, 2_i32);
  }

  auto m2 = NoCopyMove();
  {
    auto r = Result<NoCopyMove&, NonTrivialMove>::with(m);
    auto r2 = sus::move(r);
    r = Result<NoCopyMove&, NonTrivialMove>::with(m2);
    EXPECT_EQ(&r.as_ok(), &m2);
    EXPECT_EQ(&r2.as_ok(), &m);
  }
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

  auto m = NoCopyMove();
  auto rm = Result<NoCopyMove&, i32>::with(m);
  rm = sus::move(rm);
  EXPECT_EQ(&rm.as_ok(), &m);
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

  auto m = NoCopyMove();
  auto rm = Result<NoCopyMove&, i32>::with(m);
  rm = rm;
  EXPECT_EQ(&rm.as_ok(), &m);
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

  auto m = NoCopyMove();
  auto rm = Result<NoCopyMove&, i32>::with(m);
  sus::clone_into(rm, rm);
  EXPECT_EQ(&rm.as_ok(), &m);
}

TEST(Result, Iter) {
  auto x = Result<i32, u8>::with_err(2_u8);
  for ([[maybe_unused]] auto i : x.iter()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Result<i32, u8>::with(-4_i32);
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
    auto ok = Result<NoCopyMove&, u8>::with(m);
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
    auto it = Result<NoCopyMove&, u8>::with(m).iter();
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
  auto y = Result<i32, u8>::with(-3_i32);
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
    auto ok = Result<NoCopyMove&, u8>::with(m);
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
    auto it = Result<NoCopyMove&, u8>::with(m).iter_mut();
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
  auto y = Result<MoveOnly, u8>::with(MoveOnly(-3));
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
    auto ok = Result<NoCopyMove&, u8>::with(m);
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
  auto y = Result<MoveOnly, u8>::with(MoveOnly(-3));
  for (const auto& m : y) {
    static_assert(std::same_as<decltype(m), const MoveOnly&>, "");
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
    Clone(i32 i) : i(i) {}
    Clone clone() const { return Clone(i + 1_i32); }

    Clone(Clone&& o) = default;
    Clone& operator=(Clone&& o) = default;

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
    const auto s = Result<Clone, i32>::with(Clone(1));
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Result<Clone, i32>>);
    EXPECT_EQ(s.as_ok().i, 1_i32);
    EXPECT_EQ(s2.as_ok().i, 2_i32);
  }
  {
    const auto s = Result<Clone, i32>::with_err(2);
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Result<Clone, i32>>);
    EXPECT_EQ(s.as_err(), 2_i32);
    EXPECT_EQ(s2.as_err(), 2_i32);
  }
  {
    const auto s = Result<Clone, i32>::with(Clone(1));
    auto s2 = Result<Clone, i32>::with(Clone(4));
    sus::clone_into(s2, s);
    EXPECT_EQ(s.as_ok().i, 1_i32);
    EXPECT_EQ(s2.as_ok().i, 2_i32);
  }
  {
    const auto s = Result<Clone, i32>::with(Clone(1));
    auto s2 = Result<Clone, i32>::with_err(2);
    sus::clone_into(s2, s);
    EXPECT_EQ(s.as_ok().i, 1_i32);
    EXPECT_EQ(s2.as_ok().i, 2_i32);
  }
  {
    const auto s = Result<Clone, i32>::with_err(2);
    auto s2 = Result<Clone, i32>::with(Clone(1));
    sus::clone_into(s2, s);
    EXPECT_EQ(s.as_err(), 2_i32);
    EXPECT_EQ(s2.as_err(), 2_i32);
  }

  {
    const auto v = Result<void, Clone>::with();
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
    const auto v = Result<void, Clone>::with();
    auto v2 = Result<void, Clone>::with();
    sus::clone_into(v2, v);
    EXPECT_TRUE(v.is_ok());
    EXPECT_TRUE(v2.is_ok());
  }
  {
    const auto v = Result<void, Clone>::with();
    auto v2 = Result<void, Clone>::with_err(Clone(2));
    sus::clone_into(v2, v);
    EXPECT_TRUE(v.is_ok());
    EXPECT_TRUE(v2.is_ok());
  }
  {
    const auto v = Result<void, Clone>::with_err(Clone(2));
    auto v2 = Result<void, Clone>::with();
    sus::clone_into(v2, v);
    EXPECT_EQ(v.as_err().i, 2);
    EXPECT_EQ(v2.as_err().i, 3);
  }

  auto m = NoCopyMove();
  {
    const auto v = Result<NoCopyMove&, i32>::with(m);
    auto v2 = sus::clone(v);
    static_assert(std::same_as<decltype(v2), Result<NoCopyMove&, i32>>);
    EXPECT_EQ(&v.as_ok(), &m);
    EXPECT_EQ(&v2.as_ok(), &m);
  }
  {
    const auto v = Result<NoCopyMove&, i32>::with_err(2);
    auto v2 = sus::clone(v);
    static_assert(std::same_as<decltype(v2), Result<NoCopyMove&, i32>>);
    EXPECT_EQ(v.as_err(), 2);
    EXPECT_EQ(v2.as_err(), 2);
  }
  {
    const auto v = Result<NoCopyMove&, i32>::with(m);
    auto v2 = Result<NoCopyMove&, i32>::with(m);
    sus::clone_into(v2, v);
    EXPECT_EQ(&v.as_ok(), &m);
    EXPECT_EQ(&v2.as_ok(), &m);
  }
  {
    const auto v = Result<NoCopyMove&, i32>::with(m);
    auto v2 = Result<NoCopyMove&, i32>::with_err(2);
    sus::clone_into(v2, v);
    EXPECT_EQ(&v.as_ok(), &m);
    EXPECT_EQ(&v2.as_ok(), &m);
  }
  {
    const auto v = Result<NoCopyMove&, i32>::with_err(2);
    auto v2 = Result<NoCopyMove&, i32>::with(m);
    sus::clone_into(v2, v);
    EXPECT_EQ(v.as_err(), 2);
    EXPECT_EQ(v2.as_err(), 2);
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

  auto m = NoCopyMove();
  auto m2 = NoCopyMove();
  EXPECT_EQ((Result<NoCopyMove&, i32>::with(m)),
            (Result<NoCopyMove&, i32>::with(m)));
  EXPECT_NE((Result<NoCopyMove&, i32>::with(m)),
            (Result<NoCopyMove&, i32>::with(m2)));
  EXPECT_EQ((Result<NoCopyMove&, i32>::with_err(1)),
            (Result<NoCopyMove&, i32>::with_err(1)));
  EXPECT_NE((Result<NoCopyMove&, i32>::with_err(1)),
            (Result<NoCopyMove&, i32>::with_err(2)));
}

TEST(Result, Ord) {
  static_assert(::sus::ops::Ord<Result<i32, i32>>);

  static_assert(Result<i32, i32>::with(1) < Result<i32, i32>::with(2));
  static_assert(Result<i32, i32>::with(3) > Result<i32, i32>::with(2));
  static_assert(Result<i32, i32>::with_err(1) < Result<i32, i32>::with_err(2));
  static_assert(Result<i32, i32>::with_err(3) > Result<i32, i32>::with_err(2));

  static_assert(Result<i32, i32>::with(1) > Result<i32, i32>::with_err(2));
  static_assert(Result<i32, i32>::with_err(1) < Result<i32, i32>::with(2));

  NoCopyMove m[2];
  EXPECT_LE((Result<NoCopyMove&, i32>::with(m[0])),
            (Result<NoCopyMove&, i32>::with(m[0])));
  EXPECT_LT((Result<NoCopyMove&, i32>::with(m[0])),
            (Result<NoCopyMove&, i32>::with(m[1])));
  EXPECT_LE((Result<NoCopyMove&, i32>::with_err(1)),
            (Result<NoCopyMove&, i32>::with_err(1)));
  EXPECT_LT((Result<NoCopyMove&, i32>::with_err(1)),
            (Result<NoCopyMove&, i32>::with_err(2)));
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

  auto m = NoCopyMove();
  static_assert(std::strong_order(Result<NoCopyMove&, i32>::with(m),
                                  Result<NoCopyMove&, i32>::with_err(12)) ==
                std::strong_ordering::greater);
  static_assert(std::strong_order(Result<NoCopyMove&, i32>::with_err(12),
                                  Result<NoCopyMove&, i32>::with(m)) ==
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

  EXPECT_EQ(fmt::format("{}", sus::Result<void, i32>::with()), "Ok(<void>)");
  EXPECT_EQ(fmt::format("{}", sus::Result<void, i32>::with_err(12345)),
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
