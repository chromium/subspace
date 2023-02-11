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

#include "subspace/tuple/tuple.h"

#include <math.h>  // TODO: Replace with f32::NAN()

#include <concepts>
#include <tuple>
#include <utility>  // std::tuple_size.

#include "googletest/include/gtest/gtest.h"
#include "subspace/mem/clone.h"
#include "subspace/mem/copy.h"
#include "subspace/mem/move.h"
#include "subspace/num/types.h"
#include "subspace/prelude.h"
#include "subspace/test/no_copy_move.h"

namespace {

using sus::Tuple;
using sus::test::NoCopyMove;

static_assert(sus::mem::Copy<Tuple<i32>>);
static_assert(sus::mem::Clone<Tuple<i32>>);
static_assert(sus::mem::Move<Tuple<i32>>);

struct Cloneable {
  Cloneable();
  Cloneable(Cloneable&&);
  void operator=(Cloneable&&);
  Cloneable clone() const;
};

static_assert(!sus::mem::Copy<Tuple<Cloneable>>);
static_assert(sus::mem::Clone<Tuple<Cloneable>>);
static_assert(sus::mem::Move<Tuple<Cloneable>>);
static_assert(sus::mem::Copy<Tuple<Cloneable&>>);
static_assert(sus::mem::Clone<Tuple<Cloneable&>>);
static_assert(sus::mem::Move<Tuple<Cloneable&>>);

static_assert(
    std::same_as<i32, std::tuple_element_t<0, Tuple<i32, i32&, const i32&>>>);
static_assert(
    std::same_as<i32&, std::tuple_element_t<1, Tuple<i32, i32&, const i32&>>>);
static_assert(
    std::same_as<const i32&,
                 std::tuple_element_t<2, Tuple<i32, i32&, const i32&>>>);

// Tuples can be built with use-after-move checks which consume an extra 8
// bytes.
inline constexpr size_t UamBytes = Tuple<i32>::protects_uam ? 8 : 0;

TEST(Tuple, TailPadding) {
  // Tuple packs stuff efficiently (except in MSVC). However sus::Tuple has
  // extra space taken right now by the use-after-move flags. If we could borrow
  // check at compile time then we could drop use-after-move checking.
  using PackedTuple = Tuple<i8, i32, i64>;
  static_assert(sizeof(PackedTuple) ==
                (sizeof(i64) * sus_if_msvc_else(3u, 2u)) + UamBytes);

  // The std::tuple doesn't have use-after-move checks.
  using PackedStdTuple = std::tuple<i32, i8, i64>;
  static_assert(sizeof(PackedStdTuple) ==
                sizeof(i64) * sus_if_msvc_else(3u, 2u));

  // Tuple packs stuff inside each inner type's tail padding as well (except in
  // MSVC).
  struct TailPadding {
    u64 i;

   private:  // Not standard-layout so can use the tail padding.
    u32 j;
  };
  using PackedTupleMore = Tuple<i8, TailPadding>;
  static_assert(sizeof(PackedTupleMore) ==
                (sizeof(u64) * sus_if_msvc_else(3u, 2u)) + UamBytes);

  // Test the same (tuple packing inside inner types' tail padding) with more
  // than one type collapsing (except in MSVC).
  struct TailPaddingLarge {
    u64 i;

   private:  // Not standard-layout so can use the tail padding.
    u8 j;
  };
  struct TailPaddingSmall {
    u16 i;

   private:  // Not standard-layout so can use the tail padding.
    u8 j;
  };
  using PackedTupleTwiceMore = Tuple<i8, TailPaddingSmall, TailPaddingLarge>;
  static_assert(sizeof(PackedTupleTwiceMore) ==
                (sizeof(i64) * sus_if_msvc_else(4u, 2u)) + UamBytes);

  // The Tuple type, if it has tail padding, allows types to make use of that
  // tail padding.
  struct WithTuple {
    [[sus_no_unique_address]] PackedTuple t;
    char c;  // Is stored in the padding of `t` (except on MSVC).
  };
  static_assert(sizeof(WithTuple) == sizeof(std::declval<WithTuple&>().t) +
                                         sus_if_msvc_else(sizeof(i64), 0));

  // The example from the Tuple docs.
  struct ExampleFromDocs {
    [[sus_no_unique_address]] Tuple<u32, u64> tuple;  // 16 bytes.
    u32 val;                                          // 4 bytes.
  };  // 16 bytes, since `val` is stored inside `tuple`.
  static_assert(sizeof(ExampleFromDocs) ==
                (16 + UamBytes) + sus_if_msvc_else(8, 0));
}

TEST(Tuple, With) {
  auto t1 = Tuple<i32>::with(2);
  auto t2 = Tuple<i32, f32>::with(2, 3.f);
  auto t3 = Tuple<i32, f32, i32>::with(2, 3.f, 4);

  [[maybe_unused]] constexpr auto c = Tuple<i32, f32>::with(2, 3.f);
}

TEST(Tuple, ConstructorFunction) {
  {
    // All parameters match the tuple type.
    Tuple<u32, u32, u32> a = sus::tuple(1_u32, 2_u32, 3_u32);
    EXPECT_EQ(a.get_ref<0>(), 1_u32);
    EXPECT_EQ(a.get_ref<1>(), 2_u32);
    EXPECT_EQ(a.get_ref<2>(), 3_u32);
  }
  {
    // Some parameters convert to u32.
    Tuple<u32, u32, u32> a = sus::tuple(1_u32, 2u, 3_u32);
    EXPECT_EQ(a.get_ref<0>(), 1_u32);
    EXPECT_EQ(a.get_ref<1>(), 2_u32);
    EXPECT_EQ(a.get_ref<2>(), 3_u32);
  }
  {
    // All parameters convert to u32.
    Tuple<u32, u32, u32> a = sus::tuple(1u, 2u, 3u);
    EXPECT_EQ(a.get_ref<0>(), 1_u32);
    EXPECT_EQ(a.get_ref<1>(), 2_u32);
    EXPECT_EQ(a.get_ref<2>(), 3_u32);
  }
  {
    // into() as an input to the tuple.
    Tuple<u32, u32, u32> a = sus::tuple(1_u32, sus::into(2), 3_u32);
    EXPECT_EQ(a.get_ref<0>(), 1_u32);
    EXPECT_EQ(a.get_ref<1>(), 2_u32);
    EXPECT_EQ(a.get_ref<2>(), 3_u32);
  }
  {
    // Copies the lvalue and const lvalue.
    auto i = 1_u32;
    const auto j = 2_u32;
    Tuple<u32, u32, u32> a = sus::tuple(i, j, 3_u32);
    EXPECT_EQ(a.get_ref<0>(), 1_u32);
    EXPECT_EQ(a.get_ref<1>(), 2_u32);
    EXPECT_EQ(a.get_ref<2>(), 3_u32);
  }
  {
    // Copies the rvalue reference.
    auto i = 1_u32;
    Tuple<u32, u32, u32> a = sus::tuple(sus::move(i), 2_u32, 3_u32);
    EXPECT_EQ(a.get_ref<0>(), 1_u32);
    EXPECT_EQ(a.get_ref<1>(), 2_u32);
    EXPECT_EQ(a.get_ref<2>(), 3_u32);
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
    auto marker = sus::tuple(s);
    EXPECT_EQ(copies, 0);
    Tuple<S> tuple = sus::move(marker);
    EXPECT_GE(copies, 1);
  }
}

TEST(Tuple, ConstructorReferences) {
  {
    // All parameters match the tuple type.
    [](Tuple<const u32&, const u32&, const u32&> a) {
      EXPECT_EQ(a.get_ref<0>(), 1_u32);
      EXPECT_EQ(a.get_ref<1>(), 2_u32);
      EXPECT_EQ(a.get_ref<2>(), 3_u32);
    }(sus::tuple(1_u32, 2_u32, 3_u32));
  }
}

TEST(Tuple, Copy) {
  {
    auto t1 = Tuple<i32>::with(2);
    auto t2 = t1;
    EXPECT_EQ(t1, t2);
  }

  {
    auto n = NoCopyMove();
    auto t1 = Tuple<NoCopyMove&>::with(mref(n));
    auto t2 = t1;
    EXPECT_EQ(t1, t2);
  }
}

TEST(Tuple, CloneCopy) {
  {
    auto t1 = Tuple<i32>::with(2);
    auto t2 = sus::clone(t1);
    EXPECT_EQ(t1, t2);
  }

  {
    auto n = NoCopyMove();
    auto t1 = Tuple<NoCopyMove&>::with(mref(n));
    auto t2 = sus::clone(t1);
    EXPECT_EQ(t1, t2);
  }
}

TEST(Tuple, Clone) {
  struct Cloneable {
    Cloneable(i32 i) : i(i) {}
    Cloneable(Cloneable&&) = default;
    Cloneable& operator=(Cloneable&&) = default;

    Cloneable clone() const { return Cloneable(i + 1_i32); }

    i32 i = 2_i32;
  };

  auto t1 = Tuple<Cloneable>::with(2_i32);
  auto t2 = ::sus::clone(t1);
  EXPECT_EQ(t1.get_ref<0>().i + 1_i32, t2.get_ref<0>().i);
}

TEST(TupleDeathTest, Move) {
  {
    auto t1 = Tuple<int>::with(2);
    auto t2 = sus::move(t1);
    EXPECT_EQ(t2.get_ref<0>(), 2);
    EXPECT_DEATH(t1.get_ref<0>(), "");
  }

  {
    auto n = NoCopyMove();
    auto t1 = Tuple<NoCopyMove&>::with(mref(n));
    auto t2 = sus::move(t1);
    EXPECT_EQ(t2.get_ref<0>(), n);
    EXPECT_DEATH(t1.get_ref<0>(), "");
  }
}

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

  auto n = NoCopyMove();
  auto tn = Tuple<NoCopyMove&>::with(mref(n));
  static_assert(std::same_as<const NoCopyMove&, decltype(tn.get_ref<0>())>);
  EXPECT_EQ(tn.get_ref<0>(), n);

  [[maybe_unused]] constexpr auto c0 = []() {
    constexpr auto t = Tuple<int, float>::with(2, 3.f);
    return t.get_ref<0>();
  }();
  [[maybe_unused]] constexpr auto c1 = []() {
    constexpr auto t = Tuple<int, float>::with(2, 3.f);
    return t.get_ref<1>();
  }();
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

  auto n = NoCopyMove();
  auto tn = Tuple<NoCopyMove&>::with(mref(n));
  static_assert(std::same_as<NoCopyMove&, decltype(tn.get_mut<0>())>);
  EXPECT_EQ(tn.get_mut<0>(), n);
}

TEST(Tuple, IntoInner) {
  auto t1 = Tuple<i32, u32>::with(2, 3u);
  static_assert(std::same_as<decltype(sus::move(t1).into_inner<0>()), i32&&>);
  static_assert(std::same_as<decltype(sus::move(t1).into_inner<1>()), u32&&>);
  EXPECT_EQ(sus::move(t1).into_inner<0>(), 2_i32);

  auto n = NoCopyMove();
  auto tn = Tuple<NoCopyMove&>::with(mref(n));
  static_assert(
      std::same_as<NoCopyMove&, decltype(sus::move(tn).into_inner<0>())>);
  EXPECT_EQ(sus::move(tn).into_inner<0>(), n);
}

TEST(Tuple, Eq) {
  struct NotEq {};
  static_assert(!sus::ops::Eq<NotEq>);
  static_assert(sus::ops::Eq<Tuple<int>>);
  static_assert(!sus::ops::Eq<Tuple<NotEq>>);

  EXPECT_EQ(Tuple<int>::with(1), Tuple<int>::with(1));
  EXPECT_NE(Tuple<int>::with(1), Tuple<int>::with(2));
  EXPECT_EQ((Tuple<int, int>::with(2, 1)), (Tuple<int, int>::with(2, 1)));
  EXPECT_NE((Tuple<int, int>::with(2, 1)), (Tuple<int, int>::with(2, 2)));

  int i;
  EXPECT_EQ(Tuple<int*>::with(&i), Tuple<int*>::with(&i));

  EXPECT_EQ(Tuple<f32>::with(1.f), Tuple<f32>::with(1.f));
  EXPECT_EQ(Tuple<f32>::with(0.f), Tuple<f32>::with(0.f));
  EXPECT_EQ(Tuple<f32>::with(0.f), Tuple<f32>::with(-0.f));
  EXPECT_NE(Tuple<f32>::with(f32::NAN), Tuple<f32>::with(f32::NAN));

  auto n1 = NoCopyMove();
  auto tn1 = Tuple<NoCopyMove&>::with(mref(n1));
  auto n2 = NoCopyMove();
  auto tn2 = Tuple<NoCopyMove&>::with(mref(n2));
  EXPECT_EQ(tn1, tn1);
  EXPECT_NE(tn1, tn2);
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

  NoCopyMove ns[] = {NoCopyMove(), NoCopyMove()};
  auto tn1 = Tuple<NoCopyMove&>::with(mref(ns[0]));
  auto tn2 = Tuple<NoCopyMove&>::with(mref(ns[1]));
  EXPECT_GE(tn1, tn1);
  EXPECT_LT(tn1, tn2);
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
  EXPECT_EQ(std::partial_order(Tuple<f32>::with(12.f), Tuple<f32>::with(12.f)),
            std::partial_ordering::equivalent);
  EXPECT_EQ(std::partial_order(Tuple<f32, f32>::with(12.f, 13.f),
                               Tuple<f32, f32>::with(12.f, 11.f)),
            std::partial_ordering::greater);
  EXPECT_EQ(std::partial_order(Tuple<f32>::with(0.f), Tuple<f32>::with(-0.f)),
            std::partial_ordering::equivalent);
  EXPECT_EQ(std::partial_order(Tuple<f32>::with(0.f), Tuple<f32>::with(1.f)),
            std::partial_ordering::less);
  EXPECT_EQ(
      std::partial_order(Tuple<f32>::with(0.f), Tuple<f32>::with(f32::NAN)),
      std::partial_ordering::unordered);
  EXPECT_EQ(std::partial_order(Tuple<f32>::with(f32::NAN),
                               Tuple<f32>::with(f32::NAN)),
            std::partial_ordering::unordered);
  EXPECT_EQ(std::partial_order(Tuple<f32>::with(0.f),
                               Tuple<f32>::with(f32::INFINITY)),
            std::partial_ordering::less);
  EXPECT_EQ(std::partial_order(Tuple<f32>::with(0.f),
                               Tuple<f32>::with(f32::NEG_INFINITY)),
            std::partial_ordering::greater);
}

struct NotCmp {};
static_assert(!sus::ops::PartialOrd<NotCmp>);

static_assert(sus::ops::Ord<Tuple<int>>);
static_assert(!sus::ops::Ord<Tuple<Weak>>);
static_assert(sus::ops::WeakOrd<Tuple<Weak>>);
static_assert(!sus::ops::WeakOrd<Tuple<float>>);
static_assert(!sus::ops::WeakOrd<Tuple<float>>);
static_assert(!sus::ops::PartialOrd<Tuple<NotCmp>>);

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

TEST(Tuple, StructuredBindingMoves) {
  static usize moves = 0u;
  struct Moves {
    Moves() {}
    Moves(Moves&&) { moves += 1u; }
  };
  auto t = Tuple<Moves, Moves, Moves>();
  moves = 0u;
  auto [a, b, c] = sus::move(t);
  EXPECT_EQ(moves, 3u);
}

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
