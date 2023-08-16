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

#include "sus/collections/slice.h"

#include <sstream>

#include "googletest/include/gtest/gtest.h"
#include "sus/collections/array.h"
#include "sus/collections/vec.h"
#include "sus/construct/into.h"
#include "sus/iter/iterator.h"
#include "sus/mem/clone.h"
#include "sus/mem/copy.h"
#include "sus/mem/move.h"
#include "sus/num/types.h"
#include "sus/prelude.h"
#include "sus/result/result.h"
#include "sus/test/ensure_use.h"
#include "sus/test/no_copy_move.h"

using sus::collections::Slice;
using sus::collections::SliceMut;
using sus::iter::IterRefCounter;
using sus::test::ensure_use;
using sus::test::NoCopyMove;

namespace {

static_assert(sus::mem::Copy<Slice<i32>>);
static_assert(sus::mem::Clone<Slice<i32>>);
static_assert(sus::mem::Move<Slice<i32>>);

TEST(Slice, FromRawParts) {
  i32 a[] = {1, 2, 3};
  auto sc = Slice<i32>::from_raw_parts(unsafe_fn, a, 3_usize);
  auto sm = SliceMut<i32>::from_raw_parts_mut(unsafe_fn, a, 3_usize);
}

TEST(Slice, Index) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();
  const auto scc = sc;
  const auto smc = sm;

  static_assert(std::same_as<const i32&, decltype(sc[0u])>);
  EXPECT_EQ(sc[0_usize], 1_i32);
  EXPECT_EQ(sc[2_usize], 3_i32);
  static_assert(std::same_as<const i32&, decltype(scc[0u])>);
  EXPECT_EQ(scc[0_usize], 1_i32);
  EXPECT_EQ(scc[2_usize], 3_i32);
  static_assert(std::same_as<i32&, decltype(sm[0u])>);
  EXPECT_EQ(sm[0_usize], 1_i32);
  EXPECT_EQ(sm[2_usize], 3_i32);
  static_assert(std::same_as<i32&, decltype(smc[0u])>);
  EXPECT_EQ(smc[0_usize], 1_i32);
  EXPECT_EQ(smc[2_usize], 3_i32);
}

TEST(SliceDeathTest, Index) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = sc[3_usize];
        EXPECT_EQ(x, 3);
      },
      "");
  EXPECT_DEATH(
      {
        auto y = sm[3_usize];
        EXPECT_EQ(y, 3);
      },
      "");
#endif
}

TEST(Slice, Get) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();

  EXPECT_EQ(sc.get(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(sc.get(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(sc.get(3_usize), sus::None);

  EXPECT_EQ(sm.get(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(sm.get(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(sm.get(3_usize), sus::None);
}

template <class T, class U>
concept HasGetMut = requires(T t, U u) { t.get_mut(u); };

// get_mut() is only available for mutable slices. A "const" mutable slice can
// still give mutable access as the mutability is encoded in the type.
static_assert(!HasGetMut<Slice<i32>, usize>);
static_assert(HasGetMut<SliceMut<i32>, usize>);
static_assert(!HasGetMut<const Slice<i32>, usize>);
static_assert(HasGetMut<const SliceMut<i32>, usize>);

TEST(Slice, GetMut) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();

  EXPECT_EQ(sm.get_mut(1_usize).unwrap(), 2_i32);
  EXPECT_EQ(sm.get_mut(2_usize).unwrap(), 3_i32);
  EXPECT_EQ(sm.get_mut(3_usize), sus::None);
}

TEST(Slice, GetUnchecked) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();

  EXPECT_EQ(sc.get_unchecked(unsafe_fn, 1_usize), 2_i32);
  EXPECT_EQ(sc.get_unchecked(unsafe_fn, 2_usize), 3_i32);

  EXPECT_EQ(sm.get_unchecked(unsafe_fn, 1_usize), 2_i32);
  EXPECT_EQ(sm.get_unchecked(unsafe_fn, 2_usize), 3_i32);
}

template <class T, class U>
concept HasGetUncheckedMut =
    requires(T t, U u) { t.get_unchecked_mut(unsafe_fn, u); };

// get_unchecked_mut() is only available for mutable slices of mutable types.
static_assert(!HasGetUncheckedMut<Slice<i32>, usize>);
static_assert(HasGetUncheckedMut<SliceMut<i32>, usize>);
static_assert(!HasGetUncheckedMut<const Slice<i32>, usize>);
static_assert(HasGetUncheckedMut<const SliceMut<i32>, usize>);

TEST(Slice, GetUncheckedMut) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sm = v.as_mut_slice();

  EXPECT_EQ(sm.get_unchecked_mut(unsafe_fn, 1_usize), 2_i32);
  EXPECT_EQ(sm.get_unchecked_mut(unsafe_fn, 2_usize), 3_i32);
}

TEST(Slice, IndexRange) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();
  const auto scc = sc;
  const auto smc = sm;

  static_assert(std::same_as<Slice<i32>, decltype(sc[".."_r])>);
  static_assert(std::same_as<Slice<i32>, decltype(scc[".."_r])>);
  static_assert(std::same_as<SliceMut<i32>, decltype(sm[".."_r])>);
  static_assert(std::same_as<SliceMut<i32>, decltype(smc[".."_r])>);

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
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto s = sc["0..4"_r];
        EXPECT_TRUE(s.is_empty());
      },
      "");
  EXPECT_DEATH(
      {
        auto s = sc["3..4"_r];
        EXPECT_TRUE(s.is_empty());
      },
      "");
  EXPECT_DEATH(
      {
        auto s = sm["1..4"_r];
        EXPECT_TRUE(s.is_empty());
      },
      "");
  EXPECT_DEATH(
      {
        auto s = sm["2..4"_r];
        EXPECT_TRUE(s.is_empty());
      },
      "");
  EXPECT_DEATH(
      {
        auto s = sm["4..4"_r];
        EXPECT_TRUE(s.is_empty());
      },
      "");
#endif
}

TEST(Slice, GetRange) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();

  EXPECT_EQ(sc.get_range("0..3"_r).unwrap()[1u], 2_i32);
  EXPECT_EQ(sc.get_range("1..3"_r).unwrap()[1u], 3_i32);
  EXPECT_EQ(sc.get_range("1..4"_r), sus::None);
  EXPECT_EQ(sc.get_range("3..3"_r).unwrap().len(), 0_usize);
  EXPECT_EQ(sc.get_range("4..4"_r), sus::None);

  EXPECT_EQ(sm.get_range("0..3"_r).unwrap()[1u], 2_i32);
  EXPECT_EQ(sm.get_range("1..3"_r).unwrap()[1u], 3_i32);
  EXPECT_EQ(sm.get_range("1..4"_r), sus::None);
  EXPECT_EQ(sm.get_range("3..3"_r).unwrap().len(), 0_usize);
  EXPECT_EQ(sm.get_range("4..4"_r), sus::None);

  // Rvalue Slices are usable as they are reference types.
  EXPECT_EQ(sc.get_range("3..3"_r).unwrap().get_range("0..0"_r).unwrap().len(),
            0u);
  EXPECT_EQ(sc.get_range("1..3"_r).unwrap().get_range("1..2"_r).unwrap().len(),
            1u);
  EXPECT_EQ(sc.get_range("1..3"_r).unwrap().get_range("1..2"_r).unwrap()[0u],
            3_i32);
}

TEST(Slice, GetUncheckedRange) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();

  EXPECT_EQ(sc.get_range_unchecked(unsafe_fn, "0..2"_r)[1u], 2_i32);
  EXPECT_EQ(sc.get_range_unchecked(unsafe_fn, "2..3"_r)[0u], 3_i32);

  EXPECT_EQ(sm.get_range_unchecked(unsafe_fn, "0..2"_r)[1u], 2_i32);
  EXPECT_EQ(sm.get_range_unchecked(unsafe_fn, "2..3"_r)[0u], 3_i32);
}

TEST(Slice, Into) {
  i32 a[] = {1, 2, 3};
  Slice<i32> s = sus::array_into(a);
  EXPECT_EQ(s.len(), 3u);
  SliceMut<i32> sm = sus::array_into(a);
  EXPECT_EQ(sm.len(), 3u);
}

TEST(Slice, From) {
  i32 a[] = {1, 2, 3};
  auto s = Slice<i32>::from(a);
  auto sm = SliceMut<i32>::from(a);
}

TEST(Slice, RangedForIter) {
  {
    usize ar[] = {1u, 2u, 3u};
    const auto slice = Slice<usize>::from(ar);
    auto sum = 0_usize;
    for (const usize& i : slice) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    const auto mslice = SliceMut<usize>::from(ar);
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
    const auto slice = Slice<usize>::from(ar);
    auto sum = 0_usize;
    for (const usize& i : slice.iter()) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    const auto mslice = SliceMut<usize>::from(ar);
    auto sum = 0_usize;
    for (const usize& i : mslice.iter()) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
}

TEST(Slice, IterMut) {
  usize ar[] = {1u, 2u, 3u};
  auto slice = SliceMut<usize>::from(ar);
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
    auto slice = Slice<usize>::from(ar);
    auto sum = 0_usize;
    for (const usize& i : sus::move(slice).into_iter()) {
      sum += i;
    }
    EXPECT_EQ(sum, 6_usize);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    auto slice = SliceMut<usize>::from(ar);
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
    auto slice = Slice<usize>::from(ar);

    auto it = slice.iter();
    static_assert(sus::iter::DoubleEndedIterator<decltype(it), const usize&>);
    EXPECT_EQ(it.next_back(), sus::some(3_usize).construct());
    EXPECT_EQ(it.next_back(), sus::some(2_usize).construct());
    EXPECT_EQ(it.next_back(), sus::some(1_usize).construct());
    EXPECT_EQ(it.next_back(), sus::None);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    auto slice = SliceMut<usize>::from(ar);

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
    auto slice = Slice<usize>::from(ar);

    auto it = slice.iter();
    static_assert(sus::iter::ExactSizeIterator<decltype(it), const usize&>);
    EXPECT_EQ(it.size_hint().lower, 3u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>(3u));
    EXPECT_EQ(it.exact_size_hint(), 3u);
    EXPECT_EQ(it.next_back(), sus::some(3_usize).construct());
    EXPECT_EQ(it.size_hint().lower, 2u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>(2u));
    EXPECT_EQ(it.exact_size_hint(), 2u);
    EXPECT_EQ(it.next_back(), sus::some(2_usize).construct());
    EXPECT_EQ(it.size_hint().lower, 1u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>(1u));
    EXPECT_EQ(it.exact_size_hint(), 1u);
    EXPECT_EQ(it.next_back(), sus::some(1_usize).construct());
    EXPECT_EQ(it.size_hint().lower, 0u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>(0u));
    EXPECT_EQ(it.exact_size_hint(), 0u);
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.size_hint().lower, 0u);
    EXPECT_EQ(it.size_hint().upper, sus::Option<usize>(0u));
    EXPECT_EQ(it.exact_size_hint(), 0u);
  }
  {
    usize ar[] = {1u, 2u, 3u};
    auto slice = SliceMut<usize>::from(ar);

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
  auto slice = Slice<usize>::from(ar);
  auto sum = 0_usize;
  for (const usize& i : slice) {
    sum += i;
  }
  EXPECT_EQ(sum, 6_usize);
}

TEST(Slice, Len) {
  auto v = sus::Vec<i32>(1, 2);
  v.push(3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();

  EXPECT_EQ(sc.len(), 3u);
  EXPECT_EQ(sm.len(), 3u);

  auto ve = sus::Vec<i32>();
  auto sce = ve.as_slice();
  auto sme = ve.as_mut_slice();

  EXPECT_EQ(sce.len(), 0u);
  EXPECT_EQ(sme.len(), 0u);
}

TEST(Slice, IsEmpty) {
  auto v = sus::Vec<i32>(1, 2, 3);
  auto sc = v.as_slice();
  auto sm = v.as_mut_slice();

  EXPECT_FALSE(sc.is_empty());
  EXPECT_FALSE(sm.is_empty());

  auto ve = sus::Vec<i32>();
  auto sce = ve.as_slice();
  auto sme = ve.as_mut_slice();

  EXPECT_TRUE(sce.is_empty());
  EXPECT_TRUE(sme.is_empty());
}

struct Sortable {
  Sortable(i32 value, i32 unique) : value(value), unique(unique) {}

  i32 value;
  i32 unique;

  friend bool operator==(const Sortable& a, const Sortable& b) noexcept {
    return a.value == b.value && a.unique == b.unique;
  }
  friend std::weak_ordering operator<=>(const Sortable& a,
                                        const Sortable& b) noexcept {
    return a.value <=> b.value;
  }
};
static_assert(sus::ops::Ord<Sortable>);

TEST(SliceMut, Sort) {
  {
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

    SliceMut<Sortable> s = unsorted.as_mut_slice();
    s.sort();
    for (usize i = 0u; i < s.len(); i += 1u) {
      EXPECT_EQ(sorted[i], s[i]);
    }
  }

  // Weak ordering is sufficient.
  {
    struct S {
      i32 i;
      constexpr std::weak_ordering operator<=>(const S& b) const noexcept {
        return i <=> b.i;
      }
    };

    auto a = sus::Vec<S>(S(1), S(3), S(0), S(4));
    a.as_mut_slice().sort();
    EXPECT_TRUE(a.iter().eq_by(sus::Slice<S>::from({S(0), S(1), S(3), S(4)}),
                               [](S a, S b) { return a.i == b.i; }));
  }
}

TEST(SliceMut, SortBy) {
  {
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

    SliceMut<Sortable> s = unsorted.as_mut_slice();
    // Sorts backward.
    s.sort_by([](const auto& a, const auto& b) { return b <=> a; });
    for (usize i = 0u; i < s.len(); i += 1u) {
      EXPECT_EQ(sorted[i], s[i]);
    }
  }

  // Weak ordering is sufficient.
  {
    struct S {
      i32 i;
    };

    auto a = sus::Vec<S>(S(1), S(3), S(0), S(4));
    a.as_mut_slice().sort_by(
        [](S a, S b) noexcept -> std::weak_ordering { return a.i <=> b.i; });
    EXPECT_TRUE(a.iter().eq_by(sus::Slice<S>::from({S(0), S(1), S(3), S(4)}),
                               [](S a, S b) { return a.i == b.i; }));
  }
}

TEST(SliceMut, SortByKey) {
  struct Unsortable {
    Sortable sortable;
  };
  auto unsortable_key =
      [](const Unsortable& u sus_lifetimebound) noexcept -> const Sortable& {
    return u.sortable;
  };

  // clang-format off
  sus::Array<Unsortable, 9> unsorted = sus::array(
    Unsortable(Sortable(3, 0)),
    Unsortable(Sortable(3, 1)),
    Unsortable(Sortable(4, 0)),
    Unsortable(Sortable(2, 0)),
    Unsortable(Sortable(2, 1)),
    Unsortable(Sortable(1, 0)),
    Unsortable(Sortable(3, 2)),
    Unsortable(Sortable(6, 0)),
    Unsortable(Sortable(5, 0))
  );
  sus::Array<Unsortable, 9> sorted = sus::array(
    Unsortable(Sortable(1, 0)),
    Unsortable(Sortable(2, 0)),
    Unsortable(Sortable(2, 1)),
    Unsortable(Sortable(3, 0)),
    Unsortable(Sortable(3, 1)),
    Unsortable(Sortable(3, 2)),
    Unsortable(Sortable(4, 0)),
    Unsortable(Sortable(5, 0)),
    Unsortable(Sortable(6, 0))
  );
  // clang-format on

  SliceMut<Unsortable> s = unsorted.as_mut_slice();
  // Sorts by the inner Sortable.
  s.sort_by_key(unsortable_key);
  for (usize i = 0u; i < s.len(); i += 1u) {
    EXPECT_EQ(sorted[i].sortable, s[i].sortable);
  }
}

TEST(SliceMut, SortByCachedKey) {
  struct Unsortable {
    Sortable sortable;
  };
  auto unsortable_key =
      [](const Unsortable& u sus_lifetimebound) noexcept -> const Sortable& {
    return u.sortable;
  };

  // clang-format off
  sus::Array<Unsortable, 9> unsorted = sus::array(
    Unsortable(Sortable(3, 0)),
    Unsortable(Sortable(3, 1)),
    Unsortable(Sortable(4, 0)),
    Unsortable(Sortable(2, 0)),
    Unsortable(Sortable(2, 1)),
    Unsortable(Sortable(1, 0)),
    Unsortable(Sortable(3, 2)),
    Unsortable(Sortable(6, 0)),
    Unsortable(Sortable(5, 0))
  );
  sus::Array<Unsortable, 9> sorted = sus::array(
    Unsortable(Sortable(1, 0)),
    Unsortable(Sortable(2, 0)),
    Unsortable(Sortable(2, 1)),
    Unsortable(Sortable(3, 0)),
    Unsortable(Sortable(3, 1)),
    Unsortable(Sortable(3, 2)),
    Unsortable(Sortable(4, 0)),
    Unsortable(Sortable(5, 0)),
    Unsortable(Sortable(6, 0))
  );
  // clang-format on

  SliceMut<Unsortable> s = unsorted.as_mut_slice();
  // Sorts by the inner Sortable.
  s.sort_by_cached_key(unsortable_key);
  for (usize i = 0u; i < s.len(); i += 1u) {
    EXPECT_EQ(sorted[i].sortable, s[i].sortable);
  }
}

TEST(SliceMut, SortUnstable) {
  {
    sus::Array<i32, 6> unsorted = sus::array(3, 4, 2, 1, 6, 5);
    sus::Array<i32, 6> sorted = sus::array(1, 2, 3, 4, 5, 6);

    SliceMut<i32> s = unsorted.as_mut_slice();
    s.sort_unstable();
    for (usize i = 0u; i < s.len(); i += 1u) {
      EXPECT_EQ(sorted[i], s[i]);
    }
  }

  // Weak ordering is sufficient.
  {
    struct S {
      i32 i;
      constexpr std::weak_ordering operator<=>(const S& b) const noexcept {
        return i <=> b.i;
      }
    };

    auto a = sus::Vec<S>(S(1), S(3), S(0), S(4));
    a.as_mut_slice().sort_unstable();
    EXPECT_TRUE(a.iter().eq_by(sus::Slice<S>::from({S(0), S(1), S(3), S(4)}),
                               [](S a, S b) { return a.i == b.i; }));
  }
}

TEST(SliceMut, SortUnstableBy) {
  {
    sus::Array<i32, 6> unsorted = sus::array(3, 4, 2, 1, 6, 5);
    sus::Array<i32, 6> sorted = sus::array(6, 5, 4, 3, 2, 1);

    SliceMut<i32> s = unsorted.as_mut_slice();
    // Sorts backward.
    s.sort_unstable_by([](const auto& a, const auto& b) { return b <=> a; });
    for (usize i = 0u; i < s.len(); i += 1u) {
      EXPECT_EQ(sorted[i], s[i]);
    }
  }

  // Weak ordering is sufficient.
  {
    struct S {
      i32 i;
    };

    auto a = sus::Vec<S>(S(1), S(3), S(0), S(4));
    a.as_mut_slice().sort_unstable_by(
        [](S a, S b) noexcept -> std::weak_ordering { return a.i <=> b.i; });
    EXPECT_TRUE(a.iter().eq_by(sus::Slice<S>::from({S(0), S(1), S(3), S(4)}),
                               [](S a, S b) { return a.i == b.i; }));
  }
}

TEST(SliceMut, SortUnstableByKey) {
  struct Unsortable {
    i32 sortable;
  };
  auto unsortable_key =
      [](const Unsortable& u sus_lifetimebound) noexcept -> const i32& {
    return u.sortable;
  };

  sus::Array<Unsortable, 6> unsorted =
      sus::array(Unsortable(3), Unsortable(4), Unsortable(2), Unsortable(1),
                 Unsortable(6), Unsortable(5));
  sus::Array<Unsortable, 6> sorted =
      sus::array(Unsortable(1), Unsortable(2), Unsortable(3), Unsortable(4),
                 Unsortable(5), Unsortable(6));

  SliceMut<Unsortable> s = unsorted.as_mut_slice();
  s.sort_unstable_by_key(unsortable_key);
  for (usize i = 0u; i < s.len(); i += 1u) {
    EXPECT_EQ(sorted[i].sortable, s[i].sortable);
  }
}

static_assert(sus::construct::Default<Slice<i32>>);
static_assert(sus::construct::Default<SliceMut<i32>>);

TEST(Slice, Default) {
  Slice<i32> s;
  EXPECT_TRUE(s.is_empty());
  SliceMut<i32> sm;
  EXPECT_TRUE(sm.is_empty());
}

TEST(Slice, ToVec) {
  sus::Array<i32, 6> array = sus::array(3, 4, 2, 1, 6, 5);
  {
    auto s = array.as_slice();
    EXPECT_EQ(array.as_ptr(), s.as_ptr());
    sus::Vec<i32> vec = s.to_vec();
    // The Vec is a new allocation.
    EXPECT_NE(vec.as_ptr(), s.as_ptr());
    // And it has all the same content, cloned.
    EXPECT_EQ(vec.len(), 6u);
    EXPECT_EQ(vec[0u], 3);
    EXPECT_EQ(vec[1u], 4);
    EXPECT_EQ(vec[2u], 2);
    EXPECT_EQ(vec[3u], 1);
    EXPECT_EQ(vec[4u], 6);
    EXPECT_EQ(vec[5u], 5);
  }
  {
    auto sm = array.as_mut_slice();
    EXPECT_EQ(array.as_ptr(), sm.as_ptr());
    sus::Vec<i32> vec = sm.to_vec();
    // The Vec is a new allocation.
    EXPECT_NE(vec.as_ptr(), sm.as_ptr());
    // And it has all the same content, cloned.
    EXPECT_EQ(vec.len(), 6u);
    EXPECT_EQ(vec[0u], 3);
    EXPECT_EQ(vec[1u], 4);
    EXPECT_EQ(vec[2u], 2);
    EXPECT_EQ(vec[3u], 1);
    EXPECT_EQ(vec[4u], 6);
    EXPECT_EQ(vec[5u], 5);
  }

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
  {
    auto s = v.as_slice();
    sus::Vec<Cloner> v2 = s.to_vec();
    EXPECT_NE(v.as_ptr(), v2.as_ptr());
    EXPECT_EQ(v.len(), v2.len());
    EXPECT_EQ(v[0u].i + 1, v2[0u].i);
    EXPECT_EQ(v[1u].i + 1, v2[1u].i);
  }
  {
    auto sm = v.as_mut_slice();
    sus::Vec<Cloner> v2 = sm.to_vec();
    EXPECT_NE(v.as_ptr(), v2.as_ptr());
    EXPECT_EQ(v.len(), v2.len());
    EXPECT_EQ(v[0u].i + 1, v2[0u].i);
    EXPECT_EQ(v[1u].i + 1, v2[1u].i);
  }
}

TEST(Slice, AsPtr) {
  sus::Array<i32, 3> array = sus::array(3, 4, 2);
  auto s = array.as_slice();
  EXPECT_EQ(s.as_ptr(), array.as_ptr());

  auto sm = array.as_mut_slice();
  EXPECT_EQ(sm.as_ptr(), array.as_ptr());
}

TEST(Slice, AsPtrRange) {
  sus::Array<i32, 3> array = sus::array(3, 4, 2);
  {
    auto s = array.as_slice();
    auto r = s.as_ptr_range();
    static_assert(std::same_as<decltype(r), sus::ops::Range<const i32*>>);
    EXPECT_EQ(r.start, array.as_ptr());
    EXPECT_EQ(r.finish, array.as_ptr() + 3u);

    auto [a, b] = s.as_ptr_range();
    EXPECT_EQ(a, array.as_ptr());
    EXPECT_EQ(b, array.as_ptr() + 3u);
  }
  {
    auto sm = array.as_mut_slice();
    auto r = sm.as_ptr_range();
    static_assert(std::same_as<decltype(r), sus::ops::Range<const i32*>>);
    EXPECT_EQ(r.start, array.as_ptr());
    EXPECT_EQ(r.finish, array.as_ptr() + 3u);

    auto [a, b] = sm.as_mut_ptr_range();
    EXPECT_EQ(a, array.as_ptr());
    EXPECT_EQ(b, array.as_ptr() + 3u);
  }
}

TEST(SliceMut, AsMutPtr) {
  sus::Array<i32, 3> array = sus::array(3, 4, 2);
  auto sm = array.as_mut_slice();
  EXPECT_EQ(sm.as_mut_ptr(), array.as_mut_ptr());
}

TEST(SliceMut, AsMutPtrRange) {
  sus::Array<i32, 3> array = sus::array(3, 4, 2);
  auto sm = array.as_mut_slice();
  auto r = sm.as_mut_ptr_range();
  static_assert(std::same_as<decltype(r), sus::ops::Range<i32*>>);
  EXPECT_EQ(r.start, array.as_mut_ptr());
  EXPECT_EQ(r.finish, array.as_mut_ptr() + 3u);

  auto [a, b] = sm.as_mut_ptr_range();
  EXPECT_EQ(a, array.as_ptr());
  EXPECT_EQ(b, array.as_ptr() + 3u);
}

TEST(Slice, BinarySearch) {
  sus::Vec<i32> v = sus::vec(0, 1, 1, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55);
  {
    auto s = v.as_slice();
    EXPECT_EQ(s.binary_search(13), sus::result::ok(9_usize).construct<usize>());
    EXPECT_EQ(s.binary_search(4), sus::result::err(7_usize).construct<usize>());
    EXPECT_EQ(s.binary_search(100),
              sus::result::err(13_usize).construct<usize>());
    auto r = s.binary_search(1);
    EXPECT_TRUE("1..=4"_r.contains(sus::move(r).unwrap()));
  }
  {
    auto sm = v.as_mut_slice();
    EXPECT_EQ(sm.binary_search(13),
              sus::result::ok(9_usize).construct<usize>());
    EXPECT_EQ(sm.binary_search(4),
              sus::result::err(7_usize).construct<usize>());
    EXPECT_EQ(sm.binary_search(100),
              sus::result::err(13_usize).construct<usize>());
    auto r = sm.binary_search(1);
    EXPECT_TRUE("1..=4"_r.contains(sus::move(r).unwrap()));
  }
}

TEST(Slice, BinarySearchBy) {
  sus::Vec<i32> v = sus::vec(0, 1, 1, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55);

  {
    auto s = v.as_slice();
    EXPECT_EQ(s.binary_search_by([](const i32& p) { return p <=> 13; }),
              sus::result::ok(9_usize).construct<usize>());
    EXPECT_EQ(s.binary_search_by([](const i32& p) { return p <=> 4; }),
              sus::result::err(7_usize).construct<usize>());
    EXPECT_EQ(s.binary_search_by([](const i32& p) { return p <=> 100; }),
              sus::result::err(13_usize).construct<usize>());
    auto r = s.binary_search_by([](const i32& p) { return p <=> 1; });
    EXPECT_TRUE("1..=4"_r.contains(sus::move(r).unwrap()));
  }
  {
    auto sm = v.as_mut_slice();
    EXPECT_EQ(sm.binary_search_by([](const i32& p) { return p <=> 13; }),
              sus::result::ok(9_usize).construct<usize>());
    EXPECT_EQ(sm.binary_search_by([](const i32& p) { return p <=> 4; }),
              sus::result::err(7_usize).construct<usize>());
    EXPECT_EQ(sm.binary_search_by([](const i32& p) { return p <=> 100; }),
              sus::result::err(13_usize).construct<usize>());
    auto r = sm.binary_search_by([](const i32& p) { return p <=> 1; });
    EXPECT_TRUE("1..=4"_r.contains(sus::move(r).unwrap()));
  }
}

TEST(Slice, BinarySearchByKey) {
  sus::Array<sus::Tuple<i32, i32>, 13> arr = sus::array(
      sus::tuple(0, 0), sus::tuple(2, 1), sus::tuple(4, 1), sus::tuple(5, 1),
      sus::tuple(3, 1), sus::tuple(1, 2), sus::tuple(2, 3), sus::tuple(4, 5),
      sus::tuple(5, 8), sus::tuple(3, 13), sus::tuple(1, 21), sus::tuple(2, 34),
      sus::tuple(4, 55));
  {
    auto s = arr.as_slice();

    EXPECT_EQ(
        s.binary_search_by_key(13_i32,
                               [](const sus::Tuple<i32, i32>& pair) -> i32 {
                                 return pair.at<1u>();
                               }),
        sus::result::ok(9_usize).construct<usize>());
    EXPECT_EQ(
        s.binary_search_by_key(4_i32,
                               [](const sus::Tuple<i32, i32>& pair) -> i32 {
                                 return pair.at<1u>();
                               }),
        sus::result::err(7_usize).construct<usize>());
    EXPECT_EQ(
        s.binary_search_by_key(100_i32,
                               [](const sus::Tuple<i32, i32>& pair) -> i32 {
                                 return pair.at<1u>();
                               }),
        sus::result::err(13_usize).construct<usize>());
    auto r = s.binary_search_by_key(
        1_i32,
        [](const sus::Tuple<i32, i32>& pair) -> i32 { return pair.at<1u>(); });
    EXPECT_TRUE("1..=4"_r.contains(sus::move(r).unwrap()));
  }
  {
    auto sm = arr.as_mut_slice();

    EXPECT_EQ(
        sm.binary_search_by_key(13_i32,
                                [](const sus::Tuple<i32, i32>& pair) -> i32 {
                                  return pair.at<1u>();
                                }),
        sus::result::ok(9_usize).construct<usize>());
    EXPECT_EQ(
        sm.binary_search_by_key(4_i32,
                                [](const sus::Tuple<i32, i32>& pair) -> i32 {
                                  return pair.at<1u>();
                                }),
        sus::result::err(7_usize).construct<usize>());
    EXPECT_EQ(
        sm.binary_search_by_key(100_i32,
                                [](const sus::Tuple<i32, i32>& pair) -> i32 {
                                  return pair.at<1u>();
                                }),
        sus::result::err(13_usize).construct<usize>());
    auto r = sm.binary_search_by_key(
        1_i32,
        [](const sus::Tuple<i32, i32>& pair) -> i32 { return pair.at<1u>(); });
    EXPECT_TRUE("1..=4"_r.contains(sus::move(r).unwrap()));
  }
}

TEST(Slice, Chunks) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_slice();
  auto sm = v.as_mut_slice();
  static_assert(std::same_as<decltype(s.chunks(3u)), decltype(sm.chunks(3u))>);

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::Slice<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::Slice<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(10u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(10u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(13u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(13u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(20u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(20u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(5u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(5u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(7u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks(7u);
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

TEST(Slice, ChunksMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_mut_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks_mut(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::SliceMut<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::SliceMut<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(10u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(10u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(13u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(13u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(20u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(20u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_mut(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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

TEST(Slice, ChunksExact) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks_exact(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::Slice<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::Slice<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(10u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(10u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(13u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(13u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(20u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(20u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(5u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(5u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(7u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.chunks_exact(7u);
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

TEST(Slice, ChunksExactMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_mut_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks_exact_mut(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::SliceMut<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::SliceMut<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(10u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(10u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(13u);
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(13u);
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(20u);
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(20u);
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.chunks_exact_mut(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[7u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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

TEST(Slice, SplitAt) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  sus::Slice<i32> s = v.as_slice();

  {
    // Empty left.
    auto [a, b] = s.split_at(0u);
    static_assert(std::same_as<decltype(a), sus::Slice<i32>>);
    static_assert(std::same_as<decltype(b), sus::Slice<i32>>);
    EXPECT_EQ(a.len(), 0u);
    EXPECT_EQ(b.len(), 10u);
    EXPECT_EQ(b.as_ptr(), &v[0u]);
  }
  {
    // Empty Right.
    auto [a, b] = s.split_at(10u);
    EXPECT_EQ(a.len(), 10u);
    EXPECT_EQ(b.len(), 0u);
    EXPECT_EQ(a.as_ptr(), &v[0u]);
  }
  {
    // Middle.
    auto [a, b] = s.split_at(6u);
    EXPECT_EQ(a.len(), 6u);
    EXPECT_EQ(b.len(), 4u);
    EXPECT_EQ(a.as_ptr(), &v[0u]);
    EXPECT_EQ(b.as_ptr(), &v[6u]);
  }
}

TEST(SliceDeathTest, SplitAtOutOfBounds) {
#if GTEST_HAS_DEATH_TEST
  auto v = sus::Vec<i32>(0, 1, 2);
  EXPECT_DEATH(
      {
        auto t = v.as_slice().split_at(4u);
        EXPECT_EQ(t.at<0>(), t.at<1>());
      },
      "");
  EXPECT_DEATH(
      {
        auto t = v.as_slice().split_at(usize::MAX);
        EXPECT_EQ(t.at<0>(), t.at<1>());
      },
      "");
#endif
}

TEST(Slice, SplitAtUnchecked) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  sus::Slice<i32> s = v.as_slice();

  {
    // Empty left.
    auto [a, b] = s.split_at_unchecked(unsafe_fn, 0u);
    static_assert(std::same_as<decltype(a), sus::Slice<i32>>);
    static_assert(std::same_as<decltype(b), sus::Slice<i32>>);
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

TEST(SliceMut, SplitAtMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  sus::SliceMut<i32> s = v.as_mut_slice();

  {
    // Empty left.
    auto [a, b] = s.split_at_mut(0u);
    static_assert(std::same_as<decltype(a), sus::SliceMut<i32>>);
    static_assert(std::same_as<decltype(b), sus::SliceMut<i32>>);
    EXPECT_EQ(a.len(), 0u);
    EXPECT_EQ(b.len(), 10u);
    EXPECT_EQ(b.as_ptr(), &v[0u]);
  }
  {
    // Empty Right.
    auto [a, b] = s.split_at_mut(10u);
    EXPECT_EQ(a.len(), 10u);
    EXPECT_EQ(b.len(), 0u);
    EXPECT_EQ(a.as_ptr(), &v[0u]);
  }
  {
    // Middle.
    auto [a, b] = s.split_at_mut(6u);
    EXPECT_EQ(a.len(), 6u);
    EXPECT_EQ(b.len(), 4u);
    EXPECT_EQ(a.as_ptr(), &v[0u]);
    EXPECT_EQ(b.as_ptr(), &v[6u]);
  }
}

TEST(SliceMutDeathTest, SplitAtOutOfBounds) {
#if GTEST_HAS_DEATH_TEST
  auto v = sus::Vec<i32>(0, 1, 2);
  EXPECT_DEATH(
      {
        auto t = v.as_mut_slice().split_at(4u);
        EXPECT_EQ(t.at<0>(), t.at<1>());
      },
      "");
  EXPECT_DEATH(
      {
        auto t = v.as_mut_slice().split_at(usize::MAX);
        EXPECT_EQ(t.at<0>(), t.at<1>());
      },
      "");
#endif
}
TEST(SliceMut, SplitAtMutUnchecked) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  sus::SliceMut<i32> s = v.as_mut_slice();

  {
    // Empty left.
    auto [a, b] = s.split_at_mut_unchecked(unsafe_fn, 0u);
    static_assert(std::same_as<decltype(a), sus::SliceMut<i32>>);
    static_assert(std::same_as<decltype(b), sus::SliceMut<i32>>);
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

TEST(Slice, ConcatSlices) {
  static_assert(sus::collections::Concat<const Slice<i32>>);
  static_assert(sus::collections::Concat<Slice<i32>>);
  static_assert(sus::collections::Concat<const SliceMut<i32>>);
  static_assert(sus::collections::Concat<SliceMut<i32>>);

  Vec<i32> v1 = sus::vec(1, 2, 3, 4);
  Vec<i32> v2 = sus::vec(5, 6);
  Vec<i32> v3 = sus::vec(7, 8, 9);
  {
    Vec<Slice<i32>> vs = sus::vec(v1.as_slice(), v2.as_slice(), v3.as_slice());
    Slice<Slice<i32>> s = vs.as_slice();
    auto c = s.concat();
    static_assert(std::same_as<decltype(c), Vec<i32>>);
    EXPECT_EQ(c, Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9));
  }
  {
    Vec<SliceMut<i32>> vs =
        sus::vec(v1.as_mut_slice(), v2.as_mut_slice(), v3.as_mut_slice());
    SliceMut<SliceMut<i32>> s = vs.as_mut_slice();
    auto c = s.concat();
    static_assert(std::same_as<decltype(c), Vec<i32>>);
    EXPECT_EQ(c, Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9));
  }
}

TEST(Slice, ConcatExample) {
  i32 a1[] = {1, 2}, a2[] = {3, 4};
  Slice<i32> as[] = {Slice<i32>::from(a1), Slice<i32>::from(a2)};
  Vec<i32> v = Slice<Slice<i32>>::from(as).concat();
  sus::check(v == Slice<i32>::from({1, 2, 3, 4}));
}

TEST(Slice, JoinSlices) {
  static_assert(sus::collections::Join<const Slice<i32>, const Slice<i32>>);
  static_assert(sus::collections::Join<const Slice<i32>, Slice<i32>>);
  static_assert(sus::collections::Join<const Slice<i32>, i32>);
  static_assert(sus::collections::Join<Slice<i32>, const Slice<i32>>);
  static_assert(sus::collections::Join<Slice<i32>, Slice<i32>>);
  static_assert(sus::collections::Join<Slice<i32>, i32>);
  static_assert(
      sus::collections::Join<const SliceMut<i32>, const SliceMut<i32>>);
  static_assert(sus::collections::Join<const SliceMut<i32>, SliceMut<i32>>);
  static_assert(sus::collections::Join<const SliceMut<i32>, i32>);
  static_assert(sus::collections::Join<SliceMut<i32>, const SliceMut<i32>>);
  static_assert(sus::collections::Join<SliceMut<i32>, SliceMut<i32>>);
  static_assert(sus::collections::Join<SliceMut<i32>, i32>);
  // The separator can be converted.
  static_assert(sus::collections::Join<SliceMut<i32>, int>);

  Vec<i32> v1 = sus::vec(1, 2, 3, 4);
  Vec<i32> v2 = sus::vec(5, 6);
  Vec<i32> v3 = sus::vec(7, 8, 9);
  Vec<i32> vsep = sus::vec(98, 99);
  {
    Vec<Slice<i32>> vs = sus::vec(v1.as_slice(), v2.as_slice(), v3.as_slice());
    Slice<Slice<i32>> s = vs.as_slice();
    auto c = s.join(99);
    static_assert(std::same_as<decltype(c), Vec<i32>>);
    EXPECT_EQ(c, Vec<i32>(1, 2, 3, 4, 99, 5, 6, 99, 7, 8, 9));

    auto c2 = s.join(vsep);
    static_assert(std::same_as<decltype(c2), Vec<i32>>);
    EXPECT_EQ(c2, Vec<i32>(1, 2, 3, 4, 98, 99, 5, 6, 98, 99, 7, 8, 9));
  }
  {
    Vec<SliceMut<i32>> vs =
        sus::vec(v1.as_mut_slice(), v2.as_mut_slice(), v3.as_mut_slice());
    SliceMut<SliceMut<i32>> s = vs.as_mut_slice();
    auto c = s.join(99);
    static_assert(std::same_as<decltype(c), Vec<i32>>);
    EXPECT_EQ(c, Vec<i32>(1, 2, 3, 4, 99, 5, 6, 99, 7, 8, 9));

    auto c2 = s.join(vsep);
    static_assert(std::same_as<decltype(c2), Vec<i32>>);
    EXPECT_EQ(c2, Vec<i32>(1, 2, 3, 4, 98, 99, 5, 6, 98, 99, 7, 8, 9));
  }
}

TEST(Slice, JoinExample) {
  i32 a1[] = {1, 2}, a2[] = {3, 4}, asep[] = {10, 11, 12};
  Slice<i32> as[] = {Slice<i32>::from(a1), Slice<i32>::from(a2)};

  // Join slices with a slice between.
  Vec<i32> v = Slice<Slice<i32>>::from(as).join(Slice<i32>::from(asep));
  sus::check(v == sus::Vec<i32>(1, 2, 10, 11, 12, 3, 4));

  // Join slices with a single item between.
  Vec<i32> v2 = Slice<Slice<i32>>::from(as).join(99);
  sus::check(v2 == sus::Vec<i32>(1, 2, 99, 3, 4));
}

TEST(SliceMut, ConvertsToSlice) {
  Vec<i32> v = sus::vec(1, 2, 3, 4);
  SliceMut<i32> sm = v.as_mut_slice();
  const SliceMut<i32> csm = v.as_mut_slice();
  // Explicit construction.
  {
    [[maybe_unused]] Slice<i32> s(v.as_mut_slice());
    [[maybe_unused]] Slice<i32> s2(sm);
    [[maybe_unused]] Slice<i32> s3(csm);
  }
  // Implicit construction.
  {
    [[maybe_unused]] Slice<i32> s = v.as_mut_slice();
    [[maybe_unused]] Slice<i32> s2 = sm;
    [[maybe_unused]] Slice<i32> s3 = csm;
  }
  // Function calls.
  {
    [](Slice<i32>) {}(v.as_mut_slice());
    [](Slice<i32>) {}(sm);
    [](Slice<i32>) {}(csm);
  }
  // References.
  {
    [[maybe_unused]] const Slice<i32>& s = v.as_mut_slice();
    [[maybe_unused]] const Slice<i32>& s2 = sm;
    [[maybe_unused]] const Slice<i32>& s3 = csm;
    [[maybe_unused]] Slice<i32>& s4 = sm;
    [[maybe_unused]] Slice<i32>&& s5 = sus::move(sm);
  }
}

TEST(Slice, Contains) {
  Vec<i32> v1 = sus::vec(1, 2, 3, 4);
  auto s = v1.as_slice();
  EXPECT_EQ(s.contains(0), false);
  EXPECT_EQ(s.contains(1), true);
  EXPECT_EQ(s.contains(2), true);
  EXPECT_EQ(s.contains(3), true);
  EXPECT_EQ(s.contains(4), true);
  EXPECT_EQ(s.contains(5), false);
}

TEST(Slice, CopyFromSlice) {
  Vec<i32> v1 = sus::vec(1, 2, 3, 4);
  Vec<i32> v2 = sus::vec(5, 6, 7, 8);
  // Same slice, non overlapping.
  v1["0..2"_r].copy_from_slice(v1["2..4"_r]);
  EXPECT_EQ(v1[0u], 3);
  EXPECT_EQ(v1[1u], 4);
  EXPECT_EQ(v1[2u], 3);
  EXPECT_EQ(v1[3u], 4);
  // Different slice.
  v1["0..2"_r].copy_from_slice(v2["2..4"_r]);
  EXPECT_EQ(v1[0u], 7);
  EXPECT_EQ(v1[1u], 8);
  EXPECT_EQ(v1[2u], 3);
  EXPECT_EQ(v1[3u], 4);

  // The source slice was untouched.
  EXPECT_EQ(v2[0u], 5);
  EXPECT_EQ(v2[1u], 6);
  EXPECT_EQ(v2[2u], 7);
  EXPECT_EQ(v2[3u], 8);

  // Constexpr.
  constexpr auto x = []() constexpr {
    i32 i[] = {1, 2, 3, 4};
    auto s = SliceMut<i32>::from(i);
    auto [s1, s2] = s.split_at_mut_unchecked(unsafe_fn, 2u);
    s1.copy_from_slice(s2);
    return s1[0u];
  }();
  EXPECT_EQ(x, 3);
}

TEST(SliceDeathTest, CopyFromSliceChecks) {
  Vec<i32> v1 = sus::vec(1, 2, 3, 4);
#if GTEST_HAS_DEATH_TEST
  // Overlapping.
  EXPECT_DEATH(v1["0..2"_r].copy_from_slice(v1["1..4"_r]), "");
  // Different sizes.
  EXPECT_DEATH(v1["0..1"_r].copy_from_slice(v1["1..4"_r]), "");
  EXPECT_DEATH(v1["0..2"_r].copy_from_slice(v1["3..4"_r]), "");
#endif
}

TEST(Slice, CopyFromSliceUnchecked) {
  Vec<i32> v1 = sus::vec(1, 2, 3, 4);
  Vec<i32> v2 = sus::vec(5, 6, 7, 8);
  // Same slice, non overlapping.
  EXPECT_EQ(v1["0..2"_r].len(), 2u);
  v1["0..2"_r].copy_from_slice_unchecked(unsafe_fn, v1["2..4"_r]);
  EXPECT_EQ(v1[0u], 3);
  EXPECT_EQ(v1[1u], 4);
  EXPECT_EQ(v1[2u], 3);
  EXPECT_EQ(v1[3u], 4);
  // Different slice.
  v1["0..2"_r].copy_from_slice_unchecked(unsafe_fn, v2["2..4"_r]);
  EXPECT_EQ(v1[0u], 7);
  EXPECT_EQ(v1[1u], 8);
  EXPECT_EQ(v1[2u], 3);
  EXPECT_EQ(v1[3u], 4);

  // Overlapping oops. Does not check(), but we can't verify what the outcome
  // would be.
  v1["0..2"_r].copy_from_slice_unchecked(unsafe_fn, v2["1..3"_r]);

  // The source slice was untouched.
  EXPECT_EQ(v2[0u], 5);
  EXPECT_EQ(v2[1u], 6);
  EXPECT_EQ(v2[2u], 7);
  EXPECT_EQ(v2[3u], 8);

  // Constexpr.
  constexpr auto x = []() constexpr {
    i32 i[] = {1, 2, 3, 4};
    auto s = SliceMut<i32>::from(i);
    auto [s1, s2] = s.split_at_mut_unchecked(unsafe_fn, 2u);
    s1.copy_from_slice_unchecked(unsafe_fn, s2);
    return s1[0u];
  }();
  EXPECT_EQ(x, 3);
}

TEST(Slice, CloneFromSlice) {
  struct Cloner {
    i32 i;

    constexpr Cloner(i32 i) : i(i) {}

    Cloner(Cloner&&) = default;
    Cloner& operator=(Cloner&&) = default;

    constexpr Cloner clone() const noexcept { return Cloner(i * 10); }
  };

  Vec<Cloner> v1 = sus::vec(1_i32, 2_i32, 3_i32, 4_i32);
  Vec<Cloner> v2 = sus::vec(6_i32, 7_i32, 8_i32, 9_i32);
  v1["0..2"_r].clone_from_slice(v2["2..4"_r]);
  EXPECT_EQ(v1[0u].i, 80);
  EXPECT_EQ(v1[1u].i, 90);
  EXPECT_EQ(v1[2u].i, 3);
  EXPECT_EQ(v1[3u].i, 4);

  // The source slice was untouched.
  EXPECT_EQ(v2[0u].i, 6);
  EXPECT_EQ(v2[1u].i, 7);
  EXPECT_EQ(v2[2u].i, 8);
  EXPECT_EQ(v2[3u].i, 9);

  // Constexpr.
  constexpr auto x = []() constexpr {
    Cloner i[] = {1_i32, 2_i32, 3_i32, 4_i32};
    auto s = SliceMut<Cloner>::from(i);
    auto [s1, s2] = s.split_at_mut_unchecked(unsafe_fn, 2u);
    s1.clone_from_slice(s2);
    return s1[0u].i;
  }();
  EXPECT_EQ(x, 30);
}

TEST(SliceDeathTest, CloneFromSliceChecks) {
  Vec<i32> v1 = sus::vec(1, 2, 3, 4);
#if GTEST_HAS_DEATH_TEST
  // Different sizes.
  EXPECT_DEATH(v1["0..1"_r].clone_from_slice(v1["1..4"_r]), "");
  EXPECT_DEATH(v1["0..2"_r].clone_from_slice(v1["3..4"_r]), "");
#endif
}

TEST(Slice, EndsWith) {
  Vec<i32> v1 = sus::vec(1, 2, 3, 4);
  EXPECT_TRUE(v1[".."_r].ends_with(v1["4..4"_r]));
  EXPECT_TRUE(v1[".."_r].ends_with(v1["3..4"_r]));
  EXPECT_TRUE(v1[".."_r].ends_with(v1["2..4"_r]));
  EXPECT_TRUE(v1[".."_r].ends_with(v1["1..4"_r]));
  EXPECT_TRUE(v1[".."_r].ends_with(v1["0..4"_r]));
  EXPECT_FALSE(v1[".."_r].ends_with(v1["2..3"_r]));
  EXPECT_FALSE(v1[".."_r].ends_with(v1["1..3"_r]));
  EXPECT_FALSE(v1[".."_r].ends_with(v1["0..3"_r]));
}

TEST(Slice, Eq) {
  struct NotEq {};
  static_assert(!sus::ops::Eq<NotEq>);

  static_assert(sus::ops::Eq<Slice<int>>);
  static_assert(!sus::ops::Eq<Slice<int>, Slice<NotEq>>);
  static_assert(!sus::ops::Eq<Slice<NotEq>>);

  static_assert(sus::ops::Eq<SliceMut<int>>);
  static_assert(!sus::ops::Eq<SliceMut<int>, SliceMut<NotEq>>);
  static_assert(!sus::ops::Eq<SliceMut<NotEq>>);

  Vec<i32> v1 = sus::vec(1, 2, 3, 4);
  EXPECT_EQ(v1.as_slice(), v1.as_slice());
  EXPECT_EQ(v1.as_mut_slice(), v1.as_slice());
  EXPECT_EQ(v1.as_slice(), v1.as_mut_slice());
  EXPECT_EQ(v1.as_mut_slice(), v1.as_mut_slice());
  Vec<i32> v2 = sus::vec(1, 2, 3, 4);
  EXPECT_EQ(v1.as_slice(), v2.as_slice());
  EXPECT_EQ(v1.as_mut_slice(), v2.as_slice());
  EXPECT_EQ(v1.as_slice(), v2.as_mut_slice());
  EXPECT_EQ(v1.as_mut_slice(), v2.as_mut_slice());
  static_assert(std::same_as<decltype(v1[".."_r]), SliceMut<i32>>);
  EXPECT_EQ(v1[".."_r], v2[".."_r]);
  EXPECT_EQ(v1["1.."_r], v2["1.."_r]);
  EXPECT_EQ(v1["1..3"_r], v2["1..3"_r]);
  EXPECT_EQ(v1["1..3"_r].as_slice(), v2["1..3"_r].as_slice());
  v1[3u] += 1;
  EXPECT_EQ(v1["1.."_r], v1["1.."_r]);
  EXPECT_NE(v1["1.."_r], v2["1.."_r]);
}

TEST(SliceMut, Fill) {
  auto v1 = Vec<i32>(1, 2, 3, 4);
  v1["0..2"_r].fill(5);
  EXPECT_EQ(v1[0u], 5);
  EXPECT_EQ(v1[1u], 5);
  EXPECT_EQ(v1[2u], 3);
  EXPECT_EQ(v1[3u], 4);
  v1["1..3"_r].fill(6);
  EXPECT_EQ(v1[0u], 5);
  EXPECT_EQ(v1[1u], 6);
  EXPECT_EQ(v1[2u], 6);
  EXPECT_EQ(v1[3u], 4);
  v1[".."_r].fill(9);
  EXPECT_EQ(v1[0u], 9);
  EXPECT_EQ(v1[1u], 9);
  EXPECT_EQ(v1[2u], 9);
  EXPECT_EQ(v1[3u], 9);

  struct S {
    i32 i;

    S& operator=(const S& s) {
      i = s.i + 1;
      return *this;
    }
  };
  static_assert(sus::mem::Clone<S>);
  auto v2 = Vec<S>(S(1), S(10));
  // Fill from a value in the Vec. If the element being modified also changes
  // the input, then v2[1] will be 3 instead of 2.
  v2[".."_r].fill(v2[0u]);
  EXPECT_EQ(v2[0u].i, 2);
  EXPECT_EQ(v2[1u].i, 2);
}

TEST(SliceMut, FillWith) {
  auto f = [i = 6_i32]() mutable { return ::sus::mem::replace(i, i + 1); };
  auto v1 = Vec<i32>(1, 2, 3, 4);
  v1[".."_r].fill_with(f);
  EXPECT_EQ(v1[0u], 6);
  EXPECT_EQ(v1[1u], 7);
  EXPECT_EQ(v1[2u], 8);
  EXPECT_EQ(v1[3u], 9);
  v1["2..4"_r].fill_with(f);
  EXPECT_EQ(v1[0u], 6);
  EXPECT_EQ(v1[1u], 7);
  EXPECT_EQ(v1[2u], 10);
  EXPECT_EQ(v1[3u], 11);
}

TEST(SliceMut, FillWithDefault) {
  auto v1 = Vec<i32>(1, 2, 3, 4);
  v1["2..4"_r].fill_with_default();
  EXPECT_EQ(v1[0u], 1);
  EXPECT_EQ(v1[1u], 2);
  EXPECT_EQ(v1[2u], 0);
  EXPECT_EQ(v1[3u], 0);
  v1[".."_r].fill_with_default();
  EXPECT_EQ(v1[0u], 0);
  EXPECT_EQ(v1[1u], 0);
  EXPECT_EQ(v1[2u], 0);
  EXPECT_EQ(v1[3u], 0);
}

TEST(Slice, First) {
  const auto v1 = Vec<i32>(1, 2, 3, 4);
  EXPECT_EQ(&v1[".."_r].first().unwrap(), v1.as_ptr());
  EXPECT_EQ(&v1["1.."_r].first().unwrap(), v1["1.."_r].as_ptr());
  EXPECT_EQ(v1["1..1"_r].first(), sus::None);

  NoCopyMove n[] = {NoCopyMove(), NoCopyMove()};
  auto s = Slice<NoCopyMove>::from(n);
  EXPECT_EQ(&s[".."_r].first().unwrap(), &n[0]);
  EXPECT_EQ(&s["1.."_r].first().unwrap(), &n[1]);
  EXPECT_EQ(s["2..1"_r].first(), sus::None);

  static_assert(
      std::same_as<sus::Option<const NoCopyMove&>, decltype(s.first())>);
}

TEST(SliceMut, FirstMut) {
  auto v1 = Vec<i32>(1, 2, 3, 4);
  EXPECT_EQ(&v1[".."_r].first_mut().unwrap(), v1.as_ptr());
  EXPECT_EQ(&v1["1.."_r].first_mut().unwrap(), v1["1.."_r].as_ptr());
  EXPECT_EQ(v1["1..1"_r].first_mut(), sus::None);

  NoCopyMove n[] = {NoCopyMove(), NoCopyMove()};
  auto s = SliceMut<NoCopyMove>::from(n);
  EXPECT_EQ(&s[".."_r].first_mut().unwrap(), &n[0]);
  EXPECT_EQ(&s["1.."_r].first_mut().unwrap(), &n[1]);
  EXPECT_EQ(s["2..1"_r].first_mut(), sus::None);

  static_assert(
      std::same_as<sus::Option<const NoCopyMove&>, decltype(s.first())>);
  static_assert(
      std::same_as<sus::Option<NoCopyMove&>, decltype(s.first_mut())>);
}

TEST(Slice, Last) {
  const auto v1 = Vec<i32>(1, 2, 3, 4);
  EXPECT_EQ(&v1[".."_r].last().unwrap(), v1.as_ptr() + 3u);
  EXPECT_EQ(&v1["..2"_r].last().unwrap(), v1.as_ptr() + 1u);
  EXPECT_EQ(v1["1..1"_r].last(), sus::None);

  NoCopyMove n[] = {NoCopyMove(), NoCopyMove(), NoCopyMove()};
  auto s = Slice<NoCopyMove>::from(n);
  EXPECT_EQ(&s[".."_r].last().unwrap(), &n[2]);
  EXPECT_EQ(&s["..2"_r].last().unwrap(), &n[1]);
  EXPECT_EQ(s["2..1"_r].last(), sus::None);

  static_assert(
      std::same_as<sus::Option<const NoCopyMove&>, decltype(s.last())>);
}

TEST(SliceMut, LastMut) {
  auto v1 = Vec<i32>(1, 2, 3, 4);
  EXPECT_EQ(&v1[".."_r].last_mut().unwrap(), v1.as_ptr() + 3u);
  EXPECT_EQ(&v1["..2"_r].last_mut().unwrap(), v1.as_ptr() + 1u);
  EXPECT_EQ(v1["1..1"_r].last_mut(), sus::None);

  NoCopyMove n[] = {NoCopyMove(), NoCopyMove(), NoCopyMove()};
  auto s = SliceMut<NoCopyMove>::from(n);
  EXPECT_EQ(&s[".."_r].last_mut().unwrap(), &n[2]);
  EXPECT_EQ(&s["..2"_r].last_mut().unwrap(), &n[1]);
  EXPECT_EQ(s["2..1"_r].last_mut(), sus::None);

  static_assert(
      std::same_as<sus::Option<const NoCopyMove&>, decltype(s.last())>);
  static_assert(std::same_as<sus::Option<NoCopyMove&>, decltype(s.last_mut())>);
}

TEST(Slice, Repeat) {
  {
    auto v1 = Vec<i32>(1, 2);
    auto v2 = v1.as_slice().repeat(0u);
    EXPECT_EQ(v2, sus::Vec<i32>());
  }
  {
    auto v1 = Vec<i32>(1);
    auto v2 = v1.as_slice().repeat(1u);
    EXPECT_EQ(v2, sus::vec(1).construct<i32>());
  }
  {
    auto v1 = Vec<i32>(1, 2, 3, 4, 5);
    auto v2 = v1.as_slice().repeat(1u);
    EXPECT_EQ(v2, sus::vec(1, 2, 3, 4, 5).construct<i32>());
  }
  {
    auto v1 = Vec<i32>(1, 2);
    auto v2 = v1.as_slice().repeat(3u);
    EXPECT_EQ(v2, sus::vec(1, 2, 1, 2, 1, 2).construct<i32>());
  }
  {
    auto v1 = Vec<i32>(1, 2);
    auto v2 = v1.as_slice().repeat(27u);
    EXPECT_EQ(v2, sus::vec(1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
                           1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
                           1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2)
                      .construct<i32>());
  }
  {
    auto v1 = Vec<i32>(1, 2, 3, 4, 5);
    auto v2 = v1.as_slice().repeat(13u);
    EXPECT_EQ(v2, sus::vec(1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3,
                           4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1,
                           2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4,
                           5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5)
                      .construct<i32>());
  }
}

TEST(Slice, RChunks) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_slice();
  auto sm = v.as_mut_slice();
  static_assert(std::same_as<decltype(s.chunks(3u)), decltype(sm.chunks(3u))>);

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::Slice<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::Slice<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(10u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(10u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(13u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(13u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(20u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(20u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
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
    // Chunk size divides into len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[3u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 3u);
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
    // Chunk size doesn't divide into len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 3u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[3u]);

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

TEST(Slice, RChunksMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_mut_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks_mut(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::SliceMut<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::SliceMut<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(10u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(10u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(13u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(13u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(20u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(20u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
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
    // Chunk size divides into len: next_back().
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(5u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[3u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 3u);
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
    // Chunk size doesn't divide into len: next_back().
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_mut(7u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 3u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[3u]);

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

TEST(Slice, RChunksExact) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks_exact(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::Slice<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::Slice<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(10u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(10u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(13u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(13u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(20u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(20u);
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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
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
    // Chunk size divides into len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

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
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[3u]);

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
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  {
    // Chunk size doesn't divide into len: next_back().
    sus::iter::Iterator<sus::Slice<i32>> auto it = s.rchunks_exact(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::Slice<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[3u]);

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
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
}

TEST(Slice, RChunksExactMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  auto s = v.as_mut_slice();

  {
    // Check the iterator type.
    decltype(auto) it = s.chunks_exact_mut(3u);
    static_assert(sus::iter::Iterator<decltype(it), sus::SliceMut<i32>>);
    static_assert(
        sus::iter::DoubleEndedIterator<decltype(it), sus::SliceMut<i32>>);
    static_assert(sus::mem::Copy<decltype(it)>);
    static_assert(sus::mem::Clone<decltype(it)>);
    static_assert(sus::mem::Move<decltype(it)>);
  }
  {
    // Chunk size == len: next().
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(10u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(10u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(13u);
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(13u);
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(20u);
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(20u);
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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next().unwrap();
    EXPECT_EQ(n.len(), 5u);
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
    // Chunk size divides into len: next_back().
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(5u);
    EXPECT_EQ(it.remainder().len(), 0u);
    {
      EXPECT_EQ(it.exact_size_hint(), 2u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 2u);
      EXPECT_EQ(upper, sus::some(2u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[0u]);

    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 5u);
    EXPECT_EQ(n.as_ptr(), &v[5u]);

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
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[3u]);

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
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
  {
    // Chunk size doesn't divide into len: next_back().
    sus::iter::Iterator<sus::SliceMut<i32>> auto it = s.rchunks_exact_mut(7u);
    // Remainder is available immediately.
    EXPECT_EQ(it.remainder().len(), 3u);
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
    {
      EXPECT_EQ(it.exact_size_hint(), 1u);
      auto [lower, upper] = it.size_hint();
      EXPECT_EQ(lower, 1u);
      EXPECT_EQ(upper, sus::some(1u));
    }
    sus::SliceMut<i32> n = it.next_back().unwrap();
    EXPECT_EQ(n.len(), 7u);
    EXPECT_EQ(n.as_ptr(), &v[3u]);

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
    EXPECT_EQ(it.remainder().as_ptr(), &v[0u]);
  }
}

TEST(SliceMut, Reverse_Example) {
  auto forward = sus::Vec<i32>(1, 2, 3);
  auto sf = forward[".."_r];
  auto backward = sus::Vec<i32>(3, 2, 1);
  auto sb = backward[".."_r];
  sf.reverse();
  sus::check(sf == sb);
}

TEST(SliceMut, Reverse) {
  // Empty.
  {
    auto v = sus::Vec<i32>();
    auto s = v.as_mut_slice();
    s.reverse();
    auto expected = Vec<i32>();
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Odd.
  {
    auto v = sus::Vec<i32>(1, 2, 3);
    auto s = v[".."_r];
    s.reverse();
    auto expected = Vec<i32>(3, 2, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Even.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v[".."_r];
    s.reverse();
    auto expected = Vec<i32>(4, 3, 2, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Larger?
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    auto s = v[".."_r];
    s.reverse();
    auto expected = Vec<i32>(11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
}

TEST(SliceMut, RotateLeft) {
  // Empty. Nothing to do.
  {
    auto v = sus::Vec<i32>();
    auto s = v.as_mut_slice();
    s.rotate_left(0u);
    auto expected = Vec<i32>();
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Single. Nothing to do.
  {
    auto v = sus::Vec<i32>(4);
    auto s = v.as_mut_slice();
    s.rotate_left(0u);
    auto expected = Vec<i32>(4);
    EXPECT_EQ(s, expected.as_mut_slice());
    s.rotate_left(1u);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Two.
  {
    auto v = sus::Vec<i32>(1, 2);
    auto s = v.as_mut_slice();
    s.rotate_left(0u);
    auto expected = Vec<i32>(1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2);
    auto s = v.as_mut_slice();
    s.rotate_left(1u);
    auto expected = Vec<i32>(2, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2);
    auto s = v.as_mut_slice();
    s.rotate_left(2u);
    auto expected = Vec<i32>(1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Three.
  {
    auto v = sus::Vec<i32>(1, 2, 3);
    auto s = v.as_mut_slice();
    s.rotate_left(0u);
    auto expected = Vec<i32>(1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3);
    auto s = v.as_mut_slice();
    s.rotate_left(1u);
    auto expected = Vec<i32>(2, 3, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3);
    auto s = v.as_mut_slice();
    s.rotate_left(2u);
    auto expected = Vec<i32>(3, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3);
    auto s = v.as_mut_slice();
    s.rotate_left(3u);
    auto expected = Vec<i32>(1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Four.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_left(0u);
    auto expected = Vec<i32>(1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_left(1u);
    auto expected = Vec<i32>(2, 3, 4, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_left(2u);
    auto expected = Vec<i32>(3, 4, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_left(3u);
    auto expected = Vec<i32>(4, 1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_left(4u);
    auto expected = Vec<i32>(1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Five.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_left(0u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_left(1u);
    auto expected = Vec<i32>(2, 3, 4, 5, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_left(2u);
    auto expected = Vec<i32>(3, 4, 5, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_left(3u);
    auto expected = Vec<i32>(4, 5, 1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_left(4u);
    auto expected = Vec<i32>(5, 1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_left(5u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Larger even size.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_left(0u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();

    s.rotate_left(1u);
    auto expected = Vec<i32>(2, 3, 4, 5, 6, 7, 8, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_left(2u);
    auto expected = Vec<i32>(3, 4, 5, 6, 7, 8, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_left(3u);
    auto expected = Vec<i32>(4, 5, 6, 7, 8, 1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_left(4u);
    auto expected = Vec<i32>(5, 6, 7, 8, 1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_left(5u);
    auto expected = Vec<i32>(6, 7, 8, 1, 2, 3, 4, 5);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_left(6u);
    auto expected = Vec<i32>(7, 8, 1, 2, 3, 4, 5, 6);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_left(7u);
    auto expected = Vec<i32>(8, 1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_left(8u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Larger odd size.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_left(0u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();

    s.rotate_left(1u);
    auto expected = Vec<i32>(2, 3, 4, 5, 6, 7, 8, 9, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_left(2u);
    auto expected = Vec<i32>(3, 4, 5, 6, 7, 8, 9, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_left(3u);
    auto expected = Vec<i32>(4, 5, 6, 7, 8, 9, 1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_left(4u);
    auto expected = Vec<i32>(5, 6, 7, 8, 9, 1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_left(5u);
    auto expected = Vec<i32>(6, 7, 8, 9, 1, 2, 3, 4, 5);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_left(6u);
    auto expected = Vec<i32>(7, 8, 9, 1, 2, 3, 4, 5, 6);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_left(7u);
    auto expected = Vec<i32>(8, 9, 1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_left(8u);
    auto expected = Vec<i32>(9, 1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_left(9u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
}

TEST(SliceMutDeathTest, RotateLeftOutOfBounds) {
  // Empty.
  {
    auto v = sus::Vec<i32>();
    EXPECT_DEATH(v.as_mut_slice().rotate_left(1u), "");
  }
  // Single.
  {
    auto v = sus::Vec<i32>(4);
    EXPECT_DEATH(v.as_mut_slice().rotate_left(2u), "");
  }
  // Odd size.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_DEATH(v.as_mut_slice().rotate_left(10u), "");
  }
  // Odd size.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    EXPECT_DEATH(v.as_mut_slice().rotate_left(11u), "");
  }
}

TEST(SliceMut, RotateRight) {
  // Empty. Nothing to do.
  {
    auto v = sus::Vec<i32>();
    auto s = v.as_mut_slice();
    s.rotate_right(0u);
    auto expected = Vec<i32>();
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Single. Nothing to do.
  {
    auto v = sus::Vec<i32>(4);
    auto s = v.as_mut_slice();
    s.rotate_right(0u);
    auto expected = Vec<i32>(4);
    EXPECT_EQ(s, expected.as_mut_slice());
    s.rotate_right(1u);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Two.
  {
    auto v = sus::Vec<i32>(1, 2);
    auto s = v.as_mut_slice();
    s.rotate_right(0u);
    auto expected = Vec<i32>(1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2);
    auto s = v.as_mut_slice();
    s.rotate_right(1u);
    auto expected = Vec<i32>(2, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2);
    auto s = v.as_mut_slice();
    s.rotate_right(2u);
    auto expected = Vec<i32>(1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Three.
  {
    auto v = sus::Vec<i32>(1, 2, 3);
    auto s = v.as_mut_slice();
    s.rotate_right(0u);
    auto expected = Vec<i32>(1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3);
    auto s = v.as_mut_slice();
    s.rotate_right(1u);
    auto expected = Vec<i32>(3, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3);
    auto s = v.as_mut_slice();
    s.rotate_right(2u);
    auto expected = Vec<i32>(2, 3, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3);
    auto s = v.as_mut_slice();
    s.rotate_right(3u);
    auto expected = Vec<i32>(1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Four.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_right(0u);
    auto expected = Vec<i32>(1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_right(1u);
    auto expected = Vec<i32>(4, 1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_right(2u);
    auto expected = Vec<i32>(3, 4, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_right(3u);
    auto expected = Vec<i32>(2, 3, 4, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4);
    auto s = v.as_mut_slice();
    s.rotate_right(4u);
    auto expected = Vec<i32>(1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Five.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_right(0u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_right(1u);
    auto expected = Vec<i32>(5, 1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_right(2u);
    auto expected = Vec<i32>(4, 5, 1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_right(3u);
    auto expected = Vec<i32>(3, 4, 5, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_right(4u);
    auto expected = Vec<i32>(2, 3, 4, 5, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5);
    auto s = v.as_mut_slice();
    s.rotate_right(5u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Larger even size.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_right(0u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();

    s.rotate_right(1u);
    auto expected = Vec<i32>(8, 1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_right(2u);
    auto expected = Vec<i32>(7, 8, 1, 2, 3, 4, 5, 6);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_right(3u);
    auto expected = Vec<i32>(6, 7, 8, 1, 2, 3, 4, 5);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_right(4u);
    auto expected = Vec<i32>(5, 6, 7, 8, 1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_right(5u);
    auto expected = Vec<i32>(4, 5, 6, 7, 8, 1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_right(6u);
    auto expected = Vec<i32>(3, 4, 5, 6, 7, 8, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_right(7u);
    auto expected = Vec<i32>(2, 3, 4, 5, 6, 7, 8, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    auto s = v.as_mut_slice();
    s.rotate_right(8u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  // Larger odd size.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_right(0u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();

    s.rotate_right(1u);
    auto expected = Vec<i32>(9, 1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_right(2u);
    auto expected = Vec<i32>(8, 9, 1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_right(3u);
    auto expected = Vec<i32>(7, 8, 9, 1, 2, 3, 4, 5, 6);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_right(4u);
    auto expected = Vec<i32>(6, 7, 8, 9, 1, 2, 3, 4, 5);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_right(5u);
    auto expected = Vec<i32>(5, 6, 7, 8, 9, 1, 2, 3, 4);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_right(6u);
    auto expected = Vec<i32>(4, 5, 6, 7, 8, 9, 1, 2, 3);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_right(7u);
    auto expected = Vec<i32>(3, 4, 5, 6, 7, 8, 9, 1, 2);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_right(8u);
    auto expected = Vec<i32>(2, 3, 4, 5, 6, 7, 8, 9, 1);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    auto s = v.as_mut_slice();
    s.rotate_right(9u);
    auto expected = Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_EQ(s, expected.as_mut_slice());
  }
}

TEST(SliceMutDeathTest, RotateRightOutOfBounds) {
  // Empty.
  {
    auto v = sus::Vec<i32>();
    EXPECT_DEATH(v.as_mut_slice().rotate_right(1u), "");
  }
  // Single.
  {
    auto v = sus::Vec<i32>(4);
    EXPECT_DEATH(v.as_mut_slice().rotate_right(2u), "");
  }
  // Odd size.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_DEATH(v.as_mut_slice().rotate_right(10u), "");
  }
  // Odd size.
  {
    auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    EXPECT_DEATH(v.as_mut_slice().rotate_right(11u), "");
  }
}

TEST(Slice, Split) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_slice();

  // No match. Front.
  {
    auto it = s.split([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<Slice<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // No match. Back.
  {
    auto it = s.split([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next_back();
    static_assert(std::same_as<decltype(o), sus::Option<Slice<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle. Front.
  {
    auto it = s.split([](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // One match middle. Back.
  {
    auto it = s.split([](const i32& i) { return i == 3; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Edge matches. Front.
  {
    auto it = s.split([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Edge matches. Back.
  {
    auto it = s.split([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Consecutive matches. Front.
  {
    auto it = s.split([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Consecutive matches. Back.
  {
    auto it = s.split([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(SliceMut, SplitMut) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_mut_slice();

  // No match. Front.
  {
    auto it = s.split_mut([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<SliceMut<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // No match. Back.
  {
    auto it = s.split_mut([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next_back();
    static_assert(std::same_as<decltype(o), sus::Option<SliceMut<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle. Front.
  {
    auto it = s.split_mut([](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // One match middle. Back.
  {
    auto it = s.split_mut([](const i32& i) { return i == 3; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Edge matches. Front.
  {
    auto it = s.split_mut([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Edge matches. Back.
  {
    auto it = s.split_mut([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Consecutive matches. Front.
  {
    auto it = s.split_mut([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Consecutive matches. Back.
  {
    auto it = s.split_mut([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(Slice, SplitInclusive) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_slice();

  // No match. Front.
  {
    auto it = s.split_inclusive([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<Slice<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // No match. Back.
  {
    auto it = s.split_inclusive([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next_back();
    static_assert(std::same_as<decltype(o), sus::Option<Slice<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle. Front.
  {
    auto it = s.split_inclusive([](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // One match middle. Back.
  {
    auto it = s.split_inclusive([](const i32& i) { return i == 3; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Edge matches. Front.
  {
    auto it = s.split_inclusive([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Edge matches. Back.
  {
    auto it = s.split_inclusive([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Consecutive matches. Front.
  {
    auto it = s.split_inclusive([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(5));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Consecutive matches. Back.
  {
    auto it = s.split_inclusive([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(5));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(SliceMut, SplitInclusiveMut) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_mut_slice();

  // No match. Front.
  {
    auto it = s.split_inclusive_mut([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<SliceMut<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // No match. Back.
  {
    auto it = s.split_inclusive_mut([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next_back();
    static_assert(std::same_as<decltype(o), sus::Option<SliceMut<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle. Front.
  {
    auto it = s.split_inclusive_mut([](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // One match middle. Back.
  {
    auto it = s.split_inclusive_mut([](const i32& i) { return i == 3; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Edge matches. Front.
  {
    auto it =
        s.split_inclusive_mut([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Edge matches. Back.
  {
    auto it =
        s.split_inclusive_mut([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Consecutive matches. Front.
  {
    auto it =
        s.split_inclusive_mut([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(5));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Consecutive matches. Back.
  {
    auto it =
        s.split_inclusive_mut([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(5));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(Slice, Rsplit) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_slice();

  // No match. Front.
  {
    auto it = s.rsplit([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<Slice<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // No match. Back.
  {
    auto it = s.rsplit([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next_back();
    static_assert(std::same_as<decltype(o), sus::Option<Slice<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle. Front.
  {
    auto it = s.rsplit([](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // One match middle. Back.
  {
    auto it = s.rsplit([](const i32& i) { return i == 3; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Edge matches. Front.
  {
    auto it = s.rsplit([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Edge matches. Back.
  {
    auto it = s.rsplit([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Consecutive matches. Front.
  {
    auto it = s.rsplit([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Consecutive matches. Back.
  {
    auto it = s.rsplit([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(SliceMut, RsplitMut) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_mut_slice();

  // No match. Front.
  {
    auto it = s.rsplit_mut([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<SliceMut<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // No match. Back.
  {
    auto it = s.rsplit_mut([](const i32& i) { return i == -1; });
    decltype(auto) o = it.next_back();
    static_assert(std::same_as<decltype(o), sus::Option<SliceMut<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle. Front.
  {
    auto it = s.rsplit_mut([](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // One match middle. Back.
  {
    auto it = s.rsplit_mut([](const i32& i) { return i == 3; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Edge matches. Front.
  {
    auto it = s.rsplit_mut([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Edge matches. Back.
  {
    auto it = s.rsplit_mut([](const i32& i) { return i == 1 || i == 8; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4, 5, 5, 6, 7, 7, 7));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
  // Consecutive matches. Front.
  {
    auto it = s.rsplit_mut([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    EXPECT_EQ(it.next(), sus::None);
    EXPECT_EQ(it.next_back(), sus::None);
  }
  // Consecutive matches. Back.
  {
    auto it = s.rsplit_mut([](const i32& i) { return i == 1 || i == 5; });
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(2, 2, 3, 4));
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next_back().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next_back(), sus::None);
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(Slice, SplitN) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_slice();

  // No match.
  {
    auto it = s.splitn(1u, [](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<Slice<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle.
  {
    auto it = s.splitn(3u, [](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 1.
  {
    auto it = s.splitn(1u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 2.
  {
    auto it = s.splitn(2u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 3.
  {
    auto it = s.splitn(3u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 4.
  {
    auto it = s.splitn(4u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(SliceMut, SplitNMut) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_mut_slice();

  // No match.
  {
    auto it = s.splitn_mut(1u, [](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<SliceMut<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle.
  {
    auto it = s.splitn_mut(3u, [](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 1.
  {
    auto it = s.splitn_mut(1u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 2.
  {
    auto it = s.splitn_mut(2u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 3.
  {
    auto it = s.splitn_mut(3u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 4.
  {
    auto it = s.splitn_mut(4u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(Slice, RSplitN) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_slice();

  // No match.
  {
    auto it = s.rsplitn(1u, [](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<Slice<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle.
  {
    auto it = s.rsplitn(3u, [](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 1.
  {
    auto it = s.rsplitn(1u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 2.
  {
    auto it = s.rsplitn(2u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 3.
  {
    auto it = s.rsplitn(3u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 4.
  {
    auto it = s.rsplitn(4u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(SliceMut, RSplitNMut) {
  auto v = sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8);
  auto s = v.as_mut_slice();

  // No match.
  {
    auto it = s.rsplitn_mut(1u, [](const i32& i) { return i == -1; });
    decltype(auto) o = it.next();
    static_assert(std::same_as<decltype(o), sus::Option<SliceMut<i32>>>);
    EXPECT_EQ(sus::move(o).unwrap(), s);
    EXPECT_EQ(it.next(), sus::None);
  }
  // One match middle.
  {
    auto it = s.rsplitn_mut(3u, [](const i32& i) { return i == 3; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(4, 5, 5, 6, 7, 7, 7, 8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 1.
  {
    auto it = s.rsplitn_mut(1u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 7, 8));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 2.
  {
    auto it = s.rsplitn_mut(2u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7, 7));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 3.
  {
    auto it = s.rsplitn_mut(3u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6, 7));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
  // Limit matches to 4.
  {
    auto it = s.rsplitn_mut(4u, [](const i32& i) { return i == 7; });
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(8));
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>());
    }
    {
      auto o = it.next().unwrap();
      EXPECT_EQ(o, sus::Vec<i32>(1, 2, 2, 3, 4, 5, 5, 6));
    }
    EXPECT_EQ(it.next(), sus::None);
  }
}

TEST(SliceMut, Swap) {
  {
    sus::Vec<i32> v = sus::vec(1, 2, 3, 4, 5, 6);
    SliceMut<i32> s = v.as_mut_slice();
    s.swap(0u, 0u);
    sus::Vec<i32> expected = sus::vec(1, 2, 3, 4, 5, 6);
    EXPECT_EQ(s, expected);
  }
  {
    sus::Vec<i32> v = sus::vec(1, 2, 3, 4, 5, 6);
    SliceMut<i32> s = v.as_mut_slice();
    s.swap(0u, 1u);
    sus::Vec<i32> expected = sus::vec(2, 1, 3, 4, 5, 6);
    EXPECT_EQ(s, expected);
  }
  {
    sus::Vec<i32> v = sus::vec(1, 2, 3, 4, 5, 6);
    SliceMut<i32> s = v.as_mut_slice();
    s.swap(3u, 5u);
    sus::Vec<i32> expected = sus::vec(1, 2, 3, 6, 5, 4);
    EXPECT_EQ(s, expected);
  }
}

TEST(SliceMut, SwapNonOverlapping) {
  {
    sus::Vec<i32> v = sus::vec(1, 2, 3, 4, 5, 6);
    SliceMut<i32> s = v.as_mut_slice();
    s.swap_nonoverlapping(unsafe_fn, 0u, 1u);
    sus::Vec<i32> expected = sus::vec(2, 1, 3, 4, 5, 6);
    EXPECT_EQ(s, expected);
  }
  {
    sus::Vec<i32> v = sus::vec(1, 2, 3, 4, 5, 6);
    SliceMut<i32> s = v.as_mut_slice();
    s.swap_nonoverlapping(unsafe_fn, 3u, 5u);
    sus::Vec<i32> expected = sus::vec(1, 2, 3, 6, 5, 4);
    EXPECT_EQ(s, expected);
  }
}

TEST(SliceMut, SwapUnchecked) {
  {
    sus::Vec<i32> v = sus::vec(1, 2, 3, 4, 5, 6);
    SliceMut<i32> s = v.as_mut_slice();
    s.swap_unchecked(unsafe_fn, 0u, 0u);
    sus::Vec<i32> expected = sus::vec(1, 2, 3, 4, 5, 6);
    EXPECT_EQ(s, expected);
  }
  {
    sus::Vec<i32> v = sus::vec(1, 2, 3, 4, 5, 6);
    SliceMut<i32> s = v.as_mut_slice();
    s.swap_unchecked(unsafe_fn, 0u, 1u);
    sus::Vec<i32> expected = sus::vec(2, 1, 3, 4, 5, 6);
    EXPECT_EQ(s, expected);
  }
  {
    sus::Vec<i32> v = sus::vec(1, 2, 3, 4, 5, 6);
    SliceMut<i32> s = v.as_mut_slice();
    s.swap_unchecked(unsafe_fn, 3u, 5u);
    sus::Vec<i32> expected = sus::vec(1, 2, 3, 6, 5, 4);
    EXPECT_EQ(s, expected);
  }
}

TEST(SliceMut, SwapWithSlice) {
  {
    sus::Vec<i32> v1 = sus::vec(1, 2, 3, 4, 5, 6);
    SliceMut<i32> s1 = v1.as_mut_slice();
    sus::Vec<i32> v2 = sus::vec(101, 102, 103, 104, 105, 106);
    SliceMut<i32> s2 = v2.as_mut_slice();

    s1.swap_with_slice(s2);
    sus::Vec<i32> expected1 = sus::vec(1, 2, 3, 4, 5, 6);
    sus::Vec<i32> expected2 = sus::vec(101, 102, 103, 104, 105, 106);
    EXPECT_EQ(s1, expected2);
    EXPECT_EQ(s2, expected1);
    s1.swap_with_slice(s2);
    EXPECT_EQ(s1, expected1);
    EXPECT_EQ(s2, expected2);

    s1["2..4"_r].swap_with_slice(s2["3..5"_r]);
    sus::Vec<i32> expected3 = sus::vec(1, 2, 104, 105, 5, 6);
    sus::Vec<i32> expected4 = sus::vec(101, 102, 103, 3, 4, 106);
    EXPECT_EQ(s1, expected3);
    EXPECT_EQ(s2, expected4);
  }
}

TEST(Slice, SplitFirst) {
  sus::Vec<i32> v = sus::vec(0, 1, 2);
  sus::Slice<i32> s = v.as_slice();

  auto&& [first, rest] = s.split_first().unwrap();
  static_assert(std::same_as<decltype(first), const i32&>);
  static_assert(std::same_as<decltype(rest), Slice<i32>>);
  EXPECT_EQ(&first, &v[0u]);
  EXPECT_EQ(first, 0);
  EXPECT_EQ(rest.len(), 2_usize);

  auto&& [first2, rest2] = rest.split_first().unwrap();
  EXPECT_EQ(&first2, &v[1u]);
  EXPECT_EQ(first2, 1);
  EXPECT_EQ(rest2.len(), 1_usize);

  auto&& [first3, rest3] = rest2.split_first().unwrap();
  EXPECT_EQ(&first3, &v[2u]);
  EXPECT_EQ(first3, 2);
  EXPECT_EQ(rest3.len(), 0_usize);

  EXPECT_EQ(rest3.split_first(), sus::None);
}

TEST(SliceMut, SplitFirstMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2);
  sus::SliceMut<i32> s = v.as_mut_slice();

  auto&& [first, rest] = s.split_first_mut().unwrap();
  static_assert(std::same_as<decltype(first), i32&>);
  static_assert(std::same_as<decltype(rest), SliceMut<i32>>);
  EXPECT_EQ(&first, &v[0u]);
  EXPECT_EQ(first, 0);
  EXPECT_EQ(rest.len(), 2_usize);

  auto&& [first2, rest2] = rest.split_first_mut().unwrap();
  EXPECT_EQ(&first2, &v[1u]);
  EXPECT_EQ(first2, 1);
  EXPECT_EQ(rest2.len(), 1_usize);

  auto&& [first3, rest3] = rest2.split_first_mut().unwrap();
  EXPECT_EQ(&first3, &v[2u]);
  EXPECT_EQ(first3, 2);
  EXPECT_EQ(rest3.len(), 0_usize);

  EXPECT_EQ(rest3.split_first_mut(), sus::None);
}

TEST(Slice, SplitLast) {
  sus::Vec<i32> v = sus::vec(0, 1, 2);
  sus::Slice<i32> s = v.as_slice();

  auto&& [last, rest] = s.split_last().unwrap();
  static_assert(std::same_as<decltype(last), const i32&>);
  static_assert(std::same_as<decltype(rest), Slice<i32>>);
  EXPECT_EQ(&last, &v[2u]);
  EXPECT_EQ(last, 2);
  EXPECT_EQ(rest.len(), 2_usize);

  auto&& [last2, rest2] = rest.split_last().unwrap();
  EXPECT_EQ(&last2, &v[1u]);
  EXPECT_EQ(last2, 1);
  EXPECT_EQ(rest2.len(), 1_usize);

  auto&& [last3, rest3] = rest2.split_last().unwrap();
  EXPECT_EQ(&last3, &v[0u]);
  EXPECT_EQ(last3, 0);
  EXPECT_EQ(rest3.len(), 0_usize);

  EXPECT_EQ(rest3.split_last(), sus::None);
}

TEST(SliceMut, SplitLastMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2);
  sus::SliceMut<i32> s = v.as_mut_slice();

  auto&& [last, rest] = s.split_last_mut().unwrap();
  static_assert(std::same_as<decltype(last), i32&>);
  static_assert(std::same_as<decltype(rest), SliceMut<i32>>);
  EXPECT_EQ(&last, &v[2u]);
  EXPECT_EQ(last, 2);
  EXPECT_EQ(rest.len(), 2_usize);

  auto&& [last2, rest2] = rest.split_last_mut().unwrap();
  EXPECT_EQ(&last2, &v[1u]);
  EXPECT_EQ(last2, 1);
  EXPECT_EQ(rest2.len(), 1_usize);

  auto&& [last3, rest3] = rest2.split_last_mut().unwrap();
  EXPECT_EQ(&last3, &v[0u]);
  EXPECT_EQ(last3, 0);
  EXPECT_EQ(rest3.len(), 0_usize);

  EXPECT_EQ(rest3.split_last_mut(), sus::None);
}

TEST(Slice, StartsWith) {
  sus::Vec<i32> v = sus::vec(1, 2, 2, 3, 4, 5);
  sus::Slice<i32> s = v.as_slice();
  // Comparing with itself.
  EXPECT_EQ(s.starts_with(s[".."_r]), true);
  // Comparing with a prefix.
  EXPECT_EQ(s.starts_with(s["..4"_r]), true);
  EXPECT_EQ(s.starts_with(s["..3"_r]), true);
  EXPECT_EQ(s.starts_with(s["..2"_r]), true);
  EXPECT_EQ(s.starts_with(s["..1"_r]), true);
  // Comparing with empty Slice.
  EXPECT_EQ(s.starts_with(s["..0"_r]), true);
  // Comparing with a SliceMut.
  EXPECT_EQ(s.starts_with(v["..4"_r]), true);
  // Comparing with a non-prefix.
  EXPECT_EQ(s.starts_with(s["1..4"_r]), false);
  // Comparing with a prefix + extra content.
  EXPECT_EQ(s["0..4"_r].starts_with(s), false);
}

TEST(Slice, StripPrefix) {
  sus::Vec<i32> v = sus::vec(1, 2, 2, 3, 4, 5);
  sus::Slice<i32> s = v.as_slice();

  static_assert(std::same_as<decltype(s.strip_prefix(Slice<i32>())),
                             sus::Option<sus::Slice<i32>>>);

  EXPECT_EQ(s.strip_prefix(Slice<i32>()), sus::Some);
  EXPECT_EQ(s.strip_prefix(s), sus::Some);
  EXPECT_EQ(s.strip_prefix(v["..5"_r]), sus::Some);
  EXPECT_EQ(s.strip_prefix(v["1..5"_r]), sus::None);
  sus::Vec<i32> more = sus::vec(1, 2, 2, 3, 4, 5, 6);
  EXPECT_EQ(s.strip_prefix(more), sus::None);

  EXPECT_EQ(s.strip_prefix(Slice<i32>()).unwrap(),
            sus::Vec<i32>(1, 2, 2, 3, 4, 5));
  EXPECT_EQ(s.strip_prefix(v["..2"_r]).unwrap(), sus::Vec<i32>(2, 3, 4, 5));
  EXPECT_EQ(s.strip_prefix(v["..5"_r]).unwrap(), sus::Vec<i32>(5));
  EXPECT_EQ(s.strip_prefix(v[".."_r]).unwrap(), sus::Vec<i32>());
}

TEST(SliceMut, StripPrefixMut) {
  sus::Vec<i32> v = sus::vec(1, 2, 2, 3, 4, 5);
  sus::SliceMut<i32> s = v.as_mut_slice();

  static_assert(std::same_as<decltype(s.strip_prefix_mut(Slice<i32>())),
                             sus::Option<sus::SliceMut<i32>>>);

  EXPECT_EQ(s.strip_prefix_mut(Slice<i32>()), sus::Some);
  EXPECT_EQ(s.strip_prefix_mut(s), sus::Some);
  EXPECT_EQ(s.strip_prefix_mut(v["..5"_r]), sus::Some);
  EXPECT_EQ(s.strip_prefix_mut(v["1..5"_r]), sus::None);
  sus::Vec<i32> more = sus::vec(1, 2, 2, 3, 4, 5, 6);
  EXPECT_EQ(s.strip_prefix_mut(more), sus::None);

  EXPECT_EQ(s.strip_prefix_mut(Slice<i32>()).unwrap(),
            sus::Vec<i32>(1, 2, 2, 3, 4, 5));
  EXPECT_EQ(s.strip_prefix_mut(v["..2"_r]).unwrap(), sus::Vec<i32>(2, 3, 4, 5));
  EXPECT_EQ(s.strip_prefix_mut(v["..5"_r]).unwrap(), sus::Vec<i32>(5));
  EXPECT_EQ(s.strip_prefix_mut(v[".."_r]).unwrap(), sus::Vec<i32>());
}

TEST(Slice, StripSuffix) {
  sus::Vec<i32> v = sus::vec(1, 2, 2, 3, 4, 5);
  sus::Slice<i32> s = v.as_slice();

  static_assert(std::same_as<decltype(s.strip_suffix(Slice<i32>())),
                             sus::Option<sus::Slice<i32>>>);

  EXPECT_EQ(s.strip_suffix(Slice<i32>()), sus::Some);
  EXPECT_EQ(s.strip_suffix(s), sus::Some);
  EXPECT_EQ(s.strip_suffix(v["1.."_r]), sus::Some);
  EXPECT_EQ(s.strip_suffix(v["1..5"_r]), sus::None);
  sus::Vec<i32> more = sus::vec(1, 2, 2, 3, 4, 5, 6);
  EXPECT_EQ(s.strip_suffix(more), sus::None);

  EXPECT_EQ(s.strip_suffix(Slice<i32>()).unwrap(),
            sus::Vec<i32>(1, 2, 2, 3, 4, 5));
  EXPECT_EQ(s.strip_suffix(v["4.."_r]).unwrap(), sus::Vec<i32>(1, 2, 2, 3));
  EXPECT_EQ(s.strip_suffix(v["1.."_r]).unwrap(), sus::Vec<i32>(1));
  EXPECT_EQ(s.strip_suffix(v[".."_r]).unwrap(), sus::Vec<i32>());
}

TEST(SliceMut, StripSuffixMut) {
  sus::Vec<i32> v = sus::vec(1, 2, 2, 3, 4, 5);
  sus::SliceMut<i32> s = v.as_mut_slice();

  static_assert(std::same_as<decltype(s.strip_suffix_mut(Slice<i32>())),
                             sus::Option<sus::SliceMut<i32>>>);

  EXPECT_EQ(s.strip_suffix_mut(Slice<i32>()), sus::Some);
  EXPECT_EQ(s.strip_suffix_mut(s), sus::Some);
  EXPECT_EQ(s.strip_suffix_mut(v["1.."_r]), sus::Some);
  EXPECT_EQ(s.strip_suffix_mut(v["1..5"_r]), sus::None);
  sus::Vec<i32> more = sus::vec(1, 2, 2, 3, 4, 5, 6);
  EXPECT_EQ(s.strip_suffix_mut(more), sus::None);

  EXPECT_EQ(s.strip_suffix_mut(Slice<i32>()).unwrap(),
            sus::Vec<i32>(1, 2, 2, 3, 4, 5));
  EXPECT_EQ(s.strip_suffix_mut(v["4.."_r]).unwrap(), sus::Vec<i32>(1, 2, 2, 3));
  EXPECT_EQ(s.strip_suffix_mut(v["1.."_r]).unwrap(), sus::Vec<i32>(1));
  EXPECT_EQ(s.strip_suffix_mut(v[".."_r]).unwrap(), sus::Vec<i32>());
}

TEST(Slice, Windows) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7);
  sus::Slice<i32> s = v.as_slice();

  // Larger than the slice size.
  EXPECT_EQ(s.windows(9u).next(), sus::None);

  // Equal to the slice size.
  EXPECT_EQ(s.windows(8u).next().unwrap(), s);

  auto w1 = s.windows(1u);
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(0));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(1));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(2));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(3));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(4));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(5));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(6));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(7));
  EXPECT_EQ(w1.next(), sus::None);

  auto w2 = s.windows(2u);
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(0, 1));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(1, 2));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(2, 3));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(3, 4));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(4, 5));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(5, 6));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(6, 7));
  EXPECT_EQ(w2.next(), sus::None);

  auto w3 = s.windows(3u);
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(0, 1, 2));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(1, 2, 3));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(2, 3, 4));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(3, 4, 5));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(4, 5, 6));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(5, 6, 7));
  EXPECT_EQ(w3.next(), sus::None);

  auto w7 = s.windows(7u);
  EXPECT_EQ(w7.next().unwrap(), sus::Vec<i32>(0, 1, 2, 3, 4, 5, 6));
  EXPECT_EQ(w7.next().unwrap(), sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7));
  EXPECT_EQ(w7.next(), sus::None);
}

TEST(SliceMut, WindowsMut) {
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7);
  sus::SliceMut<i32> s = v.as_mut_slice();

  // Larger than the slice size.
  EXPECT_EQ(s.windows_mut(9u).next(), sus::None);

  // Equal to the slice size.
  EXPECT_EQ(s.windows_mut(8u).next().unwrap(), s);

  auto w1 = s.windows_mut(1u);
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(0));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(1));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(2));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(3));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(4));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(5));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(6));
  EXPECT_EQ(w1.next().unwrap(), sus::Vec<i32>(7));
  EXPECT_EQ(w1.next(), sus::None);

  auto w2 = s.windows_mut(2u);
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(0, 1));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(1, 2));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(2, 3));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(3, 4));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(4, 5));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(5, 6));
  EXPECT_EQ(w2.next().unwrap(), sus::Vec<i32>(6, 7));
  EXPECT_EQ(w2.next(), sus::None);

  auto w3 = s.windows_mut(3u);
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(0, 1, 2));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(1, 2, 3));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(2, 3, 4));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(3, 4, 5));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(4, 5, 6));
  EXPECT_EQ(w3.next().unwrap(), sus::Vec<i32>(5, 6, 7));
  EXPECT_EQ(w3.next(), sus::None);

  auto w7 = s.windows_mut(7u);
  EXPECT_EQ(w7.next().unwrap(), sus::Vec<i32>(0, 1, 2, 3, 4, 5, 6));
  EXPECT_EQ(w7.next().unwrap(), sus::Vec<i32>(1, 2, 3, 4, 5, 6, 7));
  EXPECT_EQ(w7.next(), sus::None);
}

TEST(Slice, fmt) {
  auto v = Vec<i32>(1, 2, 3, 4, 5);
  EXPECT_EQ(fmt::format("{}", v.as_slice()), "[1, 2, 3, 4, 5]");
  EXPECT_EQ(fmt::format("{:02}", v.as_slice()), "[01, 02, 03, 04, 05]");

  auto empty = Vec<i32>();
  EXPECT_EQ(fmt::format("{}", empty.as_slice()), "[]");
  EXPECT_EQ(fmt::format("{:02}", empty.as_slice()), "[]");

  struct NoFormat {
    i32 a = 0x16ae3cf2;
  };
  static_assert(!fmt::is_formattable<NoFormat, char>::value);

  auto vn = Vec<NoFormat>(NoFormat(), NoFormat(0xf00d));
  EXPECT_EQ(fmt::format("{}", vn), "[f2-3c-ae-16, 0d-f0-00-00]");
}

TEST(Slice, Stream) {
  std::stringstream s;
  auto v = Vec<i32>(1, 2, 3, 4, 5);
  s << v.as_slice();
  EXPECT_EQ(s.str(), "[1, 2, 3, 4, 5]");
}

TEST(Slice, GTest) {
  auto v = Vec<i32>(1, 2, 3, 4, 5);
  EXPECT_EQ(testing::PrintToString(v.as_slice()), "[1, 2, 3, 4, 5]");
}

TEST(SliceMut, fmt) {
  auto v = Vec<i32>(1, 2, 3, 4, 5);
  EXPECT_EQ(fmt::format("{}", v.as_mut_slice()), "[1, 2, 3, 4, 5]");
  EXPECT_EQ(fmt::format("{:02}", v.as_mut_slice()), "[01, 02, 03, 04, 05]");

  auto empty = Vec<i32>();
  EXPECT_EQ(fmt::format("{}", empty.as_mut_slice()), "[]");
  EXPECT_EQ(fmt::format("{:02}", empty.as_mut_slice()), "[]");

  struct NoFormat {
    i32 a = 0x16ae3cf2;
  };
  static_assert(!fmt::is_formattable<NoFormat, char>::value);

  auto vn = Vec<NoFormat>(NoFormat(), NoFormat(0xf00d));
  EXPECT_EQ(fmt::format("{}", vn), "[f2-3c-ae-16, 0d-f0-00-00]");
}

TEST(SliceMut, Stream) {
  std::stringstream s;
  auto v = Vec<i32>(1, 2, 3, 4, 5);
  s << v.as_mut_slice();
  EXPECT_EQ(s.str(), "[1, 2, 3, 4, 5]");
}

TEST(SliceMut, GTest) {
  auto v = Vec<i32>(1, 2, 3, 4, 5);
  EXPECT_EQ(testing::PrintToString(v.as_mut_slice()), "[1, 2, 3, 4, 5]");
}

TEST(Slice, DropIteratorInvalidationTracking) {
  auto v = Vec<i32>(1, 2, 3, 4, 5);
  auto s = v.as_slice();
  s.drop_iterator_invalidation_tracking(::sus::marker::unsafe_fn);
  auto it = sus::move(s).into_iter();
  v.clear();  // Does not track the invalidation and panic. `it` is now invalid.
}

TEST(SliceMut, DropIteratorInvalidationTracking) {
  auto v = Vec<i32>(1, 2, 3, 4, 5);
  auto s = v.as_mut_slice();
  s.drop_iterator_invalidation_tracking(::sus::marker::unsafe_fn);
  auto it = sus::move(s).into_iter();
  v.clear();  // Does not track the invalidation and panic. `it` is now invalid.
}

TEST(SliceDeathTest, ChunksInvalidation) {
#if GTEST_HAS_DEATH_TEST
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  EXPECT_DEATH(
      {
        auto it = v.chunks(2u);
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.chunks_mut(2u);
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.chunks_exact(2u);
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.chunks_exact_mut(2u);
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.rchunks(2u);
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.rchunks_mut(2u);
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.rchunks_exact(2u);
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.rchunks_exact_mut(2u);
        v.push(10);
        ensure_use(&it);
      },
      "");
#endif
}

TEST(SliceDeathTest, SplitInvalidation) {
#if GTEST_HAS_DEATH_TEST
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  EXPECT_DEATH(
      {
        auto it = v.split([](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.split_mut([](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.splitn(1u, [](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.splitn_mut(1u, [](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.rsplit([](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.rsplit_mut([](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.rsplitn(1u, [](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.rsplitn_mut(1u, [](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.split_inclusive([](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.split_inclusive_mut([](auto) { return true; });
        v.push(10);
        ensure_use(&it);
      },
      "");
#endif
}

TEST(SliceDeathTest, WindowsInvalidation) {
#if GTEST_HAS_DEATH_TEST
  sus::Vec<i32> v = sus::vec(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
  EXPECT_DEATH(
      {
        auto it = v.windows(3u);
        v.push(10);
        ensure_use(&it);
      },
      "");
  EXPECT_DEATH(
      {
        auto it = v.windows_mut(3u);
        v.push(10);
        ensure_use(&it);
      },
      "");
#endif
}

}  // namespace
