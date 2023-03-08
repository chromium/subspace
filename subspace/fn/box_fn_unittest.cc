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

#include "subspace/fn/fn.h"

#include <concepts>

#include "googletest/include/gtest/gtest.h"
#include "subspace/construct/into.h"
#include "subspace/mem/forward.h"
#include "subspace/mem/move.h"
#include "subspace/mem/replace.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

namespace {

using sus::construct::Into;
using sus::fn::BoxFn;
using sus::fn::BoxFnMut;
using sus::fn::BoxFnOnce;

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

// clang-format-off

struct BaseClass {};
struct SubClass : public BaseClass {};

static_assert(sizeof(BoxFnOnce<void()>) > sizeof(void (*)()));
static_assert(sizeof(BoxFnOnce<void()>) <= sizeof(void (*)()) * 2);

void v_v_function() {}
int i_f_function(float) { return 0; }
void v_f_function(float) {}
BaseClass* b_b_function(BaseClass* b) { return b; }
SubClass* s_b_function(BaseClass* b) { return static_cast<SubClass*>(b); }
SubClass* s_s_function(SubClass* b) { return b; }

// These emulate binding with sus_bind(), but don't use it cuz capturing lambdas in a constant
// expression won't work.
auto b_b_lambda = sus::fn::__private::SusBind([a = 1](BaseClass* b) -> BaseClass* { return b; });
auto s_b_lambda = sus::fn::__private::SusBind([a = 1](BaseClass* b) -> SubClass* { return static_cast<SubClass*>(b); });
auto s_s_lambda = sus::fn::__private::SusBind([a = 1](SubClass* b) -> SubClass* { return b; });

// BoxFn types all have a never-value field.
static_assert(sus::mem::NeverValueField<BoxFnOnce<void()>>);
static_assert(sus::mem::NeverValueField<BoxFnMut<void()>>);
static_assert(sus::mem::NeverValueField<BoxFn<void()>>);
// Which allows them to not require a flag in Option.
static_assert(sizeof(sus::Option<BoxFnOnce<void()>>) == sizeof(BoxFnOnce<void()>));

// Closures can not be copied, as their storage is uniquely owned.
static_assert(!std::is_copy_constructible_v<BoxFnOnce<void()>>);
static_assert(!std::is_copy_assignable_v<BoxFnOnce<void()>>);
static_assert(!std::is_copy_constructible_v<BoxFnMut<void()>>);
static_assert(!std::is_copy_assignable_v<BoxFnMut<void()>>);
static_assert(!std::is_copy_constructible_v<BoxFn<void()>>);
static_assert(!std::is_copy_assignable_v<BoxFn<void()>>);
// Closures can be moved.
static_assert(std::is_move_constructible_v<BoxFnOnce<void()>>);
static_assert(std::is_move_assignable_v<BoxFnOnce<void()>>);
static_assert(std::is_move_constructible_v<BoxFnMut<void()>>);
static_assert(std::is_move_assignable_v<BoxFnMut<void()>>);
static_assert(std::is_move_constructible_v<BoxFn<void()>>);
static_assert(std::is_move_assignable_v<BoxFn<void()>>);

// Closures are trivially relocatable.
static_assert(sus::mem::relocate_by_memcpy<BoxFnOnce<void()>>);
static_assert(sus::mem::relocate_by_memcpy<BoxFnMut<void()>>);
static_assert(sus::mem::relocate_by_memcpy<BoxFn<void()>>);

// A function pointer, or convertible lambda, can be bound to BoxFnOnce, BoxFnMut and BoxFn.
static_assert(std::is_constructible_v<BoxFnOnce<void()>, decltype([](){})>);
static_assert(std::is_constructible_v<BoxFnMut<void()>, decltype([](){})>);
static_assert(std::is_constructible_v<BoxFn<void()>, decltype([](){})>);
static_assert(std::is_constructible_v<BoxFnOnce<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<BoxFnMut<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<BoxFn<void()>, decltype(v_v_function)>);
// Non-void types for the same.
static_assert(std::is_constructible_v<BoxFnOnce<int(float)>, decltype([](float){return 1;})>);
static_assert(std::is_constructible_v<BoxFnMut<int(float)>, decltype([](float){return 1;})>);
static_assert(std::is_constructible_v<BoxFn<int(float)>, decltype([](float){return 1;})>);
static_assert(std::is_constructible_v<BoxFnOnce<int(float)>, decltype(i_f_function)>);
static_assert(std::is_constructible_v<BoxFnMut<int(float)>, decltype(i_f_function)>);
static_assert(std::is_constructible_v<BoxFn<int(float)>, decltype(i_f_function)>);

// Lambdas with bound args can't be passed without sus_bind().
static_assert(!std::is_constructible_v<BoxFnOnce<void()>, decltype([i = int(1)](){ (void)i; })>);
static_assert(!std::is_constructible_v<BoxFnOnce<void()>, decltype([i = int(1)]() mutable { ++i; })>);

// The return type of the BoxFn must match that of the lambda. It will not allow
// converting to void.
static_assert(std::is_constructible_v<BoxFn<SubClass*(BaseClass*)>, decltype(s_b_function)>);
static_assert(!std::is_constructible_v<BoxFn<void(BaseClass*)>, decltype(b_b_function)>);
static_assert(!std::is_constructible_v<BoxFn<SubClass*(BaseClass*)>, decltype(b_b_function)>);
static_assert(std::is_constructible_v<BoxFn<SubClass*(BaseClass*)>, decltype(s_b_lambda)>);
static_assert(!std::is_constructible_v<BoxFn<void(BaseClass*)>, decltype(b_b_lambda)>);
static_assert(!std::is_constructible_v<BoxFn<SubClass*(BaseClass*)>, decltype(b_b_lambda)>);
// Similarly, argument types can't be converted to a different type.
static_assert(std::is_constructible_v<BoxFn<SubClass*(SubClass*)>, decltype(s_s_function)>);
static_assert(!std::is_constructible_v<BoxFn<SubClass*(BaseClass*)>, decltype(s_s_function)>);
static_assert(std::is_constructible_v<BoxFn<SubClass*(SubClass*)>, decltype(s_s_lambda)>);
static_assert(!std::is_constructible_v<BoxFn<SubClass*(BaseClass*)>, decltype(s_s_lambda)>);
// But BoxFn type is compatible with convertible return and argument types in
// opposite directions.
// - If the return type Y of a lambda is convertible _to_ X, then BoxFn<X()> can be
//   used to store it.
// - If the argument type Y of a lambda is convertible _from_ X, then BoxFn<(X)>
//   can be used to store it.
//
// In both cases, the BoxFn is more strict than the lambda, guaranteeing that the
// lambda's requirements are met.
static_assert(std::is_constructible_v<BoxFn<BaseClass*(BaseClass*)>, decltype(s_b_lambda)>);
static_assert(std::is_constructible_v<BoxFn<SubClass*(SubClass*)>, decltype(s_b_lambda)>);
// HOWEVER: When BoxFn is passed a function pointer, it stores a function pointer. C++20 does not yet
// allow us to erase the type of that function pointer in a constexpr context. So the types in
// the pointer must match exactly to the BoxFn's signature.
static_assert(!std::is_constructible_v<BoxFn<BaseClass*(BaseClass*)>, decltype(s_b_function)>);
static_assert(!std::is_constructible_v<BoxFn<SubClass*(SubClass*)>, decltype(s_b_function)>);

// Lambdas with bound args can be passed with sus_bind(). Can use sus_bind0()
// when there's no captured variables.
static_assert(std::is_constructible_v<BoxFnOnce<void()>, decltype([]() { return sus_bind0([i = int(1)](){}); }())>);
// And use sus_bind0_mut for a mutable lambda.
static_assert(std::is_constructible_v<BoxFnOnce<void()>, decltype([]() { return sus_bind0_mut([i = int(1)]() mutable {++i;}); }())>);

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
//     BoxFnOnce<void()>,
//     decltype([]() {
//       return sus_bind0([i = int(1)]() mutable {++i;});
//     }())
// >);
// #pragma GCC diagnostic pop

template <class R, class Param, class Arg>
concept can_run = requires (
  Arg arg, BoxFnOnce<R(Param)> fnonce, BoxFnMut<R(Param)> fnmut, BoxFn<R(Param)> fn
) {
  { static_cast<BoxFnOnce<R(Param)>&&>(fnonce)(sus::forward<Arg>(arg)) };
  { static_cast<BoxFnOnce<R(Param)>&&>(fnmut)(sus::forward<Arg>(arg)) };
  { static_cast<BoxFnOnce<R(Param)>&&>(fn)(sus::forward<Arg>(arg)) };
};
// clang-format on

// Int is copyable, so references are copied when passed.
static_assert(can_run<void, int, int>);
static_assert(can_run<void, int, int&&>);
static_assert(can_run<void, int, int&>);
static_assert(can_run<void, int, const int>);
static_assert(can_run<void, int, const int&>);
static_assert(can_run<void, int, const int&&>);
// But for a move-only type, it can only be passed along as a reference or an
// rvalue.
static_assert(can_run<void, MoveOnly, MoveOnly>);
static_assert(can_run<void, MoveOnly, MoveOnly&&>);
static_assert(!can_run<void, MoveOnly, MoveOnly&>);
static_assert(!can_run<void, MoveOnly, const MoveOnly>);
static_assert(!can_run<void, MoveOnly, const MoveOnly&>);
static_assert(!can_run<void, MoveOnly, const MoveOnly&&>);
static_assert(can_run<void, const MoveOnly&, MoveOnly&>);
static_assert(can_run<void, const MoveOnly&, const MoveOnly&>);
static_assert(can_run<void, MoveOnly&, MoveOnly&>);
static_assert(!can_run<void, MoveOnly&, const MoveOnly&>);

// Receiving a mutable reference means it must be passed as a mutable reference.
static_assert(can_run<void, int&, int&>);
static_assert(!can_run<void, int&, const int&>);
static_assert(!can_run<void, int&, int>);

// Receiving a const reference means it must be passed as a reference.
static_assert(can_run<void, const int&, int&>);
static_assert(can_run<void, const int&, const int&>);
static_assert(!can_run<void, int&, int>);
static_assert(!can_run<void, int&, const int>);
static_assert(!can_run<void, int&, int&&>);
static_assert(!can_run<void, int&, const int&&>);

TEST(BoxFn, Pointer) {
  {
    auto fn = BoxFnOnce<int(int, int)>([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(sus::move(fn)(1, 2), 4);
  }
  {
    auto fn = BoxFnMut<int(int, int)>([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(fn(1, 2), 4);
  }
  {
    auto fn = BoxFn<int(int, int)>([](int a, int b) { return a * 2 + b; });
    EXPECT_EQ(sus::move(fn)(1, 2), 4);
  }
}

TEST(BoxFn, InlineCapture) {
  {
    auto fn = BoxFnOnce<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
  {
    auto fn = BoxFnMut<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
  {
    auto fn = BoxFn<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
}

TEST(BoxFn, OutsideCapture) {
  int a = 1;
  {
    auto fn = BoxFnOnce<int(int)>(
        sus_bind(sus_store(a), [a](int b) { return a * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
  {
    auto fn = BoxFnMut<int(int)>(
        sus_bind(sus_store(a), [a](int b) { return a * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
  {
    auto fn =
        BoxFn<int(int)>(sus_bind(sus_store(a), [a](int b) { return a * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
}

TEST(BoxFn, BothCapture) {
  int a = 1;
  {
    auto fn = BoxFnOnce<int()>(
        sus_bind(sus_store(a), [a, b = 2]() { return a * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(), 4);
  }
  {
    auto fn = BoxFnMut<int()>(
        sus_bind(sus_store(a), [a, b = 2]() { return a * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(), 4);
  }
  {
    auto fn =
        BoxFn<int()>(sus_bind(sus_store(a), [a, b = 2]() { return a * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(), 4);
  }
}

TEST(BoxFn, CopyFromCapture) {
  auto c = Copyable(1);
  {
    auto fn = BoxFnOnce<int(int)>(
        sus_bind(sus_store(c), [c](int b) { return c.i * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
  {
    auto fn = BoxFnMut<int(int)>(
        sus_bind(sus_store(c), [c](int b) { return c.i * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
  {
    auto fn = BoxFn<int(int)>(
        sus_bind(sus_store(c), [c](int b) { return c.i * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
}

TEST(BoxFn, MoveFromCapture) {
  {
    auto m = MoveOnly(1);
    auto fn = BoxFnOnce<int(int)>(sus_bind_mut(
        sus_store(sus_take(m)),
        [m = static_cast<MoveOnly&&>(m)](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
  {
    auto m = MoveOnly(1);
    auto fn = BoxFnMut<int(int)>(sus_bind_mut(
        sus_store(sus_take(m)),
        [m = static_cast<MoveOnly&&>(m)](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    // The stored value was moved into the run lambda (so `m` holds `-1`).
    EXPECT_EQ(m.i, -1);
    EXPECT_EQ(fn(-2), -4);
    EXPECT_EQ(sus::move(fn)(-2), -4);
  }
  // Can't hold sus_bind_mut() in BoxFn.
}

TEST(BoxFn, MoveIntoCapture) {
  {
    auto m = MoveOnly(1);
    auto fn = BoxFnOnce<int(int)>(
        sus_bind(sus_store(sus_take(m)), [&m](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(sus::move(fn)(2), 4);
  }
  {
    auto m = MoveOnly(1);
    auto fn = BoxFnMut<int(int)>(
        sus_bind(sus_store(sus_take(m)), [&m](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    EXPECT_EQ(fn(2), 4);
  }
  // Can modify the captured m with sus_bind_mut().
  {
    auto m = MoveOnly(1);
    auto fn = BoxFnMut<int(int)>(sus_bind_mut(
        sus_store(sus_take(m)), [&m](int b) { return m.i++ * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    EXPECT_EQ(fn(2), 6);
  }
  {
    auto m = MoveOnly(1);
    auto fn = BoxFn<int(int)>(
        sus_bind(sus_store(sus_take(m)), [&m](int b) { return m.i * 2 + b; }));
    EXPECT_EQ(fn(2), 4);
    EXPECT_EQ(fn(2), 4);
  }
}

TEST(BoxFn, MoveBoxFn) {
  {
    auto fn = BoxFnOnce<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto fn2 = sus::move(fn);
    EXPECT_EQ(sus::move(fn2)(1, 2), 4);
  }
  {
    auto fn = BoxFnOnce<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = sus::move(fn);
    EXPECT_EQ(sus::move(fn2)(2), 4);
  }
  {
    auto fn = BoxFnMut<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto fn2 = sus::move(fn);
    EXPECT_EQ(sus::move(fn2)(1, 2), 4);
  }
  {
    auto fn = BoxFnMut<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = sus::move(fn);
    EXPECT_EQ(sus::move(fn2)(2), 4);
  }
  {
    auto fn = BoxFn<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto fn2 = sus::move(fn);
    EXPECT_EQ(sus::move(fn2)(1, 2), 4);
  }
  {
    auto fn = BoxFn<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto fn2 = sus::move(fn);
    EXPECT_EQ(sus::move(fn2)(2), 4);
  }
}

TEST(BoxFn, BoxFnIsBoxFnMut) {
  {
    auto fn = BoxFn<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto mut = BoxFnMut<int(int, int)>(sus::move(fn));
    EXPECT_EQ(mut(1, 2), 4);
  }
  {
    auto fn = BoxFn<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto mut = BoxFnMut<int(int)>(sus::move(fn));
    EXPECT_EQ(mut(2), 4);
  }
}

TEST(BoxFn, BoxFnIsBoxFnOnce) {
  {
    auto fn = BoxFn<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto once = BoxFnOnce<int(int, int)>(sus::move(fn));
    EXPECT_EQ(sus::move(once)(1, 2), 4);
  }
  {
    auto fn = BoxFn<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto once = BoxFnOnce<int(int)>(sus::move(fn));
    EXPECT_EQ(sus::move(once)(2), 4);
  }
}

TEST(BoxFn, BoxFnMutIsBoxFnOnce) {
  {
    auto fn = BoxFnMut<int(int, int)>([](int a, int b) { return a * 2 + b; });
    auto once = BoxFnOnce<int(int, int)>(sus::move(fn));
    EXPECT_EQ(sus::move(once)(1, 2), 4);
  }
  {
    auto fn = BoxFnMut<int(int)>(sus_bind0([a = 1](int b) { return a * 2 + b; }));
    auto once = BoxFnOnce<int(int)>(sus::move(fn));
    EXPECT_EQ(sus::move(once)(2), 4);
  }
}

TEST(BoxFn, BindUnsafePointer) {
  int a = 1;
  int* pa = &a;
  int b = 2;
  auto fn = BoxFn<int()>(sus_bind(sus_store(sus_unsafe_pointer(pa), b), [pa, b]() {
    // sus_bind() will store pointers as const.
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*pa)>>);
    return *pa * 2 + b;
  }));
  EXPECT_EQ(fn(), 4);

  auto fnmut = BoxFnMut<int()>(
      sus_bind_mut(sus_store(sus_unsafe_pointer(pa), b), [pa, b]() {
        // sus_bind_mut() will store pointers as mutable.
        static_assert(!std::is_const_v<std::remove_reference_t<decltype(*pa)>>);
        return (*pa)++ * 2 + b;
      }));
  EXPECT_EQ(fnmut(), 4);
}

TEST(BoxFn, Into) {
  auto into_fnonce = []<Into<BoxFnOnce<int(int)>> F>(F into_f) {
    BoxFnOnce<int(int)> f = sus::move_into(into_f);
    return sus::move(f)(1);
  };
  EXPECT_EQ(into_fnonce([](int i) { return i + 1; }), 2);
  EXPECT_EQ(into_fnonce(sus_bind0([](int i) { return i + 1; })), 2);

  auto into_fnmut = []<Into<BoxFnMut<int(int)>> F>(F into_f) {
    return BoxFnMut<int(int)>::from(::sus::move(into_f))(1);
  };
  EXPECT_EQ(into_fnmut([](int i) { return i + 1; }), 2);
  EXPECT_EQ(into_fnmut(sus_bind0([](int i) { return i + 1; })), 2);

  auto into_fn = []<Into<BoxFn<int(int)>> F>(F into_f) {
    BoxFn<int(int)> f = sus::move_into(into_f);
    return sus::move(f)(1);
  };
  EXPECT_EQ(into_fn([](int i) { return i + 1; }), 2);
  EXPECT_EQ(into_fn(sus_bind0([](int i) { return i + 1; })), 2);
}

TEST(BoxFnDeathTest, NullPointer) {
  void (*f)() = nullptr;
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(BoxFnOnce<void()>::from(f), "");
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(BoxFnMut<void()>::from(f), "");
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(BoxFn<void()>::from(f), "");
#endif
}

TEST(BoxFnDeathTest, CallAfterMoveConstruct) {
  {
    auto x = BoxFnOnce<void()>::from([]() {});
    [[maybe_unused]] auto y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
  {
    auto x = BoxFnMut<void()>::from([]() {});
    [[maybe_unused]] auto y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = BoxFn<void()>::from([]() {});
    [[maybe_unused]] auto y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = BoxFnOnce<void()>::from(sus_bind0([]() {}));
    [[maybe_unused]] auto y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
  {
    auto x = BoxFnMut<void()>::from(sus_bind0([]() {}));
    [[maybe_unused]] auto y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = BoxFn<void()>::from(sus_bind0([]() {}));
    [[maybe_unused]] auto y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
}

TEST(BoxFnDeathTest, CallAfterMoveAssign) {
  {
    auto x = BoxFnOnce<void()>::from([]() {});
    auto y = BoxFnOnce<void()>::from([]() {});
    y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
  {
    auto x = BoxFnMut<void()>::from([]() {});
    auto y = BoxFnOnce<void()>::from([]() {});
    y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = BoxFn<void()>::from([]() {});
    auto y = BoxFnOnce<void()>::from([]() {});
    y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = BoxFnOnce<void()>::from(sus_bind0([]() {}));
    auto y = BoxFnOnce<void()>::from([]() {});
    y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
  {
    auto x = BoxFnMut<void()>::from(sus_bind0([]() {}));
    auto y = BoxFnOnce<void()>::from([]() {});
    y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
  {
    auto x = BoxFn<void()>::from(sus_bind0([]() {}));
    auto y = BoxFnOnce<void()>::from([]() {});
    y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(x(), "");
#endif
  }
}

TEST(BoxFnDeathTest, CallAfterCall) {
  {
    auto x = BoxFnOnce<void()>::from([]() {});
    sus::move(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
  {
    auto x = BoxFnMut<void()>::from([]() {});
    sus::move(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
  {
    auto x = BoxFn<void()>::from([]() {});
    sus::move(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
  {
    auto x = BoxFnOnce<void()>::from(sus_bind0([]() {}));
    sus::move(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
  {
    auto x = BoxFnMut<void()>::from(sus_bind0([]() {}));
    sus::move(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
  {
    auto x = BoxFn<void()>::from(sus_bind0([]() {}));
    sus::move(x)();
#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(sus::move(x)(), "");
#endif
  }
}

}  // namespace
