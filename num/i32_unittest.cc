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

#include "num/i32.h"

#include <type_traits>

#include "concepts/make_default.h"
#include "mem/__private/relocate.h"
#include "num/num_concepts.h"
#include "option/option.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::None;
using sus::Option;

static_assert(std::is_signed_v<decltype(i32::primitive_value)>);
static_assert(sizeof(decltype(i32::primitive_value)) == 4);
static_assert(sizeof(i32) == sizeof(decltype(i32::primitive_value)));

namespace behaviour {
using T = i32;
using From = decltype(i32::primitive_value);
static_assert(!std::is_trivial_v<T>, "");
static_assert(!std::is_aggregate_v<T>, "");
static_assert(std::is_standard_layout_v<T>, "");
static_assert(!std::is_trivially_default_constructible_v<T>, "");
static_assert(std::is_default_constructible_v<T>, "");
static_assert(std::is_trivially_copy_constructible_v<T>, "");
static_assert(std::is_trivially_copy_assignable_v<T>, "");
static_assert(std::is_trivially_move_constructible_v<T>, "");
static_assert(std::is_trivially_move_assignable_v<T>, "");
static_assert(std::is_trivially_destructible_v<T>, "");
static_assert(std::is_copy_constructible_v<T>, "");
static_assert(std::is_copy_assignable_v<T>, "");
static_assert(std::is_move_constructible_v<T>, "");
static_assert(std::is_move_assignable_v<T>, "");
static_assert(std::is_nothrow_swappable_v<T>, "");
static_assert(std::is_constructible_v<T, From&&>, "");
static_assert(std::is_assignable_v<T, From&&>, "");
static_assert(std::is_constructible_v<T, const From&>, "");
static_assert(std::is_assignable_v<T, const From&>, "");
static_assert(std::is_constructible_v<T, From>, "");
static_assert(!std::is_trivially_constructible_v<T, From>, "");
static_assert(std::is_assignable_v<T, From>, "");
static_assert(std::is_nothrow_destructible_v<T>, "");
static_assert(sus::concepts::MakeDefault<T>::has_concept, "");
static_assert(sus::mem::__private::relocate_one_by_memcpy_v<T>, "");
static_assert(sus::mem::__private::relocate_array_by_memcpy_v<T>, "");
}  // namespace behaviour

// Hex.
static_assert((0x123abC_i32).primitive_value == 0x123abC);
static_assert((0X123abC_i32).primitive_value == 0X123abC);
// Binary.
static_assert((0b101_i32).primitive_value == 0b101);
static_assert((0B101_i32).primitive_value == 0B101);
// Octal.
static_assert((0123_i32).primitive_value == 0123);

// Decimal.
static_assert((0_i32).primitive_value == 0);
static_assert((1_i32).primitive_value == 1);
static_assert((12_i32).primitive_value == 12);
static_assert((123_i32).primitive_value == 123);
static_assert((1234_i32).primitive_value == 1234);
static_assert((12345_i32).primitive_value == 12345);
static_assert((123456_i32).primitive_value == 123456);
static_assert((1234567_i32).primitive_value == 1234567);
static_assert((12345678_i32).primitive_value == 12345678);
static_assert((123456789_i32).primitive_value == 123456789);
static_assert((1234567891_i32).primitive_value == 1234567891);

// i32::MAX()
static_assert((0x7fffffff_i32).primitive_value == 0x7fffffff);
template <class T>
concept MaxInRange = requires {
                       { 0x7fffffff_i32 } -> std::same_as<T>;
                       { T(0x7fffffff) } -> std::same_as<T>;
                     };
static_assert(MaxInRange<i32>);

// TODO: How can we get this to build on GCC? Is this not legal?
#if defined(_MSC_VER)
// i32::MAX() + 1
template <class T>
concept MaxPlusOneInRange = requires {
                              { 0x80000000_i32 } -> std::same_as<T>;
                            };
static_assert(!MaxPlusOneInRange<i32>);
template <class T>
concept MaxPlusOneInRangeConstruct = requires {
                                       { T(0x80000000) } -> std::same_as<T>;
                                     };
static_assert(!MaxPlusOneInRangeConstruct<i32>);
#endif

TEST(i32, Traits) {
  static_assert(sus::num::Neg<i32>);
  static_assert(sus::num::Add<i32, i32>);
  static_assert(sus::num::AddAssign<i32, i32>);
  static_assert(sus::num::Sub<i32, i32>);
  static_assert(sus::num::SubAssign<i32, i32>);
  static_assert(sus::num::Mul<i32, i32>);
  static_assert(sus::num::MulAssign<i32, i32>);
  static_assert(sus::num::Div<i32, i32>);
  static_assert(sus::num::DivAssign<i32, i32>);
  static_assert(sus::num::Rem<i32, i32>);
  static_assert(sus::num::RemAssign<i32, i32>);
  static_assert(sus::num::BitAnd<i32, i32>);
  static_assert(sus::num::BitAndAssign<i32, i32>);
  static_assert(sus::num::BitOr<i32, i32>);
  static_assert(sus::num::BitOrAssign<i32, i32>);
  static_assert(sus::num::BitXor<i32, i32>);
  static_assert(sus::num::BitXorAssign<i32, i32>);
  static_assert(sus::num::BitNot<i32>);
  static_assert(sus::num::Shl<i32>);
  static_assert(sus::num::ShlAssign<i32>);
  static_assert(sus::num::Shr<i32>);
  static_assert(sus::num::ShrAssign<i32>);

  static_assert(sus::num::Ord<i32, i32>);
  static_assert(1_i32 >= 1_i32);
  static_assert(2_i32 > 1_i32);
  static_assert(1_i32 <= 1_i32);
  static_assert(1_i32 < 2_i32);
  static_assert(sus::num::Eq<i32, i32>);
  static_assert(1_i32 == 1_i32);
  static_assert(!(1_i32 == 2_i32));
  static_assert(1_i32 != 2_i32);
  static_assert(!(1_i32 != 1_i32));

  // Verify constexpr.
  constexpr i32 c = 1_i32 + 2_i32 - 3_i32 * 4_i32 / 5_i32 % 6_i32 & 7_i32 |
                    8_i32 ^ 9_i32 + -10_i32;
  constexpr std::strong_ordering o = 2_i32 <=> 3_i32;
}

TEST(i32, Abs) {
  [[maybe_unused]] constexpr auto a = i32(-1).abs();

  EXPECT_EQ(i32().abs(), 0_i32);
  EXPECT_EQ(i32(1).abs(), 1_i32);
  EXPECT_EQ(i32(-1).abs(), 1_i32);
  EXPECT_EQ(i32(1234567).abs(), 1234567_i32);
  EXPECT_EQ(i32(-1234567).abs(), 1234567_i32);
  EXPECT_EQ(i32::MAX().abs(), i32::MAX());
  EXPECT_EQ((i32::MIN() + 1_i32).abs(), i32::MAX());

  // lvalue.
  auto i = -9000_i32;
  EXPECT_EQ(i.abs(), 9000_i32);
}

TEST(i32DeathTest, Abs) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH([[maybe_unused]] auto x = i32::MIN().abs(), "");
#endif
}

TEST(i32, CheckedAbs) {
  [[maybe_unused]] constexpr auto a = i32(-1).checked_abs();

  EXPECT_EQ(i32(1234567).checked_abs(), Option<i32>::some(1234567_i32));
  EXPECT_EQ(i32(-1234567).checked_abs(), Option<i32>::some(1234567_i32));
  EXPECT_EQ(i32::MAX().checked_abs(), Option<i32>::some(i32::MAX()));
  EXPECT_EQ((i32::MIN() + 1_i32).checked_abs(), Option<i32>::some(i32::MAX()));
  EXPECT_EQ(i32::MIN().checked_abs(), None);

  // lvalue.
  auto i = -9000_i32;
  EXPECT_EQ(i.checked_abs(), Option<i32>::some(9000_i32));
}

TEST(i32, SaturatingAbs) {
  [[maybe_unused]] constexpr auto a = i32(-1).saturating_abs();

  EXPECT_EQ(i32(1234567).saturating_abs(), 1234567_i32);
  EXPECT_EQ(i32(-1234567).saturating_abs(), 1234567_i32);
  EXPECT_EQ(i32::MAX().saturating_abs(), i32::MAX());
  EXPECT_EQ((i32::MIN() + 1_i32).saturating_abs(), i32::MAX());
  EXPECT_EQ((i32::MIN()).saturating_abs(), i32::MAX());

  // lvalue.
  auto i = -9000_i32;
  EXPECT_EQ(i.saturating_abs(), 9000_i32);
}

TEST(i32, UnsignedAbs) {
  [[maybe_unused]] constexpr auto a = i32(-1).unsigned_abs();

  EXPECT_EQ(i32(1234567).unsigned_abs(), 1234567);
  EXPECT_EQ(i32(-1234567).saturating_abs(), 1234567);
  EXPECT_EQ(i32::MAX().unsigned_abs(), 0x7fffffff);
  EXPECT_EQ((i32::MIN() + 1_i32).unsigned_abs(), 0x7fffffff);
  EXPECT_EQ((i32::MIN()).unsigned_abs(), 0x80000000);

  // lvalue.
  auto i = -9000_i32;
  EXPECT_EQ(i.unsigned_abs(), 9000);
}

TEST(i32, WrappingAbs) {
  [[maybe_unused]] constexpr auto a = i32(-1).wrapping_abs();

  EXPECT_EQ(i32(1234567).wrapping_abs(), 1234567_i32);
  EXPECT_EQ(i32(-1234567).wrapping_abs(), 1234567_i32);
  EXPECT_EQ(i32::MAX().wrapping_abs(), i32::MAX());
  EXPECT_EQ((i32::MIN() + 1_i32).wrapping_abs(), i32::MAX());
  EXPECT_EQ((i32::MIN()).wrapping_abs(), i32::MIN());

  // lvalue.
  auto i = -9000_i32;
  EXPECT_EQ(i.wrapping_abs(), 9000_i32);
}

TEST(i32, AbsDiff) {
  [[maybe_unused]] constexpr auto a = i32(-1).abs_diff(10_i32);

  EXPECT_EQ(i32(0).abs_diff(i32(0)), 0u);
  EXPECT_EQ(i32().abs_diff(i32(123456)), 123456u);
  EXPECT_EQ(i32(123456).abs_diff(i32(0)), 123456u);
  EXPECT_EQ(i32().abs_diff(i32(-123456)), 123456u);
  EXPECT_EQ(i32(-123456).abs_diff(i32(0)), 123456u);
  EXPECT_EQ(i32(-123456).abs_diff(i32(-123456)), 0u);
  EXPECT_EQ(i32(123456).abs_diff(i32(123456)), 0u);
  EXPECT_EQ(i32::MAX().abs_diff(i32::MIN()), 0x80000000u + 0x7fffffffu);
  EXPECT_EQ(i32::MIN().abs_diff(i32::MAX()), 0x80000000u + 0x7fffffffu);

  // lvalue.
  auto i = -9000_i32;
  auto j = -1000_i32;
  EXPECT_EQ(i.abs_diff(j), 8000u);
}

TEST(i32, Add) {
  [[maybe_unused]] constexpr auto a = -1_i32 + 3_i32;

  EXPECT_EQ(0_i32 + 0_i32, 0_i32);
  EXPECT_EQ(-12345_i32 + 12345_i32, 0_i32);
  EXPECT_EQ(-12345_i32 + 1_i32, -12344_i32);
  EXPECT_EQ(12345_i32 + 1_i32, 12346_i32);
  EXPECT_EQ(12345_i32 + -1_i32, 12344_i32);
  EXPECT_EQ(i32::MAX() + 0_i32, i32::MAX());
  EXPECT_EQ(i32::MAX() + -1_i32, i32(i32::MAX_PRIMITIVE - 1));
  EXPECT_EQ(i32::MIN() + 0_i32, i32::MIN());
  EXPECT_EQ(i32::MIN() + 1_i32, i32(i32::MIN_PRIMITIVE + 1));
  EXPECT_EQ(i32::MIN() + i32::MAX(), -1_i32);
  EXPECT_EQ(i32::MAX() + i32::MIN(), -1_i32);

  auto x = 0_i32;
  x += 0_i32;
  EXPECT_EQ(x, 0_i32);
  x = -12345_i32;
  x += 12345_i32;
  EXPECT_EQ(x, 0_i32);
  x = -12345_i32;
  x += 1_i32;
  EXPECT_EQ(x, -12344_i32);
  x = 12345_i32;
  x += 1_i32;
  EXPECT_EQ(x, 12346_i32);
  x = 12345_i32;
  x += -1_i32;
  EXPECT_EQ(x, 12344_i32);
  x = i32::MAX();
  x += 0_i32;
  EXPECT_EQ(x, i32::MAX());
  x = i32::MAX();
  x += -1_i32;
  EXPECT_EQ(x, i32(i32::MAX_PRIMITIVE - 1));
  x = i32::MIN();
  x += 0_i32;
  EXPECT_EQ(x, i32::MIN());
  x = i32::MIN();
  x += 1_i32;
  EXPECT_EQ(x, i32(i32::MIN_PRIMITIVE + 1));
}

TEST(i32DeathTest, AddOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX() + 1_i32, "");
  EXPECT_DEATH(i32::MAX() + i32::MAX(), "");
  EXPECT_DEATH(i32::MIN() + -1_i32, "");
  EXPECT_DEATH(i32::MIN() + i32::MIN(), "");
#endif
}

TEST(i32, CheckedAdd) {
  [[maybe_unused]] constexpr auto a = (-1_i32).checked_add(3_i32);

  EXPECT_EQ((0_i32).checked_add(0_i32).unwrap(), 0_i32);
  EXPECT_EQ((-12345_i32).checked_add(12345_i32).unwrap(), 0_i32);

  EXPECT_EQ(i32::MAX().checked_add(1_i32), None);
  EXPECT_EQ((1_i32).checked_add(i32::MAX()), None);
  EXPECT_EQ(i32::MAX().checked_add(i32::MAX()), None);
  EXPECT_EQ(i32::MIN().checked_add(-1_i32), None);
  EXPECT_EQ((-1_i32).checked_add(i32::MIN()), None);
  EXPECT_EQ(i32::MIN().checked_add(i32::MIN()), None);
}

TEST(i32, SaturatingAdd) {
  [[maybe_unused]] constexpr auto a = (-1_i32).saturating_add(3_i32);

  EXPECT_EQ((0_i32).saturating_add(0_i32), 0_i32);
  EXPECT_EQ((-12345_i32).saturating_add(12345_i32), 0_i32);

  EXPECT_EQ(i32::MAX().saturating_add(1_i32), i32::MAX());
  EXPECT_EQ((1_i32).saturating_add(i32::MAX()), i32::MAX());
  EXPECT_EQ(i32::MAX().saturating_add(i32::MAX()), i32::MAX());
  EXPECT_EQ(i32::MIN().saturating_add(-1_i32), i32::MIN());
  EXPECT_EQ((-1_i32).saturating_add(i32::MIN()), i32::MIN());
  EXPECT_EQ(i32::MIN().saturating_add(i32::MIN()), i32::MIN());
}

TEST(i32, UncheckedAdd) {
  [[maybe_unused]] constexpr auto a = (-1_i32).unchecked_add(unsafe_fn, 3_i32);

  EXPECT_EQ((0_i32).unchecked_add(unsafe_fn, 0_i32), 0_i32);
  EXPECT_EQ((-12345_i32).unchecked_add(unsafe_fn, 12345_i32), 0_i32);
  EXPECT_EQ((-12345_i32).unchecked_add(unsafe_fn, 1_i32), -12344_i32);
  EXPECT_EQ((12345_i32).unchecked_add(unsafe_fn, 1_i32), 12346_i32);
  EXPECT_EQ((12345_i32).unchecked_add(unsafe_fn, -1_i32), 12344_i32);
  EXPECT_EQ(i32::MAX().unchecked_add(unsafe_fn, 0_i32), i32::MAX());
  EXPECT_EQ(i32::MAX().unchecked_add(unsafe_fn, -1_i32),
            i32(i32::MAX_PRIMITIVE - 1));
  EXPECT_EQ(i32::MIN().unchecked_add(unsafe_fn, 0_i32), i32::MIN());
  EXPECT_EQ(i32::MIN().unchecked_add(unsafe_fn, 1_i32),
            i32(i32::MIN_PRIMITIVE + 1));
  EXPECT_EQ(i32::MIN().unchecked_add(unsafe_fn, i32::MAX()), -1_i32);
  EXPECT_EQ(i32::MAX().unchecked_add(unsafe_fn, i32::MIN()), -1_i32);
}

TEST(i32, WrappingAdd) {
  [[maybe_unused]] constexpr auto a = (-1_i32).wrapping_add(3_i32);

  EXPECT_EQ((0_i32).wrapping_add(0_i32), 0_i32);
  EXPECT_EQ((-12345_i32).wrapping_add(12345_i32), 0_i32);

  EXPECT_EQ(i32::MAX().wrapping_add(1_i32), i32::MIN());
  EXPECT_EQ(i32::MAX().wrapping_add(2_i32), i32::MIN() + 1_i32);
  EXPECT_EQ((2_i32).wrapping_add(i32::MAX()), i32::MIN() + 1_i32);
  EXPECT_EQ(i32::MAX().wrapping_add(i32::MAX()), -2_i32);
  EXPECT_EQ(i32::MIN().wrapping_add(-1_i32), i32::MAX());
  EXPECT_EQ(i32::MIN().wrapping_add(-2_i32), i32::MAX() - 1_i32);
  EXPECT_EQ((-2_i32).wrapping_add(i32::MIN()), (i32::MAX() - 1_i32));
  EXPECT_EQ(i32::MIN().wrapping_add(i32::MIN()), 0_i32);
}

TEST(i32, Div) {
  [[maybe_unused]] constexpr auto a = -1_i32 / 3_i32;

  EXPECT_EQ(0_i32 / 123_i32, 0_i32);
  EXPECT_EQ(-2345_i32 / 1_i32, -2345_i32);
  EXPECT_EQ(2345_i32 / 1_i32, 2345_i32);
  EXPECT_EQ(-2345_i32 / -1_i32, 2345_i32);
  EXPECT_EQ(2345_i32 / -1_i32, -2345_i32);
  EXPECT_EQ(2222_i32 / 2_i32, 1111_i32);
  EXPECT_EQ(-2222_i32 / 2_i32, -1111_i32);
  EXPECT_EQ(2222_i32 / -2_i32, -1111_i32);
  EXPECT_EQ(-2222_i32 / -2_i32, 1111_i32);
  EXPECT_EQ(5_i32 / 2_i32, 2_i32);
  EXPECT_EQ(-5_i32 / 2_i32, -2_i32);

  auto x = 0_i32;
  x /= 123_i32;
  EXPECT_EQ(x, 0_i32);
  x = -2345_i32;
  x /= 1_i32;
  EXPECT_EQ(x, -2345_i32);
  x = 2345_i32;
  x /= 1_i32;
  EXPECT_EQ(x, 2345_i32);
  x = -2345_i32;
  x /= -1_i32;
  EXPECT_EQ(x, 2345_i32);
  x = 2345_i32;
  x /= -1_i32;
  EXPECT_EQ(x, -2345_i32);
  x = 2222_i32;
  x /= 2_i32;
  EXPECT_EQ(x, 1111_i32);
  x = -2222_i32;
  x /= 2_i32;
  EXPECT_EQ(x, -1111_i32);
  x = 2222_i32;
  x /= -2_i32;
  EXPECT_EQ(x, -1111_i32);
  x = -2222_i32;
  x /= -2_i32;
  EXPECT_EQ(x, 1111_i32);
  x = 5_i32;
  x /= 2_i32;
  EXPECT_EQ(x, 2_i32);
  x = -5_i32;
  x /= 2_i32;
  EXPECT_EQ(x, -2_i32);
}

TEST(i32DeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX() / 0_i32, "");
  EXPECT_DEATH(0_i32 / 0_i32, "");
  EXPECT_DEATH(1_i32 / 0_i32, "");
  EXPECT_DEATH(-1_i32 / 0_i32, "");
  EXPECT_DEATH(i32::MIN() / 0_i32, "");
  EXPECT_DEATH(i32::MIN() / -1_i32, "");

  auto x = i32::MAX();
  EXPECT_DEATH(x /= 0_i32, "");
  x = 0_i32;
  EXPECT_DEATH(x /= 0_i32, "");
  x = 1_i32;
  EXPECT_DEATH(x /= 0_i32, "");
  x = -1_i32;
  EXPECT_DEATH(x /= 0_i32, "");
  x = i32::MIN();
  EXPECT_DEATH(x /= 0_i32, "");
  x = i32::MIN();
  EXPECT_DEATH(x /= -1_i32, "");
#endif
}

TEST(i32, CheckedDiv) {
  [[maybe_unused]] constexpr auto a = (-1_i32).checked_div(3_i32);

  EXPECT_EQ((0_i32).checked_div(123_i32), Option<i32>::some(0_i32));
  EXPECT_EQ((-2345_i32).checked_div(1_i32), Option<i32>::some(-2345_i32));

  EXPECT_EQ(i32::MAX().checked_div(0_i32), None);
  EXPECT_EQ((0_i32).checked_div(0_i32), None);
  EXPECT_EQ((1_i32).checked_div(0_i32), None);
  EXPECT_EQ((-1_i32).checked_div(0_i32), None);
  EXPECT_EQ(i32::MIN().checked_div(0_i32), None);
  EXPECT_EQ(i32::MIN().checked_div(-1_i32), None);
}

TEST(i32, SaturatingDiv) {
  [[maybe_unused]] constexpr auto a = (-1_i32).saturating_div(3_i32);

  EXPECT_EQ((0_i32).saturating_div(123_i32), 0_i32);
  EXPECT_EQ((-2345_i32).saturating_div(1_i32), -2345_i32);

  EXPECT_EQ(i32::MIN().saturating_div(-1_i32), i32::MAX());
}

TEST(i32DeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX().saturating_div(0_i32), "");
  EXPECT_DEATH((0_i32).saturating_div(0_i32), "");
  EXPECT_DEATH((1_i32).saturating_div(0_i32), "");
  EXPECT_DEATH((-1_i32).saturating_div(0_i32), "");
  EXPECT_DEATH(i32::MIN().saturating_div(0_i32), "");
#endif
}

TEST(i32, WrappingDiv) {
  [[maybe_unused]] constexpr auto a = (-1_i32).wrapping_div(3_i32);

  EXPECT_EQ((0_i32).wrapping_div(123_i32), 0_i32);
  EXPECT_EQ((-2345_i32).wrapping_div(1_i32), -2345_i32);

  EXPECT_EQ(i32::MIN().wrapping_div(-1_i32), i32::MIN());
}

TEST(i32DeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX().wrapping_div(0_i32), "");
  EXPECT_DEATH((0_i32).wrapping_div(0_i32), "");
  EXPECT_DEATH((1_i32).wrapping_div(0_i32), "");
  EXPECT_DEATH((-1_i32).wrapping_div(0_i32), "");
  EXPECT_DEATH(i32::MIN().wrapping_div(0_i32), "");
#endif
}

TEST(i32, Mul) {
  [[maybe_unused]] constexpr auto a = (-1_i32) * (3_i32);

  EXPECT_EQ(0_i32 * 21_i32, 0_i32);
  EXPECT_EQ(21_i32 * 0_i32, 0_i32);
  EXPECT_EQ(0_i32 * -21_i32, 0_i32);
  EXPECT_EQ(-21_i32 * 0_i32, 0_i32);
  EXPECT_EQ(-0_i32 * 21_i32, 0_i32);
  EXPECT_EQ(21_i32 * -0_i32, 0_i32);
  EXPECT_EQ(-0_i32 * -21_i32, 0_i32);
  EXPECT_EQ(-21_i32 * -0_i32, 0_i32);
  EXPECT_EQ(1_i32 * 21_i32, 21_i32);
  EXPECT_EQ(21_i32 * 1_i32, 21_i32);
  EXPECT_EQ(-1_i32 * 21_i32, -21_i32);
  EXPECT_EQ(21_i32 * -1_i32, -21_i32);
  EXPECT_EQ(100_i32 * 21_i32, 2100_i32);
  EXPECT_EQ(21_i32 * 100_i32, 2100_i32);
  EXPECT_EQ(1_i32 * i32::MAX(), i32::MAX());
  EXPECT_EQ(i32::MIN() * 1_i32, i32::MIN());
  EXPECT_EQ(-1_i32 * i32::MAX(), i32::MIN() + 1_i32);

  auto x = 5_i32;
  x *= 20_i32;
  EXPECT_EQ(x, i32(20 * 5));
  x *= -4_i32;
  EXPECT_EQ(x, i32(20 * 5 * -4));
}

TEST(i32DeathTest, MulOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX() * 2, "");
  EXPECT_DEATH(i32::MAX() * -2, "");
  EXPECT_DEATH(i32::MIN() * 2, "");
  EXPECT_DEATH(i32::MIN() * -2, "");
  EXPECT_DEATH(i32::MIN() * -1, "");
#endif
}

TEST(i32, SaturatedMul) {
  [[maybe_unused]] constexpr auto a = (-1_i32).saturating_mul(3_i32);

  EXPECT_EQ((100_i32).saturating_mul(21_i32), 2100_i32);
  EXPECT_EQ((21_i32).saturating_mul(100_i32), 2100_i32);
  EXPECT_EQ((123456_i32).saturating_mul(23456_i32), i32::MAX());
  EXPECT_EQ((-123456_i32).saturating_mul(-23456_i32), i32::MAX());
  EXPECT_EQ((123456_i32).saturating_mul(-23456_i32), i32::MIN());
}

TEST(i32, UncheckedMul) {
  [[maybe_unused]] constexpr auto a = (-1_i32).unchecked_mul(unsafe_fn, 3_i32);

  EXPECT_EQ((100_i32).unchecked_mul(unsafe_fn, 21_i32), 2100_i32);
  EXPECT_EQ((21_i32).unchecked_mul(unsafe_fn, 100_i32), 2100_i32);
}

TEST(i32, WrappingMul) {
  [[maybe_unused]] constexpr auto a = (123456_i32).wrapping_mul(23456_i32);

  EXPECT_EQ((100_i32).wrapping_mul(21_i32), 2100_i32);
  EXPECT_EQ((21_i32).wrapping_mul(100_i32), 2100_i32);
  EXPECT_EQ((123456_i32).wrapping_mul(23456_i32), (-1399183360_i32));
  EXPECT_EQ((-123456_i32).wrapping_mul(-23456_i32), (-1399183360_i32));
  EXPECT_EQ((123456_i32).wrapping_mul(-23456_i32), 1399183360_i32);
}

TEST(i32, Neg) {
  [[maybe_unused]] constexpr auto a = -(1_i32);

  EXPECT_EQ(-(0_i32), i32(0));
  EXPECT_EQ(-(10_i32), i32(-10));
  EXPECT_EQ(-(-10_i32), i32(10));
  EXPECT_EQ(-i32::MAX(), i32::MIN() + i32(1));
  EXPECT_EQ(-(i32::MIN() + 1_i32), i32::MAX());
}

TEST(i32DeathTest, NegOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(-i32::MIN(), "");
#endif
}

TEST(i32, CheckedNeg) {
  [[maybe_unused]] constexpr auto a = (123456_i32).checked_neg();

  EXPECT_EQ(i32::MIN().checked_neg(), None);
  EXPECT_EQ(i32::MAX().checked_neg(), Option<i32>::some(i32::MIN() + i32(1)));
  EXPECT_EQ((0_i32).checked_neg(), Option<i32>::some(i32(0)));
  EXPECT_EQ((20_i32).checked_neg(), Option<i32>::some(i32(-20)));
}

TEST(i32, SaturatingNeg) {
  [[maybe_unused]] constexpr auto a = (123456_i32).saturating_neg();

  EXPECT_EQ(i32::MIN().saturating_neg(), i32::MAX());
  EXPECT_EQ(i32::MAX().saturating_neg(), i32::MIN() + i32(1));
  EXPECT_EQ((0_i32).saturating_neg(), i32(0));
  EXPECT_EQ((20_i32).saturating_neg(), i32(-20));
}

TEST(i32, WrappingNeg) {
  [[maybe_unused]] constexpr auto a = (123456_i32).wrapping_neg();

  EXPECT_EQ(i32::MIN().wrapping_neg(), i32::MIN());
  EXPECT_EQ(i32::MAX().wrapping_neg(), i32::MIN() + i32(1));
  EXPECT_EQ((0_i32).wrapping_neg(), i32(0));
  EXPECT_EQ((20_i32).wrapping_neg(), i32(-20));
}

TEST(i32, Rem) {
  [[maybe_unused]] constexpr auto a = -1_i32 % 3_i32;

  EXPECT_EQ(0_i32 % 123_i32, 0_i32);
  EXPECT_EQ(5_i32 % 2_i32, 1_i32);
  EXPECT_EQ(5_i32 % 1_i32, 0_i32);
  EXPECT_EQ(-5_i32 % 2_i32, -1_i32);
  EXPECT_EQ(-5_i32 % 1_i32, 0_i32);
  EXPECT_EQ(5_i32 % -2_i32, 1_i32);
  EXPECT_EQ(5_i32 % -1_i32, 0_i32);
  EXPECT_EQ(6_i32 % -1_i32, 0_i32);

  auto x = 0_i32;
  x %= 123_i32;
  EXPECT_EQ(x, 0_i32);
  x = 5_i32;
  x %= 2_i32;
  EXPECT_EQ(x, 1_i32);
  x = 5_i32;
  x %= 1_i32;
  EXPECT_EQ(x, 0_i32);
  x = -5_i32;
  x %= 2_i32;
  EXPECT_EQ(x, -1_i32);
  x = -5_i32;
  x %= 1_i32;
  EXPECT_EQ(x, 0_i32);
  x = 5_i32;
  x %= -2_i32;
  EXPECT_EQ(x, 1_i32);
  x = 5_i32;
  x %= -1_i32;
  EXPECT_EQ(x, 0_i32);
  x = 6_i32;
  x %= -1_i32;
  EXPECT_EQ(x, 0_i32);
}

TEST(i32DeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX() % 0_i32, "");
  EXPECT_DEATH(0_i32 % 0_i32, "");
  EXPECT_DEATH(1_i32 % 0_i32, "");
  EXPECT_DEATH(-1_i32 % 0_i32, "");
  EXPECT_DEATH(i32::MIN() % 0_i32, "");
  EXPECT_DEATH(i32::MIN() % -1_i32, "");

  auto x = i32::MAX();
  EXPECT_DEATH(x %= 0_i32, "");
  x = 0_i32;
  EXPECT_DEATH(x %= 0_i32, "");
  x = 1_i32;
  EXPECT_DEATH(x %= 0_i32, "");
  x = -1_i32;
  EXPECT_DEATH(x %= 0_i32, "");
  x = i32::MIN();
  EXPECT_DEATH(x %= 0_i32, "");
  x = i32::MIN();
  EXPECT_DEATH(x %= -1_i32, "");
#endif
}

TEST(i32, CheckedRem) {
  [[maybe_unused]] constexpr auto a = (-1_i32).checked_rem(3_i32);

  EXPECT_EQ((0_i32).checked_rem(123_i32), Option<i32>::some(0_i32));
  EXPECT_EQ((-2345_i32).checked_rem(5_i32), Option<i32>::some(i32(-2345 % 5)));

  EXPECT_EQ(i32::MAX().checked_rem(0_i32), None);
  EXPECT_EQ((0_i32).checked_rem(0_i32), None);
  EXPECT_EQ((1_i32).checked_rem(0_i32), None);
  EXPECT_EQ((-1_i32).checked_rem(0_i32), None);
  EXPECT_EQ(i32::MIN().checked_rem(0_i32), None);
  EXPECT_EQ(i32::MIN().checked_rem(-1_i32), None);
}

TEST(i32, WrappingRem) {
  [[maybe_unused]] constexpr auto a = (-1_i32).wrapping_rem(3_i32);

  EXPECT_EQ((0_i32).wrapping_rem(123_i32), i32(0));
  EXPECT_EQ((-2345_i32).wrapping_rem(5_i32), i32(-2345_i32 % 5));

  EXPECT_EQ(i32::MIN().wrapping_rem(-1_i32), i32(0));
}

TEST(i32DeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX().wrapping_rem(0_i32), "");
  EXPECT_DEATH((0_i32).wrapping_rem(0_i32), "");
  EXPECT_DEATH((1_i32).wrapping_rem(0_i32), "");
  EXPECT_DEATH((-1_i32).wrapping_rem(0_i32), "");
  EXPECT_DEATH(i32::MIN().wrapping_rem(0_i32), "");
#endif
}

TEST(i32, Shl) {
  [[maybe_unused]] constexpr auto a = (5_i32) << 1;

  EXPECT_EQ(2_i32 << 1, 4_i32);
  EXPECT_EQ(-2_i32 << 1,
            i32(static_cast<int32_t>(static_cast<uint32_t>(-2) << 1)));
  EXPECT_EQ(1_i32 << 31,
            i32(static_cast<int32_t>(static_cast<uint32_t>(1) << 31)));

  auto x = 2_i32;
  x <<= 1;
  EXPECT_EQ(x, 4_i32);
  x = -2_i32;
  x <<= 1;
  EXPECT_EQ(x, i32(static_cast<int32_t>(static_cast<uint32_t>(-2) << 1)));
}

TEST(i32DeathTest, ShlOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(0_i32 << 32, "");
  EXPECT_DEATH(1_i32 << 33, "");
  EXPECT_DEATH(2_i32 << 64, "");
#endif
}

TEST(i32, CheckedShl) {
  [[maybe_unused]] constexpr auto a = (5_i32).checked_shl(1);

  EXPECT_EQ((2_i32).checked_shl(1), Option<i32>::some(4_i32));
  EXPECT_EQ((-2_i32).checked_shl(1), Option<i32>::some(i32(static_cast<int32_t>(
                                         static_cast<uint32_t>(-2) << 1))));

  EXPECT_EQ((0_i32).checked_shl(32), None);
  EXPECT_EQ((1_i32).checked_shl(33), None);
  EXPECT_EQ((2_i32).checked_shl(64), None);
}

TEST(i32, WrappingShl) {
  [[maybe_unused]] constexpr auto a = (5_i32).wrapping_shl(1);

  EXPECT_EQ((2_i32).wrapping_shl(1), 4_i32);
  EXPECT_EQ((-2_i32).wrapping_shl(1),
            i32(static_cast<int32_t>(static_cast<uint32_t>(-2) << 1)));

  EXPECT_EQ((2_i32).wrapping_shl(32), 2_i32);  // Masks out everything.
  EXPECT_EQ((2_i32).wrapping_shl(33),
            4_i32);  // Masks out everything but the 1.
}

TEST(i32, Shr) {
  [[maybe_unused]] constexpr auto a = (5_i32) >> 1;

  EXPECT_EQ(4_i32 >> 1, 2_i32);
  EXPECT_EQ(-4_i32 >> 1,
            i32(static_cast<int32_t>(static_cast<uint32_t>(-4) >> 1)));
  EXPECT_EQ(-1_i32 >> 31, 1_i32);

  auto x = 4_i32;
  x >>= 1;
  EXPECT_EQ(x, 2_i32);
  x = -4_i32;
  x >>= 1;
  EXPECT_EQ(x, i32(static_cast<int32_t>(static_cast<uint32_t>(-4) >> 1)));
}

TEST(i32DeathTest, ShrOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(-1_i32 >> 32, "");
  EXPECT_DEATH(0_i32 >> 33, "");
  EXPECT_DEATH(1_i32 >> 64, "");
#endif
}

TEST(i32, CheckedShr) {
  [[maybe_unused]] constexpr auto a = (5_i32).checked_shr(1);

  EXPECT_EQ((4_i32).checked_shr(1), Option<i32>::some(2_i32));
  EXPECT_EQ((-2_i32).checked_shr(1), Option<i32>::some(i32(static_cast<int32_t>(
                                         static_cast<uint32_t>(-2) >> 1))));

  EXPECT_EQ((-1_i32).checked_shr(32), None);
  EXPECT_EQ((0_i32).checked_shr(33), None);
  EXPECT_EQ((1_i32).checked_shr(64), None);
}

TEST(i32, WrappingShr) {
  [[maybe_unused]] constexpr auto a = (5_i32).wrapping_shr(1);

  EXPECT_EQ((4_i32).wrapping_shr(1), 2_i32);
  EXPECT_EQ((-2_i32).wrapping_shr(1),
            i32(static_cast<int32_t>(static_cast<uint32_t>(-2) >> 1)));

  EXPECT_EQ((4_i32).wrapping_shr(32), 4_i32);  // Masks out everything.
  EXPECT_EQ((4_i32).wrapping_shr(33),
            2_i32);  // Masks out everything but the 1.
}

TEST(i32, Sub) {
  [[maybe_unused]] constexpr auto a = -1_i32 - 3_i32;

  EXPECT_EQ(0_i32 - 0_i32, 0_i32);
  EXPECT_EQ(12345_i32 - 12345_i32, 0_i32);
  EXPECT_EQ(-12345_i32 - 1_i32, -12346_i32);
  EXPECT_EQ(12345_i32 - 1_i32, 12344_i32);
  EXPECT_EQ(12345_i32 - (-1_i32), 12346_i32);
  EXPECT_EQ(i32::MAX() - i32::MAX(), 0_i32);
  EXPECT_EQ(i32::MIN() - i32::MIN(), 0_i32);
  EXPECT_EQ(0_i32 - (i32::MIN() + 1), i32::MAX());

  auto x = 0_i32;
  x -= 0_i32;
  EXPECT_EQ(x, 0_i32);
  x = 12345_i32;
  x -= 345_i32;
  EXPECT_EQ(x, 12000_i32);
  x -= -345_i32;
  EXPECT_EQ(x, 12345_i32);
}

TEST(i32DeathTest, SubOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX() - (-1_i32), "");
  EXPECT_DEATH(i32::MAX() - i32::MIN(), "");
  EXPECT_DEATH(i32::MIN() - 1_i32, "");
  EXPECT_DEATH(i32::MIN() - i32::MAX(), "");
#endif
}

TEST(i32, CheckedSub) {
  [[maybe_unused]] constexpr auto a = (-1_i32).checked_sub(3_i32);

  EXPECT_EQ((0_i32).checked_sub(0_i32).unwrap(), 0_i32);
  EXPECT_EQ((-12345_i32).checked_sub(-12345_i32).unwrap(), 0_i32);

  EXPECT_EQ(i32::MAX().checked_sub(-1_i32), None);
  EXPECT_EQ(i32::MIN().checked_sub(1_i32), None);
  EXPECT_EQ(i32::MIN().checked_sub(2_i32), None);
  EXPECT_EQ((-2_i32).checked_sub(i32::MAX()), None);
  EXPECT_EQ((1_i32).checked_sub(-i32::MAX()), None);
  EXPECT_EQ(i32::MIN().checked_sub(i32::MAX()), None);
}

TEST(i32, SaturatingSub) {
  [[maybe_unused]] constexpr auto a = (-1_i32).saturating_sub(3_i32);

  EXPECT_EQ((0_i32).saturating_sub(0_i32), 0_i32);
  EXPECT_EQ((-12345_i32).saturating_sub(-12345_i32), 0_i32);

  EXPECT_EQ(i32::MAX().saturating_sub(-1_i32), i32::MAX());
  EXPECT_EQ(i32::MIN().saturating_sub(1_i32), i32::MIN());
  EXPECT_EQ(i32::MIN().saturating_sub(2_i32), i32::MIN());
  EXPECT_EQ((-2_i32).saturating_sub(i32::MAX()), i32::MIN());
  EXPECT_EQ((1_i32).saturating_sub(-i32::MAX()), i32::MAX());
  EXPECT_EQ(i32::MIN().saturating_sub(i32::MAX()), i32::MIN());
}

TEST(i32, UncheckedSub) {
  [[maybe_unused]] constexpr auto a = (-1_i32).unchecked_sub(unsafe_fn, 3_i32);

  EXPECT_EQ((0_i32).unchecked_sub(unsafe_fn, 0_i32), 0_i32);
  EXPECT_EQ((12345_i32).unchecked_sub(unsafe_fn, 12345_i32), 0_i32);
  EXPECT_EQ((-12345_i32).unchecked_sub(unsafe_fn, 1_i32), -12346_i32);
  EXPECT_EQ((12345_i32).unchecked_sub(unsafe_fn, 1_i32), 12344_i32);
  EXPECT_EQ((12345_i32).unchecked_sub(unsafe_fn, -1_i32), 12346_i32);
  EXPECT_EQ(i32::MAX().unchecked_sub(unsafe_fn, i32::MAX()), 0_i32);
  EXPECT_EQ(i32::MIN().unchecked_sub(unsafe_fn, i32::MIN()), 0_i32);
  EXPECT_EQ((0_i32).unchecked_sub(unsafe_fn, i32::MIN() + 1), i32::MAX());
}

TEST(i32, WrappingSub) {
  [[maybe_unused]] constexpr auto a = (-1_i32).wrapping_sub(3_i32);

  EXPECT_EQ((0_i32).wrapping_sub(0_i32), 0_i32);
  EXPECT_EQ((-12345_i32).wrapping_sub(-12345_i32), 0_i32);

  EXPECT_EQ(i32::MAX().wrapping_sub(-1_i32), i32::MIN());
  EXPECT_EQ(i32::MIN().wrapping_sub(1_i32), i32::MAX());
  EXPECT_EQ(i32::MIN().wrapping_sub(2_i32), i32::MAX() - 1_i32);
  EXPECT_EQ((-2_i32).wrapping_sub(i32::MAX()), i32::MAX());
  EXPECT_EQ((1_i32).wrapping_sub(-i32::MAX()), i32::MIN());
  EXPECT_EQ(i32::MIN().wrapping_sub(i32::MAX()), 1_i32);
}

TEST(i32, CountOnes) {
  EXPECT_EQ(
      []() constexpr { return (7_i32).count_ones(); }(), 3);
  EXPECT_EQ(
      []() constexpr { return (0_i32).count_ones(); }(), 0);
  EXPECT_EQ(
      []() constexpr { return (-1_i32).count_ones(); }(), 32);

  EXPECT_EQ((7_i32).count_ones(), 3);
  EXPECT_EQ((0_i32).count_ones(), 0);
  EXPECT_EQ((-1_i32).count_ones(), 32);
}

TEST(i32, CountZeros) {
  EXPECT_EQ(
      []() constexpr { return (7_i32).count_zeros(); }(), 32 - 3);
  EXPECT_EQ(
      []() constexpr { return (0_i32).count_zeros(); }(), 32);
  EXPECT_EQ(
      []() constexpr { return (-1_i32).count_zeros(); }(), 0);

  EXPECT_EQ((7_i32).count_zeros(), 32 - 3);
  EXPECT_EQ((0_i32).count_zeros(), 32);
  EXPECT_EQ((-1_i32).count_zeros(), 0);
}

TEST(i32, IsNegative) {
  EXPECT_EQ(
      []() constexpr { return (7_i32).is_negative(); }(), false);
  EXPECT_EQ(
      []() constexpr { return (0_i32).is_negative(); }(), false);
  EXPECT_EQ(
      []() constexpr { return (-1_i32).is_negative(); }(), true);

  EXPECT_EQ((7_i32).is_negative(), false);
  EXPECT_EQ((0_i32).is_negative(), false);
  EXPECT_EQ((-1_i32).is_negative(), true);
}

TEST(i32, IsPositive) {
  EXPECT_EQ(
      []() constexpr { return (7_i32).is_positive(); }(), true);
  EXPECT_EQ(
      []() constexpr { return (0_i32).is_positive(); }(), false);
  EXPECT_EQ(
      []() constexpr { return (-1_i32).is_positive(); }(), false);

  EXPECT_EQ((7_i32).is_positive(), true);
  EXPECT_EQ((0_i32).is_positive(), false);
  EXPECT_EQ((-1_i32).is_positive(), false);
}

TEST(i32, LeadingZeros) {
  EXPECT_EQ(
      []() constexpr { return (0_i32).leading_zeros(); }(), 32);
  EXPECT_EQ(
      []() constexpr { return (1_i32).leading_zeros(); }(), 31);
  EXPECT_EQ(
      []() constexpr { return (3_i32).leading_zeros(); }(), 30);
  EXPECT_EQ(
      []() constexpr { return (i32::MAX()).leading_zeros(); }(), 1);
  EXPECT_EQ(
      []() constexpr { return (-1_i32).leading_zeros(); }(), 0);

  EXPECT_EQ((0_i32).leading_zeros(), 32);
  EXPECT_EQ((1_i32).leading_zeros(), 31);
  EXPECT_EQ((3_i32).leading_zeros(), 30);
  EXPECT_EQ((i32::MAX()).leading_zeros(), 1);
  EXPECT_EQ((-1_i32).leading_zeros(), 0);
}

TEST(i32, LeadingOnes) {
  EXPECT_EQ(
      []() constexpr { return (0_i32).leading_ones(); }(), 0);
  EXPECT_EQ(
      []() constexpr { return (1_i32).leading_ones(); }(), 0);
  EXPECT_EQ(
      []() constexpr { return (i32::MAX()).leading_ones(); }(), 0);
  EXPECT_EQ(
      []() constexpr { return (-1_i32).leading_ones(); }(), 32);
  EXPECT_EQ(
      []() constexpr { return (-2_i32).leading_ones(); }(), 31);

  EXPECT_EQ((0_i32).leading_ones(), 0);
  EXPECT_EQ((1_i32).leading_ones(), 0);
  EXPECT_EQ((i32::MAX()).leading_ones(), 0);
  EXPECT_EQ((-1_i32).leading_ones(), 32);
  EXPECT_EQ((-2_i32).leading_ones(), 31);
}

TEST(i32, TrailingZeros) {
  EXPECT_EQ(
      []() constexpr { return (0_i32).trailing_zeros(); }(), 32);
  EXPECT_EQ(
      []() constexpr { return (1_i32).trailing_zeros(); }(), 0);
  EXPECT_EQ(
      []() constexpr { return (2_i32).trailing_zeros(); }(), 1);
  EXPECT_EQ(
      []() constexpr { return (i32::MIN()).trailing_zeros(); }(), 31);
  EXPECT_EQ(
      []() constexpr { return (-1_i32).trailing_zeros(); }(), 0);

  EXPECT_EQ((0_i32).trailing_zeros(), 32);
  EXPECT_EQ((1_i32).trailing_zeros(), 0);
  EXPECT_EQ((2_i32).trailing_zeros(), 1);
  EXPECT_EQ((i32::MIN()).trailing_zeros(), 31);
  EXPECT_EQ((-1_i32).trailing_zeros(), 0);
}

TEST(i32, TrailingOnes) {
  EXPECT_EQ(
      []() constexpr { return (0_i32).trailing_ones(); }(), 0);
  EXPECT_EQ(
      []() constexpr { return (1_i32).trailing_ones(); }(), 1);
  EXPECT_EQ(
      []() constexpr { return (3_i32).trailing_ones(); }(), 2);
  EXPECT_EQ(
      []() constexpr { return (i32::MAX()).trailing_ones(); }(), 31);
  EXPECT_EQ(
      []() constexpr { return (-1_i32).trailing_ones(); }(), 32);

  EXPECT_EQ((0_i32).trailing_ones(), 0);
  EXPECT_EQ((1_i32).trailing_ones(), 1);
  EXPECT_EQ((3_i32).trailing_ones(), 2);
  EXPECT_EQ((i32::MAX()).trailing_ones(), 31);
  EXPECT_EQ((-1_i32).trailing_ones(), 32);
}

TEST(i32, Pow) {
  constexpr auto a = (2_i32).pow(5);

  EXPECT_EQ((2_i32).pow(5), 32_i32);
  EXPECT_EQ((2_i32).pow(0), 1_i32);
  EXPECT_EQ((2_i32).pow(1), 2_i32);
  EXPECT_EQ((2_i32).pow(30), i32(1 << 30));
  EXPECT_EQ((1_i32).pow(/* TODO: u32::MAX() */ 1000000), 1_i32);
  EXPECT_EQ((i32::MAX()).pow(1), i32::MAX());
  EXPECT_EQ((i32::MAX()).pow(0), 1_i32);
}

TEST(i32DeathTest, PowOverflow) {
#if GTEST_HAS_DEATH_TEST
  // Crashes on the final acc * base.
  EXPECT_DEATH((2_i32).pow(31), "");
  // Crashes on base * base.
  EXPECT_DEATH((i32::MAX()).pow(31), "");
  // Crashes on acc * base inside the exponent loop.
  EXPECT_DEATH((2_i32).pow((1 << 30) - 1), "");
#endif
}

TEST(i32, CheckedPow) {
  constexpr auto a = (2_i32).checked_pow(5);

  EXPECT_EQ((2_i32).checked_pow(5), Option<i32>::some(32_i32));
  EXPECT_EQ((2_i32).checked_pow(0), Option<i32>::some(1_i32));
  EXPECT_EQ((2_i32).checked_pow(1), Option<i32>::some(2_i32));
  EXPECT_EQ((2_i32).checked_pow(30), Option<i32>::some(i32(1 << 30)));
  EXPECT_EQ((1_i32).checked_pow(/* TODO: u32::MAX() */ 1000000),
            Option<i32>::some(1_i32));
  EXPECT_EQ((i32::MAX()).checked_pow(1), Option<i32>::some(i32::MAX()));
  EXPECT_EQ((i32::MAX()).checked_pow(0), Option<i32>::some(1_i32));

  // Fails on the final acc * base.
  EXPECT_EQ((2_i32).checked_pow(31), None);
  // Fails on base * base.
  EXPECT_EQ((i32::MAX()).checked_pow(31), None);
  // Fails on acc * base inside the exponent loop.
  EXPECT_EQ((2_i32).checked_pow((1 << 30) - 1), None);
}

TEST(i32, ReverseBits) {
  EXPECT_EQ(
      []() constexpr { return (0_i32).reverse_bits(); }(), 0_i32);
  EXPECT_EQ(
      []() constexpr { return (2_i32).reverse_bits(); }(), i32(1 << 30));
  EXPECT_EQ(
      []() constexpr { return (i32(0x00f8f800)).reverse_bits(); }(),
      i32(0x001f1f00));
  EXPECT_EQ(
      []() constexpr { return (i32(-1)).reverse_bits(); }(), i32(-1));
  EXPECT_EQ(
      []() constexpr { return (i32(1)).reverse_bits(); }().primitive_value,
      i32::MIN());

  EXPECT_EQ((0_i32).reverse_bits(), 0_i32);
  EXPECT_EQ((2_i32).reverse_bits(), i32(1 << 30));
  EXPECT_EQ((i32(0x00f8f800)).reverse_bits(), i32(0x001f1f00));
  EXPECT_EQ((i32(-1)).reverse_bits(), i32(-1));
  EXPECT_EQ((i32(1)).reverse_bits().primitive_value, i32::MIN());
}

TEST(i32, RotateLeft) {
  constexpr auto a = (3_i32).rotate_left(2);

  EXPECT_EQ((1_i32).rotate_left(1), 2_i32);
  EXPECT_EQ((1_i32).rotate_left(4), 16_i32);
  EXPECT_EQ((1_i32).rotate_left(31), i32::MIN());
  EXPECT_EQ((1_i32).rotate_left(32), 1_i32);
  EXPECT_EQ((1_i32).rotate_left(63), i32::MIN());
  EXPECT_EQ((1_i32).rotate_left(64), 1_i32);
}

TEST(i32, RotateRight) {
  constexpr auto a = (3_i32).rotate_right(2);

  EXPECT_EQ((2_i32).rotate_right(1), 1_i32);
  EXPECT_EQ((16_i32).rotate_right(4), 1_i32);
  EXPECT_EQ((1_i32).rotate_right(1), i32::MIN());
  EXPECT_EQ((1_i32).rotate_right(32), 1_i32);
  EXPECT_EQ((1_i32).rotate_right(33), i32::MIN());
  EXPECT_EQ((1_i32).rotate_right(64), 1_i32);
  EXPECT_EQ((1_i32).rotate_right(65), i32::MIN());
}

TEST(i32, Signum) {
  EXPECT_EQ(
      []() constexpr { return (i32(10_i32)).signum(); }(), i32(1));
  EXPECT_EQ(
      []() constexpr { return (i32(0_i32)).signum(); }(), i32(0));
  EXPECT_EQ(
      []() constexpr { return (i32(-7_i32)).signum(); }(), i32(-1));

  EXPECT_EQ((i32(10_i32)).signum(), i32(1));
  EXPECT_EQ((i32(0_i32)).signum(), i32(0));
  EXPECT_EQ((i32(-7_i32)).signum(), i32(-1));
}

TEST(i32, SwapBytes) {
  EXPECT_EQ(
      []() constexpr { return (i32(0x12345678_i32)).swap_bytes(); }(),
      0x78563412_i32);

  EXPECT_EQ((0x12345678_i32).swap_bytes(), 0x78563412_i32);
  EXPECT_EQ((0_i32).swap_bytes(), 0_i32);
  EXPECT_EQ((-1_i32).swap_bytes(), -1_i32);
  EXPECT_EQ((i32::MIN()).swap_bytes(), (0x80_i32));
}

}  // namespace
