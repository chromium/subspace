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

#include "concepts/into.h"
#include "mem/forward.h"
#include "mem/replace.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::concepts::into::Into;
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

void v_v_function() {}
int i_f_function(float) { return 0; }

// clang-format off

// Closures can not be copied, as their storage is uniquely owned.
static_assert(!std::is_copy_constructible_v<FnOnce<void()>>);
static_assert(!std::is_copy_assignable_v<FnOnce<void()>>);
static_assert(!std::is_copy_constructible_v<FnMut<void()>>);
static_assert(!std::is_copy_assignable_v<FnMut<void()>>);
static_assert(!std::is_copy_constructible_v<Fn<void()>>);
static_assert(!std::is_copy_assignable_v<Fn<void()>>);
// Closures can be moved.
static_assert(std::is_move_constructible_v<FnOnce<void()>>);
static_assert(std::is_move_assignable_v<FnOnce<void()>>);
static_assert(std::is_move_constructible_v<FnMut<void()>>);
static_assert(std::is_move_assignable_v<FnMut<void()>>);
static_assert(std::is_move_constructible_v<Fn<void()>>);
static_assert(std::is_move_assignable_v<Fn<void()>>);

// A function pointer, or convertible lambda, can be bound to FnOnce, FnMut and Fn.
static_assert(std::is_constructible_v<FnOnce<void()>, decltype([](){})>);
static_assert(std::is_constructible_v<FnMut<void()>, decltype([](){})>);
static_assert(std::is_constructible_v<Fn<void()>, decltype([](){})>);
static_assert(std::is_constructible_v<FnOnce<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<FnMut<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<Fn<void()>, decltype(v_v_function)>);
// Non-void types for the same.
static_assert(std::is_constructible_v<FnOnce<int(float)>, decltype([](float){return 1;})>);
static_assert(std::is_constructible_v<FnMut<int(float)>, decltype([](float){return 1;})>);
static_assert(std::is_constructible_v<Fn<int(float)>, decltype([](float){return 1;})>);
static_assert(std::is_constructible_v<FnOnce<int(float)>, decltype(i_f_function)>);
static_assert(std::is_constructible_v<FnMut<int(float)>, decltype(i_f_function)>);
static_assert(std::is_constructible_v<Fn<int(float)>, decltype(i_f_function)>);

// Lambdas with bound args can't be passed without sus_bind().
static_assert(!std::is_constructible_v<FnOnce<void()>, decltype([i = int(1)](){})>);
static_assert(!std::is_constructible_v<FnOnce<void()>, decltype([i = int(1)]() mutable {++i;})>);

// Lambdas with bound args can be passed with sus_bind(). Can use sus_bind0()
// when there's no captured variables.
static_assert(std::is_constructible_v<FnOnce<void()>, decltype([]() { return sus_bind0([i = int(1)](){}); }())>);
// And use sus_bind0_mut for a mutable lambda.
static_assert(std::is_constructible_v<FnOnce<void()>, decltype([]() { return sus_bind0_mut([i = int(1)]() mutable {++i;}); }())>);

// This incorrectly uses sus_bind0 with a mutable lambda, which produces a
// warning while also being non-constructible. But we build with errors enabled
// so it fails to compile instead of just failing the assert.
//
// TODO: It doesn't seem possible to disable this error selectively here, as the
// actual call happens in a template elsewhere. So that prevents us from
// checking the non-constructibility if the warning isn't compiled as an error.
//
// It would be nice to be able to produce a nice warning while also failing
// overload resolution nicely. But with -Werror the warning becomes an error,
// and we can't explain why overload resolution is failing with extra
// information in that error message.
//
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
// static_assert(!std::is_constructible_v<
//     FnOnce<void()>,
//     decltype([]() {
//       return sus_bind0([i = int(1)]() mutable {++i;});
//     }())
// >);
// #pragma GCC diagnostic pop

// clang-format on

TEST(Fn, Pointer) {
  {
    auto fn = FnOnce<int(int, int)>([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(1, 2), 4);
  }
  {
    auto fn = FnMut<int(int, int)>([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(fn(1, 2), 4);
  }
  {
    auto fn = Fn<int(int, int)>([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(1, 2), 4);
  }
}

TEST(Fn, InlineCapture) {
  {
    auto fn = FnOnce<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = FnMut<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = Fn<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
}

TEST(Fn, OutsideCapture) {
  int a = 1;
  {
    auto fn = FnOnce<int(int)>(
        sus_bind(sus_store(a), [a](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = FnMut<int(int)>(
        sus_bind(sus_store(a), [a](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn =
        Fn<int(int)>(sus_bind(sus_store(a), [a](int b) { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
}

TEST(Fn, BothCapture) {
  int a = 1;
  {
    auto fn = FnOnce<int()>(
        sus_bind(sus_store(a), [a, b = 2]() { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(), 4);
  }
  {
    auto fn = FnMut<int()>(
        sus_bind(sus_store(a), [a, b = 2]() { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(), 4);
  }
  {
    auto fn =
        Fn<int()>(sus_bind(sus_store(a), [a, b = 2]() { return a * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(), 4);
  }
}

TEST(Fn, CopyFromCapture) {
  auto c = Copyable(1);
  {
    auto fn = FnOnce<int(int)>(
        sus_bind(sus_store(c), [c](int b) { return c.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = FnMut<int(int)>(
        sus_bind(sus_store(c), [c](int b) { return c.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto fn = Fn<int(int)>(
        sus_bind(sus_store(c), [c](int b) { return c.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
}

TEST(Fn, MoveFromCapture) {
  {
    auto m = MoveOnly(1);
    auto fn = FnOnce<int(int)>(sus_bind_mut(
        sus_store(sus_take(m)),
        [m = static_cast<MoveOnly&&>(m)](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto m = MoveOnly(1);
    auto fn = FnMut<int(int)>(sus_bind_mut(
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
    auto fn = FnOnce<int(int)>(
        sus_bind(sus_store(sus_take(m)), [&m](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn)(2), 4);
  }
  {
    auto m = MoveOnly(1);
    auto fn = FnMut<int(int)>(
        sus_bind(sus_store(sus_take(m)), [&m](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    EXPECT_EQ(fn(2), 4);
  }
  // Can modify the captured m with sus_bind_mut().
  {
    auto m = MoveOnly(1);
    auto fn = FnMut<int(int)>(sus_bind_mut(
        sus_store(sus_take(m)), [&m](int b) { return m.i++ * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    EXPECT_EQ(fn(2), 6);
  }
  {
    auto m = MoveOnly(1);
    auto fn = Fn<int(int)>(
        sus_bind(sus_store(sus_take(m)), [&m](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    EXPECT_EQ(fn(2), 4);
  }
}

TEST(Fn, MoveFn) {
  {
    auto fn = FnOnce<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(1, 2), 4);
  }
  {
    auto fn = FnOnce<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(2), 4);
  }
  {
    auto fn = FnMut<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(1, 2), 4);
  }
  {
    auto fn = FnMut<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(2), 4);
  }
  {
    auto fn = Fn<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(1, 2), 4);
  }
  {
    auto fn = Fn<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2)(2), 4);
  }
}

TEST(Fn, FnIsFnMut) {
  {
    auto fn = Fn<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto mut = FnMut<int(int, int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(mut(1, 2), 4);
  }
  {
    auto fn = Fn<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto mut = FnMut<int(int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(mut(2), 4);
  }
}

TEST(Fn, FnIsFnOnce) {
  {
    auto fn = Fn<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto once = FnOnce<int(int, int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(1, 2), 4);
  }
  {
    auto fn = Fn<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto once = FnOnce<int(int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(2), 4);
  }
}

TEST(Fn, FnMutIsFnOnce) {
  {
    auto fn = FnMut<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto once = FnOnce<int(int, int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(1, 2), 4);
  }
  {
    auto fn = FnMut<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto once = FnOnce<int(int)>(static_cast<decltype(fn)&&>(fn));
    EXPECT_EQ(static_cast<decltype(once)&&>(once)(2), 4);
  }
}

TEST(Fn, BindUnsafePointer) {
  int a = 1;
  int* pa = &a;
  int b = 2;
  auto fn = Fn<int()>(sus_bind(sus_store(sus_unsafe_pointer(pa), b),
                               [pa, b]() { return *pa * 2 + b; }));
  EXPECT_EQ(fn(), 4);
}

TEST(Fn, Into) {
  auto into_fnonce = []<Into<FnOnce<int(int)>> F>(F into_f) {
    FnOnce<int(int)> f = sus::move_into(into_f);
    return static_cast<decltype(f)&&>(f)(1);
  };
  EXPECT_EQ(into_fnonce([](int i) { return i + 1; }), 2);
  EXPECT_EQ(into_fnonce(sus_bind0([](int i) { return i + 1; })), 2);

  auto into_fnmut = []<Into<FnMut<int(int)>> F>(F into_f) {
    return FnMut<int(int)>::from(static_cast<F&&>(into_f))(1);
  };
  EXPECT_EQ(into_fnmut([](int i) { return i + 1; }), 2);
  EXPECT_EQ(into_fnmut(sus_bind0([](int i) { return i + 1; })), 2);

  auto into_fn = []<Into<Fn<int(int)>> F>(F into_f) {
    Fn<int(int)> f = sus::move_into(into_f);
    return static_cast<decltype(f)&&>(f)(1);
  };
  EXPECT_EQ(into_fn([](int i) { return i + 1; }), 2);
  EXPECT_EQ(into_fn(sus_bind0([](int i) { return i + 1; })), 2);
}

TEST(FnDeathTest, CallAfterMoveConstruct) {
  {
    auto x = FnOnce<void()>::from([]() {});
    [[maybe_unused]] auto y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
  {
    auto x = FnMut<void()>::from([]() {});
    [[maybe_unused]] auto y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = Fn<void()>::from([]() {});
    [[maybe_unused]] auto y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = FnOnce<void()>::from(sus_bind0([]() {}));
    [[maybe_unused]] auto y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
  {
    auto x = FnMut<void()>::from(sus_bind0([]() {}));
    [[maybe_unused]] auto y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = Fn<void()>::from(sus_bind0([]() {}));
    [[maybe_unused]] auto y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
}

TEST(FnDeathTest, CallAfterMoveAssign) {
  {
    auto x = FnOnce<void()>::from([]() {});
    auto y = FnOnce<void()>::from([]() {});
    y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
  {
    auto x = FnMut<void()>::from([]() {});
    auto y = FnOnce<void()>::from([]() {});
    y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = Fn<void()>::from([]() {});
    auto y = FnOnce<void()>::from([]() {});
    y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = FnOnce<void()>::from(sus_bind0([]() {}));
    auto y = FnOnce<void()>::from([]() {});
    y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
  {
    auto x = FnMut<void()>::from(sus_bind0([]() {}));
    auto y = FnOnce<void()>::from([]() {});
    y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = Fn<void()>::from(sus_bind0([]() {}));
    auto y = FnOnce<void()>::from([]() {});
    y = static_cast<decltype(x)&&>(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
}

TEST(FnDeathTest, CallAfterCall) {
  {
    auto x = FnOnce<void()>::from([]() {});
    static_cast<decltype(x)&&>(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
  {
    auto x = FnMut<void()>::from([]() {});
    static_cast<decltype(x)&&>(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
  {
    auto x = Fn<void()>::from([]() {});
    static_cast<decltype(x)&&>(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
  {
    auto x = FnOnce<void()>::from(sus_bind0([]() {}));
    static_cast<decltype(x)&&>(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
  {
    auto x = FnMut<void()>::from(sus_bind0([]() {}));
    static_cast<decltype(x)&&>(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
  {
    auto x = Fn<void()>::from(sus_bind0([]() {}));
    static_cast<decltype(x)&&>(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(static_cast<decltype(x)&&>(x)(), "");
#endif
  }
}

}  // namespace
