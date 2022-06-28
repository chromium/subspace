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

static_assert(sizeof(FnOnce<void()>) > sizeof(void (*)()));
static_assert(sizeof(FnOnce<void()>) <= sizeof(void (*)()) * 2);

template <class FnType, class F>
concept has_with = requires(F&& f) {
                     { FnType::with(sus::forward<F>(f)) };
                   };

void v_v_function() {}
int i_f_function(float) { return 0; }

// clang-format off

// A function pointer, or convertible lambda, can be bound to FnOnce, FnMut and Fn.
static_assert(has_with<FnOnce<void()>, decltype([](){})>);
static_assert(has_with<FnMut<void()>, decltype([](){})>);
static_assert(has_with<Fn<void()>, decltype([](){})>);
static_assert(has_with<FnOnce<void()>, decltype(v_v_function)>);
static_assert(has_with<FnMut<void()>, decltype(v_v_function)>);
static_assert(has_with<Fn<void()>, decltype(v_v_function)>);
// Non-void types for the same.
static_assert(has_with<FnOnce<int(float)>, decltype([](float){return 1;})>);
static_assert(has_with<FnMut<int(float)>, decltype([](float){return 1;})>);
static_assert(has_with<Fn<int(float)>, decltype([](float){return 1;})>);
static_assert(has_with<FnOnce<int(float)>, decltype(i_f_function)>);
static_assert(has_with<FnMut<int(float)>, decltype(i_f_function)>);
static_assert(has_with<Fn<int(float)>, decltype(i_f_function)>);

// Lambdas with bound args can't be passed without sus_bind().
static_assert(!has_with<FnOnce<void()>, decltype([i = int(1)](){})>);
static_assert(!has_with<FnOnce<void()>, decltype([i = int(1)]() mutable {++i;})>);

// Lambdas with bound args can be passed with sus_bind(). Can use sus_bind0()
// when there's no captured variables.
static_assert(has_with<FnOnce<void()>, decltype(sus_bind0([i = int(1)](){}))>);
// And use sus_bind0_mut for a mutable lambda.
static_assert(has_with<FnOnce<void()>, decltype(sus_bind0_mut([i = int(1)]() mutable {++i;}))>);
// This incorrectly uses sus_bind0 with a mutable lambda, which produces a warning/error.
#pragma warning(suppress: 4996)
static_assert(!has_with<FnOnce<void()>, decltype(sus_bind0([i = int(1)]() mutable {++i;}))>);

// clang-format on

TEST(Fn, Pointer) {
  {
    auto fn =
        FnOnce<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(1, 2), 4);
  }
  {
    auto fn =
        FnMut<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(fn(1, 2), 4);
  }
  {
    auto fn = Fn<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(1, 2), 4);
  }
}

TEST(Fn, InlineCapture) {
  static_assert(::sus::concepts::callable::LambdaConst<decltype([a = 1](int b) {
    return a * 2 + b;
  })>);
  {
    auto fn =
        FnOnce<int(int)>::with(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn =
        FnMut<int(int)>::with(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn =
        Fn<int(int)>::with(sus_bind0([a = 1](int b) { return a * 2 + b; }));
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
    auto fn =
        FnOnce<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(1, 2), 4);
  }
  {
    auto fn =
        FnOnce<int(int)>::with(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(2), 4);
  }
  {
    auto fn =
        FnMut<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(1, 2), 4);
  }
  {
    auto fn =
        FnMut<int(int)>::with(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(2), 4);
  }
  {
    auto fn = Fn<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(1, 2), 4);
  }
  {
    auto fn =
        Fn<int(int)>::with(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(2), 4);
  }
}

TEST(Fn, FnIsFnMut) {
  {
    auto fn = Fn<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    auto mut = FnMut<int(int, int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(mut(1, 2), 4);
  }
  {
    auto fn =
        Fn<int(int)>::with(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto mut = FnMut<int(int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(mut(2), 4);
  }
}

TEST(Fn, FnIsFnOnce) {
  {
    auto fn = Fn<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    auto once = FnOnce<int(int, int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(1, 2), 4);
  }
  {
    auto fn =
        Fn<int(int)>::with(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto once = FnOnce<int(int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(2), 4);
  }
}

TEST(Fn, FnMutIsFnOnce) {
  {
    auto fn =
        FnMut<int(int, int)>::with([](int a, int b) { return a * 2 + b; });
    auto once = FnOnce<int(int, int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(1, 2), 4);
  }
  {
    auto fn =
        FnMut<int(int)>::with(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto once = FnOnce<int(int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(2), 4);
  }
}

TEST(Fn, BindUnsafePointer) {
  int a = 1;
  int* pa = &a;
  int b = 2;
  auto fn = Fn<int()>::with(sus_bind(sus_store(sus_unsafe_pointer(pa), b),
                                     [pa, b]() { return *pa * 2 + b; }));
  EXPECT_EQ(fn(), 4);
}

}  // namespace
