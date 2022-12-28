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

#include "option/option.h"

#include <math.h>  // TODO: Replace with f32::NAN()

#include "containers/array.h"
#include "googletest/include/gtest/gtest.h"
#include "iter/iterator.h"
#include "macros/__private/compiler_bugs.h"
#include "macros/builtin.h"
#include "mem/nonnull.h"
#include "mem/relocate.h"
#include "num/types.h"
#include "prelude.h"
#include "result/result.h"
#include "test/behaviour_types.h"
#include "test/from_i32.h"
#include "test/no_copy_move.h"
#include "tuple/tuple.h"

using sus::construct::Default;
using sus::mem::relocate_array_by_memcpy;
using sus::mem::relocate_one_by_memcpy;
using sus::option::None;
using sus::option::Option;
using sus::option::Some;
using sus::tuple::Tuple;
using namespace sus::test;

namespace {

#define IS_SOME(x)              \
  do {                          \
    EXPECT_TRUE(x.is_some());   \
    EXPECT_FALSE(x.is_none());  \
    switch (x) {                \
      case Some: break;         \
      case None: ADD_FAILURE(); \
    }                           \
  } while (false)

#define IS_NONE(x)              \
  do {                          \
    EXPECT_TRUE(x.is_none());   \
    EXPECT_FALSE(x.is_some());  \
    switch (x) {                \
      case None: break;         \
      case Some: ADD_FAILURE(); \
    }                           \
  } while (false)

template <class T, class U>
constexpr size_t max_sizeof() {
  return sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U);
}

struct LargerThanAPointer {
  size_t a, b, c, d;
};

static_assert(sizeof(Option<bool>) == sizeof(bool) + sizeof(bool));
static_assert(sizeof(Option<bool&>) == sizeof(bool*));
// An Option has space for T plus a bool, but it's size is rounded up to the
// alignment of T.
static_assert(sizeof(Option<int>) == sizeof(int) + max_sizeof<bool, int>());
static_assert(sizeof(Option<LargerThanAPointer&>) ==
              sizeof(LargerThanAPointer*));

template <class T, class = void, class... Args>
struct is_some_callable : std::false_type {};

template <class T, class... Args>
struct is_some_callable<
    T, std::void_t<decltype(T::some(std::declval<Args>()...))>, Args...>
    : std::true_type {};

template <class T, class... Args>
inline constexpr bool is_some_callable_v =
    is_some_callable<T, void, Args...>::value;

static_assert(is_some_callable_v<Option<int>, int>);
static_assert(is_some_callable_v<Option<int>, const int>);
static_assert(is_some_callable_v<Option<int>, int&>);
static_assert(is_some_callable_v<Option<int>, int&&>);
static_assert(is_some_callable_v<Option<int>, const int&>);

static_assert(!is_some_callable_v<Option<int&>, int>);
static_assert(!is_some_callable_v<Option<int&>, const int>);
static_assert(is_some_callable_v<Option<int&>, int&>);
static_assert(!is_some_callable_v<Option<int&>, const int&>);

TEST(Option, Construct) {
  {
    using T = DefaultConstructible;
    auto x = Option<T>::some(T());
    auto y = Option<T>::none();
    auto t = T();
    auto z = Option<T>::some(t);
  }
  {
    using T = NotDefaultConstructible;
    auto x = Option<T>::some(T(1));
    auto y = Option<T>::none();
    auto t = T(1);
    auto z = Option<T>::some(t);
  }
  {
    using T = TriviallyCopyable;
    auto x = Option<T>::some(T(1));
    auto y = Option<T>::none();
    auto t = T(1);
    auto z = Option<T>::some(t);
  }
  {
    using T = TriviallyMoveableAndRelocatable;
    auto x = Option<T>::some(T(1));
    auto y = Option<T>::none();
    // Not copyable.
    // auto t = T(1);
    // auto z = Option<T>::some(t);
  }
  {
    using T = TriviallyCopyableNotDestructible;
    auto x = Option<T>::some(T(1));
    auto y = Option<T>::none();
    auto t = T(1);
    auto z = Option<T>::some(t);
  }
  {
    using T = TriviallyMoveableNotDestructible;
    auto x = Option<T>::some(T(1));
    auto y = Option<T>::none();
    // Not copyable.
    // auto t = T(1);
    // auto z = Option<T>::some(t);
  }
  {
    using T = NotTriviallyRelocatableCopyableOrMoveable;
    auto x = Option<T>::some(T(1));
    auto y = Option<T>::none();
    // Not copyable.
    // auto t = T(1);
    // auto z = Option<T>::some(t);
  }
  {
    using T = TrivialAbiRelocatable;
    auto x = Option<T>::some(T(1));
    auto y = Option<T>::none();
    // Not copyable.
    // auto t = T(1);
    // auto z = Option<T>::some(t);
  }
  {
    using T = const NoCopyMove&;
    auto i = NoCopyMove();
    auto x = Option<T>::some(static_cast<T>(i));
    auto y = Option<T>::none();
    T t = i;
    auto z = Option<T>::some(t);
  }
  {
    using T = NoCopyMove&;
    auto i = NoCopyMove();
    auto x = Option<T>::some(mref(static_cast<T>(i)));
    auto y = Option<T>::none();
    T t = i;
    auto z = Option<T>::some(mref(t));
  }
}

TEST(Option, SomeNoneHelpers) {
  auto a = Option<i32>::some(2_i32);
  Option<i32> a2 = sus::some(2_i32);
  EXPECT_EQ(a, a2);

  auto b = Option<i32>::none();
  Option<i32> b2 = sus::none();
  EXPECT_EQ(b, b2);

  auto i = NoCopyMove();
  auto c = Option<NoCopyMove&>::some(i);
  Option<NoCopyMove&> c2 = sus::some(i);
  EXPECT_EQ(&c.unwrap_ref(), &c2.unwrap_ref());

  const auto ci = NoCopyMove();
  const auto cc = Option<const NoCopyMove&>::some(ci);
  Option<const NoCopyMove&> cc2 = sus::some(ci);
  EXPECT_EQ(&cc.unwrap_ref(), &cc2.unwrap_ref());

  Option<const NoCopyMove&> mut_to_const = sus::some(i);
  EXPECT_EQ(&i, &mut_to_const.unwrap_ref());
}

TEST(Option, Move) {
  // This type has a user defined move constructor, which deletes the implicit
  // move constructor in Option.
  struct Type {
    Type() = default;
    Type(Type&&) = default;
    Type& operator=(Type&&) = default;
  };
  auto x = Option<Type>::some(Type());
  auto y = sus::move(x);
  IS_SOME(y);
  x = sus::move(y);
  IS_SOME(x);

  struct MoveableLvalue {
    MoveableLvalue(int i) : i(i) {}
    MoveableLvalue(const MoveableLvalue& m) : i(m.i) {}
    void operator=(const MoveableLvalue& m) { i = m.i; }
    MoveableLvalue(MoveableLvalue&& m) : i(m.i) { m.i = 0; }
    void operator=(MoveableLvalue&& m) {
      i = m.i;
      m.i = 0;
    }
    int i;
  };
  MoveableLvalue lvalue(2);
  auto a = Option<MoveableLvalue>::some(lvalue);
  EXPECT_EQ(a.as_ref().unwrap().i, 2);
  EXPECT_EQ(lvalue.i, 2);

  auto b = Option<MoveableLvalue>::some(sus::move(lvalue));
  EXPECT_EQ(b.as_ref().unwrap().i, 2);
  EXPECT_EQ(lvalue.i, 0);
}

// No code should use Option after moving from it; that's what
// `Option<T>::take()` is for. But bugs can happen and we don't have the static
// analysis or compiler tools to prevent them yet. So when they do, we should
// get a defined state. Either Some with a valid value, or None. Never Some with
// a moved-from value.
TEST(Option, UseAfterMove) {
  auto x = Option<TriviallyMoveableAndRelocatable>::some(
      TriviallyMoveableAndRelocatable(1));
  auto y = sus::move(x);
  IS_SOME(x);  // Trivially relocatable leaves `x` fully in tact.
  EXPECT_EQ(x.as_ref().unwrap().i, 1);
  x = sus::move(y);
  IS_SOME(y);  // Trivially relocatable leaves `y` fully in tact.
  EXPECT_EQ(y.as_ref().unwrap().i, 1);

  auto a = Option<NotTriviallyRelocatableCopyableOrMoveable>::some(
      NotTriviallyRelocatableCopyableOrMoveable(1));
  auto b = sus::move(a);
  IS_NONE(a);  // Not trivially relocatable moves-from the value in `a`.
  a = sus::move(b);
  IS_NONE(b);  // Not trivially relocatable moves-from the value in `b`.
}

TEST(Option, Some) {
  auto x = Option<DefaultConstructible>::some(DefaultConstructible());
  IS_SOME(x);

  auto y = Option<NotDefaultConstructible>::some(NotDefaultConstructible(3));
  IS_SOME(y);

  auto i = NoCopyMove();
  auto ix = Option<NoCopyMove&>::some(mref(i));
  IS_SOME(ix);

  constexpr auto cx(
      Option<DefaultConstructible>::some(DefaultConstructible()).unwrap());
  static_assert(cx.i == 2);

  constexpr auto cy(
      Option<NotDefaultConstructible>::some(NotDefaultConstructible(3))
          .unwrap());
  static_assert(cy.i == 3);
}

TEST(Option, None) {
  auto x = Option<DefaultConstructible>::none();
  IS_NONE(x);

  auto y = Option<NotDefaultConstructible>::none();
  IS_NONE(y);

  auto ix = Option<NoCopyMove&>::none();
  IS_NONE(ix);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // constexpr auto cx(Option<DefaultConstructible>::none());
  // static_assert(cx.is_none());

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // constexpr auto cy(Option<NotDefaultConstructible>::none());
  // static_assert(cy.is_none());
}

TEST(Option, Default) {
  auto x = Option<DefaultConstructible>();
  IS_NONE(x);
}

TEST(Option, Destructor) {
  static int count = 0;
  struct WatchDestructor {
    ~WatchDestructor() { ++count; }
  };
  {
    auto x = Option<WatchDestructor>::some(WatchDestructor());
    count = 0;  // Init moves may cause destructor to run without optimizations.
  }
  EXPECT_EQ(1, count);

  WatchDestructor w;
  {
    auto x = Option<WatchDestructor&>::some(mref(w));
    count = 0;
  }
  EXPECT_EQ(0, count);
}

TEST(Option, ExpectSome) {
  auto x = Option<int>::some(0).expect("hello world");
  EXPECT_EQ(x, 0);

  auto i = NoCopyMove();
  auto& xi = Option<NoCopyMove&>::some(mref(i)).expect("hello world");
  EXPECT_EQ(&xi, &i);
}

TEST(OptionDeathTest, ExpectNone) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(Option<int>::none().expect("hello world"), "hello world");
  EXPECT_DEATH(Option<NoCopyMove&>::none().expect("hello world"),
               "hello world");
#endif
}

TEST(Option, UnwrapSome) {
  auto x = Option<int>::some(0).unwrap();
  EXPECT_EQ(x, 0);

  auto i = NoCopyMove();
  auto& ix = Option<NoCopyMove&>::some(mref(i)).unwrap();
  EXPECT_EQ(&ix, &i);
}

TEST(OptionDeathTest, UnwrapNone) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(Option<int>::none().unwrap(), "");
  EXPECT_DEATH(Option<NoCopyMove&>::none().unwrap(), "");
#endif
}

TEST(Option, UnwrapUncheckedSome) {
  auto x = Option<int>::some(0).unwrap_unchecked(unsafe_fn);
  EXPECT_EQ(x, 0);

  auto i = NoCopyMove();
  auto& ix = Option<NoCopyMove&>::some(mref(i)).unwrap_unchecked(unsafe_fn);
  EXPECT_EQ(&ix, &i);
}

TEST(Option, Take) {
  auto x = Option<int>::some(404);
  auto y = x.take();
  // The value has moved from `x` to `y`.
  IS_NONE(x);
  IS_SOME(y);
  EXPECT_EQ(sus::move(y).unwrap(), 404);

  auto n = Option<int>::none();
  auto m = n.take();
  // The None has moved from `n` to `m`, which is a no-op on `n`.
  IS_NONE(n);
  IS_NONE(m);

  auto i = NoCopyMove();
  auto ix = Option<NoCopyMove&>::some(mref(i));
  auto iy = ix.take();
  IS_NONE(ix);
  IS_SOME(iy);
  EXPECT_EQ(&sus::move(iy).unwrap(), &i);

  auto in = Option<NoCopyMove&>::none();
  auto im = in.take();
  IS_NONE(in);
  IS_NONE(im);
}

TEST(Option, UnwrapOr) {
  auto x = Option<int>::some(2).unwrap_or(3);
  EXPECT_EQ(x, 2);
  auto y = Option<int>::none().unwrap_or(3);
  EXPECT_EQ(y, 3);

  NoCopyMove i, i2;
  auto& ix = Option<NoCopyMove&>::some(mref(i)).unwrap_or(i2);
  EXPECT_EQ(&ix, &i);

  auto& iy = Option<NoCopyMove&>::none().unwrap_or(i2);
  EXPECT_EQ(&iy, &i2);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // static_assert(Option<int>::none().unwrap_or(3) == 3);
  // constexpr int ci = 2;
  // static_assert(Option<const int&>::none().unwrap_or(ci) == 2);
}

TEST(Option, UnwrapOrElse) {
  auto x = Option<int>::some(2).unwrap_or_else([]() { return int(3); });
  EXPECT_EQ(x, 2);
  auto y = Option<int>::none().unwrap_or_else([]() { return int(3); });
  EXPECT_EQ(y, 3);

  NoCopyMove i, i2;
  auto& ix = Option<NoCopyMove&>::some(mref(i)).unwrap_or_else(
      [&]() -> NoCopyMove& { return i2; });
  EXPECT_EQ(&ix, &i);

  auto& iy = Option<NoCopyMove&>::none().unwrap_or_else(
      [&]() -> NoCopyMove& { return i2; });
  EXPECT_EQ(&iy, &i2);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // static_assert(
  //     Option<int>::none().unwrap_or_else([]() { return int(3); }) == 3);
  // TODO: Why does this not work? (Well, now we have disabled constexpr so it
  // won't work.)
  // static constexpr int ci = 2;
  // static_assert(Option<const int&>::none().unwrap_or_else(
  //                   []() ->const int& { return ci; }) == 2,
  //               "");
}

TEST(Option, UnwrapOrDefault) {
  auto x = Option<DefaultConstructible>::some(DefaultConstructible{.i = 4})
               .unwrap_or_default();
  EXPECT_EQ(x.i, 4);
  auto y = Option<DefaultConstructible>::none().unwrap_or_default();
  EXPECT_EQ(y.i, 2);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // static_assert(Option<int>::none().unwrap_or_default() == 0);
}

TEST(Option, Map) {
  struct Mapped {
    int i;
  };

  bool called = false;
  auto x = Option<int>::some(2).map([&](int&& i) {
    called = true;
    return Mapped(i + 1);
  });
  static_assert(std::is_same_v<decltype(x), Option<Mapped>>);
  EXPECT_EQ(sus::move(x).unwrap().i, 3);
  EXPECT_TRUE(called);

  called = false;
  auto y = Option<int>::none().map([&](int&& i) {
    called = true;
    return Mapped(i + 1);
  });
  static_assert(std::is_same_v<decltype(y), Option<Mapped>>);
  IS_NONE(y);
  EXPECT_FALSE(called);

  called = false;
  NoCopyMove i;
  auto ix = Option<NoCopyMove&>::some(mref(i)).map([&](NoCopyMove&) {
    called = true;
    return Mapped(2);
  });
  static_assert(std::is_same_v<decltype(ix), Option<Mapped>>);
  EXPECT_EQ(ix.as_ref().unwrap().i, 2);
  EXPECT_TRUE(called);

  called = false;
  auto iy = Option<NoCopyMove&>::none().map([&](NoCopyMove&) {
    called = true;
    return Mapped(2);
  });
  static_assert(std::is_same_v<decltype(iy), Option<Mapped>>);
  IS_NONE(iy);
  EXPECT_FALSE(called);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // static_assert(Option<int>::some(2)
  //                       .map([](int&& i) { return Mapped(i + 1); })
  //                       .unwrap()
  //                       .i == 3,
  //               "");
  // constexpr int ci = 2;
  // static_assert(Option<const int&>::some(ci)
  //                       .map([](const int& i) { return Mapped(i + 1); })
  //                       .unwrap()
  //                       .i == 3,
  //               "");
}

TEST(Option, MapOr) {
  struct Mapped {
    int i;
  };

  auto x = Option<int>::some(2).map_or(
      Mapped(4), [](int&& i) { return static_cast<Mapped>(i + 1); });
  static_assert(std::is_same_v<decltype(x), Mapped>);
  EXPECT_EQ(x.i, 3);

  auto y = Option<int>::none().map_or(Mapped(4),
                                      [](int&& i) { return Mapped(i + 1); });
  static_assert(std::is_same_v<decltype(y), Mapped>);
  EXPECT_EQ(y.i, 4);

  auto i = NoCopyMove();
  auto ix = Option<NoCopyMove&>::some(mref(i)).map_or(
      Mapped(1), [](NoCopyMove&) { return Mapped(2); });
  static_assert(std::is_same_v<decltype(ix), Mapped>);
  EXPECT_EQ(ix.i, 2);

  auto iy = Option<NoCopyMove&>::none().map_or(
      Mapped(1), [](NoCopyMove&) { return Mapped(2); });
  static_assert(std::is_same_v<decltype(ix), Mapped>);
  EXPECT_EQ(iy.i, 1);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // static_assert(
  //     Option<int>::none().map_or(4, [](int&&) { return 1; }).unwrap() == 4,
  //     "");
}

TEST(Option, MapOrElse) {
  struct Mapped {
    int i;
  };

  bool map_called = false;
  bool else_called = false;
  auto x = Option<int>::some(2).map_or_else(
      [&]() {
        else_called = true;
        return Mapped(4);
      },
      [&](int&& i) {
        map_called = true;
        return Mapped(i + 1);
      });
  static_assert(std::is_same_v<decltype(x), Mapped>);
  EXPECT_EQ(x.i, 3);
  EXPECT_TRUE(map_called);
  EXPECT_FALSE(else_called);

  map_called = else_called = false;
  auto y = Option<int>::none().map_or_else(
      [&]() {
        else_called = true;
        return Mapped(4);
      },
      [&](int&& i) {
        map_called = true;
        return Mapped(i + 1);
      });
  static_assert(std::is_same_v<decltype(y), Mapped>);
  EXPECT_EQ(y.i, 4);
  EXPECT_FALSE(map_called);
  EXPECT_TRUE(else_called);

  auto i = NoCopyMove();
  map_called = else_called = false;
  auto ix = Option<NoCopyMove&>::some(mref(i)).map_or_else(
      [&]() {
        else_called = true;
        return Mapped(1);
      },
      [&](NoCopyMove&) {
        map_called = true;
        return Mapped(2);
      });
  static_assert(std::is_same_v<decltype(ix), Mapped>);
  EXPECT_EQ(ix.i, 2);
  EXPECT_TRUE(map_called);
  EXPECT_FALSE(else_called);

  map_called = else_called = false;
  auto iy = Option<NoCopyMove&>::none().map_or_else(
      [&]() {
        else_called = true;
        return Mapped(1);
      },
      [&](NoCopyMove&) {
        map_called = true;
        return Mapped(2);
      });
  static_assert(std::is_same_v<decltype(iy), Mapped>);
  EXPECT_EQ(iy.i, 1);
  EXPECT_FALSE(map_called);
  EXPECT_TRUE(else_called);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // static_assert(Option<int>::none()
  //                       .map_or_else([]() { return Mapped(4); },
  //                                    [](int&&) { return Mapped(1); })
  //                       .unwrap()
  //                       .i == 4,
  //               "");
  // static_assert(Option<int>::some(2)
  //                       .map_or_else([]() { return Mapped(4); },
  //                                    [](int&&) { return Mapped(1); })
  //                       .unwrap()
  //                       .i == 1,
  //               "");
  // constexpr int ci = 2;
  // static_assert(Option<const int&>::none()
  //                       .map_or_else([]() { return Mapped(4); },
  //                                    [](const int&) { return Mapped(1); })
  //                       .unwrap()
  //                       .i == 4,
  //               "");
  // static_assert(Option<const int&>::some(ci)
  //                       .map_or_else([]() { return Mapped(4); },
  //                                    [](const int& i) { return Mapped(i + 1);
  //                                    })
  //                       .unwrap()
  //                       .i == 3,
  //               "");
}

TEST(Option, Filter) {
  auto x = Option<int>::some(2).filter([](const int&) { return true; });
  static_assert(std::is_same_v<decltype(x), Option<int>>);
  IS_SOME(x);

  auto y = Option<int>::some(2).filter([](const int&) { return false; });
  static_assert(std::is_same_v<decltype(y), Option<int>>);
  IS_NONE(y);

  auto nx = Option<int>::none().filter([](const int&) { return true; });
  static_assert(std::is_same_v<decltype(nx), Option<int>>);
  IS_NONE(nx);

  auto ny = Option<int>::none().filter([](const int&) { return false; });
  static_assert(std::is_same_v<decltype(ny), Option<int>>);
  IS_NONE(ny);

  auto i = NoCopyMove();
  auto ix = Option<NoCopyMove&>::some(mref(i)).filter(
      [](const NoCopyMove&) { return true; });
  static_assert(std::is_same_v<decltype(ix), Option<NoCopyMove&>>);
  IS_SOME(ix);

  auto iy = Option<NoCopyMove&>::some(mref(i)).filter(
      [](const NoCopyMove&) { return false; });
  static_assert(std::is_same_v<decltype(iy), Option<NoCopyMove&>>);
  IS_NONE(iy);

  auto inx = Option<NoCopyMove&>::none().filter(
      [](const NoCopyMove&) { return true; });
  static_assert(std::is_same_v<decltype(inx), Option<NoCopyMove&>>);
  IS_NONE(inx);

  auto iny = Option<NoCopyMove&>::none().filter(
      [](const NoCopyMove&) { return false; });
  static_assert(std::is_same_v<decltype(iny), Option<NoCopyMove&>>);
  IS_NONE(iny);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // static_assert(
  //     Option<int>::some(2).filter([](const int&) { return true; }).unwrap()
  //     ==
  //         2,
  //     "");
  // constexpr int ci = 2;
  // static_assert(Option<const int&>::some(ci)
  //                       .filter([](const int&) { return true; })
  //                       .unwrap() == 2,
  //               "");

  static int count = 0;
  struct WatchDestructor {
    ~WatchDestructor() { ++count; }
  };

  static int hold_count;
  {
    auto a = Option<WatchDestructor>::some(WatchDestructor());
    count = 0;
    auto af = sus::move(a).filter([](const WatchDestructor&) { return true; });
    // The WatchDestructor was moved from `a` to `af` and the temporary's
    // was destroyed.
    EXPECT_GE(count, 1);
    hold_count = count;
  }
  // No double destruction in `a`, but there's one in `af`.
  EXPECT_EQ(count, hold_count + 1);

  {
    auto b = Option<WatchDestructor>::some(WatchDestructor());
    count = 0;
    auto bf = sus::move(b).filter([](const WatchDestructor&) { return false; });
    // The WatchDestructor in `b` was destroyed.
    EXPECT_GE(count, 1);
  }
  // No double destruction in `b`.
  EXPECT_GE(count, 1);

  {
    count = 0;
    auto c = Option<WatchDestructor>::none();
    auto cf = sus::move(c).filter([](const WatchDestructor&) { return false; });
    // Nothing constructed or destructed.
    EXPECT_EQ(count, 0);
  }
  EXPECT_EQ(count, 0);

  WatchDestructor w;
  {
    count = 0;
    auto c = Option<WatchDestructor&>::some(mref(w));
    auto cf = sus::move(c).filter([](const WatchDestructor&) { return false; });
    // Nothing constructed or destructed.
    EXPECT_EQ(count, 0);
  }
  EXPECT_EQ(count, 0);
}

TEST(Option, And) {
  auto x = Option<int>::some(2).and_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(x, 3);

  auto y = Option<int>::some(2).and_opt(Option<int>::none());
  IS_NONE(y);

  auto nx = Option<int>::none().and_opt(Option<int>::some(3));
  IS_NONE(nx);

  auto ny = Option<int>::none().and_opt(Option<int>::none());
  IS_NONE(ny);

  NoCopyMove i2, i3;
  auto& ix = Option<NoCopyMove&>::some(mref(i2))
                 .and_opt(Option<NoCopyMove&>::some(mref(i3)))
                 .unwrap();
  EXPECT_EQ(&ix, &i3);

  auto iy =
      Option<NoCopyMove&>::some(mref(i2)).and_opt(Option<NoCopyMove&>::none());
  IS_NONE(iy);

  auto inx =
      Option<NoCopyMove&>::none().and_opt(Option<NoCopyMove&>::some(mref(i3)));
  IS_NONE(inx);

  auto iny = Option<NoCopyMove&>::none().and_opt(Option<NoCopyMove&>::none());
  IS_NONE(iny);
}

TEST(Option, AndThen) {
  struct And {
    int i;
  };

  bool called = false;
  auto x = Option<int>::some(2).and_then([&](int&&) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(x), Option<And>>);
  EXPECT_EQ(sus::move(x).unwrap().i, 3);
  EXPECT_TRUE(called);

  called = false;
  auto y = Option<int>::some(2).and_then([&](int&&) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(y), Option<And>>);
  IS_NONE(y);
  EXPECT_TRUE(called);

  called = false;
  auto nx = Option<int>::none().and_then([&](int&&) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(nx), Option<And>>);
  IS_NONE(nx);
  EXPECT_FALSE(called);

  called = false;
  auto ny = Option<int>::none().and_then([&](int&&) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(ny), Option<And>>);
  IS_NONE(ny);
  EXPECT_FALSE(called);

  auto i = NoCopyMove();

  called = false;
  auto ix = Option<NoCopyMove&>::some(mref(i)).and_then([&](NoCopyMove&) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(ix), Option<And>>);
  EXPECT_EQ(ix.as_ref().unwrap().i, 3);
  EXPECT_TRUE(called);

  called = false;
  auto iy = Option<NoCopyMove&>::some(mref(i)).and_then([&](NoCopyMove&) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(iy), Option<And>>);
  IS_NONE(iy);
  EXPECT_TRUE(called);

  called = false;
  auto inx = Option<NoCopyMove&>::none().and_then([&](NoCopyMove&) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(inx), Option<And>>);
  IS_NONE(inx);
  EXPECT_FALSE(called);

  called = false;
  auto iny = Option<NoCopyMove&>::none().and_then([&](NoCopyMove&) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(iny), Option<And>>);
  IS_NONE(iny);
  EXPECT_FALSE(called);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // constexpr auto cx =
  //     Option<int>::some(2)
  //         .and_then([&](int&&) { return Option<And>::some(And(3)); })
  //         .unwrap();
  // static_assert(cx.i == 3);
  // constexpr int ci = 2;
  // constexpr auto icx =
  //     Option<const int&>::some(ci)
  //         .and_then([&](const int&) { return Option<And>::some(And(3)); })
  //         .unwrap();
  // static_assert(icx.i == 3);
}

TEST(Option, Or) {
  auto x = Option<int>::some(2).or_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(x, 2);

  auto y = Option<int>::some(2).or_opt(Option<int>::none()).unwrap();
  EXPECT_EQ(y, 2);

  auto nx = Option<int>::none().or_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(nx, 3);

  auto ny = Option<int>::none().or_opt(Option<int>::none());
  IS_NONE(ny);

  NoCopyMove i2, i3;

  auto& ix = Option<NoCopyMove&>::some(mref(i2))
                 .or_opt(Option<NoCopyMove&>::some(mref(i3)))
                 .unwrap();
  EXPECT_EQ(&ix, &i2);

  auto& iy = Option<NoCopyMove&>::some(mref(i2))
                 .or_opt(Option<NoCopyMove&>::none())
                 .unwrap();
  EXPECT_EQ(&iy, &i2);

  auto& inx = Option<NoCopyMove&>::none()
                  .or_opt(Option<NoCopyMove&>::some(mref(i3)))
                  .unwrap();
  EXPECT_EQ(&inx, &i3);

  auto iny = Option<NoCopyMove&>::none().or_opt(Option<NoCopyMove&>::none());
  IS_NONE(iny);
}

TEST(Option, OrElse) {
  bool called = false;
  auto x = Option<int>::some(2)
               .or_else([&]() {
                 called = true;
                 return Option<int>::some(3);
               })
               .unwrap();
  EXPECT_EQ(x, 2);
  EXPECT_FALSE(called);

  called = false;
  auto y = Option<int>::some(2)
               .or_else([&]() {
                 called = true;
                 return Option<int>::none();
               })
               .unwrap();
  EXPECT_EQ(y, 2);
  EXPECT_FALSE(called);

  called = false;
  auto nx = Option<int>::none()
                .or_else([&]() {
                  called = true;
                  return Option<int>::some(3);
                })
                .unwrap();
  EXPECT_EQ(nx, 3);
  EXPECT_TRUE(called);

  called = false;
  auto ny = Option<int>::none().or_else([&]() {
    called = true;
    return Option<int>::none();
  });
  IS_NONE(ny);
  EXPECT_TRUE(called);

  NoCopyMove i2, i3;

  called = false;
  auto& ix = Option<NoCopyMove&>::some(mref(i2))
                 .or_else([&]() {
                   called = true;
                   return Option<NoCopyMove&>::some(mref(i3));
                 })
                 .unwrap();
  EXPECT_EQ(&ix, &i2);
  EXPECT_FALSE(called);

  called = false;
  auto& iy = Option<NoCopyMove&>::some(mref(i2))
                 .or_else([&]() {
                   called = true;
                   return Option<NoCopyMove&>::none();
                 })
                 .unwrap();
  EXPECT_EQ(&iy, &i2);
  EXPECT_FALSE(called);

  called = false;
  auto& inx = Option<NoCopyMove&>::none()
                  .or_else([&]() {
                    called = true;
                    return Option<NoCopyMove&>::some(mref(i3));
                  })
                  .unwrap();
  EXPECT_EQ(&inx, &i3);
  EXPECT_TRUE(called);

  called = false;
  auto iny = Option<NoCopyMove&>::none().or_else([&]() {
    called = true;
    return Option<NoCopyMove&>::none();
  });
  IS_NONE(iny);
  EXPECT_TRUE(called);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // constexpr auto cx = Option<int>::some(2)
  //                         .or_else([&]() { return Option<int>::some(3); })
  //                         .unwrap();
  // static_assert(cx == 2);
  // TODO: Why does this not work? (Well, now we have disabled constexpr so it
  // won't work.)
  // constexpr int ci2 = 2, ci3 = 3;
  // constexpr auto icx = Option<const int&>::some(ci2)
  //                         .or_else([&]() { return Option<const
  //                         int&>::some(ci3); }) .unwrap();
  // static_assert(icx == 2);
}

TEST(Option, Xor) {
  auto x = Option<int>::some(2).xor_opt(Option<int>::some(3));
  IS_NONE(x);

  auto y = Option<int>::some(2).xor_opt(Option<int>::none()).unwrap();
  EXPECT_EQ(y, 2);

  auto nx = Option<int>::none().xor_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(nx, 3);

  auto ny = Option<int>::none().xor_opt(Option<int>::none());
  IS_NONE(ny);

  NoCopyMove i2, i3;

  auto ix = Option<NoCopyMove&>::some(mref(i2)).xor_opt(
      Option<NoCopyMove&>::some(mref(i3)));
  IS_NONE(ix);

  auto& iy = Option<NoCopyMove&>::some(mref(i2))
                 .xor_opt(Option<NoCopyMove&>::none())
                 .unwrap();
  EXPECT_EQ(&iy, &i2);

  auto& inx = Option<NoCopyMove&>::none()
                  .xor_opt(Option<NoCopyMove&>::some(mref(i3)))
                  .unwrap();
  EXPECT_EQ(&inx, &i3);

  auto iny = Option<NoCopyMove&>::none().xor_opt(Option<NoCopyMove&>::none());
  IS_NONE(iny);
}

TEST(Option, Insert) {
  auto x = Option<int>::none();
  x.insert(3);
  EXPECT_EQ(x.as_ref().unwrap(), 3);

  auto y = Option<int>::some(4);
  y.insert(5);
  EXPECT_EQ(y.as_ref().unwrap(), 5);

  NoCopyMove i2, i3;

  auto ix = Option<NoCopyMove&>::none();
  ix.insert(mref(i2));
  EXPECT_EQ(&ix.unwrap_ref(), &i2);

  auto iy = Option<NoCopyMove&>::some(mref(i2));
  iy.insert(mref(i3));
  EXPECT_EQ(&iy.unwrap_ref(), &i3);
}

TEST(Option, GetOrInsert) {
  auto x = Option<int>::none();
  auto& rx = x.get_or_insert(9);
  static_assert(std::is_same_v<decltype(x.get_or_insert(9)), int&>);
  EXPECT_EQ(rx, 9);
  rx = 5;
  EXPECT_EQ(sus::move(x).unwrap(), 5);

  auto y = Option<int>::some(11);
  auto& ry = y.get_or_insert(7);
  static_assert(std::is_same_v<decltype(y.get_or_insert(7)), int&>);
  EXPECT_EQ(ry, 11);
  EXPECT_EQ(sus::move(y).unwrap(), 11);

  NoCopyMove i2, i3;

  auto ix = Option<NoCopyMove&>::none();
  auto& irx = ix.get_or_insert(mref(i3));
  static_assert(std::is_same_v<decltype(ix.get_or_insert(i3)), NoCopyMove&>);
  EXPECT_EQ(&irx, &i3);
  EXPECT_EQ(&ix.as_ref().unwrap(), &i3);

  auto iy = Option<NoCopyMove&>::some(mref(i2));
  auto& iry = iy.get_or_insert(mref(i3));
  static_assert(std::is_same_v<decltype(iy.get_or_insert(i3)), NoCopyMove&>);
  EXPECT_EQ(&iry, &i2);
  EXPECT_EQ(&iy.as_ref().unwrap(), &i2);
}

TEST(Option, GetOrInsertDefault) {
  auto x = Option<DefaultConstructible>::none();
  auto& rx = x.get_or_insert_default();
  static_assert(std::is_same_v<decltype(rx), DefaultConstructible&>);
  EXPECT_EQ(rx.i, 2);
  IS_SOME(x);
  EXPECT_EQ(sus::move(x).unwrap().i, 2);

  auto y = Option<DefaultConstructible>::some(DefaultConstructible(404));
  auto& ry = y.get_or_insert_default();
  static_assert(std::is_same_v<decltype(ry), DefaultConstructible&>);
  EXPECT_EQ(ry.i, 404);
  IS_SOME(y);
  EXPECT_EQ(sus::move(y).unwrap().i, 404);
}

TEST(Option, GetOrInsertWith) {
  bool called = false;
  auto x = Option<int>::none();
  auto&& rx = x.get_or_insert_with([&]() {
    called = true;
    return 9;
  });
  static_assert(std::is_same_v<decltype(rx), int&>);
  EXPECT_EQ(rx, 9);
  rx = 12;
  EXPECT_TRUE(called);
  IS_SOME(x);
  EXPECT_EQ(sus::move(x).unwrap(), 12);

  called = false;
  auto y = Option<int>::some(11);
  auto&& ry = y.get_or_insert_with([&]() {
    called = true;
    return 7;
  });
  static_assert(std::is_same_v<decltype(ry), int&>);
  EXPECT_EQ(ry, 11);
  ry = 18;
  EXPECT_FALSE(called);
  IS_SOME(y);
  EXPECT_EQ(sus::move(y).unwrap(), 18);

  NoCopyMove i2, i3;

  called = false;
  auto ix = Option<NoCopyMove&>::none();
  auto&& irx = ix.get_or_insert_with([&]() -> NoCopyMove& {
    called = true;
    return i3;
  });
  static_assert(std::is_same_v<decltype(irx), NoCopyMove&>);
  EXPECT_TRUE(called);
  EXPECT_EQ(&irx, &i3);
  EXPECT_EQ(&ix.as_ref().unwrap(), &i3);

  called = false;
  auto iy = Option<NoCopyMove&>::some(mref(i2));
  auto&& iry = iy.get_or_insert_with([&]() -> NoCopyMove& {
    called = true;
    return i3;
  });
  static_assert(std::is_same_v<decltype(iry), NoCopyMove&>);
  EXPECT_FALSE(called);
  EXPECT_EQ(&iry, &i2);
  EXPECT_EQ(&iy.as_ref().unwrap(), &i2);
}

TEST(Option, AsRef) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.as_ref()), Option<const int&>>);
  EXPECT_EQ(&x.get_or_insert(0), &x.as_ref().unwrap());

  auto n = Option<int>::none();
  IS_NONE(n.as_ref());

  auto i = NoCopyMove();

  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(
      std::is_same_v<decltype(ix.as_ref()), Option<const NoCopyMove&>>);
  EXPECT_EQ(&i, &ix.as_ref().unwrap());

  auto in = Option<NoCopyMove&>::none();
  IS_NONE(in.as_ref());

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // constexpr auto cx = Option<int>::some(3);
  // static_assert(cx.as_ref().unwrap() == 3);
  // constexpr int ci = 2;
  // constexpr auto icx = Option<int>::some(ci);
  // static_assert(icx.as_ref().unwrap() == 2);
}

TEST(Option, UnwrapRefSome) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.unwrap_ref()), const int&>);
  EXPECT_EQ(&x.unwrap_ref(), &x.as_ref().unwrap());

  auto i = NoCopyMove();

  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.unwrap_ref()), const NoCopyMove&>);
  EXPECT_EQ(&ix.unwrap_ref(), &ix.as_ref().unwrap());

  // Verify constexpr.
  constexpr auto cx = Option<int>::some(3);
  static_assert(cx.unwrap_ref() == 3);
}

TEST(OptionDeathTest, UnwrapRefNone) {
  auto n = Option<int>::none();
  auto in = Option<NoCopyMove&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(n.unwrap_ref(), "");
  EXPECT_DEATH(in.unwrap_ref(), "");
#endif
}

TEST(Option, ExpectRefSome) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.expect_ref("")), const int&>);
  EXPECT_EQ(&x.expect_ref(""), &x.as_ref().unwrap());

  auto i = NoCopyMove();

  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.expect_ref("")), const NoCopyMove&>);
  EXPECT_EQ(&ix.expect_ref(""), &ix.as_ref().unwrap());

  // Verify constexpr.
  static_assert(Option<int>::some(3).expect_ref("") == 3);
  constexpr auto ci = NoCopyMove();
  static_assert(&Option<const NoCopyMove&>::some(ci).expect_ref("") == &ci);
}

TEST(OptionDeathTest, ExpectRefNone) {
  auto n = Option<int>::none();
  auto in = Option<NoCopyMove&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(n.expect_ref("hello world"), "hello world");
  EXPECT_DEATH(in.expect_ref("hello world"), "hello world");
#endif
}

TEST(Option, AsMut) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.as_mut()), Option<int&>>);
  EXPECT_EQ(&x.get_or_insert(0), &x.as_mut().unwrap());

  auto n = Option<int>::none();
  IS_NONE(n.as_mut());

  auto i = NoCopyMove();

  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.as_mut()), Option<NoCopyMove&>>);
  EXPECT_EQ(&i, &ix.as_mut().unwrap());

  auto in = Option<NoCopyMove&>::none();
  IS_NONE(in.as_mut());
}

TEST(Option, UnwrapMutSome) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.unwrap_mut()), int&>);
  EXPECT_EQ(&x.unwrap_mut(), &x.as_mut().unwrap());

  auto i = NoCopyMove();

  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.unwrap_mut()), NoCopyMove&>);
  EXPECT_EQ(&ix.unwrap_mut(), &i);
}

TEST(OptionDeathTest, UnwrapMutNone) {
  auto n = Option<int>::none();
  auto in = Option<NoCopyMove&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(n.unwrap_mut(), "");
  EXPECT_DEATH(in.unwrap_mut(), "");
#endif
}

TEST(Option, ExpectMutSome) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.expect_mut("")), int&>);
  EXPECT_EQ(&x.expect_mut(""), &x.as_mut().unwrap());

  auto i = NoCopyMove();

  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.expect_mut("")), NoCopyMove&>);
  EXPECT_EQ(&ix.expect_mut(""), &i);
}

TEST(OptionDeathTest, ExpectMutNone) {
  auto n = Option<int>::none();
  auto in = Option<NoCopyMove&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(n.expect_mut("hello world"), "hello world");
  EXPECT_DEATH(in.expect_mut("hello world"), "hello world");
#endif
}

TEST(Option, TrivialMove) {
  auto x = Option<sus::test::TriviallyMoveableAndRelocatable>::some(
      sus::test::TriviallyMoveableAndRelocatable(int(3423782)));
  auto y = sus::move(x);  // Move-construct.
  EXPECT_EQ(y.as_ref().unwrap().i, 3423782);

  y.as_mut().unwrap().i = int(6589043);
  x = sus::move(y);  // Move-assign.
  EXPECT_EQ(x.as_ref().unwrap().i, 6589043);
}

TEST(Option, TrivialCopy) {
  auto x = Option<sus::test::TriviallyCopyable>::some(
      sus::test::TriviallyCopyable(int(458790)));
  auto z = x;  // Copy-construct.
  EXPECT_EQ(z.as_ref().unwrap().i, 458790);

  z.as_mut().unwrap().i = int(98563453);
  auto y = Option<sus::test::TriviallyCopyable>::none();
  y = z;  // Copy-assign.
  EXPECT_EQ(y.as_ref().unwrap().i, 98563453);

  auto i = NoCopyMove();

  auto ix = Option<NoCopyMove&>::some(mref(i));
  auto iy = sus::move(ix);  // Move-construct.
  EXPECT_EQ(&iy.as_ref().unwrap(), &i);
  ix = sus::move(iy);  // Move-assign.
  EXPECT_EQ(&ix.as_ref().unwrap(), &i);
  auto iz = ix;  // Copy-construct.
  EXPECT_EQ(&ix.as_ref().unwrap(), &i);
  EXPECT_EQ(&iz.as_ref().unwrap(), &i);
  auto izz = Option<NoCopyMove&>::none();
  izz = iz;  // Copy-assign.
  EXPECT_EQ(&iz.as_ref().unwrap(), &i);
  EXPECT_EQ(&izz.as_ref().unwrap(), &i);
}

TEST(Option, Replace) {
  auto x = Option<int>::some(2);
  static_assert(std::is_same_v<decltype(x.replace(3)), Option<int>>);
  auto y = x.replace(3);
  EXPECT_EQ(x.as_ref().unwrap(), 3);
  EXPECT_EQ(y.as_ref().unwrap(), 2);

  auto z = Option<int>::none();
  static_assert(std::is_same_v<decltype(z.replace(3)), Option<int>>);
  auto zz = z.replace(3);
  EXPECT_EQ(z.as_ref().unwrap(), 3);
  IS_NONE(zz);

  NoCopyMove i2, i3;

  auto ix = Option<NoCopyMove&>::some(mref(i2));
  static_assert(std::is_same_v<decltype(ix.replace(i3)), Option<NoCopyMove&>>);
  auto iy = ix.replace(mref(i3));
  EXPECT_EQ(&ix.as_ref().unwrap(), &i3);
  EXPECT_EQ(&iy.as_ref().unwrap(), &i2);

  auto iz = Option<NoCopyMove&>::none();
  static_assert(std::is_same_v<decltype(iz.replace(i3)), Option<NoCopyMove&>>);
  auto izz = iz.replace(mref(i3));
  EXPECT_EQ(&iz.as_ref().unwrap(), &i3);
  IS_NONE(izz);
}

TEST(Option, Copied) {
  auto i = int(2);
  auto x = Option<int&>::none().copied();
  IS_NONE(x);

  auto y = Option<int&>::some(mref(i)).copied();
  EXPECT_NE(&y.as_ref().unwrap(), &i);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // constexpr int ic = 2;
  // static_assert(Option<int&>::none().copied().is_none());
  // static_assert(Option<const int&>::some(ic).copied().unwrap() == 2);
}

TEST(Option, Cloned) {
  struct Cloneable {
    Cloneable() = default;
    Cloneable(Cloneable&&) = default;
    Cloneable& operator=(Cloneable&&) = default;

    Cloneable clone() const { return Cloneable(); }
  };
  static_assert(!::sus::mem::Copy<Cloneable>);
  static_assert(::sus::mem::Clone<Cloneable>);

  auto c = Cloneable();
  auto x = Option<Cloneable&>::none().cloned();
  IS_NONE(x);

  auto y = Option<Cloneable&>::some(mref(c)).cloned();
  EXPECT_NE(&y.as_ref().unwrap(), &c);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // constexpr auto cc = Cloneable();
  // static_assert(Option<Cloneable&>::none().copied().is_none());
  // static_assert(&Option<const Cloneable&>::some(cc).cloned().unwrap() ==
  // &cc);
}

TEST(Option, Flatten) {
  static_assert(std::is_same_v<decltype(Option<Option<int>>::none().flatten()),
                               Option<int>>,
                "");
  static_assert(std::is_same_v<decltype(Option<Option<int&>>::none().flatten()),
                               Option<int&>>,
                "");
  static_assert(
      std::is_same_v<decltype(Option<Option<Option<int>>>::none().flatten()),
                     Option<Option<int>>>,
      "");

  EXPECT_TRUE(
      Option<Option<Option<int>>>::none().flatten().flatten().is_none());
  EXPECT_EQ(Option<Option<Option<int>>>::some(
                Option<Option<int>>::some(Option<int>::some(4)))
                .flatten()
                .flatten()
                .unwrap(),
            4);

  int i = 2;
  EXPECT_EQ(&Option<Option<int&>>::some(Option<int&>::some(mref(i)))
                 .flatten()
                 .unwrap(),
            &i);

  // TODO: Reading the Option's state can't be constexpr due to NeverValue field
  // optimization.
  // static_assert(Option<Option<int>>::none().flatten().is_none());
  // static_assert(
  //     Option<Option<int>>::some(Option<int>::none()).flatten().is_none());
  // static_assert(
  //     Option<Option<int>>::some(Option<int>::some(3)).flatten().unwrap() ==
  //     3,
  //     "");
}

TEST(Option, Iter) {
  auto x = Option<int>::none();
  for ([[maybe_unused]] auto i : x.iter()) {
    ADD_FAILURE();
  }

  auto xn = Option<NoCopyMove&>::none();
  for ([[maybe_unused]] auto& i : xn.iter()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Option<int>::some(2);
  for (auto& i : y.iter()) {
    static_assert(std::is_same_v<decltype(i), const int&>);
    EXPECT_EQ(i, 2);
    ++count;
  }
  EXPECT_EQ(count, 1);

  count = 0;
  auto n = NoCopyMove();
  auto z = Option<NoCopyMove&>::some(mref(n));
  for (auto& i : z.iter()) {
    static_assert(std::is_same_v<decltype(i), const NoCopyMove&>);
    EXPECT_EQ(&i, &n);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

TEST(Option, IterMut) {
  auto x = Option<int>::none();
  for ([[maybe_unused]] auto i : x.iter_mut()) {
    ADD_FAILURE();
  }

  auto xn = Option<NoCopyMove&>::none();
  for ([[maybe_unused]] auto& i : xn.iter_mut()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Option<int>::some(2);
  for (auto& i : y.iter_mut()) {
    static_assert(std::is_same_v<decltype(i), int&>);
    EXPECT_EQ(i, 2);
    i += 1;
    ++count;
  }
  EXPECT_EQ(y.as_ref().unwrap(), 3);

  count = 0;
  auto n = NoCopyMove();
  auto z = Option<NoCopyMove&>::some(mref(n));
  for (auto& i : z.iter_mut()) {
    static_assert(std::is_same_v<decltype(i), NoCopyMove&>);
    EXPECT_EQ(&i, &n);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

struct MoveOnly {
  explicit MoveOnly(int i) : i(i) {}
  MoveOnly(MoveOnly&& o) : i(o.i) {}
  void operator=(MoveOnly&& o) { i = o.i; }

  int i;
};

TEST(Option, IntoIter) {
  auto x = Option<int>::none();
  for ([[maybe_unused]] auto i : sus::move(x).into_iter()) {
    ADD_FAILURE();
  }

  auto xn = Option<NoCopyMove&>::none();
  for ([[maybe_unused]] auto& i : sus::move(xn).into_iter()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Option<MoveOnly>::some(MoveOnly(2));
  for (auto m : sus::move(y).into_iter()) {
    static_assert(std::is_same_v<decltype(m), MoveOnly>);
    EXPECT_EQ(m.i, 2);
    ++count;
  }
  EXPECT_EQ(count, 1);

  count = 0;
  auto n = NoCopyMove();
  auto z = Option<NoCopyMove&>::some(mref(n));
  for (auto& i : sus::move(z).into_iter()) {
    static_assert(std::is_same_v<decltype(i), NoCopyMove&>);
    EXPECT_EQ(&i, &n);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

TEST(Option, ImplicitIter) {
  auto x = Option<int>::none();
  for ([[maybe_unused]] auto i : x) {
    ADD_FAILURE();
  }

  auto xn = Option<NoCopyMove&>::none();
  for ([[maybe_unused]] auto& i : xn) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Option<MoveOnly>::some(MoveOnly(2));

  for (const auto& m : y) {
    static_assert(std::is_same_v<decltype(m), const MoveOnly&>);
    EXPECT_EQ(m.i, 2);
    ++count;
  }
  EXPECT_EQ(count, 1);

  count = 0;
  auto n = NoCopyMove();
  auto z = Option<NoCopyMove&>::some(mref(n));
  for (auto& i : z) {
    static_assert(std::is_same_v<decltype(i), const NoCopyMove&>);
    EXPECT_EQ(&i, &n);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

TEST(Option, Eq) {
  struct NotEq {};
  static_assert(!sus::ops::Eq<NotEq>);

  static_assert(::sus::ops::Eq<Option<int>, Option<int>>);
  static_assert(!::sus::ops::Eq<Option<NotEq>, Option<NotEq>>);

  EXPECT_EQ(Option<int>::some(1), Option<int>::some(1));
  EXPECT_NE(Option<int>::some(1), Option<int>::some(2));
  EXPECT_NE(Option<int>::none(), Option<int>::some(1));
  EXPECT_EQ(Option<int>::none(), Option<int>::none());
  EXPECT_EQ(Option<f32>::some(1.f), Option<f32>::some(1.f));
  EXPECT_EQ(Option<f32>::some(0.f), Option<f32>::some(-0.f));
  EXPECT_NE(Option<f32>::some(f32::TODO_NAN()),
            Option<f32>::some(f32::TODO_NAN()));
}

TEST(Option, Ord) {
  EXPECT_LT(Option<int>::some(1), Option<int>::some(2));
  EXPECT_GT(Option<int>::some(3), Option<int>::some(2));

  EXPECT_LT(Option<int>::none(), Option<int>::some(2));
  EXPECT_GT(Option<int>::some(1), Option<int>::none());

  int i1 = 1, i2 = 2;
  EXPECT_LT(Option<const int&>::some(i1), Option<const int&>::some(i2));
}

TEST(Option, StrongOrder) {
  EXPECT_EQ(std::strong_order(Option<int>::some(12), Option<int>::some(12)),
            std::strong_ordering::equal);
  EXPECT_EQ(std::strong_order(Option<int>::some(12), Option<int>::some(13)),
            std::strong_ordering::less);
  EXPECT_EQ(std::strong_order(Option<int>::some(12), Option<int>::some(11)),
            std::strong_ordering::greater);
  EXPECT_EQ(std::strong_order(Option<int>::some(12), Option<int>::none()),
            std::strong_ordering::greater);
  EXPECT_EQ(std::strong_order(Option<int>::none(), Option<int>::none()),
            std::strong_ordering::equal);
}

struct Weak {
  auto operator==(const Weak& o) const& { return a == o.a && b == o.b; }
  auto operator<=>(const Weak& o) const& {
    if (a == o.a) return std::weak_ordering::equivalent;
    if (a < o.a) return std::weak_ordering::less;
    return std::weak_ordering::greater;
  }

  Weak(int a, int b) : a(a), b(b) {}
  int a;
  int b;
};

TEST(Option, WeakOrder) {
  auto x = std::weak_order(Option<Weak>::some(Weak(1, 2)),
                           Option<Weak>::some(Weak(1, 2)));
  EXPECT_EQ(x, std::weak_ordering::equivalent);
  EXPECT_EQ(std::weak_order(Option<Weak>::some(Weak(1, 2)),
                            Option<Weak>::some(Weak(1, 3))),
            std::weak_ordering::equivalent);
  EXPECT_EQ(std::weak_order(Option<Weak>::some(Weak(1, 2)),
                            Option<Weak>::some(Weak(2, 3))),
            std::weak_ordering::less);
  EXPECT_EQ(std::weak_order(Option<Weak>::some(Weak(2, 2)),
                            Option<Weak>::some(Weak(1, 3))),
            std::weak_ordering::greater);
}

TEST(Option, PartialOrder) {
  EXPECT_EQ(
      std::partial_order(Option<float>::some(0.f), Option<float>::some(-0.f)),
      std::partial_ordering::equivalent);
  EXPECT_EQ(
      std::partial_order(Option<float>::some(12.f), Option<float>::some(12.f)),
      std::partial_ordering::equivalent);
  EXPECT_EQ(
      std::partial_order(Option<float>::some(13.f), Option<float>::some(12.f)),
      std::partial_ordering::greater);
  EXPECT_EQ(
      std::partial_order(Option<float>::some(11.f), Option<float>::some(12.f)),
      std::partial_ordering::less);
  EXPECT_EQ(std::partial_order(Option<f32>::some(11.f),
                               Option<f32>::some(f32::TODO_NAN())),
            std::partial_ordering::unordered);
  EXPECT_EQ(std::partial_order(Option<f32>::some(f32::TODO_NAN()),
                               Option<f32>::some(f32::TODO_NAN())),
            std::partial_ordering::unordered);
  EXPECT_EQ(std::partial_order(
                Option<f32>::some(0.f),
                Option<f32>::some(/* TODO: f32::INFINITY() */ HUGE_VALF)),
            std::partial_ordering::less);
  EXPECT_EQ(std::partial_order(Option<f32>::some(0.f),
                               Option<f32>::some(
                                   /* TODO: f32::NEG_INFINITY() */ -HUGE_VALF)),
            std::partial_ordering::greater);

  EXPECT_EQ(std::partial_order(Option<f32>::some(0.f), Option<f32>::none()),
            std::partial_ordering::greater);
  EXPECT_EQ(std::partial_order(Option<f32>::none(),
                               Option<f32>::some(f32::TODO_NAN())),
            std::partial_ordering::less);
}

struct NotCmp {};
static_assert(!sus::ops::PartialOrd<NotCmp>);

static_assert(::sus::ops::Ord<Option<int>, Option<int>>);
static_assert(!::sus::ops::Ord<Option<Weak>, Option<Weak>>);
static_assert(::sus::ops::WeakOrd<Option<Weak>, Option<Weak>>);
static_assert(!::sus::ops::WeakOrd<Option<float>, Option<float>>);
static_assert(::sus::ops::PartialOrd<Option<float>, Option<float>>);
static_assert(!::sus::ops::PartialOrd<Option<NotCmp>, Option<NotCmp>>);

TEST(Option, OkOr) {
  {
    auto o = Option<u8>::some(3_u8);
    auto r = sus::move(o).ok_or(-5_i32);
    IS_NONE(o);
    static_assert(std::same_as<sus::result::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(sus::move(r).unwrap(), 3_u8);
  }
  {
    auto o = Option<u8>::none();
    auto r = sus::move(o).ok_or(-5_i32);
    IS_NONE(o);
    static_assert(std::same_as<sus::result::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(sus::move(r).unwrap_err(), -5_i32);
  }

  // TODO: Result references are not yet supported.
  // https://github.com/chromium/subspace/issues/133
  /*
  {
    auto i = 1_i32;
    auto o = Option<i32&>::some(mref(i));
    auto r = sus::move(o).ok_or(-5_i32);
    IS_SOME(o);
    static_assert(std::same_as<sus::result::Result<i32&, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(&sus::move(r).unwrap(), &i);
  }
  {
    auto i = 1_i32;
    auto o = Option<i32&>::none();
    auto r = sus::move(o).ok_or(-5_i32);
    IS_NONE(o);
    static_assert(std::same_as<sus::result::Result<i32&, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(sus::move(r).unwrap_err(), -5_i32);
  }
  */
}

TEST(Option, OkOrElse) {
  {
    auto o = Option<u8>::some(3_u8);
    auto r = sus::move(o).ok_or_else([]() { return -5_i32; });
    IS_NONE(o);
    static_assert(std::same_as<sus::result::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(sus::move(r).unwrap(), 3_u8);
  }
  {
    auto o = Option<u8>::none();
    auto r = sus::move(o).ok_or_else([]() { return -5_i32; });
    IS_NONE(o);
    static_assert(std::same_as<sus::result::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(sus::move(r).unwrap_err(), -5_i32);
  }

  // TODO: Result references are not yet supported, test Option<u8&> when they
  // are. https://github.com/chromium/subspace/issues/133
}

TEST(Option, Transpose) {
  auto none = Option<sus::result::Result<u8, i32>>::none();
  auto t1 = sus::move(none).transpose();
  static_assert(
      std::same_as<sus::result::Result<Option<u8>, i32>, decltype(t1)>);
  EXPECT_EQ(t1.is_ok(), true);
  EXPECT_EQ(sus::move(t1).unwrap(), None);

  auto some_ok = Option<sus::result::Result<u8, i32>>::some(
      sus::result::Result<u8, i32>::with(5_u8));
  auto t2 = sus::move(some_ok).transpose();
  static_assert(
      std::same_as<sus::result::Result<Option<u8>, i32>, decltype(t2)>);
  EXPECT_EQ(t2.is_ok(), true);
  EXPECT_EQ(sus::move(t2).unwrap().unwrap(), 5_u8);

  auto some_err = Option<sus::result::Result<u8, i32>>::some(
      sus::result::Result<u8, i32>::with_err(-2_i32));
  auto t3 = sus::move(some_err).transpose();
  static_assert(
      std::same_as<sus::result::Result<Option<u8>, i32>, decltype(t3)>);
  EXPECT_EQ(t3.is_err(), true);
  EXPECT_EQ(sus::move(t3).unwrap_err(), -2_i32);

  // TODO: Result references are not yet supported, test them when they
  // are. https://github.com/chromium/subspace/issues/133
}

TEST(Option, Zip) {
  EXPECT_EQ(Option<i32>::none().zip(Option<i32>::none()), None);
  EXPECT_EQ(Option<i32>::some(1_i32).zip(Option<i32>::none()), None);
  EXPECT_EQ(Option<i32>::none().zip(Option<i32>::some(1_i32)), None);
  EXPECT_EQ(Option<i32>::some(2_i32).zip(Option<i32>::some(1_i32)), Some);

  {
    auto o = Option<i32>::some(-2_i32);
    EXPECT_EQ(sus::move(o).zip(Option<u8>::some(3_u8)).unwrap(),
              (Tuple<i32, u8>::with(-2_i32, 3_u8)));
    EXPECT_EQ(o, None);
  }

  {
    auto a = -2_i32;
    auto b = NoCopyMove();
    auto i = Option<const i32&>::some(a);
    auto u = Option<const NoCopyMove&>::some(b);
    static_assert(
        std::same_as<Tuple<const i32&, const NoCopyMove&>,
                     decltype(sus::move(i).zip(sus::move(u)).unwrap())>);
    auto zip = sus::move(i).zip(sus::move(u)).unwrap();
    EXPECT_EQ(&zip.get_ref<0>(), &a);
    EXPECT_EQ(&zip.get_ref<1>(), &b);
    EXPECT_EQ(i, None);
  }
}

TEST(Option, Unzip) {
  {
    auto s =
        Option<Tuple<i32, u32>>::some(Tuple<i32, u32>::with(-2_i32, 4_u32));
    auto sr = sus::move(s).unzip();
    static_assert(std::same_as<decltype(sr), Tuple<Option<i32>, Option<u32>>>);
    EXPECT_EQ(sr, (Tuple<Option<i32>, Option<u32>>::with(
                      Option<i32>::some(-2_i32), Option<u32>::some(4_u32))));
    auto n = Option<Tuple<i32, u32>>::none();
    auto nr = sus::move(n).unzip();
    static_assert(std::same_as<decltype(nr), Tuple<Option<i32>, Option<u32>>>);
    EXPECT_EQ(nr, (Tuple<Option<i32>, Option<u32>>::with(Option<i32>::none(),
                                                         Option<u32>::none())));
  }
  {
    auto i = NoCopyMove();
    auto u = 4_u32;
    auto s = Option<Tuple<NoCopyMove&, u32&>>::some(
        Tuple<NoCopyMove&, u32&>::with(i, u));
    auto sr = sus::move(s).unzip();
    static_assert(
        std::same_as<decltype(sr), Tuple<Option<NoCopyMove&>, Option<u32&>>>);
    EXPECT_EQ(sr, (Tuple<Option<NoCopyMove&>, Option<u32&>>::with(
                      Option<NoCopyMove&>::some(mref(i)),
                      Option<u32&>::some(mref(u)))));

    auto ci = NoCopyMove();
    auto cu = 4_u32;

    auto sc = Option<Tuple<const NoCopyMove&, const u32&>>::some(
        Tuple<const NoCopyMove&, const u32&>::with(ci, cu));
    auto scr = sus::move(sc).unzip();
    static_assert(
        std::same_as<decltype(scr),
                     Tuple<Option<const NoCopyMove&>, Option<const u32&>>>);

    auto n = Option<Tuple<NoCopyMove&, u32&>>::none();
    auto nr = sus::move(n).unzip();
    static_assert(
        std::same_as<decltype(nr), Tuple<Option<NoCopyMove&>, Option<u32&>>>);
    EXPECT_EQ(nr, (Tuple<Option<NoCopyMove&>, Option<u32&>>::with(
                      Option<NoCopyMove&>::none(), Option<u32&>::none())));
  }
}

TEST(Option, NonZeroField) {
  using T = sus::mem::NonNull<int>;
  static_assert(sizeof(Option<T>) == sizeof(T));
  int i = 3;

  EXPECT_EQ(Option<T>::none(), None);
  EXPECT_EQ(Option<T>::some(T::with(i)), Some);

  EXPECT_EQ(Option<T>(static_cast<const Option<T>&>(Option<T>::none())), None);
  EXPECT_EQ(
      Option<T>(static_cast<const Option<T>&>(Option<T>::some(T::with(i)))),
      Some);
  auto o = Option<T>::none();
  o = static_cast<const Option<T>&>(Option<T>::some(T::with(i)));
  EXPECT_EQ(o, Some);

  EXPECT_EQ(Option<T>(Option<T>::none()), None);
  EXPECT_EQ(Option<T>(Option<T>::some(T::with(i))), Some);
  o = Option<T>::none();
  EXPECT_EQ(o, None);

  o.insert(T::with(i));
  EXPECT_EQ(o, Some);

  o.take();
  EXPECT_EQ(o, None);

  EXPECT_EQ(Option<T>::some(T::with(i)).unwrap().as_ref(), 3);

  EXPECT_EQ(o, None);
  EXPECT_EQ(o.get_or_insert(T::with(i)).as_ref(), 3);
  EXPECT_EQ(o, Some);

  o.take();
  EXPECT_EQ(o, None);
  EXPECT_EQ(o.get_or_insert_with([&i]() { return T::with(i); }).as_ref(), 3);
  EXPECT_EQ(o, Some);

  EXPECT_EQ(o.take().unwrap().as_ref(), 3);

  EXPECT_EQ(o, None);
  EXPECT_EQ(sus::move(o).and_opt(Option<T>::some(T::with(i))), None);
  EXPECT_EQ(o, None);

  o = Option<T>::some(T::with(i));
  EXPECT_EQ(sus::move(o).and_opt(Option<T>::some(T::with(i))), Some);
  EXPECT_EQ(o, None);

  o = Option<T>::some(T::with(i));
  EXPECT_EQ(sus::move(o).xor_opt(Option<T>::some(T::with(i))), None);
  EXPECT_EQ(o, None);

  o = Option<T>::some(T::with(i));
  EXPECT_EQ(sus::move(o).xor_opt(Option<T>::none()), Some);
  EXPECT_EQ(o, None);

  o = Option<T>::some(T::with(i));
  EXPECT_EQ(
      sus::move(o).zip(Option<T>::some(T::with(i))),
      (Option<Tuple<T, T>>::some(Tuple<T, T>::with(T::with(i), T::with(i)))));
  EXPECT_EQ(o, None);

  o = Option<T>::some(T::with(i));
  EXPECT_EQ(sus::move(o).zip(Option<T>::none()), None);
  EXPECT_EQ(o, None);

  o = Option<T>::none();
  EXPECT_EQ(sus::move(o).zip(Option<T>::none()), None);
  EXPECT_EQ(o, None);

  o = Option<T>::some(T::with(i));
  int j = 4;
  o.replace(T::with(j));
  EXPECT_EQ(o.unwrap_ref().as_ptr(), &j);
}

template <class T>
struct CollectSum {
  sus_clang_bug_54050(CollectSum(T sum) : sum(sum){});

  static constexpr CollectSum from_iter(
      sus::iter::IteratorBase<T>&& iter) noexcept {
    T sum = T();
    for (const T& t : iter) sum += t;
    return CollectSum(sum);
  }

  T sum;
};

TEST(Option, FromIter) {
  auto all_some = sus::Array<Option<usize>, 3>::with_values(
                      Option<usize>::some(1u), Option<usize>::some(2u),
                      Option<usize>::some(3u))
                      .into_iter()
                      .collect<Option<CollectSum<usize>>>();
  EXPECT_EQ(all_some, Some);
  EXPECT_EQ(all_some.unwrap_ref().sum, 1u + 2u + 3u);

  auto one_none = sus::Array<Option<usize>, 3>::with_values(
                      Option<usize>::some(1u), Option<usize>::none(),
                      Option<usize>::some(3u))
                      .into_iter()
                      .collect<Option<CollectSum<usize>>>();
  EXPECT_EQ(one_none, None);
}

struct CollectRefs {
  sus_clang_bug_54050(CollectRefs(sus::Vec<const NoCopyMove*> v)
                      : vec(sus::move(v)){});

  static CollectRefs from_iter(
      sus::iter::IteratorBase<const NoCopyMove&>&& iter) noexcept {
    auto v = sus::Vec<const NoCopyMove*>();
    for (const NoCopyMove& t : iter) v.push(&t);
    return CollectRefs(sus::move(v));
  }

  sus::Vec<const NoCopyMove*> vec;
};

TEST(Option, FromIterWithRefs) {
  auto u1 = NoCopyMove();
  auto u2 = NoCopyMove();
  auto u3 = NoCopyMove();

  auto all_some = sus::Array<Option<const NoCopyMove&>, 3>::with_values(
                      sus::some(u1), sus::some(u2), sus::some(u3))
                      .into_iter()
                      .collect<Option<CollectRefs>>();
  EXPECT_EQ(all_some, Some);
  EXPECT_EQ(all_some.unwrap_ref().vec[0u], &u1);
  EXPECT_EQ(all_some.unwrap_ref().vec[1u], &u2);
  EXPECT_EQ(all_some.unwrap_ref().vec[2u], &u3);

  auto one_none = sus::Array<Option<const NoCopyMove&>, 3>::with_values(
                      sus::some(u1), sus::none(), sus::some(u3))
                      .into_iter()
                      .collect<Option<CollectRefs>>();
  EXPECT_EQ(one_none, None);
}

TEST(Option, Clone) {
  struct Copy {
    Copy() {}
    Copy(const Copy& o) : i(o.i + 1_i32) {}
    Copy& operator=(const Copy&) = default;
    Copy(Copy&&) = delete;
    Copy& operator=(Copy&&) = delete;
    i32 i = 1_i32;
  };

  static_assert(sus::mem::Copy<Copy>);
  static_assert(sus::mem::Clone<Copy>);
  static_assert(sus::mem::CloneInto<Copy>);
  static_assert(!sus::mem::Move<Copy>);
  static_assert(sus::mem::Copy<Option<Copy>>);
  static_assert(sus::mem::Clone<Option<Copy>>);
  static_assert(sus::mem::CloneInto<Option<Copy>>);
  static_assert(!sus::mem::Move<Option<Copy>>);

  {
    const auto s = Option<Copy>::some(Copy());
    i32 i = s.unwrap_ref().i;
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Option<Copy>>);
    EXPECT_EQ(s2.unwrap_ref().i, i + 1_i32);
  }

  {
    const auto s = Option<Copy>::some(Copy());
    i32 i = s.unwrap_ref().i;
    auto s2 = Option<Copy>::some(Copy());
    s2.unwrap_mut().i = 1000_i32;
    sus::clone_into(mref(s2), s);
    EXPECT_EQ(s2.unwrap_ref().i, i);
  }

  struct Clone {
    Clone() {}
    Clone clone() const {
      auto c = Clone();
      c.i = i + 1_i32;
      return c;
    }

    Clone(Clone&&) = default;
    Clone& operator=(Clone&&) = default;

    i32 i = 1_i32;
  };

  static_assert(!sus::mem::Copy<Clone>);
  static_assert(sus::mem::Clone<Clone>);
  static_assert(!sus::mem::CloneInto<Clone>);
  static_assert(sus::mem::Move<Clone>);
  static_assert(!sus::mem::Copy<Option<Clone>>);
  static_assert(sus::mem::Clone<Option<Clone>>);
  static_assert(sus::mem::CloneInto<Option<Clone>>);
  static_assert(sus::mem::Move<Option<Clone>>);

  {
    const auto s = Option<Clone>::some(Clone());
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Option<Clone>>);
    EXPECT_EQ(s2.unwrap_ref().i, 2_i32);
  }

  {
    const auto s = Option<Clone>::some(Clone());
    auto s2 = Option<Clone>::some(Clone());
    s2.unwrap_mut().i = 1000_i32;
    sus::clone_into(mref(s2), s);
    EXPECT_EQ(s2.unwrap_ref().i, 2_i32);
  }

  {
    auto i = 1_i32;
    const auto s = Option<const i32&>::some(i);
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Option<const i32&>>);
    EXPECT_EQ(&s2.unwrap_ref(), &i);
  }

  {
    auto i = 1_i32;
    const auto s = Option<i32&>::some(mref(i));
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Option<i32&>>);
    EXPECT_EQ(&s2.unwrap_ref(), &i);
  }
}

}  // namespace
