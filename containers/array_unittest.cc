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

#include "containers/array.h"

#include <type_traits>

#include "marker/unsafe.h"
#include "mem/__private/relocate.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

using ::sus::containers::Array;
using ::sus::mem::__private::relocate_array_by_memcpy_v;
using ::sus::mem::__private::relocate_one_by_memcpy_v;

namespace {

struct TriviallyRelocatable {
  int i;
};

static_assert(!std::is_trivially_constructible_v<Array<int, 2>>, "");
static_assert(!std::is_trivial_v<Array<int, 2>>, "");
static_assert(!std::is_aggregate_v<Array<int, 2>>, "");
static_assert(std::is_standard_layout_v<Array<int, 2>>, "");

// TODO: This covers a trivially relocatable type, but what about the rest?
// (like Option unit tests.)
namespace trivially_relocatable {
using T = TriviallyRelocatable;
static_assert(std::is_move_constructible_v<Array<T, 2>>, "");
static_assert(std::is_move_assignable_v<Array<T, 2>>, "");
static_assert(std::is_trivially_destructible_v<Array<T, 2>>, "");
static_assert(std::is_trivially_move_constructible_v<Array<T, 2>>, "");
static_assert(std::is_trivially_move_assignable_v<Array<T, 2>>, "");
static_assert(std::is_nothrow_swappable_v<Array<T, 2>>, "");
static_assert(std::is_trivially_copy_constructible_v<Array<T, 2>>, "");
static_assert(std::is_trivially_copy_assignable_v<Array<T, 2>>, "");
static_assert(!std::is_constructible_v<Array<T, 2>, T&&>, "");
static_assert(!std::is_assignable_v<Array<T, 2>, T&&>, "");
static_assert(!std::is_constructible_v<Array<T, 2>, const T&>, "");
static_assert(!std::is_assignable_v<Array<T, 2>, const T&>, "");
static_assert(!std::is_constructible_v<Array<T, 2>, T>, "");
static_assert(!std::is_assignable_v<Array<T, 2>, T>, "");
static_assert(std::is_nothrow_destructible_v<Array<T, 2>>, "");
static_assert(relocate_one_by_memcpy_v<Array<T, 2>>, "");
static_assert(relocate_array_by_memcpy_v<Array<T, 2>>, "");
}  // namespace trivially_relocatable

struct NonAggregate {
  ~NonAggregate() {}
};

TEST(Array, WithDefault) {
  auto a = Array<int, 5>::with_default();
  static_assert(sizeof(a) == sizeof(int[5]), "");
  for (size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(a.get(i), 0);
  }

  static_assert(sizeof(Array<int, 5>::with_default()) == sizeof(int[5]), "");
}

TEST(Array, Zero) {
  auto a = Array<int, 0>::with_default();
  static_assert(sizeof(a) == 1, "");
  static_assert(sizeof(Array<int, 0>::with_default()) == 1, "");
}

TEST(Array, WithUninitialized) {
  static_assert(
      sizeof(Array<int, 5>::with_uninitialized(unsafe_fn)) == sizeof(int[5]),
      "");
}

TEST(Array, WithInitializer) {
  auto a = Array<int, 5>::with_initializer([i = 1]() mutable { return i++; });
  static_assert(sizeof(a) == sizeof(int[5]), "");
  for (size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(a.get(i), i + 1);
  }

  struct NotTriviallyDefaultConstructible {
    NotTriviallyDefaultConstructible() {}
    NotTriviallyDefaultConstructible(int i) : i(i) {}
    int i;
  };
  static_assert(!std::is_trivially_default_constructible_v<
                    NotTriviallyDefaultConstructible>,
                "");
  auto b = Array<NotTriviallyDefaultConstructible, 5>::with_initializer(
      [i = 1]() mutable -> NotTriviallyDefaultConstructible { return {i++}; });
  static_assert(sizeof(b) == sizeof(NotTriviallyDefaultConstructible[5]), "");
  for (size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(b.get(i).i, i + 1);
  }

  auto lvalue = [i = 1]() mutable { return i++; };
  auto c = Array<int, 5>::with_initializer(lvalue);
  static_assert(sizeof(c) == sizeof(int[5]), "");
  for (size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(c.get(i), i + 1);
  }

  static_assert(sizeof(Array<int, 5>::with_initializer([]() { return 1; })) ==
                    sizeof(int[5]),
                "");
}

TEST(Array, WithValue) {
  auto a = Array<int, 5>::with_value(3);
  static_assert(sizeof(a) == sizeof(int[5]), "");
  for (size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(a.get(i), 3);
  }
}

}  // namespace
