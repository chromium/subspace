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

#include "assertions/builtin.h"
#include "iter/iterator.h"
#include "mem/__private/relocate.h"
#include "test/behaviour_types.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

using ::sus::concepts::MakeDefault;
using ::sus::mem::Mref;
using ::sus::mem::__private::relocate_array_by_memcpy_v;
using ::sus::mem::__private::relocate_one_by_memcpy_v;
using ::sus::option::None;
using ::sus::option::Option;
using ::sus::option::Some;
using namespace ::sus::test;

namespace {

#define IS_SOME(x)             \
  do {                         \
    EXPECT_TRUE(x.is_some());  \
    EXPECT_FALSE(x.is_none()); \
    switch (x) {               \
      case Some:               \
        break;                 \
      case None:               \
        ADD_FAILURE();         \
    }                          \
  } while (false)

#define IS_NONE(x)             \
  do {                         \
    EXPECT_TRUE(x.is_none());  \
    EXPECT_FALSE(x.is_some()); \
    switch (x) {               \
      case None:               \
        break;                 \
      case Some:               \
        ADD_FAILURE();         \
    }                          \
  } while (false)

template <class T, class U>
constexpr size_t max_sizeof() {
  return sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U);
}

static_assert(sizeof(Option<bool>) == sizeof(bool) + sizeof(bool), "");
static_assert(sizeof(Option<bool&>) == sizeof(bool*), "");
// An Option has space for T plus a bool, but it's size is rounded up to the
// alignment of T.
static_assert(sizeof(Option<int>) == sizeof(int) + max_sizeof<bool, int>(), "");
static_assert(sizeof(Option<int&>) == sizeof(int*), "");

template <class T, class = void, class... Args>
struct is_some_callable : std::false_type {};

template <class T, class... Args>
struct is_some_callable<
    T, std::void_t<decltype(T::some(std::declval<Args>()...))>, Args...>
    : std::true_type {};

template <class T, class... Args>
inline constexpr bool is_some_callable_v =
    is_some_callable<T, void, Args...>::value;

static_assert(is_some_callable_v<Option<int>, int>, "");
static_assert(is_some_callable_v<Option<int>, const int>, "");
static_assert(is_some_callable_v<Option<int>, int&>, "");
static_assert(is_some_callable_v<Option<int>, int&&>, "");
static_assert(is_some_callable_v<Option<int>, Mref<int&>>, "");
static_assert(is_some_callable_v<Option<int>, const int&>, "");

static_assert(!is_some_callable_v<Option<int&>, int>, "");
static_assert(!is_some_callable_v<Option<int&>, const int>, "");
static_assert(!is_some_callable_v<Option<int&>, int&>, "");
static_assert(!is_some_callable_v<Option<int&>, int&>, "");
static_assert(is_some_callable_v<Option<int&>, Mref<int&>>, "");
static_assert(!is_some_callable_v<Option<int&>, const int&>, "");

static_assert(is_some_callable_v<Option<const int>, int>, "");
static_assert(is_some_callable_v<Option<const int>, const int>, "");
static_assert(is_some_callable_v<Option<const int>, int&>, "");
static_assert(is_some_callable_v<Option<const int>, int&&>, "");
static_assert(is_some_callable_v<Option<const int>, Mref<int&>>, "");
static_assert(is_some_callable_v<Option<const int>, const int&>, "");

static_assert(is_some_callable_v<Option<const int&>, int>, "");
static_assert(is_some_callable_v<Option<const int&>, const int>, "");
static_assert(is_some_callable_v<Option<const int&>, int&>, "");
static_assert(is_some_callable_v<Option<const int&>, int&&>, "");
static_assert(is_some_callable_v<Option<const int&>, Mref<int&>>, "");
static_assert(is_some_callable_v<Option<const int&>, const int&>, "");

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
    using T = WithDefaultConstructible;
    auto x = Option<T>::some(T::with_default());
    auto y = Option<T>::none();
    auto t = T::with_default();
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
  auto y = static_cast<decltype(x)&&>(x);
  IS_SOME(y);
  x = static_cast<decltype(y)&&>(y);
  IS_SOME(x);

  struct MoveableLvalue {
    MoveableLvalue(int i) : i(i) {}
    MoveableLvalue(const MoveableLvalue& m) : i(m.i) {}
    MoveableLvalue(MoveableLvalue&& m) : i(m.i) { m.i = 0; }
    int i;
  };
  MoveableLvalue lvalue(2);
  auto a = Option<MoveableLvalue>::some(lvalue);
  EXPECT_EQ(a.as_ref().unwrap().i, 2);
  EXPECT_EQ(lvalue.i, 2);

  auto b =
      Option<MoveableLvalue>::some(static_cast<decltype(lvalue)&&>(lvalue));
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
  auto y = static_cast<decltype(x)&&>(x);
  IS_SOME(x);  // Trivially relocatable leaves `x` fully in tact.
  EXPECT_EQ(x.as_ref().unwrap().i, 1);
  x = static_cast<decltype(y)&&>(y);
  IS_SOME(y);  // Trivially relocatable leaves `y` fully in tact.
  EXPECT_EQ(y.as_ref().unwrap().i, 1);

  auto a = Option<NotTriviallyRelocatableCopyableOrMoveable>::some(
      NotTriviallyRelocatableCopyableOrMoveable(1));
  auto b = static_cast<decltype(a)&&>(a);
  IS_NONE(a);  // Not trivially relocatable moves-from the value in `a`.
  a = static_cast<decltype(b)&&>(b);
  IS_NONE(b);  // Not trivially relocatable moves-from the value in `b`.
}

TEST(Option, Some) {
  auto x = Option<DefaultConstructible>::some(DefaultConstructible());
  IS_SOME(x);

  auto y = Option<NotDefaultConstructible>::some(NotDefaultConstructible(3));
  IS_SOME(y);

  int i = 2;
  auto ix = Option<int&>::some(mref(i));
  IS_SOME(ix);

  constexpr auto cx(
      Option<DefaultConstructible>::some(DefaultConstructible()).unwrap());
  static_assert(cx.i == 2, "");

  constexpr auto cy(
      Option<NotDefaultConstructible>::some(NotDefaultConstructible(3))
          .unwrap());
  static_assert(cy.i == 3, "");
}

TEST(Option, None) {
  auto x = Option<DefaultConstructible>::none();
  IS_NONE(x);

  auto y = Option<NotDefaultConstructible>::none();
  IS_NONE(y);

  auto ix = Option<int&>::none();
  IS_NONE(ix);

  constexpr auto cx(Option<DefaultConstructible>::none());
  static_assert(cx.is_none(), "");

  constexpr auto cy(Option<NotDefaultConstructible>::none());
  static_assert(cy.is_none(), "");
}

TEST(Option, WithDefault) {
  auto x = Option<DefaultConstructible>::with_default();
  IS_SOME(x);
  EXPECT_EQ(static_cast<decltype(x)&&>(x).unwrap().i, 2);

  auto y = Option<WithDefaultConstructible>::with_default();
  IS_SOME(y);
  EXPECT_EQ(static_cast<decltype(y)&&>(y).unwrap().i, 3);

  constexpr auto cx(Option<DefaultConstructible>::with_default());
  static_assert(cx.is_some(), "");

  constexpr auto cy(Option<WithDefaultConstructible>::with_default());
  static_assert(cy.is_some(), "");

  auto x2 = MakeDefault<Option<DefaultConstructible>>::make_default();
  IS_SOME(x2);
  EXPECT_EQ(static_cast<decltype(x2)&&>(x2).unwrap().i, 2);

  auto y2 = MakeDefault<Option<WithDefaultConstructible>>::make_default();
  IS_SOME(y2);
  EXPECT_EQ(static_cast<decltype(y2)&&>(y2).unwrap().i, 3);
}

TEST(Option, Destructor) {
  static int count = 0;
  struct WatchDestructor {
    ~WatchDestructor() { ++count; }
  };
  {
    auto x = Option<WatchDestructor>::with_default();
    count = 0;  // Without optimizations, moves may run a destructor.
  }
  EXPECT_EQ(1, count);

  WatchDestructor w;
  {
    auto x = Option<WatchDestructor&>::some(mref(w));
    count = 0;
  }
  EXPECT_EQ(0, count);
}

TEST(Option, Clear) {
  static int count = 0;
  struct WatchDestructor {
    ~WatchDestructor() { ++count; }
  };
  {
    auto x = Option<WatchDestructor>::with_default();
    count = 0;  // Without optimizations, moves may run a destructor.
    x.clear();
    IS_NONE(x);
    EXPECT_EQ(count, 1);
  }
  EXPECT_EQ(count, 1);

  WatchDestructor w;
  {
    auto x = Option<WatchDestructor&>::some(mref(w));
    count = 0;
    x.clear();
    IS_NONE(x);
  }
  EXPECT_EQ(count, 0);
}

TEST(Option, ExpectSome) {
  auto x = Option<int>::with_default().expect("hello world");
  EXPECT_EQ(x, 0);

  int i;
  auto& xi = Option<int&>::some(mref(i)).expect("hello world");
  EXPECT_EQ(&xi, &i);
}

TEST(OptionDeathTest, ExpectNone) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(Option<int>::none().expect("hello world"), "hello world");
  EXPECT_DEATH(Option<int&>::none().expect("hello world"), "hello world");
#endif
}

TEST(Option, UnwrapSome) {
  auto x = Option<int>::with_default().unwrap();
  EXPECT_EQ(x, 0);

  int i;
  auto& ix = Option<int&>::some(mref(i)).unwrap();
  EXPECT_EQ(&ix, &i);
}

TEST(OptionDeathTest, UnwrapNone) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(Option<int>::none().unwrap(), "");
  EXPECT_DEATH(Option<int&>::none().unwrap(), "");
#endif
}

TEST(Option, UnwrapUncheckedSome) {
  auto x = Option<int>::with_default().unwrap_unchecked(unsafe_fn);
  EXPECT_EQ(x, 0);

  int i;
  auto& ix = Option<int&>::some(mref(i)).unwrap_unchecked(unsafe_fn);
  EXPECT_EQ(&ix, &i);
}

TEST(Option, Take) {
  auto x = Option<int>::some(404);
  auto y = x.take();
  // The value has moved from `x` to `y`.
  IS_NONE(x);
  IS_SOME(y);
  EXPECT_EQ(static_cast<decltype(y)&&>(y).unwrap(), 404);

  auto n = Option<int>::none();
  auto m = n.take();
  // The None has moved from `n` to `m`, which is a no-op on `n`.
  IS_NONE(n);
  IS_NONE(m);

  int i;
  auto ix = Option<int&>::some(mref(i));
  auto iy = ix.take();
  IS_NONE(ix);
  IS_SOME(iy);
  EXPECT_EQ(&iy.as_ref().unwrap(), &i);

  auto in = Option<int&>::none();
  auto im = in.take();
  IS_NONE(in);
  IS_NONE(im);
}

TEST(Option, UnwrapOr) {
  auto x = Option<int>::some(2).unwrap_or(3);
  EXPECT_EQ(x, 2);
  auto y = Option<int>::none().unwrap_or(3);
  EXPECT_EQ(y, 3);

  int i, i2;
  auto& ix = Option<int&>::some(mref(i)).unwrap_or(i2);
  EXPECT_EQ(&ix, &i);

  auto& iy = Option<int&>::none().unwrap_or(i2);
  EXPECT_EQ(&iy, &i2);

  // Verify constexpr.
  static_assert(Option<int>::none().unwrap_or(3) == 3, "");
  constexpr int ci = 2;
  static_assert(Option<const int&>::none().unwrap_or(ci) == 2, "");
}

TEST(Option, UnwrapOrElse) {
  auto x = Option<int>::some(2).unwrap_or_else([]() { return int(3); });
  EXPECT_EQ(x, 2);
  auto y = Option<int>::none().unwrap_or_else([]() { return int(3); });
  EXPECT_EQ(y, 3);

  int i, i2;
  auto& ix =
      Option<int&>::some(mref(i)).unwrap_or_else([&]() -> int& { return i2; });
  EXPECT_EQ(&ix, &i);

  auto& iy = Option<int&>::none().unwrap_or_else([&]() -> int& { return i2; });
  EXPECT_EQ(&iy, &i2);

  // Verify constexpr.
  static_assert(
      Option<int>::none().unwrap_or_else([]() { return int(3); }) == 3, "");
  // TODO: Why does this not work?
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

  auto wx = Option<WithDefaultConstructible>::some(WithDefaultConstructible(4))
                .unwrap_or_default();
  EXPECT_EQ(wx.i, 4);
  auto wy = Option<WithDefaultConstructible>::none().unwrap_or_default();
  EXPECT_EQ(wy.i, 3);

  // Verify constexpr.
  static_assert(Option<int>::none().unwrap_or_default() == 0, "");
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
  static_assert(std::is_same_v<decltype(x), Option<Mapped>>, "");
  EXPECT_EQ(static_cast<decltype(x)&&>(x).unwrap().i, 3);
  EXPECT_TRUE(called);

  called = false;
  auto y = Option<int>::none().map([&](int&& i) {
    called = true;
    return Mapped(i + 1);
  });
  static_assert(std::is_same_v<decltype(y), Option<Mapped>>, "");
  IS_NONE(y);
  EXPECT_FALSE(called);

  called = false;
  int i = 2;
  auto ix = Option<int&>::some(mref(i)).map([&](int& i) {
    called = true;
    return Mapped(i + 1);
  });
  static_assert(std::is_same_v<decltype(ix), Option<Mapped>>, "");
  EXPECT_EQ(ix.as_ref().unwrap().i, 3);
  EXPECT_TRUE(called);

  called = false;
  auto iy = Option<int&>::none().map([&](int& i) {
    called = true;
    return Mapped(3);
  });
  static_assert(std::is_same_v<decltype(iy), Option<Mapped>>, "");
  IS_NONE(iy);
  EXPECT_FALSE(called);

  // Verify constexpr.
  static_assert(Option<int>::some(2)
                        .map([](int&& i) { return Mapped(i + 1); })
                        .unwrap()
                        .i == 3,
                "");
  constexpr int ci = 2;
  static_assert(Option<const int&>::some(ci)
                        .map([](const int& i) { return Mapped(i + 1); })
                        .unwrap()
                        .i == 3,
                "");
}

TEST(Option, MapOr) {
  struct Mapped {
    int i;
  };

  auto x = Option<int>::some(2).map_or(
      Mapped(4), [](int&& i) { return static_cast<Mapped>(i + 1); });
  static_assert(std::is_same_v<decltype(x), Option<Mapped>>, "");
  EXPECT_EQ(static_cast<decltype(x)&&>(x).unwrap().i, 3);

  auto y = Option<int>::none().map_or(Mapped(4),
                                      [](int&& i) { return Mapped(i + 1); });
  static_assert(std::is_same_v<decltype(y), Option<Mapped>>, "");
  EXPECT_EQ(static_cast<decltype(y)&&>(y).unwrap().i, 4);

  int i = 2;
  auto ix = Option<int&>::some(mref(i)).map_or(
      Mapped(4), [](int& i) { return static_cast<Mapped>(i + 1); });
  static_assert(std::is_same_v<decltype(ix), Option<Mapped>>, "");
  EXPECT_EQ(ix.as_ref().unwrap().i, 3);

  auto iy = Option<int&>::none().map_or(
      Mapped(4), [](int& i) { return static_cast<Mapped>(i + 1); });
  static_assert(std::is_same_v<decltype(ix), Option<Mapped>>, "");
  EXPECT_EQ(iy.as_ref().unwrap().i, 4);

  // Verify constexpr.
  static_assert(
      Option<int>::none().map_or(4, [](int&& i) { return 1; }).unwrap() == 4,
      "");
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
  static_assert(std::is_same_v<decltype(x), Option<Mapped>>, "");
  EXPECT_EQ(static_cast<decltype(x)&&>(x).unwrap().i, 3);
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
  static_assert(std::is_same_v<decltype(y), Option<Mapped>>, "");
  EXPECT_EQ(static_cast<decltype(y)&&>(y).unwrap().i, 4);
  EXPECT_FALSE(map_called);
  EXPECT_TRUE(else_called);

  int i = 2;
  map_called = else_called = false;
  auto ix = Option<int&>::some(mref(i)).map_or_else(
      [&]() {
        else_called = true;
        return Mapped(4);
      },
      [&](int& i) {
        map_called = true;
        return Mapped(i + 1);
      });
  static_assert(std::is_same_v<decltype(ix), Option<Mapped>>, "");
  EXPECT_EQ(ix.as_ref().unwrap().i, 3);
  EXPECT_TRUE(map_called);
  EXPECT_FALSE(else_called);

  map_called = else_called = false;
  auto iy = Option<int&>::none().map_or_else(
      [&]() {
        else_called = true;
        return Mapped(4);
      },
      [&](int& i) {
        map_called = true;
        return Mapped(i + 1);
      });
  static_assert(std::is_same_v<decltype(iy), Option<Mapped>>, "");
  EXPECT_EQ(iy.as_ref().unwrap().i, 4);
  EXPECT_FALSE(map_called);
  EXPECT_TRUE(else_called);

  // Verify constexpr.
  static_assert(Option<int>::none()
                        .map_or_else([]() { return Mapped(4); },
                                     [](int&& i) { return Mapped(1); })
                        .unwrap()
                        .i == 4,
                "");
  static_assert(Option<int>::some(2)
                        .map_or_else([]() { return Mapped(4); },
                                     [](int&& i) { return Mapped(1); })
                        .unwrap()
                        .i == 1,
                "");
  constexpr int ci = 2;
  static_assert(Option<const int&>::none()
                        .map_or_else([]() { return Mapped(4); },
                                     [](const int& i) { return Mapped(1); })
                        .unwrap()
                        .i == 4,
                "");
  static_assert(Option<const int&>::some(ci)
                        .map_or_else([]() { return Mapped(4); },
                                     [](const int& i) { return Mapped(i + 1); })
                        .unwrap()
                        .i == 3,
                "");
}

TEST(Option, Filter) {
  auto x = Option<int>::some(2).filter([](const int& i) { return true; });
  static_assert(std::is_same_v<decltype(x), Option<int>>, "");
  IS_SOME(x);

  auto y = Option<int>::some(2).filter([](const int& i) { return false; });
  static_assert(std::is_same_v<decltype(y), Option<int>>, "");
  IS_NONE(y);

  auto nx = Option<int>::none().filter([](const int& i) { return true; });
  static_assert(std::is_same_v<decltype(nx), Option<int>>, "");
  IS_NONE(nx);

  auto ny = Option<int>::none().filter([](const int& i) { return false; });
  static_assert(std::is_same_v<decltype(ny), Option<int>>, "");
  IS_NONE(ny);

  int i = 2;
  auto ix =
      Option<int&>::some(mref(i)).filter([](const int& i) { return true; });
  static_assert(std::is_same_v<decltype(ix), Option<int&>>, "");
  IS_SOME(ix);

  auto iy =
      Option<int&>::some(mref(i)).filter([](const int& i) { return false; });
  static_assert(std::is_same_v<decltype(iy), Option<int&>>, "");
  IS_NONE(iy);

  auto inx = Option<int&>::none().filter([](const int& i) { return true; });
  static_assert(std::is_same_v<decltype(inx), Option<int&>>, "");
  IS_NONE(inx);

  auto iny = Option<int&>::none().filter([](const int& i) { return false; });
  static_assert(std::is_same_v<decltype(iny), Option<int&>>, "");
  IS_NONE(iny);

  // Verify constexpr.
  static_assert(
      Option<int>::some(2).filter([](const int& i) { return true; }).unwrap() ==
          2,
      "");
  constexpr int ci = 2;
  static_assert(Option<const int&>::some(ci)
                        .filter([](const int& i) { return true; })
                        .unwrap() == 2,
                "");

  static int count = 0;
  struct WatchDestructor {
    ~WatchDestructor() { ++count; }
  };

  static int hold_count;
  {
    auto a = Option<WatchDestructor>::with_default();
    count = 0;
    auto af = static_cast<decltype(a)&&>(a).filter(
        [](const WatchDestructor& i) { return true; });
    // The WatchDestructor was moved from `a` to `af` and the temporary's
    // was destroyed.
    EXPECT_GE(count, 1);
    hold_count = count;
  }
  // No double destruction in `a`, but there's one in `af`.
  EXPECT_EQ(count, hold_count + 1);

  {
    auto b = Option<WatchDestructor>::with_default();
    count = 0;
    auto bf = static_cast<decltype(b)&&>(b).filter(
        [](const WatchDestructor& i) { return false; });
    // The WatchDestructor in `b` was destroyed.
    EXPECT_GE(count, 1);
  }
  // No double destruction in `b`.
  EXPECT_GE(count, 1);

  {
    count = 0;
    auto c = Option<WatchDestructor>::none();
    auto cf = static_cast<decltype(c)&&>(c).filter(
        [](const WatchDestructor& i) { return false; });
    // Nothing constructed or destructed.
    EXPECT_EQ(count, 0);
  }
  EXPECT_EQ(count, 0);

  WatchDestructor w;
  {
    count = 0;
    auto c = Option<WatchDestructor&>::some(mref(w));
    auto cf = static_cast<decltype(c)&&>(c).filter(
        [](const WatchDestructor& i) { return false; });
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

  int i2 = 2, i3 = 3;
  auto ix = Option<int&>::some(mref(i2))
                .and_opt(Option<int&>::some(mref(i3)))
                .unwrap();
  EXPECT_EQ(ix, 3);

  auto iy = Option<int&>::some(mref(i2)).and_opt(Option<int&>::none());
  IS_NONE(iy);

  auto inx = Option<int&>::none().and_opt(Option<int&>::some(mref(i3)));
  IS_NONE(inx);

  auto iny = Option<int&>::none().and_opt(Option<int&>::none());
  IS_NONE(iny);
}

TEST(Option, AndThen) {
  struct And {
    int i;
  };

  bool called = false;
  auto x = Option<int>::some(2).and_then([&](int&& i) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(x), Option<And>>, "");
  EXPECT_EQ(static_cast<decltype(x)&&>(x).unwrap().i, 3);
  EXPECT_TRUE(called);

  called = false;
  auto y = Option<int>::some(2).and_then([&](int&& i) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(y), Option<And>>, "");
  IS_NONE(y);
  EXPECT_TRUE(called);

  called = false;
  auto nx = Option<int>::none().and_then([&](int&& i) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(nx), Option<And>>, "");
  IS_NONE(nx);
  EXPECT_FALSE(called);

  called = false;
  auto ny = Option<int>::none().and_then([&](int&& i) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(ny), Option<And>>, "");
  IS_NONE(ny);
  EXPECT_FALSE(called);

  int i = 2;

  called = false;
  auto ix = Option<int&>::some(mref(i)).and_then([&](int& i) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(ix), Option<And>>, "");
  EXPECT_EQ(ix.as_ref().unwrap().i, 3);
  EXPECT_TRUE(called);

  called = false;
  auto iy = Option<int&>::some(mref(i)).and_then([&](int& i) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(iy), Option<And>>, "");
  IS_NONE(iy);
  EXPECT_TRUE(called);

  called = false;
  auto inx = Option<int&>::none().and_then([&](int& i) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(inx), Option<And>>, "");
  IS_NONE(inx);
  EXPECT_FALSE(called);

  called = false;
  auto iny = Option<int&>::none().and_then([&](int& i) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(iny), Option<And>>, "");
  IS_NONE(iny);
  EXPECT_FALSE(called);

  // Verify constexpr.
  constexpr auto cx =
      Option<int>::some(2)
          .and_then([&](int&& i) { return Option<And>::some(And(3)); })
          .unwrap();
  static_assert(cx.i == 3, "");
  constexpr int ci = 2;
  constexpr auto icx =
      Option<const int&>::some(ci)
          .and_then([&](const int& i) { return Option<And>::some(And(3)); })
          .unwrap();
  static_assert(icx.i == 3, "");
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

  int i2 = 2, i3 = 3;

  auto ix = Option<int&>::some(mref(i2))
                .or_opt(Option<int&>::some(mref(i3)))
                .unwrap();
  EXPECT_EQ(ix, 2);

  auto iy = Option<int&>::some(mref(i2)).or_opt(Option<int&>::none()).unwrap();
  EXPECT_EQ(iy, 2);

  auto inx = Option<int&>::none().or_opt(Option<int&>::some(mref(i3))).unwrap();
  EXPECT_EQ(inx, 3);

  auto iny = Option<int&>::none().or_opt(Option<int&>::none());
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

  int i2 = 2, i3 = 3;

  called = false;
  auto ix = Option<int&>::some(mref(i2))
                .or_else([&]() {
                  called = true;
                  return Option<int&>::some(mref(i3));
                })
                .unwrap();
  EXPECT_EQ(ix, 2);
  EXPECT_FALSE(called);

  called = false;
  auto iy = Option<int&>::some(mref(i2))
                .or_else([&]() {
                  called = true;
                  return Option<int&>::none();
                })
                .unwrap();
  EXPECT_EQ(iy, 2);
  EXPECT_FALSE(called);

  called = false;
  auto inx = Option<int&>::none()
                 .or_else([&]() {
                   called = true;
                   return Option<int&>::some(mref(i3));
                 })
                 .unwrap();
  EXPECT_EQ(inx, 3);
  EXPECT_TRUE(called);

  called = false;
  auto iny = Option<int&>::none().or_else([&]() {
    called = true;
    return Option<int&>::none();
  });
  IS_NONE(iny);
  EXPECT_TRUE(called);

  // Verify constexpr.
  constexpr auto cx = Option<int>::some(2)
                          .or_else([&]() { return Option<int>::some(3); })
                          .unwrap();
  static_assert(cx == 2, "");
  // TODO: Why does this not work?
  // constexpr int ci2 = 2, ci3 = 3;
  // constexpr auto icx = Option<const int&>::some(ci2)
  //                         .or_else([&]() { return Option<const
  //                         int&>::some(ci3); }) .unwrap();
  // static_assert(icx == 2, "");
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

  int i2 = 2, i3 = 3;

  auto ix = Option<int&>::some(mref(i2)).xor_opt(Option<int&>::some(mref(i3)));
  IS_NONE(ix);

  auto iy = Option<int&>::some(mref(i2)).xor_opt(Option<int&>::none()).unwrap();
  EXPECT_EQ(iy, 2);

  auto inx =
      Option<int&>::none().xor_opt(Option<int&>::some(mref(i3))).unwrap();
  EXPECT_EQ(inx, 3);

  auto iny = Option<int&>::none().xor_opt(Option<int&>::none());
  IS_NONE(iny);
}

TEST(Option, Insert) {
  auto x = Option<int>::none();
  x.insert(3);
  EXPECT_EQ(x.as_ref().unwrap(), 3);

  auto y = Option<int>::some(4);
  y.insert(5);
  EXPECT_EQ(y.as_ref().unwrap(), 5);

  int i2 = 2, i3 = 3;

  auto ix = Option<int&>::none();
  ix.insert(i2);
  EXPECT_EQ(ix.as_ref().unwrap(), 2);

  auto iy = Option<int&>::some(mref(i2));
  iy.insert(i3);
  EXPECT_EQ(iy.as_ref().unwrap(), 3);
}

TEST(Option, GetOrInsert) {
  auto x = Option<int>::none();
  auto& rx = x.get_or_insert(9);
  static_assert(std::is_same_v<decltype(rx), int&>, "");
  EXPECT_EQ(rx, 9);
  rx = 5;
  EXPECT_EQ(static_cast<decltype(x)&&>(x).unwrap(), 5);

  auto y = Option<int>::some(11);
  auto& ry = y.get_or_insert(7);
  static_assert(std::is_same_v<decltype(ry), int&>, "");
  EXPECT_EQ(ry, 11);
  EXPECT_EQ(static_cast<decltype(y)&&>(y).unwrap(), 11);

  int i2 = 2, i3 = 3;

  auto ix = Option<int&>::none();
  auto& irx = ix.get_or_insert(i3);
  static_assert(std::is_same_v<decltype(irx), int&>, "");
  EXPECT_EQ(&irx, &i3);
  EXPECT_EQ(&ix.as_ref().unwrap(), &i3);

  auto iy = Option<int&>::some(mref(i2));
  auto& iry = iy.get_or_insert(i3);
  static_assert(std::is_same_v<decltype(iry), int&>, "");
  EXPECT_EQ(&iry, &i2);
  EXPECT_EQ(&iy.as_ref().unwrap(), &i2);
}

TEST(Option, GetOrInsertDefault) {
  auto x = Option<DefaultConstructible>::none();
  auto& rx = x.get_or_insert_default();
  static_assert(std::is_same_v<decltype(rx), DefaultConstructible&>, "");
  EXPECT_EQ(rx.i, 2);
  IS_SOME(x);
  EXPECT_EQ(static_cast<decltype(x)&&>(x).unwrap().i, 2);

  auto w = Option<WithDefaultConstructible>::none();
  auto& rw = w.get_or_insert_default();
  static_assert(std::is_same_v<decltype(rw), WithDefaultConstructible&>, "");
  EXPECT_EQ(rw.i, 3);
  IS_SOME(w);
  EXPECT_EQ(static_cast<decltype(w)&&>(w).unwrap().i, 3);

  auto y = Option<DefaultConstructible>::some(DefaultConstructible(404));
  auto& ry = y.get_or_insert_default();
  static_assert(std::is_same_v<decltype(ry), DefaultConstructible&>, "");
  EXPECT_EQ(ry.i, 404);
  IS_SOME(y);
  EXPECT_EQ(static_cast<decltype(y)&&>(y).unwrap().i, 404);
}

TEST(Option, GetOrInsertWith) {
  bool called = false;
  auto x = Option<int>::none();
  auto& rx = x.get_or_insert_with([&]() {
    called = true;
    return 9;
  });
  static_assert(std::is_same_v<decltype(rx), int&>, "");
  EXPECT_EQ(rx, 9);
  rx = 12;
  EXPECT_TRUE(called);
  IS_SOME(x);
  EXPECT_EQ(static_cast<decltype(x)&&>(x).unwrap(), 12);

  called = false;
  auto y = Option<int>::some(11);
  auto& ry = y.get_or_insert_with([&]() {
    called = true;
    return 7;
  });
  static_assert(std::is_same_v<decltype(ry), int&>, "");
  EXPECT_EQ(ry, 11);
  ry = 18;
  EXPECT_FALSE(called);
  IS_SOME(y);
  EXPECT_EQ(static_cast<decltype(y)&&>(y).unwrap(), 18);

  int i2 = 2, i3 = 3;

  called = false;
  auto ix = Option<int&>::none();
  auto& irx = ix.get_or_insert_with([&]() -> int& {
    called = true;
    return i3;
  });
  static_assert(std::is_same_v<decltype(irx), int&>, "");
  EXPECT_TRUE(called);
  EXPECT_EQ(&irx, &i3);
  EXPECT_EQ(&ix.as_ref().unwrap(), &i3);

  called = false;
  auto iy = Option<int&>::some(mref(i2));
  auto& iry = iy.get_or_insert_with([&]() -> int& {
    called = true;
    return i3;
  });
  static_assert(std::is_same_v<decltype(iry), int&>, "");
  EXPECT_FALSE(called);
  EXPECT_EQ(&iry, &i2);
  EXPECT_EQ(&iy.as_ref().unwrap(), &i2);
}

TEST(Option, AsRef) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.as_ref()), Option<const int&>>, "");
  EXPECT_EQ(&x.get_or_insert(0), &x.as_ref().unwrap());

  auto n = Option<int>::none();
  IS_NONE(n.as_ref());

  int i = 2;

  auto ix = Option<int&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.as_ref()), Option<const int&>>, "");
  EXPECT_EQ(&i, &ix.as_ref().unwrap());

  auto in = Option<int&>::none();
  IS_NONE(in.as_ref());

  // Verify constexpr.
  constexpr auto cx = Option<int>::some(3);
  static_assert(cx.as_ref().unwrap() == 3, "");
  constexpr int ci = 2;
  constexpr auto icx = Option<int>::some(ci);
  static_assert(icx.as_ref().unwrap() == 2, "");
}

TEST(Option, UnwrapRefSome) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.unwrap_ref()), const int&>, "");
  EXPECT_EQ(&x.unwrap_ref(), &x.as_ref().unwrap());

  int i = 2;

  auto ix = Option<int&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.unwrap_ref()), const int&>, "");
  EXPECT_EQ(&ix.unwrap_ref(), &ix.as_ref().unwrap());

  // Verify constexpr.
  constexpr auto cx = Option<int>::some(3);
  static_assert(cx.unwrap_ref() == 3, "");
}

TEST(OptionDeathTest, UnwrapRefNone) {
  auto n = Option<int>::none();
  auto in = Option<int&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(n.unwrap_ref(), "");
  EXPECT_DEATH(in.unwrap_ref(), "");
#endif
}

TEST(Option, ExpectRefSome) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.expect_ref("")), const int&>, "");
  EXPECT_EQ(&x.expect_ref(""), &x.as_ref().unwrap());

  int i = 2;

  auto ix = Option<int&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.expect_ref("")), const int&>, "");
  EXPECT_EQ(&ix.expect_ref(""), &ix.as_ref().unwrap());

  // Verify constexpr.
  static_assert(Option<int>::some(3).expect_ref("") == 3, "");
  constexpr int ci = 2;
  static_assert(Option<const int&>::some(ci).expect_ref("") == 2, "");
}

TEST(OptionDeathTest, ExpectRefNone) {
  auto n = Option<int>::none();
  auto in = Option<int&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(n.expect_ref("hello world"), "hello world");
  EXPECT_DEATH(in.expect_ref("hello world"), "hello world");
#endif
}

TEST(Option, AsMut) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.as_mut()), Option<int&>>, "");
  EXPECT_EQ(&x.get_or_insert(0), &x.as_mut().unwrap());

  auto n = Option<int>::none();
  IS_NONE(n.as_mut());

  int i = 2;

  auto ix = Option<int&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.as_mut()), Option<int&>>, "");
  EXPECT_EQ(&i, &ix.as_mut().unwrap());

  auto in = Option<int&>::none();
  IS_NONE(in.as_mut());
}

TEST(Option, UnwrapMutSome) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.unwrap_mut()), int&>, "");
  EXPECT_EQ(&x.unwrap_mut(), &x.as_mut().unwrap());

  int i = 2;

  auto ix = Option<int&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.unwrap_mut()), int&>, "");
  EXPECT_EQ(&ix.unwrap_mut(), &i);
}

TEST(OptionDeathTest, UnwrapMutNone) {
  auto n = Option<int>::none();
  auto in = Option<int&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(n.unwrap_mut(), "");
  EXPECT_DEATH(in.unwrap_mut(), "");
#endif
}

TEST(Option, ExpectMutSome) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.expect_mut("")), int&>, "");
  EXPECT_EQ(&x.expect_mut(""), &x.as_mut().unwrap());

  int i = 2;

  auto ix = Option<int&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.expect_mut("")), int&>, "");
  EXPECT_EQ(&ix.expect_mut(""), &i);
}

TEST(OptionDeathTest, ExpectMutNone) {
  auto n = Option<int>::none();
  auto in = Option<int&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(n.expect_mut("hello world"), "hello world");
  EXPECT_DEATH(in.expect_mut("hello world"), "hello world");
#endif
}

TEST(Option, TrivialMove) {
  auto x = Option<sus::test::TriviallyMoveableAndRelocatable>::some(
      sus::test::TriviallyMoveableAndRelocatable(int(3423782)));
  auto y = static_cast<decltype(x)&&>(x);  // Move-construct.
  EXPECT_EQ(y.as_ref().unwrap().i, 3423782);

  y.as_mut().unwrap().i = int(6589043);
  x = static_cast<decltype(y)&&>(y);  // Move-assign.
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

  int i = 2;

  auto ix = Option<int&>::some(mref(i));
  auto iy = static_cast<decltype(ix)&&>(ix);  // Move-construct.
  EXPECT_EQ(&iy.as_ref().unwrap(), &i);
  ix = static_cast<decltype(iy)&&>(iy);  // Move-assign.
  EXPECT_EQ(&ix.as_ref().unwrap(), &i);
  auto iz = ix;  // Copy-construct.
  EXPECT_EQ(&ix.as_ref().unwrap(), &i);
  EXPECT_EQ(&iz.as_ref().unwrap(), &i);
  auto izz = Option<int&>::none();
  izz = iz;  // Copy-assign.
  EXPECT_EQ(&iz.as_ref().unwrap(), &i);
  EXPECT_EQ(&izz.as_ref().unwrap(), &i);
}

TEST(Option, Replace) {
  auto x = Option<int>::some(2);
  static_assert(std::is_same_v<decltype(x.replace(3)), Option<int>>, "");
  auto y = x.replace(3);
  EXPECT_EQ(x.as_ref().unwrap(), 3);
  EXPECT_EQ(y.as_ref().unwrap(), 2);

  auto z = Option<int>::none();
  static_assert(std::is_same_v<decltype(z.replace(3)), Option<int>>, "");
  auto zz = z.replace(3);
  EXPECT_EQ(z.as_ref().unwrap(), 3);
  IS_NONE(zz);

  int i2 = 2, i3 = 3;

  auto ix = Option<int&>::some(mref(i2));
  static_assert(std::is_same_v<decltype(ix.replace(i3)), Option<int&>>, "");
  auto iy = ix.replace(i3);
  EXPECT_EQ(&ix.as_ref().unwrap(), &i3);
  EXPECT_EQ(&iy.as_ref().unwrap(), &i2);

  auto iz = Option<int&>::none();
  static_assert(std::is_same_v<decltype(iz.replace(i3)), Option<int&>>, "");
  auto izz = iz.replace(i3);
  EXPECT_EQ(&iz.as_ref().unwrap(), &i3);
  IS_NONE(izz);
}

TEST(Option, Copied) {
  int i = 2;
  auto x = Option<int&>::none().copied();
  IS_NONE(x);

  auto y = Option<int&>::some(mref(i)).copied();
  EXPECT_EQ(y.as_ref().unwrap(), 2);
  EXPECT_NE(&y.as_ref().unwrap(), &i);

  // Verify constexpr.
  constexpr int ic = 2;
  static_assert(Option<int&>::none().copied().is_none(), "");
  static_assert(Option<const int&>::some(ic).copied().unwrap() == 2, "");
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

  // Verify constexpr.
  static_assert(Option<Option<int>>::none().flatten().is_none(), "");
  static_assert(
      Option<Option<int>>::some(Option<int>::none()).flatten().is_none(), "");
  static_assert(
      Option<Option<int>>::some(Option<int>::some(3)).flatten().unwrap() == 3,
      "");
}

TEST(Option, Iter) {
  auto x = Option<int>::none();
  for (auto i : x.iter()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Option<int>::some(2);
  for (auto& i : y.iter()) {
    static_assert(std::is_same_v<decltype(i), const int&>, "");
    EXPECT_EQ(i, 2);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

TEST(Option, IterMut) {
  auto x = Option<int>::none();
  for (auto i : x.iter_mut()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Option<int>::some(2);
  for (auto& i : y.iter_mut()) {
    static_assert(std::is_same_v<decltype(i), int&>, "");
    EXPECT_EQ(i, 2);
    i += 1;
    ++count;
  }
  EXPECT_EQ(y.as_ref().unwrap(), 3);
}

struct MoveOnly {
  explicit MoveOnly(int i) : i(i) {}
  MoveOnly(MoveOnly&& o) : i(o.i) {}
  void operator=(MoveOnly&& o) { i = o.i; }

  int i;
};

TEST(Option, IntoIter) {
  auto x = Option<int>::none();
  for (auto i : x.iter_mut()) {
    ADD_FAILURE();
  }

  int count = 0;
  auto y = Option<MoveOnly>::some(MoveOnly(2));
  for (auto m : static_cast<decltype(y)&&>(y).into_iter()) {
    static_assert(std::is_same_v<decltype(m), MoveOnly>, "");
    EXPECT_EQ(m.i, 2);
    ++count;
  }
  EXPECT_EQ(count, 1);
}

}  // namespace
