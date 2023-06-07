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

#include "subspace/option/option.h"

#include <sstream>

#include "fmt/std.h"
#include "googletest/include/gtest/gtest.h"
#include "subspace/assertions/endian.h"
#include "subspace/containers/array.h"
#include "subspace/iter/iterator.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/macros/builtin.h"
#include "subspace/mem/nonnull.h"
#include "subspace/mem/relocate.h"
#include "subspace/num/types.h"
#include "subspace/prelude.h"
#include "subspace/result/result.h"
#include "subspace/test/behaviour_types.h"
#include "subspace/test/from_i32.h"
#include "subspace/test/no_copy_move.h"
#include "subspace/tuple/tuple.h"

using sus::None;
using sus::Option;
using sus::Some;
using sus::Tuple;
using sus::construct::Default;
using sus::mem::relocate_by_memcpy;
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
  {
    auto a = Option<i32>::some(2_i32);
    Option<i32> a2 = sus::some(2_i32);
    EXPECT_EQ(a, a2);
    auto mv = 2_i32;
    a2 = sus::some(mref(mv));
    EXPECT_EQ(a, a2);
    const auto cv = 2_i32;
    a2 = sus::some(cv);
    EXPECT_EQ(a, a2);

    auto b = Option<i32>::none();
    Option<i32> b2 = sus::none();
    EXPECT_EQ(b, b2);

    auto i = NoCopyMove();
    auto c = Option<NoCopyMove&>::some(i);
    Option<NoCopyMove&> c2 = sus::some(i);
    EXPECT_EQ(&c.as_ref().unwrap(), &c2.as_ref().unwrap());

    const auto ci = NoCopyMove();
    const auto cc = Option<const NoCopyMove&>::some(ci);
    Option<const NoCopyMove&> cc2 = sus::some(ci);
    EXPECT_EQ(&cc.as_ref().unwrap(), &cc2.as_ref().unwrap());

    Option<const NoCopyMove&> mut_to_const = sus::some(i);
    EXPECT_EQ(&i, &mut_to_const.as_ref().unwrap());
  }

  // Verify no copies happen in the marker.
  {
    static i32 copies;
    struct S {
      S() {}
      S(const S&) { copies += 1; }
      S& operator=(const S&) {
        copies += 1;
        return *this;
      }
    };
    copies = 0;
    S s;
    auto marker = sus::some(s);
    EXPECT_EQ(copies, 0);
    Option<S> o = sus::move(marker);
    EXPECT_GE(copies, 1);
  }

  // In place explicit construction.
  {
    const auto i = 2_i32;
    auto a = sus::some(i).construct();
    static_assert(std::same_as<decltype(a), Option<const i32&>>);
    EXPECT_EQ(*a, 2_i32);

    auto b = sus::some(2_i32).construct();
    static_assert(std::same_as<decltype(b), Option<i32>>);
    EXPECT_EQ(*a, 2_i32);

    auto c = sus::some(i).construct<i32>();
    static_assert(std::same_as<decltype(c), Option<i32>>);
    EXPECT_EQ(*a, 2_i32);
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
  EXPECT_EQ(a->i, 2);
  EXPECT_EQ(lvalue.i, 2);

  auto b = Option<MoveableLvalue>::some(sus::move(lvalue));
  EXPECT_EQ(b->i, 2);
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
  EXPECT_EQ(x->i, 1);
  x = sus::move(y);
  IS_SOME(y);  // Trivially relocatable leaves `y` fully in tact.
  EXPECT_EQ(y->i, 1);

  auto a = Option<NotTriviallyRelocatableCopyableOrMoveable>::some(
      NotTriviallyRelocatableCopyableOrMoveable(1));
  auto b = sus::move(a);
  IS_NONE(a);  // Not trivially relocatable moves-from the value in `a`.
  a = sus::move(b);
  IS_NONE(b);  // Not trivially relocatable moves-from the value in `b`.
}

TEST(Option, MoveSelfAssign) {
  auto x = sus::Option<TriviallyMoveableAndRelocatable>::some(
      TriviallyMoveableAndRelocatable(2));
  x = sus::move(x);
  IS_SOME(x);
  EXPECT_EQ(sus::move(x).unwrap().i, 2);

  auto y = Option<NotTriviallyRelocatableCopyableOrMoveable>::some(
      NotTriviallyRelocatableCopyableOrMoveable(2));
  y = sus::move(y);
  IS_SOME(y);
  EXPECT_EQ(sus::move(y).unwrap().i, 2);
}

TEST(Option, CopySelfAssign) {
  auto x = sus::Option<TriviallyCopyable>::some(TriviallyCopyable(2));
  x = x;
  IS_SOME(x);
  EXPECT_EQ(sus::move(x).unwrap().i, 2);

  auto y = Option<NotTriviallyRelocatableCopyableOrMoveable>::some(
      NotTriviallyRelocatableCopyableOrMoveable(2));
  y = y;
  IS_SOME(y);
  EXPECT_EQ(sus::move(y).unwrap().i, 2);
}

TEST(Option, CloneIntoSelfAssign) {
  auto x = sus::Option<TriviallyCopyable>::some(TriviallyCopyable(2));
  sus::clone_into(x, x);
  IS_SOME(x);
  EXPECT_EQ(sus::move(x).unwrap().i, 2);

  auto y = Option<NotTriviallyRelocatableCopyableOrMoveable>::some(
      NotTriviallyRelocatableCopyableOrMoveable(2));
  sus::clone_into(y, y);
  IS_SOME(y);
  EXPECT_EQ(sus::move(y).unwrap().i, 2);
}

TEST(Option, Some) {
  auto x = Option<DefaultConstructible>::some(DefaultConstructible());
  IS_SOME(x);

  auto y = Option<NotDefaultConstructible>::some(NotDefaultConstructible(3));
  IS_SOME(y);

  auto i = NoCopyMove();
  auto ix = Option<NoCopyMove&>::some(mref(i));
  IS_SOME(ix);

  static_assert(
      Option<DefaultConstructible>::some(DefaultConstructible()).is_some());
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

  static_assert(Option<DefaultConstructible>::none().is_none());
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

  static_assert([]() {
    auto o = Option<i32>::some(2_i32);
    return o.take().unwrap();
  }() == 2_i32);
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

  static_assert(Option<int>::none().unwrap_or(3) == 3);
  constexpr NoCopyMove ci;
  static_assert(&Option<const NoCopyMove&>::none().unwrap_or(ci) == &ci);
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

  static_assert(Option<i32>::none().unwrap_or_else([]() { return 3_i32; }) ==
                3_i32);
}

TEST(Option, UnwrapOrDefault) {
  auto x = Option<DefaultConstructible>::some(DefaultConstructible{.i = 4})
               .unwrap_or_default();
  EXPECT_EQ(x.i, 4);
  auto y = Option<DefaultConstructible>::none().unwrap_or_default();
  EXPECT_EQ(y.i, 2);

  static_assert(Option<i32>::none().unwrap_or_default() == 0_i32);
}

TEST(Option, Map) {
  struct Mapped {
    sus_clang_bug_54040(constexpr inline Mapped(int i) : i(i){});
    int i;
  };

  // Rvalue.
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

  // Lvalue.
  called = false;
  auto o = Option<int>::some(2);
  auto lx = o.map([&](int&& i) {
    EXPECT_NE(&i, &o.as_value_mut());  // It was copied.
    called = true;
    return Mapped(i + 1);
  });
  static_assert(std::is_same_v<decltype(lx), Option<Mapped>>);
  EXPECT_EQ(sus::move(lx).unwrap().i, 3);
  EXPECT_TRUE(called);

  called = false;
  o = Option<int>::none();
  auto ly = o.map([&](int&& i) {
    EXPECT_NE(&i, &o.as_value_mut());  // It was copied.
    called = true;
    return Mapped(i + 1);
  });
  static_assert(std::is_same_v<decltype(ly), Option<Mapped>>);
  IS_NONE(ly);
  EXPECT_FALSE(called);

  // Reference.
  called = false;
  NoCopyMove i;
  auto ix = Option<NoCopyMove&>::some(mref(i)).map([&](NoCopyMove&) {
    called = true;
    return Mapped(2);
  });
  static_assert(std::is_same_v<decltype(ix), Option<Mapped>>);
  EXPECT_EQ(ix->i, 2);
  EXPECT_TRUE(called);

  called = false;
  auto iy = Option<NoCopyMove&>::none().map([&](NoCopyMove&) {
    called = true;
    return Mapped(2);
  });
  static_assert(std::is_same_v<decltype(iy), Option<Mapped>>);
  IS_NONE(iy);
  EXPECT_FALSE(called);

  static_assert(Option<int>::some(2)
                    .map([](int&& i) { return Mapped(i + 1); })
                    .unwrap()
                    .i == 3);
  constexpr int ci = 2;
  static_assert(Option<const int&>::some(ci)
                    .map([](const int& i) { return Mapped(i + 1); })
                    .unwrap()
                    .i == 3);
  static_assert(
      Option<int>::none().map([](int&& i) { return Mapped(i + 1); }).is_none());
}

TEST(Option, MapOr) {
  struct Mapped {
    sus_clang_bug_54040(constexpr inline Mapped(int i) : i(i){});
    int i;
  };

  // Rvalue.
  auto x = Option<int>::some(2).map_or(
      Mapped(4), [](int&& i) { return static_cast<Mapped>(i + 1); });
  static_assert(std::is_same_v<decltype(x), Mapped>);
  EXPECT_EQ(x.i, 3);

  auto y = Option<int>::none().map_or(Mapped(4),
                                      [](int&& i) { return Mapped(i + 1); });
  static_assert(std::is_same_v<decltype(y), Mapped>);
  EXPECT_EQ(y.i, 4);

  // Lvalue.
  auto o = Option<int>::some(2);
  auto lx = o.map_or(Mapped(4), [&o](int&& i) {
    EXPECT_NE(&i, &o.as_value_mut());  // It was copied.
    return static_cast<Mapped>(i + 1);
  });
  static_assert(std::is_same_v<decltype(lx), Mapped>);
  EXPECT_EQ(lx.i, 3);

  o = Option<int>::none();
  auto ly = o.map_or(Mapped(4), [&o](int&& i) {
    EXPECT_NE(&i, &o.as_value_mut());  // It was copied.
    return Mapped(i + 1);
  });
  static_assert(std::is_same_v<decltype(ly), Mapped>);
  EXPECT_EQ(ly.i, 4);

  // Reference.
  auto i = NoCopyMove();
  auto ix = Option<NoCopyMove&>::some(mref(i)).map_or(
      Mapped(1), [](NoCopyMove&) { return Mapped(2); });
  static_assert(std::is_same_v<decltype(ix), Mapped>);
  EXPECT_EQ(ix.i, 2);

  auto iy = Option<NoCopyMove&>::none().map_or(
      Mapped(1), [](NoCopyMove&) { return Mapped(2); });
  static_assert(std::is_same_v<decltype(ix), Mapped>);
  EXPECT_EQ(iy.i, 1);

  static_assert(Option<int>::some(2).map_or(4, [](int&&) { return 1; }) == 1);
  static_assert(Option<int>::none().map_or(4, [](int&&) { return 1; }) == 4);
}

TEST(Option, MapOrElse) {
  struct Mapped {
    sus_clang_bug_54040(constexpr inline Mapped(int i) : i(i){});
    int i;
  };

  // Rvalue.
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

  // Lvalue.
  map_called = false;
  else_called = false;
  auto o = Option<int>::some(2);
  auto lx = o.map_or_else(
      [&]() {
        else_called = true;
        return Mapped(4);
      },
      [&](int&& i) {
        map_called = true;
        return Mapped(i + 1);
      });
  static_assert(std::is_same_v<decltype(lx), Mapped>);
  EXPECT_EQ(lx.i, 3);
  EXPECT_TRUE(map_called);
  EXPECT_FALSE(else_called);

  map_called = else_called = false;
  o = Option<int>::none();
  auto ly = o.map_or_else(
      [&]() {
        else_called = true;
        return Mapped(4);
      },
      [&](int&& i) {
        map_called = true;
        return Mapped(i + 1);
      });
  static_assert(std::is_same_v<decltype(ly), Mapped>);
  EXPECT_EQ(ly.i, 4);
  EXPECT_FALSE(map_called);
  EXPECT_TRUE(else_called);

  // Reference.
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

  static_assert(Option<int>::none()
                    .map_or_else([]() { return Mapped(4); },
                                 [](int&&) { return Mapped(1); })
                    .i == 4);
  static_assert(Option<int>::some(2)
                    .map_or_else([]() { return Mapped(4); },
                                 [](int&&) { return Mapped(1); })
                    .i == 1);
  constexpr int ci = 2;
  static_assert(Option<const int&>::none()
                    .map_or_else([]() { return Mapped(4); },
                                 [](const int&) { return Mapped(1); })
                    .i == 4);
  static_assert(Option<const int&>::some(ci)
                    .map_or_else([]() { return Mapped(4); },
                                 [](const int& i) { return Mapped(i + 1); })
                    .i == 3);
}

TEST(Option, Filter) {
  // Rvalue.
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

  // Lvalue.
  auto o = Option<int>::some(2);
  auto lx = o.filter([](const int&) { return true; });
  static_assert(std::is_same_v<decltype(lx), Option<int>>);
  IS_SOME(lx);

  auto ly = o.filter([](const int&) { return false; });
  static_assert(std::is_same_v<decltype(ly), Option<int>>);
  IS_NONE(ly);

  o = Option<int>::none();
  auto lnx = o.filter([](const int&) { return true; });
  static_assert(std::is_same_v<decltype(lnx), Option<int>>);
  IS_NONE(lnx);

  auto lny = o.filter([](const int&) { return false; });
  static_assert(std::is_same_v<decltype(lny), Option<int>>);
  IS_NONE(lny);

  // Reference.
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

  static_assert(
      Option<int>::some(2).filter([](const int&) { return true; }).is_some());
  static_assert(
      Option<int>::some(2).filter([](const int&) { return false; }).is_none());
  static_assert(
      Option<int>::none().filter([](const int&) { return true; }).is_none());
  static_assert(
      Option<int>::none().filter([](const int&) { return false; }).is_none());

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
  // Rvalue.
  auto x = Option<int>::some(2).and_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(x, 3);

  auto y = Option<int>::some(2).and_opt(Option<int>::none());
  IS_NONE(y);

  auto nx = Option<int>::none().and_opt(Option<int>::some(3));
  IS_NONE(nx);

  auto ny = Option<int>::none().and_opt(Option<int>::none());
  IS_NONE(ny);

  // Lvalue.
  auto o = Option<int>::some(2);
  auto lx = o.and_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(lx, 3);

  auto ly = o.and_opt(Option<int>::none());
  IS_NONE(ly);

  o = Option<int>::none();
  auto lnx = o.and_opt(Option<int>::some(3));
  IS_NONE(lnx);

  auto lny = o.and_opt(Option<int>::none());
  IS_NONE(lny);

  // Reference.
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

  static_assert(Option<int>::some(1).and_opt(Option<int>::some(2)).is_some());
  static_assert(Option<int>::some(1).and_opt(Option<int>::none()).is_none());
  static_assert(Option<int>::none().and_opt(Option<int>::some(2)).is_none());
  static_assert(Option<int>::none().and_opt(Option<int>::none()).is_none());
}

TEST(Option, AndThen) {
  struct And {
    sus_clang_bug_54040(constexpr inline And(int i) : i(i){});
    int i;
  };

  // Rvalue.
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

  // Lvalue.
  called = false;
  auto o = Option<int>::some(2);
  auto lx = o.and_then([&](int&&) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(lx), Option<And>>);
  EXPECT_EQ(sus::move(lx).unwrap().i, 3);
  EXPECT_TRUE(called);

  called = false;
  auto ly = o.and_then([&](int&&) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(ly), Option<And>>);
  IS_NONE(ly);
  EXPECT_TRUE(called);

  called = false;
  o = Option<int>::none();
  auto lnx = o.and_then([&](int&&) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(lnx), Option<And>>);
  IS_NONE(lnx);
  EXPECT_FALSE(called);

  called = false;
  auto lny = o.and_then([&](int&&) {
    called = true;
    return Option<And>::none();
  });
  static_assert(std::is_same_v<decltype(lny), Option<And>>);
  IS_NONE(lny);
  EXPECT_FALSE(called);

  // Reference.
  auto i = NoCopyMove();

  called = false;
  auto ix = Option<NoCopyMove&>::some(mref(i)).and_then([&](NoCopyMove&) {
    called = true;
    return Option<And>::some(And(3));
  });
  static_assert(std::is_same_v<decltype(ix), Option<And>>);
  EXPECT_EQ(ix->i, 3);
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

  static_assert(Option<int>::some(2)
                    .and_then([&](int&&) { return Option<And>::some(And(3)); })
                    .unwrap()
                    .i == 3);
  static_assert(Option<int>::some(2)
                    .and_then([&](int&&) { return Option<And>::none(); })
                    .is_none());
  static_assert(Option<int>::none()
                    .and_then([&](int&&) { return Option<And>::some(And(3)); })
                    .is_none());
  static_assert(Option<int>::none()
                    .and_then([&](int&&) { return Option<And>::none(); })
                    .is_none());
}

TEST(Option, Or) {
  // Rvalue.
  auto x = Option<int>::some(2).or_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(x, 2);

  auto y = Option<int>::some(2).or_opt(Option<int>::none()).unwrap();
  EXPECT_EQ(y, 2);

  auto nx = Option<int>::none().or_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(nx, 3);

  auto ny = Option<int>::none().or_opt(Option<int>::none());
  IS_NONE(ny);

  // Lvalue.
  auto o = Option<int>::some(2);
  auto lx = o.or_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(lx, 2);

  auto ly = o.or_opt(Option<int>::none()).unwrap();
  EXPECT_EQ(ly, 2);

  o = Option<int>::none();
  auto lnx = o.or_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(lnx, 3);

  auto lny = o.or_opt(Option<int>::none());
  IS_NONE(lny);

  // Reference.
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

  static_assert(Option<int>::some(1).or_opt(Option<int>::some(2)).is_some());
  static_assert(Option<int>::some(1).or_opt(Option<int>::none()).is_some());
  static_assert(Option<int>::none().or_opt(Option<int>::some(2)).is_some());
  static_assert(Option<int>::none().or_opt(Option<int>::none()).is_none());
}

TEST(Option, OrElse) {
  // Rvalue.
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

  // Lvalue.
  called = false;
  auto o = Option<int>::some(2);
  auto lx = o.or_else([&]() {
               called = true;
               return Option<int>::some(3);
             }).unwrap();
  EXPECT_EQ(lx, 2);
  EXPECT_FALSE(called);

  called = false;
  auto ly = o.or_else([&]() {
               called = true;
               return Option<int>::none();
             }).unwrap();
  EXPECT_EQ(ly, 2);
  EXPECT_FALSE(called);

  called = false;
  o = Option<int>::none();
  auto lnx = o.or_else([&]() {
                called = true;
                return Option<int>::some(3);
              }).unwrap();
  EXPECT_EQ(lnx, 3);
  EXPECT_TRUE(called);

  called = false;
  auto lny = o.or_else([&]() {
    called = true;
    return Option<int>::none();
  });
  IS_NONE(lny);
  EXPECT_TRUE(called);

  // Reference.
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

  static_assert(Option<int>::some(2)
                    .or_else([&]() { return Option<int>::some(3); })
                    .unwrap() == 2);
  static_assert(Option<int>::some(2)
                    .or_else([&]() { return Option<int>::none(); })
                    .unwrap() == 2);
  static_assert(Option<int>::none()
                    .or_else([&]() { return Option<int>::some(3); })
                    .unwrap() == 3);
  static_assert(Option<int>::none()
                    .or_else([&]() { return Option<int>::none(); })
                    .is_none());
}

TEST(Option, Xor) {
  // Rvalue.
  auto x = Option<int>::some(2).xor_opt(Option<int>::some(3));
  IS_NONE(x);

  auto y = Option<int>::some(2).xor_opt(Option<int>::none()).unwrap();
  EXPECT_EQ(y, 2);

  auto nx = Option<int>::none().xor_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(nx, 3);

  auto ny = Option<int>::none().xor_opt(Option<int>::none());
  IS_NONE(ny);

  // Lvalue.
  auto o = Option<int>::some(2);
  auto lx = o.xor_opt(Option<int>::some(3));
  IS_NONE(lx);

  auto ly = o.xor_opt(Option<int>::none()).unwrap();
  EXPECT_EQ(ly, 2);

  o = Option<int>::none();
  auto lnx = o.xor_opt(Option<int>::some(3)).unwrap();
  EXPECT_EQ(lnx, 3);

  auto lny = o.xor_opt(Option<int>::none());
  IS_NONE(lny);

  // Reference.
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

  static_assert(Option<int>::some(1).xor_opt(Option<int>::some(2)).is_none());
  static_assert(Option<int>::some(1).xor_opt(Option<int>::none()).is_some());
  static_assert(Option<int>::none().xor_opt(Option<int>::some(2)).is_some());
  static_assert(Option<int>::none().xor_opt(Option<int>::none()).is_none());
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
  EXPECT_EQ(&ix.as_ref().unwrap(), &i2);

  auto iy = Option<NoCopyMove&>::some(mref(i2));
  iy.insert(mref(i3));
  EXPECT_EQ(&iy.as_ref().unwrap(), &i3);
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

  auto y = Option<DefaultConstructible>::some(
      sus_clang_bug_54040(DefaultConstructible{404})
          sus_clang_bug_54040_else(DefaultConstructible(404)));
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

TEST(Option, AsValue) {
  const auto cx = Option<int>::some(11);
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(cx.as_value()), const int&>);
  static_assert(std::is_same_v<decltype(x.as_value()), const int&>);
  EXPECT_EQ(&x.as_value(), &x.as_ref().unwrap());

  // Can call on rvalue for Option holding a reference.
  int v = 11;
  EXPECT_EQ(&Option<int&>::some(v).as_value(), &v);

  auto i = NoCopyMove();

  const auto cix = Option<NoCopyMove&>::some(mref(i));
  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(std::is_same_v<decltype(cix.as_value()), const NoCopyMove&>);
  static_assert(std::is_same_v<decltype(ix.as_value()), const NoCopyMove&>);
  EXPECT_EQ(&ix.as_value(), &i);

  // Verify constexpr.
  constexpr auto cy = Option<int>::some(3);
  static_assert(cy.as_value() == 3);
}

TEST(Option, AsValueMut) {
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(x.as_value_mut()), int&>);
  EXPECT_EQ(&x.as_value(), &x.as_mut().unwrap());

  // Can call on rvalue for Option holding a reference.
  int v = 11;
  EXPECT_EQ(&Option<int&>::some(v).as_value_mut(), &v);

  auto i = NoCopyMove();

  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(std::is_same_v<decltype(ix.as_value_mut()), NoCopyMove&>);
  EXPECT_EQ(&ix.as_value_mut(), &i);

  // Verify constexpr.
  static_assert([]() {
    auto o = Option<int>::some(3);
    return o.as_value_mut();
  }() == 3);
}

TEST(Option, OperatorStarSome) {
  const auto cx = Option<int>::some(11);
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(*cx), const int&>);
  static_assert(std::is_same_v<decltype(*x), int&>);
  EXPECT_EQ(&*x, &x.as_ref().unwrap());

  // Can call on rvalue for Option holding a reference.
  int v = 11;
  EXPECT_EQ(&*Option<int&>::some(v), &v);

  auto i = NoCopyMove();

  const auto cix = Option<NoCopyMove&>::some(mref(i));
  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(std::is_same_v<decltype(*cix), const NoCopyMove&>);
  static_assert(std::is_same_v<decltype(*ix), NoCopyMove&>);
  EXPECT_EQ(&*ix, &i);

  // Verify constexpr.
  constexpr auto cy = Option<int>::some(3);
  static_assert(*cy == 3);
}

TEST(OptionDeathTest, OperatorStarNone) {
  auto n = Option<int>::none();
  auto in = Option<NoCopyMove&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto& x = *n;
        EXPECT_EQ(&x, &x);
      },
      "");
  EXPECT_DEATH(
      {
        auto& y = *in;
        EXPECT_EQ(&y, &y);
      },
      "");
  EXPECT_DEATH(
      {
        auto& y = *Option<NoCopyMove&>::none();
        EXPECT_EQ(&y, &y);
      },
      "");
#endif
}

TEST(Option, OperatorArrowSome) {
  const auto cx = Option<int>::some(11);
  auto x = Option<int>::some(11);
  static_assert(std::is_same_v<decltype(cx.operator->()), const int*>);
  static_assert(std::is_same_v<decltype(x.operator->()), int*>);
  EXPECT_EQ(x.operator->(), &x.as_ref().unwrap());

  // Can call on rvalue for Option holding a reference.
  int v = 11;
  EXPECT_EQ(Option<int&>::some(v).operator->(), &v);

  auto i = NoCopyMove();

  const auto cix = Option<NoCopyMove&>::some(mref(i));
  auto ix = Option<NoCopyMove&>::some(mref(i));
  static_assert(std::is_same_v<decltype(cix.operator->()), const NoCopyMove*>);
  static_assert(std::is_same_v<decltype(ix.operator->()), NoCopyMove*>);
  EXPECT_EQ(ix.operator->(), &ix.as_ref().unwrap());

  // Verify constexpr.
  struct WithInt {
    i32 i;
  };
  constexpr auto ccx = Option<WithInt>::some(WithInt{3});
  static_assert(ccx->i == 3);
  static constexpr auto ci = NoCopyMove();
  constexpr auto cci = Option<const NoCopyMove&>::some(ci);
  static_assert(cci.operator->() == &ci);
}

TEST(OptionDeathTest, OperatorArrowNone) {
  auto n = Option<int>::none();
  auto in = Option<NoCopyMove&>::none();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto* x = n.operator->();
        EXPECT_EQ(x, x);
      },
      "");
  EXPECT_DEATH(
      {
        auto* x = in.operator->();
        EXPECT_EQ(x, x);
      },
      "");
  EXPECT_DEATH(
      {
        auto* x = Option<NoCopyMove&>::none().operator->();
        EXPECT_EQ(x, x);
      },
      "");
#endif
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

  constexpr auto cx = Option<int>::some(3);
  static_assert(cx.as_ref().unwrap() == 3);
  static constexpr int ci = 2;
  constexpr auto icx = Option<const int&>::some(ci);
  static_assert(&icx.as_ref().unwrap() == &ci);

  // as_ref() on an rvalue allowed if the Option is holding a reference.
  static_assert(std::is_same_v<decltype(Option<NoCopyMove&>::some(i).as_ref()),
                               Option<const NoCopyMove&>>);
  EXPECT_EQ(&Option<NoCopyMove&>::some(i).as_ref().unwrap(), &i);
  EXPECT_TRUE(Option<NoCopyMove&>::none().as_ref().is_none());
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

  static_assert([]() {
    auto o = Option<int>::some(3);
    return o.as_mut().unwrap();
  }() == 3);

  // as_mut() on an rvalue allowed if the Option is holding a reference.
  static_assert(std::is_same_v<decltype(Option<NoCopyMove&>::some(i).as_mut()),
                               Option<NoCopyMove&>>);
  EXPECT_EQ(&Option<NoCopyMove&>::some(i).as_mut().unwrap(), &i);
  EXPECT_TRUE(Option<NoCopyMove&>::none().as_mut().is_none());
}

TEST(Option, TrivialMove) {
  auto x = Option<sus::test::TriviallyMoveableAndRelocatable>::some(
      sus::test::TriviallyMoveableAndRelocatable(int(3423782)));
  auto y = sus::move(x);  // Move-construct.
  EXPECT_EQ(y->i, 3423782);

  y.as_mut().unwrap().i = int(6589043);
  x = sus::move(y);  // Move-assign.
  EXPECT_EQ(x->i, 6589043);
}

TEST(Option, TrivialCopy) {
  auto x = Option<sus::test::TriviallyCopyable>::some(
      sus::test::TriviallyCopyable(int(458790)));
  auto z = x;  // Copy-construct.
  EXPECT_EQ(z->i, 458790);

  z.as_mut().unwrap().i = int(98563453);
  auto y = Option<sus::test::TriviallyCopyable>::none();
  y = z;  // Copy-assign.
  EXPECT_EQ(y->i, 98563453);

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
  static_assert(
      std::same_as<decltype(Option<int&>::none().copied()), Option<int>>);

  // Rvalue.
  auto i = int(2);
  auto x = Option<int&>::none().copied();
  IS_NONE(x);

  auto y = Option<int&>::some(mref(i)).copied();
  EXPECT_EQ(*y, i);
  EXPECT_NE(&y.as_ref().unwrap(), &i);

  // Lvalue.
  auto o = Option<int&>::none();
  EXPECT_EQ(o.copied(), None);

  o = Option<int&>::some(mref(i));
  auto lc = o.copied();
  EXPECT_EQ(*lc, i);
  EXPECT_NE(&lc.as_ref().unwrap(), &i);

  static_assert(Option<int&>::none().copied().is_none());
  constexpr int ic = 2;
  static_assert(Option<const int&>::some(ic).copied().unwrap() == 2);
}

TEST(Option, Cloned) {
  struct Cloneable {
    Cloneable() = default;
    Cloneable(Cloneable&&) = default;
    Cloneable& operator=(Cloneable&&) = default;

    constexpr Cloneable clone() const { return Cloneable(); }
  };
  static_assert(!::sus::mem::Copy<Cloneable>);
  static_assert(::sus::mem::Clone<Cloneable>);

  // Rvalue.
  auto c = Cloneable();
  auto x = Option<Cloneable&>::none().cloned();
  IS_NONE(x);

  auto y = Option<Cloneable&>::some(mref(c)).cloned();
  EXPECT_NE(&y.as_ref().unwrap(), &c);

  // Lvalue.
  auto o = Option<Cloneable&>::none();
  EXPECT_EQ(o.cloned(), None);

  o = Option<Cloneable&>::some(mref(c));
  auto lc = o.cloned();
  EXPECT_NE(&lc.as_ref().unwrap(), &c);

  static_assert(Option<int&>::none().cloned().is_none());
  [[maybe_unused]] constexpr auto cc = Cloneable();
  [[maybe_unused]] constexpr auto cc2 =
      Option<const Cloneable&>::some(cc).cloned().unwrap();
}

TEST(Option, Flatten) {
  static_assert(std::is_same_v<decltype(Option<Option<i32>>::none().flatten()),
                               Option<i32>>);
  static_assert(std::is_same_v<decltype(Option<Option<i32&>>::none().flatten()),
                               Option<i32&>>);
  static_assert(
      std::is_same_v<decltype(Option<Option<Option<i32>>>::none().flatten()),
                     Option<Option<i32>>>);

  // Rvalue.
  EXPECT_TRUE(
      Option<Option<Option<i32>>>::none().flatten().flatten().is_none());
  EXPECT_EQ(Option<Option<Option<i32>>>::some(sus::some(sus::some(4)))
                .flatten()
                .flatten()
                .unwrap(),
            4);

  // Lvalue.
  auto o = Option<Option<Option<i32>>>::none();
  auto f1 = o.flatten();
  EXPECT_EQ(f1.flatten(), None);

  o = sus::some(sus::some(sus::some(4)));
  f1 = o.flatten();
  EXPECT_EQ(f1, sus::some(sus::some(4)));
  auto f2 = f1.flatten();
  EXPECT_EQ(f2, sus::some(4));
  EXPECT_EQ(f2.as_value(), 4);

  // Reference.
  i32 i = 2;
  EXPECT_EQ(&Option<Option<i32&>>::some(Option<i32&>::some(mref(i)))
                 .flatten()
                 .unwrap(),
            &i);

  static_assert(Option<Option<i32>>::none().flatten().is_none());
  static_assert(
      Option<Option<i32>>::some(Option<i32>::none()).flatten().is_none());
  static_assert(
      Option<Option<i32>>::some(Option<i32>::some(3)).flatten().unwrap() == 3);
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

  // Iterating on an rvalue is okay if it's holding a reference.
  for (const NoCopyMove& i : Option<NoCopyMove&>::some(mref(n))) {
    EXPECT_EQ(&i, &n);
  }
  for (const NoCopyMove& i : Option<NoCopyMove&>::some(mref(n)).iter()) {
    EXPECT_EQ(&i, &n);
  }
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

  // Iterating on an rvalue is okay if it's holding a reference.
  for (NoCopyMove& i : Option<NoCopyMove&>::some(mref(n)).iter_mut()) {
    EXPECT_EQ(&i, &n);
  }
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

  // Same types.
  static_assert(::sus::ops::Eq<Option<i32>, Option<i32>>);
  // Eq types.
  static_assert(::sus::ops::Eq<Option<i32>, Option<i64>>);
  // Not Eq types.
  static_assert(!::sus::ops::Eq<Option<NotEq>, Option<NotEq>>);

  EXPECT_EQ(Option<i32>::some(1), Option<i32>::some(1));
  EXPECT_NE(Option<i32>::some(1), Option<i32>::some(2));
  EXPECT_NE(Option<i32>::none(), Option<i32>::some(1));
  EXPECT_EQ(Option<i32>::none(), Option<i32>::none());
  EXPECT_EQ(Option<f32>::some(1.f), Option<f32>::some(1.f));
  EXPECT_EQ(Option<f32>::some(0.f), Option<f32>::some(-0.f));
  EXPECT_NE(Option<f32>::some(f32::NAN), Option<f32>::some(f32::NAN));

  // Compares with the State enum.
  EXPECT_EQ(Option<i32>::some(1), Some);
  EXPECT_NE(Option<i32>::some(1), None);
  EXPECT_EQ(Option<i32>::none(), None);
  EXPECT_NE(Option<i32>::none(), Some);

  // Comparison with marker types. EXPECT_EQ also converts it to a const
  // reference, so this tests that comparison from a const marker works (if the
  // inner type is copyable).
  EXPECT_EQ(Option<i32>::some(1), sus::some(1_i32));
  EXPECT_EQ(Option<i32>::some(1), sus::some(1));
  EXPECT_NE(Option<i32>::some(1), sus::none());
  EXPECT_EQ(Option<i32>::none(), sus::none());
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
  static_assert(::sus::ops::Ord<Option<int>>);

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
  auto operator==(const Weak& o) const& noexcept {
    return a == o.a && b == o.b;
  }
  auto operator<=>(const Weak& o) const& noexcept {
    if (a == o.a) return std::weak_ordering::equivalent;
    if (a < o.a) return std::weak_ordering::less;
    return std::weak_ordering::greater;
  }

  Weak(int a, int b) : a(a), b(b) {}
  int a;
  int b;
};

TEST(Option, WeakOrder) {
  static_assert(!::sus::ops::Ord<Option<Weak>>);
  static_assert(::sus::ops::WeakOrd<Option<Weak>>);

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
  static_assert(!::sus::ops::WeakOrd<Option<float>>);
  static_assert(::sus::ops::PartialOrd<Option<float>>);

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
  EXPECT_EQ(
      std::partial_order(Option<f32>::some(11.f), Option<f32>::some(f32::NAN)),
      std::partial_ordering::unordered);
  EXPECT_EQ(std::partial_order(Option<f32>::some(f32::NAN),
                               Option<f32>::some(f32::NAN)),
            std::partial_ordering::unordered);
  EXPECT_EQ(std::partial_order(Option<f32>::some(0.f),
                               Option<f32>::some(f32::INFINITY)),
            std::partial_ordering::less);
  EXPECT_EQ(std::partial_order(Option<f32>::some(0.f),
                               Option<f32>::some(f32::NEG_INFINITY)),
            std::partial_ordering::greater);

  EXPECT_EQ(std::partial_order(Option<f32>::some(0.f), Option<f32>::none()),
            std::partial_ordering::greater);
  EXPECT_EQ(
      std::partial_order(Option<f32>::none(), Option<f32>::some(f32::NAN)),
      std::partial_ordering::less);
}

TEST(Option, NoOrder) {
  struct NotCmp {};
  static_assert(!sus::ops::PartialOrd<NotCmp>);
  static_assert(!::sus::ops::PartialOrd<Option<NotCmp>>);
}

TEST(Option, OkOr) {
  // Rvalue.
  {
    auto o = Option<u8>::some(3_u8);
    auto r = sus::move(o).ok_or(-5_i32);
    IS_NONE(o);
    static_assert(std::same_as<sus::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(sus::move(r).unwrap(), 3_u8);
  }
  {
    auto o = Option<u8>::none();
    auto r = sus::move(o).ok_or(-5_i32);
    IS_NONE(o);
    static_assert(std::same_as<sus::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(sus::move(r).unwrap_err(), -5_i32);
  }

  // Lvalue.
  {
    auto o = Option<u8>::some(3_u8);
    auto r = o.ok_or(-5_i32);
    EXPECT_EQ(o.as_value(), 3_u8);
    static_assert(std::same_as<sus::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(sus::move(r).unwrap(), 3_u8);
  }
  {
    auto o = Option<u8>::none();
    auto r = o.ok_or(-5_i32);
    IS_NONE(o);
    static_assert(std::same_as<sus::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(sus::move(r).unwrap_err(), -5_i32);
  }

  static_assert(Option<i32>::some(2_i32).ok_or(3_u32).unwrap() == 2_i32);
  static_assert(Option<i32>::none().ok_or(3_u32).unwrap_err() == 3_u32);

  // TODO: Result references are not yet supported.
  // https://github.com/chromium/subspace/issues/133
  /*
  {
    auto i = 1_i32;
    auto o = Option<i32&>::some(mref(i));
    auto r = sus::move(o).ok_or(-5_i32);
    IS_SOME(o);
    static_assert(std::same_as<sus::Result<i32&, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(&sus::move(r).unwrap(), &i);
  }
  {
    auto i = 1_i32;
    auto o = Option<i32&>::none();
    auto r = sus::move(o).ok_or(-5_i32);
    IS_NONE(o);
    static_assert(std::same_as<sus::Result<i32&, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(sus::move(r).unwrap_err(), -5_i32);
  }
  */
}

TEST(Option, OkOrElse) {
  // Rvalue.
  {
    auto o = Option<u8>::some(3_u8);
    auto r = sus::move(o).ok_or_else([]() { return -5_i32; });
    IS_NONE(o);
    static_assert(std::same_as<sus::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(sus::move(r).unwrap(), 3_u8);
  }
  {
    auto o = Option<u8>::none();
    auto r = sus::move(o).ok_or_else([]() { return -5_i32; });
    IS_NONE(o);
    static_assert(std::same_as<sus::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(sus::move(r).unwrap_err(), -5_i32);
  }

  // Lvalue.
  {
    auto o = Option<u8>::some(3_u8);
    auto r = o.ok_or_else([]() { return -5_i32; });
    EXPECT_EQ(o.as_value(), 3_u8);
    static_assert(std::same_as<sus::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(sus::move(r).unwrap(), 3_u8);
  }
  {
    auto o = Option<u8>::none();
    auto r = o.ok_or_else([]() { return -5_i32; });
    IS_NONE(o);
    static_assert(std::same_as<sus::Result<u8, i32>, decltype(r)>);
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(sus::move(r).unwrap_err(), -5_i32);
  }

  static_assert(
      Option<i32>::some(2_i32).ok_or_else([]() { return 3_u32; }).unwrap() ==
      2_i32);
  static_assert(
      Option<i32>::none().ok_or_else([]() { return 3_u32; }).unwrap_err() ==
      3_u32);

  // TODO: Result references are not yet supported, test Option<u8&> when they
  // are. https://github.com/chromium/subspace/issues/133
}

TEST(Option, Transpose) {
  // Rvalue.
  auto none = Option<sus::Result<u8, i32>>::none();
  auto t1 = sus::move(none).transpose();
  static_assert(std::same_as<sus::Result<Option<u8>, i32>, decltype(t1)>);
  EXPECT_EQ(t1.is_ok(), true);
  EXPECT_EQ(sus::move(t1).unwrap(), None);

  auto some_ok =
      Option<sus::Result<u8, i32>>::some(sus::Result<u8, i32>::with(5_u8));
  auto t2 = sus::move(some_ok).transpose();
  static_assert(std::same_as<sus::Result<Option<u8>, i32>, decltype(t2)>);
  EXPECT_EQ(t2.is_ok(), true);
  EXPECT_EQ(sus::move(t2).unwrap().unwrap(), 5_u8);

  auto some_err = Option<sus::Result<u8, i32>>::some(
      sus::Result<u8, i32>::with_err(-2_i32));
  auto t3 = sus::move(some_err).transpose();
  static_assert(std::same_as<sus::Result<Option<u8>, i32>, decltype(t3)>);
  EXPECT_EQ(t3.is_err(), true);
  EXPECT_EQ(sus::move(t3).unwrap_err(), -2_i32);

  // Lvalue.
  none = Option<sus::Result<u8, i32>>::none();
  t1 = none.transpose();
  static_assert(std::same_as<sus::Result<Option<u8>, i32>, decltype(t1)>);
  EXPECT_EQ(t1.is_ok(), true);
  EXPECT_EQ(sus::move(t1).unwrap(), None);

  some_ok =
      Option<sus::Result<u8, i32>>::some(sus::Result<u8, i32>::with(5_u8));
  t2 = some_ok.transpose();
  static_assert(std::same_as<sus::Result<Option<u8>, i32>, decltype(t2)>);
  EXPECT_EQ(t2.is_ok(), true);
  EXPECT_EQ(sus::move(t2).unwrap().unwrap(), 5_u8);

  some_err = Option<sus::Result<u8, i32>>::some(
      sus::Result<u8, i32>::with_err(-2_i32));
  t3 = some_err.transpose();
  static_assert(std::same_as<sus::Result<Option<u8>, i32>, decltype(t3)>);
  EXPECT_EQ(t3.is_err(), true);
  EXPECT_EQ(sus::move(t3).unwrap_err(), -2_i32);

  static_assert(Option<sus::Result<u8, i32>>::some(sus::ok(2_u8))
                    .transpose()
                    .unwrap()
                    .unwrap() == 2_u8);
  static_assert(Option<sus::Result<u8, i32>>::some(sus::err(3_i32))
                    .transpose()
                    .unwrap_err() == 3_i32);
  static_assert(
      Option<sus::Result<u8, i32>>::none().transpose().unwrap().is_none());

  // TODO: Result references are not yet supported, test them when they
  // are. https://github.com/chromium/subspace/issues/133
}

TEST(Option, Zip) {
  // Rvalue.
  {
    EXPECT_EQ(Option<i32>::none().zip(Option<i32>::none()), None);
    EXPECT_EQ(Option<i32>::none().zip(Option<i32>::some(1_i32)), None);
    EXPECT_EQ(Option<i32>::some(1_i32).zip(Option<i32>::none()), None);
    EXPECT_EQ(Option<i32>::some(2_i32).zip(Option<i32>::some(1_i32)), Some);

    auto o = Option<i32>::some(-2_i32);
    EXPECT_EQ(sus::move(o).zip(Option<u8>::some(3_u8)).unwrap(),
              (Tuple<i32, u8>::with(-2_i32, 3_u8)));
    EXPECT_EQ(o, None);
  }

  // Lvalue.
  {
    auto l = Option<i32>::none();
    EXPECT_EQ(l.zip(Option<i32>::none()), None);
    EXPECT_EQ(l.zip(Option<i32>::some(1_i32)), None);
    l = Option<i32>::some(1_i32);
    EXPECT_EQ(l.zip(Option<i32>::none()), None);
    l = Option<i32>::some(2_i32);
    EXPECT_EQ(l.zip(Option<i32>::some(1_i32)), Some);

    auto o = Option<i32>::some(-2_i32);
    EXPECT_EQ(o.zip(Option<u8>::some(3_u8)).unwrap(),
              (Tuple<i32, u8>::with(-2_i32, 3_u8)));
    EXPECT_EQ(o, sus::some(-2_i32));
  }

  // Reference.
  {
    auto a = -2_i32;
    auto b = NoCopyMove();
    auto i = Option<const i32&>::some(a);
    auto u = Option<const NoCopyMove&>::some(b);
    static_assert(
        std::same_as<Tuple<const i32&, const NoCopyMove&>,
                     decltype(sus::move(i).zip(sus::move(u)).unwrap())>);
    auto zip = sus::move(i).zip(sus::move(u)).unwrap();
    EXPECT_EQ(&zip.at<0>(), &a);
    EXPECT_EQ(&zip.at<1>(), &b);
    EXPECT_EQ(i, None);
  }

  static_assert(Option<i32>::none().zip(Option<i32>::none()).is_none());
  static_assert(Option<i32>::some(1_i32).zip(Option<i32>::none()).is_none());
  static_assert(Option<i32>::none().zip(Option<i32>::some(1_i32)).is_none());
  static_assert(
      Option<i32>::some(2_i32).zip(Option<i32>::some(1_i32)).is_some());
}

TEST(Option, Unzip) {
  // Rvalue.
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

  // Lvalue.
  {
    auto s =
        Option<Tuple<i32, u32>>::some(Tuple<i32, u32>::with(-2_i32, 4_u32));
    auto sr = s.unzip();
    static_assert(std::same_as<decltype(sr), Tuple<Option<i32>, Option<u32>>>);
    EXPECT_EQ(sr, (Tuple<Option<i32>, Option<u32>>::with(
                      Option<i32>::some(-2_i32), Option<u32>::some(4_u32))));
    auto n = Option<Tuple<i32, u32>>::none();
    auto nr = n.unzip();
    static_assert(std::same_as<decltype(nr), Tuple<Option<i32>, Option<u32>>>);
    EXPECT_EQ(nr, (Tuple<Option<i32>, Option<u32>>::with(Option<i32>::none(),
                                                         Option<u32>::none())));
  }

  // Reference.
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

  static_assert([]() {
    return Option<Tuple<i32, u32>>::some(Tuple<i32, u32>::with(-2_i32, 4_u32))
        .unzip();
  }() == Tuple<Option<i32>, Option<u32>>::with(Option<i32>::some(-2_i32),
                                               Option<u32>::some(4_u32)));
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
  EXPECT_EQ(o->as_ptr(), &j);
}

template <class T>
struct CollectSum {
  sus_clang_bug_54050(CollectSum(T sum) : sum(sum){});

  static constexpr CollectSum from_iter(
      ::sus::iter::IntoIterator<T> auto iter) noexcept {
    T sum = T();
    for (T t : sus::move(iter).into_iter()) sum += t;
    return CollectSum(sum);
  }

  T sum;
};

TEST(Option, FromIter) {
  decltype(auto) all_some =
      sus::Array<Option<usize>, 3>::with_values(Option<usize>::some(1u),
                                                Option<usize>::some(2u),
                                                Option<usize>::some(3u))
          .into_iter()
          .collect<Option<CollectSum<usize>>>();
  static_assert(
      std::same_as<sus::Option<CollectSum<usize>>, decltype(all_some)>);
  EXPECT_EQ(all_some, Some);
  EXPECT_EQ(all_some->sum, 1u + 2u + 3u);

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
      sus::iter::IntoIterator<const NoCopyMove&> auto iter) noexcept {
    auto v = sus::Vec<const NoCopyMove*>();
    for (const NoCopyMove& t : sus::move(iter).into_iter()) v.push(&t);
    return CollectRefs(sus::move(v));
  }

  sus::Vec<const NoCopyMove*> vec;
};

TEST(Option, FromIterWithRefs) {
  auto u1 = NoCopyMove();
  auto u2 = NoCopyMove();
  auto u3 = NoCopyMove();

  decltype(auto) all_some =
      sus::Array<Option<const NoCopyMove&>, 3>::with_values(
          sus::some(u1), sus::some(u2), sus::some(u3))
          .into_iter()
          .collect<Option<CollectRefs>>();
  static_assert(std::same_as<sus::Option<CollectRefs>, decltype(all_some)>);
  EXPECT_EQ(all_some, Some);
  EXPECT_EQ(all_some->vec[0u], &u1);
  EXPECT_EQ(all_some->vec[1u], &u2);
  EXPECT_EQ(all_some->vec[2u], &u3);

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
    i32 i = s->i;
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Option<Copy>>);
    EXPECT_EQ(s2->i, i + 1_i32);
  }

  {
    const auto s = Option<Copy>::some(Copy());
    i32 i = s->i;
    auto s2 = Option<Copy>::some(Copy());
    s2.as_mut().unwrap().i = 1000_i32;
    sus::clone_into(mref(s2), s);
    EXPECT_EQ(s2->i, i);
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
    EXPECT_EQ(s2->i, 2_i32);
  }

  {
    const auto s = Option<Clone>::some(Clone());
    auto s2 = Option<Clone>::some(Clone());
    s2.as_mut().unwrap().i = 1000_i32;
    sus::clone_into(mref(s2), s);
    EXPECT_EQ(s2->i, 2_i32);
  }

  {
    auto i = 1_i32;
    const auto s = Option<const i32&>::some(i);
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Option<const i32&>>);
    EXPECT_EQ(&s2.as_ref().unwrap(), &i);
  }

  {
    auto i = 1_i32;
    const auto s = Option<i32&>::some(mref(i));
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Option<i32&>>);
    EXPECT_EQ(&s2.as_ref().unwrap(), &i);
  }
}

TEST(Option, fmt) {
  static_assert(fmt::is_formattable<sus::Option<i32>, char>::value);
  EXPECT_EQ(fmt::format("{}", sus::Option<i32>::some(12345)), "Some(12345)");
  EXPECT_EQ(fmt::format("{:06}", sus::Option<i32>::some(12345)),
            "Some(012345)");
  EXPECT_EQ(fmt::format("{}", sus::Option<i32>::none()), "None");
  EXPECT_EQ(fmt::format("{:02}", sus::Option<i32>::none()), "None");
  EXPECT_EQ(fmt::format("{}", sus::Option<std::string_view>::some("12345")),
            "Some(12345)");
  EXPECT_EQ(fmt::format("{}", sus::Option<std::string_view>::none()), "None");

  struct NoFormat {
    i32 a = 0x16ae3cf2;
  };
  static_assert(!fmt::is_formattable<NoFormat, char>::value);
  static_assert(fmt::is_formattable<Option<NoFormat>, char>::value);
  EXPECT_EQ(fmt::format("{}", sus::Option<NoFormat>::some(NoFormat())),
            "Some(f2-3c-ae-16)");
  EXPECT_EQ(fmt::format("{}", sus::Option<NoFormat>::none()), "None");
}

TEST(Option, Stream) {
  std::stringstream s;
  s << sus::Option<i32>::some(12345) << " " << sus::Option<i32>::none();
  EXPECT_EQ(s.str(), "Some(12345) None");
}

TEST(Option, GTest) {
  EXPECT_EQ(testing::PrintToString(sus::Option<i32>::some(12345)),
            "Some(12345)");
}

}  // namespace
