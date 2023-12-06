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

#include "sus/ptr/swap.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/collections/array.h"
#include "sus/mem/replace.h"
#include "sus/prelude.h"

namespace {

TEST(PtrSwapNonOverlapping, SmallSizedType_PowTwoSized) {
  struct S {
    explicit S(u16 i) : a(i), b(i + 1_u16) {}
    bool operator==(const S&) const = default;
    u16 a, b;
  };
  static_assert(alignof(S) <= alignof(S*));
  static_assert(sus::mem::size_of<S>() <= sus::mem::size_of<S*>() * 2);
  static_assert(usize(sus::mem::size_of<S>()).is_power_of_two());
  static_assert(sus::mem::TriviallyRelocatable<S>);

  auto a = sus::Array<S, 100>::with_initializer(
      [i = 0_u8]() mutable { return S(sus::mem::replace(i, i + 1_u8)); });
  auto b = sus::Array<S, 100>::with_initializer(
      [i = 100_u8]() mutable { return S(sus::mem::replace(i, i + 1_u8)); });
  sus::ptr::swap_nonoverlapping(unsafe_fn, a.as_mut_ptr(), b.as_mut_ptr(), 100u);

  for (usize i : "0..100"_r) {
    SCOPED_TRACE(size_t{i});
    EXPECT_EQ(a[i], S(u16::try_from(100u + i).unwrap()));
    EXPECT_EQ(b[i], S(u16::try_from(0u + i).unwrap()));
  }
}

TEST(PtrSwapNonOverlapping, SmallSizedType_NonPowTwoSized) {
  struct S {
    explicit S(u16 i) : a(i), b(i + 1_u16), c(i + 2_u16) {}
    bool operator==(const S&) const = default;
    u16 a, b, c;
  };
  static_assert(alignof(S) <= alignof(S*));
  static_assert(sus::mem::size_of<S>() <= sus::mem::size_of<S*>() * 2);
  static_assert(!usize(sus::mem::size_of<S>()).is_power_of_two());
  static_assert(sus::mem::TriviallyRelocatable<S>);

  auto a = sus::Array<S, 100>::with_initializer(
      [i = 0_u8]() mutable { return S(sus::mem::replace(i, i + 1_u8)); });
  auto b = sus::Array<S, 100>::with_initializer(
      [i = 100_u8]() mutable { return S(sus::mem::replace(i, i + 1_u8)); });
  sus::ptr::swap_nonoverlapping(unsafe_fn, a.as_mut_ptr(), b.as_mut_ptr(), 100u);

  for (usize i : "0..100"_r) {
    SCOPED_TRACE(i);
    EXPECT_EQ(a[i], S(u16::try_from(100u + i).unwrap()));
    EXPECT_EQ(b[i], S(u16::try_from(0u + i).unwrap()));
  }
}

TEST(PtrSwapNonOverlapping, LargeSizedType_PtrAlign_PtrMultipleSize_Trivial) {
  struct S {
    explicit S(usize i) : a(i), b(i + 1u), c(i + 2u) {}
    bool operator==(const S&) const = default;
    usize a, b, c;
  };
  static_assert(alignof(S) == alignof(S*));
  static_assert(sus::mem::size_of<S>() > sus::mem::size_of<S*>() * 2u);
  static_assert(sus::mem::size_of<S>() % sus::mem::size_of<S*>() == 0u);
  static_assert(sus::mem::TriviallyRelocatable<S>);

  auto a = sus::Array<S, 100>::with_initializer(
      [i = 0_u8]() mutable { return S(sus::mem::replace(i, i + 1_u8)); });
  auto b = sus::Array<S, 100>::with_initializer(
      [i = 100_u8]() mutable { return S(sus::mem::replace(i, i + 1_u8)); });
  sus::ptr::swap_nonoverlapping(unsafe_fn, a.as_mut_ptr(), b.as_mut_ptr(), 100u);

  for (usize i : "0..100"_r) {
    SCOPED_TRACE(i);
    EXPECT_EQ(a[i], S(100u + i));
    EXPECT_EQ(b[i], S(0u + i));
  }
}

TEST(PtrSwapNonOverlapping,
     LargeSizedType_PtrAlign_PtrMultipleSize_NonTrivial) {
  struct S {
    explicit S(usize i) : a(i), b(i + 1u), c(i + 2u) {}
    S(const S& rhs) : a(rhs.a), b(rhs.b), c(rhs.c) {}
    S& operator=(const S&) = default;
    bool operator==(const S&) const = default;
    usize a, b, c;
  };
  static_assert(alignof(S) == alignof(S*));
  static_assert(sus::mem::size_of<S>() > sus::mem::size_of<S*>() * 2u);
  static_assert(sus::mem::size_of<S>() % sus::mem::size_of<S*>() == 0u);
  static_assert(!sus::mem::TriviallyRelocatable<S>);

  auto a = sus::Array<S, 100>::with_initializer(
      [i = 0_u8]() mutable { return S(sus::mem::replace(i, i + 1_u8)); });
  auto b = sus::Array<S, 100>::with_initializer(
      [i = 100_u8]() mutable { return S(sus::mem::replace(i, i + 1_u8)); });
  sus::ptr::swap_nonoverlapping(unsafe_fn, a.as_mut_ptr(), b.as_mut_ptr(), 100u);

  for (usize i : "0..100"_r) {
    SCOPED_TRACE(i);
    EXPECT_EQ(a[i], S(100u + i));
    EXPECT_EQ(b[i], S(0u + i));
  }
}
}  // namespace
