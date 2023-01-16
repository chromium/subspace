#include <stdio.h>
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

#include "googletest/include/gtest/gtest.h"
#include "subspace/containers/vec.h"
#include "subspace/iter/iterator.h"
#include "subspace/mem/move.h"
#include "subspace/prelude.h"

namespace {

using sus::containers::Vec;

TEST(Vec, Default) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.capacity(), 0_usize);
  EXPECT_EQ(v.len(), 0_usize);
}

TEST(Vec, IsEmpty) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.is_empty(), true);
  v.push(1);
  EXPECT_EQ(v.is_empty(), false);
}

TEST(Vec, WithCapacity) {
  {
    auto v0 = Vec<i32>::with_capacity(0_usize);
    EXPECT_EQ(v0.capacity(), 0_usize);
    EXPECT_EQ(v0.len(), 0_usize);
  }
  {
    auto v1 = Vec<i32>::with_capacity(1_usize);
    EXPECT_EQ(v1.capacity(), 1_usize);
    EXPECT_EQ(v1.len(), 0_usize);
  }
  {
    auto v2 = Vec<i32>::with_capacity(2_usize);
    EXPECT_EQ(v2.capacity(), 2_usize);
    EXPECT_EQ(v2.len(), 0_usize);
  }
  {
    auto v3 = Vec<i32>::with_capacity(3_usize);
    EXPECT_EQ(v3.capacity(), 3_usize);
    EXPECT_EQ(v3.len(), 0_usize);
  }
}

TEST(Vec, ConstructorFunction) {
  {
    // All parameters match the vec type.
    Vec<u32> a = sus::vec(1_u32, 2_u32, 3_u32);
    EXPECT_EQ(a.len(), 3_usize);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // Some parameters convert to u32.
    Vec<u32> a = sus::vec(1_u32, 2u, 3_u32);
    EXPECT_EQ(a.len(), 3_usize);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // All parameters convert to u32.
    Vec<u32> a = sus::vec(1u, 2u, 3u);
    EXPECT_EQ(a.len(), 3_usize);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // into() as an input to the vec.
    Vec<u32> a = sus::vec(1_u32, sus::into(2), 3_u32);
    EXPECT_EQ(a.len(), 3_usize);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // Copies the lvalue and const lvalue.
    auto i = 1_u32;
    const auto j = 2_u32;
    Vec<u32> a = sus::vec(i, j, 3_u32);
    EXPECT_EQ(a.len(), 3_usize);
    EXPECT_EQ(a[0u], 1_u32);
    EXPECT_EQ(a[1u], 2_u32);
    EXPECT_EQ(a[2u], 3_u32);
  }
  {
    // Copies the rvalue reference.
    auto i = 1_u32;
    Vec<u32> a = sus::vec(sus::move(i), 2_u32, 3_u32);
    EXPECT_EQ(a.len(), 3_usize);
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
    auto marker = sus::vec(s);
    EXPECT_EQ(copies, 0);
    Vec<S> vec = sus::move(marker);
    EXPECT_GE(copies, 1);
  }
}

TEST(Vec, Push) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.capacity(), 0_usize);
  EXPECT_EQ(v.len(), 0_usize);
  v.push(2_i32);
  EXPECT_GT(v.capacity(), 0_usize);
  EXPECT_EQ(v.len(), 1_usize);
}

TEST(Vec, Pop) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.pop(), sus::None);
  v.push(2_i32);
  EXPECT_EQ(v.pop().unwrap(), 2_i32);
  EXPECT_EQ(v.pop(), sus::None);
  EXPECT_GT(v.capacity(), 0_usize);
  EXPECT_EQ(v.len(), 0_usize);
}

TEST(Vec, Get) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.get_ref(0u), sus::None);
  v.push(2_i32);
  EXPECT_EQ(v.get_ref(0u).unwrap(), 2_i32);
  EXPECT_EQ(v.get_ref(1u), sus::None);
}

TEST(Vec, GetMut) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.get_mut(0u), sus::None);
  v.push(2_i32);
  // get_mut() gives a mutable reference into the vector.
  v.get_mut(0u).unwrap() += 1_i32;
  EXPECT_EQ(v.get_mut(0u).unwrap(), 3_i32);
  EXPECT_EQ(v.get_mut(1u), sus::None);
}

TEST(Vec, GetUnchecked) {
  auto v = Vec<i32>();
  v.push(2_i32);
  EXPECT_EQ(v.get_unchecked(unsafe_fn, 0u), 2_i32);
}

TEST(Vec, GetUncheckedMut) {
  auto v = Vec<i32>();
  v.push(2_i32);
  // get_unchecked_mut() gives a mutable reference into the vector.
  v.get_unchecked_mut(unsafe_fn, 0u) += 1_i32;
  EXPECT_EQ(v.get_unchecked_mut(unsafe_fn, 0u), 3_i32);
}

TEST(Vec, OperatorIndex) {
  auto v = Vec<i32>();
  v.push(2_i32);
  const auto& r = v;
  EXPECT_EQ(r[0u], 2_i32);
}

TEST(Vec, OperatorIndexMut) {
  auto v = Vec<i32>();
  v.push(2_i32);
  // operator[] gives a mutable reference into the vector.
  v[0u] += 1_i32;
  EXPECT_EQ(v[0u], 3_i32);
}

TEST(VecDeathTest, AsPtr) {
  auto v = Vec<i32>();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(v.as_ptr(), "");
#endif
}

TEST(VecDeathTest, AsMutPtr) {
  auto v = Vec<i32>();
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(v.as_mut_ptr(), "");
#endif
}

TEST(Vec, AsPtr) {
  auto v = Vec<i32>();
  v.push(2_i32);
  EXPECT_EQ(v.as_ptr(), &v[0u]);
  static_assert(std::same_as<const i32*, decltype(v.as_ptr())>);
}

TEST(Vec, AsMutPtr) {
  auto v = Vec<i32>();
  v.push(2_i32);
  EXPECT_EQ(v.as_mut_ptr(), &v[0u]);
  static_assert(std::same_as<i32*, decltype(v.as_mut_ptr())>);
}

TEST(Vec, AsRef) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.as_ref().len(), 0_usize);
  v.push(2_i32);
  auto s = v.as_ref();
  static_assert(std::same_as<decltype(s), sus::Slice<const i32>>);
  EXPECT_EQ(s.len(), 1_usize);
  EXPECT_EQ(&s[0u], &v[0u]);
}

TEST(Vec, AsMut) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.as_mut().len(), 0_usize);
  v.push(2_i32);
  auto s = v.as_mut();
  static_assert(std::same_as<decltype(s), sus::Slice<i32>>);
  EXPECT_EQ(s.len(), 1_usize);
  EXPECT_EQ(&s[0u], &v[0u]);
}

TEST(Vec, RangedForIter) {
  auto v = Vec<i32>();
  v.push(1_i32);
  v.push(2_i32);
  v.push(3_i32);

  auto sum = 0_i32;
  for (const i32& i : v) sum += i;
  EXPECT_EQ(sum, 6);
}

TEST(Vec, Iter) {
  auto v = Vec<i32>();
  v.push(1_i32);
  v.push(2_i32);
  v.push(3_i32);

  auto sum = 0_i32;
  for (const i32& i : v.iter()) sum += i;
  EXPECT_EQ(sum, 6);

  auto e = Vec<i32>();
  for (const i32& i : e.iter()) sum += i;
  EXPECT_EQ(sum, 6);
}

TEST(Vec, IterMut) {
  auto v = Vec<i32>();
  v.push(1_i32);
  v.push(2_i32);
  v.push(3_i32);

  auto sum = 0_i32;
  for (i32& i : v.iter_mut()) {
    sum += i;
    i += 1_i32;
  }
  EXPECT_EQ(v[0u], 2_i32);
  EXPECT_EQ(v[1u], 3_i32);
  EXPECT_EQ(v[2u], 4_i32);
  EXPECT_EQ(sum, 6);

  auto e = Vec<i32>();
  for (const i32& i : e.iter_mut()) sum += i;
  EXPECT_EQ(sum, 6);
}

TEST(Vec, IntoIter) {
  auto v = Vec<i32>();
  v.push(1_i32);
  v.push(2_i32);
  v.push(3_i32);

  auto sum = 0_i32;
  for (i32 i : sus::move(v).into_iter()) sum += i;
  EXPECT_EQ(sum, 6);
}

TEST(Vec, Growth) {
  auto v = Vec<i32>::with_capacity(2_usize);
  EXPECT_EQ(v.capacity(), 2_usize);
  while (v.capacity() == 2_usize) v.push(1_i32);
  // we grew capacity when we pushed the first item past existing capacity.
  EXPECT_EQ(v.len(), 3_usize);
  // The current growth formula multiplies by 3 after adding 1.
  EXPECT_EQ(v.capacity(), (2_usize + 1_usize) * 3_usize);
}

template <bool trivial>
struct TrivialLies {
  TrivialLies(usize& moves, usize& destructs)
      : moves(moves), destructs(destructs) {}
  TrivialLies(TrivialLies&& o)
      : moves(o.moves), destructs(o.destructs), i(o.i + 1_i32) {
    moves += 1_usize;
  }
  void operator=(TrivialLies&&) { sus::check(false); }
  ~TrivialLies() { destructs += 1_usize; }

  usize& moves;
  usize& destructs;
  i32 i = 0_i32;

  // The move constructor and destructor mean this type is NOT trivially
  // relocatable.. but we can lie here to see that the move constuctor and
  // destructor are elided when the type says it's trivially relocatable.
  sus_class_trivially_relocatable_if(unsafe_fn, trivial);
};

TEST(Vec, GrowthTriviallyRelocatable) {
  static auto moves = 0_usize;
  static auto destructs = 0_usize;
  auto v = Vec<TrivialLies<true>>::with_capacity(1_usize);
  v.push(TrivialLies<true>(moves, destructs));

  moves = destructs = 0_usize;
  v.reserve(2_usize);
  // TrivialLies was memcpy'd, instead of being moved and destroyed.
  EXPECT_EQ(moves, 0_usize);
  EXPECT_EQ(destructs, 0_usize);
}

TEST(Vec, GrowthNonTriviallyRelocatable) {
  static auto moves = 0_usize;
  static auto destructs = 0_usize;
  auto v = Vec<TrivialLies<false>>::with_capacity(1_usize);
  v.push(TrivialLies<false>(moves, destructs));
  v[0u].i = 42_i32;

  moves = destructs = 0_usize;
  v.reserve(2_usize);
  // TrivialLies was moved and destroyed, not just memcpy'd.
  EXPECT_EQ(moves, 1_usize);
  EXPECT_EQ(destructs, 1_usize);

  // Moving S incremented it.
  EXPECT_EQ(v[0u].i, 43_i32);
}

TEST(Vec, Reserve) {
  {
    auto v = Vec<i32>::with_capacity(2_usize);
    EXPECT_EQ(v.capacity(), 2_usize);
    v.reserve(1_usize);  // We already have room, so do nothing.
    v.reserve(1_usize);  // We already have room, so do nothing.
    v.reserve(1_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.reserve(2_usize);  // We already have room, so do nothing.
    v.reserve(2_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.reserve(3_usize);  // We need more space, so grow.
    EXPECT_GT(v.capacity(), 2_usize);
    // We didn't reserve exact, so we grew by something more than one.
    EXPECT_GT(v.capacity(), 3_usize);
  }
  {
    // Reserve considers the length of the vector.
    auto v = Vec<i32>::with_capacity(2_usize);
    v.push(1_i32);
    v.reserve(1_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.reserve(2_usize);  // We need more space, so grow.
    EXPECT_GT(v.capacity(), 2_usize);
    // We didn't reserve exact, so we grew by something more than one.
    EXPECT_GT(v.capacity(), 3_usize);
  }
}

TEST(Vec, ReserveExact) {
  {
    auto v = Vec<i32>::with_capacity(2_usize);
    EXPECT_EQ(v.capacity(), 2_usize);
    v.reserve_exact(1_usize);  // We already have room, so do nothing.
    v.reserve_exact(1_usize);  // We already have room, so do nothing.
    v.reserve_exact(1_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.reserve_exact(2_usize);  // We already have room, so do nothing.
    v.reserve_exact(2_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.reserve_exact(3_usize);  // We need more space, so grow.
    EXPECT_GT(v.capacity(), 2_usize);
    // We reserved an exact amount, so we grew by only one.
    EXPECT_EQ(v.capacity(), 3_usize);
  }
  {
    // Reserve considers the length of the vector.
    auto v = Vec<i32>::with_capacity(2_usize);
    v.push(1_i32);
    v.reserve_exact(1_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.reserve_exact(2_usize);  // We need more space, so grow.
    EXPECT_GT(v.capacity(), 2_usize);
    // We reserved an exact amount, so we grew by only one.
    EXPECT_EQ(v.capacity(), 3_usize);
  }
}

TEST(Vec, GrowToExact) {
  {
    auto v = Vec<i32>::with_capacity(2_usize);
    EXPECT_EQ(v.capacity(), 2_usize);
    v.grow_to_exact(1_usize);  // We already have room, so do nothing.
    v.grow_to_exact(1_usize);  // We already have room, so do nothing.
    v.grow_to_exact(1_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.grow_to_exact(2_usize);  // We already have room, so do nothing.
    v.grow_to_exact(2_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.grow_to_exact(3_usize);  // We need more space, so grow.
    EXPECT_GT(v.capacity(), 2_usize);
    // We reserved an exact amount, so we grew by only one.
    EXPECT_EQ(v.capacity(), 3_usize);
  }
  {
    // GrowTo does not consider the length of the vector.
    auto v = Vec<i32>::with_capacity(2_usize);
    v.push(1_i32);
    v.grow_to_exact(1_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.grow_to_exact(2_usize);  // We already have room, so do nothing.
    EXPECT_EQ(v.capacity(), 2_usize);
    v.grow_to_exact(3_usize);  // We need more space, so grow.
    EXPECT_GT(v.capacity(), 2_usize);
    // We reserved an exact amount, so we grew by only one.
    EXPECT_EQ(v.capacity(), 3_usize);
  }
}

TEST(Vec, Collect) {
  auto v = Vec<i32>();
  v.push(1_i32);
  v.push(2_i32);
  v.push(3_i32);
  auto v2 = sus::move(v).into_iter().collect<Vec<i32>>();
  EXPECT_EQ(v2.len(), 3_usize);

  auto vc = Vec<i32>();
  vc.push(1_i32);
  vc.push(2_i32);
  vc.push(3_i32);
  // TODO: This won't work because we have const refs to i32 which can't be
  // moved into vc2. We need to call `iter().cloned().collect<Vec<i32>>` when
  // cloned() exists.
  //
  // auto vc2 = v.iter().collect<Vec<i32>>();
  // EXPECT_EQ(vc2.len(), 3_usize);
}

TEST(Vec, Destroy) {
  static auto moves = 0_usize;
  static auto destructs = 0_usize;
  auto o = sus::Option<Vec<TrivialLies<false>>>::none();
  o.insert(Vec<TrivialLies<false>>::with_capacity(1_usize));
  o->push(TrivialLies<false>(moves, destructs));
  o->push(TrivialLies<false>(moves, destructs));

  moves = destructs = 0_usize;
  o.take();  // Destroys the Vec, and both objects inside it.
  EXPECT_EQ(destructs, 2_usize);
}

TEST(Vec, Clear) {
  static auto moves = 0_usize;
  static auto destructs = 0_usize;
  auto v = Vec<TrivialLies<false>>::with_capacity(1_usize);
  v.push(TrivialLies<false>(moves, destructs));
  v.push(TrivialLies<false>(moves, destructs));

  moves = destructs = 0_usize;
  EXPECT_EQ(v.len(), 2_usize);
  EXPECT_GE(v.capacity(), 2_usize);
  auto cap_before = v.capacity();
  v.clear();  // Clears the Vec, destroying both objects inside it.
  EXPECT_EQ(destructs, 2_usize);
  EXPECT_EQ(v.len(), 0_usize);
  EXPECT_EQ(v.capacity(), cap_before);
}

TEST(Vec, Move) {
  struct Move {
    Move(Move&&) = default;
    Move& operator=(Move&&) = default;
  };

  static_assert(!::sus::mem::Copy<Move>);
  static_assert(!::sus::mem::Clone<Move>);
  static_assert(!::sus::mem::CloneInto<Move>);
  static_assert(::sus::mem::Move<Move>);
  // Vec is Move but not Copy or Clone if T is not.
  static_assert(!::sus::mem::Copy<Vec<Move>>);
  static_assert(!::sus::mem::Clone<Vec<Move>>);
  static_assert(!::sus::mem::CloneInto<Vec<Move>>);
  static_assert(::sus::mem::Move<Vec<Move>>);

  static auto moves = 0_usize;
  static auto destructs = 0_usize;
  auto v = Vec<TrivialLies<false>>::with_capacity(1_usize);
  v.push(TrivialLies<false>(moves, destructs));
  v.push(TrivialLies<false>(moves, destructs));

  auto v2 = Vec<TrivialLies<false>>::with_capacity(1_usize);
  v2.push(TrivialLies<false>(moves, destructs));
  v2.push(TrivialLies<false>(moves, destructs));

  moves = destructs = 0_usize;
  v = sus::move(v2);  // Destroys the objects in `v`.
  EXPECT_EQ(moves, 0_usize);
  EXPECT_EQ(destructs, 2_usize);
}

TEST(Vec, Clone) {
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
  // Vec is always Clone (if T is Clone), but never Copy since it's expensive
  // to copy.
  static_assert(!::sus::mem::Copy<Vec<Copy>>);
  static_assert(::sus::mem::Clone<Vec<Copy>>);
  static_assert(::sus::mem::CloneInto<Vec<Copy>>);
  static_assert(::sus::mem::Move<Vec<Copy>>);

  {
    auto s = Vec<Copy>();
    s.push(Copy());
    i32 i = s[0u].i;
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Vec<Copy>>);
    EXPECT_EQ(s2.capacity(), s.capacity());
    EXPECT_EQ(s2.len(), s.len());
    EXPECT_GT(s2[0u].i, i);
  }

  {
    auto s = Vec<Copy>();
    s.push(Copy());
    i32 i = s[0u].i;
    auto s2 = Vec<Copy>();
    sus::clone_into(mref(s2), s);
    EXPECT_EQ(s2.capacity(), s.capacity());
    EXPECT_EQ(s2.len(), s.len());
    EXPECT_GT(s2[0u].i, i);
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

  static_assert(!::sus::mem::Copy<Clone>);
  static_assert(::sus::mem::Clone<Clone>);
  static_assert(!::sus::mem::CloneInto<Clone>);
  static_assert(::sus::mem::Move<Clone>);
  static_assert(!::sus::mem::Copy<Vec<Clone>>);
  static_assert(::sus::mem::Clone<Vec<Clone>>);
  static_assert(::sus::mem::CloneInto<Vec<Clone>>);
  static_assert(::sus::mem::Move<Vec<Clone>>);

  {
    auto s = Vec<Clone>();
    s.push(Clone());
    i32 i = s[0u].i;
    auto s2 = sus::clone(s);
    static_assert(std::same_as<decltype(s2), Vec<Clone>>);
    EXPECT_EQ(s2.capacity(), s.capacity());
    EXPECT_EQ(s2.len(), s.len());
    EXPECT_GT(s2[0u].i, i);
  }
}

TEST(Vec, CloneInto) {
  static auto count = 0_usize;
  struct S {
    S() { count += 1u; }
    S(const S&) { count += 1u; }
    ~S() { count -= 1u; }
  };

  auto v1 = Vec<S>();
  auto v2 = Vec<S>();

  // Case 1: Clone from larger vector.
  v1.push(S());
  v2.push(S());
  v2.push(S());
  ::sus::clone_into(mref(v1), v2);
  v1.clear();
  v2.clear();
  EXPECT_EQ(count, 0_usize);
  EXPECT_EQ(v1.len(), v2.len());
  EXPECT_EQ(v1.capacity(), v2.capacity());

  // Case 2: Clone from same size vector.
  v1.push(S());
  v2.push(S());
  ::sus::clone_into(mref(v1), v2);
  v1.clear();
  v2.clear();
  EXPECT_EQ(count, 0_usize);
  EXPECT_EQ(v1.len(), v2.len());
  EXPECT_EQ(v1.capacity(), v2.capacity());

  // Case 3: Clone from smaller vector.
  v1.push(S());
  v1.push(S());
  v2.push(S());
  ::sus::clone_into(mref(v1), v2);
  v1.clear();
  v2.clear();
  EXPECT_EQ(count, 0_usize);
  EXPECT_EQ(v1.len(), v2.len());
  EXPECT_EQ(v1.capacity(), v2.capacity());
}

struct Sortable {
  Sortable(i32 value, i32 unique) : value(value), unique(unique) {}

  i32 value;
  i32 unique;

  friend bool operator==(const Sortable& a, const Sortable& b) noexcept {
    return a.value == b.value && a.unique == b.unique;
  }
  friend auto operator<=>(const Sortable& a, const Sortable& b) noexcept {
    return a.value <=> b.value;
  }
};

TEST(Vec, Sort) {
  // clang-format off
  sus::Vec<Sortable> unsorted = sus::vec(
    Sortable(3, 0),
    Sortable(3, 1),
    Sortable(4, 0),
    Sortable(2, 0),
    Sortable(2, 1),
    Sortable(1, 0),
    Sortable(3, 2),
    Sortable(6, 0),
    Sortable(5, 0)
  );
  sus::Vec<Sortable> sorted = sus::vec(
    Sortable(1, 0),
    Sortable(2, 0),
    Sortable(2, 1),
    Sortable(3, 0),
    Sortable(3, 1),
    Sortable(3, 2),
    Sortable(4, 0),
    Sortable(5, 0),
    Sortable(6, 0)
  );
  // clang-format on

  unsorted.sort();
  for (usize i = 0u; i < unsorted.len(); i += 1u) {
    EXPECT_EQ(sorted[i], unsorted[i]);
  }
}

TEST(Vec, SortBy) {
  // clang-format off
  sus::Vec<Sortable> unsorted = sus::vec(
    Sortable(3, 0),
    Sortable(3, 1),
    Sortable(4, 0),
    Sortable(2, 0),
    Sortable(2, 1),
    Sortable(1, 0),
    Sortable(3, 2),
    Sortable(6, 0),
    Sortable(5, 0)
  );
  sus::Vec<Sortable> sorted = sus::vec(
    Sortable(6, 0),
    Sortable(5, 0),
    Sortable(4, 0),
    Sortable(3, 0),
    Sortable(3, 1),
    Sortable(3, 2),
    Sortable(2, 0),
    Sortable(2, 1),
    Sortable(1, 0)
  );
  // clang-format on

  // Sorts backward.
  unsorted.sort_by([](const auto& a, const auto& b) { return b <=> a; });
  for (usize i = 0u; i < unsorted.len(); i += 1u) {
    EXPECT_EQ(sorted[i], unsorted[i]);
  }
}

TEST(Vec, SortUnstable) {
  sus::Vec<i32> unsorted = sus::vec(3, 4, 2, 1, 6, 5);
  sus::Vec<i32> sorted = sus::vec(1, 2, 3, 4, 5, 6);

  unsorted.sort_unstable();
  for (usize i = 0u; i < unsorted.len(); i += 1u) {
    EXPECT_EQ(sorted[i], unsorted[i]);
  }
}

TEST(Vec, SortUnstableBy) {
  sus::Vec<i32> unsorted = sus::vec(3, 4, 2, 1, 6, 5);
  sus::Vec<i32> sorted = sus::vec(6, 5, 4, 3, 2, 1);

  // Sorts backward.
  unsorted.sort_unstable_by(
      [](const auto& a, const auto& b) { return b <=> a; });
  for (usize i = 0u; i < unsorted.len(); i += 1u) {
    EXPECT_EQ(sorted[i], unsorted[i]);
  }
}
}  // namespace
