// Copyright 2023 Google LLC
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

#include "sus/fn/fn_concepts.h"

#include <concepts>

#include "googletest/include/gtest/gtest.h"
#include "sus/mem/move.h"
#include "sus/prelude.h"
#include "sus/tuple/tuple.h"

namespace {

using namespace sus::fn;

struct Parent {};
struct Child : public Parent {};

struct MoveOnly {
  MoveOnly() = default;
  ~MoveOnly() = default;
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;
};

// Function pointer from lambda.
using FVoid = decltype(+[]() {});
static_assert(Fn<FVoid, void()>);
static_assert(FnMut<FVoid, void()>);
static_assert(FnOnce<FVoid, void()>);
static_assert(!Fn<FVoid, int()>);
static_assert(!FnMut<FVoid, int()>);
static_assert(!FnOnce<FVoid, int()>);
using FVoidIntRvalue = decltype(+[](int&&) {});
static_assert(Fn<FVoidIntRvalue, void(int&&)>);
static_assert(FnMut<FVoidIntRvalue, void(int&&)>);
static_assert(FnOnce<FVoidIntRvalue, void(int&&)>);
using FVoidMoveOnlyRvalue = decltype(+[](MoveOnly&&) {});
static_assert(Fn<FVoidMoveOnlyRvalue, void(MoveOnly&&)>);
static_assert(FnMut<FVoidMoveOnlyRvalue, void(MoveOnly&&)>);
static_assert(FnOnce<FVoidMoveOnlyRvalue, void(MoveOnly&&)>);
// Passing it an rvalue but receiving by value is compatible.
using FVoidIntValue = decltype(+[](int) {});
static_assert(Fn<FVoidIntValue, void(int&&)>);
static_assert(FnMut<FVoidIntValue, void(int&&)>);
static_assert(FnOnce<FVoidIntValue, void(int&&)>);
using FVoidMoveOnlyValue = decltype(+[](MoveOnly) {});
static_assert(Fn<FVoidMoveOnlyValue, void(MoveOnly&&)>);
static_assert(FnMut<FVoidMoveOnlyValue, void(MoveOnly&&)>);
static_assert(FnOnce<FVoidMoveOnlyValue, void(MoveOnly&&)>);
// Copy from an lvalue if copyable.
static_assert(Fn<FVoidIntValue, void(int&)>);
static_assert(FnMut<FVoidIntValue, void(int&)>);
static_assert(FnOnce<FVoidIntValue, void(int&)>);
static_assert(!Fn<FVoidMoveOnlyValue, void(MoveOnly&)>);
static_assert(!FnMut<FVoidMoveOnlyValue, void(MoveOnly&)>);
static_assert(!FnOnce<FVoidMoveOnlyValue, void(MoveOnly&)>);
// Move from an rvalue if moveable.
static_assert(Fn<FVoidIntValue, void(int)>);
static_assert(FnMut<FVoidIntValue, void(int)>);
static_assert(FnOnce<FVoidIntValue, void(int)>);
static_assert(Fn<FVoidMoveOnlyValue, void(MoveOnly)>);
static_assert(FnMut<FVoidMoveOnlyValue, void(MoveOnly)>);
static_assert(FnOnce<FVoidMoveOnlyValue, void(MoveOnly)>);
// Convertible argument and return types.
using FChildParent = decltype(+[](Parent*) -> Child* { return nullptr; });
static_assert(Fn<FChildParent, Parent*(Child*)>);
static_assert(FnMut<FChildParent, Parent*(Child*)>);
static_assert(FnOnce<FChildParent, Parent*(Child*)>);
// non-convertible argument or return types.
using FParentChild = decltype(+[](Child*) -> Parent* { return nullptr; });
static_assert(!Fn<FParentChild, Parent*(Parent*)>);
static_assert(!FnMut<FParentChild, Parent*(Parent*)>);
static_assert(!FnOnce<FParentChild, Parent*(Parent*)>);
static_assert(!Fn<FParentChild, Child*(Child*)>);
static_assert(!FnMut<FParentChild, Child*(Child*)>);
static_assert(!FnOnce<FParentChild, Child*(Child*)>);
static_assert(!Fn<FParentChild, Child*(Parent*)>);
static_assert(!FnMut<FParentChild, Child*(Parent*)>);
static_assert(!FnOnce<FParentChild, Child*(Parent*)>);
using Fi32i32 = decltype(+[](i32) -> i32 { return 0; });
static_assert(Fn<Fi32i32, i32(i32)>);
static_assert(FnMut<Fi32i32, i32(i32)>);
static_assert(FnOnce<Fi32i32, i32(i32)>);

// Any return type.
static_assert(Fn<FVoid, Anything()>);
static_assert(Fn<FVoidIntRvalue, Anything(int)>);
static_assert(Fn<FVoidIntRvalue, Anything(int)>);
static_assert(Fn<FChildParent, Anything(Child*)>);
static_assert(!Fn<FChildParent, Anything(int)>);  // Incompatible argument.
static_assert(FnMut<FVoid, Anything()>);
static_assert(FnMut<FVoidIntRvalue, Anything(int)>);
static_assert(FnMut<FVoidIntRvalue, Anything(int)>);
static_assert(FnMut<FChildParent, Anything(Child*)>);
static_assert(!FnMut<FChildParent, Anything(int)>);  // Incompatible argument.
static_assert(FnOnce<FVoid, Anything()>);
static_assert(FnOnce<FVoidIntRvalue, Anything(int)>);
static_assert(FnOnce<FVoidIntRvalue, Anything(int)>);
static_assert(FnOnce<FChildParent, Anything(Child*)>);
static_assert(!FnOnce<FChildParent, Anything(int)>);  // Incompatible argument.

// Non-void return type.
static_assert(!Fn<FVoid, NonVoid()>);              // Void return.
static_assert(!Fn<FVoidIntRvalue, NonVoid(int)>);  // Void return.
static_assert(Fn<FChildParent, NonVoid(Child*)>);
static_assert(!Fn<FChildParent, NonVoid(int)>);       // Incompatible argument.
static_assert(!FnMut<FVoid, NonVoid()>);              // Void return.
static_assert(!FnMut<FVoidIntRvalue, NonVoid(int)>);  // Void return.
static_assert(FnMut<FChildParent, NonVoid(Child*)>);
static_assert(!FnMut<FChildParent, NonVoid(int)>);     // Incompatible argument.
static_assert(!FnOnce<FVoid, NonVoid()>);              // Void return.
static_assert(!FnOnce<FVoidIntRvalue, NonVoid(int)>);  // Void return.
static_assert(FnOnce<FChildParent, NonVoid(Child*)>);
static_assert(!FnOnce<FChildParent, NonVoid(int)>);  // Incompatible argument.

// There's no implementation difference for lambdas compared to function
// pointers, so we just verify that they also satisfy the concepts.

// Actual pointer.
static_assert(Fn<void (*)(), void()>);
static_assert(FnMut<void (*)(), void()>);
static_assert(FnOnce<void (*)(), void()>);
static_assert(!Fn<void (*)(), int()>);
static_assert(!FnMut<void (*)(), int()>);
static_assert(!FnOnce<void (*)(), int()>);

// Captureless lambda.
using CapturelessChildParent =
    decltype([](Parent*) -> Child* { return nullptr; });
static_assert(Fn<CapturelessChildParent, Parent*(Child*)>);
static_assert(FnMut<CapturelessChildParent, Parent*(Child*)>);
static_assert(FnOnce<CapturelessChildParent, Parent*(Child*)>);

// Capturing lambda.
using CapturingChildParent =
    decltype([i = 1](Parent*) -> Child* { return nullptr; });
static_assert(Fn<CapturingChildParent, Parent*(Child*)>);
static_assert(FnMut<CapturingChildParent, Parent*(Child*)>);
static_assert(FnOnce<CapturingChildParent, Parent*(Child*)>);

// Mutable lambda. Note that Fn is never satisfied by a mutable lambda.
using MutableChildParent =
    decltype([i = 1](Parent*) mutable -> Child* { return i++, nullptr; });
static_assert(!Fn<MutableChildParent, Parent*(Child*)>);
static_assert(FnMut<MutableChildParent, Parent*(Child*)>);
static_assert(FnOnce<MutableChildParent, Parent*(Child*)>);

// Accepts any type that can be called once with (Option<i32>) and returns
// i32.
i32 do_stuff_once(sus::fn::FnOnce<i32(sus::Option<i32>)> auto&& f) {
  return sus::fn::call_once(sus::move(f), sus::some(400));
}

TEST(FnConcepts, FnOnceExample) {
  i32 x = do_stuff_once([](sus::Option<i32> o) -> i32 {
    return sus::move(o).unwrap_or_default() + 4;
  });
  sus::check(x == 400 + 4);
}

// Accepts any type that can be called once with (Option<i32>) and returns
// i32.
static i32 do_stuff_mut(sus::fn::FnMut<i32(sus::Option<i32>)> auto&& f) {
  return sus::fn::call_mut(f, sus::some(400)) +
         sus::fn::call_mut(f, sus::some(100));
}

TEST(FnConcepts, FnMutExample) {
  i32 x = do_stuff_mut([i = 0_i32](sus::Option<i32> o) mutable -> i32 {
    i += 1;
    return sus::move(o).unwrap_or_default() + i;
  });
  sus::check(x == 401 + 102);
}

// Accepts any type that can be called once with (Option<i32>) and returns
// i32.
static i32 do_stuff(const sus::fn::Fn<i32(sus::Option<i32>)> auto& f) {
  return sus::fn::call(f, sus::some(400)) + sus::fn::call(f, sus::some(100));
}

TEST(FnConcepts, FnExample) {
  i32 x = do_stuff([i = 1_i32](sus::Option<i32> o) -> i32 {
    return sus::move(o).unwrap_or_default() + i;
  });
  sus::check(x == 401 + 101);
}

struct S {
  static i32 fn_once(FnOnce<i32(i32)> auto&& f) { return f(2); }
  static i32 fn_mut(FnMut<i32(i32)> auto f) { return fn_once(sus::move(f)); }
  static i32 fn(const Fn<i32(i32)> auto& f) { return fn_mut(f); }
};

template <class... Ts>
struct Pack;

struct NoOverloadMatchesArguments {};

template <class R, class... Args>
struct Signature;

template <class R, class... Args>
struct Signature<R(Args...)> {
  using ReturnType = R;
  using ArgsType = Pack<Args...>;
};

template <class F, class R, class ArgsPack>
struct ActualReturnType {
  using type = NoOverloadMatchesArguments;
};

template <class F, class R, class... Ts>
  requires requires(const F& f) {
    { f(std::declval<Ts>()...) };
  }
struct ActualReturnType<F, R, Pack<Ts...>> {
  using type = decltype(std::declval<const F&>()(std::declval<Ts>()...));
};

template <class F, class S>
concept FnSig = requires(const F& f, Signature<S>::ArgsType&& args) {
  requires std::convertible_to<
      typename ActualReturnType<F, typename Signature<S>::ReturnType,
                                typename Signature<S>::ArgsType>::type,
      typename Signature<S>::ReturnType>;
};

TEST(FnConcepts, Convertible) {
  auto x = [](i32 i) -> i32 { return i * 2 + 1; };
  EXPECT_EQ(5, S::fn_once([](i32 i) -> i32 { return i * 2 + 1; }));
  EXPECT_EQ(5, S::fn_mut([](i32 i) -> i32 { return i * 2 + 1; }));
  EXPECT_EQ(5, S::fn([](i32 i) -> i32 { return i * 2 + 1; }));
}

struct R {
  static i32 fn_mut_v(FnMut<i32(i32)> auto f) { return f(2); }
  static i32 fn_mut_l(FnMut<i32(i32)> auto& f) { return f(2); }
  static i32 fn_mut_r(FnMut<i32(i32)> auto&& f) { return f(2); }
};

TEST(FnConcepts, FnMutPassByReference) {
  auto x = [j = 0_i32](i32 i) mutable {
    j += 1;
    return j + i;
  };
  EXPECT_EQ(3, R::fn_mut_v(x));  // By value, `x` is not mutated locally.
  EXPECT_EQ(3, R::fn_mut_l(x));  // By reference, `x` is mutated locally.
  EXPECT_EQ(4, R::fn_mut_r(x));  // By reference, `x` is mutated locally.
  EXPECT_EQ(5, R::fn_mut_v(x));

  // Verify by value and by rvalue ref can receive an rvalue type.
  EXPECT_EQ(3, R::fn_mut_v([](i32 i) { return i + 1; }));
  EXPECT_EQ(3, R::fn_mut_r([](i32 i) { return i + 1; }));
}

struct C {
  i32 method(i32 p) const& { return p + 1; }
  i32 method(i32 p) & { return p + 2; }
  i32 method(i32 p) && { return p + 3; }

  i32 simple() const { return 99; }
  i32 simple_mut() { return 99; }
};

// Function that receives a const C& can be satisfied by a const C method, but
// not a mutable one.
static_assert(FnOnce<decltype(&C::simple), i32(const C&)>);
static_assert(!FnOnce<decltype(&C::simple_mut), i32(const C&)>);
static_assert(FnMut<decltype(&C::simple), i32(const C&)>);
static_assert(!FnMut<decltype(&C::simple_mut), i32(const C&)>);
static_assert(Fn<decltype(&C::simple), i32(const C&)>);
static_assert(!Fn<decltype(&C::simple_mut), i32(const C&)>);

TEST(FnOnce, Methods) {
  {
    auto test_simple = [](FnOnce<i32(const C&)> auto&& y) {
      const C c;
      return call_once(sus::move(y), c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnOnce<i32(C&)> auto&& y) {
      C c;
      return call_once(sus::move(y), c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnOnce<i32(C&&)> auto&& y) {
      return call_once(sus::move(y), C());
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }

  // Overloaded methods.
  {
    auto test_const = [](FnOnce<i32(const C&, i32)> auto y) {
      return call_once(sus::move(y), C(), 10_i32);
    };
    EXPECT_EQ(  //
        test_const(static_cast<i32 (C::*)(i32) const&>(&C::method)), 10 + 1);

    auto test_mut = [](FnOnce<i32(C&, i32)> auto y) {
      C c;
      return call_once(sus::move(y), c, 10_i32);
    };
    EXPECT_EQ(  //
        test_mut(static_cast<i32 (C::*)(i32)&>(&C::method)), 10 + 2);

    auto test_rvalue = [](FnOnce<i32(C&&, i32)> auto y) {
      C c;
      return call_once(sus::move(y), sus::move(c), 10_i32);
    };
    EXPECT_EQ(  //
        test_rvalue(static_cast<i32 (C::*)(i32) &&>(&C::method)), 10 + 3);
  }
}

TEST(FnMut, Methods) {
  {
    auto test_simple = [](FnMut<i32(const C&)> auto&& y) {
      const C c;
      return call_mut(y, c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnMut<i32(C&)> auto&& y) {
      C c;
      return call_mut(y, c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](FnMut<i32(C&&)> auto&& y) {
      return call_mut(y, C());
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }

  // Overloaded methods.
  {
    auto test_const = [](FnMut<i32(const C&, i32)> auto&& y) {
      return call_mut(y, C(), 10_i32);
    };
    EXPECT_EQ(  //
        test_const(static_cast<i32 (C::*)(i32) const&>(&C::method)), 10 + 1);

    auto test_mut = [](FnMut<i32(C&, i32)> auto&& y) {
      C c;
      return call_mut(y, c, 10_i32);
    };
    EXPECT_EQ(  //
        test_mut(static_cast<i32 (C::*)(i32)&>(&C::method)), 10 + 2);

    auto test_rvalue = [](FnMut<i32(C&&, i32)> auto&& y) {
      C c;
      return call_mut(y, sus::move(c), 10_i32);
    };
    EXPECT_EQ(  //
        test_rvalue(static_cast<i32 (C::*)(i32) &&>(&C::method)), 10 + 3);
  }
}

TEST(Fn, Methods) {
  {
    auto test_simple = [](Fn<i32(const C&)> auto const& y) {
      const C c;
      return call(y, c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](Fn<i32(C&)> auto const& y) {
      C c;
      return call(y, c);
    };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }
  {
    auto test_simple = [](Fn<i32(C&&)> auto const& y) { return call(y, C()); };
    EXPECT_EQ(test_simple(&C::simple), 99);
  }

  // Overloaded methods.
  {
    auto test_const = [](Fn<i32(const C&, i32)> auto const& y) {
      return call(y, C(), 10_i32);
    };
    EXPECT_EQ(  //
        test_const(static_cast<i32 (C::*)(i32) const&>(&C::method)), 10 + 1);

    auto test_mut = [](Fn<i32(C&, i32)> auto const& y) {
      C c;
      return call(y, c, 10_i32);
    };
    EXPECT_EQ(  //
        test_mut(static_cast<i32 (C::*)(i32)&>(&C::method)), 10 + 2);

    auto test_rvalue = [](Fn<i32(C&&, i32)> auto const& y) {
      C c;
      return call(y, sus::move(c), 10_i32);
    };
    EXPECT_EQ(  //
        test_rvalue(static_cast<i32 (C::*)(i32) &&>(&C::method)), 10 + 3);
  }
}

struct Class {
  Class(i32 value) : value_(value) {}
  i32 value() const { return value_; }

 private:
  i32 value_;
};

i32 map_class_once(const Class& c,
                   sus::fn::FnOnce<i32(const Class&)> auto&& f) {
  return sus::fn::call_once(sus::move(f), c);
}

i32 map_class_mut(const Class& c, sus::fn::FnMut<i32(const Class&)> auto&& f) {
  return sus::fn::call_mut(f, c);
}

i32 map_class(const Class& c, sus::fn::Fn<i32(const Class&)> auto const& f) {
  return sus::fn::call(f, c);
}

i32 map_fn(const Class& c) { return c.value(); }

TEST(FnConcepts, ExampleFunction) {
  // Map the class C to its value().
  auto c = Class(42);
  sus::check(map_class_once(c, &map_fn) == 42);
  sus::check(map_class_mut(c, &map_fn) == 42);
  sus::check(map_class(c, &map_fn) == 42);

  auto o = sus::Option<Class>::with(Class(42));
  sus::check(o.map(&map_fn) == sus::some(42));
}

TEST(FnConcepts, Example_Method) {
  // Map the class C to its value().
  auto c = Class(42);
  sus::check(map_class_once(c, &Class::value) == 42);
  sus::check(map_class_mut(c, &Class::value) == 42);
  sus::check(map_class(c, &Class::value) == 42);

  auto o = sus::Option<Class>::with(Class(42));
  sus::check(o.map(&Class::value) == sus::some(42));
}

}  // namespace
