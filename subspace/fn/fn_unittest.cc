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

// clang-format-off

struct BaseClass {};
struct SubClass : public BaseClass {};

static_assert(sizeof(FnOnce<void()>) == 2 * sizeof(void (*)()));

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
static_assert(sus::mem::NeverValueField<FnOnce<void()>>);
static_assert(sus::mem::NeverValueField<FnMut<void()>>);
static_assert(sus::mem::NeverValueField<Fn<void()>>);
//  Which allows them to not require a flag in Option.
static_assert(sizeof(sus::Option<FnOnce<void()>>) == sizeof(FnOnce<void()>));

// Closures are not copyable.
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

// FnMut and Fn are Clone, but not FnOnce. None of them are Copy.
static_assert(sus::mem::Move<FnOnce<void()>>);
static_assert(sus::mem::Move<FnMut<void()>>);
static_assert(sus::mem::Move<Fn<void()>>);
static_assert(!sus::mem::Copy<FnOnce<void()>>);
static_assert(!sus::mem::Copy<FnMut<void()>>);
static_assert(!sus::mem::Copy<Fn<void()>>);
static_assert(!sus::mem::Clone<FnOnce<void()>>);
static_assert(sus::mem::Clone<FnMut<void()>>);
static_assert(sus::mem::Clone<Fn<void()>>);

// Closures are trivially relocatable.
static_assert(sus::mem::relocate_by_memcpy<FnOnce<void()>>);
static_assert(sus::mem::relocate_by_memcpy<FnMut<void()>>);
static_assert(sus::mem::relocate_by_memcpy<Fn<void()>>);

// clang-format off

// A function pointer, or convertible lambda, can be bound to FnOnce, FnMut and
// Fn.
static_assert(std::is_constructible_v<FnOnce<void()>, decltype([]() {})>);
static_assert(std::is_constructible_v<FnMut<void()>, decltype([]() {})>);
static_assert(std::is_constructible_v<Fn<void()>, decltype([]() {})>);
static_assert(std::is_constructible_v<FnOnce<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<FnMut<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<Fn<void()>, decltype(v_v_function)>);
//  Non-void types for the same.
static_assert(std::is_constructible_v<
    FnOnce<int(float)>, decltype([](float) { return 1; })>
);
static_assert(std::is_constructible_v<
    FnMut<int(float)>, decltype([](float) { return 1; })>
);
static_assert(std::is_constructible_v<
    Fn<int(float)>, decltype([](float) { return 1; })>
);
static_assert(std::is_constructible_v<
    FnOnce<int(float)>, decltype(i_f_function)>
);
static_assert(std::is_constructible_v<
    FnMut<int(float)>, decltype(i_f_function)>
);
static_assert(std::is_constructible_v<
    Fn<int(float)>, decltype(i_f_function)>
);
// Lambdas with bound args can be bound to FnOnce, FnMut and Fn.
static_assert(std::is_constructible_v<
    FnOnce<void()>, decltype([i = int(1)]() { (void)i; })>
);
static_assert(std::is_constructible_v<
    FnOnce<void()>, decltype([i = int(1)]() mutable { ++i; })>
);
static_assert(std::is_constructible_v<
    FnMut<void()>, decltype([i = int(1)]() { (void)i; })>
);
static_assert(std::is_constructible_v<
    FnMut<void()>, decltype([i = int(1)]() mutable { ++i; })>
);
static_assert(std::is_constructible_v<
    Fn<void()>, decltype([i = int(1)]() { (void)i; })>
);
// But Fn, which is const, can't hold a mutable lambda.
static_assert(!std::is_constructible_v<
    Fn<void()>, decltype([i = int(1)]() mutable { ++i; })>
);

// The return type of the FnOnce must match that of the lambda. It will not
// allow converting to void.
static_assert(std::is_constructible_v<
    FnOnce<SubClass*(BaseClass*)>, decltype(s_b_function)>
);
static_assert(!std::is_constructible_v<
    FnOnce<void(BaseClass*)>, decltype(b_b_function)>
);
static_assert(!std::is_constructible_v<
    FnOnce<SubClass*(BaseClass*)>, decltype(b_b_function)>
);
static_assert(std::is_constructible_v<
    FnOnce<SubClass*(BaseClass*)>, decltype(s_b_lambda)>
);
static_assert(!std::is_constructible_v<
    FnOnce<void(BaseClass*)>, decltype(b_b_lambda)>
);
static_assert(!std::is_constructible_v<
    FnOnce<SubClass*(BaseClass*)>, decltype(b_b_lambda)>
);
// Similarly, argument types can't be converted to a different type.
static_assert(std::is_constructible_v<
    FnOnce<SubClass*(SubClass*)>, decltype(s_s_function)>
);
static_assert(!std::is_constructible_v<
    FnOnce<SubClass*(BaseClass*)>, decltype(s_s_function)>
);
static_assert(std::is_constructible_v<
    FnOnce<SubClass*(SubClass*)>, decltype(s_s_lambda)>
);
static_assert(!std::is_constructible_v<
    FnOnce<SubClass*(BaseClass*)>, decltype(s_s_lambda)>
);
// But FnOnce type is compatible with convertible return and argument types in
// opposite directions.
// - If the return type Y of a lambda is convertible _to_ X, then FnOnce<X()>
// can be
//   used to store it.
// - If the argument type Y of a lambda is convertible _from_ X, then
// FnOnce<(X)>
//   can be used to store it.
//
// In both cases, the FnOnce is more strict than the lambda, guaranteeing that
// the lambda's requirements are met.
static_assert(std::is_constructible_v<
    FnOnce<BaseClass*(BaseClass*)>, decltype(s_b_lambda)>
);
static_assert(std::is_constructible_v<
    FnOnce<SubClass*(SubClass*)>, decltype(s_b_lambda)>
);
static_assert(std::is_constructible_v<
    FnOnce<BaseClass*(BaseClass*)>, decltype(s_b_function)>
);
static_assert(std::is_constructible_v<
    FnOnce<SubClass*(SubClass*)>, decltype(s_b_function)>
);

// clang-format on

template <class R, class Param, class Arg>
concept can_run =
    requires(Arg&& arg, FnOnce<R(Param)> fnonce, FnMut<R(Param)> fnmut
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

TEST(Fn, Pointer) {
  {
    auto receive_fn = [](FnOnce<i32(i32, i32)> f, i32 a, i32 b) {
      return sus::move(f)(a, b);
    };
    auto* ptr = +[](i32 a, i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(ptr, 1, 2), 4);
  }
  {
    auto receive_fn = [](FnMut<i32(i32, i32)> f, i32 a, i32 b) {
      f(a, b);
      return sus::move(f)(a, b);
    };
    auto* ptr = +[](i32 a, i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(ptr, 1, 2), 4);
  }
  {
    {
      auto receive_fn = [](Fn<i32(i32, i32)> f, i32 a, i32 b) {
        f(a, b);
        return sus::move(f)(a, b);
      };
      auto* ptr = +[](i32 a, i32 b) { return a * 2 + b; };
      EXPECT_EQ(receive_fn(ptr, 1, 2), 4);
    }
  }
}

TEST(Fn, CapturelessLambda) {
  {
    auto receive_fn = [](FnOnce<i32(i32, i32)> f, i32 a, i32 b) {
      return sus::move(f)(a, b);
    };
    auto lambda = [](i32 a, i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 1, 2), 4);
  }
  {
    auto receive_fn = [](FnMut<i32(i32, i32)> f, i32 a, i32 b) {
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
      auto receive_fn = [](Fn<i32(i32, i32)> f, i32 a, i32 b) {
        f(a, b);
        return sus::move(f)(a, b);
      };
      auto lambda = [](i32 a, i32 b) { return a * 2 + b; };
      EXPECT_EQ(receive_fn(lambda, 1, 2), 4);
    }
  }
}

TEST(Fn, Lambda) {
  {
    auto receive_fn = [](FnOnce<i32(i32)> f, i32 b) { return sus::move(f)(b); };
    auto lambda = [a = 1_i32](i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 2), 4);
  }
  {
    auto receive_fn = [](FnMut<i32(i32)> f, i32 b) {
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
    auto receive_fn = [](Fn<i32(i32)> f, i32 b) {
      f(b);
      return sus::move(f)(b);
    };
    auto lambda = [a = 1_i32](i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 2), 4);
  }
}

TEST(Fn, TemplateLambda) {
  {
    auto receive_fn = [](FnOnce<i32(i32)> f, i32 b) { return sus::move(f)(b); };
    auto lambda = [a = 1_i32](auto b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 2), 4);
  }
  {
    auto receive_fn = [](FnMut<i32(i32)> f, i32 b) {
      f(b);
      return sus::move(f)(b);
    };
    auto lambda = [a = 1_i32](auto b) mutable {
      a += 1;
      return a * 2 + b;
    };
    EXPECT_EQ(receive_fn(lambda, 2), 8);
  }
  {
    auto receive_fn = [](Fn<i32(i32)> f, i32 b) {
      f(b);
      return sus::move(f)(b);
    };
    auto lambda = [a = 1_i32](auto b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 2), 4);
  }
}

TEST(FnDeathTest, NullPointer) {
  void (*f)() = nullptr;
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((FnOnce<void()>(f)), "");
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((FnMut<void()>(f)), "");
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((Fn<void()>(f)), "");
#endif
}

TEST(FnDeathTest, CallAfterMoveConstruct) {
  {
    [](FnOnce<void()> x) {
      [](auto) {}(sus::move(x));
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](FnMut<void()> x) {
      [](auto) {}(sus::move(x));
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](Fn<void()> x) {
      [](auto) {}(sus::move(x));
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
}

TEST(FnDeathTest, CallAfterMoveAssign) {
  {
    [](FnOnce<void()> x, FnOnce<void()> y) {
      y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {}, []() {});
  }
  {
    [](FnMut<void()> x, FnMut<void()> y) {
      y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {}, []() {});
  }
  {
    [](Fn<void()> x, Fn<void()> y) {
      y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {}, []() {});
  }
}

TEST(FnDeathTest, CallAfterCall) {
  {
    [](FnOnce<void()> x) {
      sus::move(x)();
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](FnMut<void()> x) {
      sus::move(x)();
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](Fn<void()> x) {
      sus::move(x)();
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
}

TEST(FnMut, ConvertToFnOnce) {
  auto receive_fnonce = [](FnOnce<i32()> x) { return sus::move(x)(); };
  auto receive_fnmut = [&](FnMut<i32()> x) {
    return receive_fnonce(sus::move(x));
  };
  EXPECT_EQ(receive_fnmut([]() { return 2_i32; }), 2);
}

TEST(Fn, ConvertToFnOnce) {
  auto receive_fnonce = [](FnOnce<i32()> x) { return sus::move(x)(); };
  auto receive_fn = [&](Fn<i32()> x) { return receive_fnonce(sus::move(x)); };
  EXPECT_EQ(receive_fn([]() { return 2_i32; }), 2);
}

TEST(Fn, ConvertToFnMut) {
  auto receive_fnmut = [](FnMut<i32()> x) { return sus::move(x)(); };
  auto receive_fn = [&](Fn<i32()> x) { return receive_fnmut(sus::move(x)); };
  EXPECT_EQ(receive_fn([]() { return 2_i32; }), 2);
}

TEST(Fn, ConstructionFromConstMut) {
  struct Captureless {
    i32 operator()() const { return 2; }
  };
  struct Capture {
    i32 operator()() const { return i; }
    i32 i = 2;
  };

  // Const callable can be put in all the Fn types.
  static_assert(sus::fn::__private::CallableMut<Captureless, i32>);
  EXPECT_EQ(2, [](FnOnce<i32()> m) { return sus::move(m)(); }(Captureless()));
  EXPECT_EQ(2, [](FnMut<i32()> m) { return sus::move(m)(); }(Captureless()));
  EXPECT_EQ(2, [](Fn<i32()> m) { return sus::move(m)(); }(Captureless()));
  static_assert(sus::fn::__private::CallableMut<Capture, i32>);
  EXPECT_EQ(2, [](FnOnce<i32()> m) { return sus::move(m)(); }(Capture()));
  EXPECT_EQ(2, [](FnMut<i32()> m) { return sus::move(m)(); }(Capture()));
  EXPECT_EQ(2, [](Fn<i32()> m) { return sus::move(m)(); }(Capture()));

  struct CapturelessMut {
    i32 operator()() { return 2; }
  };
  struct CaptureMut {
    i32 operator()() { return i; }
    i32 i = 2;
  };

  // Mutable callable can only be put in the mutable Fn types.
  static_assert(sus::fn::__private::CallableMut<CapturelessMut, i32>);
  static_assert(std::is_constructible_v<FnOnce<i32()>, CapturelessMut&&>);
  static_assert(std::is_constructible_v<FnMut<i32()>, CapturelessMut&&>);
  static_assert(!std::is_constructible_v<Fn<i32()>, CapturelessMut&&>);
  EXPECT_EQ(2,
            [](FnOnce<i32()> m) { return sus::move(m)(); }(CapturelessMut()));
  EXPECT_EQ(2, [](FnMut<i32()> m) { return sus::move(m)(); }(CapturelessMut()));

  static_assert(sus::fn::__private::CallableMut<CaptureMut, i32>);
  static_assert(std::is_constructible_v<FnOnce<i32()>, CaptureMut&&>);
  static_assert(std::is_constructible_v<FnMut<i32()>, CaptureMut&&>);
  static_assert(!std::is_constructible_v<Fn<i32()>, CaptureMut&&>);
  EXPECT_EQ(2, [](FnOnce<i32()> m) { return sus::move(m)(); }(CaptureMut()));
  EXPECT_EQ(2, [](FnMut<i32()> m) { return sus::move(m)(); }(CaptureMut()));
}

TEST(Fn, IntoFromConstMut) {
  i32 (*f)() = +[]() { return 2_i32; };

  // Function pointer can be put in all the Fn types.
  static_assert(sus::construct::Into<decltype(f), FnOnce<i32()>>);
  static_assert(sus::construct::Into<decltype(f), FnMut<i32()>>);
  static_assert(sus::construct::Into<decltype(f), Fn<i32()>>);
  EXPECT_EQ(2, FnOnce<i32()>::from(f)());
  EXPECT_EQ(2, FnMut<i32()>::from(f)());
  EXPECT_EQ(2, Fn<i32()>::from(f)());

  struct Captureless {
    i32 operator()() const { return 2; }
  };
  struct Capture {
    i32 operator()() const { return i; }
    i32 i = 2;
  };

  // Const callable can be put in all the Fn types.
  static_assert(sus::construct::Into<Captureless, FnOnce<i32()>>);
  static_assert(sus::construct::Into<Captureless, FnMut<i32()>>);
  static_assert(sus::construct::Into<Captureless, Fn<i32()>>);
  EXPECT_EQ(2, FnOnce<i32()>::from(Captureless())());
  EXPECT_EQ(2, FnMut<i32()>::from(Captureless())());
  EXPECT_EQ(2, Fn<i32()>::from(Captureless())());
  static_assert(sus::construct::Into<Capture, FnOnce<i32()>>);
  static_assert(sus::construct::Into<Capture, FnMut<i32()>>);
  static_assert(sus::construct::Into<Capture, Fn<i32()>>);
  EXPECT_EQ(2, FnOnce<i32()>::from(Capture())());
  EXPECT_EQ(2, FnMut<i32()>::from(Capture())());
  EXPECT_EQ(2, Fn<i32()>::from(Capture())());

  struct CapturelessMut {
    i32 operator()() { return 2; }
  };
  struct CaptureMut {
    i32 operator()() { return i; }
    i32 i = 2;
  };

  // Mutable callable can only be put in the mutable Fn types.
  static_assert(sus::construct::Into<CapturelessMut, FnOnce<i32()>>);
  static_assert(sus::construct::Into<CapturelessMut, FnMut<i32()>>);
  static_assert(!sus::construct::Into<CapturelessMut, Fn<i32()>>);
  EXPECT_EQ(2, FnOnce<i32()>::from(CapturelessMut())());
  EXPECT_EQ(2, FnMut<i32()>::from(CapturelessMut())());

  static_assert(sus::construct::Into<CaptureMut, FnOnce<i32()>>);
  static_assert(sus::construct::Into<CaptureMut, FnMut<i32()>>);
  static_assert(!sus::construct::Into<CaptureMut, Fn<i32()>>);
  EXPECT_EQ(2, FnOnce<i32()>::from(CaptureMut())());
  EXPECT_EQ(2, FnMut<i32()>::from(CaptureMut())());
}

TEST(Fn, CallsCorrectOverload) {
  static i32 const_calls;
  static i32 mut_calls;
  struct S {
    void operator()() const { const_calls += 1; }
    void operator()() { mut_calls += 1; }
  };

  [](FnOnce<void()> m) { sus::move(m)(); }(S());
  EXPECT_EQ(const_calls, 0);
  EXPECT_EQ(mut_calls, 1);

  [](FnMut<void()> m) { m(); }(S());
  EXPECT_EQ(const_calls, 0);
  EXPECT_EQ(mut_calls, 2);

  [](Fn<void()> m) { m(); }(S());
  EXPECT_EQ(const_calls, 1);
  EXPECT_EQ(mut_calls, 2);

  // The Fn is converted to FnMut but still calls the const overload.
  [](Fn<void()> m) { [](FnMut<void()> m) { m(); }(sus::move(m)); }(S());
  EXPECT_EQ(const_calls, 2);
  EXPECT_EQ(mut_calls, 2);

  // The Fn is converted to FnOnce but still calls the const overload.
  [](Fn<void()> m) {
    [](FnOnce<void()> m) { sus::move(m)(); }(sus::move(m));
  }(S());
  EXPECT_EQ(const_calls, 3);
  EXPECT_EQ(mut_calls, 2);
}

TEST(Fn, Clone) {
  static_assert(sus::mem::Clone<Fn<i32()>>);
  auto clones_fn = [](Fn<i32()> f) {
    return [](FnOnce<i32()> f1) { return sus::move(f1)(); }(f.clone()) +
           [](FnOnce<i32()> f2) { return sus::move(f2)(); }(f.clone());
  };
  EXPECT_EQ(4, clones_fn([]() { return 2_i32; }));

  static_assert(sus::mem::Clone<FnMut<i32()>>);
  auto clones_fnmut = [](FnMut<i32()> f) {
    return [](FnOnce<i32()> f1) { return sus::move(f1)(); }(f.clone()) +
           [](FnOnce<i32()> f2) { return sus::move(f2)(); }(f.clone());
  };
  EXPECT_EQ(5, clones_fnmut([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));

  static_assert(!sus::mem::Clone<FnOnce<i32()>>);
}

TEST(FnOnce, Split) {
  // The return type of split() is not Copy or Move
  static_assert(
      !::sus::mem::Copy<decltype(std::declval<FnOnce<void()>&>().split())>);
  static_assert(
      !::sus::mem::Move<decltype(std::declval<FnOnce<void()>&>().split())>);
  // It's only used to build more FnOnce objects.
  static_assert(
      ::sus::construct::Into<decltype(std::declval<FnOnce<void()>&>().split()),
                             FnOnce<void()>>);
  // And not FnMut or Fn, as that loses the intention to only call it once. This
  // is implemented by making the operator() callable as an rvalue only.
  static_assert(
      !::sus::construct::Into<decltype(std::declval<FnOnce<void()>&>().split()),
                              FnMut<void()>>);
  static_assert(
      !::sus::construct::Into<decltype(std::declval<FnOnce<void()>&>().split()),
                              Fn<void()>>);

  // split() as rvalues. First split is run.
  auto rsplits_fnonce = [](FnOnce<i32()> f) {
    i32 a = [](FnOnce<i32()> f) { return sus::move(f)(); }(f.split());
    i32 b = [](FnOnce<i32()>) {
      // Don't run the `FnOnce` as only one of the splits may run the
      // FnOnce.
      return 0_i32;
    }(f.split());
    return a + b;
  };
  EXPECT_EQ(2, rsplits_fnonce([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));

  // split() as rvalues. Second split is run.
  auto rsplits_fnonce2 = [](FnOnce<i32()> f) {
    i32 a = [](FnOnce<i32()>) {
      // Don't run the `FnOnce` as only one of the splits may run the
      // FnOnce.
      return 0_i32;
    }(f.split());
    i32 b = [](FnOnce<i32()> f) { return sus::move(f)(); }(f.split());
    return a + b;
  };
  EXPECT_EQ(2, rsplits_fnonce([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));

  // split() as lvalues. First split is run.
  auto lsplits_fnonce = [](FnOnce<i32()> f) {
    auto split = f.split();
    i32 a = [](FnOnce<i32()> f) { return sus::move(f)(); }(f);
    i32 b = [](FnOnce<i32()>) {
      // Don't run the `FnOnce` as only one of the splits may run the
      // FnOnce.
      return 0_i32;
    }(f);
    return a + b;
  };
  EXPECT_EQ(2, lsplits_fnonce([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));

  // split() as lvalues. Second split is run.
  auto lsplits_fnonce2 = [](FnOnce<i32()> f) {
    auto split = f.split();
    i32 a = [](FnOnce<i32()>) {
      // Don't run the `FnOnce` as only one of the splits may run the
      // FnOnce.
      return 0_i32;
    }(f);
    i32 b = [](FnOnce<i32()> f) { return sus::move(f)(); }(f);
    return a + b;
  };
  EXPECT_EQ(2, lsplits_fnonce([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));
}

}  // namespace
