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

#include "choice/choice.h"

#include <variant>

#include "googletest/include/gtest/gtest.h"
#include "macros/__private/compiler_bugs.h"
#include "num/types.h"
#include "option/option.h"
#include "prelude.h"
#include "test/no_copy_move.h"

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

enum class Order {
  First,
  Second,
  Third,
};

inline constexpr size_t UamBytes = sus::Tuple<i32>::protects_uam ? 8 : 0;

// The Choice's tag can get stashed inside the Tuple, though this doesn't happen
// on MSVC.
static_assert(sizeof(Choice<sus_choice_types((Order::First, i32, u64))>) ==
              2 * sizeof(u64) + UamBytes + sus_if_msvc_else(sizeof(u64), 0));

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
  static_assert(sizeof(sus::Option<One>) == sizeof(One));

  // Two values in a Tuple isn't standard layout at this time. This allows the
  // Tuple to pack better but, also means we can't use the never-value field
  // optimization in Option.
  using Two = Choice<sus_choice_types((Order::First, u64, u64))>;
  static_assert(!std::is_standard_layout_v<Two>);
  static_assert(sizeof(sus::Option<Two>) > sizeof(Two));
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
    EXPECT_EQ(u.get_ref<Order::First>(), 1_u32);
  }
  {
    // All parameters convert to u32.
    U u = sus::choice<Order::First>(1u);
    EXPECT_EQ(u.get_ref<Order::First>(), 1_u32);
  }
  {
    // into() as an input to the tuple.
    U u = sus::choice<Order::First>(sus::into(1));
    EXPECT_EQ(u.get_ref<Order::First>(), 1_u32);
  }
  {
    // Copies the lvalue.
    auto i = 1_u32;
    U u = sus::choice<Order::First>(i);
    EXPECT_EQ(u.get_ref<Order::First>(), 1_u32);
  }
  {
    // Copies the const lvalue.
    const auto i = 1_u32;
    U u = sus::choice<Order::First>(i);
    EXPECT_EQ(u.get_ref<Order::First>(), 1_u32);
  }
  {
    // Copies the rvalue reference.
    auto i = 1_u32;
    U u = sus::choice<Order::First>(sus::move(i));
    EXPECT_EQ(u.get_ref<Order::First>(), 1_u32);
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
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // Some parameters convert to u32.
    U u = sus::choice<Order::First>(1_u32, 2u);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // All parameters convert to u32.
    U u = sus::choice<Order::First>(1u, 2u);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // into() as an input to the tuple.
    U u = sus::choice<Order::First>(1u, sus::into(2));
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // Copies the lvalue and const lvalue.
    auto i = 1_u32;
    const auto j = 2_u32;
    U u = sus::choice<Order::First>(i, j);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<1>(), 2_u32);
  }
  {
    // Copies the rvalue reference.
    auto i = 1_u32;
    U u = sus::choice<Order::First>(sus::move(i), 2_u32);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<0>(), 1_u32);
    EXPECT_EQ(u.get_ref<Order::First>().into_inner<1>(), 2_u32);
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

TEST(Choice, GetTypes) {
  // Single value first, double last.
  {
    auto u = Choice<sus_choice_types(
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
  // Double value first, single last.
  {
    auto u = Choice<sus_choice_types((Order::First, i8, u64),
                                     (Order::Second, u32))>::
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
  // With refs.
  {
    auto i = NoCopyMove();
    auto u = Choice<sus_choice_types(
        (Order::First, i8&, const u64&),
        (Order::Second, NoCopyMove&))>::with<Order::Second>(i);
    static_assert(std::same_as<decltype(u.get_ref<Order::First>()),
                               sus::Tuple<const i8&, const u64&>>);
    static_assert(
        std::same_as<decltype(u.get_ref<Order::Second>()), const NoCopyMove&>);
    static_assert(std::same_as<decltype(u.get_mut<Order::First>()),
                               sus::Tuple<i8&, const u64&>>);
    static_assert(
        std::same_as<decltype(u.get_mut<Order::Second>()), NoCopyMove&>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::First>()),
                     sus::Tuple<i8&, const u64&>>);
    static_assert(
        std::same_as<decltype(sus::move(u).into_inner<Order::Second>()),
                     NoCopyMove&>);
    EXPECT_EQ(&u.get_ref<Order::Second>(), &i);

    // Verify storing a reference in the first-of-N slot builds.
    auto u2 = Choice<sus_choice_types(
        (Order::First, NoCopyMove&),
        (Order::Second, i8&, const u64&))>::with<Order::First>(i);
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
  EXPECT_EQ(u.get_ref<Order::First>(), v.get_ref<Order::First>());
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
  EXPECT_EQ(u.get_ref<Order::First>(), v.get_ref<Order::First>());
  EXPECT_NE(&u.get_ref<Order::First>(), &v.get_ref<Order::First>());
}

template <class T, auto Tag>
concept CanGetRef = requires(T t) { t.template get_ref<Tag>(); };
template <class T, auto Tag>
concept CanGetMut = requires(T t) { t.template get_mut<Tag>(); };
template <class T, auto Tag>
concept CanIntoInner =
    requires(T t) { sus::move(t).template into_inner<Tag>(); };

TEST(Choice, Eq) {
  struct NotEq {};
  constexpr static NotEq not_eq_v;
  static_assert(!sus::ops::Eq<NotEq>);

  static_assert(
      sus::ops::Eq<Choice<sus_choice_types((1_i32, i32), (2_i32, i32))>>);
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
  constexpr auto operator==(const Weak& o) const& {
    return a == o.a && b == o.b;
  }
  constexpr auto operator<=>(const Weak& o) const& {
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
  auto u3 = ChoiceFloatFloat::with<Order::First>(f32::TODO_NAN);
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

}  // namespace
