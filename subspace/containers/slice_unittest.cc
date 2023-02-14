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

#include "subspace/containers/slice.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/construct/into.h"
#include "subspace/containers/array.h"
#include "subspace/iter/iterator.h"
#include "subspace/mem/clone.h"
#include "subspace/mem/copy.h"
#include "subspace/mem/move.h"
#include "subspace/num/types.h"
#include "subspace/prelude.h"

using sus::containers::Slice;

namespace {

static_assert(sus::mem::Copy<Slice<i32>>);
static_assert(sus::mem::Clone<Slice<i32>>);
static_assert(sus::mem::Move<Slice<i32>>);

TEST(Slice, FromRawParts) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
}

TEST(Slice, Index) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);

  EXPECT_EQ(sc[0_usize], 1_i32);
  EXPECT_EQ(sc[2_usize], 3_i32);
  EXPECT_EQ(sm[0_usize], 1_i32);
  EXPECT_EQ(sm[2_usize], 3_i32);
}

TEST(SliceDeathTest, Index) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(sc[3_usize], "");
  EXPECT_DEATH(sm[3_usize], "");
#endif
}

TEST(Slice, Get) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(s.get(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(s.get(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(s.get(3_usize), sus::None);

  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(sm.get(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(sm.get(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(sm.get(3_usize), sus::None);
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
  auto sc = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);

  EXPECT_EQ(sm.get_mut(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(sm.get_mut(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(sm.get_mut(3_usize), sus::None);
}

TEST(Slice, GetUnchecked) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(s.get_unchecked(unsafe_fn, 1_usize), 2_i32);
  EXPECT_EQ(s.get_unchecked(unsafe_fn, 2_usize), 3_i32);

  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
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

TEST(Slice, GetUncheckedMut) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);

  EXPECT_EQ(sm.get_unchecked_mut(unsafe_fn, 1_usize), 2_i32);
  EXPECT_EQ(sm.get_unchecked_mut(unsafe_fn, 2_usize), 3_i32);
}

TEST(Slice, IndexRange) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);

  EXPECT_EQ((sc[{0u, 1u}][0u]), 1_i32);
  EXPECT_EQ((sc[{0u, 1u}].len()), 1_usize);
  EXPECT_EQ((sc[{1u, 3u}][1u]), 3_i32);
  EXPECT_EQ((sc[{1u, 3u}].len()), 2_usize);

  EXPECT_EQ((sc[{1u, 1u}].len()), 0_usize);
  // Start == End == the original End is an empty Slice.
  EXPECT_EQ((sc[{3u, 3u}].len()), 0_usize);

  EXPECT_EQ((sm[{0u, 3u}][0u]), 1_i32);
  EXPECT_EQ((sm[{0u, 3u}].len()), 3_usize);
  EXPECT_EQ((sm[{2u, 3u}][0u]), 3_i32);
  EXPECT_EQ((sm[{2u, 3u}].len()), 1_usize);

  EXPECT_EQ((sm[{1u, 1u}].len()), 0_usize);
  // Start == End == the original End is an empty Slice.
  EXPECT_EQ((sm[{3u, 3u}].len()), 0_usize);

  // Rvalue Slices are usable as they are reference types.
  EXPECT_EQ((sc[{1u, 3u}][{1u, 2u}][0u]), 3_i32);
  EXPECT_EQ((sm[{1u, 3u}][{1u, 2u}][0u]), 3_i32);
}

TEST(SliceDeathTest, IndexRange) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((sc[{0u, 4u}]), "");
  EXPECT_DEATH((sc[{3u, 4u}]), "");
  EXPECT_DEATH((sm[{1u, 4u}]), "");
  EXPECT_DEATH((sm[{2u, 4u}]), "");
  EXPECT_DEATH((sm[{4u, 4u}]), "");
#endif
}

TEST(Slice, GetRange) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(s.get_range({0u, 3u}).unwrap()[1u], 2_i32);
  EXPECT_EQ(s.get_range({1u, 3u}).unwrap()[1u], 3_i32);
  EXPECT_EQ(s.get_range({1u, 4u}), sus::None);
  EXPECT_EQ(s.get_range({3u, 3u}).unwrap().len(), 0_usize);
  EXPECT_EQ(s.get_range({4u, 4u}), sus::None);

  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(sm.get_range({0u, 3u}).unwrap()[1u], 2_i32);
  EXPECT_EQ(sm.get_range({1u, 3u}).unwrap()[1u], 3_i32);
  EXPECT_EQ(sm.get_range({1u, 4u}), sus::None);
  EXPECT_EQ(sm.get_range({3u, 3u}).unwrap().len(), 0_usize);
  EXPECT_EQ(sm.get_range({4u, 4u}), sus::None);

  // Rvalue Slices are usable as they are reference types.
  EXPECT_EQ(s.get_range({3u, 3u}).unwrap().get_range({0u, 0u}).unwrap().len(),
            0u);
  EXPECT_EQ(s.get_range({1u, 3u}).unwrap().get_range({1u, 2u}).unwrap().len(),
            1u);
  EXPECT_EQ(s.get_range({1u, 3u}).unwrap().get_range({1u, 2u}).unwrap()[0u],
            3_i32);
}

TEST(Slice, GetUncheckedRange) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(s.get_range_unchecked(unsafe_fn, {0u, 2u})[1u], 2_i32);
  EXPECT_EQ(s.get_range_unchecked(unsafe_fn, {2u, 3u})[0u], 3_i32);

  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(sm.get_range_unchecked(unsafe_fn, {0u, 2u})[1u], 2_i32);
  EXPECT_EQ(sm.get_range_unchecked(unsafe_fn, {2u, 3u})[0u], 3_i32);
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

TEST(Slice, Len) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(s.len(), 3u);

  auto se = Slice<i32>::from_raw_parts(unsafe_fn, a, 0_usize);
  EXPECT_EQ(se.len(), 0u);
}

TEST(Slice, IsEmpty) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_FALSE(s.is_empty());

  auto se = Slice<i32>::from_raw_parts(unsafe_fn, a, 0_usize);
  EXPECT_TRUE(se.is_empty());
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

TEST(Slice, Sort) {
  // clang-format off
  sus::Array<Sortable, 9> unsorted = sus::array(
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
  sus::Array<Sortable, 9> sorted = sus::array(
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

  Slice<Sortable> s = unsorted.as_mut();
  s.sort();
  for (usize i = 0u; i < s.len(); i += 1u) {
    EXPECT_EQ(sorted[i], s[i]);
  }
}

TEST(Slice, SortBy) {
  // clang-format off
  sus::Array<Sortable, 9> unsorted = sus::array(
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
  sus::Array<Sortable, 9> sorted = sus::array(
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

  Slice<Sortable> s = unsorted.as_mut();
  // Sorts backward.
  s.sort_by([](const auto& a, const auto& b) { return b <=> a; });
  for (usize i = 0u; i < s.len(); i += 1u) {
    EXPECT_EQ(sorted[i], s[i]);
  }
}

TEST(Slice, SortUnstable) {
  sus::Array<i32, 6> unsorted = sus::array(3, 4, 2, 1, 6, 5);
  sus::Array<i32, 6> sorted = sus::array(1, 2, 3, 4, 5, 6);

  Slice<i32> s = unsorted.as_mut();
  s.sort_unstable();
  for (usize i = 0u; i < s.len(); i += 1u) {
    EXPECT_EQ(sorted[i], s[i]);
  }
}

TEST(Slice, SortUnstableBy) {
  sus::Array<i32, 6> unsorted = sus::array(3, 4, 2, 1, 6, 5);
  sus::Array<i32, 6> sorted = sus::array(6, 5, 4, 3, 2, 1);

  Slice<i32> s = unsorted.as_mut();
  // Sorts backward.
  s.sort_unstable_by([](const auto& a, const auto& b) { return b <=> a; });
  for (usize i = 0u; i < s.len(); i += 1u) {
    EXPECT_EQ(sorted[i], s[i]);
  }
}

static_assert(sus::construct::Default<Slice<i32>>);

TEST(Slice, Default) {
  Slice<i32> s;
  EXPECT_TRUE(s.is_empty());
}

}  // namespace
