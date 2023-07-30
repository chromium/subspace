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
#include "subspace/mem/never_value.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/replace.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

namespace {

using sus::construct::Into;
using sus::fn::FnMutRef;
using sus::fn::FnOnceRef;
using sus::fn::FnRef;

static_assert(sus::mem::NeverValueField<FnRef<void()>>);
static_assert(sus::mem::NeverValueField<FnMutRef<void()>>);
static_assert(sus::mem::NeverValueField<FnOnceRef<void()>>);
static_assert(sus::mem::relocate_by_memcpy<FnRef<void()>>);
static_assert(sus::mem::relocate_by_memcpy<FnMutRef<void()>>);
static_assert(sus::mem::relocate_by_memcpy<FnOnceRef<void()>>);

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

static_assert(sizeof(FnOnceRef<void()>) == 2 * sizeof(void (*)()));

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

// FnRef types all have a never-value field.
static_assert(sus::mem::NeverValueField<FnOnceRef<void()>>);
static_assert(sus::mem::NeverValueField<FnMutRef<void()>>);
static_assert(sus::mem::NeverValueField<FnRef<void()>>);
//  Which allows them to not require a flag in Option.
static_assert(sizeof(sus::Option<FnOnceRef<void()>>) ==
              sizeof(FnOnceRef<void()>));

// Closures are not copyable.
static_assert(!std::is_copy_constructible_v<FnOnceRef<void()>>);
static_assert(!std::is_copy_assignable_v<FnOnceRef<void()>>);
static_assert(!std::is_copy_constructible_v<FnMutRef<void()>>);
static_assert(!std::is_copy_assignable_v<FnMutRef<void()>>);
static_assert(!std::is_copy_constructible_v<FnRef<void()>>);
static_assert(!std::is_copy_assignable_v<FnRef<void()>>);
// Closures can be moved.
static_assert(std::is_move_constructible_v<FnOnceRef<void()>>);
static_assert(std::is_move_assignable_v<FnOnceRef<void()>>);
static_assert(std::is_move_constructible_v<FnMutRef<void()>>);
static_assert(std::is_move_assignable_v<FnMutRef<void()>>);
static_assert(std::is_move_constructible_v<FnRef<void()>>);
static_assert(std::is_move_assignable_v<FnRef<void()>>);

// FnMutRef and FnRef are Clone, but not FnOnceRef. None of them are Copy.
static_assert(sus::mem::Move<FnOnceRef<void()>>);
static_assert(sus::mem::Move<FnMutRef<void()>>);
static_assert(sus::mem::Move<FnRef<void()>>);
static_assert(!sus::mem::Copy<FnOnceRef<void()>>);
static_assert(!sus::mem::Copy<FnMutRef<void()>>);
static_assert(!sus::mem::Copy<FnRef<void()>>);
static_assert(!sus::mem::Clone<FnOnceRef<void()>>);
static_assert(sus::mem::Clone<FnMutRef<void()>>);
static_assert(sus::mem::Clone<FnRef<void()>>);

// Closures are trivially relocatable.
static_assert(sus::mem::relocate_by_memcpy<FnOnceRef<void()>>);
static_assert(sus::mem::relocate_by_memcpy<FnMutRef<void()>>);
static_assert(sus::mem::relocate_by_memcpy<FnRef<void()>>);

// clang-format off

// A function pointer, or convertible lambda, can be bound to FnOnceRef, FnMutRef and
// FnRef.
static_assert(std::is_constructible_v<FnOnceRef<void()>, decltype([]() {})>);
static_assert(std::is_constructible_v<FnMutRef<void()>, decltype([]() {})>);
static_assert(std::is_constructible_v<FnRef<void()>, decltype([]() {})>);
static_assert(std::is_constructible_v<FnOnceRef<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<FnMutRef<void()>, decltype(v_v_function)>);
static_assert(std::is_constructible_v<FnRef<void()>, decltype(v_v_function)>);
//  Non-void types for the same.
static_assert(std::is_constructible_v<
    FnOnceRef<int(float)>, decltype([](float) { return 1; })>
);
static_assert(std::is_constructible_v<
    FnMutRef<int(float)>, decltype([](float) { return 1; })>
);
static_assert(std::is_constructible_v<
    FnRef<int(float)>, decltype([](float) { return 1; })>
);
static_assert(std::is_constructible_v<
    FnOnceRef<int(float)>, decltype(i_f_function)>
);
static_assert(std::is_constructible_v<
    FnMutRef<int(float)>, decltype(i_f_function)>
);
static_assert(std::is_constructible_v<
    FnRef<int(float)>, decltype(i_f_function)>
);
// Lambdas with bound args can be bound to FnOnceRef, FnMutRef and FnRef.
static_assert(std::is_constructible_v<
    FnOnceRef<void()>, decltype([i = int(1)]() { (void)i; })>
);
static_assert(std::is_constructible_v<
    FnOnceRef<void()>, decltype([i = int(1)]() mutable { ++i; })>
);
static_assert(std::is_constructible_v<
    FnMutRef<void()>, decltype([i = int(1)]() { (void)i; })>
);
static_assert(std::is_constructible_v<
    FnMutRef<void()>, decltype([i = int(1)]() mutable { ++i; })>
);
static_assert(std::is_constructible_v<
    FnRef<void()>, decltype([i = int(1)]() { (void)i; })>
);
// But FnRef, which is const, can't hold a mutable lambda.
static_assert(!std::is_constructible_v<
    FnRef<void()>, decltype([i = int(1)]() mutable { ++i; })>
);

// The return type of the FnOnceRef must match that of the lambda. It will not
// allow converting to void.
static_assert(std::is_constructible_v<
    FnOnceRef<SubClass*(BaseClass*)>, decltype(s_b_function)>
);
static_assert(!std::is_constructible_v<
    FnOnceRef<void(BaseClass*)>, decltype(b_b_function)>
);
static_assert(!std::is_constructible_v<
    FnOnceRef<SubClass*(BaseClass*)>, decltype(b_b_function)>
);
static_assert(std::is_constructible_v<
    FnOnceRef<SubClass*(BaseClass*)>, decltype(s_b_lambda)>
);
static_assert(!std::is_constructible_v<
    FnOnceRef<void(BaseClass*)>, decltype(b_b_lambda)>
);
static_assert(!std::is_constructible_v<
    FnOnceRef<SubClass*(BaseClass*)>, decltype(b_b_lambda)>
);
// Similarly, argument types can't be converted to a different type.
static_assert(std::is_constructible_v<
    FnOnceRef<SubClass*(SubClass*)>, decltype(s_s_function)>
);
static_assert(!std::is_constructible_v<
    FnOnceRef<SubClass*(BaseClass*)>, decltype(s_s_function)>
);
static_assert(std::is_constructible_v<
    FnOnceRef<SubClass*(SubClass*)>, decltype(s_s_lambda)>
);
static_assert(!std::is_constructible_v<
    FnOnceRef<SubClass*(BaseClass*)>, decltype(s_s_lambda)>
);
// But FnOnceRef type is compatible with convertible return and argument types in
// opposite directions.
// - If the return type Y of a lambda is convertible _to_ X, then FnOnceRef<X()>
// can be
//   used to store it.
// - If the argument type Y of a lambda is convertible _from_ X, then
// FnOnceRef<(X)>
//   can be used to store it.
//
// In both cases, the FnOnceRef is more strict than the lambda, guaranteeing that
// the lambda's requirements are met.
static_assert(std::is_constructible_v<
    FnOnceRef<BaseClass*(BaseClass*)>, decltype(s_b_lambda)>
);
static_assert(std::is_constructible_v<
    FnOnceRef<SubClass*(SubClass*)>, decltype(s_b_lambda)>
);
static_assert(std::is_constructible_v<
    FnOnceRef<BaseClass*(BaseClass*)>, decltype(s_b_function)>
);
static_assert(std::is_constructible_v<
    FnOnceRef<SubClass*(SubClass*)>, decltype(s_b_function)>
);

// clang-format on

template <class R, class Param, class Arg>
concept can_run =
    requires(Arg&& arg, FnOnceRef<R(Param)> fnonce, FnMutRef<R(Param)> FnMutRef
             //,
             //  FnRef<R(Param)> FnRef
    ) {
      { sus::move(fnonce)(sus::forward<Arg>(arg)) };
      { FnMutRef(sus::forward<Arg>(arg)) };
      //{ sus::move(FnRef)(sus::forward<Arg>(arg)) };
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

TEST(FnRef, Pointer) {
  {
    auto receive_fn = [](FnOnceRef<i32(i32, i32)> f, i32 a, i32 b) {
      return sus::move(f)(a, b);
    };
    auto* ptr = +[](i32 a, i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(ptr, 1, 2), 4);
  }
  {
    auto receive_fn = [](FnMutRef<i32(i32, i32)> f, i32 a, i32 b) {
      f(a, b);
      return sus::move(f)(a, b);
    };
    auto* ptr = +[](i32 a, i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(ptr, 1, 2), 4);
  }
  {
    {
      auto receive_fn = [](FnRef<i32(i32, i32)> f, i32 a, i32 b) {
        f(a, b);
        return sus::move(f)(a, b);
      };
      auto* ptr = +[](i32 a, i32 b) { return a * 2 + b; };
      EXPECT_EQ(receive_fn(ptr, 1, 2), 4);
    }
  }
}

TEST(FnRef, CapturelessLambda) {
  {
    auto receive_fn = [](FnOnceRef<i32(i32, i32)> f, i32 a, i32 b) {
      return sus::move(f)(a, b);
    };
    auto lambda = [](i32 a, i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 1, 2), 4);
  }
  {
    auto receive_fn = [](FnMutRef<i32(i32, i32)> f, i32 a, i32 b) {
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
      auto receive_fn = [](FnRef<i32(i32, i32)> f, i32 a, i32 b) {
        f(a, b);
        return sus::move(f)(a, b);
      };
      auto lambda = [](i32 a, i32 b) { return a * 2 + b; };
      EXPECT_EQ(receive_fn(lambda, 1, 2), 4);
    }
  }
}

TEST(FnRef, Lambda) {
  {
    auto receive_fn = [](FnOnceRef<i32(i32)> f, i32 b) {
      return sus::move(f)(b);
    };
    auto lambda = [a = 1_i32](i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 2), 4);
  }
  {
    auto receive_fn = [](FnMutRef<i32(i32)> f, i32 b) {
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
    auto receive_fn = [](FnRef<i32(i32)> f, i32 b) {
      f(b);
      return sus::move(f)(b);
    };
    auto lambda = [a = 1_i32](i32 b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 2), 4);
  }
}

TEST(FnRef, TemplateLambda) {
  {
    auto receive_fn = [](FnOnceRef<i32(i32)> f, i32 b) {
      return sus::move(f)(b);
    };
    auto lambda = [a = 1_i32](auto b) { return a * 2 + b; };
    EXPECT_EQ(receive_fn(lambda, 2), 4);
  }
  {
    auto receive_fn = [](FnMutRef<i32(i32)> f, i32 b) {
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
    auto receive_fn = [](FnRef<i32(i32)> f, i32 b) {
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
  EXPECT_DEATH((FnOnceRef<void()>(f)), "");
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((FnMutRef<void()>(f)), "");
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((FnRef<void()>(f)), "");
#endif
}

TEST(FnDeathTest, CallAfterMoveConstruct) {
  {
    [](FnOnceRef<void()> x) {
      [](auto) {}(sus::move(x));
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](FnMutRef<void()> x) {
      [](auto) {}(sus::move(x));
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](FnRef<void()> x) {
      [](auto) {}(sus::move(x));
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
}

TEST(FnDeathTest, CallAfterMoveAssign) {
  {
    [](FnOnceRef<void()> x, FnOnceRef<void()> y) {
      y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {}, []() {});
  }
  {
    [](FnMutRef<void()> x, FnMutRef<void()> y) {
      y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {}, []() {});
  }
  {
    [](FnRef<void()> x, FnRef<void()> y) {
      y = sus::move(x);
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {}, []() {});
  }
}

TEST(FnDeathTest, CallAfterCall) {
  {
    [](FnOnceRef<void()> x) {
      sus::move(x)();
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](FnMutRef<void()> x) {
      sus::move(x)();
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
  {
    [](FnRef<void()> x) {
      sus::move(x)();
#if GTEST_HAS_DEATH_TEST
      EXPECT_DEATH(sus::move(x)(), "");
#endif
    }([]() {});
  }
}

TEST(FnMutRef, ConvertToFnOnce) {
  auto receive_fnonce = [](FnOnceRef<i32()> x) { return sus::move(x)(); };
  auto receive_fnmut = [&](FnMutRef<i32()> x) {
    return receive_fnonce(sus::move(x));
  };
  EXPECT_EQ(receive_fnmut([]() { return 2_i32; }), 2);
}

TEST(FnRef, ConvertToFnOnce) {
  auto receive_fnonce = [](FnOnceRef<i32()> x) { return sus::move(x)(); };
  auto receive_fn = [&](FnRef<i32()> x) {
    return receive_fnonce(sus::move(x));
  };
  EXPECT_EQ(receive_fn([]() { return 2_i32; }), 2);
}

TEST(FnRef, ConvertToFnMut) {
  auto receive_fnmut = [](FnMutRef<i32()> x) { return sus::move(x)(); };
  auto receive_fn = [&](FnRef<i32()> x) { return receive_fnmut(sus::move(x)); };
  EXPECT_EQ(receive_fn([]() { return 2_i32; }), 2);
}

TEST(FnRef, ConstructionFromConstMut) {
  struct Captureless {
    i32 operator()() const { return 2; }
  };
  struct Capture {
    i32 operator()() const { return i; }
    i32 i = 2;
  };

  // Const callable can be put in all the FnRef types.
  static_assert(sus::fn::__private::CallableMut<Captureless, i32>);
  EXPECT_EQ(2,
            [](FnOnceRef<i32()> m) { return sus::move(m)(); }(Captureless()));
  EXPECT_EQ(2, [](FnMutRef<i32()> m) { return sus::move(m)(); }(Captureless()));
  EXPECT_EQ(2, [](FnRef<i32()> m) { return sus::move(m)(); }(Captureless()));
  static_assert(sus::fn::__private::CallableMut<Capture, i32>);
  EXPECT_EQ(2, [](FnOnceRef<i32()> m) { return sus::move(m)(); }(Capture()));
  EXPECT_EQ(2, [](FnMutRef<i32()> m) { return sus::move(m)(); }(Capture()));
  EXPECT_EQ(2, [](FnRef<i32()> m) { return sus::move(m)(); }(Capture()));

  struct CapturelessMut {
    i32 operator()() { return 2; }
  };
  struct CaptureMut {
    i32 operator()() { return i; }
    i32 i = 2;
  };

  // Mutable callable can only be put in the mutable FnRef types.
  static_assert(sus::fn::__private::CallableMut<CapturelessMut, i32>);
  static_assert(std::is_constructible_v<FnOnceRef<i32()>, CapturelessMut&&>);
  static_assert(std::is_constructible_v<FnMutRef<i32()>, CapturelessMut&&>);
  static_assert(!std::is_constructible_v<FnRef<i32()>, CapturelessMut&&>);
  EXPECT_EQ(
      2, [](FnOnceRef<i32()> m) { return sus::move(m)(); }(CapturelessMut()));
  EXPECT_EQ(2,
            [](FnMutRef<i32()> m) { return sus::move(m)(); }(CapturelessMut()));

  static_assert(sus::fn::__private::CallableMut<CaptureMut, i32>);
  static_assert(std::is_constructible_v<FnOnceRef<i32()>, CaptureMut&&>);
  static_assert(std::is_constructible_v<FnMutRef<i32()>, CaptureMut&&>);
  static_assert(!std::is_constructible_v<FnRef<i32()>, CaptureMut&&>);
  EXPECT_EQ(2, [](FnOnceRef<i32()> m) { return sus::move(m)(); }(CaptureMut()));
  EXPECT_EQ(2, [](FnMutRef<i32()> m) { return sus::move(m)(); }(CaptureMut()));
}

TEST(FnRef, IntoFromConstMut) {
  i32 (*f)() = +[]() { return 2_i32; };

  // Function pointer can be put in all the FnRef types.
  static_assert(sus::construct::Into<decltype(f), FnOnceRef<i32()>>);
  static_assert(sus::construct::Into<decltype(f), FnMutRef<i32()>>);
  static_assert(sus::construct::Into<decltype(f), FnRef<i32()>>);
  EXPECT_EQ(2, FnOnceRef<i32()>::from(f)());
  EXPECT_EQ(2, FnMutRef<i32()>::from(f)());
  EXPECT_EQ(2, FnRef<i32()>::from(f)());

  struct Captureless {
    i32 operator()() const { return 2; }
  };
  struct Capture {
    i32 operator()() const { return i; }
    i32 i = 2;
  };

  // Const callable can be put in all the FnRef types.
  static_assert(sus::construct::Into<Captureless, FnOnceRef<i32()>>);
  static_assert(sus::construct::Into<Captureless, FnMutRef<i32()>>);
  static_assert(sus::construct::Into<Captureless, FnRef<i32()>>);
  EXPECT_EQ(2, FnOnceRef<i32()>::from(Captureless())());
  EXPECT_EQ(2, FnMutRef<i32()>::from(Captureless())());
  EXPECT_EQ(2, FnRef<i32()>::from(Captureless())());
  static_assert(sus::construct::Into<Capture, FnOnceRef<i32()>>);
  static_assert(sus::construct::Into<Capture, FnMutRef<i32()>>);
  static_assert(sus::construct::Into<Capture, FnRef<i32()>>);
  EXPECT_EQ(2, FnOnceRef<i32()>::from(Capture())());
  EXPECT_EQ(2, FnMutRef<i32()>::from(Capture())());
  EXPECT_EQ(2, FnRef<i32()>::from(Capture())());

  struct CapturelessMut {
    i32 operator()() { return 2; }
  };
  struct CaptureMut {
    i32 operator()() { return i; }
    i32 i = 2;
  };

  // Mutable callable can only be put in the mutable FnRef types.
  static_assert(sus::construct::Into<CapturelessMut, FnOnceRef<i32()>>);
  static_assert(sus::construct::Into<CapturelessMut, FnMutRef<i32()>>);
  static_assert(!sus::construct::Into<CapturelessMut, FnRef<i32()>>);
  EXPECT_EQ(2, FnOnceRef<i32()>::from(CapturelessMut())());
  EXPECT_EQ(2, FnMutRef<i32()>::from(CapturelessMut())());

  static_assert(sus::construct::Into<CaptureMut, FnOnceRef<i32()>>);
  static_assert(sus::construct::Into<CaptureMut, FnMutRef<i32()>>);
  static_assert(!sus::construct::Into<CaptureMut, FnRef<i32()>>);
  EXPECT_EQ(2, FnOnceRef<i32()>::from(CaptureMut())());
  EXPECT_EQ(2, FnMutRef<i32()>::from(CaptureMut())());
}

TEST(FnRef, CallsCorrectOverload) {
  static i32 const_calls;
  static i32 mut_calls;
  struct S {
    void operator()() const { const_calls += 1; }
    void operator()() { mut_calls += 1; }
  };

  [](FnOnceRef<void()> m) { sus::move(m)(); }(S());
  EXPECT_EQ(const_calls, 0);
  EXPECT_EQ(mut_calls, 1);

  [](FnMutRef<void()> m) { m(); }(S());
  EXPECT_EQ(const_calls, 0);
  EXPECT_EQ(mut_calls, 2);

  [](FnRef<void()> m) { m(); }(S());
  EXPECT_EQ(const_calls, 1);
  EXPECT_EQ(mut_calls, 2);

  // The FnRef is converted to FnMutRef but still calls the const overload.
  [](FnRef<void()> m) { [](FnMutRef<void()> m) { m(); }(sus::move(m)); }(S());
  EXPECT_EQ(const_calls, 2);
  EXPECT_EQ(mut_calls, 2);

  // The FnRef is converted to FnOnceRef but still calls the const overload.
  [](FnRef<void()> m) {
    [](FnOnceRef<void()> m) { sus::move(m)(); }(sus::move(m));
  }(S());
  EXPECT_EQ(const_calls, 3);
  EXPECT_EQ(mut_calls, 2);
}

TEST(FnRef, Clone) {
  static_assert(sus::mem::Clone<FnRef<i32()>>);
  auto clones_fn = [](FnRef<i32()> f) {
    return [](FnOnceRef<i32()> f1) { return sus::move(f1)(); }(f.clone()) +
           [](FnOnceRef<i32()> f2) { return sus::move(f2)(); }(f.clone());
  };
  EXPECT_EQ(4, clones_fn([]() { return 2_i32; }));

  static_assert(sus::mem::Clone<FnMutRef<i32()>>);
  auto clones_fnmut = [](FnMutRef<i32()> f) {
    return [](FnOnceRef<i32()> f1) { return sus::move(f1)(); }(f.clone()) +
           [](FnOnceRef<i32()> f2) { return sus::move(f2)(); }(f.clone());
  };
  EXPECT_EQ(5, clones_fnmut([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));

  static_assert(!sus::mem::Clone<FnOnceRef<i32()>>);
}

TEST(FnOnceRef, Split) {
  // The return type of split() is not Copy or Move
  static_assert(
      !::sus::mem::Copy<decltype(std::declval<FnOnceRef<void()>&>().split())>);
  static_assert(
      !::sus::mem::Move<decltype(std::declval<FnOnceRef<void()>&>().split())>);
  // It's only used to build more FnOnceRef objects.
  static_assert(::sus::construct::Into<
                decltype(std::declval<FnOnceRef<void()>&>().split()),
                FnOnceRef<void()>>);
  // And not FnMutRef or FnRef, as that loses the intention to only call it
  // once. This is implemented by making the operator() callable as an rvalue
  // only.
  static_assert(!::sus::construct::Into<
                decltype(std::declval<FnOnceRef<void()>&>().split()),
                FnMutRef<void()>>);
  static_assert(
      !::sus::construct::Into<
          decltype(std::declval<FnOnceRef<void()>&>().split()), FnRef<void()>>);

  // split() as rvalues. First split is run.
  auto rsplits_fnonce = [](FnOnceRef<i32()> f) {
    i32 a = [](FnOnceRef<i32()> f) { return sus::move(f)(); }(f.split());
    i32 b = [](FnOnceRef<i32()>) {
      // Don't run the `FnOnceRef` as only one of the splits may run the
      // FnOnceRef.
      return 0_i32;
    }(f.split());
    return a + b;
  };
  EXPECT_EQ(2, rsplits_fnonce([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));

  // split() as rvalues. Second split is run.
  auto rsplits_fnonce2 = [](FnOnceRef<i32()> f) {
    i32 a = [](FnOnceRef<i32()>) {
      // Don't run the `FnOnceRef` as only one of the splits may run the
      // FnOnceRef.
      return 0_i32;
    }(f.split());
    i32 b = [](FnOnceRef<i32()> f) { return sus::move(f)(); }(f.split());
    return a + b;
  };
  EXPECT_EQ(2, rsplits_fnonce([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));

  // split() as lvalues. First split is run.
  auto lsplits_fnonce = [](FnOnceRef<i32()> f) {
    auto split = f.split();
    i32 a = [](FnOnceRef<i32()> f) { return sus::move(f)(); }(f);
    i32 b = [](FnOnceRef<i32()>) {
      // Don't run the `FnOnceRef` as only one of the splits may run the
      // FnOnceRef.
      return 0_i32;
    }(f);
    return a + b;
  };
  EXPECT_EQ(2, lsplits_fnonce([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));

  // split() as lvalues. Second split is run.
  auto lsplits_fnonce2 = [](FnOnceRef<i32()> f) {
    auto split = f.split();
    i32 a = [](FnOnceRef<i32()>) {
      // Don't run the `FnOnceRef` as only one of the splits may run the
      // FnOnceRef.
      return 0_i32;
    }(f);
    i32 b = [](FnOnceRef<i32()> f) { return sus::move(f)(); }(f);
    return a + b;
  };
  EXPECT_EQ(2, lsplits_fnonce([i = 1_i32]() mutable {
              i += 1;
              return i;
            }));
}

struct C {
  i32 method(i32 p) const& { return p + 1; }
  i32 method(i32 p) & { return p + 2; }
  i32 method(i32 p) && { return p + 3; }

  i32 simple() const { return 99; }
};

TEST(FnOnceRef, Methods) {
  {
    auto test_simple = [](FnOnceRef<i32(const C&)> y) {
      const C c;
      return call_once(sus::move(y), c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnOnceRef<i32(C&)> y) {
      C c;
      return call_once(sus::move(y), c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnOnceRef<i32(C&&)> y) {
      return call_once(sus::move(y), C());
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }

  // Overloaded methods.
  {
    auto test_const = [](FnOnceRef<i32(const C&, i32)> y) {
      return call_once(sus::move(y), C(), 10_i32);
    };
    EXPECT_EQ(  //
        test_const(static_cast<i32 (C::*)(i32) const&>(&C::method)), 10 + 1);

    auto test_mut = [](FnOnceRef<i32(C&, i32)> y) {
      C c;
      return call_once(sus::move(y), c, 10_i32);
    };
    EXPECT_EQ(  //
        test_mut(static_cast<i32 (C::*)(i32)&>(&C::method)), 10 + 2);

    auto test_rvalue = [](FnOnceRef<i32(C&&, i32)> y) {
      C c;
      return call_once(sus::move(y), sus::move(c), 10_i32);
    };
    EXPECT_EQ(  //
        test_rvalue(static_cast<i32 (C::*)(i32) &&>(&C::method)), 10 + 3);
  }
}

TEST(FnMutRef, Methods) {
  {
    auto test_simple = [](FnMutRef<i32(const C&)> y) {
      const C c;
      return call_mut(y, c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnMutRef<i32(C&)> y) {
      C c;
      return call_mut(y, c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnMutRef<i32(C&&)> y) { return call_mut(y, C()); };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }

  // Overloaded methods.
  {
    auto test_const = [](FnMutRef<i32(const C&, i32)> y) {
      return call_mut(y, C(), 10_i32);
    };
    EXPECT_EQ(  //
        test_const(static_cast<i32 (C::*)(i32) const&>(&C::method)), 10 + 1);

    auto test_mut = [](FnMutRef<i32(C&, i32)> y) {
      C c;
      return call_mut(y, c, 10_i32);
    };
    EXPECT_EQ(  //
        test_mut(static_cast<i32 (C::*)(i32)&>(&C::method)), 10 + 2);

    auto test_rvalue = [](FnMutRef<i32(C&&, i32)> y) {
      C c;
      return call_mut(y, sus::move(c), 10_i32);
    };
    EXPECT_EQ(  //
        test_rvalue(static_cast<i32 (C::*)(i32) &&>(&C::method)), 10 + 3);
  }
}

TEST(FnRef, Methods) {
  {
    auto test_simple = [](FnRef<i32(const C&)> y) {
      const C c;
      return call(y, c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnRef<i32(C&)> y) {
      C c;
      return call(y, c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnRef<i32(C&&)> y) { return call(y, C()); };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }

  // Overloaded methods.
  {
    auto test_const = [](FnRef<i32(const C&, i32)> y) {
      return call(y, C(), 10_i32);
    };
    EXPECT_EQ(  //
        test_const(static_cast<i32 (C::*)(i32) const&>(&C::method)), 10 + 1);

    auto test_mut = [](FnRef<i32(C&, i32)> y) {
      C c;
      return call(y, c, 10_i32);
    };
    EXPECT_EQ(  //
        test_mut(static_cast<i32 (C::*)(i32)&>(&C::method)), 10 + 2);

    auto test_rvalue = [](FnRef<i32(C&&, i32)> y) {
      C c;
      return call(y, sus::move(c), 10_i32);
    };
    EXPECT_EQ(  //
        test_rvalue(static_cast<i32 (C::*)(i32) &&>(&C::method)), 10 + 3);
  }
}

}  // namespace
