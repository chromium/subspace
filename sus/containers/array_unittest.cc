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

#include "sus/containers/array.h"

#include <sstream>
#include <type_traits>

#include "googletest/include/gtest/gtest.h"
#include "sus/construct/into.h"
#include "sus/iter/iterator.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/num/types.h"
#include "sus/prelude.h"
#include "sus/test/ensure_use.h"

using sus::containers::Array;
using sus::mem::relocate_by_memcpy;

namespace {

static_assert(sus::mem::Clone<Array<i32, 0>>);
static_assert(sus::mem::Clone<Array<i32, 3>>);
static_assert(sus::mem::Clone<sus::containers::ArrayIntoIter<i32, 0>>);
static_assert(sus::mem::Clone<sus::containers::ArrayIntoIter<i32, 3>>);

static_assert(sus::iter::Iterator<sus::containers::ArrayIntoIter<i32, 0>, i32>);
static_assert(sus::iter::Iterator<sus::containers::ArrayIntoIter<i32, 3>, i32>);
static_assert(sus::iter::DoubleEndedIterator<
              sus::containers::ArrayIntoIter<i32, 0>, i32>);
static_assert(sus::iter::DoubleEndedIterator<
              sus::containers::ArrayIntoIter<i32, 3>, i32>);
static_assert(
    sus::iter::ExactSizeIterator<sus::containers::ArrayIntoIter<i32, 0>, i32>);
static_assert(
    sus::iter::ExactSizeIterator<sus::containers::ArrayIntoIter<i32, 3>, i32>);

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
static_assert(!std::is_trivially_move_constructible_v<Array<T, 2>>);
static_assert(!std::is_trivially_move_assignable_v<Array<T, 2>>);
static_assert(std::is_move_constructible_v<Array<T, 2>>);
static_assert(std::is_move_assignable_v<Array<T, 2>>);
static_assert(std::is_nothrow_swappable_v<Array<T, 2>>);
static_assert(!std::is_trivially_copy_constructible_v<Array<T, 2>>);
static_assert(!std::is_trivially_copy_assignable_v<Array<T, 2>>);
static_assert(!std::is_constructible_v<Array<T, 2>, T&&>);
static_assert(!std::is_assignable_v<Array<T, 2>, T&&>);
static_assert(!std::is_constructible_v<Array<T, 2>, const T&>);
static_assert(!std::is_assignable_v<Array<T, 2>, const T&>);
static_assert(!std::is_constructible_v<Array<T, 2>, T>);
static_assert(!std::is_assignable_v<Array<T, 2>, T>);
static_assert(std::is_nothrow_destructible_v<Array<T, 2>>);
static_assert(relocate_by_memcpy<Array<T, 2>>);
}  // namespace trivially_relocatable

struct NonAggregate {
  ~NonAggregate() {}
};

TEST(Array, Default) {
  auto a = Array<int, 5>();
  for (usize i; i < 5u; i += 1u) {
    EXPECT_EQ(a[i], 0);
  }
}

TEST(Array, Zero) {
  auto a = Array<int, 0>();
  static_assert(sizeof(a) == 1);
  static_assert(sizeof(Array<int, 0>) == 1);
}

TEST(Array, WithInitializer) {
  auto a =
      Array<usize, 5>::with_initializer([i = 1u]() mutable { return i++; });
  for (auto i = 0_usize; i < 5u; i += 1u) {
    EXPECT_EQ(a[i], i + 1u);
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
  for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
    EXPECT_EQ(b[i].i, i + 1_usize);
  }

  auto lvalue = [i = 1u]() mutable { return i++; };
  auto c = Array<usize, 5>::with_initializer(lvalue);
  for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
    EXPECT_EQ(c[i], i + 1_usize);
  }
}

TEST(Array, WithValue) {
  auto a = Array<usize, 5>::with_value(3u);
  for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
    EXPECT_EQ(a[i], 3_usize);
  }
}

TEST(Array, WithValues) {
  {
    auto a = Array<usize, 5>::with(3u, 4u, 5u, 6u, 7u);
    for (auto i = 0_usize; i < 5_usize; i += 1_usize) {
      EXPECT_EQ(a[i], 3_usize + i);
    }
  }
  {
    auto a = Array<u8, 5>::with(3_u8, 4_u8, 5_u8, 6_u8, 7_u8);
    for (auto i = 0_u8; i < 5_u8; i += 1_u8) {
      EXPECT_EQ(a[usize::from(i)], 3_u8 + i);
    }
  }
}

TEST(Array, ConstructorFunction) {
  {
    // All parameters match the array type.
    Array<u32, 3> a = sus::array(1_u32, 2_u32, 3_u32);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // Some parameters convert to u32.
    Array<u32, 3> a = sus::array(1_u32, 2u, 3_u32);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // All parameters convert to u32.
    Array<u32, 3> a = sus::array(1u, 2u, 3u);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // into() as an input to the array.
    Array<u32, 3> a = sus::array(1_u32, sus::into(2_u16), 3_u32);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // Copies the lvalue and const lvalue.
    auto i = 1_u32;
    const auto j = 2_u32;
    Array<u32, 3> a = sus::array(i, j, 3_u32);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // Copies the rvalue reference.
    auto i = 1_u32;
    Array<u32, 3> a = sus::array(sus::move(i), 2_u32, 3_u32);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
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
    auto marker = sus::array(s);
    EXPECT_EQ(copies, 0);
    Array<S, 1> a = sus::move(marker);
    EXPECT_GE(copies, 1);
  }

  // In place explicit construction.
  {
    auto a = sus::array(1_i32, 2_i32).construct();
    static_assert(std::same_as<decltype(a), Array<i32, 2u>>);
    EXPECT_EQ(a[0u], 1_i32);
    EXPECT_EQ(a[1u], 2_i32);

    auto b = sus::array(1, 2).construct<i32>();
    static_assert(std::same_as<decltype(b), Array<i32, 2u>>);
    EXPECT_EQ(b[0u], 1_i32);
    EXPECT_EQ(b[1u], 2_i32);
  }
}

TEST(Array, Get) {
  {
    constexpr auto r = []() constexpr {
      constexpr auto a =
          Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
      return a.get(2_usize).unwrap();
    }();
    static_assert(std::same_as<decltype(r), const int>);
    EXPECT_EQ(3, r);
  }
  {
    auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
    EXPECT_EQ(3, a.get(2_usize).unwrap());
  }
}

TEST(Array, GetUnchecked) {
  {
    constexpr auto r = []() constexpr {
      constexpr auto a =
          Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
      return a.get_unchecked(unsafe_fn, 2_usize);
    }();
    static_assert(std::same_as<decltype(r), const int>);
    EXPECT_EQ(3, r);
  }
  {
    auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
    EXPECT_EQ(3, a.get_unchecked(unsafe_fn, 2_usize));
  }
}

template <class T, class U>
concept HasGetMut = requires(T t, U u) { t.get_mut(u); };

// get_mut() is only available for mutable arrays.
static_assert(!HasGetMut<const Array<i32, 3>, usize>);
static_assert(HasGetMut<Array<i32, 3>, usize>);

TEST(Array, GetMut) {
  {
    constexpr auto a = []() constexpr {
      auto a =
          Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
      a.get_mut(0_usize).unwrap() = 101;
      return a;
    }();
    EXPECT_EQ(a[0_usize], 101);
  }
  {
    auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
    a.get_mut(0_usize).unwrap() = 101;
    EXPECT_EQ(a[0_usize], 101);
  }
}

template <class T, class U>
concept HasGetUncheckedMut =
    requires(T t, U u) { t.get_unchecked_mut(unsafe_fn, u); };

// get_unchecked_mut() is only available for mutable arrays.
static_assert(!HasGetUncheckedMut<const Array<i32, 3>, usize>);
static_assert(HasGetUncheckedMut<Array<i32, 3>, usize>);

TEST(Array, GetUncheckedMut) {
  {
    constexpr auto a = []() constexpr {
      auto a =
          Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
      a.get_unchecked_mut(unsafe_fn, 0_usize) = 101;
      return a;
    }();
    EXPECT_EQ(a[0_usize], 101);
  }
  {
    auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
    a.get_unchecked_mut(unsafe_fn, 0_usize) = 101;
    EXPECT_EQ(a[0_usize], 101);
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
  auto r = a.as_mut_ptr();
  static_assert(std::same_as<decltype(r), int*>);
  *(r + 2) = 101;
  EXPECT_EQ(101, *(r + 2));
}

TEST(Array, Eq) {
  struct NotEq {};
  static_assert(!sus::ops::Eq<NotEq>);

  static_assert(sus::ops::Eq<Array<int, 3>>);
  static_assert(!sus::ops::Eq<Array<int, 3>, Array<int, 4>>);
  static_assert(!sus::ops::Eq<Array<NotEq, 3>>);

  auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  auto b = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  EXPECT_EQ(a, b);
  b[3_usize] += 1;
  EXPECT_NE(a, b);
}

TEST(Array, Ord) {
  auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  auto b = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  EXPECT_LE(a, b);
  EXPECT_GE(a, b);
  b[3_usize] += 1;
  EXPECT_LT(a, b);
}

TEST(Array, StrongOrder) {
  auto a = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  auto b = Array<int, 5>::with_initializer([i = 0]() mutable { return ++i; });
  EXPECT_EQ(std::strong_order(a, b), std::strong_ordering::equal);
  b[3_usize] += 1;
  EXPECT_EQ(std::strong_order(a, b), std::strong_ordering::less);
}

struct Weak final {
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

TEST(Array, WeakOrder) {
  auto a = Array<Weak, 5>::with_initializer(
      [i = 0]() mutable { return Weak(++i, 2); });
  auto b = Array<Weak, 5>::with_initializer(
      [i = 0]() mutable { return Weak(++i, 2); });
  EXPECT_EQ(std::weak_order(a, b), std::weak_ordering::equivalent);
  b[3_usize].a += 1;
  EXPECT_EQ(std::weak_order(a, b), std::weak_ordering::less);
}

TEST(Array, PartialOrder) {
  auto a =
      Array<float, 5>::with_initializer([i = 0.f]() mutable { return ++i; });
  auto b =
      Array<float, 5>::with_initializer([i = 0.f]() mutable { return ++i; });
  EXPECT_EQ(std::partial_order(a, b), std::partial_ordering::equivalent);
  b[3_usize] += 1;
  EXPECT_EQ(std::partial_order(a, b), std::partial_ordering::less);
}

struct NotCmp {};
static_assert(!sus::ops::PartialOrd<NotCmp>);

static_assert(!sus::ops::Ord<Array<int, 3>, Array<int, 4>>);
static_assert(!sus::ops::WeakOrd<Array<int, 3>, Array<int, 4>>);
static_assert(!sus::ops::PartialOrd<Array<int, 3>, Array<int, 4>>);

static_assert(sus::ops::Ord<Array<int, 3>>);
static_assert(!sus::ops::Ord<Array<Weak, 3>>);
static_assert(sus::ops::WeakOrd<Array<Weak, 3>>);
static_assert(!sus::ops::WeakOrd<Array<float, 3>>);
static_assert(!sus::ops::WeakOrd<Array<float, 3>>);
static_assert(!sus::ops::PartialOrd<Array<NotCmp, 3>>);

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

TEST(Array, IntoIterDoubleEnded) {
  auto a = Array<usize, 3>::with(1u, 2u, 3u);

  auto it = sus::move(a).into_iter();
  EXPECT_EQ(it.next_back(), sus::some(3_usize).construct());
  EXPECT_EQ(it.next_back(), sus::some(2_usize).construct());
  EXPECT_EQ(it.next_back(), sus::some(1_usize).construct());
  EXPECT_EQ(it.next_back(), sus::None);
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
  auto a = Array<usize, 3>::with(3u, 4u, 5u);
  auto a2 = sus::move(a).map(+[](usize i) { return u32::try_from(i + 1_usize).unwrap(); });
  EXPECT_EQ(a2, (Array<u32, 3>::with(4_u32, 5_u32, 6_u32)));
}

TEST(Array, Index) {
  const auto ac = Array<i32, 3>::with(1, 2, 3);
  auto am = Array<i32, 3>::with(1, 2, 3);

  EXPECT_EQ(ac[0_usize], 1_i32);
  EXPECT_EQ(ac[2_usize], 3_i32);
  EXPECT_EQ(am[0_usize], 1_i32);
  EXPECT_EQ(am[2_usize], 3_i32);
}

TEST(ArrayDeathTest, Index) {
  const auto ac = Array<i32, 3>::with(1, 2, 3);
  auto am = Array<i32, 3>::with(1, 2, 3);

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(ac[3_usize], "");
  EXPECT_DEATH(am[3_usize], "");
#endif
}

TEST(Array, AsSlice) {
  const auto ac = Array<i32, 3>::with_value(2);
  auto am = Array<i32, 3>::with_value(2);
  static_assert(std::same_as<decltype(ac.as_slice()), sus::Slice<i32>>);
  static_assert(std::same_as<decltype(am.as_slice()), sus::Slice<i32>>);
  EXPECT_EQ(ac.as_slice().len(), 3u);
  EXPECT_EQ(am.as_slice().len(), 3u);
}

TEST(Array, AsMutSlice) {
  auto am = Array<i32, 3>::with_value(2);
  static_assert(std::same_as<decltype(am.as_mut_slice()), sus::SliceMut<i32>>);
  EXPECT_EQ(am.as_mut_slice().len(), 3u);
}

TEST(Array, Clone) {
  struct Copy {
    Copy() {}
    Copy(const Copy& o) : i(o.i + 1_i32) {}
    Copy& operator=(const Copy&) = default;
    i32 i = 1_i32;
  };

  static_assert(::sus::mem::Copy<Copy>);
  static_assert(::sus::mem::Clone<Copy>);
  static_assert(::sus::mem::CloneInto<Copy>);
  static_assert(::sus::mem::Move<Copy>);
  // Array is always Clone (if T is Clone), but never Copy since it's expensive
  // to copy.
  static_assert(!::sus::mem::Copy<Array<Copy, 1>>);
  static_assert(::sus::mem::Clone<Array<Copy, 1>>);
  static_assert(::sus::mem::CloneInto<Array<Copy, 1>>);
  static_assert(::sus::mem::Move<Array<Copy, 1>>);

  // Cloning empty.
  {
    auto a = sus::clone(Array<Copy, 0>());
    auto b = Array<Copy, 0>();
    sus::clone_into(a, b);
  }

  {
    const auto s = Array<Copy, 1>::with(Copy());
    i32 i = s[0u].i;
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Array<Copy, 1>>);
    EXPECT_GT(s2[0u].i, i);
  }

  {
    auto s = Array<Copy, 1>::with(Copy());
    s[0u].i = 1000_i32;
    auto s2 = Array<Copy, 1>::with(Copy());
    ::sus::clone_into(s2, s);
    EXPECT_EQ(s2[0u].i, 1000);
  }

  struct Clone {
    Clone() {}

    Clone clone() const {
      auto c = Clone();
      c.i = i + 1_i32;
      return c;
    }

    void clone_from(const Clone& o) & noexcept { i = o.i + 200_i32; }

    Clone(Clone&&) = default;
    Clone& operator=(Clone&&) = default;

    i32 i = 1_i32;
  };

  static_assert(sus::mem::__private::HasCloneFromMethod<Clone>);

  static_assert(!::sus::mem::Copy<Clone>);
  static_assert(::sus::mem::Clone<Clone>);
  static_assert(::sus::mem::CloneInto<Clone>);
  static_assert(::sus::mem::Move<Clone>);
  static_assert(!::sus::mem::Copy<Array<Clone, 1>>);
  static_assert(::sus::mem::Clone<Array<Clone, 1>>);
  static_assert(::sus::mem::CloneInto<Array<Clone, 1>>);
  static_assert(::sus::mem::Move<Array<Clone, 1>>);

  {
    const auto s = Array<Clone, 1>::with(Clone());
    i32 i = s[0u].i;
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Array<Clone, 1>>);
    EXPECT_GT(s2[0u].i, i);
  }

  {
    auto s = Array<Clone, 1>::with(Clone());
    s[0u].i = 1000_i32;
    auto s2 = Array<Clone, 1>::with(Clone());
    ::sus::clone_into(s2, s);
    EXPECT_EQ(s2[0u].i.primitive_value, (1200_i32).primitive_value);
  }
}

TEST(Array, StructuredBinding) {
  auto a3 = Array<i32, 3>::with(1, 2, 3);
  auto& [a, b, c] = a3;
  static_assert(std::same_as<decltype(a), i32>);
  static_assert(std::same_as<decltype(b), i32>);
  static_assert(std::same_as<decltype(c), i32>);

  a3.get_mut(0u).unwrap() += 1;
  a3.get_mut(1u).unwrap() += 2;
  a3.get_mut(2u).unwrap() += 3;
  EXPECT_EQ(a3, (Array<i32, 3>::with(2, 4, 6)));

  const auto& [d, e, f] = a3;
  static_assert(std::same_as<decltype(d), const i32>);
  static_assert(std::same_as<decltype(e), const i32>);
  static_assert(std::same_as<decltype(f), const i32>);
  EXPECT_EQ((Array<i32, 3>::with(d, e, f)), (Array<i32, 3>::with(2, 4, 6)));

  auto [g, h, i] = sus::move(a3);
  static_assert(std::same_as<decltype(g), i32>);
  static_assert(std::same_as<decltype(h), i32>);
  static_assert(std::same_as<decltype(i), i32>);
  EXPECT_EQ((Array<i32, 3>::with(g, h, i)), (Array<i32, 3>::with(2, 4, 6)));
}

TEST(Array, fmt) {
  auto a = Array<i32, 5>::with(1, 2, 3, 4, 5);
  EXPECT_EQ(fmt::format("{}", a), "[1, 2, 3, 4, 5]");
  EXPECT_EQ(fmt::format("{:02}", a), "[01, 02, 03, 04, 05]");

  EXPECT_EQ(fmt::format("{}", Array<i32, 0>()), "[]");
  EXPECT_EQ(fmt::format("{:02}", Array<i32, 0>()), "[]");

  struct NoFormat {
    i32 a = 0x16ae3cf2;
  };
  static_assert(!fmt::is_formattable<NoFormat, char>::value);

  auto an = Array<NoFormat, 2>::with(NoFormat(), NoFormat(0xf00d));
  EXPECT_EQ(fmt::format("{}", an), "[f2-3c-ae-16, 0d-f0-00-00]");
}

TEST(Array, Stream) {
  std::stringstream s;
  s << Array<i32, 5>::with(1, 2, 3, 4, 5);
  EXPECT_EQ(s.str(), "[1, 2, 3, 4, 5]");
}

TEST(Array, GTest) {
  EXPECT_EQ(testing::PrintToString(Array<i32, 5>::with(1, 2, 3, 4, 5)),
            "[1, 2, 3, 4, 5]");
}

TEST(ArrayDeathTest, IteratorInvalidation) {
#if GTEST_HAS_DEATH_TEST
  auto v = sus::Array<i32, 2>::with(1, 2);
  auto it = v.iter();
  it.next();
  EXPECT_DEATH(
      {
        auto v2 = sus::move(v);
        ensure_use(&v2);
      },
      "");
#endif
}
}  // namespace
