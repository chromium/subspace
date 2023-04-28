// Copyright 2023 Google LLC
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

#include "subspace/ptr/swap.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/containers/array.h"
#include "subspace/mem/replace.h"
#include "subspace/prelude.h"

namespace {

TEST(PtrSwapNonOverlapping, SmallSizedType_PowTwoSized) {
  struct S {
    explicit S(u16 i) : a(i), b(i + 1) {}
    bool operator==(const S&) const = default;
    u16 a, b;
  };
  static_assert(alignof(S) <= alignof(S*));
  static_assert(sus::mem::size_of<S>() <= sus::mem::size_of<S*>() * 2);
  static_assert(usize(sus::mem::size_of<S>()).is_power_of_two());
  static_assert(sus::mem::relocate_by_memcpy<S>);

  auto a = sus::Array<S, 100>::with_initializer(
      [i = 0_u8]() mutable { return S(sus::mem::replace(mref(i), i + 1)); });
  auto b = sus::Array<S, 100>::with_initializer(
      [i = 100_u8]() mutable { return S(sus::mem::replace(mref(i), i + 1)); });
  sus::ptr::swap_nonoverlapping(unsafe_fn, a.as_mut_ptr(), b.as_mut_ptr(), 100);

  for (usize i: "0..100"_r) {
    SCOPED_TRACE(size_t{i});
    EXPECT_EQ(a[i], S(u16::from(100 + i)));
    EXPECT_EQ(b[i], S(u16::from(0 + i)));
  }
}

TEST(PtrSwapNonOverlapping, SmallSizedType_NonPowTwoSized) {
  struct S {
    explicit S(u16 i) : a(i), b(i + 1), c(i + 2) {}
    bool operator==(const S&) const = default;
    u16 a, b, c;
  };
  static_assert(alignof(S) <= alignof(S*));
  static_assert(sus::mem::size_of<S>() <= sus::mem::size_of<S*>() * 2);
  static_assert(!usize(sus::mem::size_of<S>()).is_power_of_two());
  static_assert(sus::mem::relocate_by_memcpy<S>);

  auto a = sus::Array<S, 100>::with_initializer(
      [i = 0_u8]() mutable { return S(sus::mem::replace(mref(i), i + 1)); });
  auto b = sus::Array<S, 100>::with_initializer(
      [i = 100_u8]() mutable { return S(sus::mem::replace(mref(i), i + 1)); });
  sus::ptr::swap_nonoverlapping(unsafe_fn, a.as_mut_ptr(), b.as_mut_ptr(), 100);

  for (usize i: "0..100"_r) {
    SCOPED_TRACE(size_t{i});
    EXPECT_EQ(a[i], S(u16::from(100 + i)));
    EXPECT_EQ(b[i], S(u16::from(0 + i)));
  }
}

TEST(PtrSwapNonOverlapping, LargeSizedType_PtrAlign_PtrMultipleSize_Trivial) {
  struct S {
    explicit S(usize i) : a(i), b(i + 1), c(i + 2) {}
    bool operator==(const S&) const = default;
    usize a, b, c;
  };
  static_assert(alignof(S) == alignof(S*));
  static_assert(sus::mem::size_of<S>() > sus::mem::size_of<S*>() * 2);
  static_assert(sus::mem::size_of<S>() % sus::mem::size_of<S*>() == 0);
  static_assert(sus::mem::relocate_by_memcpy<S>);

  auto a = sus::Array<S, 100>::with_initializer(
      [i = 0_u8]() mutable { return S(sus::mem::replace(mref(i), i + 1)); });
  auto b = sus::Array<S, 100>::with_initializer(
      [i = 100_u8]() mutable { return S(sus::mem::replace(mref(i), i + 1)); });
  sus::ptr::swap_nonoverlapping(unsafe_fn, a.as_mut_ptr(), b.as_mut_ptr(), 100);

  for (usize i: "0..100"_r) {
    SCOPED_TRACE(size_t{i});
    EXPECT_EQ(a[i], S(100 + i));
    EXPECT_EQ(b[i], S(0 + i));
  }
}

TEST(PtrSwapNonOverlapping,
     LargeSizedType_PtrAlign_PtrMultipleSize_NonTrivial) {
  struct S {
    explicit S(usize i) : a(i), b(i + 1), c(i + 2) {}
    S(const S& rhs) : a(rhs.a), b(rhs.b), c(rhs.c) {}
    S& operator=(const S&) = default;
    bool operator==(const S&) const = default;
    usize a, b, c;
  };
  static_assert(alignof(S) == alignof(S*));
  static_assert(sus::mem::size_of<S>() > sus::mem::size_of<S*>() * 2);
  static_assert(sus::mem::size_of<S>() % sus::mem::size_of<S*>() == 0);
  static_assert(!sus::mem::relocate_by_memcpy<S>);

  auto a = sus::Array<S, 100>::with_initializer(
      [i = 0_u8]() mutable { return S(sus::mem::replace(mref(i), i + 1)); });
  auto b = sus::Array<S, 100>::with_initializer(
      [i = 100_u8]() mutable { return S(sus::mem::replace(mref(i), i + 1)); });
  sus::ptr::swap_nonoverlapping(unsafe_fn, a.as_mut_ptr(), b.as_mut_ptr(), 100);

  for (usize i: "0..100"_r) {
    SCOPED_TRACE(size_t{i});
    EXPECT_EQ(a[i], S(100 + i));
    EXPECT_EQ(b[i], S(0 + i));
  }
}
}  // namespace
