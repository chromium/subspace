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
#include "subspace/containers/vec.h"
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

  EXPECT_EQ((sc["0..1"_r][0u]), 1_i32);
  EXPECT_EQ((sc["0..1"_r].len()), 1_usize);
  EXPECT_EQ((sc["1..3"_r][1u]), 3_i32);
  EXPECT_EQ((sc["1..3"_r].len()), 2_usize);

  EXPECT_EQ((sc["1..1"_r].len()), 0_usize);
  // Start == End == the original End is an empty Slice.
  EXPECT_EQ((sc["3..3"_r].len()), 0_usize);

  EXPECT_EQ((sm["0..3"_r][0u]), 1_i32);
  EXPECT_EQ((sm["0..3"_r].len()), 3_usize);
  EXPECT_EQ((sm["2..3"_r][0u]), 3_i32);
  EXPECT_EQ((sm["2..3"_r].len()), 1_usize);

  EXPECT_EQ((sm["1..1"_r].len()), 0_usize);
  // Start == End == the original End is an empty Slice.
  EXPECT_EQ((sm["3..3"_r].len()), 0_usize);

  // Rvalue Slices are usable as they are reference types.
  EXPECT_EQ((sc["1..3"_r]["1..2"_r][0u]), 3_i32);
  EXPECT_EQ((sm["1..3"_r]["1..2"_r][0u]), 3_i32);
}

TEST(SliceDeathTest, IndexRange) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((sc["0..4"_r]), "");
  EXPECT_DEATH((sc["3..4"_r]), "");
  EXPECT_DEATH((sm["1..4"_r]), "");
  EXPECT_DEATH((sm["2..4"_r]), "");
  EXPECT_DEATH((sm["4..4"_r]), "");
#endif
}

TEST(Slice, GetRange) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(s.get_range("0..3"_r).unwrap()[1u], 2_i32);
  EXPECT_EQ(s.get_range("1..3"_r).unwrap()[1u], 3_i32);
  EXPECT_EQ(s.get_range("1..4"_r), sus::None);
  EXPECT_EQ(s.get_range("3..3"_r).unwrap().len(), 0_usize);
  EXPECT_EQ(s.get_range("4..4"_r), sus::None);

  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(sm.get_range("0..3"_r).unwrap()[1u], 2_i32);
  EXPECT_EQ(sm.get_range("1..3"_r).unwrap()[1u], 3_i32);
  EXPECT_EQ(sm.get_range("1..4"_r), sus::None);
  EXPECT_EQ(sm.get_range("3..3"_r).unwrap().len(), 0_usize);
  EXPECT_EQ(sm.get_range("4..4"_r), sus::None);

  // Rvalue Slices are usable as they are reference types.
  EXPECT_EQ(s.get_range("3..3"_r).unwrap().get_range("0..0"_r).unwrap().len(),
            0u);
  EXPECT_EQ(s.get_range("1..3"_r).unwrap().get_range("1..2"_r).unwrap().len(),
            1u);
  EXPECT_EQ(s.get_range("1..3"_r).unwrap().get_range("1..2"_r).unwrap()[0u],
            3_i32);
}

TEST(Slice, GetUncheckedRange) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<const i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(s.get_range_unchecked(unsafe_fn, "0..2"_r)[1u], 2_i32);
  EXPECT_EQ(s.get_range_unchecked(unsafe_fn, "2..3"_r)[0u], 3_i32);

  auto sm = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  EXPECT_EQ(sm.get_range_unchecked(unsafe_fn, "0..2"_r)[1u], 2_i32);
  EXPECT_EQ(sm.get_range_unchecked(unsafe_fn, "2..3"_r)[0u], 3_i32);
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

TEST(Slice, DoubleEndedIterator) {
  {
    const usize ar[] = {1u, 2u, 3u};
    auto slice = Slice<const usize>::from(ar);

    auto it = slice.iter();
    static_assert(sus::iter::DoubleEndedIterator<decltype(it), const usize&>);
    EXPECT_EQ(it.next_back(), sus::some(3_usize).construct());
    EXPECT_EQ(it.next_back(), sus::some(2_usize).construct());
    EXPECT_EQ(it.next_back(), sus::some(1_usize).construct());
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    auto slice = Slice<usize>::from(ar);

    auto it = slice.iter_mut();
    static_assert(sus::iter::DoubleEndedIterator<decltype(it), usize&>);
    EXPECT_EQ(it.next_back(), sus::some(3_usize).construct());
    EXPECT_EQ(it.next_back(), sus::some(2_usize).construct());
    EXPECT_EQ(it.next_back(), sus::some(1_usize).construct());
    EXPECT_EQ(it.next_back(), sus::None);
  }
}

TEST(Slice, ExactSizeIterator) {
  {
    const usize ar[] = {1u, 2u, 3u};
    auto slice = Slice<const usize>::from(ar);

    auto it = slice.iter();
    static_assert(sus::iter::ExactSizeIterator<decltype(it), const usize&>);
    EXPECT_EQ(it.size_hint().lower, 3u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>::some(3u));
    EXPECT_EQ(it.exact_size_hint(), 3u);
    EXPECT_EQ(it.next_back(), sus::some(3_usize).construct());
    EXPECT_EQ(it.size_hint().lower, 2u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>::some(2u));
    EXPECT_EQ(it.exact_size_hint(), 2u);
    EXPECT_EQ(it.next_back(), sus::some(2_usize).construct());
    EXPECT_EQ(it.size_hint().lower, 1u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>::some(1u));
    EXPECT_EQ(it.exact_size_hint(), 1u);
    EXPECT_EQ(it.next_back(), sus::some(1_usize).construct());
    EXPECT_EQ(it.size_hint().lower, 0u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>::some(0u));
    EXPECT_EQ(it.exact_size_hint(), 0u);
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.size_hint().lower, 0u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>::some(0u));
    EXPECT_EQ(it.exact_size_hint(), 0u);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    auto slice = Slice<usize>::from(ar);

    auto it = slice.iter_mut();
    static_assert(sus::iter::ExactSizeIterator<decltype(it), usize&>);
    EXPECT_EQ(it.next_back(), sus::some(3_usize).construct());
    EXPECT_EQ(it.next_back(), sus::some(2_usize).construct());
    EXPECT_EQ(it.next_back(), sus::some(1_usize).construct());
    EXPECT_EQ(it.next_back(), sus::None);
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

  Slice<Sortable> s = unsorted.as_mut_slice();
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

  Slice<Sortable> s = unsorted.as_mut_slice();
  // Sorts backward.
  s.sort_by([](const auto& a, const auto& b) { return b <=> a; });
  for (usize i = 0u; i < s.len(); i += 1u) {
    EXPECT_EQ(sorted[i], s[i]);
  }
}

TEST(Slice, SortUnstable) {
  sus::Array<i32, 6> unsorted = sus::array(3, 4, 2, 1, 6, 5);
  sus::Array<i32, 6> sorted = sus::array(1, 2, 3, 4, 5, 6);

  Slice<i32> s = unsorted.as_mut_slice();
  s.sort_unstable();
  for (usize i = 0u; i < s.len(); i += 1u) {
    EXPECT_EQ(sorted[i], s[i]);
  }
}

TEST(Slice, SortUnstableBy) {
  sus::Array<i32, 6> unsorted = sus::array(3, 4, 2, 1, 6, 5);
  sus::Array<i32, 6> sorted = sus::array(6, 5, 4, 3, 2, 1);

  Slice<i32> s = unsorted.as_mut_slice();
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

TEST(Slice, ToVec) {
  sus::Array<i32, 6> array = sus::array(3, 4, 2, 1, 6, 5);
  auto slice =
      sus::Slice<const i32>::from_raw_parts(unsafe_fn, array.as_ptr(), 6u);
  EXPECT_EQ(array.as_ptr(), slice.as_ptr());
  sus::Vec<i32> vec = slice.to_vec();
  // The Vec is a new allocation.
  EXPECT_NE(vec.as_ptr(), slice.as_ptr());
  // And it has all the same content, cloned.
  EXPECT_EQ(vec.len(), 6u);
  EXPECT_EQ(vec[0u], 3);
  EXPECT_EQ(vec[1u], 4);
  EXPECT_EQ(vec[2u], 2);
  EXPECT_EQ(vec[3u], 1);
  EXPECT_EQ(vec[4u], 6);
  EXPECT_EQ(vec[5u], 5);

  // Verify Clone is used, not just Copy.
  struct Cloner {
    i32 i;

    Cloner(i32 i) : i(i) {}

    Cloner(Cloner&&) = default;
    Cloner& operator=(Cloner&&) = default;

    Cloner clone() const noexcept { return Cloner(i + 1); }
  };
  static_assert(sus::mem::Clone<Cloner>);
  static_assert(!sus::mem::Copy<Cloner>);
  sus::Array<Cloner, 2> v = sus::array(Cloner(1), Cloner(2));
  sus::Vec<Cloner> v2 =
      sus::Slice<const Cloner>::from_raw_parts(unsafe_fn, v.as_ptr(), 2u)
          .to_vec();
  EXPECT_NE(v.as_ptr(), v2.as_ptr());
  EXPECT_EQ(v.len(), v2.len());
  EXPECT_EQ(v[0u].i + 1, v2[0u].i);
  EXPECT_EQ(v[1u].i + 1, v2[1u].i);
}

TEST(Slice, AsPtr) {
  sus::Array<i32, 3> array = sus::array(3, 4, 2);
  auto slice =
      sus::Slice<const i32>::from_raw_parts(unsafe_fn, array.as_ptr(), 3u);
  EXPECT_EQ(slice.as_ptr(), array.as_ptr());
}

TEST(Slice, AsPtrRange) {
  sus::Array<i32, 3> array = sus::array(3, 4, 2);
  auto slice =
      sus::Slice<const i32>::from_raw_parts(unsafe_fn, array.as_ptr(), 3u);
  auto r = slice.as_ptr_range();
  static_assert(std::same_as<decltype(r), sus::ops::Range<const i32*>>);
  EXPECT_EQ(r.start, array.as_ptr());
  EXPECT_EQ(r.finish, array.as_ptr() + 3u);
}

TEST(Slice, AsMutPtr) {
  sus::Array<i32, 3> array = sus::array(3, 4, 2);
  auto slice =
      sus::Slice<i32>::from_raw_parts(unsafe_fn, array.as_mut_ptr(), 3u);
  EXPECT_EQ(slice.as_mut_ptr(), array.as_mut_ptr());
}

TEST(Slice, AsMutPtrRange) {
  sus::Array<i32, 3> array = sus::array(3, 4, 2);
  auto slice =
      sus::Slice<i32>::from_raw_parts(unsafe_fn, array.as_mut_ptr(), 3u);
  auto r = slice.as_mut_ptr_range();
  static_assert(std::same_as<decltype(r), sus::ops::Range<i32*>>);
  EXPECT_EQ(r.start, array.as_mut_ptr());
  EXPECT_EQ(r.finish, array.as_mut_ptr() + 3u);
}

TEST(Slice, BinarySearch) {
  sus::Vec<i32> v = sus::vec(0, 1, 1, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55);
  auto s = v.as_slice();

  EXPECT_EQ(s.binary_search(13), sus::ok(9_usize).construct<usize>());
  EXPECT_EQ(s.binary_search(4), sus::err(7_usize).construct<usize>());
  EXPECT_EQ(s.binary_search(100), sus::err(13_usize).construct<usize>());
  auto r = s.binary_search(1);
  EXPECT_TRUE("1..=4"_r.contains(sus::move(r).unwrap()));
}

TEST(Slice, BinarySearchBy) {
  sus::Vec<i32> v = sus::vec(0, 1, 1, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55);
  auto s = v.as_slice();

  EXPECT_EQ(s.binary_search_by([](const i32& p) { return p <=> 13; }),
            sus::ok(9_usize).construct<usize>());
  EXPECT_EQ(s.binary_search_by([](const i32& p) { return p <=> 4; }),
            sus::err(7_usize).construct<usize>());
  EXPECT_EQ(s.binary_search_by([](const i32& p) { return p <=> 100; }),
            sus::err(13_usize).construct<usize>());
  auto r = s.binary_search_by([](const i32& p) { return p <=> 1; });
  EXPECT_TRUE("1..=4"_r.contains(sus::move(r).unwrap()));
}

TEST(Slice, BinarySearchByKey) {
  sus::Array<sus::Tuple<i32, i32>, 13> arr = sus::array(
      sus::tuple(0, 0), sus::tuple(2, 1), sus::tuple(4, 1), sus::tuple(5, 1),
      sus::tuple(3, 1), sus::tuple(1, 2), sus::tuple(2, 3), sus::tuple(4, 5),
      sus::tuple(5, 8), sus::tuple(3, 13), sus::tuple(1, 21), sus::tuple(2, 34),
      sus::tuple(4, 55));
  auto s = sus::Slice<const sus::Tuple<i32, i32>>::from_raw_parts(
      unsafe_fn, arr.as_ptr(), arr.len());

  EXPECT_EQ(s.binary_search_by_key(13_i32,
                                   [](const sus::Tuple<i32, i32>& pair) -> i32 {
                                     return pair.at<1u>();
                                   }),
            sus::ok(9_usize).construct<usize>());
  EXPECT_EQ(s.binary_search_by_key(4_i32,
                                   [](const sus::Tuple<i32, i32>& pair) -> i32 {
                                     return pair.at<1u>();
                                   }),
            sus::err(7_usize).construct<usize>());
  EXPECT_EQ(s.binary_search_by_key(100_i32,
                                   [](const sus::Tuple<i32, i32>& pair) -> i32 {
                                     return pair.at<1u>();
                                   }),
            sus::err(13_usize).construct<usize>());
  auto r = s.binary_search_by_key(
      1_i32,
      [](const sus::Tuple<i32, i32>& pair) -> i32 { return pair.at<1u>(); });
  EXPECT_TRUE("1..=4"_r.contains(sus::move(r).unwrap()));
}

TEST(Slice, Chunks) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::Slice<const i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::Slice<const i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(10u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size == len: next_back().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(10u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size > len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(13u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Chunk size > len: next_back().
  {
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(13u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size > len, and multiple of len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(20u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size > len, and multiple of len: next_back().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(20u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size divides into len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<const i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size divides into len: next_back().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<const i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size doesn't divide into len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<const i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 3u);
    EXPECT_EQ(n.as_ptr(), &v[7u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size doesn't divide into len: next_back().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<const i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 3u);
    EXPECT_EQ(n.as_ptr(), &v[7u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(Slice, ChunksMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_mut_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks_mut(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::Slice<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::Slice<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(10u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size == len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(10u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size > len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(13u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Chunk size > len: next_back().
  {
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(13u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size > len, and multiple of len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(20u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size > len, and multiple of len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(20u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size divides into len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size divides into len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size doesn't divide into len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 3u);
    EXPECT_EQ(n.as_ptr(), &v[7u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size doesn't divide into len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_mut(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 3u);
    EXPECT_EQ(n.as_ptr(), &v[7u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(Slice, SplitAtUnchecked) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  sus::Slice<const i32> s = v.as_slice();

  {
    // Empty left.
    auto [a, b] = s.split_at_unchecked(unsafe_fn, 0u);
    static_assert(std::same_as<decltype(a), sus::Slice<const i32>>);
    static_assert(std::same_as<decltype(b), sus::Slice<const i32>>);
    EXPECT_EQ(a.len(), 0u);
    EXPECT_EQ(b.len(), 10u);
    EXPECT_EQ(b.as_ptr(), &v[0u]);
  }
  {
    // Empty Right.
    auto [a, b] = s.split_at_unchecked(unsafe_fn, 10u);
    EXPECT_EQ(a.len(), 10u);
    EXPECT_EQ(b.len(), 0u);
    EXPECT_EQ(a.as_ptr(), &v[0u]);
  }
  {
    // Middle.
    auto [a, b] = s.split_at_unchecked(unsafe_fn, 6u);
    EXPECT_EQ(a.len(), 6u);
    EXPECT_EQ(b.len(), 4u);
    EXPECT_EQ(a.as_ptr(), &v[0u]);
    EXPECT_EQ(b.as_ptr(), &v[6u]);
  }
}

TEST(Slice, SplitAtMutUnchecked) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  sus::Slice<i32> s = v.as_mut_slice();

  {
    // Empty left.
    auto [a, b] = s.split_at_mut_unchecked(unsafe_fn, 0u);
    static_assert(std::same_as<decltype(a), sus::Slice<i32>>);
    static_assert(std::same_as<decltype(b), sus::Slice<i32>>);
    EXPECT_EQ(a.len(), 0u);
    EXPECT_EQ(b.len(), 10u);
    EXPECT_EQ(b.as_ptr(), &v[0u]);
  }
  {
    // Empty Right.
    auto [a, b] = s.split_at_mut_unchecked(unsafe_fn, 10u);
    EXPECT_EQ(a.len(), 10u);
    EXPECT_EQ(b.len(), 0u);
    EXPECT_EQ(a.as_ptr(), &v[0u]);
  }
  {
    // Middle.
    auto [a, b] = s.split_at_mut_unchecked(unsafe_fn, 6u);
    EXPECT_EQ(a.len(), 6u);
    EXPECT_EQ(b.len(), 4u);
    EXPECT_EQ(a.as_ptr(), &v[0u]);
    EXPECT_EQ(b.as_ptr(), &v[6u]);
  }
}

TEST(Slice, ChunksExact) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks_exact(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::Slice<const i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::Slice<const i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(10u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size == len: next_back().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(10u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size > len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(13u);
    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);

    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  // Chunk size > len: next_back().
  {
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(13u);
    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);

    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  {
    // Chunk size > len, and multiple of len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(20u);
    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);

    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  {
    // Chunk size > len, and multiple of len: next_back().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(20u);
    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);

    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  {
    // Chunk size divides into len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<const i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size divides into len: next_back().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<const i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size doesn't divide into len: next().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);

    // Remainder is available at the end too.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
  }
  {
    // Chunk size doesn't divide into len: next_back().
    sus::iter::Iterator<sus::Slice<const i32>> auto it = s.chunks_exact(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<const i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);

    // Remainder is available at the end too.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
  }
}

TEST(Slice, ChunksExactMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_mut_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks_exact_mut(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::Slice<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::Slice<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(10u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size == len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(10u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 10u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size > len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(13u);
    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);

    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  // Chunk size > len: next_back().
  {
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(13u);
    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);

    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  {
    // Chunk size > len, and multiple of len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(20u);
    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);

    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  {
    // Chunk size > len, and multiple of len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(20u);
    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);

    EXPECT_EQ(it.remainder().len(), 10u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  {
    // Chunk size divides into len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    // Chunk size divides into len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  {
    // Chunk size doesn't divide into len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);

    // Remainder is available at the end too.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
  }
  {
    // Chunk size doesn't divide into len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact_mut(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 0u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 0u);
      EXPECT_EQ(upper, sus::some(0u));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);

    // Remainder is available at the end too.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
  }
}

}  // namespace
