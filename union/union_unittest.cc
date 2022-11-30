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

#include "union/union.h"

#include <variant>

#include "macros/__private/compiler_bugs.h"
#include "num/types.h"
#include "prelude.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::Union;

enum class Order {
  First,
  Second,
  Third,
};

inline constexpr size_t UamBytes = sus::Tuple<i32>::protects_uam ? 8 : 0;

// The Union's tag can get stashed inside the Tuple, though this doesn't happen
// on MSVC.
static_assert(sizeof(Union<sus_value_types((Order::First, i32, u64))>) ==
              2 * sizeof(u64) + UamBytes + sus_if_msvc_else(sizeof(u64), 0));

TEST(Union, GetTypes) {
  // Single value first, double last.
  {
    auto u = Union<sus_value_types(
        (Order::First, u32), (Order::Second, i8, u64))>::with<Order::First>(3u);
    static_assert(
        std::same_as<decltype(u.get_ref<Order::First>()), const u32&>);
    static_assert(std::same_as<decltype(u.get_ref<Order::Second>()),
                               sus::Tuple<const i8&, const u64&>>);
    static_assert(std::same_as<decltype(u.get_mut<Order::First>()), u32&>);
    static_assert(std::same_as<decltype(u.get_mut<Order::Second>()),
                               sus::Tuple<i8&, u64&>>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::First>()), u32&&>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::Second>()),
                     sus::Tuple<i8, u64>>);
  }
  // Double value first, singke last.
  {
    auto u =
        Union<sus_value_types((Order::First, i8, u64), (Order::Second, u32))>::
            with<Order::First>(sus::Tuple<i8, u64>::with(1_i8, 2_u64));
    static_assert(std::same_as<decltype(u.get_ref<Order::First>()),
                               sus::Tuple<const i8&, const u64&>>);
    static_assert(
        std::same_as<decltype(u.get_ref<Order::Second>()), const u32&>);
    static_assert(std::same_as<decltype(u.get_mut<Order::First>()),
                               sus::Tuple<i8&, u64&>>);
    static_assert(std::same_as<decltype(u.get_mut<Order::Second>()), u32&>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::First>()),
                     sus::Tuple<i8, u64>>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::Second>()),
                     u32&&>);
  }
  // With const refs.
  {
    auto i = 3_u32;
    auto u =
        Union<sus_value_types((Order::First, i8&, const u64&),
                              (Order::Second, u32))>::with<Order::Second>(i);
    static_assert(std::same_as<decltype(u.get_ref<Order::First>()),
                               sus::Tuple<const i8&, const u64&>>);
    static_assert(
        std::same_as<decltype(u.get_ref<Order::Second>()), const u32&>);
    static_assert(std::same_as<decltype(u.get_mut<Order::First>()),
                               sus::Tuple<i8&, const u64&>>);
    static_assert(std::same_as<decltype(u.get_mut<Order::Second>()), u32&>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::First>()),
                     sus::Tuple<i8&, const u64&>>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::Second>()),
                     u32&&>);
  }
}

TEST(Union, Switch) {
  auto u = Union<sus_value_types((Order::First, u32),
                                 (Order::Second, u8))>::with<Order::First>(4u);
  switch (u) {
    case Order::First: break;
    case Order::Second: ADD_FAILURE();
    case Order::Third: ADD_FAILURE();
  }
}

TEST(Union, Which) {
  auto u = Union<sus_value_types((Order::First, u32),
                                 (Order::Second, u8))>::with<Order::First>(4u);
  EXPECT_EQ(u.which(), Order::First);
}

TEST(Union, Copy) {
  auto u = Union<sus_value_types((Order::First, u32),
                                 (Order::Second, u8))>::with<Order::First>(4u);
  static_assert(sus::mem::Copy<decltype(u)>);
  auto v = u;
  EXPECT_EQ(u.which(), v.which());
  EXPECT_EQ(u.get_ref<Order::First>(), v.get_ref<Order::First>());
}

TEST(Union, Clone) {
  struct S {
    S(u32 i) : i(i) {}
    S(S&&) = default;
    S& operator=(S&&) = default;

    S clone() const { return S(i); }

    bool operator==(const S& s) const noexcept { return i == s.i; }

    u32 i;
  };
  static_assert(::sus::mem::Clone<S>);

  auto u = Union<sus_value_types(
      (Order::First, S), (Order::Second, S))>::with<Order::First>(S(4u));
  static_assert(sus::mem::Clone<decltype(u)>);
  auto v = sus::clone(u);
  EXPECT_EQ(u.which(), v.which());
  EXPECT_EQ(u.get_ref<Order::First>(), v.get_ref<Order::First>());
}

TEST(Union, Eq) {
  auto u1 = Union<sus_value_types((Order::First, u32),
                                  (Order::Second, u8))>::with<Order::First>(4u);
  EXPECT_EQ(u1, u1);
  auto u2 = Union<sus_value_types(
      (Order::First, u32), (Order::Second, u8))>::with<Order::Second>(4_u8);
  EXPECT_EQ(u2, u2);
  EXPECT_NE(u1, u2);

  u2.set<Order::First>(5u);
  EXPECT_NE(u1, u2);

  u2.set<Order::First>(4u);
  EXPECT_EQ(u1, u2);
}

// TODO: Test cloneable tag.
// TODO: Test moveonly tag.
// TODO: Test different but comparable tag for Eq.

}  // namespace
