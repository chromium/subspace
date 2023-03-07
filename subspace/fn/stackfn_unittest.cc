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

#include <concepts>

#include "googletest/include/gtest/gtest.h"
#include "subspace/construct/into.h"
#include "subspace/fn/fn.h"
#include "subspace/mem/forward.h"
#include "subspace/mem/move.h"
#include "subspace/mem/replace.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

namespace {

using sus::construct::Into;
using sus::fn::SFn;
using sus::fn::SFnMut;
using sus::fn::SFnOnce;

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

static_assert(sizeof(SFnOnce<void()>) == 2 * sizeof(void (*)()));

void v_v_function() {}
int i_f_function(float) { return 0; }
void v_f_function(float) {}
BaseClass* b_b_function(BaseClass* b) { return b; }
SubClass* s_b_function(BaseClass* b) { return static_cast<SubClass*>(b); }
SubClass* s_s_function(SubClass* b) { return b; }

auto b_b_lambda = [a = 1](BaseClass* b) -> BaseClass* { return b; };
auto s_b_lambda = [a = 1](BaseClass* b) -> SubClass* {
  return static_cast<SubClass*>(b);
};
auto s_s_lambda = [a = 1](SubClass* b) -> SubClass* { return b; };

// Fn types all have a never-value field.
static_assert(sus::mem::NeverValueField<SFnOnce<void()>>);
static_assert(sus::mem::NeverValueField<SFnMut<void()>>);
static_assert(sus::mem::NeverValueField<SFn<void()>>);
//  Which allows them to not require a flag in Option.
static_assert(sizeof(sus::Option<SFnOnce<void()>>) == sizeof(SFnOnce<void()>));

// Closures are not copyable.
static_assert(!std::is_copy_constructible_v<SFnOnce<void()>>);
static_assert(!std::is_copy_assignable_v<SFnOnce<void()>>);
static_assert(!std::is_copy_constructible_v<SFnMut<void()>>);
static_assert(!std::is_copy_assignable_v<SFnMut<void()>>);
static_assert(!std::is_copy_constructible_v<SFn<void()>>);
static_assert(!std::is_copy_assignable_v<SFn<void()>>);
// Closures can be moved.
static_assert(std::is_move_constructible_v<SFnOnce<void()>>);
static_assert(std::is_move_assignable_v<SFnOnce<void()>>);
static_assert(std::is_move_constructible_v<SFnMut<void()>>);
static_assert(std::is_move_assignable_v<SFnMut<void()>>);
static_assert(std::is_move_constructible_v<SFn<void()>>);
static_assert(std::is_move_assignable_v<SFn<void()>>);

// FnMut and Fn are Clone, but not FnOnce. None of them are Copy.
static_assert(sus::mem::Move<SFnOnce<void()>>);
static_assert(sus::mem::Move<SFnMut<void()>>);
static_assert(sus::mem::Move<SFn<void()>>);
static_assert(!sus::mem::Copy<SFnOnce<void()>>);
static_assert(!sus::mem::Copy<SFnMut<void()>>);
static_assert(!sus::mem::Copy<SFn<void()>>);
static_assert(!sus::mem::Clone<SFnOnce<void()>>);
static_assert(sus::mem::Clone<SFnMut<void()>>);
static_assert(sus::mem::Clone<SFn<void()>>);

// Closures are trivially relocatable.
static_assert(sus::mem::relocate_by_memcpy<SFnOnce<void()>>);
static_assert(sus::mem::relocate_by_memcpy<SFnMut<void()>>);
static_assert(sus::mem::relocate_by_memcpy<SFn<void()>>);

// A function pointer, or convertible lambda, can be bound to FnOnce, FnMut and
// Fn.
static_assert(std::is_constructible_v<SFnOnce<void()>, decltype([]() {})>);
static_assert(std::is_constructible_v<SFnMut<void()>, decltype([]() {})>);
static_assert(std::is_constructible_v<SFn<void()>, decltype([]() {})>);
static_assert(std::is_constructible_v<SFnOnce<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<SFnMut<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<SFn<void()>, decltype(v_v_function)>);
//  Non-void types for the same.
static_assert(std::is_constructible_v<SFnOnce<int(float)>,
                                      decltype([](float) { return 1; })>);
static_assert(std::is_constructible_v<SFnMut<int(float)>,
                                      decltype([](float) { return 1; })>);
static_assert(std::is_constructible_v<SFn<int(float)>,
                                      decltype([](float) { return 1; })>);
static_assert(
    std::is_constructible_v<SFnOnce<int(float)>, decltype(i_f_function)>);
static_assert(
    std::is_constructible_v<SFnMut<int(float)>, decltype(i_f_function)>);
static_assert(std::is_constructible_v<SFn<int(float)>, decltype(i_f_function)>);
//  Lambdas with bound args can be bound to FnOnce, FnMut and Fn.
static_assert(std::is_constructible_v<SFnOnce<void()>,
                                      decltype([i = int(1)]() { (void)i; })>);
static_assert(std::is_constructible_v<
              SFnOnce<void()>, decltype([i = int(1)]() mutable { ++i; })>);
static_assert(std::is_constructible_v<SFnMut<void()>,
                                      decltype([i = int(1)]() { (void)i; })>);
static_assert(std::is_constructible_v<
              SFnMut<void()>, decltype([i = int(1)]() mutable { ++i; })>);
static_assert(std::is_constructible_v<SFn<void()>,
                                      decltype([i = int(1)]() { (void)i; })>);
static_assert(std::is_constructible_v<
              SFn<void()>, decltype([i = int(1)]() mutable { ++i; })>);

// The return type of the SFnOnce must match that of the lambda. It will not
// allow converting to void.
static_assert(std::is_constructible_v<SFnOnce<SubClass*(BaseClass*)>,
                                      decltype(s_b_function)>);
static_assert(!std::is_constructible_v<SFnOnce<void(BaseClass*)>,
                                       decltype(b_b_function)>);
static_assert(!std::is_constructible_v<SFnOnce<SubClass*(BaseClass*)>,
                                       decltype(b_b_function)>);
static_assert(std::is_constructible_v<SFnOnce<SubClass*(BaseClass*)>,
                                      decltype(s_b_lambda)>);
static_assert(
    !std::is_constructible_v<SFnOnce<void(BaseClass*)>, decltype(b_b_lambda)>);
static_assert(!std::is_constructible_v<SFnOnce<SubClass*(BaseClass*)>,
                                       decltype(b_b_lambda)>);
// Similarly, argument types can't be converted to a different type.
static_assert(std::is_constructible_v<SFnOnce<SubClass*(SubClass*)>,
                                      decltype(s_s_function)>);
static_assert(!std::is_constructible_v<SFnOnce<SubClass*(BaseClass*)>,
                                       decltype(s_s_function)>);
static_assert(std::is_constructible_v<SFnOnce<SubClass*(SubClass*)>,
                                      decltype(s_s_lambda)>);
static_assert(!std::is_constructible_v<SFnOnce<SubClass*(BaseClass*)>,
                                       decltype(s_s_lambda)>);
// But SFnOnce type is compatible with convertible return and argument types in
// opposite directions.
// - If the return type Y of a lambda is convertible _to_ X, then SFnOnce<X()>
// can be
//   used to store it.
// - If the argument type Y of a lambda is convertible _from_ X, then
// SFnOnce<(X)>
//   can be used to store it.
//
// In both cases, the SFnOnce is more strict than the lambda, guaranteeing that
// the lambda's requirements are met.
static_assert(std::is_constructible_v<SFnOnce<BaseClass*(BaseClass*)>,
                                      decltype(s_b_lambda)>);
static_assert(std::is_constructible_v<SFnOnce<SubClass*(SubClass*)>,
                                      decltype(s_b_lambda)>);
// HOWEVER: When Fn is passed a function pointer, it stores a function pointer.
// C++20 does not yet allow us to erase the type of that function pointer in a
// constexpr context. So the types in the pointer must match exactly to the Fn's
// signature.
static_assert(!std::is_constructible_v<SFnOnce<BaseClass*(BaseClass*)>,
                                       decltype(s_b_function)>);
static_assert(!std::is_constructible_v<SFnOnce<SubClass*(SubClass*)>,
                                       decltype(s_b_function)>);

template <class R, class Param, class Arg>
concept can_run =
    requires(Arg&& arg, SFnOnce<R(Param)> fnonce, SFnMut<R(Param)> fnmut
             //,
             //  Fn<R(Param)> fn
    ) {
      { sus::move(fnonce)(sus::forward<Arg>(arg)) };
      { fnmut(sus::forward<Arg>(arg)) };
      //{ sus::move(fn)(sus::forward<Arg>(arg)) };
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

TEST(SFn, Pointer) {
  {
    auto receive_fn = [](SFnOnce<i32(i32, i32)> f, i32 a, i32 b) {
      return sus::move(f)(a, b);
    };
    auto lambda = [](i32 a, i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 1, 2), 4);
  }
  {
    auto receive_fn = [](SFnMut<i32(i32, i32)> f, i32 a, i32 b) {
      f(a, b);
      return sus::move(f)(a, b);
    };
    auto lambda = [i = 1_i32](i32 a, i32 b) mutable {
      i += 1;
      return a * 2 + b;
    };
    EXPECT_EQ(receive_fn(lambda, 1, 2), 4);
  }
  {
    {
      auto receive_fn = [](SFn<i32(i32, i32)> f, i32 a, i32 b) {
        f(a, b);
        return sus::move(f)(a, b);
      };
      auto lambda = [](i32 a, i32 b) { return a * 2 + b; };
      EXPECT_EQ(receive_fn(lambda, 1, 2), 4);
    }
  }
}

TEST(SFn, Lambda) {
  {
    auto receive_fn = [](SFnOnce<i32(i32)> f, i32 b) {
      return sus::move(f)(b);
    };
    auto lambda = [a = 1_i32](i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 2), 4);
  }
  {
    auto receive_fn = [](SFnMut<i32(i32)> f, i32 b) {
      f(b);
      return sus::move(f)(b);
    };
    auto lambda = [a = 1_i32](i32 b) mutable {
      a += 1;
      return a * 2 + b;
    };
    EXPECT_EQ(receive_fn(lambda, 2), 8);
  }
  {
    auto receive_fn = [](SFn<i32(i32)> f, i32 b) {
      f(b);
      return sus::move(f)(b);
    };
    auto lambda = [a = 1_i32](i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 2), 4);
  }
}

TEST(SFnDeathTest, NullPointer) {
  void (*f)() = nullptr;
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((SFnOnce<void()>(f)), "");
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((SFnMut<void()>(f)), "");
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((SFn<void()>(f)), "");
#endif
}

TEST(SFnDeathTest, CallAfterMoveConstruct) {
  {
    [](SFnOnce<void()> x) {
      [](auto) {}(sus::move(x));
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](SFnMut<void()> x) {
      [](auto) {}(sus::move(x));
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](SFn<void()> x) {
      [](auto) {}(sus::move(x));
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
}

TEST(SFnDeathTest, CallAfterMoveAssign) {
  {
    [](SFnOnce<void()> x, SFnOnce<void()> y) {
      y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {}, []() {});
  }
  {
    [](SFnMut<void()> x, SFnMut<void()> y) {
      y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {}, []() {});
  }
  {
    [](SFn<void()> x, SFn<void()> y) {
      y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {}, []() {});
  }
}

TEST(SFnDeathTest, CallAfterCall) {
  {
    [](SFnOnce<void()> x) {
      sus::move(x)();
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](SFnMut<void()> x) {
      sus::move(x)();
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](SFn<void()> x) {
      sus::move(x)();
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
}

TEST(SFnMut, ConvertToSFnOnce) {
  auto receive_fnonce = [](SFnOnce<i32()> x) { return sus::move(x)(); };
  auto receive_fnmut = [&](SFnMut<i32()> x) {
    return receive_fnonce(sus::move(x));
  };
  EXPECT_EQ(receive_fnmut([]() { return 2_i32; }), 2);
}

TEST(SFn, ConvertToSFnOnce) {
  auto receive_fnonce = [](SFnOnce<i32()> x) { return sus::move(x)(); };
  auto receive_fn = [&](SFn<i32()> x) { return receive_fnonce(sus::move(x)); };
  EXPECT_EQ(receive_fn([]() { return 2_i32; }), 2);
}

TEST(SFn, ConvertToSFnMut) {
  auto receive_fnmut = [](SFnMut<i32()> x) { return sus::move(x)(); };
  auto receive_fn = [&](SFn<i32()> x) { return receive_fnmut(sus::move(x)); };
  EXPECT_EQ(receive_fn([]() { return 2_i32; }), 2);
}

TEST(SFn, CallsCorrectOverload) {
  static i32 const_calls;
  static i32 mut_calls;
  struct S {
    void operator()() const { const_calls += 1; }
    void operator()() { mut_calls += 1; }
  };

  [](SFnOnce<void()> m) { sus::move(m)(); }(S());
  EXPECT_EQ(const_calls, 0);
  EXPECT_EQ(mut_calls, 1);

  [](SFnMut<void()> m) { m(); }(S());
  EXPECT_EQ(const_calls, 0);
  EXPECT_EQ(mut_calls, 2);

  [](SFn<void()> m) { m(); }(S());
  EXPECT_EQ(const_calls, 1);
  EXPECT_EQ(mut_calls, 2);

  // The SFn is converted to SFnMut but still calls the const overload.
  [](SFn<void()> m) { [](SFnMut<void()> m) { m(); }(sus::move(m)); }(S());
  EXPECT_EQ(const_calls, 2);
  EXPECT_EQ(mut_calls, 2);

  // The SFn is converted to SFnOnce but still calls the const overload.
  [](SFn<void()> m) {
    [](SFnOnce<void()> m) { sus::move(m)(); }(sus::move(m));
  }(S());
  EXPECT_EQ(const_calls, 3);
  EXPECT_EQ(mut_calls, 2);
}

}  // namespace
