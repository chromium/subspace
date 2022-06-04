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

#include "third_party/googletest/googletest/include/gtest/gtest.h"

using sus::option::None;
using sus::option::Option;
using sus::option::Some;

namespace {

struct DefaultConstructible {
  int i = 2;
};

struct NotDefaultConstructible {
  int i;

  constexpr NotDefaultConstructible(int i) : i(i) {}
};

struct WithDefaultConstructible {
  int i;

  static inline constexpr WithDefaultConstructible with_default() {
    return WithDefaultConstructible(3);
  }

  constexpr WithDefaultConstructible(int i) : i(i) {}
};

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

TEST(Option, Some) {
  auto x = Option<DefaultConstructible>::some(DefaultConstructible());
  IS_SOME(x);

  auto y = Option<NotDefaultConstructible>::some(NotDefaultConstructible(3));
  IS_SOME(y);

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

  using sus::traits::MakeDefault;
  static_assert(MakeDefault<Option<DefaultConstructible>>::has_trait, "");
  static_assert(MakeDefault<Option<WithDefaultConstructible>>::has_trait, "");
  static_assert(!MakeDefault<Option<NotDefaultConstructible>>::has_trait, "");

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
}

TEST(Option, Clear) {
  static int count = 0;
  struct WatchDestructor {
    ~WatchDestructor() { ++count; }
  };
  {
    auto x = Option<WatchDestructor>::with_default();
    count = 0;  // Without optimizations, moves may run a destructor.
    IS_SOME(x);
    x.clear();
    IS_NONE(x);
    EXPECT_EQ(1, count);
  }
  EXPECT_EQ(1, count);
}

TEST(Option, ExpectSome) {
  auto x = Option<int>::with_default().expect("hello world");
  EXPECT_EQ(x, 0);
}

TEST(OptionDeathTest, ExpectNone) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(Option<int>::none().expect("hello world"), "hello world");
#endif
}

TEST(Option, UnwrapSome) {
  auto x = Option<int>::with_default().unwrap();
  EXPECT_EQ(x, 0);
}

TEST(OptionDeathTest, UnwrapNone) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(Option<int>::none().unwrap(), "");
#endif
}

TEST(Option, UnwrapUncheckedSome) {
  auto x = Option<int>::with_default().unwrap_unchecked(unsafe_fn);
  EXPECT_EQ(x, 0);
}

TEST(Option, Take) {
  auto x = Option<int>::some(404);
  IS_SOME(x);
  auto y = x.take();
  // The value has moved from `x` to `y`.
  IS_NONE(x);
  IS_SOME(y);
  EXPECT_EQ(static_cast<decltype(y)&&>(y).unwrap(), 404);

  auto n = Option<int>::none();
  IS_NONE(n);
  auto m = n.take();
  // The None has moved from `n` to `m`, which is a no-op on `n`.
  IS_NONE(n);
  IS_NONE(m);
}

TEST(Option, UnwrapOr) {
  auto x = Option<int>::some(2).unwrap_or(3);
  EXPECT_EQ(x, 2);
  auto y = Option<int>::none().unwrap_or(3);
  EXPECT_EQ(y, 3);

  // Verify constexpr.
  static_assert(Option<int>::none().unwrap_or(3) == 3, "");
}

TEST(Option, UnwrapOrElse) {
  auto x = Option<int>::some(2).unwrap_or_else([]() { return int(3); });
  EXPECT_EQ(x, 2);
  auto y = Option<int>::none().unwrap_or_else([]() { return int(3); });
  EXPECT_EQ(y, 3);

  // Verify constexpr.
  static_assert(
      Option<int>::none().unwrap_or_else([]() { return int(3); }) == 3, "");
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

  auto x = Option<int>::some(2).map([](int&& i) { return Mapped(i + 1); });
  static_assert(std::is_same_v<decltype(x), Option<Mapped>>, "");
  EXPECT_EQ(static_cast<decltype(x)&&>(x).unwrap().i, 3);

  auto y = Option<int>::none().map([](int&& i) { return Mapped(i + 1); });
  static_assert(std::is_same_v<decltype(y), Option<Mapped>>, "");
  IS_NONE(y);

  // Verify constexpr.
  static_assert(Option<int>::some(2)
                        .map([](int&& i) { return Mapped(i + 1); })
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

  // Verify constexpr.
  static_assert(
      Option<int>::some(2).filter([](const int& i) { return true; }).unwrap() ==
          2,
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
}

}  // namespace
