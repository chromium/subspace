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

#include "containers/array.h"

#include <type_traits>

#include "construct/into.h"
#include "iter/iterator.h"
#include "marker/unsafe.h"
#include "mem/move.h"
#include "mem/relocate.h"
#include "num/types.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

using ::sus::containers::Array;
using ::sus::mem::relocate_array_by_memcpy;
using ::sus::mem::relocate_one_by_memcpy;

namespace {

struct TriviallyRelocatable {
  int i;
};

static_assert(!std::is_trivially_constructible_v<Array<int, 2>>);
static_assert(!std::is_trivial_v<Array<int, 2>>);
static_assert(!std::is_aggregate_v<Array<int, 2>>);
static_assert(std::is_standard_layout_v<Array<int, 2>>);

// TODO: This covers a trivially relocatable type, but what about the rest?
// (like Option unit tests.)
namespace trivially_relocatable {
using T = TriviallyRelocatable;
static_assert(std::is_move_constructible_v<Array<T, 2>>);
static_assert(std::is_move_assignable_v<Array<T, 2>>);
static_assert(std::is_trivially_destructible_v<Array<T, 2>>);
static_assert(std::is_trivially_move_constructible_v<Array<T, 2>>);
static_assert(std::is_trivially_move_assignable_v<Array<T, 2>>);
static_assert(std::is_nothrow_swappable_v<Array<T, 2>>);
static_assert(std::is_trivially_copy_constructible_v<Array<T, 2>>);
static_assert(std::is_trivially_copy_assignable_v<Array<T, 2>>);
static_assert(!std::is_constructible_v<Array<T, 2>, T&&>);
static_assert(!std::is_assignable_v<Array<T, 2>, T&&>);
static_assert(!std::is_constructible_v<Array<T, 2>, const T&>);
static_assert(!std::is_assignable_v<Array<T, 2>, const T&>);
static_assert(!std::is_constructible_v<Array<T, 2>, T>);
static_assert(!std::is_assignable_v<Array<T, 2>, T>);
static_assert(std::is_nothrow_destructible_v<Array<T, 2>>);
static_assert(relocate_one_by_memcpy<Array<T, 2>>);
static_assert(relocate_array_by_memcpy<Array<T, 2>>);
}  // namespace trivially_relocatable

struct NonAggregate {
  ~NonAggregate() {}
};

TEST(Array, WithDefault) {
  auto a = Array<int, 5>::with_default();
  static_assert(sizeof(a) == sizeof(int[5]));
  for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
    EXPECT_EQ(a.get(i), 0);
  }

  static_assert(sizeof(Array<int, 5>::with_default()) == sizeof(int[5]));
}

TEST(Array, Zero) {
  auto a = Array<int, 0>::with_default();
  static_assert(sizeof(a) == 1);
  static_assert(sizeof(Array<int, 0>::with_default()) == 1);
}

TEST(Array, WithUninitialized) {
  static_assert(
      sizeof(Array<int, 5>::with_uninitialized(unsafe_fn)) == sizeof(int[5]),
      "");
}

TEST(Array, WithInitializer) {
  auto a =
      Array<usize, 5>::with_initializer([i = 1u]() mutable { return i++; });
  static_assert(sizeof(a) == sizeof(usize[5]));
  for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
    EXPECT_EQ(a.get(i), i + 1_usize);
  }

  struct NotTriviallyDefaultConstructible {
    NotTriviallyDefaultConstructible() {}
    NotTriviallyDefaultConstructible(usize i) : i(i) {}
    usize i;
  };
  static_assert(!std::is_trivially_default_constructible_v<
                    NotTriviallyDefaultConstructible>,
                "");
  auto b = Array<NotTriviallyDefaultConstructible, 5>::with_initializer(
      [i = 1u]() mutable -> NotTriviallyDefaultConstructible { return {i++}; });
  static_assert(sizeof(b) == sizeof(NotTriviallyDefaultConstructible[5]));
  for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
    EXPECT_EQ(b.get(i).i, i + 1_usize);
  }

  auto lvalue = [i = 1u]() mutable { return i++; };
  auto c = Array<usize, 5>::with_initializer(lvalue);
  static_assert(sizeof(c) == sizeof(usize[5]));
  for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
    EXPECT_EQ(c.get(i), i + 1_usize);
  }

  static_assert(sizeof(Array<usize, 5>::with_initializer(
                    []() { return 1u; })) == sizeof(usize[5]),
                "");
}

TEST(Array, WithValue) {
  auto a = Array<usize, 5>::with_value(3u);
  static_assert(sizeof(a) == sizeof(usize[5]));
  for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
    EXPECT_EQ(a.get(i), 3_usize);
  }
}

TEST(Array, WithValues) {
  {
    auto a = Array<usize, 5>::with_values(3u, 4u, 5u, 6u, 7u);
    static_assert(sizeof(a) == sizeof(usize[5]));
    for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
      EXPECT_EQ(a.get(i), 3_usize + i);
    }
  }
  {
    auto a = Array<u8, 5>::with_values(sus::into(3), sus::into(4), sus::into(5),
                                       sus::into(6), sus::into(7));
    static_assert(sizeof(a) == sizeof(u8[5]));
    for (auto i = 0_u8; i < 5_u8; i += 1_u8) {
      EXPECT_EQ(a.get(usize::from(i)), 3_u8 + i);
    }
  }
}

TEST(Array, Get) {
  {
    constexpr auto r = []() constexpr {
      constexpr auto a =
          Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
      return a.get(2_usize);
    }
    ();
    static_assert(std::same_as<decltype(r), const int>);
    EXPECT_EQ(3, r);
  }
  {
    auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
    EXPECT_EQ(3, a.get(2_usize));
  }
}

TEST(Array, GetMut) {
  {
    constexpr auto a = []() constexpr {
      auto a =
          Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
      a.get_mut(0_usize) = 101;
      return a;
    }
    ();
    EXPECT_EQ(a.get(0_usize), 101);
  }
  {
    auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
    a.get_mut(0_usize) = 101;
    EXPECT_EQ(a.get(0_usize), 101);
  }
}

TEST(Array, AsPtr) {
  auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  auto r = a.as_ptr();
  static_assert(std::same_as<decltype(r), const int*>);
  EXPECT_EQ(3, *(r + 2));
}

TEST(Array, AsPtrMut) {
  auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  auto r = a.as_ptr_mut();
  static_assert(std::same_as<decltype(r), int*>);
  *(r + 2) = 101;
  EXPECT_EQ(101, *(r + 2));
}

TEST(Array, Eq) {
  auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  auto b = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  EXPECT_EQ(a, b);
  b.get_mut(3_usize) += 1;
  EXPECT_NE(a, b);
}

TEST(Array, Ord) {
  auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  auto b = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  EXPECT_LE(a, b);
  EXPECT_GE(a, b);
  b.get_mut(3_usize) += 1;
  EXPECT_LT(a, b);
}

TEST(Array, StrongOrder) {
  auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  auto b = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  EXPECT_EQ(std::strong_order(a, b), std::strong_ordering::equal);
  b.get_mut(3_usize) += 1;
  EXPECT_EQ(std::strong_order(a, b), std::strong_ordering::less);
}

struct Weak final {
  auto operator==(Weak const& o) const& { return a == o.a && b == o.b; }
  auto operator<=>(Weak const& o) const& {
    if (a == o.a) return std::weak_ordering::equivalent;
    if (a < o.a) return std::weak_ordering::less;
    return std::weak_ordering::greater;
  }

  Weak(int a, int b) : a(a), b(b) {}
  int a;
  int b;
};

TEST(Array, WeakOrder) {
  auto a = Array<Weak, 5>::with_initializer(
      [i = 0]() mutable { return Weak(++i, 2); });
  auto b = Array<Weak, 5>::with_initializer(
      [i = 0]() mutable { return Weak(++i, 2); });
  EXPECT_EQ(std::weak_order(a, b), std::weak_ordering::equivalent);
  b.get_mut(3_usize).a += 1;
  EXPECT_EQ(std::weak_order(a, b), std::weak_ordering::less);
}

TEST(Array, PartialOrder) {
  auto a =
      Array<float, 5>::with_initializer([i = 0.f]() mutable { return ++i; });
  auto b =
      Array<float, 5>::with_initializer([i = 0.f]() mutable { return ++i; });
  EXPECT_EQ(std::partial_order(a, b), std::partial_ordering::equivalent);
  b.get_mut(3_usize) += 1;
  EXPECT_EQ(std::partial_order(a, b), std::partial_ordering::less);
}

TEST(Array, Iter) {
  const auto a = Array<usize, 5>::with_value(3u);
  auto sum = 0_usize;
  for (const usize& i : a.iter()) {
    sum += i;
  }
  EXPECT_EQ(sum, 15_usize);
}

TEST(Array, IterMut) {
  auto a = Array<usize, 5>::with_value(3u);
  auto sum = 0_usize;
  for (usize& i : a.iter_mut()) {
    sum += i;
    i += 1_usize;
  }
  EXPECT_EQ(sum, 15_usize);

  sum = 0_usize;
  for (const usize& i : a.iter()) {
    sum += i;
  }
  EXPECT_EQ(sum, 20_usize);
}

TEST(Array, IntoIter) {
  auto a = Array<usize, 5>::with_value(3u);
  auto sum = 0_usize;
  for (usize i : sus::move(a).into_iter()) {
    sum += i;
  }
  EXPECT_EQ(sum, 15_usize);
}

TEST(Array, ImplicitIter) {
  const auto a = Array<usize, 5>::with_value(3u);
  auto sum = 0_usize;
  for (const usize& i : a) {
    sum += i;
  }
  EXPECT_EQ(sum, 15_usize);
}

TEST(Array, Map) {
  auto a = Array<usize, 3>::with_values(3u, 4u, 5u);
  auto a2 = sus::move(a).map(+[](usize i) { return u32::from(i + 1_usize); });
  EXPECT_EQ(a2, (Array<u32, 3>::with_values(4_u32, 5_u32, 6_u32)));
}

}  // namespace
