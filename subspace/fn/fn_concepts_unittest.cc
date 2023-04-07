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

#include "subspace/fn/fn_concepts.h"

#include <concepts>

#include "googletest/include/gtest/gtest.h"
#include "subspace/mem/move.h"
#include "subspace/prelude.h"
#include "subspace/tuple/tuple.h"

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
i32 call_once(sus::fn::FnOnce<i32(sus::Option<i32>)> auto&& f) {
  return sus::move(f)(sus::some(400));  // Returns an i32.
}

TEST(FnConcepts, FnOnceExample) {
  i32 x = call_once([](sus::Option<i32> o) -> i32 {
    return sus::move(o).unwrap_or_default() + 4;
  });
  sus::check(x == 400 + 4);
}

// Accepts any type that can be called once with (Option<i32>) and returns
// i32.
static i32 call_mut(sus::fn::FnMut<i32(sus::Option<i32>)> auto&& f) {
  return f(sus::some(400)) + f(sus::some(100));  // Returns an i32.
}

TEST(FnConcepts, FnMutExample) {
  i32 x = call_mut([i = 0_i32](sus::Option<i32> o) mutable -> i32 {
    i += 1;
    return sus::move(o).unwrap_or_default() + i;
  });
  sus::check(x == 401 + 102);
}

// Accepts any type that can be called once with (Option<i32>) and returns
// i32.
static i32 call_fn(const sus::fn::Fn<i32(sus::Option<i32>)> auto& f) {
  return f(sus::some(400)) + f(sus::some(100));  // Returns an i32.
}

TEST(FnConcepts, FnExample) {
  i32 x = call_fn([i = 1_i32](sus::Option<i32> o) -> i32 {
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

}  // namespace
