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

#include "containers/slice.h"

#include "construct/into.h"
#include "googletest/include/gtest/gtest.h"
#include "iter/iterator.h"
#include "mem/clone.h"
#include "mem/copy.h"
#include "mem/move.h"
#include "num/types.h"
#include "prelude.h"

using sus::containers::Slice;

namespace {

static_assert(sus::mem::Copy<Slice<i32>>);
static_assert(sus::mem::Clone<Slice<i32>>);
static_assert(sus::mem::Move<Slice<i32>>);

TEST(Slice, FromRawParts) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);
}

TEST(Slice, Get) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from_raw_parts(a, 3_usize);
  EXPECT_EQ(s.get_ref(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(s.get_ref(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(s.get_ref(3_usize), sus::None);

  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);
  EXPECT_EQ(sm.get_ref(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(sm.get_ref(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(sm.get_ref(3_usize), sus::None);
}

template <class T, class U>
concept HasGetMut = requires(T t, U u) { t.get_mut(u); };

// get_mut() is only available for mutable slices of mutable types.
static_assert(!HasGetMut<Slice<const i32>, usize>);
static_assert(HasGetMut<Slice<i32>, usize>);
static_assert(!HasGetMut<const Slice<const i32>, usize>);
static_assert(!HasGetMut<const Slice<i32>, usize>);

TEST(Slice, GetMut) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);

  EXPECT_EQ(sm.get_mut(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(sm.get_mut(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(sm.get_mut(3_usize), sus::None);
}

TEST(Slice, GetUnchecked) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from_raw_parts(a, 3_usize);
  EXPECT_EQ(s.get_unchecked(unsafe_fn, 1_usize), 2_i32);
  EXPECT_EQ(s.get_unchecked(unsafe_fn, 2_usize), 3_i32);

  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);
  EXPECT_EQ(sm.get_unchecked(unsafe_fn, 1_usize), 2_i32);
  EXPECT_EQ(sm.get_unchecked(unsafe_fn, 2_usize), 3_i32);
}

template <class T, class U>
concept HasGetUncheckedMut =
    requires(T t, U u) { t.get_unchecked_mut(unsafe_fn, u); };

// get_unchecked_mut() is only available for mutable slices of mutable types.
static_assert(!HasGetUncheckedMut<Slice<const i32>, usize>);
static_assert(HasGetUncheckedMut<Slice<i32>, usize>);
static_assert(!HasGetUncheckedMut<const Slice<const i32>, usize>);
static_assert(!HasGetUncheckedMut<const Slice<i32>, usize>);

TEST(Slice, GetMutUnchecked) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);

  EXPECT_EQ(sm.get_unchecked_mut(unsafe_fn, 1_usize), 2_i32);
  EXPECT_EQ(sm.get_unchecked_mut(unsafe_fn, 2_usize), 3_i32);
}

TEST(Slice, Into) {
  i32 a[] = {1, 2, 3};
  Slice<const i32> s = sus::array_into(a);
  EXPECT_EQ(s.len(), 3u);
  Slice<i32> sm = sus::array_into(a);
  EXPECT_EQ(sm.len(), 3u);
}

TEST(Slice, From) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from(a);
  auto sm = Slice<i32>::from(a);
}

TEST(Slice, Index) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);

  EXPECT_EQ(sc[0_usize], 1_i32);
  EXPECT_EQ(sc[2_usize], 3_i32);
  EXPECT_EQ(sm[0_usize], 1_i32);
  EXPECT_EQ(sm[2_usize], 3_i32);
}

TEST(SliceDeathTest, Index) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(sc[3_usize], "");
  EXPECT_DEATH(sm[3_usize], "");
#endif
}

TEST(Slice, RangedForIter) {
  {
    usize ar[] = {1u, 2u, 3u};
    const auto slice = Slice<const usize>::from(ar);
    auto sum = 0_usize;
    for (const usize& i : slice) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    const auto mslice = Slice<usize>::from(ar);
    auto sum = 0_usize;
    for (const usize& i : mslice) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
}

TEST(Slice, Iter) {
  {
    usize ar[] = {1u, 2u, 3u};
    const auto slice = Slice<const usize>::from(ar);
    auto sum = 0_usize;
    for (const usize& i : slice.iter()) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    const auto mslice = Slice<usize>::from(ar);
    auto sum = 0_usize;
    for (const usize& i : mslice.iter()) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
}

TEST(Slice, IterMut) {
  usize ar[] = {1u, 2u, 3u};
  auto slice = Slice<usize>::from(ar);
  auto sum = 0_usize;
  for (usize& i : slice.iter_mut()) {
    sum += i;
    i += 1_usize;
  }
  EXPECT_EQ(sum, 6_usize);

  sum = 0_usize;
  for (const usize& i : slice.iter()) {
    sum += i;
  }
  EXPECT_EQ(sum, 9_usize);
}

TEST(Slice, IntoIter) {
  {
    const usize ar[] = {1u, 2u, 3u};
    auto slice = Slice<const usize>::from(ar);
    auto sum = 0_usize;
    for (const usize& i : sus::move(slice).into_iter()) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    auto slice = Slice<usize>::from(ar);
    auto sum = 0_usize;
    for (usize& i : sus::move(slice).into_iter()) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
}

TEST(Slice, ImplicitIter) {
  const usize ar[] = {1u, 2u, 3u};
  auto slice = Slice<const usize>::from(ar);
  auto sum = 0_usize;
  for (const usize& i : slice) {
    sum += i;
  }
  EXPECT_EQ(sum, 6_usize);
}

}  // namespace
