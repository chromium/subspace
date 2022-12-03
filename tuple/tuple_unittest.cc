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

#include "tuple/tuple.h"

#include <math.h>  // TODO: Replace with f32::NAN()

#include <concepts>
#include <tuple>
#include <utility>  // std::tuple_size.

#include "mem/clone.h"
#include "mem/copy.h"
#include "mem/move.h"
#include "num/types.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::tuple::Tuple;

static_assert(sus::mem::Copy<Tuple<i32>>);
static_assert(sus::mem::Clone<Tuple<i32>>);
static_assert(sus::mem::Move<Tuple<i32>>);

// Tuple is standard layout for one element.
static_assert(std::is_standard_layout_v<Tuple<i32>>);
// TODO: It would be nice to be standard layout for more! Then you can do fun
// stuff with tuples in unions, and we could optimize Union more
// (https://patshaughnessy.net/2018/3/15/how-rust-implements-tagged-unions). But
// we'd need to not use inheritence and [[no_unique_address]] does not pack as
// well.
static_assert(!std::is_standard_layout_v<Tuple<i32, i32>>);

static_assert(
    std::same_as<i32, std::tuple_element_t<0, Tuple<i32, i32&, const i32&>>>);
static_assert(
    std::same_as<i32&, std::tuple_element_t<1, Tuple<i32, i32&, const i32&>>>);
static_assert(
    std::same_as<const i32&,
                 std::tuple_element_t<2, Tuple<i32, i32&, const i32&>>>);

// Tuple packs stuff efficiently (except in MSVC). However sus::Tuple has extra
// space taken right now by the use-after-move flags. If we could borrow check
// at compile time then we could drop use-after-move checking.
using PackedTuple = Tuple<i32, i8, i64>;
static_assert(PackedTuple::protects_uam ||
              sizeof(PackedTuple) == sizeof(i64) * sus_if_msvc_else(3u, 2u));
static_assert(!PackedTuple::protects_uam ||
              sizeof(PackedTuple) == sizeof(i64) * sus_if_msvc_else(4u, 3u));

// The std::tuple doesn't have use-after-move checks.
using PackedStdTuple = std::tuple<i32, i8, i64>;
static_assert(sizeof(PackedStdTuple) == sizeof(i64) * sus_if_msvc_else(3u, 2u));

TEST(Tuple, With) {
  auto t1 = Tuple<int>::with(2);
  auto t2 = Tuple<int, float>::with(2, 3.f);
  auto t3 = Tuple<int, float, int>::with(2, 3.f, 4);

  [[maybe_unused]] constexpr auto c = Tuple<int, float>::with(2, 3.f);
}

TEST(TupleDeathTest, Move) {
  auto t1 = Tuple<int>::with(2);
  auto t2 = sus::move(t1);
  EXPECT_EQ(t2.get_ref<0>(), 2);
  EXPECT_DEATH(t1.get_ref<0>(), "");
}

template <size_t I>
constexpr auto GetFromTuple() noexcept {
  constexpr auto t = Tuple<int, float>::with(2, 3.f);
  return t.get_ref<I>();
};

TEST(Tuple, GetRef) {
  auto t1 = Tuple<int>::with(2);
  EXPECT_EQ(t1.get_ref<0>(), 2);
  static_assert(std::same_as<const int&, decltype(t1.get_ref<0>())>);

  const auto t2 = Tuple<int, float>::with(2, 3.f);
  EXPECT_EQ(t2.get_ref<0>(), 2);
  static_assert(std::same_as<const int&, decltype(t2.get_ref<0>())>);
  EXPECT_EQ(t2.get_ref<1>(), 3.f);
  static_assert(std::same_as<const float&, decltype(t2.get_ref<1>())>);

  auto t3 = Tuple<int, float, int>::with(2, 3.f, 4);
  EXPECT_EQ(t3.get_ref<0>(), 2);
  static_assert(std::same_as<const int&, decltype(t3.get_ref<0>())>);
  EXPECT_EQ(t3.get_ref<1>(), 3.f);
  static_assert(std::same_as<const float&, decltype(t3.get_ref<1>())>);
  EXPECT_EQ(t3.get_ref<2>(), 4);
  static_assert(std::same_as<const int&, decltype(t3.get_ref<2>())>);

  [[maybe_unused]] constexpr auto c0 = GetFromTuple<0>();
  [[maybe_unused]] constexpr auto c1 = GetFromTuple<1>();
}

TEST(Tuple, GetMut) {
  auto t1 = Tuple<int>::with(2);
  EXPECT_EQ(t1.get_mut<0>(), 2);
  t1.get_mut<0>() += 1;
  EXPECT_EQ(t1.get_mut<0>(), 3);
  static_assert(std::same_as<int&, decltype(t1.get_mut<0>())>);

  auto t2 = Tuple<int, float>::with(2, 3.f);
  EXPECT_EQ(t2.get_mut<0>(), 2);
  t2.get_mut<0>() += 1;
  EXPECT_EQ(t2.get_mut<0>(), 3);
  static_assert(std::same_as<int&, decltype(t2.get_mut<0>())>);
  EXPECT_EQ(t2.get_mut<1>(), 3.f);
  t2.get_mut<1>() += 1.f;
  EXPECT_EQ(t2.get_mut<1>(), 4.f);
  static_assert(std::same_as<float&, decltype(t2.get_mut<1>())>);

  auto t3 = Tuple<int, float, int>::with(2, 3.f, 4);
  EXPECT_EQ(t3.get_mut<0>(), 2);
  t3.get_mut<0>() += 1;
  EXPECT_EQ(t3.get_mut<0>(), 3);
  static_assert(std::same_as<int&, decltype(t3.get_mut<0>())>);
  EXPECT_EQ(t3.get_mut<1>(), 3.f);
  t3.get_mut<1>() += 1.f;
  EXPECT_EQ(t3.get_mut<1>(), 4.f);
  static_assert(std::same_as<float&, decltype(t3.get_mut<1>())>);
  EXPECT_EQ(t3.get_mut<2>(), 4);
  t3.get_mut<2>() += 1;
  EXPECT_EQ(t3.get_mut<2>(), 5);
  static_assert(std::same_as<int&, decltype(t3.get_mut<2>())>);
}

TEST(Tuple, Unwrap) {
  {
    auto make = []() { return Tuple<int>::with(2); };
    EXPECT_EQ(make().unwrap<0>(), 2);
    static_assert(std::same_as<int&&, decltype(make().unwrap<0>())>);
  }
  {
    auto make = []() { return Tuple<int, float>::with(2, 3.f); };
    EXPECT_EQ(make().unwrap<0>(), 2);
    static_assert(std::same_as<int&&, decltype(make().unwrap<0>())>);
    EXPECT_EQ(make().unwrap<1>(), 3.f);
    static_assert(std::same_as<float&&, decltype(make().unwrap<1>())>);
  }
  {
    auto make = []() { return Tuple<int, float, int>::with(2, 3.f, 4); };
    EXPECT_EQ(make().unwrap<0>(), 2);
    static_assert(std::same_as<int&&, decltype(make().unwrap<0>())>);
    EXPECT_EQ(make().unwrap<1>(), 3.f);
    static_assert(std::same_as<float&&, decltype(make().unwrap<1>())>);
    EXPECT_EQ(make().unwrap<2>(), 4);
    static_assert(std::same_as<int&&, decltype(make().unwrap<2>())>);
  }

  static int counter = 0;
  struct MoveCounter final {
    MoveCounter() = default;
    MoveCounter(MoveCounter&&) { counter++; }
  };
  auto m = Tuple<MoveCounter>::with(MoveCounter()).unwrap<0>();
  EXPECT_EQ(counter, 2);  // One move into the Tuple, and one move out.

  [[maybe_unused]] constexpr auto c0 =
      Tuple<int, float>::with(2, 3.f).unwrap<0>();
  [[maybe_unused]] constexpr auto c1 =
      Tuple<int, float>::with(2, 3.f).unwrap<1>();
}

TEST(TupleDeathTest, Moved) {
#if GTEST_HAS_DEATH_TEST
  auto m1 = Tuple<int>::with(1);
  sus::move(m1).unwrap<0>();
  EXPECT_DEATH(m1.get_ref<0>(), "");

  auto m2 = Tuple<int>::with(1);
  sus::move(m2).unwrap<0>();
  EXPECT_DEATH(m2.get_mut<0>(), "");

  auto m3 = Tuple<int>::with(1);
  sus::move(m3).unwrap<0>();
  EXPECT_DEATH(sus::move(m3).unwrap<0>(), "");
#endif
}

TEST(Tuple, Eq) {
  EXPECT_EQ(Tuple<int>::with(1), Tuple<int>::with(1));
  EXPECT_NE(Tuple<int>::with(1), Tuple<int>::with(2));
  EXPECT_EQ((Tuple<int, int>::with(2, 1)), (Tuple<int, int>::with(2, 1)));
  EXPECT_NE((Tuple<int, int>::with(2, 1)), (Tuple<int, int>::with(2, 2)));

  int i;
  EXPECT_EQ(Tuple<int*>::with(&i), Tuple<int*>::with(&i));

  EXPECT_EQ(Tuple<float>::with(1.f), Tuple<float>::with(1.f));
  EXPECT_EQ(Tuple<float>::with(0.f), Tuple<float>::with(0.f));
  EXPECT_EQ(Tuple<float>::with(0.f), Tuple<float>::with(-0.f));
  EXPECT_NE(Tuple<float>::with(/* TODO: f32::NAN() */ NAN),
            Tuple<float>::with(/* TODO: f32::NAN() */ NAN));
}

TEST(Tuple, Ord) {
  EXPECT_LT(Tuple<int>::with(1), Tuple<int>::with(2));
  EXPECT_GT(Tuple<int>::with(3), Tuple<int>::with(2));
  EXPECT_GT((Tuple<int, int>::with(3, 4)), (Tuple<int, int>::with(3, 3)));
  EXPECT_GE((Tuple<int, int>::with(3, 4)), (Tuple<int, int>::with(3, 3)));
  EXPECT_GE((Tuple<int, int>::with(3, 3)), (Tuple<int, int>::with(3, 3)));
  EXPECT_GT((Tuple<int, int, int>::with(3, 4, 2)),
            (Tuple<int, int, int>::with(3, 3, 3)));

  int i[2];
  EXPECT_LT(Tuple<int*>::with(&i[0]), Tuple<int*>::with(&i[1]));
}

TEST(Tuple, StrongOrder) {
  EXPECT_EQ(std::strong_order(Tuple<int>::with(12), Tuple<int>::with(12)),
            std::strong_ordering::equal);
  EXPECT_EQ(std::strong_order(Tuple<int>::with(12), Tuple<int>::with(12)),
            std::strong_ordering::equivalent);
  EXPECT_EQ(std::strong_order(Tuple<int>::with(12), Tuple<int>::with(13)),
            std::strong_ordering::less);
  EXPECT_EQ(std::strong_order(Tuple<int, int>::with(12, 13),
                              Tuple<int, int>::with(12, 12)),
            std::strong_ordering::greater);
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

TEST(Tuple, WeakOrder) {
  EXPECT_EQ(std::weak_order(Tuple<Weak>::with(Weak(1, 2)),
                            Tuple<Weak>::with(Weak(1, 2))),
            std::weak_ordering::equivalent);
  EXPECT_EQ(std::weak_order(Tuple<Weak>::with(Weak(1, 2)),
                            Tuple<Weak>::with(Weak(1, 3))),
            std::weak_ordering::equivalent);
  EXPECT_EQ(std::weak_order(Tuple<Weak>::with(Weak(1, 2)),
                            Tuple<Weak>::with(Weak(2, 3))),
            std::weak_ordering::less);
  EXPECT_EQ(std::weak_order(Tuple<Weak, Weak>::with(Weak(1, 2), Weak(1, 3)),
                            Tuple<Weak, Weak>::with(Weak(1, 1), Weak(1, 4))),
            std::weak_ordering::equivalent);
  EXPECT_EQ(std::weak_order(Tuple<Weak, Weak>::with(Weak(1, 2), Weak(2, 3)),
                            Tuple<Weak, Weak>::with(Weak(1, 1), Weak(1, 4))),
            std::weak_ordering::greater);
  EXPECT_EQ(std::weak_order(Tuple<Weak, Weak>::with(Weak(1, 2), Weak(2, 3)),
                            Tuple<Weak, Weak>::with(Weak(2, 1), Weak(1, 4))),
            std::weak_ordering::less);
}

TEST(Tuple, PartialOrder) {
  EXPECT_EQ(
      std::partial_order(Tuple<float>::with(12.f), Tuple<float>::with(12.f)),
      std::partial_ordering::equivalent);
  EXPECT_EQ(std::partial_order(Tuple<float, float>::with(12.f, 13.f),
                               Tuple<float, float>::with(12.f, 11.f)),
            std::partial_ordering::greater);
  EXPECT_EQ(
      std::partial_order(Tuple<float>::with(0.f), Tuple<float>::with(-0.f)),
      std::partial_ordering::equivalent);
  EXPECT_EQ(
      std::partial_order(Tuple<float>::with(0.f), Tuple<float>::with(1.f)),
      std::partial_ordering::less);
  EXPECT_EQ(std::partial_order(Tuple<float>::with(0.f),
                               Tuple<float>::with(/* TODO: f32::NAN() */ NAN)),
            std::partial_ordering::unordered);
  EXPECT_EQ(std::partial_order(Tuple<float>::with(/* TODO: f32::NAN() */ NAN),
                               Tuple<float>::with(/* TODO: f32::NAN() */ NAN)),
            std::partial_ordering::unordered);
  EXPECT_EQ(std::partial_order(
                Tuple<float>::with(0.f),
                Tuple<float>::with(/* TODO: f32::INFINITY() */ HUGE_VALF)),
            std::partial_ordering::less);
  EXPECT_EQ(std::partial_order(
                Tuple<float>::with(0.f),
                Tuple<float>::with(/* TODO: f32::NEG_INFINITY() */ -HUGE_VALF)),
            std::partial_ordering::greater);
}

TEST(Tuple, StructuredBinding) {
  auto t3 = Tuple<int, float, char>::with(2, 3.f, 'c');
  auto& [a, b, c] = t3;
  static_assert(std::same_as<decltype(a), int>);
  static_assert(std::same_as<decltype(b), float>);
  static_assert(std::same_as<decltype(c), char>);

  t3.get_mut<0>() += 1;
  t3.get_mut<1>() += 2.f;
  t3.get_mut<2>() += 3;
  EXPECT_EQ(t3, (Tuple<int, float, char>::with(3, 5.f, 'f')));

  const auto& [d, e, f] = t3;
  static_assert(std::same_as<decltype(d), const int>);
  static_assert(std::same_as<decltype(e), const float>);
  static_assert(std::same_as<decltype(f), const char>);
  EXPECT_EQ((Tuple<int, float, char>::with(d, e, f)),
            (Tuple<int, float, char>::with(3, 5.f, 'f')));

  auto [g, h, i] = sus::move(t3);
  static_assert(std::same_as<decltype(g), int>);
  static_assert(std::same_as<decltype(h), float>);
  static_assert(std::same_as<decltype(i), char>);
  EXPECT_EQ((Tuple<int, float, char>::with(d, e, f)),
            (Tuple<int, float, char>::with(3, 5.f, 'f')));
}

TEST(Tuple, References) {
  i32 i = 1, j = 2;
  auto t = Tuple<i32&, const i32&, i32>::with(i, j, 3);
  EXPECT_EQ(t.get_ref<0>(), 1);
  EXPECT_EQ(t.get_ref<1>(), 2);
  static_assert(std::same_as<const i32&, decltype(t.get_ref<0>())>);
  static_assert(std::same_as<const i32&, decltype(t.get_ref<1>())>);

  EXPECT_EQ(t.get_mut<0>(), 1);
  static_assert(std::same_as<i32&, decltype(t.get_mut<0>())>);

  static_assert(std::same_as<i32&, decltype(sus::move(t).unwrap<0>())>);
  static_assert(std::same_as<const i32&, decltype(sus::move(t).unwrap<1>())>);
  static_assert(std::same_as<i32&&, decltype(sus::move(t).unwrap<2>())>);
  EXPECT_EQ(std::move(t).unwrap<0>(), 1);
}

template <class T, size_t I>
concept HasGetMut = requires(T t) {
                      { t.template get_mut<I>() };
                    };

static_assert(HasGetMut<Tuple<i32, i32&, const i32&>, 0>);
static_assert(HasGetMut<Tuple<i32, i32&, const i32&>, 1>);
static_assert(!HasGetMut<Tuple<i32, i32&, const i32&>, 2>);

TEST(Tuple, Destroy) {
  static usize destroy = 0u;
  struct S {
    S(usize i) : i(i) {}
    ~S() { destroy = (destroy + i) * i; }
    usize i;
  };

  {
    auto t = Tuple<S, S, S>::with(S(1u), S(2u), S(3u));
    destroy = 0u;
  }
  // Tuple elements are destroyed from first to last.
  EXPECT_EQ(destroy.primitive_value, (((0u + 1u) * 1u + 2u) * 2u + 3u) * 3u);
}

}  // namespace
