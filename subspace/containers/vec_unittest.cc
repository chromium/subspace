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

using sus::containers::Slice;
using sus::containers::SliceMut;
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
    EXPECT_GE(v1.capacity(), 1_usize);
    EXPECT_EQ(v1.len(), 0_usize);
  }
  {
    auto v2 = Vec<i32>::with_capacity(2_usize);
    EXPECT_GE(v2.capacity(), 2_usize);
    EXPECT_EQ(v2.len(), 0_usize);
  }
  {
    auto v3 = Vec<i32>::with_capacity(1025_usize);
    EXPECT_GE(v3.capacity(), 1025_usize);
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

  // In place explicit construction.
  {
    auto a = sus::vec(1_i32, 2_i32).construct();
    static_assert(std::same_as<decltype(a), Vec<i32>>);
    EXPECT_EQ(a[0u], 1_i32);
    EXPECT_EQ(a[1u], 2_i32);

    auto b = sus::vec(1, 2).construct<i32>();
    static_assert(std::same_as<decltype(b), Vec<i32>>);
    EXPECT_EQ(b[0u], 1_i32);
    EXPECT_EQ(b[1u], 2_i32);
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
  EXPECT_EQ(v.get(0u), sus::None);
  v.push(2_i32);
  EXPECT_EQ(v.get(0u).unwrap(), 2_i32);
  EXPECT_EQ(v.get(1u), sus::None);
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

TEST(Vec, AsSlice) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.as_slice().len(), 0_usize);
  v.push(2_i32);
  auto s = v.as_slice();
  static_assert(std::same_as<decltype(s), sus::Slice<i32>>);
  EXPECT_EQ(s.len(), 1_usize);
  EXPECT_EQ(&s[0u], &v[0u]);
}

TEST(Vec, AsMutSlice) {
  auto v = Vec<i32>();
  EXPECT_EQ(v.as_mut_slice().len(), 0_usize);
  v.push(2_i32);
  auto s = v.as_mut_slice();
  static_assert(std::same_as<decltype(s), sus::SliceMut<i32>>);
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

TEST(Vec, IntoIterDoubleEnded) {
  auto v = Vec<i32>();
  v.push(1_i32);
  v.push(2_i32);
  v.push(3_i32);

  auto it = sus::move(v).into_iter();
  static_assert(sus::iter::DoubleEndedIterator<decltype(it), i32>);
  EXPECT_EQ(it.next_back(), sus::some(3_i32).construct());
  EXPECT_EQ(it.next_back(), sus::some(2_i32).construct());
  EXPECT_EQ(it.next_back(), sus::some(1_i32).construct());
  EXPECT_EQ(it.next_back(), sus::None);
}

TEST(Vec, Growth) {
  auto v = Vec<i32>();
  v.reserve_exact(2_usize);
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
  auto v = Vec<TrivialLies<true>>();
  v.reserve_exact(1_usize);
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
  auto v = Vec<TrivialLies<false>>();
  v.reserve_exact(1_usize);
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
    auto v = Vec<i32>();
    v.reserve_exact(2_usize);
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
    auto v = Vec<i32>();
    v.reserve_exact(2_usize);
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
    auto v = Vec<i32>();
    v.reserve_exact(2_usize);
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
    auto v = Vec<i32>();
    v.reserve_exact(2_usize);
    EXPECT_EQ(v.capacity(), 2_usize);
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
    auto v = Vec<i32>();
    v.reserve_exact(2_usize);
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
    auto v = Vec<i32>();
    v.reserve_exact(2_usize);
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
  EXPECT_EQ(v2.capacity(), 3_usize);
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

TEST(Vec, SizeHint) {
  auto v = Vec<i32>();
  v.push(1_i32);
  v.push(2_i32);
  v.push(3_i32);
  auto it = sus::move(v).into_iter();
  {
    auto [lower, upper] = it.size_hint();
    EXPECT_EQ(lower, 3_usize);
    EXPECT_EQ(upper, sus::Option<usize>::some(3_usize));
  }
  EXPECT_EQ(it.next(), sus::Some);
  {
    auto [lower, upper] = it.size_hint();
    EXPECT_EQ(lower, 2_usize);
    EXPECT_EQ(upper, sus::Option<usize>::some(2_usize));
  }
  EXPECT_EQ(it.next(), sus::Some);
  {
    auto [lower, upper] = it.size_hint();
    EXPECT_EQ(lower, 1_usize);
    EXPECT_EQ(upper, sus::Option<usize>::some(1_usize));
  }
  EXPECT_EQ(it.next(), sus::Some);
  {
    auto [lower, upper] = it.size_hint();
    EXPECT_EQ(lower, 0_usize);
    EXPECT_EQ(upper, sus::Option<usize>::some(0_usize));
  }
  EXPECT_EQ(it.next(), sus::None);
  {
    auto [lower, upper] = it.size_hint();
    EXPECT_EQ(lower, 0_usize);
    EXPECT_EQ(upper, sus::Option<usize>::some(0_usize));
  }
}

TEST(Vec, ExactSizeIterator) {
  auto v = Vec<i32>();
  v.push(1_i32);
  v.push(2_i32);
  v.push(3_i32);
  auto it = sus::move(v).into_iter();
  static_assert(sus::iter::ExactSizeIterator<decltype(it), i32>);
  EXPECT_EQ(it.exact_size_hint(), 3_usize);
  EXPECT_EQ(it.next(), sus::Some);
  EXPECT_EQ(it.exact_size_hint(), 2_usize);
  EXPECT_EQ(it.next(), sus::Some);
  EXPECT_EQ(it.exact_size_hint(), 1_usize);
  EXPECT_EQ(it.next(), sus::Some);
  EXPECT_EQ(it.exact_size_hint(), 0_usize);
  EXPECT_EQ(it.next(), sus::None);
  EXPECT_EQ(it.exact_size_hint(), 0_usize);
}

TEST(Vec, Destroy) {
  static auto moves = 0_usize;
  static auto destructs = 0_usize;
  auto o = sus::Option<Vec<TrivialLies<false>>>::none();
  o.insert(Vec<TrivialLies<false>>());
  o->push(TrivialLies<false>(moves, destructs));
  o->push(TrivialLies<false>(moves, destructs));

  moves = destructs = 0_usize;
  o.take();  // Destroys the Vec, and both objects inside it.
  EXPECT_EQ(destructs, 2_usize);
}

TEST(Vec, Clear) {
  static auto moves = 0_usize;
  static auto destructs = 0_usize;
  auto v = Vec<TrivialLies<false>>();
  v.reserve_exact(2_usize);
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
  auto v = Vec<TrivialLies<false>>();
  v.reserve_exact(1_usize);
  v.push(TrivialLies<false>(moves, destructs));
  v.push(TrivialLies<false>(moves, destructs));

  auto v2 = Vec<TrivialLies<false>>();
  v2.reserve_exact(1_usize);
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

TEST(Vec, RawParts) {
  auto v = Vec<i32>();
  v.reserve_exact(12_usize);
  v.push(1);
  v.push(2);
  v.push(3);
  const i32* v_ptr = v.as_ptr();
  auto raw = sus::move(v).into_raw_parts();
  static_assert(std::same_as<decltype(raw), sus::Tuple<i32*, usize, usize>>);
  auto [ptr, len, cap] = sus::move(raw);
  EXPECT_EQ(ptr, v_ptr);
  EXPECT_EQ(len, 3_usize);
  EXPECT_EQ(cap, 12_usize);
  auto v2 = sus::Vec<i32>::from_raw_parts(unsafe_fn, ptr, len, cap);
  EXPECT_EQ(v2.capacity(), 12_usize);
  EXPECT_EQ(v2.len(), 3_usize);
  EXPECT_EQ(v2.as_ptr(), v_ptr);
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

TEST(Vec, FromSlice) {
  sus::Vec<i32> original = sus::vec(1, 2, 3, 4);
  {
    sus::Slice<i32> s = original.as_slice();
    sus::Vec<i32> from = sus::into(s);

    EXPECT_EQ(from.len(), original.len());
    EXPECT_EQ(from[0u], 1);
    EXPECT_EQ(from[1u], 2);
    EXPECT_EQ(from[2u], 3);
    EXPECT_EQ(from[3u], 4);
  }
  {
    sus::SliceMut<i32> mut_s = original.as_mut_slice();
    sus::Vec<i32> from = sus::into(mut_s);

    EXPECT_EQ(from.len(), original.len());
    EXPECT_EQ(from[0u], 1);
    EXPECT_EQ(from[1u], 2);
    EXPECT_EQ(from[2u], 3);
    EXPECT_EQ(from[3u], 4);
  }
}

TEST(Vec, ExtendFromSlice) {
  sus::Vec<i32> v = sus::vec(1, 2, 3, 4);
  sus::Vec<i32> out;
  out.extend_from_slice(v.as_slice()["2..3"_r]);
  EXPECT_EQ(out.len(), 1u);
  EXPECT_EQ(out[0u], 3);

  out.extend_from_slice(v.as_slice());
  EXPECT_EQ(out.len(), 5u);
  EXPECT_EQ(out[0u], 3);
  EXPECT_EQ(out[1u], 1);
  EXPECT_EQ(out[2u], 2);
  EXPECT_EQ(out[3u], 3);
  EXPECT_EQ(out[4u], 4);

  out.extend_from_slice(v.as_slice()["0..0"_r]);
  EXPECT_EQ(out.len(), 5u);
}

TEST(VecDeathTest, ExtendFromSliceAliases) {
  sus::Vec<i32> v = sus::vec(1, 2, 3, 4);
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(v.extend_from_slice(v.as_slice()), "");
  EXPECT_DEATH(v.extend_from_slice(v.as_slice()["1.."_r]), "");
  EXPECT_DEATH(v.extend_from_slice(v.as_slice()["2.."_r]), "");
  EXPECT_DEATH(v.extend_from_slice(v.as_slice()["3.."_r]), "");
#endif
  // Empty does not panic.
  v.extend_from_slice(v.as_slice()["4.."_r]);
}

TEST(Vec, ConvertsToSlice) {
  Vec<i32> v = sus::vec(1, 2, 3, 4);
  const Vec<i32> cv = sus::vec(1, 2, 3, 4);
  // Explicit construction.
  {
    [[maybe_unused]] Slice<i32> s2(v);
    [[maybe_unused]] Slice<i32> s3(cv);
    [[maybe_unused]] Slice<i32> s4(sus::move(v));
    [[maybe_unused]] Slice<i32> s5(sus::clone(v));
    [[maybe_unused]] SliceMut<i32> sm2(v);
    [[maybe_unused]] SliceMut<i32> sm4(sus::move(v));
    [[maybe_unused]] SliceMut<i32> sm5(sus::clone(v));
  }
  // Implicit construction.
  {
    [[maybe_unused]] Slice<i32> s2 = v;
    [[maybe_unused]] Slice<i32> s3 = cv;
    [[maybe_unused]] SliceMut<i32> sm2 = v;
  }
  // Function calls.
  {
    [](Slice<i32>) {}(v);
    [](Slice<i32>) {}(cv);
    [](SliceMut<i32>) {}(v);
  }
  // References.
  {
    [[maybe_unused]] const Slice<i32>& s2 = v;
    [[maybe_unused]] const Slice<i32>& s3 = cv;
    [[maybe_unused]] Slice<i32>& s4 = v;
    [[maybe_unused]] Slice<i32>&& s5 = sus::move(v);
    [[maybe_unused]] const SliceMut<i32>& s6 = v;
    [[maybe_unused]] SliceMut<i32>& s8 = v;
    [[maybe_unused]] SliceMut<i32>&& s9 = sus::move(v);
  }
}

}  // namespace
