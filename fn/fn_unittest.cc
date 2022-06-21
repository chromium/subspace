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

#include "fn/fn.h"

#include <concepts>

#include "mem/forward.h"
#include "mem/replace.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::fn::Fn;
using sus::fn::FnMut;
using sus::fn::FnOnce;

struct Copyable {
  Copyable(int i) : i(i) {}
  Copyable(const Copyable& c) = default;
  Copyable& operator=(const Copyable& c) = default;
  ~Copyable() { i = -10000000; }

  int i = 0;
};

struct MoveOnly {
  MoveOnly(int i) : i(i) {}
  MoveOnly(const MoveOnly& c) = delete;
  MoveOnly& operator=(const MoveOnly& c) = delete;
  MoveOnly(MoveOnly&& c) : i(::sus::mem::replace(mref(c.i), -1)) {}
  MoveOnly& operator=(MoveOnly&& c) {
    i = ::sus::mem::replace(mref(c.i), -1);
    return *this;
  }
  ~MoveOnly() { i = -10000000; }

  int i = 0;
};

#if 0

template <class Fn, class Functor, class... StoredArgs>
concept has_with = requires(Functor f, StoredArgs... args) {
                                { Fn::with(f, args...) };
                              };
template <class Fn, class Functor, class... StoredArgs>
concept has_with = requires(Functor f, StoredArgs... args) {
                             { Fn::with(f, args...) };
                           };

void v_v_function() {}
using VVFunction = decltype(v_v_function);
int i_f_function(float) { return 1; }
using IFFunction = decltype(i_f_function);
using VVLambda = decltype([]() {});
using VVLambdaBound = decltype([i = int(2)]() {});
using VVLambdaBoundMut = decltype([i = int(2)]() mutable {});
using IFLambda = decltype([](float) { return 1; });

static_assert(sizeof(FnOnce<void()>) > sizeof(void (*)()));
static_assert(sizeof(FnOnce<void()>) <= sizeof(void (*)()) * 2);

// clang-format off

// A function pointer can be wrapped in Fn directly or from a lambda. There's no
// real difference between Fn/FnMut/FnOnce for a function pointer, for
// construction, as there's no storage.
static_assert(has_with<FnOnce<void()>, VVFunction>);
static_assert(has_with<FnOnce<void()>, VVLambda>);
static_assert(has_with<FnMut<void()>, VVFunction>);
static_assert(has_with<FnMut<void()>, VVLambda>);
static_assert(has_with<Fn<void()>, VVFunction>);
static_assert(has_with<Fn<void()>, VVLambda>);
// Non-void arguments with a function pointer.
static_assert(has_with<FnOnce<int(float)>, IFFunction>);
static_assert(has_with<FnOnce<int(float)>, IFLambda>);
static_assert(has_with<FnMut<int(float)>, IFFunction>);
static_assert(has_with<FnMut<int(float)>, IFLambda>);
static_assert(has_with<Fn<int(float)>, IFFunction>);
static_assert(has_with<Fn<int(float)>, IFLambda>);
// Stored arguments can't be passed for a function pointer.
static_assert(!has_with<FnOnce<int(float)>, IFFunction, float>);
static_assert(!has_with<FnMut<int(float)>, IFFunction, float>);
static_assert(!has_with<Fn<int(float)>, IFFunction, float>);
// A lambda with bound args can't be passed as a function pointer.
static_assert(!has_with<FnOnce<void()>, VVLambdaBound>);
static_assert(!has_with<FnMut<void()>, VVLambdaBound>);
static_assert(!has_with<Fn<void()>, VVLambdaBound>);
static_assert(!has_with<FnOnce<void()>, VVLambdaBoundMut>);
static_assert(!has_with<FnMut<void()>, VVLambdaBoundMut>);
static_assert(!has_with<Fn<void()>, VVLambdaBoundMut>);

// Storage can be requested even if it's not used.
static_assert(has_with<FnOnce<void()>, VVFunction>);
static_assert(has_with<FnOnce<void()>, VVLambda>);
static_assert(has_with<FnMut<void()>, VVFunction>);
static_assert(has_with<FnMut<void()>, VVLambda>);
static_assert(has_with<Fn<void()>, VVFunction>);
static_assert(has_with<Fn<void()>, VVLambda>);
// With storage, lambdas with bounds can be used.
static_assert(has_with<FnOnce<void()>, VVLambdaBound>);
static_assert(has_with<FnMut<void()>, VVLambdaBound>);
static_assert(has_with<Fn<void()>, VVLambdaBound>);
static_assert(has_with<FnOnce<void()>, VVLambdaBoundMut>);
static_assert(has_with<FnMut<void()>, VVLambdaBoundMut>);
// But a mutable lambda can't be stored in a (const) Fn.
static_assert(!has_with<Fn<void()>, VVLambdaBoundMut>);
// Stored variables, without call variables. They must match in number.
static_assert(has_with<FnOnce<void()>, decltype([](int) {}), int>);
static_assert(has_with<FnOnce<void()>, decltype([](int, int) {}), int, int>);
static_assert(!has_with<FnOnce<void()>, decltype([](int) {}), int, int>);
static_assert(!has_with<FnOnce<void()>, decltype([](int, int) {}), int>);
static_assert(has_with<FnMut<void()>, decltype([](int) {}), int>);
static_assert(has_with<FnMut<void()>, decltype([](int, int) {}), int, int>);
static_assert(!has_with<FnMut<void()>, decltype([](int) {}), int, int>);
static_assert(!has_with<FnMut<void()>, decltype([](int, int) {}), int>);
static_assert(has_with<Fn<void()>, decltype([](int) {}), int>);
static_assert(has_with<Fn<void()>, decltype([](int, int) {}), int, int>);
static_assert(!has_with<Fn<void()>, decltype([](int) {}), int, int>);
static_assert(!has_with<Fn<void()>, decltype([](int, int) {}), int>);
// Stored variables, with call variables. They must match in number.
static_assert(has_with<FnOnce<void(int)>, decltype([](int, int) {}), int>);
static_assert(has_with<FnOnce<void(int)>, decltype([](int, int, int) {}), int, int>);
static_assert(has_with<FnOnce<void(int, int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with<FnOnce<void(int)>, decltype([](int, int) {}), int, int>);
static_assert(!has_with<FnOnce<void(int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with<FnOnce<void(int, int)>, decltype([](int, int) {}), int>);
static_assert(has_with<FnMut<void(int)>, decltype([](int, int) {}), int>);
static_assert(has_with<FnMut<void(int)>, decltype([](int, int, int) {}), int, int>);
static_assert(has_with<FnMut<void(int, int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with<FnMut<void(int)>, decltype([](int, int) {}), int, int>);
static_assert(!has_with<FnMut<void(int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with<FnMut<void(int, int)>, decltype([](int, int) {}), int>);
static_assert(has_with<Fn<void(int)>, decltype([](int, int) {}), int>);
static_assert(has_with<Fn<void(int)>, decltype([](int, int, int) {}), int, int>);
static_assert(has_with<Fn<void(int, int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with<Fn<void(int)>, decltype([](int, int) {}), int, int>);
static_assert(!has_with<Fn<void(int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with<Fn<void(int, int)>, decltype([](int, int) {}), int>);
// FnOnce moves its storage to the receiver function. It must be able to receive
// that, but we allow it to receive it as a mutable reference as well. In
// practice, that means it can receive in any way it likes.
static_assert(has_with<FnOnce<void()>, decltype([](const MoveOnly&) {}), MoveOnly>);
static_assert(has_with<FnOnce<void()>, decltype([](MoveOnly&) {}), MoveOnly>);
static_assert(has_with<FnOnce<void()>, decltype([](MoveOnly&&) {}), MoveOnly>);
static_assert(has_with<FnOnce<void()>, decltype([](MoveOnly) {}), MoveOnly>);
static_assert(has_with<FnOnce<void()>, decltype([](const MoveOnly) {}), MoveOnly>);
static_assert(has_with<FnOnce<void()>, decltype([](const CopyOnly&) {}), CopyOnly>);
static_assert(has_with<FnOnce<void()>, decltype([](CopyOnly&) {}), CopyOnly>);
static_assert(has_with<FnOnce<void()>, decltype([](CopyOnly&&) {}), CopyOnly>);
static_assert(!has_with<FnOnce<void()>, decltype([](CopyOnly) {}), CopyOnly>);
static_assert(!has_with<FnOnce<void()>, decltype([](const CopyOnly) {}), CopyOnly>);
static_assert(has_with<FnOnce<void()>, decltype([](const Copyable&) {}), Copyable>);
static_assert(has_with<FnOnce<void()>, decltype([](Copyable&) {}), Copyable>);
static_assert(has_with<FnOnce<void()>, decltype([](Copyable&&) {}), Copyable>);
static_assert(has_with<FnOnce<void()>, decltype([](Copyable) {}), Copyable>);
static_assert(has_with<FnOnce<void()>, decltype([](const Copyable) {}), Copyable>);
static_assert(has_with<FnOnce<void()>, decltype([](const int&) {}), int>);
static_assert(has_with<FnOnce<void()>, decltype([](int&) {}), int>);
static_assert(has_with<FnOnce<void()>, decltype([](int&&) {}), int>);
static_assert(has_with<FnOnce<void()>, decltype([](int) {}), int>);
static_assert(has_with<FnOnce<void()>, decltype([](const int) {}), int>);
// FnMut passes a mutable reference to its storage to the receiver function. It
// must be able to receive that, which means it must be receiving a const or
// mutable lvalue reference, or it will try produce a copy.
static_assert(has_with<FnMut<void()>, decltype([](const MoveOnly&) {}), MoveOnly>);
static_assert(has_with<FnMut<void()>, decltype([](MoveOnly&) {}), MoveOnly>);
static_assert(!has_with<FnMut<void()>, decltype([](MoveOnly&&) {}), MoveOnly>);
static_assert(!has_with<FnMut<void()>, decltype([](MoveOnly) {}), MoveOnly>);
static_assert(!has_with<FnMut<void()>, decltype([](const MoveOnly) {}), MoveOnly>);
static_assert(has_with<FnMut<void()>, decltype([](const CopyOnly&) {}), CopyOnly>);
static_assert(has_with<FnMut<void()>, decltype([](CopyOnly&) {}), CopyOnly>);
static_assert(!has_with<FnMut<void()>, decltype([](CopyOnly&&) {}), CopyOnly>);
static_assert(has_with<FnMut<void()>, decltype([](CopyOnly) {}), CopyOnly>);
static_assert(has_with<FnMut<void()>, decltype([](const CopyOnly) {}), CopyOnly>);
static_assert(has_with<FnMut<void()>, decltype([](const Copyable&) {}), Copyable>);
static_assert(has_with<FnMut<void()>, decltype([](Copyable&) {}), Copyable>);
static_assert(!has_with<FnMut<void()>, decltype([](Copyable&&) {}), Copyable>);
static_assert(has_with<FnMut<void()>, decltype([](Copyable) {}), Copyable>);
static_assert(has_with<FnMut<void()>, decltype([](const Copyable) {}), Copyable>);
static_assert(has_with<FnMut<void()>, decltype([](const int&) {}), int>);
static_assert(has_with<FnMut<void()>, decltype([](int&) {}), int>);
static_assert(!has_with<FnMut<void()>, decltype([](int&&) {}), int>);
static_assert(has_with<FnMut<void()>, decltype([](int) {}), int>);
static_assert(has_with<FnMut<void()>, decltype([](const int) {}), int>);
// Fn passes a const reference to its storage to the receiver function. It must
// be able to receive that, which means it must be receiving a const reference,
// or it will try produce a copy.
static_assert(has_with<Fn<void()>, decltype([](const MoveOnly&) {}), MoveOnly>);
static_assert(!has_with<Fn<void()>, decltype([](MoveOnly&) {}), MoveOnly>);
static_assert(!has_with<Fn<void()>, decltype([](MoveOnly&&) {}), MoveOnly>);
static_assert(!has_with<Fn<void()>, decltype([](MoveOnly) {}), MoveOnly>);
static_assert(!has_with<Fn<void()>, decltype([](const MoveOnly) {}), MoveOnly>);
static_assert(has_with<Fn<void()>, decltype([](const CopyOnly&) {}), CopyOnly>);
static_assert(!has_with<Fn<void()>, decltype([](CopyOnly&) {}), CopyOnly>);
static_assert(!has_with<Fn<void()>, decltype([](CopyOnly&&) {}), CopyOnly>);
static_assert(has_with<Fn<void()>, decltype([](CopyOnly) {}), CopyOnly>);
static_assert(has_with<Fn<void()>, decltype([](const CopyOnly) {}), CopyOnly>);
static_assert(has_with<Fn<void()>, decltype([](const Copyable&) {}), Copyable>);
static_assert(!has_with<Fn<void()>, decltype([](Copyable&) {}), Copyable>);
static_assert(!has_with<Fn<void()>, decltype([](Copyable&&) {}), Copyable>);
static_assert(has_with<Fn<void()>, decltype([](Copyable) {}), Copyable>);
static_assert(has_with<Fn<void()>, decltype([](const Copyable) {}), Copyable>);
static_assert(has_with<Fn<void()>, decltype([](const int&) {}), int>);
static_assert(!has_with<Fn<void()>, decltype([](int&) {}), int>);
static_assert(!has_with<Fn<void()>, decltype([](int&&) {}), int>);
static_assert(has_with<Fn<void()>, decltype([](int) {}), int>);
static_assert(has_with<Fn<void()>, decltype([](const int) {}), int>);
// The receiver of the args passed to call/call_mut/call_once must be compatible
// with the specified type in the Fn/FnMut/FnOnce.
static_assert(has_with<FnOnce<void(MoveOnly)>, decltype([](MoveOnly) {})>);
static_assert(has_with<FnOnce<void(MoveOnly)>, decltype([](const MoveOnly&) {})>);
static_assert(!has_with<FnOnce<void(MoveOnly)>, decltype([](MoveOnly&) {})>);
static_assert(has_with<FnOnce<void(MoveOnly)>, decltype([](MoveOnly&&) {})>);
static_assert(has_with<FnMut<void(MoveOnly)>, decltype([](MoveOnly) {})>);
static_assert(has_with<FnMut<void(MoveOnly)>, decltype([](const MoveOnly&) {})>);
static_assert(!has_with<FnMut<void(MoveOnly)>, decltype([](MoveOnly&) {})>);
static_assert(has_with<FnMut<void(MoveOnly)>, decltype([](MoveOnly&&) {})>);
static_assert(has_with<Fn<void(MoveOnly)>, decltype([](MoveOnly) {})>);
static_assert(has_with<Fn<void(MoveOnly)>, decltype([](const MoveOnly&) {})>);
static_assert(!has_with<Fn<void(MoveOnly)>, decltype([](MoveOnly&) {})>);
static_assert(has_with<Fn<void(MoveOnly)>, decltype([](MoveOnly&&) {})>);

#endif

// clang-format on

TEST(Fn, Pointer) {
  {
    auto fn = FnOnce<int(int, int)>::with(
        [](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(1, 2), 4);
  }
  {
    auto fn = FnMut<int(int, int)>::with(
        [](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(fn(1, 2), 4);
  }
  {
    auto fn =
        Fn<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(1, 2), 4);
  }
}

TEST(Fn, InlineCapture) {
  {
    auto fn = FnOnce<int(int)>::with(
        sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = FnMut<int(int)>::with(
        sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = Fn<int(int)>::with(
        sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
}

TEST(Fn, OutsideCapture) {
  int a = 1;
  {
    auto fn = FnOnce<int(int)>::with(
        sus_bind(sus_store(a), [a](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = FnMut<int(int)>::with(
        sus_bind(sus_store(a), [a](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = Fn<int(int)>::with(
        sus_bind(sus_store(a), [a](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
}

TEST(Fn, BothCapture) {
  int a = 1;
  {
    auto fn = FnOnce<int()>::with(
        sus_bind(sus_store(a), [a, b = 2]() { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(), 4);
  }
  {
    auto fn = FnMut<int()>::with(
        sus_bind(sus_store(a), [a, b = 2]() { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(), 4);
  }
  {
    auto fn = Fn<int()>::with(
        sus_bind(sus_store(a), [a, b = 2]() { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(), 4);
  }
}

TEST(Fn, CopyFromCapture) {
  auto c = Copyable(1);
  {
    auto fn = FnOnce<int(int)>::with(
        sus_bind(sus_store(c), [c](int b) { return c.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = FnMut<int(int)>::with(
        sus_bind(sus_store(c), [c](int b) { return c.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = Fn<int(int)>::with(
        sus_bind(sus_store(c), [c](int b) { return c.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
}

TEST(Fn, MoveFromCapture) {
  {
    auto m = MoveOnly(1);
    auto fn = FnOnce<int(int)>::with(sus_bind_mut(
        sus_store(sus_take(m)),
        [m = static_cast<MoveOnly&&>(m)](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto m = MoveOnly(1);
    auto fn = FnMut<int(int)>::with(sus_bind_mut(
        sus_store(sus_take(m)),
        [m = static_cast<MoveOnly&&>(m)](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    // The stored value was moved into the run lambda (so `m` holds `-1`).
    EXPECT_EQ(m.i, -1);
    EXPECT_EQ(fn(-2), -4);
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(-2), -4);
  }
  // Can't hold sus_bind_mut() in Fn.
}

TEST(Fn, MoveIntoCapture) {
  {
    auto m = MoveOnly(1);
    auto fn = FnOnce<int(int)>::with(
        sus_bind(sus_store(sus_take(m)), [&m](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto m = MoveOnly(1);
    auto fn = FnMut<int(int)>::with(
        sus_bind(sus_store(sus_take(m)), [&m](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    EXPECT_EQ(fn(2), 4);
  }
  // Can modify the captured m with sus_bind_mut().
  {
    auto m = MoveOnly(1);
    auto fn = FnMut<int(int)>::with(sus_bind_mut(
        sus_store(sus_take(m)), [&m](int b) { return m.i++ * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    EXPECT_EQ(fn(2), 6);
  }
  {
    auto m = MoveOnly(1);
    auto fn = Fn<int(int)>::with(
        sus_bind(sus_store(sus_take(m)), [&m](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    EXPECT_EQ(fn(2), 4);
  }
}

TEST(Fn, MoveFn) {
  {
    auto fn = FnOnce<int(int, int)>::with(
        [](int a, int b) { return a * 2 + b; });
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(1, 2), 4);
  }
  {
    auto fn = FnOnce<int(int)>::with(
        sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(2), 4);
  }
  {
    auto fn = FnMut<int(int, int)>::with(
        [](int a, int b) { return a * 2 + b; });
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(1, 2), 4);
  }
  {
    auto fn = FnMut<int(int)>::with(
        sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(2), 4);
  }
  {
    auto fn =
        Fn<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(1, 2), 4);
  }
  {
    auto fn = Fn<int(int)>::with(
        sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(2), 4);
  }
}

TEST(Fn, FnIsFnMut) {
  {
    auto fn =
        Fn<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    auto mut = FnMut<int(int, int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(mut(1, 2), 4);
  }
  {
    auto fn = Fn<int(int)>::with(
        sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto mut = FnMut<int(int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(mut(2), 4);
  }
}

TEST(Fn, FnIsFnOnce) {
  {
    auto fn =
        Fn<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    auto once = FnOnce<int(int, int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(1, 2), 4);
  }
  {
    auto fn = Fn<int(int)>::with(
        sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto once = FnOnce<int(int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(2), 4);
  }
}

TEST(Fn, FnMutIsFnOnce) {
  {
    auto fn = FnMut<int(int, int)>::with(
        [](int a, int b) { return a * 2 + b; });
    auto once = FnOnce<int(int, int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(1, 2), 4);
  }
  {
    auto fn = FnMut<int(int)>::with(
        sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto once = FnOnce<int(int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(2), 4);
  }
}

TEST(Fn, BindUnsafePointer) {
  int a = 1;
  int* pa = &a;
  int b = 2;
  auto fn = Fn<int()>::with(sus_bind(
      sus_store(sus_unsafe_pointer(pa), b), [pa, b]() { return *pa * 2 + b; }));
  EXPECT_EQ(fn(), 4);
}

}  // namespace
