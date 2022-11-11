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
#include "iter/iterator.h"
#include "num/types.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

using sus::containers::Slice;

namespace {

TEST(Slice, FromRawParts) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);
}

TEST(Slice, Get) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from_raw_parts(a, 3_usize);
  EXPECT_EQ(s.get(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(s.get(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(s.get(3_usize), sus::None);

  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);
  EXPECT_EQ(sm.get(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(sm.get(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(sm.get(3_usize), sus::None);
}

template <class T, class U>
concept HasGetMut = requires(T t, U u) { t.get_mut(u); };

TEST(Slice, GetMut) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(a, 3_usize);

  // get_mut() is only available for slices of mutable types.
  static_assert(!HasGetMut<decltype(sc), usize>);
  static_assert(HasGetMut<decltype(sm), usize>);

  EXPECT_EQ(sm.get_mut(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(sm.get_mut(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(sm.get_mut(3_usize), sus::None);
}

TEST(Slice, Into) {
  i32 a[] = {1, 2, 3};
  Slice<const i32> s = sus::into(a);
  Slice<i32> sm = sus::into(a);
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

}  // namespace
