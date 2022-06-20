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

struct CopyOnly {
  CopyOnly(int i) : i(i) {}
  CopyOnly(const CopyOnly& c) = default;
  CopyOnly& operator=(const CopyOnly& c) = default;
  CopyOnly(CopyOnly&& c) = delete;
  CopyOnly& operator=(CopyOnly&& c) = delete;
  ~CopyOnly() { i = -10000000; }

  int i = 0;
};

struct MoveOnly {
  MoveOnly(int i) : i(i) {}
  MoveOnly(const MoveOnly& c) = delete;
  MoveOnly& operator=(const MoveOnly& c) = delete;
  MoveOnly(MoveOnly&& c) = default;
  MoveOnly& operator=(MoveOnly&& c) = default;
  ~MoveOnly() { i = -10000000; }

  int i = 0;
};

template <class Fn, class Functor, class... StoredArgs>
concept has_with_fn_pointer = requires(Functor f, StoredArgs... args) {
                                { Fn::with_fn_pointer(f, args...) };
                              };
template <class Fn, class Functor, class... StoredArgs>
concept has_with_storage = requires(Functor f, StoredArgs... args) {
                             { Fn::with_storage(f, args...) };
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
static_assert(has_with_fn_pointer<FnOnce<void()>, VVFunction>);
static_assert(has_with_fn_pointer<FnOnce<void()>, VVLambda>);
static_assert(has_with_fn_pointer<FnMut<void()>, VVFunction>);
static_assert(has_with_fn_pointer<FnMut<void()>, VVLambda>);
static_assert(has_with_fn_pointer<Fn<void()>, VVFunction>);
static_assert(has_with_fn_pointer<Fn<void()>, VVLambda>);
// Non-void arguments with a function pointer.
static_assert(has_with_fn_pointer<FnOnce<int(float)>, IFFunction>);
static_assert(has_with_fn_pointer<FnOnce<int(float)>, IFLambda>);
static_assert(has_with_fn_pointer<FnMut<int(float)>, IFFunction>);
static_assert(has_with_fn_pointer<FnMut<int(float)>, IFLambda>);
static_assert(has_with_fn_pointer<Fn<int(float)>, IFFunction>);
static_assert(has_with_fn_pointer<Fn<int(float)>, IFLambda>);
// Stored arguments can't be passed for a function pointer.
static_assert(!has_with_fn_pointer<FnOnce<int(float)>, IFFunction, float>);
static_assert(!has_with_fn_pointer<FnMut<int(float)>, IFFunction, float>);
static_assert(!has_with_fn_pointer<Fn<int(float)>, IFFunction, float>);
// A lambda with bound args can't be passed as a function pointer.
static_assert(!has_with_fn_pointer<FnOnce<void()>, VVLambdaBound>);
static_assert(!has_with_fn_pointer<FnMut<void()>, VVLambdaBound>);
static_assert(!has_with_fn_pointer<Fn<void()>, VVLambdaBound>);
static_assert(!has_with_fn_pointer<FnOnce<void()>, VVLambdaBoundMut>);
static_assert(!has_with_fn_pointer<FnMut<void()>, VVLambdaBoundMut>);
static_assert(!has_with_fn_pointer<Fn<void()>, VVLambdaBoundMut>);

// Storage can be requested even if it's not used.
static_assert(has_with_storage<FnOnce<void()>, VVFunction>);
static_assert(has_with_storage<FnOnce<void()>, VVLambda>);
static_assert(has_with_storage<FnMut<void()>, VVFunction>);
static_assert(has_with_storage<FnMut<void()>, VVLambda>);
static_assert(has_with_storage<Fn<void()>, VVFunction>);
static_assert(has_with_storage<Fn<void()>, VVLambda>);
// With storage, lambdas with bounds can be used.
static_assert(has_with_storage<FnOnce<void()>, VVLambdaBound>);
static_assert(has_with_storage<FnMut<void()>, VVLambdaBound>);
static_assert(has_with_storage<Fn<void()>, VVLambdaBound>);
static_assert(has_with_storage<FnOnce<void()>, VVLambdaBoundMut>);
static_assert(has_with_storage<FnMut<void()>, VVLambdaBoundMut>);
// But a mutable lambda can't be stored in a (const) Fn.
static_assert(!has_with_storage<Fn<void()>, VVLambdaBoundMut>);
// Stored variables, without call variables. They must match in number.
static_assert(has_with_storage<FnOnce<void()>, decltype([](int) {}), int>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](int, int) {}), int, int>);
static_assert(!has_with_storage<FnOnce<void()>, decltype([](int) {}), int, int>);
static_assert(!has_with_storage<FnOnce<void()>, decltype([](int, int) {}), int>);
static_assert(has_with_storage<FnMut<void()>, decltype([](int) {}), int>);
static_assert(has_with_storage<FnMut<void()>, decltype([](int, int) {}), int, int>);
static_assert(!has_with_storage<FnMut<void()>, decltype([](int) {}), int, int>);
static_assert(!has_with_storage<FnMut<void()>, decltype([](int, int) {}), int>);
static_assert(has_with_storage<Fn<void()>, decltype([](int) {}), int>);
static_assert(has_with_storage<Fn<void()>, decltype([](int, int) {}), int, int>);
static_assert(!has_with_storage<Fn<void()>, decltype([](int) {}), int, int>);
static_assert(!has_with_storage<Fn<void()>, decltype([](int, int) {}), int>);
// Stored variables, with call variables. They must match in number.
static_assert(has_with_storage<FnOnce<void(int)>, decltype([](int, int) {}), int>);
static_assert(has_with_storage<FnOnce<void(int)>, decltype([](int, int, int) {}), int, int>);
static_assert(has_with_storage<FnOnce<void(int, int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with_storage<FnOnce<void(int)>, decltype([](int, int) {}), int, int>);
static_assert(!has_with_storage<FnOnce<void(int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with_storage<FnOnce<void(int, int)>, decltype([](int, int) {}), int>);
static_assert(has_with_storage<FnMut<void(int)>, decltype([](int, int) {}), int>);
static_assert(has_with_storage<FnMut<void(int)>, decltype([](int, int, int) {}), int, int>);
static_assert(has_with_storage<FnMut<void(int, int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with_storage<FnMut<void(int)>, decltype([](int, int) {}), int, int>);
static_assert(!has_with_storage<FnMut<void(int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with_storage<FnMut<void(int, int)>, decltype([](int, int) {}), int>);
static_assert(has_with_storage<Fn<void(int)>, decltype([](int, int) {}), int>);
static_assert(has_with_storage<Fn<void(int)>, decltype([](int, int, int) {}), int, int>);
static_assert(has_with_storage<Fn<void(int, int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with_storage<Fn<void(int)>, decltype([](int, int) {}), int, int>);
static_assert(!has_with_storage<Fn<void(int)>, decltype([](int, int, int) {}), int>);
static_assert(!has_with_storage<Fn<void(int, int)>, decltype([](int, int) {}), int>);
// FnOnce moves its storage to the receiver function. It must be able to receive
// that, but we allow it to receive it as a mutable reference as well. In
// practice, that means it can receive in any way it likes.
static_assert(has_with_storage<FnOnce<void()>, decltype([](const MoveOnly&) {}), MoveOnly>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](MoveOnly&) {}), MoveOnly>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](MoveOnly&&) {}), MoveOnly>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](MoveOnly) {}), MoveOnly>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](const MoveOnly) {}), MoveOnly>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](const CopyOnly&) {}), CopyOnly>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](CopyOnly&) {}), CopyOnly>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](CopyOnly&&) {}), CopyOnly>);
static_assert(!has_with_storage<FnOnce<void()>, decltype([](CopyOnly) {}), CopyOnly>);
static_assert(!has_with_storage<FnOnce<void()>, decltype([](const CopyOnly) {}), CopyOnly>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](const Copyable&) {}), Copyable>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](Copyable&) {}), Copyable>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](Copyable&&) {}), Copyable>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](Copyable) {}), Copyable>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](const Copyable) {}), Copyable>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](const int&) {}), int>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](int&) {}), int>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](int&&) {}), int>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](int) {}), int>);
static_assert(has_with_storage<FnOnce<void()>, decltype([](const int) {}), int>);
// FnMut passes a mutable reference to its storage to the receiver function. It
// must be able to receive that, which means it must be receiving a const or
// mutable lvalue reference, or it will try produce a copy.
static_assert(has_with_storage<FnMut<void()>, decltype([](const MoveOnly&) {}), MoveOnly>);
static_assert(has_with_storage<FnMut<void()>, decltype([](MoveOnly&) {}), MoveOnly>);
static_assert(!has_with_storage<FnMut<void()>, decltype([](MoveOnly&&) {}), MoveOnly>);
static_assert(!has_with_storage<FnMut<void()>, decltype([](MoveOnly) {}), MoveOnly>);
static_assert(!has_with_storage<FnMut<void()>, decltype([](const MoveOnly) {}), MoveOnly>);
static_assert(has_with_storage<FnMut<void()>, decltype([](const CopyOnly&) {}), CopyOnly>);
static_assert(has_with_storage<FnMut<void()>, decltype([](CopyOnly&) {}), CopyOnly>);
static_assert(!has_with_storage<FnMut<void()>, decltype([](CopyOnly&&) {}), CopyOnly>);
static_assert(has_with_storage<FnMut<void()>, decltype([](CopyOnly) {}), CopyOnly>);
static_assert(has_with_storage<FnMut<void()>, decltype([](const CopyOnly) {}), CopyOnly>);
static_assert(has_with_storage<FnMut<void()>, decltype([](const Copyable&) {}), Copyable>);
static_assert(has_with_storage<FnMut<void()>, decltype([](Copyable&) {}), Copyable>);
static_assert(!has_with_storage<FnMut<void()>, decltype([](Copyable&&) {}), Copyable>);
static_assert(has_with_storage<FnMut<void()>, decltype([](Copyable) {}), Copyable>);
static_assert(has_with_storage<FnMut<void()>, decltype([](const Copyable) {}), Copyable>);
static_assert(has_with_storage<FnMut<void()>, decltype([](const int&) {}), int>);
static_assert(has_with_storage<FnMut<void()>, decltype([](int&) {}), int>);
static_assert(!has_with_storage<FnMut<void()>, decltype([](int&&) {}), int>);
static_assert(has_with_storage<FnMut<void()>, decltype([](int) {}), int>);
static_assert(has_with_storage<FnMut<void()>, decltype([](const int) {}), int>);
// Fn passes a const reference to its storage to the receiver function. It must
// be able to receive that, which means it must be receiving a const reference,
// or it will try produce a copy.
static_assert(has_with_storage<Fn<void()>, decltype([](const MoveOnly&) {}), MoveOnly>);
static_assert(!has_with_storage<Fn<void()>, decltype([](MoveOnly&) {}), MoveOnly>);
static_assert(!has_with_storage<Fn<void()>, decltype([](MoveOnly&&) {}), MoveOnly>);
static_assert(!has_with_storage<Fn<void()>, decltype([](MoveOnly) {}), MoveOnly>);
static_assert(!has_with_storage<Fn<void()>, decltype([](const MoveOnly) {}), MoveOnly>);
static_assert(has_with_storage<Fn<void()>, decltype([](const CopyOnly&) {}), CopyOnly>);
static_assert(!has_with_storage<Fn<void()>, decltype([](CopyOnly&) {}), CopyOnly>);
static_assert(!has_with_storage<Fn<void()>, decltype([](CopyOnly&&) {}), CopyOnly>);
static_assert(has_with_storage<Fn<void()>, decltype([](CopyOnly) {}), CopyOnly>);
static_assert(has_with_storage<Fn<void()>, decltype([](const CopyOnly) {}), CopyOnly>);
static_assert(has_with_storage<Fn<void()>, decltype([](const Copyable&) {}), Copyable>);
static_assert(!has_with_storage<Fn<void()>, decltype([](Copyable&) {}), Copyable>);
static_assert(!has_with_storage<Fn<void()>, decltype([](Copyable&&) {}), Copyable>);
static_assert(has_with_storage<Fn<void()>, decltype([](Copyable) {}), Copyable>);
static_assert(has_with_storage<Fn<void()>, decltype([](const Copyable) {}), Copyable>);
static_assert(has_with_storage<Fn<void()>, decltype([](const int&) {}), int>);
static_assert(!has_with_storage<Fn<void()>, decltype([](int&) {}), int>);
static_assert(!has_with_storage<Fn<void()>, decltype([](int&&) {}), int>);
static_assert(has_with_storage<Fn<void()>, decltype([](int) {}), int>);
static_assert(has_with_storage<Fn<void()>, decltype([](const int) {}), int>);
// The receiver of the args passed to call/call_mut/call_once must be compatible
// with the specified type in the Fn/FnMut/FnOnce.
static_assert(has_with_storage<FnOnce<void(MoveOnly)>, decltype([](MoveOnly) {})>);
static_assert(has_with_storage<FnOnce<void(MoveOnly)>, decltype([](const MoveOnly&) {})>);
static_assert(!has_with_storage<FnOnce<void(MoveOnly)>, decltype([](MoveOnly&) {})>);
static_assert(has_with_storage<FnOnce<void(MoveOnly)>, decltype([](MoveOnly&&) {})>);
static_assert(has_with_storage<FnMut<void(MoveOnly)>, decltype([](MoveOnly) {})>);
static_assert(has_with_storage<FnMut<void(MoveOnly)>, decltype([](const MoveOnly&) {})>);
static_assert(!has_with_storage<FnMut<void(MoveOnly)>, decltype([](MoveOnly&) {})>);
static_assert(has_with_storage<FnMut<void(MoveOnly)>, decltype([](MoveOnly&&) {})>);
static_assert(has_with_storage<Fn<void(MoveOnly)>, decltype([](MoveOnly) {})>);
static_assert(has_with_storage<Fn<void(MoveOnly)>, decltype([](const MoveOnly&) {})>);
static_assert(!has_with_storage<Fn<void(MoveOnly)>, decltype([](MoveOnly&) {})>);
static_assert(has_with_storage<Fn<void(MoveOnly)>, decltype([](MoveOnly&&) {})>);

// clang-format on

TEST(FnOnce, FnPointer) {
  auto fn = FnOnce<int(int, int)>::with_fn_pointer(
      [](int a, int b) { return a * 2 + b; });
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(1, 2), 4);
}

TEST(FnOnce, FnPointerStorage) {
  auto fn =
      FnOnce<int(int)>::with_storage([](int a, int b) { return a * 2 + b; }, 1);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(2), 4);

  auto fn2 =
      FnOnce<int()>::with_storage([](int a, int b) { return a * 2 + b; }, 1, 2);
  EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2).call_once(), 4);
}

TEST(FnOnce, CallWithConstRef) {
  auto fn = FnOnce<const Copyable*(const Copyable&)>::with_fn_pointer(
      [](const Copyable& c) { return &c; });
  auto c = Copyable(3);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(c), &c);
}

TEST(FnOnce, CallWithMutRef) {
  auto fn = FnOnce<int(Copyable&)>::with_fn_pointer(
      [](Copyable& c) { return c.i++; });
  auto copy_only = Copyable(3);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(copy_only), 3);
  EXPECT_EQ(copy_only.i, 4);
}

TEST(FnOnce, CopyableStorage) {
  auto c = Copyable(3);
  auto fn =
      FnOnce<int()>::with_storage([](const Copyable& c) { return c.i; }, c);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 3);
}

TEST(FnOnce, MoveOnlyStorage) {
  auto fn =
      FnOnce<int()>::with_storage([](MoveOnly m) { return m.i; }, MoveOnly(3));
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 3);
}

TEST(FnOnce, CopyAndMoveOnlyStorage) {
  const auto copy_only = Copyable(3);
  auto f = [](const Copyable& c, MoveOnly m) { return c.i * 2 + m.i; };
  // Will copy Copyable into storage, and move MoveOnly.
  auto fn = FnOnce<int()>::with_storage(f, copy_only, MoveOnly(1));
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 3 * 2 + 1);
}

TEST(Fn, FnPointer) {
  auto fn = Fn<int(int, int)>::with_fn_pointer(
      [](int a, int b) { return a * 2 + b; });
  EXPECT_EQ(fn.call(1, 2), 4);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(1, 2), 4);
}

TEST(Fn, FnPointerStorage) {
  auto fn =
      Fn<int(int)>::with_storage([](int a, int b) { return a * 2 + b; }, 1);
  EXPECT_EQ(fn.call(2), 4);
  EXPECT_EQ(fn.call_mut(2), 4);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(2), 4);

  auto fn2 =
      Fn<int()>::with_storage([](int a, int b) { return a * 2 + b; }, 1, 2);
  EXPECT_EQ(fn2.call(), 4);
  EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2).call_once(), 4);
}

TEST(Fn, CallWithConstRef) {
  auto fn = Fn<const Copyable*(const Copyable&)>::with_fn_pointer(
      [](const Copyable& c) { return &c; });
  auto c = Copyable(3);
  EXPECT_EQ(fn.call(c), &c);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(c), &c);
}

TEST(Fn, CallWithMutRef) {
  auto fn =
      Fn<int(Copyable&)>::with_fn_pointer([](Copyable& c) { return c.i++; });
  auto copy_only = Copyable(3);
  EXPECT_EQ(fn.call(copy_only), 3);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(copy_only), 4);
  EXPECT_EQ(copy_only.i, 5);
}

TEST(Fn, CopyableStorage) {
  auto fn = Fn<int()>::with_storage([](const Copyable& c) { return c.i; },
                                    static_cast<const Copyable&>(Copyable(3)));
  EXPECT_EQ(fn.call(), 3);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 3);
}

TEST(Fn, MoveOnlyStorage) {
  auto fn = Fn<int()>::with_storage([](const MoveOnly& m) { return m.i; },
                                    MoveOnly(3));
  EXPECT_EQ(fn.call(), 3);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 3);
}

TEST(Fn, CopyAndMoveOnlyStorage) {
  const auto copy_only = Copyable(3);
  auto f = [](const Copyable& c, const MoveOnly& m) { return c.i * 2 + m.i; };
  // Will copy Copyable into storage, and move MoveOnly.
  auto fn = Fn<int()>::with_storage(f, copy_only, MoveOnly(1));
  EXPECT_EQ(fn.call(), 3 * 2 + 1);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 3 * 2 + 1);
}

TEST(FnMut, FnPointer) {
  auto fn = FnMut<int(int, int)>::with_fn_pointer(
      [](int a, int b) { return a * 2 + b; });
  EXPECT_EQ(fn.call_mut(1, 2), 4);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(1, 2), 4);
}

TEST(FnMut, FnPointerStorage) {
  auto fn =
      FnMut<int(int)>::with_storage([](int a, int b) { return a * 2 + b; }, 1);
  EXPECT_EQ(fn.call_mut(2), 4);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(2), 4);

  auto fn2 =
      FnMut<int()>::with_storage([](int a, int b) { return a * 2 + b; }, 1, 2);
  EXPECT_EQ(fn2.call_mut(), 4);
  EXPECT_EQ(static_cast<decltype(fn2)&&>(fn2).call_once(), 4);
}

TEST(FnMut, CallWithConstRef) {
  auto fn = FnMut<const Copyable*(const Copyable&)>::with_fn_pointer(
      [](const Copyable& c) { return &c; });
  auto c = Copyable(3);
  EXPECT_EQ(fn.call_mut(c), &c);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(c), &c);
}

TEST(FnMut, CallWithMutRef) {
  auto fn =
      FnMut<int(Copyable&)>::with_fn_pointer([](Copyable& c) { return c.i++; });
  auto copy_only = Copyable(3);
  EXPECT_EQ(fn.call_mut(copy_only), 3);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(copy_only), 4);
  EXPECT_EQ(copy_only.i, 5);
}

TEST(FnMut, CopyableStorage) {
  auto fn =
      FnMut<int()>::with_storage([](const Copyable& c) { return c.i; },
                                 static_cast<const Copyable&>(Copyable(3)));
  EXPECT_EQ(fn.call_mut(), 3);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 3);
}

TEST(FnMut, MoveOnlyStorage) {
  auto fn = FnMut<int()>::with_storage([](const MoveOnly& m) { return m.i; },
                                       MoveOnly(3));
  EXPECT_EQ(fn.call_mut(), 3);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 3);
}

TEST(FnMut, CopyAndMoveOnlyStorage) {
  const auto copy_only = Copyable(3);
  auto f = [](const Copyable& c, MoveOnly& m) { return c.i * 2 + m.i; };
  // Will copy Copyable into storage, and move MoveOnly.
  auto fn = FnMut<int()>::with_storage(f, copy_only, MoveOnly(1));
  EXPECT_EQ(fn.call_mut(), 3 * 2 + 1);
  EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 3 * 2 + 1);
}

TEST(FnMut, MutateStorage) {
  auto fn = FnMut<int()>::with_storage([](MoveOnly& m) { return ++m.i; },
                                       MoveOnly(1));
  EXPECT_EQ(2, fn.call_mut());
  EXPECT_EQ(3, fn.call_mut());
  EXPECT_EQ(4, fn.call_mut());
}

TEST(FnMut, DoesntMutateStorage) {
  // It's okay of FnMut doesn't actually mutate storage.
  auto fn = FnMut<int()>::with_storage([](const MoveOnly& m) { return m.i; },
                                       MoveOnly(1));
  EXPECT_EQ(1, fn.call_mut());
  EXPECT_EQ(1, fn.call_mut());
}

TEST(FnOnce, TakeStorage) {
  auto fn = FnOnce<int()>::with_storage([](MoveOnly m) { return ++m.i; },
                                        MoveOnly(1));
  EXPECT_EQ(2, static_cast<decltype(fn)&&>(fn).call_once());
}

TEST(FnOnce, MutateStorage) {
  auto fn = FnOnce<int()>::with_storage([](MoveOnly& m) { return ++m.i; },
                                        MoveOnly(1));
  EXPECT_EQ(2, static_cast<decltype(fn)&&>(fn).call_once());
}

TEST(FnOnce, DoesntMutateStorage) {
  // It's okay of FnOnce doesn't actually mutate storage.
  auto fn = FnOnce<int()>::with_storage([](const MoveOnly& m) { return m.i; },
                                        MoveOnly(1));
  EXPECT_EQ(1, static_cast<decltype(fn)&&>(fn).call_once());
}

TEST(Fn, CanBeUsedAsFnOrMutOrOnce) {
  {
    auto fn = Fn<int()>::with_storage([](const int& i) { return i + 1; }, 3);
    EXPECT_EQ(fn.call(), 4);
    EXPECT_EQ(fn.call_mut(), 4);
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 4);
  }
  {
    auto fn = Fn<int()>::with_storage([](const int& i) { return i + 1; }, 3);
    EXPECT_EQ(fn.call(), 4);
    auto fn_mut = FnMut{static_cast<decltype(fn)&&>(fn)};
    EXPECT_EQ(fn_mut.call_mut(), 4);
    EXPECT_EQ(FnOnce{static_cast<decltype(fn_mut)&&>(fn_mut)}.call_once(), 4);
  }
  {
    auto fn = Fn<int()>::with_storage([](const int& i) { return i + 1; }, 3);
    EXPECT_EQ(fn.call(), 4);
    EXPECT_EQ(FnOnce{static_cast<decltype(fn)&&>(fn)}.call_once(), 4);
  }
}

TEST(FnMut, CanBeUsedAsMutOrOnce) {
  {
    auto fn = FnMut<int()>::with_storage([](const int& i) { return i + 1; }, 3);
    EXPECT_EQ(fn.call_mut(), 4);
    EXPECT_EQ(static_cast<decltype(fn)&&>(fn).call_once(), 4);
  }
  {
    auto fn_mut = FnMut<int()>::with_storage([](int& i) { return ++i; }, 3);
    EXPECT_EQ(fn_mut.call_mut(), 4);
    EXPECT_EQ(FnOnce{static_cast<decltype(fn_mut)&&>(fn_mut)}.call_once(), 5);
  }
}

TEST(Fn, CanBeMovedAsFnOrMutOrOnce) {
  auto fn_once = FnOnce<int()>::with_fn_pointer([]() { return 0; });
  auto fn_mut = FnMut<int()>::with_fn_pointer([]() { return 0; });
  auto fn = Fn<int()>::with_fn_pointer([]() { return 0; });
  fprintf(stderr, "hi 1\n");

  {
    auto fn2 = Fn<int()>::with_storage([](const int& i) { return i + 1; }, 3);

    fn = static_cast<decltype(fn2)&&>(fn2);
    EXPECT_EQ(fn.call(), 4);
    fn_mut = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(fn_mut.call_mut(), 4);
    fn_once = static_cast<decltype(fn_mut)&&>(fn_mut);
    EXPECT_EQ(static_cast<decltype(fn_once)&&>(fn_once).call_once(), 4);
  }
  {
    auto fn2 = Fn<int()>::with_storage([](const int& i) { return i + 1; }, 3);

    fn = static_cast<decltype(fn2)&&>(fn2);
    EXPECT_EQ(fn.call(), 4);
    fn_once = static_cast<decltype(fn)&&>(fn);
    EXPECT_EQ(static_cast<decltype(fn_once)&&>(fn_once).call_once(), 4);
  }
}

TEST(FnMut, CanBeMovedAsMutOrOnce) {
  auto fn_once = FnOnce<int()>::with_fn_pointer([]() { return 0; });
  auto fn_mut = FnMut<int()>::with_fn_pointer([]() { return 0; });

  {
    auto fn_mut2 = FnMut<int()>::with_storage([](int& i) { return ++i; }, 3);

    fn_mut = static_cast<decltype(fn_mut2)&&>(fn_mut2);
    EXPECT_EQ(fn_mut.call_mut(), 4);
    fn_once = static_cast<decltype(fn_mut)&&>(fn_mut);
    EXPECT_EQ(static_cast<decltype(fn_once)&&>(fn_once).call_once(), 5);
  }
  {
    auto fn_mut2 = FnMut<int()>::with_storage([](int& i) { return ++i; }, 3);

    fn_once = static_cast<decltype(fn_mut2)&&>(fn_mut2);
    EXPECT_EQ(static_cast<decltype(fn_once)&&>(fn_once).call_once(), 4);
  }
}

TEST(FnMut, MutableLambda) {
  auto x = [i = int(1)]() mutable { return ++i;};
  auto fn = FnMut<int()>::with_storage(x);
  EXPECT_EQ(fn(), 2);
  EXPECT_EQ(fn(), 3);
}

TEST(FnMut, MutableLambdaWithBoundArg) {
  auto x = [i = int(1)](int j) mutable { return ++i + j;};
  auto fn = FnMut<int()>::with_storage(x, 3);
  EXPECT_EQ(fn(), 5);
  EXPECT_EQ(fn(), 6);
}

TEST(FnMut, MutableLambdaWithCallArg) {
  auto x = [i = int(1)](int j) mutable { return ++i + j;};
  auto fn = FnMut<int(int)>::with_storage(x);
  EXPECT_EQ(fn(3), 5);
  EXPECT_EQ(fn(4), 7);
}

}  // namespace
