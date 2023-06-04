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

#include "subspace/choice/choice.h"

#include <sstream>

#include "googletest/include/gtest/gtest.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/num/types.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"
#include "subspace/test/no_copy_move.h"

namespace {
enum class Order {
  First,
  Second,
  Third,
};
}

template <class Char>
struct fmt::formatter<Order, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(const Order& t, FormatContext& ctx) const {
    using enum Order;
    switch (t) {
      case First: return fmt::format_to(ctx.out(), "First");
      case Second: return fmt::format_to(ctx.out(), "Second");
      case Third: return fmt::format_to(ctx.out(), "Third");
    }
    sus::unreachable();
  }
};

namespace {

using sus::Choice;
using sus::test::NoCopyMove;

static_assert(::sus::choice_type::__private::AllValuesAreUnique<1>);
static_assert(::sus::choice_type::__private::AllValuesAreUnique<1, 2>);
static_assert(::sus::choice_type::__private::AllValuesAreUnique<1, 2, 3>);
static_assert(!::sus::choice_type::__private::AllValuesAreUnique<1, 2, 1>);
static_assert(!::sus::choice_type::__private::AllValuesAreUnique<2, 2, 1>);
static_assert(!::sus::choice_type::__private::AllValuesAreUnique<1, 2, 2>);
static_assert(!::sus::choice_type::__private::AllValuesAreUnique<1, 2, 3, 1>);
static_assert(!::sus::choice_type::__private::AllValuesAreUnique<1, 2, 1, 3>);
static_assert(!::sus::choice_type::__private::AllValuesAreUnique<1, 2, 3, 2>);

// The Choice's tag can get stashed inside the Tuple, though this doesn't happen
// on MSVC.
// NOPE this causes data to get clobbered when move-constructing into the
// storage.
// Clang: https://github.com/llvm/llvm-project/issues/60711
// GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108775
// static_assert(sizeof(Choice<sus_choice_types((Order::First, i32, u64))>) ==
//              2 * sizeof(u64) + sus_if_msvc_else(sizeof(u64), 0));
static_assert(sizeof(Choice<sus_choice_types((Order::First, i32, u64))>) ==
              2 * sizeof(u64) + sizeof(u64));

TEST(Choice, Tag) {
  using One =
      Choice<sus_choice_types((Order::First, u64), (Order::Second, u32))>;
  // `Tag` is an alias for the tag type.
  auto u = One::with<One::Tag::First>(1u);
}

TEST(Choice, NeverValue) {
  using One =
      Choice<sus_choice_types((Order::First, u64), (Order::Second, u32))>;
  static_assert(std::is_standard_layout_v<One>);
  static_assert(sus::mem::NeverValueField<One>);
  static_assert(sizeof(sus::Option<One>) == sizeof(One));

  // It used to be that Option<T> did not support the NeverValueField
  // optimization for non-Standard-Layout types, but it does now.
  using Two = Choice<sus_choice_types((Order::First, u64, u64))>;
  static_assert(!std::is_standard_layout_v<Two>);
  static_assert(sus::mem::NeverValueField<Two>);
  static_assert(sizeof(sus::Option<Two>) == sizeof(Two));
}

TEST(Choice, ConstructorFunctionNoValue) {
  using U =
      Choice<sus_choice_types((Order::First, u32), (Order::Second, void))>;
  {
    U u = sus::choice<Order::Second>();
    EXPECT_EQ(u.which(), Order::Second);
  }
}

TEST(Choice, ConstructorFunction1Value) {
  using U =
      Choice<sus_choice_types((Order::First, u32), (Order::Second, void))>;
  {
    // All parameters match the tuple type.
    U u = sus::choice<Order::First>(1_u32);
    EXPECT_EQ(u.as<Order::First>(), 1_u32);
  }
  {
    // All parameters convert to u32.
    U u = sus::choice<Order::First>(1u);
    EXPECT_EQ(u.as<Order::First>(), 1_u32);
  }
  {
    // into() as an input to the tuple.
    U u = sus::choice<Order::First>(sus::into(1));
    EXPECT_EQ(u.as<Order::First>(), 1_u32);
  }
  {
    // Copies the lvalue.
    auto i = 1_u32;
    U u = sus::choice<Order::First>(i);
    EXPECT_EQ(u.as<Order::First>(), 1_u32);
  }
  {
    // Copies the const lvalue.
    const auto i = 1_u32;
    U u = sus::choice<Order::First>(i);
    EXPECT_EQ(u.as<Order::First>(), 1_u32);
  }
  {
    // Copies the rvalue reference.
    auto i = 1_u32;
    U u = sus::choice<Order::First>(sus::move(i));
    EXPECT_EQ(u.as<Order::First>(), 1_u32);
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
    auto marker = sus::choice<Order::First>(s);
    EXPECT_EQ(copies, 0);
    Choice<sus_choice_types((Order::First, S), (Order::Second, void))> u =
        sus::move(marker);
    EXPECT_GE(copies, 1);
  }
}

TEST(Choice, ConstructorFunctionMoreThan1Value) {
  using U =
      Choice<sus_choice_types((Order::First, u32, u32), (Order::Second, void))>;
  {
    // All parameters match the tuple type.
    U u = sus::choice<Order::First>(1_u32, 2_u32);
    EXPECT_EQ(u.as<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.as<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // Some parameters convert to u32.
    U u = sus::choice<Order::First>(1_u32, 2u);
    EXPECT_EQ(u.as<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.as<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // All parameters convert to u32.
    U u = sus::choice<Order::First>(1u, 2u);
    EXPECT_EQ(u.as<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.as<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // into() as an input to the tuple.
    U u = sus::choice<Order::First>(1u, sus::into(2));
    EXPECT_EQ(u.as<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.as<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // Copies the lvalue and const lvalue.
    auto i = 1_u32;
    const auto j = 2_u32;
    U u = sus::choice<Order::First>(i, j);
    EXPECT_EQ(u.as<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.as<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // Copies the rvalue reference.
    auto i = 1_u32;
    U u = sus::choice<Order::First>(sus::move(i), 2_u32);
    EXPECT_EQ(u.as<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.as<Order::First>().into_inner<1>(), 2_u32);
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
    auto i = 2_u32;
    auto marker = sus::choice<Order::First>(s, i);
    EXPECT_EQ(copies, 0);
    Choice<sus_choice_types((Order::First, S, u32), (Order::Second, void))> u =
        sus::move(marker);
    EXPECT_GE(copies, 1);
  }
}

template <class T, auto Tag>
concept CanCallAs =
    requires(T&& choice) { sus::forward<T>(choice).template as<Tag>(); };
template <class T, auto Tag>
concept CanCallGet =
    requires(T&& choice) { sus::forward<T>(choice).template get<Tag>(); };
template <class T, auto Tag>
concept CanCallGetUnchecked = requires(T&& choice) {
  sus::forward<T>(choice).template get_unchecked<Tag>(unsafe_fn);
};

// Value types can't be accessed through an rvalue Choice as it would be a
// reference to a temporary.
using ChoiceWithValue = Choice<sus_choice_types((0, u32))>;
static_assert(!CanCallAs<ChoiceWithValue, 0>);
static_assert(!CanCallGet<ChoiceWithValue, 0>);
static_assert(!CanCallGetUnchecked<ChoiceWithValue, 0>);
// Reference types can be accessed through an rvalue Choice as the object does
// not live inside the temporary.
using ChoiceWithReference = Choice<sus_choice_types((0, u32&))>;
static_assert(CanCallAs<ChoiceWithReference, 0>);
static_assert(CanCallGet<ChoiceWithReference, 0>);
static_assert(CanCallGetUnchecked<ChoiceWithReference, 0>);

TEST(Choice, AsTypes) {
  // Single value first, double last.
  {
    auto u = Choice<sus_choice_types(
        (Order::First, u32), (Order::Second, i8, u64))>::with<Order::First>(3u);
    static_assert(std::same_as<decltype(u.as<Order::First>()), const u32&>);
    static_assert(std::same_as<decltype(u.as<Order::Second>()),
                               sus::Tuple<const i8&, const u64&>>);
    static_assert(std::same_as<decltype(u.as_mut<Order::First>()), u32&>);
    static_assert(std::same_as<decltype(u.as_mut<Order::Second>()),
                               sus::Tuple<i8&, u64&>>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::First>()), u32&&>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::Second>()),
                     sus::Tuple<i8, u64>>);
  }
  // Double value first, single last.
  {
    auto u = Choice<sus_choice_types((Order::First, i8, u64),
                                     (Order::Second, u32))>::
        with<Order::First>(sus::Tuple<i8, u64>::with(1_i8, 2_u64));
    static_assert(std::same_as<decltype(u.as<Order::First>()),
                               sus::Tuple<const i8&, const u64&>>);
    static_assert(std::same_as<decltype(u.as<Order::Second>()), const u32&>);
    static_assert(std::same_as<decltype(u.as_mut<Order::First>()),
                               sus::Tuple<i8&, u64&>>);
    static_assert(std::same_as<decltype(u.as_mut<Order::Second>()), u32&>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::First>()),
                     sus::Tuple<i8, u64>>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::Second>()),
                     u32&&>);
  }
  // With refs.
  {
    auto i = NoCopyMove();
    auto u = Choice<sus_choice_types(
        (Order::First, i8&, const u64&),
        (Order::Second, NoCopyMove&))>::with<Order::Second>(i);
    static_assert(std::same_as<decltype(u.as<Order::First>()),
                               sus::Tuple<const i8&, const u64&>>);
    static_assert(
        std::same_as<decltype(u.as<Order::Second>()), const NoCopyMove&>);
    static_assert(std::same_as<decltype(u.as_mut<Order::First>()),
                               sus::Tuple<i8&, const u64&>>);
    static_assert(
        std::same_as<decltype(u.as_mut<Order::Second>()), NoCopyMove&>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::First>()),
                     sus::Tuple<i8&, const u64&>>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::Second>()),
                     NoCopyMove&>);
    EXPECT_EQ(&u.as<Order::Second>(), &i);

    // Verify storing a reference in the first-of-N slot builds.
    auto u2 = Choice<sus_choice_types(
        (Order::First, NoCopyMove&),
        (Order::Second, i8&, const u64&))>::with<Order::First>(i);
  }
}

TEST(Choice, Get) {
  // Single value first, double last.
  {
    auto u = Choice<sus_choice_types(
        (Order::First, u32), (Order::Second, i8, u64))>::with<Order::First>(3u);
    {
      auto s = u.get<Order::First>();
      EXPECT_EQ(*s, 3u);
      auto n = u.get<Order::Second>();
      EXPECT_EQ(n, sus::None);
      static_assert(std::same_as<decltype(s), sus::Option<const u32&>>);
      static_assert(
          std::same_as<decltype(n),
                       sus::Option<sus::Tuple<const i8&, const u64&>>>);
    }

    u.set<Order::Second>(sus::tuple(1_i8, 2_u64));
    {
      auto n = u.get<Order::First>();
      EXPECT_EQ(n, sus::None);
      auto s = u.get<Order::Second>();
      EXPECT_EQ(*s, (sus::tuple(1_i8, 2_u64).construct()));
      static_assert(std::same_as<decltype(n), sus::Option<const u32&>>);
      static_assert(
          std::same_as<decltype(s),
                       sus::Option<sus::Tuple<const i8&, const u64&>>>);
    }
  }
  // Double value first, single last.
  {
    auto u =
        Choice<sus_choice_types((Order::First, i8, u64),
                                (Order::Second, u32))>::with<Order::Second>(3u);
    {
      auto s = u.get<Order::Second>();
      EXPECT_EQ(*s, 3u);
      auto n = u.get<Order::First>();
      EXPECT_EQ(n, sus::None);
      static_assert(std::same_as<decltype(s), sus::Option<const u32&>>);
      static_assert(
          std::same_as<decltype(n),
                       sus::Option<sus::Tuple<const i8&, const u64&>>>);
    }

    u.set<Order::First>(sus::tuple(1_i8, 2_u64));
    {
      auto n = u.get<Order::Second>();
      EXPECT_EQ(n, sus::None);
      auto s = u.get<Order::First>();
      EXPECT_EQ(*s, (sus::tuple(1_i8, 2_u64).construct()));
      static_assert(std::same_as<decltype(n), sus::Option<const u32&>>);
      static_assert(
          std::same_as<decltype(s),
                       sus::Option<sus::Tuple<const i8&, const u64&>>>);
    }
  }
  // With refs.
  {
    auto i = NoCopyMove();
    auto u = Choice<sus_choice_types(
        (Order::First, i8&, const u64&),
        (Order::Second, NoCopyMove&))>::with<Order::Second>(i);
    auto n = u.get<Order::First>();
    EXPECT_EQ(n, sus::None);
    auto s = u.get<Order::Second>();
    EXPECT_EQ(&*s, &i);
    static_assert(std::same_as<decltype(n),
                               sus::Option<sus::Tuple<const i8&, const u64&>>>);
    static_assert(std::same_as<decltype(s), sus::Option<const NoCopyMove&>>);
  }
}

TEST(Choice, GetMut) {
  // Single value first, double last.
  {
    auto u = Choice<sus_choice_types(
        (Order::First, u32), (Order::Second, i8, u64))>::with<Order::First>(3u);
    {
      auto s = u.get_mut<Order::First>();
      EXPECT_EQ(*s, 3u);
      auto n = u.get_mut<Order::Second>();
      EXPECT_EQ(n, sus::None);
      static_assert(std::same_as<decltype(s), sus::Option<u32&>>);
      static_assert(
          std::same_as<decltype(n), sus::Option<sus::Tuple<i8&, u64&>>>);
    }

    u.set<Order::Second>(sus::tuple(1_i8, 2_u64));
    {
      auto n = u.get_mut<Order::First>();
      EXPECT_EQ(n, sus::None);
      auto s = u.get_mut<Order::Second>();
      EXPECT_EQ(*s, (sus::tuple(1_i8, 2_u64).construct()));
      static_assert(std::same_as<decltype(n), sus::Option<u32&>>);
      static_assert(
          std::same_as<decltype(s), sus::Option<sus::Tuple<i8&, u64&>>>);
    }
  }
  // Double value first, single last.
  {
    auto u =
        Choice<sus_choice_types((Order::First, i8, u64),
                                (Order::Second, u32))>::with<Order::Second>(3u);
    {
      auto s = u.get_mut<Order::Second>();
      EXPECT_EQ(*s, 3u);
      auto n = u.get_mut<Order::First>();
      EXPECT_EQ(n, sus::None);
      static_assert(std::same_as<decltype(s), sus::Option<u32&>>);
      static_assert(
          std::same_as<decltype(n), sus::Option<sus::Tuple<i8&, u64&>>>);
    }

    u.set<Order::First>(sus::tuple(1_i8, 2_u64));
    {
      auto n = u.get_mut<Order::Second>();
      EXPECT_EQ(n, sus::None);
      auto s = u.get_mut<Order::First>();
      EXPECT_EQ(*s, (sus::tuple(1_i8, 2_u64).construct()));
      static_assert(std::same_as<decltype(n), sus::Option<u32&>>);
      static_assert(
          std::same_as<decltype(s), sus::Option<sus::Tuple<i8&, u64&>>>);
    }
  }
  // With refs.
  {
    auto i = NoCopyMove();
    auto u = Choice<sus_choice_types(
        (Order::First, i8&, const u64&),
        (Order::Second, NoCopyMove&))>::with<Order::Second>(i);
    auto n = u.get_mut<Order::First>();
    EXPECT_EQ(n, sus::None);
    auto s = u.get_mut<Order::Second>();
    EXPECT_EQ(&*s, &i);
    static_assert(
        std::same_as<decltype(n), sus::Option<sus::Tuple<i8&, const u64&>>>);
    static_assert(std::same_as<decltype(s), sus::Option<NoCopyMove&>>);
  }
}

TEST(Choice, GetUnchecked) {
  // Single value first, double last.
  {
    auto u = Choice<sus_choice_types(
        (Order::First, u32), (Order::Second, i8, u64))>::with<Order::First>(3u);
    {
      decltype(auto) s = u.get_unchecked<Order::First>(unsafe_fn);
      EXPECT_EQ(s, 3u);
      static_assert(std::same_as<decltype(s), const u32&>);
    }

    u.set<Order::Second>(sus::tuple(1_i8, 2_u64));
    {
      decltype(auto) s = u.get_unchecked<Order::Second>(unsafe_fn);
      EXPECT_EQ(s, (sus::tuple(1_i8, 2_u64).construct()));
      static_assert(
          std::same_as<decltype(s), sus::Tuple<const i8&, const u64&>>);
    }
  }
  // Double value first, single last.
  {
    auto u =
        Choice<sus_choice_types((Order::First, i8, u64),
                                (Order::Second, u32))>::with<Order::Second>(3u);
    {
      decltype(auto) s = u.get_unchecked<Order::Second>(unsafe_fn);
      EXPECT_EQ(s, 3u);
      static_assert(std::same_as<decltype(s), const u32&>);
    }

    u.set<Order::First>(sus::tuple(1_i8, 2_u64));
    {
      decltype(auto) s = u.get_unchecked<Order::First>(unsafe_fn);
      EXPECT_EQ(s, (sus::tuple(1_i8, 2_u64).construct()));
      static_assert(
          std::same_as<decltype(s), sus::Tuple<const i8&, const u64&>>);
    }
  }
  // With refs.
  {
    auto i = 1_i8;
    const auto j = 2_u64;
    auto u = Choice<sus_choice_types(
        (Order::First, i8&, const u64&),
        (Order::Second, NoCopyMove&))>::with<Order::First>(sus::tuple(i, j));
    decltype(auto) s = u.get_unchecked<Order::First>(unsafe_fn);
    EXPECT_EQ(&s.at<0u>(), &i);
    EXPECT_EQ(&s.at<1u>(), &j);
    static_assert(std::same_as<decltype(s), sus::Tuple<const i8&, const u64&>>);
  }
  {
    auto i = NoCopyMove();
    auto u = Choice<sus_choice_types(
        (Order::First, i8&, const u64&),
        (Order::Second, NoCopyMove&))>::with<Order::Second>(i);
    decltype(auto) s = u.get_unchecked<Order::Second>(unsafe_fn);
    EXPECT_EQ(&s, &i);
    static_assert(std::same_as<decltype(s), const NoCopyMove&>);
  }
}

TEST(Choice, Switch) {
  auto u = Choice<sus_choice_types(
      (Order::First, u32), (Order::Second, u8))>::with<Order::First>(4u);
  switch (u) {
    case Order::First: break;
    case Order::Second: ADD_FAILURE();
    case Order::Third: ADD_FAILURE();
  }
}

TEST(Choice, Which) {
  auto u = Choice<sus_choice_types(
      (Order::First, u32), (Order::Second, u8))>::with<Order::First>(4u);
  EXPECT_EQ(u.which(), Order::First);

  auto v = Choice<sus_choice_types(
      (Order::First, u32), (Order::Second, u8))>::with<Order::Second>(4_u8);
  EXPECT_EQ(v.which(), Order::Second);
}

TEST(Choice, Copy) {
  auto u = Choice<sus_choice_types(
      (Order::First, u32), (Order::Second, u8))>::with<Order::First>(4u);
  static_assert(sus::mem::Copy<decltype(u)>);
  auto v = u;
  EXPECT_EQ(u.which(), v.which());
  EXPECT_EQ(u.as<Order::First>(), v.as<Order::First>());
}

TEST(Choice, Clone) {
  struct S {
    S(u32 i) : i(i) {}
    S(S&&) = default;
    S& operator=(S&&) = default;

    S clone() const { return S(i); }

    bool operator==(const S& s) const noexcept { return i == s.i; }

    u32 i;
  };
  static_assert(::sus::mem::Clone<S>);

  auto u = Choice<sus_choice_types(
      (Order::First, S), (Order::Second, S))>::with<Order::First>(S(4u));
  static_assert(!sus::mem::Copy<decltype(u)>);
  static_assert(sus::mem::Clone<decltype(u)>);
  auto v = sus::clone(u);
  EXPECT_EQ(u.which(), v.which());
  EXPECT_EQ(u.as<Order::First>(), v.as<Order::First>());
  EXPECT_NE(&u.as<Order::First>(), &v.as<Order::First>());
}

template <class T, auto Tag>
concept CanGetRef = requires(T t) { t.template as<Tag>(); };
template <class T, auto Tag>
concept CanGetMut = requires(T t) { t.template as_mut<Tag>(); };
template <class T, auto Tag>
concept CanIntoInner =
    requires(T t) { sus::move(t).template into_inner<Tag>(); };

TEST(Choice, Eq) {
  struct NotEq {};
  constexpr static NotEq not_eq_v;
  static_assert(!sus::ops::Eq<NotEq>);

  // Same types.
  static_assert(
      sus::ops::Eq<Choice<sus_choice_types((1_i32, i32), (2_i32, i32))>>);
  // Eq types.
  static_assert(
      sus::ops::Eq<Choice<sus_choice_types((1_i32, i32), (2_i32, i32))>,
                   Choice<sus_choice_types((1_i64, i64), (2_i64, i16))>>);
  // Not Eq types.
  static_assert(
      !sus::ops::Eq<Choice<sus_choice_types((1, NotEq), (2, NotEq))>>);

  using OrderChoice =
      Choice<sus_choice_types((Order::First, u32), (Order::Second, u8))>;

  auto u1 = OrderChoice::with<Order::First>(4u);
  EXPECT_EQ(u1, u1);
  auto u2 = Choice<sus_choice_types(
      (Order::First, u32), (Order::Second, u8))>::with<Order::Second>(4_u8);
  EXPECT_EQ(u2, u2);
  EXPECT_NE(u1, u2);

  u2.set<Order::First>(5u);
  EXPECT_NE(u1, u2);

  u2.set<Order::First>(4u);
  EXPECT_EQ(u1, u2);

  // Comparison with marker types. EXPECT_EQ also converts it to a const
  // reference, so this tests that comparison from a const marker works (if the
  // inner type is copyable).
  auto no_storage =
      Choice<sus_choice_types((Order::First, void))>::with<Order::First>();
  EXPECT_EQ(no_storage, sus::choice<Order::First>());
  auto single_storage = OrderChoice::with<Order::First>(4u);
  EXPECT_EQ(single_storage, sus::choice<Order::First>(4u));
  auto double_storage =
      Choice<sus_choice_types((Order::First, u32, u64))>::with<Order::First>(
          sus::tuple(2_u32, 3_u64));
  EXPECT_EQ(double_storage, sus::choice<Order::First>(2u, 3u));
}

TEST(Choice, Ord) {
  using OrderChoice =
      Choice<sus_choice_types((Order::First, u32), (Order::Second, u8))>;
  auto u1 = OrderChoice::with<Order::First>(4u);
  auto u2 = OrderChoice::with<Order::First>(5u);
  EXPECT_EQ(u1, u1);
  EXPECT_LT(u1, u2);
  auto u3 = OrderChoice::with<Order::Second>(4_u8);
  EXPECT_LT(u1, u3);
}

TEST(Choice, StrongOrder) {
  using OrderChoice =
      Choice<sus_choice_types((Order::First, u32), (Order::Second, u8))>;
  using RevOrderChoice =
      Choice<sus_choice_types((Order::Second, u8), (Order::First, u32))>;

  auto u1 = OrderChoice::with<Order::First>(4u);
  // Same enum value and inner value.
  EXPECT_EQ(std::strong_order(u1, u1), std::strong_ordering::equivalent);
  auto u2 = OrderChoice::with<Order::First>(5u);
  // Same enum value and different inner value.
  EXPECT_EQ(std::strong_order(u1, u2), std::strong_ordering::less);

  // Different enum value, compare the enum values.
  auto u3 = OrderChoice::with<Order::Second>(1_u8);
  EXPECT_EQ(std::strong_order(u1, u3), std::strong_ordering::less);

  // The higher enum value comes first. Different enum values, the enum values
  // are compared (as opposed to the position of the enum value in the union
  // defn).
  auto r1 = RevOrderChoice::with<Order::First>(1_u32);
  auto r2 = RevOrderChoice::with<Order::Second>(1_u8);
  EXPECT_EQ(std::strong_order(r1, r2), std::strong_ordering::less);
}

struct Weak {
  sus_clang_bug_54040(constexpr inline Weak(i32 a, i32 b) : a(a), b(b){});

  constexpr auto operator==(const Weak& o) const& noexcept {
    return a == o.a && b == o.b;
  }
  constexpr auto operator<=>(const Weak& o) const& noexcept {
    if (a == o.a) return std::weak_ordering::equivalent;
    if (a < o.a) return std::weak_ordering::less;
    return std::weak_ordering::greater;
  }

  i32 a;
  i32 b;
};

TEST(Choice, WeakOrder) {
  using ChoiceWeak =
      Choice<sus_choice_types((Order::First, Weak), (Order::Second, Weak))>;
  static_assert(!sus::ops::Ord<ChoiceWeak>);
  static_assert(sus::ops::WeakOrd<ChoiceWeak>);

  // Same enum value and inner value.
  auto u1 = ChoiceWeak::with<Order::First>(Weak(1, 1));
  EXPECT_EQ(std::weak_order(u1, u1), std::weak_ordering::equivalent);

  // Different inner values, but weak equivalence.
  auto u2 = ChoiceWeak::with<Order::First>(Weak(1, 2));
  EXPECT_EQ(std::weak_order(u1, u2), std::weak_ordering::equivalent);

  // Different inner values.
  auto u3 = ChoiceWeak::with<Order::First>(Weak(2, 1));
  EXPECT_EQ(std::weak_order(u1, u3), std::weak_ordering::less);

  // Weak order tags, the requirement is that they are Eq.
  constexpr Weak a(1, 2);
  constexpr Weak b(2, 3);
  static_assert(::sus::ops::Eq<Weak>);
  EXPECT_EQ((std::weak_order(
                Choice<sus_choice_types((a, i32), (b, i32))>::with<a>(1),
                Choice<sus_choice_types((a, i32), (b, i32))>::with<b>(2))),
            std::weak_ordering::less);
}

TEST(Choice, PartialOrder) {
  using ChoiceFloatFloat =
      Choice<sus_choice_types((Order::First, f32), (Order::Second, i32))>;

  // Different values.
  auto u1 = ChoiceFloatFloat::with<Order::First>(1.f);
  auto u2 = ChoiceFloatFloat::with<Order::First>(2.f);
  EXPECT_EQ(std::partial_order(u1, u2), std::partial_ordering::less);

  // NaN is unordered.
  auto u3 = ChoiceFloatFloat::with<Order::First>(f32::NAN);
  EXPECT_EQ(std::partial_order(u1, u3), std::partial_ordering::unordered);

  // 0 == -0.
  EXPECT_EQ((std::partial_order(ChoiceFloatFloat::with<Order::First>(0.f),
                                ChoiceFloatFloat::with<Order::First>(-0.f))),
            std::partial_ordering::equivalent);

  // Different tags.
  EXPECT_EQ((std::partial_order(ChoiceFloatFloat::with<Order::First>(0.f),
                                ChoiceFloatFloat::with<Order::Second>(3_i32))),
            std::partial_ordering::less);

  // Partial order tags, the requirement is that they are Eq.
  constexpr f32 a = 0.f;
  constexpr f32 b = 1.f;
  static_assert(::sus::ops::Eq<f32>);
  EXPECT_EQ((std::partial_order(
                Choice<sus_choice_types((a, i32), (b, i32))>::with<a>(1),
                Choice<sus_choice_types((a, i32), (b, i32))>::with<b>(2))),
            std::partial_ordering::less);
}

struct NotCmp {};
static_assert(!sus::ops::PartialOrd<NotCmp>);

static_assert(::sus::ops::Ord<Choice<sus_choice_types((1, int))>,
                              Choice<sus_choice_types((1, int))>>);
static_assert(!::sus::ops::Ord<Choice<sus_choice_types((1, Weak))>,
                               Choice<sus_choice_types((1, Weak))>>);
static_assert(::sus::ops::WeakOrd<Choice<sus_choice_types((1, Weak))>,
                                  Choice<sus_choice_types((1, Weak))>>);
static_assert(!::sus::ops::WeakOrd<Choice<sus_choice_types((1, float))>,
                                   Choice<sus_choice_types((1, float))>>);
static_assert(::sus::ops::PartialOrd<Choice<sus_choice_types((1, float))>,
                                     Choice<sus_choice_types((1, float))>>);
static_assert(!::sus::ops::PartialOrd<Choice<sus_choice_types((1, NotCmp))>,
                                      Choice<sus_choice_types((1, NotCmp))>>);

TEST(Choice, VoidValues) {
  auto u1 = Choice<sus_choice_types(
      (Order::First, u32), (Order::Second, void))>::with<Order::First>(4u);
  auto u2 = Choice<sus_choice_types(
      (Order::First, u32), (Order::Second, void))>::with<Order::Second>();
  auto u3 = Choice<sus_choice_types(
      (Order::First, void), (Order::Second, u32))>::with<Order::First>();
  auto u4 = Choice<sus_choice_types(
      (Order::First, void), (Order::Second, u32))>::with<Order::Second>(4u);

  static_assert(sus::mem::Copy<decltype(u1)>);
  static_assert(sus::mem::Copy<decltype(u3)>);
  static_assert(sus::ops::Eq<decltype(u1)>);
  static_assert(sus::ops::Eq<decltype(u3)>);
  static_assert(sus::ops::Ord<decltype(u1)>);
  static_assert(sus::ops::Ord<decltype(u3)>);

  static_assert(CanGetRef<decltype(u1), Order::First>);
  static_assert(!CanGetRef<decltype(u1), Order::Second>);
  static_assert(!CanGetRef<decltype(u3), Order::First>);
  static_assert(CanGetRef<decltype(u3), Order::Second>);
  static_assert(CanGetMut<decltype(u1), Order::First>);
  static_assert(!CanGetMut<decltype(u1), Order::Second>);
  static_assert(!CanGetMut<decltype(u3), Order::First>);
  static_assert(CanGetMut<decltype(u3), Order::Second>);
  static_assert(CanIntoInner<decltype(u1), Order::First>);
  static_assert(!CanIntoInner<decltype(u1), Order::Second>);
  static_assert(!CanIntoInner<decltype(u3), Order::First>);
  static_assert(CanIntoInner<decltype(u3), Order::Second>);

  u2 = sus::move(u1);       // Move assign with void value.
  u4 = u3;                  // Copy assign with void value.
  auto u5 = sus::move(u2);  // Move construct with void value.
  auto u6 = u4;             // Copy construct with void value.

  EXPECT_EQ(u4.which(), u6.which());

  u5.set<Order::Second>();
  u5.set<Order::Second>();
  u5.set<Order::First>(3u);
  u5.set<Order::First>(3u);

  u6.set<Order::First>();
  u6.set<Order::First>();
  u6.set<Order::Second>(3u);
  u6.set<Order::Second>(3u);

  EXPECT_NE(u4, u6);
  EXPECT_EQ(u6, u6);
  EXPECT_LT(u4, u6);
}

TEST(Choice, fmt) {
  auto u = Choice<sus_choice_types(
      (Order::First, u32), (Order::Second, void))>::with<Order::First>(4u);

  static_assert(fmt::is_formattable<decltype(u), char>::value);

  EXPECT_EQ(fmt::format("{}", u), "Choice(First, 4)");
  u.set<Order::Second>();
  EXPECT_EQ(fmt::format("{}", u), "Choice(Second)");

  struct NoFormat {
    i32 a = 0x16ae3cf2;
    constexpr bool operator==(const NoFormat& rhs) const noexcept {
      return a == rhs.a;
    }
  };
  static_assert(sus::ops::Eq<NoFormat>);
  static_assert(!fmt::is_formattable<NoFormat, char>::value);

  constexpr auto taga = NoFormat();
  constexpr auto tagb = NoFormat(0xf00d);
  auto un = Choice<sus_choice_types((taga, u32), (tagb, void))>::with<taga>(4u);
  static_assert(fmt::is_formattable<decltype(un), char>::value);
  EXPECT_EQ(fmt::format("{}", un), "Choice(f2-3c-ae-16, 4)");
  un.set<tagb>();
  EXPECT_EQ(fmt::format("{}", un), "Choice(0d-f0-00-00)");
}

TEST(Choice, Stream) {
  std::stringstream s;
  s << Choice<sus_choice_types((Order::First, u32),
                               (Order::Second, void))>::with<Order::First>(4u);
  EXPECT_EQ(s.str(), "Choice(First, 4)");
}

TEST(Choice, GTest) {
  EXPECT_EQ(
      testing::PrintToString(
          Choice<sus_choice_types((Order::First, u32), (Order::Second, void))>::
              with<Order::First>(4u)),
      "Choice(First, 4)");
}

}  // namespace
