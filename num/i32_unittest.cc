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

#include <type_traits>

#include "construct/into.h"
#include "construct/make_default.h"
#include "mem/relocate.h"
#include "num/num_concepts.h"
#include "num/types.h"
#include "option/option.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

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
static_assert(sus::construct::MakeDefault<T>, "");
static_assert(sus::mem::relocate_one_by_memcpy<T>, "");
static_assert(sus::mem::relocate_array_by_memcpy<T>, "");
}  // namespace behaviour

// i32::MAX()
static_assert(i32::MAX().primitive_value == 0x7fffffff);
template <class T>
concept MaxInRange = requires {
                       { 0x7fffffff_i32 } -> std::same_as<T>;
                       { i32(0x7fffffff) } -> std::same_as<T>;
                     };
static_assert(MaxInRange<i32>);

TEST(i32, Traits) {
  // ** Signed only **
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
  constexpr i32 c =
      1_i32 + 2_i32 - 3_i32 * 4_i32 / 5_i32 % 6_i32 & 7_i32 | 8_i32 ^ -9_i32;
  constexpr std::strong_ordering o = 2_i32 <=> 3_i32;
}

TEST(i32, Literals) {
  // Hex.
  static_assert((0x123abC_i32).primitive_value == 0x123abC);
  static_assert((0X123abC_i32).primitive_value == 0X123abC);
  static_assert((0X00123abC_i32).primitive_value == 0X123abC);
  EXPECT_EQ((0x123abC_i32).primitive_value, 0x123abC);
  EXPECT_EQ((0X123abC_i32).primitive_value, 0X123abC);
  EXPECT_EQ((0X00123abC_i32).primitive_value, 0X123abC);
  // Binary.
  static_assert((0b101_i32).primitive_value == 0b101);
  static_assert((0B101_i32).primitive_value == 0B101);
  static_assert((0b00101_i32).primitive_value == 0b101);
  EXPECT_EQ((0b101_i32).primitive_value, 0b101);
  EXPECT_EQ((0B101_i32).primitive_value, 0B101);
  EXPECT_EQ((0b00101_i32).primitive_value, 0b101);
  // Octal.
  static_assert((0123_i32).primitive_value == 0123);
  static_assert((000123_i32).primitive_value == 0123);
  EXPECT_EQ((0123_i32).primitive_value, 0123);
  EXPECT_EQ((000123_i32).primitive_value, 0123);
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
}

TEST(i32, Constants) {
  constexpr auto max = i32::MAX();
  static_assert(std::same_as<decltype(max), const i32>);
  EXPECT_EQ(max.primitive_value, 0x7fffffff);
  constexpr auto min = i32::MIN();
  static_assert(std::same_as<decltype(min), const i32>);
  EXPECT_EQ(min.primitive_value, -0x7fffffff - 1);
  constexpr auto bits = i32::BITS();
  static_assert(std::same_as<decltype(bits), const u32>);
  EXPECT_EQ(bits, 32u);
}

TEST(i32, From) {
  static_assert(sus::construct::From<i32, char>);
  static_assert(sus::construct::From<i32, size_t>);
  static_assert(sus::construct::From<i32, int8_t>);
  static_assert(sus::construct::From<i32, int16_t>);
  static_assert(sus::construct::From<i32, int32_t>);
  static_assert(sus::construct::From<i32, int64_t>);
  static_assert(sus::construct::From<i32, uint8_t>);
  static_assert(sus::construct::From<i32, uint16_t>);
  static_assert(sus::construct::From<i32, uint32_t>);
  static_assert(sus::construct::From<i32, uint64_t>);

  EXPECT_EQ(i32::from(char{2}), 2_i32);
  EXPECT_EQ(i32::from(size_t{2}), 2_i32);
  EXPECT_EQ(i32::from(int8_t{2}), 2_i32);
  EXPECT_EQ(i32::from(int16_t{2}), 2_i32);
  EXPECT_EQ(i32::from(int32_t{2}), 2_i32);
  EXPECT_EQ(i32::from(int64_t{2}), 2_i32);
  EXPECT_EQ(i32::from(uint8_t{2}), 2_i32);
  EXPECT_EQ(i32::from(uint16_t{2}), 2_i32);
  EXPECT_EQ(i32::from(uint32_t{2}), 2_i32);
  EXPECT_EQ(i32::from(uint64_t{2}), 2_i32);

  static_assert(sus::construct::From<i32, i8>);
  static_assert(sus::construct::From<i32, i16>);
  static_assert(sus::construct::From<i32, i32>);
  static_assert(sus::construct::From<i32, i64>);
  static_assert(sus::construct::From<i32, isize>);
  static_assert(sus::construct::From<i32, u8>);
  static_assert(sus::construct::From<i32, u16>);
  static_assert(sus::construct::From<i32, u32>);
  static_assert(sus::construct::From<i32, u64>);
  static_assert(sus::construct::From<i32, usize>);

  EXPECT_EQ(i32::from(2_i8), 2_i32);
  EXPECT_EQ(i32::from(2_i16), 2_i32);
  EXPECT_EQ(i32::from(2_i32), 2_i32);
  EXPECT_EQ(i32::from(2_i64), 2_i32);
  EXPECT_EQ(i32::from(2_isize), 2_i32);
  EXPECT_EQ(i32::from(2_u8), 2_i32);
  EXPECT_EQ(i32::from(2_u16), 2_i32);
  EXPECT_EQ(i32::from(2_u32), 2_i32);
  EXPECT_EQ(i32::from(2_u64), 2_i32);
  EXPECT_EQ(i32::from(2_usize), 2_i32);
}

TEST(i32DeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::from(int64_t{-1 - 0x7fff'ffff'ffff'ffff}), "");
  EXPECT_DEATH(i32::from(uint64_t{0xffff'ffff'ffff'ffff}), "");

  EXPECT_DEATH(i32::from(i64::MAX()), "");
  EXPECT_DEATH(i32::from(u32::MAX()), "");
  EXPECT_DEATH(i32::from(u64::MAX()), "");
  EXPECT_DEATH(i32::from(usize::MAX()), "");
#endif
}

// ** Signed only
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

// ** Signed only
TEST(i32DeathTest, Abs) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH([[maybe_unused]] auto x = i32::MIN().abs(), "");
#endif
}

// ** Signed only
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

// ** Signed only
TEST(i32, OverflowingAbs) {
  [[maybe_unused]] constexpr auto a = i32(-1).overflowing_abs();

  EXPECT_EQ(i32(1234567).overflowing_abs(),
            (Tuple<i32, bool>::with(1234567_i32, false)));
  EXPECT_EQ(i32(-1234567).overflowing_abs(),
            (Tuple<i32, bool>::with(1234567_i32, false)));
  EXPECT_EQ(i32::MAX().overflowing_abs(),
            (Tuple<i32, bool>::with(i32::MAX(), false)));
  EXPECT_EQ((i32::MIN() + 1_i32).overflowing_abs(),
            (Tuple<i32, bool>::with(i32::MAX(), false)));
  EXPECT_EQ((i32::MIN()).overflowing_abs(),
            (Tuple<i32, bool>::with(i32::MIN(), true)));

  // lvalue.
  auto i = -9000_i32;
  EXPECT_EQ(i.overflowing_abs(), (Tuple<i32, bool>::with(9000_i32, false)));
}

// ** Signed only
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

// ** Signed only
TEST(i32, UnsignedAbs) {
  constexpr auto a = i32(-1).unsigned_abs();
  EXPECT_EQ(a, 1_u32);

  EXPECT_EQ(i32(1234567).unsigned_abs(), 1234567_u32);
  EXPECT_EQ(i32(-1234567).unsigned_abs(), 1234567_u32);
  EXPECT_EQ(i32::MAX().unsigned_abs(), 0x7fffffff_u32);
  EXPECT_EQ((i32::MIN() + 1_i32).unsigned_abs(), 0x7fffffff_u32);
  EXPECT_EQ((i32::MIN()).unsigned_abs(), 0x80000000_u32);

  // lvalue.
  auto i = -9000_i32;
  EXPECT_EQ(i.unsigned_abs(), 9000_u32);
}

// ** Signed only
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
  constexpr auto a = (-1_i32).abs_diff(10_i32);
  EXPECT_EQ(a, 11_u32);

  EXPECT_EQ((0_i32).abs_diff(0_i32), 0_u32);
  EXPECT_EQ((0_i32).abs_diff(123456_i32), 123456_u32);
  EXPECT_EQ((123456_i32).abs_diff(0_i32), 123456_u32);
  EXPECT_EQ((123456_i32).abs_diff(123456_i32), 0_u32);
  EXPECT_EQ(i32::MAX().abs_diff(i32::MIN()), 0xffffffff_u32);
  EXPECT_EQ(i32::MIN().abs_diff(i32::MAX()), 0xffffffff_u32);

  // ** Signed only
  EXPECT_EQ(i32(0).abs_diff(-123456_i32), 123456_u32);
  EXPECT_EQ(i32(-123456).abs_diff(0_i32), 123456_u32);
  EXPECT_EQ(i32(-123456).abs_diff(-123456_i32), 0_u32);

  // lvalue.
  auto i = 9000_i32;
  auto j = 1000_i32;
  EXPECT_EQ(i.abs_diff(j), 8000_u32);
}

TEST(i32, Add) {
  constexpr auto a = 1_i32 + 3_i32;
  EXPECT_EQ(a, 4_i32);

  EXPECT_EQ(0_i32 + 0_i32, 0_i32);
  EXPECT_EQ(12345_i32 + 1_i32, 12346_i32);
  EXPECT_EQ(i32::MAX() + 0_i32, i32::MAX());
  EXPECT_EQ(i32::MIN() + 0_i32, i32::MIN());
  EXPECT_EQ(i32::MIN() + 1_i32, i32(i32::MIN_PRIMITIVE + 1));

  // ** Signed only
  EXPECT_EQ(-12345_i32 + 12345_i32, 0_i32);
  EXPECT_EQ(-12345_i32 + 1_i32, -12344_i32);
  EXPECT_EQ(12345_i32 + -1_i32, 12344_i32);
  EXPECT_EQ(i32::MAX() + -1_i32, i32(i32::MAX_PRIMITIVE - 1));
  EXPECT_EQ(i32::MIN() + i32::MAX(), -1_i32);
  EXPECT_EQ(i32::MAX() + i32::MIN(), -1_i32);

  auto x = 0_i32;
  x += 0_i32;
  EXPECT_EQ(x, 0_i32);
  x = 12345_i32;
  x += 1_i32;
  EXPECT_EQ(x, 12346_i32);
  x = i32::MAX();
  x += 0_i32;
  EXPECT_EQ(x, i32::MAX());
  x = i32::MIN();
  x += 0_i32;
  EXPECT_EQ(x, i32::MIN());
  x = i32::MIN();
  x += 1_i32;
  EXPECT_EQ(x, i32(i32::MIN_PRIMITIVE + 1));

  // ** Signed only
  x = -12345_i32;
  x += 12345_i32;
  EXPECT_EQ(x, 0_i32);
  x = -12345_i32;
  x += 1_i32;
  EXPECT_EQ(x, -12344_i32);
  x = 12345_i32;
  x += -1_i32;
  EXPECT_EQ(x, 12344_i32);
  x = i32::MAX();
  x += -1_i32;
  EXPECT_EQ(x, i32(i32::MAX_PRIMITIVE - 1));
}

TEST(i32DeathTest, AddOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX() + 1_i32, "");
  EXPECT_DEATH(i32::MAX() + i32::MAX(), "");

  // ** Signed only.
  EXPECT_DEATH(i32::MIN() + -1_i32, "");
  EXPECT_DEATH(i32::MIN() + i32::MIN(), "");
#endif
}

TEST(i32, CheckedAdd) {
  constexpr auto a = (1_i32).checked_add(3_i32);
  EXPECT_EQ(a, Option<i32>::some(4_i32));

  EXPECT_EQ((0_i32).checked_add(0_i32).unwrap(), 0_i32);

  EXPECT_EQ(i32::MAX().checked_add(1_i32), None);
  EXPECT_EQ((1_i32).checked_add(i32::MAX()), None);
  EXPECT_EQ(i32::MAX().checked_add(i32::MAX()), None);

  // ** Signed only.
  EXPECT_EQ((-12345_i32).checked_add(12345_i32).unwrap(), 0_i32);
  EXPECT_EQ(i32::MIN().checked_add(-1_i32), None);
  EXPECT_EQ((-1_i32).checked_add(i32::MIN()), None);
  EXPECT_EQ(i32::MIN().checked_add(i32::MIN()), None);
}

TEST(i32, OverflowingAdd) {
  constexpr auto a = (1_i32).overflowing_add(3_i32);
  EXPECT_EQ(a, (Tuple<i32, bool>::with(4_i32, false)));

  EXPECT_EQ((0_i32).overflowing_add(0_i32),
            (Tuple<i32, bool>::with(0_i32, false)));

  EXPECT_EQ(i32::MAX().overflowing_add(1_i32),
            (Tuple<i32, bool>::with(i32::MIN(), true)));
  EXPECT_EQ(i32::MAX().overflowing_add(2_i32),
            (Tuple<i32, bool>::with(i32::MIN() + 1_i32, true)));
  EXPECT_EQ((2_i32).overflowing_add(i32::MAX()),
            (Tuple<i32, bool>::with(i32::MIN() + 1_i32, true)));
  EXPECT_EQ(i32::MAX().overflowing_add(i32::MAX()),
            (Tuple<i32, bool>::with(i32::MIN() + i32::MAX() - 1_i32, true)));

  // ** Signed only
  EXPECT_EQ((-12345_i32).overflowing_add(12345_i32),
            (Tuple<i32, bool>::with(0_i32, false)));
  EXPECT_EQ(i32::MIN().overflowing_add(-1_i32),
            (Tuple<i32, bool>::with(i32::MAX(), true)));
  EXPECT_EQ(i32::MIN().overflowing_add(-2_i32),
            (Tuple<i32, bool>::with(i32::MAX() - 1_i32, true)));
  EXPECT_EQ((-2_i32).overflowing_add(i32::MIN()),
            (Tuple<i32, bool>::with(i32::MAX() - 1_i32, true)));
  EXPECT_EQ(i32::MIN().overflowing_add(i32::MIN()),
            (Tuple<i32, bool>::with(0_i32, true)));
}

TEST(i32, SaturatingAdd) {
  constexpr auto a = (1_i32).saturating_add(3_i32);
  EXPECT_EQ(a, 4_i32);

  EXPECT_EQ((0_i32).saturating_add(0_i32), 0_i32);

  EXPECT_EQ(i32::MAX().saturating_add(1_i32), i32::MAX());
  EXPECT_EQ((1_i32).saturating_add(i32::MAX()), i32::MAX());
  EXPECT_EQ(i32::MAX().saturating_add(i32::MAX()), i32::MAX());

  // ** Signed only.
  EXPECT_EQ((-12345_i32).saturating_add(12345_i32), 0_i32);
  EXPECT_EQ(i32::MIN().saturating_add(-1_i32), i32::MIN());
  EXPECT_EQ((-1_i32).saturating_add(i32::MIN()), i32::MIN());
  EXPECT_EQ(i32::MIN().saturating_add(i32::MIN()), i32::MIN());
}

TEST(i32, UncheckedAdd) {
  constexpr auto a = (1_i32).unchecked_add(unsafe_fn, 3_i32);
  EXPECT_EQ(a, 4_i32);

  EXPECT_EQ((0_i32).unchecked_add(unsafe_fn, 0_i32), 0_i32);
  EXPECT_EQ((12345_i32).unchecked_add(unsafe_fn, 1_i32), 12346_i32);
  EXPECT_EQ(i32::MAX().unchecked_add(unsafe_fn, 0_i32), i32::MAX());
  EXPECT_EQ(i32::MIN().unchecked_add(unsafe_fn, 0_i32), i32::MIN());
  EXPECT_EQ(i32::MIN().unchecked_add(unsafe_fn, 1_i32),
            i32(i32::MIN_PRIMITIVE + 1));
  EXPECT_EQ(i32::MIN().unchecked_add(unsafe_fn, i32::MAX()),
            i32::MIN() + i32::MAX());
  EXPECT_EQ(i32::MAX().unchecked_add(unsafe_fn, i32::MIN()),
            i32::MIN() + i32::MAX());

  // ** Signed only.
  EXPECT_EQ((-12345_i32).unchecked_add(unsafe_fn, 12345_i32), 0_i32);
  EXPECT_EQ((-12345_i32).unchecked_add(unsafe_fn, 1_i32), -12344_i32);
  EXPECT_EQ((12345_i32).unchecked_add(unsafe_fn, -1_i32), 12344_i32);
  EXPECT_EQ(i32::MAX().unchecked_add(unsafe_fn, -1_i32),
            i32(i32::MAX_PRIMITIVE - 1));
}

TEST(i32, WrappingAdd) {
  constexpr auto a = (1_i32).wrapping_add(3_i32);
  EXPECT_EQ(a, 4_i32);

  EXPECT_EQ((0_i32).wrapping_add(0_i32), 0_i32);

  EXPECT_EQ(i32::MAX().wrapping_add(1_i32), i32::MIN());
  EXPECT_EQ(i32::MAX().wrapping_add(2_i32), i32::MIN() + 1_i32);
  EXPECT_EQ((2_i32).wrapping_add(i32::MAX()), i32::MIN() + 1_i32);
  EXPECT_EQ(i32::MAX().wrapping_add(i32::MAX()),
            i32::MIN() + i32::MAX() - 1_i32);

  // ** Signed only.
  EXPECT_EQ((-12345_i32).wrapping_add(12345_i32), 0_i32);
  EXPECT_EQ(i32::MIN().wrapping_add(-1_i32), i32::MAX());
  EXPECT_EQ(i32::MIN().wrapping_add(-2_i32), i32::MAX() - 1_i32);
  EXPECT_EQ((-2_i32).wrapping_add(i32::MIN()), (i32::MAX() - 1_i32));
  EXPECT_EQ(i32::MIN().wrapping_add(i32::MIN()), 0_i32);
}

TEST(i32, Div) {
  constexpr auto a = 4_i32 / 2_i32;
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ(0_i32 / 123_i32, 0_i32);
  EXPECT_EQ(2345_i32 / 1_i32, 2345_i32);
  EXPECT_EQ(2222_i32 / 2_i32, 1111_i32);
  EXPECT_EQ(5_i32 / 2_i32, 2_i32);

  // ** Signed only.
  EXPECT_EQ(-2345_i32 / 1_i32, -2345_i32);
  EXPECT_EQ(-2345_i32 / -1_i32, 2345_i32);
  EXPECT_EQ(2345_i32 / -1_i32, -2345_i32);
  EXPECT_EQ(-2222_i32 / 2_i32, -1111_i32);
  EXPECT_EQ(2222_i32 / -2_i32, -1111_i32);
  EXPECT_EQ(-2222_i32 / -2_i32, 1111_i32);
  EXPECT_EQ(-5_i32 / 2_i32, -2_i32);

  auto x = 0_i32;
  x /= 123_i32;
  EXPECT_EQ(x, 0_i32);
  x = 2345_i32;
  x /= 1_i32;
  EXPECT_EQ(x, 2345_i32);
  x = 2222_i32;
  x /= 2_i32;
  EXPECT_EQ(x, 1111_i32);
  x = 5_i32;
  x /= 2_i32;
  EXPECT_EQ(x, 2_i32);

  // ** Signed only.
  x = -2345_i32;
  x /= 1_i32;
  EXPECT_EQ(x, -2345_i32);
  x = -2345_i32;
  x /= -1_i32;
  EXPECT_EQ(x, 2345_i32);
  x = 2345_i32;
  x /= -1_i32;
  EXPECT_EQ(x, -2345_i32);
  x = -2222_i32;
  x /= 2_i32;
  EXPECT_EQ(x, -1111_i32);
  x = 2222_i32;
  x /= -2_i32;
  EXPECT_EQ(x, -1111_i32);
  x = -2222_i32;
  x /= -2_i32;
  EXPECT_EQ(x, 1111_i32);
  x = -5_i32;
  x /= 2_i32;
  EXPECT_EQ(x, -2_i32);
}

TEST(i32DeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX() / 0_i32, "");
  EXPECT_DEATH(0_i32 / 0_i32, "");
  EXPECT_DEATH(1_i32 / 0_i32, "");
  EXPECT_DEATH(i32::MIN() / 0_i32, "");

  // ** Signed only.
  EXPECT_DEATH(-1_i32 / 0_i32, "");
  EXPECT_DEATH(i32::MIN() / -1_i32, "");

  auto x = i32::MAX();
  EXPECT_DEATH(x /= 0_i32, "");
  x = 0_i32;
  EXPECT_DEATH(x /= 0_i32, "");
  x = 1_i32;
  EXPECT_DEATH(x /= 0_i32, "");
  x = i32::MIN();
  EXPECT_DEATH(x /= 0_i32, "");

  // ** Signed only.
  x = -1_i32;
  EXPECT_DEATH(x /= 0_i32, "");
  x = i32::MIN();
  EXPECT_DEATH(x /= -1_i32, "");
#endif
}

TEST(i32, CheckedDiv) {
  constexpr auto a = (4_i32).checked_div(2_i32);

  EXPECT_EQ((0_i32).checked_div(123_i32), Option<i32>::some(0_i32));
  EXPECT_EQ((2345_i32).checked_div(1_i32), Option<i32>::some(2345_i32));

  EXPECT_EQ(i32::MAX().checked_div(0_i32), None);
  EXPECT_EQ((0_i32).checked_div(0_i32), None);
  EXPECT_EQ((1_i32).checked_div(0_i32), None);
  EXPECT_EQ(i32::MIN().checked_div(0_i32), None);

  // ** Signed only.
  EXPECT_EQ((-2345_i32).checked_div(1_i32), Option<i32>::some(-2345_i32));
  EXPECT_EQ((-1_i32).checked_div(0_i32), None);
  EXPECT_EQ(i32::MIN().checked_div(-1_i32), None);
}

TEST(i32, OverflowingDiv) {
  [[maybe_unused]] constexpr auto a = (-1_i32).overflowing_div(3_i32);

  EXPECT_EQ((0_i32).overflowing_div(123_i32),
            (Tuple<i32, bool>::with(0_i32, false)));

  // ** Signed only.
  EXPECT_EQ((-2345_i32).overflowing_div(1_i32),
            (Tuple<i32, bool>::with(-2345_i32, false)));
  EXPECT_EQ(i32::MIN().overflowing_div(-1_i32),
            (Tuple<i32, bool>::with(i32::MIN(), true)));
}

TEST(i32DeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX().overflowing_div(0_i32), "");
  EXPECT_DEATH((0_i32).overflowing_div(0_i32), "");
  EXPECT_DEATH((1_i32).overflowing_div(0_i32), "");
  EXPECT_DEATH(i32::MIN().overflowing_div(0_i32), "");

  // ** Signed only.
  EXPECT_DEATH((-1_i32).overflowing_div(0_i32), "");
#endif
}

TEST(i32, SaturatingDiv) {
  [[maybe_unused]] constexpr auto a = (-1_i32).saturating_div(3_i32);

  EXPECT_EQ((0_i32).saturating_div(123_i32), 0_i32);
  EXPECT_EQ((2345_i32).saturating_div(1_i32), 2345_i32);

  // ** Signed only.
  EXPECT_EQ((-2345_i32).saturating_div(1_i32), -2345_i32);
  EXPECT_EQ(i32::MIN().saturating_div(-1_i32), i32::MAX());
}

TEST(i32DeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX().saturating_div(0_i32), "");
  EXPECT_DEATH((0_i32).saturating_div(0_i32), "");
  EXPECT_DEATH((1_i32).saturating_div(0_i32), "");
  EXPECT_DEATH(i32::MIN().saturating_div(0_i32), "");

  // ** Signed only.
  EXPECT_DEATH((-1_i32).saturating_div(0_i32), "");
#endif
}

TEST(i32, WrappingDiv) {
  constexpr auto a = (4_i32).saturating_div(2_i32);
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ((0_i32).wrapping_div(123_i32), 0_i32);

  // ** Signed only.
  EXPECT_EQ((-2345_i32).wrapping_div(1_i32), -2345_i32);
  EXPECT_EQ(i32::MIN().wrapping_div(-1_i32), i32::MIN());
}

TEST(i32DeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX().wrapping_div(0_i32), "");
  EXPECT_DEATH((0_i32).wrapping_div(0_i32), "");
  EXPECT_DEATH((1_i32).wrapping_div(0_i32), "");
  EXPECT_DEATH(i32::MIN().wrapping_div(0_i32), "");

  // ** Signed only.
  EXPECT_DEATH((-1_i32).wrapping_div(0_i32), "");
#endif
}

TEST(i32, Mul) {
  constexpr auto a = (1_i32) * (3_i32);
  EXPECT_EQ(a, 3_i32);

  EXPECT_EQ(0_i32 * 21_i32, 0_i32);
  EXPECT_EQ(21_i32 * 0_i32, 0_i32);
  EXPECT_EQ(1_i32 * 21_i32, 21_i32);
  EXPECT_EQ(21_i32 * 1_i32, 21_i32);
  EXPECT_EQ(100_i32 * 21_i32, 2100_i32);
  EXPECT_EQ(21_i32 * 100_i32, 2100_i32);
  EXPECT_EQ(1_i32 * i32::MAX(), i32::MAX());
  EXPECT_EQ(i32::MIN() * 1_i32, i32::MIN());

  // ** Signed only.
  EXPECT_EQ(0_i32 * -21_i32, 0_i32);
  EXPECT_EQ(-21_i32 * 0_i32, 0_i32);
  EXPECT_EQ(-0_i32 * 21_i32, 0_i32);
  EXPECT_EQ(21_i32 * -0_i32, 0_i32);
  EXPECT_EQ(-0_i32 * -21_i32, 0_i32);
  EXPECT_EQ(-21_i32 * -0_i32, 0_i32);
  EXPECT_EQ(-1_i32 * 21_i32, -21_i32);
  EXPECT_EQ(21_i32 * -1_i32, -21_i32);
  EXPECT_EQ(-1_i32 * i32::MAX(), i32::MIN() + 1_i32);

  auto x = 5_i32;
  x *= 20_i32;
  EXPECT_EQ(x, 100_i32);

  // ** Signed only.
  x *= -4_i32;
  EXPECT_EQ(x, -400_i32);
}

TEST(i32DeathTest, MulOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX() * 2_i32, "");

  // ** Signed only.
  EXPECT_DEATH(i32::MAX() * -2_i32, "");
  EXPECT_DEATH(i32::MIN() * 2_i32, "");
  EXPECT_DEATH(i32::MIN() * -2_i32, "");
  EXPECT_DEATH(i32::MIN() * -1_i32, "");
#endif
}

TEST(i32, OverflowingMul) {
  constexpr auto a = (123456_i32).overflowing_mul(234567_i32);
  EXPECT_EQ(
      a,
      (Tuple<i32, bool>::with(
          i32(static_cast<decltype(i32::primitive_value)>(123456u * 234567u)),
          true)));
  constexpr auto b = (-123456_i32).overflowing_mul(234567_i32);
  EXPECT_EQ(b,
            (Tuple<i32, bool>::with(
                i32(static_cast<decltype(i32::primitive_value)>(
                    static_cast<int32_t>(int64_t{-123456} * int64_t{234567}))),
                true)));
  constexpr auto c = (-123456_i32).overflowing_mul(-234567_i32);
  EXPECT_EQ(c,
            (Tuple<i32, bool>::with(
                i32(static_cast<decltype(i32::primitive_value)>(
                    static_cast<int32_t>(int64_t{-123456} * int64_t{-234567}))),
                true)));
  constexpr auto d = (i32::MIN() / 2_i32).overflowing_mul(2);
  EXPECT_EQ(d, (Tuple<i32, bool>::with(i32::MIN(), false)));

  EXPECT_EQ((100_i32).overflowing_mul(21_i32),
            (Tuple<i32, bool>::with(2100_i32, false)));
  EXPECT_EQ((21_i32).overflowing_mul(100_i32),
            (Tuple<i32, bool>::with(2100_i32, false)));
  EXPECT_EQ(
      (123456_i32).overflowing_mul(234567_i32),
      (Tuple<i32, bool>::with(
          i32(static_cast<decltype(i32::primitive_value)>(123456u * 234567u)),
          true)));
  EXPECT_EQ((1'000'000'000_i32).overflowing_mul(10_i32),
            (Tuple<i32, bool>::with(1410065408_i32, true)));

  // ** Signed only.
  EXPECT_EQ((-123456_i32).overflowing_mul(-23456_i32),
            (Tuple<i32, bool>::with(-1399183360_i32, true)));
  EXPECT_EQ((123456_i32).overflowing_mul(-23456_i32),
            (Tuple<i32, bool>::with(1399183360_i32, true)));
}

TEST(i32, SaturatedMul) {
  constexpr auto a = (1_i32).saturating_mul(3_i32);
  EXPECT_EQ(a, 3_i32);

  EXPECT_EQ((100_i32).saturating_mul(21_i32), 2100_i32);
  EXPECT_EQ((21_i32).saturating_mul(100_i32), 2100_i32);
  EXPECT_EQ((123456_i32).saturating_mul(234567_i32), i32::MAX());

  // ** Signed only.
  EXPECT_EQ((-123456_i32).saturating_mul(-23456_i32), i32::MAX());
  EXPECT_EQ((123456_i32).saturating_mul(-23456_i32), i32::MIN());
}

TEST(i32, UncheckedMul) {
  constexpr auto a = (5_i32).unchecked_mul(unsafe_fn, 3_i32);
  EXPECT_EQ(a, 15_i32);

  EXPECT_EQ((100_i32).unchecked_mul(unsafe_fn, 21_i32), 2100_i32);
  EXPECT_EQ((21_i32).unchecked_mul(unsafe_fn, 100_i32), 2100_i32);
}

TEST(i32, WrappingMul) {
  constexpr auto a = (123456_i32).wrapping_mul(234567_i32);
  EXPECT_EQ(a.primitive_value, -1106067520);

  EXPECT_EQ((100_i32).wrapping_mul(21_i32), 2100_i32);
  EXPECT_EQ((21_i32).wrapping_mul(100_i32), 2100_i32);
  EXPECT_EQ(
      (123456_i32).wrapping_mul(234567_i32),
      i32(static_cast<decltype(i32::primitive_value)>(123456u * 234567u)));

  // ** Signed only.
  EXPECT_EQ((-123456_i32).wrapping_mul(-23456_i32), (-1399183360_i32));
  EXPECT_EQ((123456_i32).wrapping_mul(-23456_i32), 1399183360_i32);
}

// ** Signed only.
TEST(i32, Neg) {
  [[maybe_unused]] constexpr auto a = -(1_i32);

  EXPECT_EQ(-(0_i32), i32(0));
  EXPECT_EQ(-(10_i32), i32(-10));
  EXPECT_EQ(-(-10_i32), i32(10));
  EXPECT_EQ(-i32::MAX(), i32::MIN() + i32(1));
  EXPECT_EQ(-(i32::MIN() + 1_i32), i32::MAX());
}

// ** Signed only.
TEST(i32DeathTest, NegOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(-i32::MIN(), "");
#endif
}

TEST(i32, CheckedNeg) {
  constexpr auto a = (0_i32).checked_neg();
  EXPECT_EQ(a, Option<i32>::some(0_i32));

  EXPECT_EQ((0_i32).checked_neg(), Option<i32>::some(0_i32));

  // ** Unsigned only.
  // EXPECT_EQ((123_i32).checked_neg(), None);

  // ** Signed only.
  constexpr auto b = (123456_i32).checked_neg();
  EXPECT_EQ(b, Option<i32>::some(-123456_i32));
  EXPECT_EQ(i32::MIN().checked_neg(), None);
  EXPECT_EQ(i32::MAX().checked_neg(), Option<i32>::some(i32::MIN() + i32(1)));
  EXPECT_EQ((20_i32).checked_neg(), Option<i32>::some(-20_i32));
}

TEST(i32, OverflowingNeg) {
  constexpr auto a = (0_i32).overflowing_neg();
  EXPECT_EQ(a, (Tuple<i32, bool>::with(0_i32, false)));

  EXPECT_EQ((0_i32).overflowing_neg(), (Tuple<i32, bool>::with(0_i32, false)));

  // ** Unsigned only.
  // EXPECT_EQ((123_i32).overflowing_neg(),
  //          (Tuple<i32, bool>::with(
  //               static_cast<decltype(i32::primitive_value)>(-123), true)));

  // ** Signed only.
  EXPECT_EQ(i32::MIN().overflowing_neg(),
            (Tuple<i32, bool>::with(i32::MIN(), true)));
  EXPECT_EQ(i32::MAX().overflowing_neg(),
            (Tuple<i32, bool>::with(i32::MIN() + i32(1), false)));
  EXPECT_EQ((20_i32).overflowing_neg(),
            (Tuple<i32, bool>::with(-20_i32, false)));
}

// ** Signed only.
TEST(i32, SaturatingNeg) {
  constexpr auto a = (123456_i32).saturating_neg();
  EXPECT_EQ(a, -123456_i32);

  EXPECT_EQ(i32::MIN().saturating_neg(), i32::MAX());
  EXPECT_EQ(i32::MAX().saturating_neg(), i32::MIN() + i32(1));
  EXPECT_EQ((0_i32).saturating_neg(), i32(0));
  EXPECT_EQ((20_i32).saturating_neg(), i32(-20));
}

TEST(i32, WrappingNeg) {
  constexpr auto a = (123456_i32).wrapping_neg();
  EXPECT_EQ(a, -123456_i32);

  EXPECT_EQ(i32::MIN().wrapping_neg(), i32::MIN());
  EXPECT_EQ(i32::MAX().wrapping_neg(), i32::MIN() + 1_i32);
  EXPECT_EQ((0_i32).wrapping_neg(), 0_i32);

  // ** Signed only.
  EXPECT_EQ((20_i32).wrapping_neg(), -20_i32);
}

TEST(i32, Rem) {
  constexpr auto a = 5_i32 % 3_i32;
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ(0_i32 % 123_i32, 0_i32);
  EXPECT_EQ(5_i32 % 2_i32, 1_i32);
  EXPECT_EQ(5_i32 % 1_i32, 0_i32);

  // ** Signed only.
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

  // ** Signed only.
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
  EXPECT_DEATH(i32::MIN() % 0_i32, "");

  // ** Signed only.
  EXPECT_DEATH(-1_i32 % 0_i32, "");
  EXPECT_DEATH(i32::MIN() % -1_i32, "");

  auto x = i32::MAX();
  EXPECT_DEATH(x %= 0_i32, "");
  x = 0_i32;
  EXPECT_DEATH(x %= 0_i32, "");
  x = 1_i32;
  EXPECT_DEATH(x %= 0_i32, "");
  x = i32::MIN();
  EXPECT_DEATH(x %= 0_i32, "");

  // ** Signed only.
  x = -1_i32;
  EXPECT_DEATH(x %= 0_i32, "");
  x = i32::MIN();
  EXPECT_DEATH(x %= -1_i32, "");
#endif
}

TEST(i32, CheckedRem) {
  constexpr auto a = (5_i32).checked_rem(3_i32);
  EXPECT_EQ(a, Option<i32>::some(2_i32));

  EXPECT_EQ((0_i32).checked_rem(123_i32), Option<i32>::some(0_i32));
  EXPECT_EQ((2345_i32).checked_rem(4_i32), Option<i32>::some(1_i32));

  EXPECT_EQ(i32::MAX().checked_rem(0_i32), None);
  EXPECT_EQ((0_i32).checked_rem(0_i32), None);
  EXPECT_EQ((1_i32).checked_rem(0_i32), None);
  EXPECT_EQ(i32::MIN().checked_rem(0_i32), None);

  // ** Signed only.
  EXPECT_EQ((-2345_i32).checked_rem(4_i32), Option<i32>::some(-1_i32));
  EXPECT_EQ((-1_i32).checked_rem(0_i32), None);
  EXPECT_EQ(i32::MIN().checked_rem(-1_i32), None);
}

TEST(i32, OverflowingRem) {
  constexpr auto a = (5_i32).overflowing_rem(3_i32);
  EXPECT_EQ(a, (Tuple<i32, bool>::with(2_i32, false)));

  EXPECT_EQ((0_i32).overflowing_rem(123_i32),
            (Tuple<i32, bool>::with(0_i32, false)));
  EXPECT_EQ((2345_i32).overflowing_rem(4_i32),
            (Tuple<i32, bool>::with(1_i32, false)));

  // ** Signed only.
  EXPECT_EQ((-2345_i32).overflowing_rem(4_i32),
            (Tuple<i32, bool>::with(-1_i32, false)));
  EXPECT_EQ(i32::MIN().overflowing_rem(-1_i32),
            (Tuple<i32, bool>::with(0_i32, true)));
}

TEST(i32DeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX().overflowing_rem(0_i32), "");
  EXPECT_DEATH((0_i32).overflowing_rem(0_i32), "");
  EXPECT_DEATH((1_i32).overflowing_rem(0_i32), "");

  // ** Signed only.
  EXPECT_DEATH(i32::MIN().overflowing_rem(0_i32), "");
  EXPECT_DEATH((-1_i32).overflowing_rem(0_i32), "");
#endif
}

TEST(i32, WrappingRem) {
  constexpr auto a = (5_i32).wrapping_rem(3_i32);
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ((0_i32).wrapping_rem(123_i32), 0_i32);
  EXPECT_EQ((2345_i32).wrapping_rem(4_i32), 1_i32);

  // ** Signed only.
  EXPECT_EQ((-2345_i32).wrapping_rem(4_i32), -1_i32);
  EXPECT_EQ(i32::MIN().wrapping_rem(-1_i32), i32(0));
}

TEST(i32DeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MAX().wrapping_rem(0_i32), "");
  EXPECT_DEATH((0_i32).wrapping_rem(0_i32), "");
  EXPECT_DEATH((1_i32).wrapping_rem(0_i32), "");
  EXPECT_DEATH(i32::MIN().wrapping_rem(0_i32), "");

  // ** Signed only.
  EXPECT_DEATH((-1_i32).wrapping_rem(0_i32), "");
#endif
}

TEST(i32, Shl) {
  [[maybe_unused]] constexpr auto a = (5_i32) << 1_u32;

  EXPECT_EQ(2_i32 << 1_u32, 4_i32);
  EXPECT_EQ(1_i32 << 31_u32, i32(static_cast<decltype(i32::primitive_value)>(
                                 static_cast<uint32_t>(1u) << 31)));

  // ** Signed only.
  EXPECT_EQ(-2_i32 << 1_u32, i32(static_cast<decltype(i32::primitive_value)>(
                                 static_cast<uint32_t>(-2) << 1)));

  auto x = 2_i32;
  x <<= 1_u32;
  EXPECT_EQ(x, 4_i32);

  // ** Signed only.
  x = -2_i32;
  x <<= 1_u32;
  EXPECT_EQ(x, i32(static_cast<int32_t>(static_cast<uint32_t>(-2) << 1)));
}

TEST(i32DeathTest, ShlOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(0_i32 << 32_u32, "");
  EXPECT_DEATH(1_i32 << 33_u32, "");
  EXPECT_DEATH(2_i32 << 64_u32, "");
#endif
}

TEST(i32, OverflowingShl) {
  [[maybe_unused]] constexpr auto a = (5_i32).overflowing_shl(1_u32);

  EXPECT_EQ((2_i32).overflowing_shl(1_u32),
            (Tuple<i32, bool>::with(4_i32, false)));

  // Masks out everything.
  EXPECT_EQ((2_i32).overflowing_shl(32_u32),
            (Tuple<i32, bool>::with(2_i32, true)));
  // Masks out everything but the 1.
  EXPECT_EQ((2_i32).overflowing_shl(33_u32),
            (Tuple<i32, bool>::with(4_i32, true)));

  // ** Signed only.
  EXPECT_EQ(
      (-2_i32).overflowing_shl(1_u32),
      (Tuple<i32, bool>::with(
          i32(static_cast<int32_t>(static_cast<uint32_t>(-2) << 1u)), false)));
}

TEST(i32, CheckedShl) {
  constexpr auto a = (5_i32).checked_shl(1_u32);
  EXPECT_EQ(a, Option<i32>::some(10_i32));

  EXPECT_EQ((2_i32).checked_shl(1_u32), Option<i32>::some(4_i32));

  EXPECT_EQ((0_i32).checked_shl(32_u32), None);
  EXPECT_EQ((1_i32).checked_shl(33_u32), None);
  EXPECT_EQ((2_i32).checked_shl(64_u32), None);

  // ** Signed only.
  EXPECT_EQ((-2_i32).checked_shl(1_u32),
            Option<i32>::some(
                i32(static_cast<int32_t>(static_cast<uint32_t>(-2) << 1))));
}

TEST(i32, WrappingShl) {
  constexpr auto a = (5_i32).wrapping_shl(1_u32);
  EXPECT_EQ(a, 10_i32);

  EXPECT_EQ((2_i32).wrapping_shl(1_u32), 4_i32);

  EXPECT_EQ((2_i32).wrapping_shl(32_u32), 2_i32);  // Masks out everything.
  EXPECT_EQ((2_i32).wrapping_shl(33_u32),
            4_i32);  // Masks out everything but the 1.

  // ** Signed only.
  EXPECT_EQ((-2_i32).wrapping_shl(1_u32),
            i32(static_cast<int32_t>(static_cast<uint32_t>(-2) << 1)));
}

TEST(i32, Shr) {
  constexpr auto a = (5_i32) >> 1_u32;
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ(4_i32 >> 1_u32, 2_i32);

  // ** Signed only.
  EXPECT_EQ(-4_i32 >> 1_u32,
            i32(static_cast<int32_t>(static_cast<uint32_t>(-4) >> 1)));
  EXPECT_EQ(-1_i32 >> 31_u32, 1_i32);

  auto x = 4_i32;
  x >>= 1_u32;
  EXPECT_EQ(x, 2_i32);

  // ** Signed only.
  x = -4_i32;
  x >>= 1_u32;
  EXPECT_EQ(x, i32(static_cast<int32_t>(static_cast<uint32_t>(-4) >> 1)));
}

TEST(i32DeathTest, ShrOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(0_i32 >> 33_u32, "");
  EXPECT_DEATH(1_i32 >> 64_u32, "");

  // ** Signed only.
  EXPECT_DEATH(-1_i32 >> 32_u32, "");
#endif
}

TEST(i32, CheckedShr) {
  constexpr auto a = (5_i32).checked_shr(1_u32);
  EXPECT_EQ(a, Option<i32>::some(2_i32));

  EXPECT_EQ((4_i32).checked_shr(1_u32), Option<i32>::some(2_i32));
  EXPECT_EQ((0_i32).checked_shr(33_u32), None);
  EXPECT_EQ((1_i32).checked_shr(64_u32), None);

  // ** Signed only.
  EXPECT_EQ((-2_i32).checked_shr(1_u32),
            Option<i32>::some(
                i32(static_cast<int32_t>(static_cast<uint32_t>(-2) >> 1))));
  EXPECT_EQ((-1_i32).checked_shr(32_u32), None);
}

TEST(i32, OverflowingShr) {
  [[maybe_unused]] constexpr auto a = (5_i32).overflowing_shr(1_u32);

  EXPECT_EQ((4_i32).overflowing_shr(1_u32),
            (Tuple<i32, bool>::with(2_i32, false)));

  // Masks out everything.
  EXPECT_EQ((4_i32).overflowing_shr(32_u32),
            (Tuple<i32, bool>::with(4_i32, true)));
  // Masks out everything but the 1.
  EXPECT_EQ((4_i32).overflowing_shr(33_u32),
            (Tuple<i32, bool>::with(2_i32, true)));

  // ** Signed only.
  EXPECT_EQ(
      (-2_i32).overflowing_shr(1_u32),
      (Tuple<i32, bool>::with(
          i32(static_cast<int32_t>(static_cast<uint32_t>(-2) >> 1u)), false)));
}

TEST(i32, WrappingShr) {
  constexpr auto a = (5_i32).wrapping_shr(1_u32);
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ((4_i32).wrapping_shr(1_u32), 2_i32);

  EXPECT_EQ((4_i32).wrapping_shr(32_u32), 4_i32);  // Masks out everything.
  EXPECT_EQ((4_i32).wrapping_shr(33_u32),
            2_i32);  // Masks out everything but the 1.

  // ** Signed only.
  EXPECT_EQ((-2_i32).wrapping_shr(1_u32),
            i32(static_cast<int32_t>(static_cast<uint32_t>(-2) >> 1u)));
}

TEST(i32, Sub) {
  constexpr auto a = 5_i32 - 3_i32;
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ(0_i32 - 0_i32, 0_i32);
  EXPECT_EQ(12345_i32 - 12345_i32, 0_i32);
  EXPECT_EQ(12345_i32 - 1_i32, 12344_i32);
  EXPECT_EQ(i32::MAX() - i32::MAX(), 0_i32);
  EXPECT_EQ(i32::MIN() - i32::MIN(), 0_i32);

  // ** Signed only.
  EXPECT_EQ(-12345_i32 - 1_i32, -12346_i32);
  EXPECT_EQ(12345_i32 - (-1_i32), 12346_i32);
  EXPECT_EQ(0_i32 - (i32::MIN() + 1_i32), i32::MAX());

  auto x = 0_i32;
  x -= 0_i32;
  EXPECT_EQ(x, 0_i32);
  x = 12345_i32;
  x -= 345_i32;
  EXPECT_EQ(x, 12000_i32);

  // ** Signed only.
  x -= -345_i32;
  EXPECT_EQ(x, 12345_i32);
}

TEST(i32DeathTest, SubOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i32::MIN() - 1_i32, "");
  EXPECT_DEATH(i32::MIN() - i32::MAX(), "");

  // ** Signed only.
  EXPECT_DEATH(i32::MAX() - (-1_i32), "");
  EXPECT_DEATH(i32::MAX() - i32::MIN(), "");
#endif
}

TEST(i32, CheckedSub) {
  constexpr auto a = (5_i32).checked_sub(3_i32);
  EXPECT_EQ(a, Option<i32>::some(2_i32));

  EXPECT_EQ((0_i32).checked_sub(0_i32).unwrap(), 0_i32);
  EXPECT_EQ((12345_i32).checked_sub(12345_i32).unwrap(), 0_i32);

  EXPECT_EQ(i32::MIN().checked_sub(1_i32), None);
  EXPECT_EQ(i32::MIN().checked_sub(2_i32), None);
  EXPECT_EQ(i32::MIN().checked_sub(i32::MAX()), None);

  // ** Signed only.
  EXPECT_EQ((-12345_i32).checked_sub(-12345_i32).unwrap(), 0_i32);
  EXPECT_EQ(i32::MAX().checked_sub(-1_i32), None);
  EXPECT_EQ((-2_i32).checked_sub(i32::MAX()), None);
  EXPECT_EQ((1_i32).checked_sub(-i32::MAX()), None);
  EXPECT_EQ(i32::MAX().checked_sub(i32::MIN()), None);
}

TEST(i32, OverflowingSub) {
  constexpr auto a = (5_i32).overflowing_sub(3_i32);
  EXPECT_EQ(a, (Tuple<i32, bool>::with(2_i32, false)));

  EXPECT_EQ((0_i32).overflowing_sub(0_i32),
            (Tuple<i32, bool>::with(0_i32, false)));
  EXPECT_EQ((12345_i32).overflowing_sub(12345_i32),
            (Tuple<i32, bool>::with(0_i32, false)));

  EXPECT_EQ(i32::MIN().overflowing_sub(1_i32),
            (Tuple<i32, bool>::with(i32::MAX(), true)));
  EXPECT_EQ(i32::MIN().overflowing_sub(2_i32),
            (Tuple<i32, bool>::with(i32::MAX() - 1_i32, true)));
  EXPECT_EQ(i32::MIN().overflowing_sub(i32::MAX()),
            (Tuple<i32, bool>::with(1_i32, true)));

  // ** Signed only.
  EXPECT_EQ(i32::MAX().overflowing_sub(-1_i32),
            (Tuple<i32, bool>::with(i32::MIN(), true)));
  EXPECT_EQ((-12345_i32).overflowing_sub(-12345_i32),
            (Tuple<i32, bool>::with(0_i32, false)));
  EXPECT_EQ((-2_i32).overflowing_sub(i32::MAX()),
            (Tuple<i32, bool>::with(i32::MAX(), true)));
  EXPECT_EQ((1_i32).overflowing_sub(-i32::MAX()),
            (Tuple<i32, bool>::with(i32::MIN(), true)));
}

TEST(i32, SaturatingSub) {
  constexpr auto a = (5_i32).saturating_sub(3_i32);
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ((0_i32).saturating_sub(0_i32), 0_i32);
  EXPECT_EQ((12345_i32).saturating_sub(12345_i32), 0_i32);

  EXPECT_EQ(i32::MIN().saturating_sub(1_i32), i32::MIN());
  EXPECT_EQ(i32::MIN().saturating_sub(2_i32), i32::MIN());
  EXPECT_EQ(i32::MIN().saturating_sub(i32::MAX()), i32::MIN());

  // ** Signed only.
  EXPECT_EQ((-12345_i32).saturating_sub(-12345_i32), 0_i32);
  EXPECT_EQ(i32::MAX().saturating_sub(-1_i32), i32::MAX());
  EXPECT_EQ((-2_i32).saturating_sub(i32::MAX()), i32::MIN());
  EXPECT_EQ((1_i32).saturating_sub(-i32::MAX()), i32::MAX());
}

TEST(i32, UncheckedSub) {
  constexpr auto a = (5_i32).unchecked_sub(unsafe_fn, 3_i32);
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ((0_i32).unchecked_sub(unsafe_fn, 0_i32), 0_i32);
  EXPECT_EQ((12345_i32).unchecked_sub(unsafe_fn, 12345_i32), 0_i32);
  EXPECT_EQ((12345_i32).unchecked_sub(unsafe_fn, 1_i32), 12344_i32);
  EXPECT_EQ(i32::MAX().unchecked_sub(unsafe_fn, i32::MAX()), 0_i32);
  EXPECT_EQ(i32::MIN().unchecked_sub(unsafe_fn, i32::MIN()), 0_i32);
  EXPECT_EQ((0_i32).unchecked_sub(unsafe_fn, i32::MIN() + 1_i32), i32::MAX());

  // ** Signed only.
  EXPECT_EQ((-12345_i32).unchecked_sub(unsafe_fn, 1_i32), -12346_i32);
  EXPECT_EQ((12345_i32).unchecked_sub(unsafe_fn, -1_i32), 12346_i32);
}

TEST(i32, WrappingSub) {
  constexpr auto a = (5_i32).wrapping_sub(3_i32);
  EXPECT_EQ(a, 2_i32);

  EXPECT_EQ((0_i32).wrapping_sub(0_i32), 0_i32);
  EXPECT_EQ((12345_i32).wrapping_sub(12345_i32), 0_i32);

  EXPECT_EQ(i32::MIN().wrapping_sub(1_i32), i32::MAX());
  EXPECT_EQ(i32::MIN().wrapping_sub(2_i32), i32::MAX() - 1_i32);
  EXPECT_EQ(i32::MIN().wrapping_sub(i32::MAX()), 1_i32);

  // ** Signed only.
  EXPECT_EQ((-12345_i32).wrapping_sub(-12345_i32), 0_i32);
  EXPECT_EQ(i32::MAX().wrapping_sub(-1_i32), i32::MIN());
  EXPECT_EQ((-2_i32).wrapping_sub(i32::MAX()), i32::MAX());
  EXPECT_EQ((1_i32).wrapping_sub(-i32::MAX()), i32::MIN());
}

TEST(i32, CountOnes) {
  constexpr auto a1 = (7_i32).count_ones();
  EXPECT_EQ(a1, 3_u32);
  constexpr auto a2 = (0_i32).count_ones();
  EXPECT_EQ(a2, 0_u32);

  // ** Signed only.
  constexpr auto a3 = (-1_i32).count_ones();
  EXPECT_EQ(a3, 32_u32);

  EXPECT_EQ((7_i32).count_ones(), 3_u32);
  EXPECT_EQ((0_i32).count_ones(), 0_u32);

  // ** Signed only.
  EXPECT_EQ((-1_i32).count_ones(), 32_u32);
}

TEST(i32, CountZeros) {
  constexpr auto a1 = (7_i32).count_zeros();
  EXPECT_EQ(a1, 32_u32 - 3_u32);
  constexpr auto a2 = (0_i32).count_zeros();
  EXPECT_EQ(a2, 32_u32);

  // ** Signed only.
  constexpr auto a3 = (-1_i32).count_zeros();
  EXPECT_EQ(a3, 0_u32);

  EXPECT_EQ((7_i32).count_zeros(), 32_u32 - 3_u32);
  EXPECT_EQ((0_i32).count_zeros(), 32_u32);

  // ** Signed only.
  EXPECT_EQ((-1_i32).count_zeros(), 0_u32);
}

// ** Signed only.
TEST(i32, IsNegative) {
  constexpr auto a1 = (7_i32).is_negative();
  EXPECT_EQ(a1, false);
  constexpr auto a2 = (0_i32).is_negative();
  EXPECT_EQ(a2, false);
  constexpr auto a3 = (-1_i32).is_negative();
  EXPECT_EQ(a3, true);

  EXPECT_EQ((7_i32).is_negative(), false);
  EXPECT_EQ((0_i32).is_negative(), false);
  EXPECT_EQ((-1_i32).is_negative(), true);
}

// ** Signed only.
TEST(i32, IsPositive) {
  constexpr auto a1 = (7_i32).is_positive();
  EXPECT_EQ(a1, true);
  constexpr auto a2 = (0_i32).is_positive();
  EXPECT_EQ(a2, false);
  constexpr auto a3 = (-1_i32).is_positive();
  EXPECT_EQ(a3, false);

  EXPECT_EQ((7_i32).is_positive(), true);
  EXPECT_EQ((0_i32).is_positive(), false);
  EXPECT_EQ((-1_i32).is_positive(), false);
}

TEST(i32, LeadingZeros) {
  constexpr auto a1 = (0_i32).leading_zeros();
  EXPECT_EQ(a1, 32_u32);
  constexpr auto a2 = (1_i32).leading_zeros();
  EXPECT_EQ(a2, 31_u32);
  constexpr auto a3 = (3_i32).leading_zeros();
  EXPECT_EQ(a3, 30_u32);

  // ** Unsigned only.
  // constexpr auto a4 = (u32::MAX()).leading_zeros();
  // EXPECT_EQ(a4, 0);

  // ** Signed only.
  constexpr auto a4 = (i32::MAX()).leading_zeros();
  EXPECT_EQ(a4, 1_u32);
  constexpr auto a5 = (-1_i32).leading_zeros();
  EXPECT_EQ(a5, 0_u32);

  EXPECT_EQ((0_i32).leading_zeros(), 32_u32);
  EXPECT_EQ((1_i32).leading_zeros(), 31_u32);
  EXPECT_EQ((3_i32).leading_zeros(), 30_u32);
  EXPECT_EQ((i32::MAX()).leading_zeros(), 1_u32);

  // ** Signed only.
  EXPECT_EQ((-1_i32).leading_zeros(), 0_u32);
}

TEST(i32, LeadingOnes) {
  constexpr auto a1 = (0_i32).leading_ones();
  EXPECT_EQ(a1, 0_u32);
  constexpr auto a2 = (1_i32).leading_ones();
  EXPECT_EQ(a2, 0_u32);

  // ** Unsigned only.
  // constexpr auto a3 = (u32::MAX()).leading_ones();
  // EXPECT_EQ(a3, 32_u32);

  // ** Signed only.
  constexpr auto a3 = (i32::MAX()).leading_ones();
  EXPECT_EQ(a3, 0_u32);
  constexpr auto a4 = (-1_i32).leading_ones();
  EXPECT_EQ(a4, 32_u32);
  constexpr auto a5 = (-2_i32).leading_ones();
  EXPECT_EQ(a5, 31_u32);

  EXPECT_EQ((0_i32).leading_ones(), 0_u32);
  EXPECT_EQ((1_i32).leading_ones(), 0_u32);

  // ** Unsigned only.
  // EXPECT_EQ((u32::MAX()).leading_ones(), 32);

  // ** Signed only.
  EXPECT_EQ((i32::MAX()).leading_ones(), 0_u32);
  EXPECT_EQ((-1_i32).leading_ones(), 32_u32);
  EXPECT_EQ((-2_i32).leading_ones(), 31_u32);
}

TEST(i32, TrailingZeros) {
  constexpr auto a1 = (0_i32).trailing_zeros();
  EXPECT_EQ(a1, 32_u32);
  constexpr auto a2 = (1_i32).trailing_zeros();
  EXPECT_EQ(a2, 0_u32);
  constexpr auto a3 = (2_i32).trailing_zeros();
  EXPECT_EQ(a3, 1_u32);

  // ** Signed only.
  constexpr auto a4 = (i32::MIN()).trailing_zeros();
  EXPECT_EQ(a4, 31_u32);
  constexpr auto a5 = (-1_i32).trailing_zeros();
  EXPECT_EQ(a5, 0_u32);

  EXPECT_EQ((0_i32).trailing_zeros(), 32_u32);
  EXPECT_EQ((1_i32).trailing_zeros(), 0_u32);
  EXPECT_EQ((2_i32).trailing_zeros(), 1_u32);

  // ** Signed only.
  EXPECT_EQ((i32::MIN()).trailing_zeros(), 31_u32);
  EXPECT_EQ((-1_i32).trailing_zeros(), 0_u32);
}

TEST(i32, TrailingOnes) {
  constexpr auto a1 = (0_i32).trailing_ones();
  EXPECT_EQ(a1, 0_u32);
  constexpr auto a2 = (1_i32).trailing_ones();
  EXPECT_EQ(a2, 1_u32);
  constexpr auto a3 = (3_i32).trailing_ones();
  EXPECT_EQ(a3, 2_u32);

  // ** Unsigned only.
  // constexpr auto a4 = (i32::MAX()).trailing_ones();
  // EXPECT_EQ(a4, 32_u32);

  // ** Signed only.
  constexpr auto a4 = (i32::MAX()).trailing_ones();
  EXPECT_EQ(a4, 31_u32);
  constexpr auto a5 = (-1_i32).trailing_ones();
  EXPECT_EQ(a5, 32_u32);

  EXPECT_EQ((0_i32).trailing_ones(), 0_u32);
  EXPECT_EQ((1_i32).trailing_ones(), 1_u32);
  EXPECT_EQ((3_i32).trailing_ones(), 2_u32);

  // ** Unsigned only.
  // EXPECT_EQ((i32::MAX()).trailing_ones(), 32);

  // ** Signed only.
  EXPECT_EQ((i32::MAX()).trailing_ones(), 31_u32);
  EXPECT_EQ((-1_i32).trailing_ones(), 32_u32);
}

TEST(i32, Pow) {
  constexpr auto a = (2_i32).pow(5_u32);

  EXPECT_EQ((2_i32).pow(5_u32), 32_i32);
  EXPECT_EQ((2_i32).pow(0_u32), 1_i32);
  EXPECT_EQ((2_i32).pow(1_u32), 2_i32);
  EXPECT_EQ((2_i32).pow(30_u32), 1_i32 << 30_u32);
  EXPECT_EQ((1_i32).pow(u32::MAX()), 1_i32);
  EXPECT_EQ((i32::MAX()).pow(1_u32), i32::MAX());
  EXPECT_EQ((i32::MAX()).pow(0_u32), 1_i32);
}

TEST(i32DeathTest, PowOverflow) {
#if GTEST_HAS_DEATH_TEST
  // Crashes on the final acc * base.
  EXPECT_DEATH((3_i32).pow(31_u32), "");
  // Crashes on base * base.
  EXPECT_DEATH((i32::MAX() / 2_i32).pow(31_u32), "");
  // Crashes on acc * base inside the exponent loop.
  EXPECT_DEATH((2_i32).pow((1_u32 << 30_u32) - 1_u32), "");
#endif
}

TEST(i32, OverflowingPow) {
  constexpr auto a = (2_i32).overflowing_pow(5_u32);

  EXPECT_EQ((2_i32).overflowing_pow(5_u32),
            (Tuple<i32, bool>::with(32_i32, false)));
  EXPECT_EQ((2_i32).overflowing_pow(0_u32),
            (Tuple<i32, bool>::with(1_i32, false)));
  EXPECT_EQ((i32::MAX()).overflowing_pow(1_u32),
            (Tuple<i32, bool>::with(i32::MAX(), false)));
  EXPECT_EQ((i32::MAX()).overflowing_pow(2_u32),
            (Tuple<i32, bool>::with(1_i32, true)));
}

TEST(i32, CheckedPow) {
  constexpr auto a = (2_i32).checked_pow(5_u32);
  EXPECT_EQ(a, Option<i32>::some(32_i32));

  EXPECT_EQ((2_i32).checked_pow(5_u32), Option<i32>::some(32_i32));
  EXPECT_EQ((2_i32).checked_pow(0_u32), Option<i32>::some(1_i32));
  EXPECT_EQ((2_i32).checked_pow(1_u32), Option<i32>::some(2_i32));
  EXPECT_EQ((2_i32).checked_pow(30_u32), Option<i32>::some(1_i32 << 30_u32));
  EXPECT_EQ((1_i32).checked_pow(u32::MAX()), Option<i32>::some(1_i32));
  EXPECT_EQ((i32::MAX()).checked_pow(1_u32), Option<i32>::some(i32::MAX()));
  EXPECT_EQ((i32::MAX()).checked_pow(0_u32), Option<i32>::some(1_i32));

  // Fails on the final acc * base.
  EXPECT_EQ((3_i32).checked_pow(31_u32), None);
  // Fails on base * base.
  EXPECT_EQ((i32::MAX() / 2_i32).checked_pow(31_u32), None);
  // Fails on acc * base inside the exponent loop.
  EXPECT_EQ((4_i32).checked_pow((1_u32 << 30_u32) - 1_u32), None);
}

TEST(i32, WrappingPow) {
  constexpr auto a = (2_i32).wrapping_pow(5_u32);
  EXPECT_EQ(a, 32_i32);

  EXPECT_EQ((2_i32).wrapping_pow(5_u32), 32_i32);
  EXPECT_EQ((2_i32).wrapping_pow(0_u32), 1_i32);
  EXPECT_EQ((i32::MAX()).wrapping_pow(1_u32), i32::MAX());
  EXPECT_EQ((i32::MAX()).wrapping_pow(2_u32), 1_i32);
}

TEST(i32, ReverseBits) {
  constexpr auto a1 = (0_i32).reverse_bits();
  EXPECT_EQ(a1, 0_i32);
  constexpr auto a2 = (2_i32).reverse_bits();
  EXPECT_EQ(a2, 1_i32 << 30_u32);
  constexpr auto a3 = (0xf8f800_i32).reverse_bits();
  EXPECT_EQ(a3, 0x1f1f00_i32);
  constexpr auto a4 = (-1_i32).reverse_bits();
  EXPECT_EQ(a4, i32(-1));
  constexpr auto a5 = i32(1).reverse_bits();
  EXPECT_EQ(a5, 1_i32 << (sus::into(sizeof(i32)) * 8_u32 - 1_u32));

  EXPECT_EQ((0_i32).reverse_bits(), 0_i32);
  EXPECT_EQ((2_i32).reverse_bits(), 1_i32 << 30_u32);
  EXPECT_EQ((0xf8f800_i32).reverse_bits(), 0x1f1f00_i32);
  EXPECT_EQ((i32(-1)).reverse_bits(), -1_i32);
  EXPECT_EQ((i32(1)).reverse_bits().primitive_value,
            1_i32 << (sus::into(sizeof(i32)) * 8_u32 - 1_u32));
}

TEST(i32, RotateLeft) {
  constexpr auto a = (3_i32).rotate_left(2_u32);
  EXPECT_EQ(a, 12_i32);

  EXPECT_EQ((1_i32).rotate_left(1_u32), 2_i32);
  EXPECT_EQ((1_i32).rotate_left(4_u32), 16_i32);
  EXPECT_EQ((1_i32).rotate_left(31_u32), 1_i32 << 31_u32);
  EXPECT_EQ((1_i32).rotate_left(32_u32), 1_i32);
  EXPECT_EQ((1_i32).rotate_left(63_u32), 1_i32 << 31_u32);
  EXPECT_EQ((1_i32).rotate_left(64_u32), 1_i32);
}

TEST(i32, RotateRight) {
  constexpr auto a = (2_i32).rotate_right(1_u32);
  EXPECT_EQ(a, 1_i32);

  EXPECT_EQ((2_i32).rotate_right(1_u32), 1_i32);
  EXPECT_EQ((16_i32).rotate_right(4_u32), 1_i32);
  EXPECT_EQ((1_i32).rotate_right(1_u32), 1_i32 << 31_u32);
  EXPECT_EQ((1_i32).rotate_right(32_u32), 1_i32);
  EXPECT_EQ((1_i32).rotate_right(33_u32), 1_i32 << 31_u32);
  EXPECT_EQ((1_i32).rotate_right(64_u32), 1_i32);
  EXPECT_EQ((1_i32).rotate_right(65_u32), 1_i32 << 31_u32);
}

// ** Signed only.
TEST(i32, Signum) {
  constexpr auto a1 = (10_i32).signum();
  EXPECT_EQ(a1, i32(1));
  constexpr auto a2 = (0_i32).signum();
  EXPECT_EQ(a2, i32(0));
  constexpr auto a3 = (-7_i32).signum();
  EXPECT_EQ(a3, i32(-1));

  EXPECT_EQ((i32(10_i32)).signum(), i32(1));
  EXPECT_EQ((i32(0_i32)).signum(), i32(0));
  EXPECT_EQ((i32(-7_i32)).signum(), i32(-1));
}

TEST(i32, SwapBytes) {
  constexpr auto a = (0x12345678_i32).swap_bytes();
  EXPECT_EQ(a, 0x78563412_i32);

  EXPECT_EQ((0x12345678_i32).swap_bytes(), 0x78563412_i32);
  EXPECT_EQ((0_i32).swap_bytes(), 0_i32);
  EXPECT_EQ((-1_i32).swap_bytes(), -1_i32);
  EXPECT_EQ((1_i32 << 31_u32).swap_bytes(), (0x80_i32));
}

TEST(i32, Log2) {
  constexpr auto a = (2_i32).log2();
  EXPECT_EQ(a, 1u);

  EXPECT_EQ((2_i32).log2(), 1u);
  EXPECT_EQ((55555_i32).log2(), 15u);

  // ** Unsigned only.
  // EXPECT_EQ((u32::MAX() / 2_u32).log2(), 30u);

  // ** Signed only.
  EXPECT_EQ((i32::MAX()).log2(), 30u);
}

TEST(i32DeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((0_i32).log2(), "");

  // ** Signed only.
  EXPECT_DEATH((-1_i32).log2(), "");
#endif
}

TEST(i32, CheckedLog2) {
  constexpr auto a = (2_i32).checked_log2();
  EXPECT_EQ(a, Option<u32>::some(1_u32));

  EXPECT_EQ((2_i32).checked_log2(), Option<u32>::some(1_u32));
  EXPECT_EQ((55555_i32).checked_log2(), Option<u32>::some(15_u32));
  EXPECT_EQ((0_i32).checked_log2(), None);

  // ** Unsigned only.
  // EXPECT_EQ((u32::MAX() / 2_u32).checked_log2(),
  // Option<uint32_t>::some(30u));

  // ** Signed only.
  EXPECT_EQ((i32::MAX()).checked_log2(), Option<u32>::some(30_u32));
  EXPECT_EQ((-1_i32).checked_log2(), None);
}

TEST(i32, Log10) {
  constexpr auto a = (55555_i32).log10();
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((2_i32).log10(), 0_u32);
  EXPECT_EQ((55555_i32).log10(), 4_u32);
  EXPECT_EQ((i32::MAX()).log10(), 9_u32);
}

TEST(i32DeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((0_i32).log10(), "");

  // ** Signed only.
  EXPECT_DEATH((-1_i32).log10(), "");
#endif
}

TEST(i32, CheckedLog10) {
  constexpr auto a = (55555_i32).checked_log10();
  EXPECT_EQ(a, Option<u32>::some(4_u32));

  EXPECT_EQ((2_i32).checked_log10(), Option<u32>::some(0_u32));
  EXPECT_EQ((55555_i32).checked_log10(), Option<u32>::some(4_u32));
  EXPECT_EQ((i32::MAX()).checked_log10(), Option<u32>::some(9_u32));
  EXPECT_EQ((0_i32).checked_log10(), None);

  // ** Signed only.
  EXPECT_EQ((-1_i32).checked_log10(), None);
}

TEST(i32, Log) {
  constexpr auto a = (55555_i32).log(10_i32);
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((2_i32).log(10_i32), 0_u32);
  EXPECT_EQ((55555_i32).log(10_i32), 4_u32);
  EXPECT_EQ((i32::MAX()).log(10_i32), 9_u32);
}

TEST(i32DeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((0_i32).log(10_i32), "");
  EXPECT_DEATH((2_i32).log(0_i32), "");
  EXPECT_DEATH((2_i32).log(1_i32), "");

  // ** Signed only.
  EXPECT_DEATH((-1_i32).log(10_i32), "");
  EXPECT_DEATH((2_i32).log(-2_i32), "");
#endif
}

TEST(i32, CheckedLog) {
  constexpr auto a = (55555_i32).checked_log(10_i32);
  EXPECT_EQ(a, Option<u32>::some(4_u32));

  EXPECT_EQ((2_i32).checked_log(10_i32), Option<u32>::some(0_u32));
  EXPECT_EQ((55555_i32).checked_log(10_i32), Option<u32>::some(4_u32));
  EXPECT_EQ(i32::MAX().checked_log(10_i32), Option<u32>::some(9_u32));
  EXPECT_EQ((0_i32).checked_log(10_i32), None);

  // ** Signed only.
  EXPECT_EQ((-1_i32).checked_log(10_i32), None);
}

TEST(i32, ToBe) {
  if constexpr (sus::assertions::is_little_endian()) {
    constexpr auto a = (0x12345678_i32).to_be();
    EXPECT_EQ(a, 0x78563412_i32);

    EXPECT_EQ((0x12345678_i32).to_be(), 0x78563412_i32);
    EXPECT_EQ((0_i32).to_be(), 0_i32);
    EXPECT_EQ((1_i32 << 31_u32).to_be(), (1_i32 << 7_u32));

    // ** Signed only.
    EXPECT_EQ((-1_i32).to_be(), -1_i32);
  } else {
    constexpr auto a = (0x12345678_i32).to_be();
    EXPECT_EQ(a, 0x12345678_i32);

    EXPECT_EQ((0x12345678_i32).to_be(), 0x12345678_i32);
    EXPECT_EQ((0_i32).to_be(), 0_i32);
    EXPECT_EQ((1_i32 << 31_u32).to_be(), (1_i32 << 31_u32));

    // ** Signed only.
    EXPECT_EQ((-1_i32).to_be(), -1_i32);
  }
}

TEST(i32, FromBe) {
  if constexpr (sus::assertions::is_little_endian()) {
    constexpr auto a = i32::from_be(0x12345678_i32);
    EXPECT_EQ(a, 0x78563412_i32);

    EXPECT_EQ(i32::from_be(0x12345678_i32), 0x78563412_i32);
    EXPECT_EQ(i32::from_be(0_i32), 0_i32);
    EXPECT_EQ(i32::from_be(1_i32 << 31_u32), 1_i32 << 7_u32);

    // ** Signed only.
    EXPECT_EQ(i32::from_be(-1_i32), -1_i32);
  } else {
    constexpr auto a = i32::from_be(0x12345678_i32);
    EXPECT_EQ(a, 0x12345678_i32);

    EXPECT_EQ(i32::from_be(0x12345678_i32), 0x12345678_i32);
    EXPECT_EQ(i32::from_be(0_i32), 0_i32);
    EXPECT_EQ(i32::from_be(1_i32 << 31_u32), 1_i32 << 31_u32);

    // ** Signed only.
    EXPECT_EQ(i32::from_be(-1_i32), -1_i32);
  }
}

TEST(i32, ToLe) {
  if constexpr (sus::assertions::is_big_endian()) {
    constexpr auto a = (0x12345678_i32).to_le();
    EXPECT_EQ(a, 0x78563412_i32);

    EXPECT_EQ((0x12345678_i32).to_le(), 0x78563412_i32);
    EXPECT_EQ((0_i32).to_le(), 0_i32);
    EXPECT_EQ(i32::MIN().to_le(), 0x80_i32);

    // ** Signed only.
    EXPECT_EQ((-1_i32).to_le(), -1_i32);
  } else {
    constexpr auto a = (0x12345678_i32).to_le();
    EXPECT_EQ(a, 0x12345678_i32);

    EXPECT_EQ((0x12345678_i32).to_le(), 0x12345678_i32);
    EXPECT_EQ((0_i32).to_le(), 0_i32);
    EXPECT_EQ(i32::MIN().to_le(), i32::MIN());

    // ** Signed only.
    EXPECT_EQ((-1_i32).to_le(), -1_i32);
  }
}

TEST(i32, FromLe) {
  if constexpr (sus::assertions::is_big_endian()) {
    constexpr auto a = i32::from_le(0x12345678_i32);
    EXPECT_EQ(a, 0x78563412_i32);

    EXPECT_EQ(i32::from_le(0x12345678_i32), 0x78563412_i32);
    EXPECT_EQ(i32::from_le(0_i32), 0_i32);
    EXPECT_EQ(i32::from_le(i32::MIN()), 0x80_i32);

    // ** Signed only.
    EXPECT_EQ(i32::from_le(-1_i32), -1_i32);
  } else {
    constexpr auto a = i32::from_le(0x12345678_i32);
    EXPECT_EQ(a, 0x12345678_i32);

    EXPECT_EQ(i32::from_le(0x12345678_i32), 0x12345678_i32);
    EXPECT_EQ(i32::from_le(0_i32), 0_i32);
    EXPECT_EQ(i32::from_le(i32::MIN()), i32::MIN());

    // ** Signed only.
    EXPECT_EQ(i32::from_le(-1_i32), -1_i32);
  }
}

TEST(i32, ToBeBytes) {
  if constexpr (sus::assertions::is_little_endian()) {
    {
      constexpr auto a = (0x12345678_i32).to_be_bytes();
      EXPECT_EQ(a.get(0), 0x12_u8);
      EXPECT_EQ(a.get(1), 0x34_u8);
      EXPECT_EQ(a.get(2), 0x56_u8);
      EXPECT_EQ(a.get(3), 0x78_u8);
    }
    {
      auto a = (0x12345678_i32).to_be_bytes();
      EXPECT_EQ(a.get(0), 0x12_u8);
      EXPECT_EQ(a.get(1), 0x34_u8);
      EXPECT_EQ(a.get(2), 0x56_u8);
      EXPECT_EQ(a.get(3), 0x78_u8);
    }
  } else {
    {
      constexpr auto a = (0x12345678_i32).to_be_bytes();
      EXPECT_EQ(a.get(0), 0x12_u8);
      EXPECT_EQ(a.get(1), 0x34_u8);
      EXPECT_EQ(a.get(2), 0x56_u8);
      EXPECT_EQ(a.get(3), 0x78_u8);
    }
    {
      auto a = (0x12345678_i32).to_be_bytes();
      EXPECT_EQ(a.get(0), 0x12_u8);
      EXPECT_EQ(a.get(1), 0x34_u8);
      EXPECT_EQ(a.get(2), 0x56_u8);
      EXPECT_EQ(a.get(3), 0x78_u8);
    }
  }
}

TEST(i32, ToLeBytes) {
  if constexpr (sus::assertions::is_big_endian()) {
    constexpr auto a = (0x12345678_i32).to_le_bytes();
    EXPECT_EQ(a.get(0), 0x78_u8);
    EXPECT_EQ(a.get(1), 0x56_u8);
    EXPECT_EQ(a.get(2), 0x34_u8);
    EXPECT_EQ(a.get(3), 0x12_u8);
  } else {
    auto a = (0x12345678_i32).to_le_bytes();
    EXPECT_EQ(a.get(0), 0x78_u8);
    EXPECT_EQ(a.get(1), 0x56_u8);
    EXPECT_EQ(a.get(2), 0x34_u8);
    EXPECT_EQ(a.get(3), 0x12_u8);
  }
}

TEST(i32, ToNeBytes) {
  if constexpr (sus::assertions::is_big_endian()) {
    {
      constexpr auto a = (0x12345678_i32).to_ne_bytes();
      EXPECT_EQ(a.get(0), 0x12_u8);
      EXPECT_EQ(a.get(1), 0x34_u8);
      EXPECT_EQ(a.get(2), 0x56_u8);
      EXPECT_EQ(a.get(3), 0x78_u8);
    }
    {
      auto a = (0x12345678_i32).to_ne_bytes();
      EXPECT_EQ(a.get(0), 0x12_u8);
      EXPECT_EQ(a.get(1), 0x34_u8);
      EXPECT_EQ(a.get(2), 0x56_u8);
      EXPECT_EQ(a.get(3), 0x78_u8);
    }
  } else {
    {
      constexpr auto a = (0x12345678_i32).to_ne_bytes();
      EXPECT_EQ(a.get(0), 0x78_u8);
      EXPECT_EQ(a.get(1), 0x56_u8);
      EXPECT_EQ(a.get(2), 0x34_u8);
      EXPECT_EQ(a.get(3), 0x12_u8);
    }
    {
      auto a = (0x12345678_i32).to_ne_bytes();
      EXPECT_EQ(a.get(0), 0x78_u8);
      EXPECT_EQ(a.get(1), 0x56_u8);
      EXPECT_EQ(a.get(2), 0x34_u8);
      EXPECT_EQ(a.get(3), 0x12_u8);
    }
  }
}

// ** Signed only.
TEST(i32, CheckedAddUnsigned) {
  constexpr auto a = (1_i32).checked_add_unsigned(3u);
  EXPECT_EQ(a, Option<i32>::some(4_i32));

  EXPECT_EQ((-1_i32).checked_add_unsigned(2u), Option<i32>::some(1_i32));
  EXPECT_EQ((i32::MIN()).checked_add_unsigned(u32::MAX()),
            Option<i32>::some(i32::MAX()));
  EXPECT_EQ((i32::MIN() + 1_i32).checked_add_unsigned(u32::MAX()), None);
  EXPECT_EQ((i32::MIN() + 1_i32).checked_add_unsigned(u32::MAX() - 1_u32),
            Option<i32>::some(i32::MAX()));
  EXPECT_EQ((i32::MAX() - 2_i32).checked_add_unsigned(3u), None);
}

// ** Signed only.
TEST(i32, OverflowingAddUnsigned) {
  constexpr auto a = (1_i32).overflowing_add_unsigned(3u);
  EXPECT_EQ(a, (Tuple<i32, bool>::with(4_i32, false)));

  EXPECT_EQ((-1_i32).overflowing_add_unsigned(2u),
            (Tuple<i32, bool>::with(1_i32, false)));
  EXPECT_EQ((i32::MIN()).overflowing_add_unsigned(u32::MAX()),
            (Tuple<i32, bool>::with(i32::MAX(), false)));
  EXPECT_EQ((i32::MIN() + 1_i32).overflowing_add_unsigned(u32::MAX()),
            (Tuple<i32, bool>::with(i32::MIN(), true)));
  EXPECT_EQ((i32::MIN() + 1_i32).overflowing_add_unsigned(u32::MAX() - 1_u32),
            (Tuple<i32, bool>::with(i32::MAX(), false)));
  EXPECT_EQ((i32::MAX() - 2_i32).overflowing_add_unsigned(3u),
            (Tuple<i32, bool>::with(i32::MIN(), true)));
}

// ** Signed only.
TEST(i32, SaturatingAddUnsigned) {
  constexpr auto a = (1_i32).saturating_add_unsigned(3u);
  EXPECT_EQ(a, 4_i32);

  EXPECT_EQ((-1_i32).saturating_add_unsigned(2u), 1_i32);
  EXPECT_EQ((i32::MIN()).saturating_add_unsigned(u32::MAX()), i32::MAX());
  EXPECT_EQ((i32::MIN() + 1_i32).saturating_add_unsigned(u32::MAX()),
            i32::MAX());
  EXPECT_EQ((i32::MIN() + 1_i32).saturating_add_unsigned(u32::MAX() - 1_u32),
            i32::MAX());
  EXPECT_EQ((i32::MAX() - 2_i32).saturating_add_unsigned(3u), i32::MAX());
}

// ** Signed only.
TEST(i32, WrappingAddUnsigned) {
  constexpr auto a = (1_i32).wrapping_add_unsigned(3u);
  EXPECT_EQ(a, 4_i32);

  EXPECT_EQ((-1_i32).wrapping_add_unsigned(2u), 1_i32);
  EXPECT_EQ((i32::MIN()).wrapping_add_unsigned(u32::MAX()), i32::MAX());
  EXPECT_EQ((i32::MIN() + 1_i32).wrapping_add_unsigned(u32::MAX()), i32::MIN());
  EXPECT_EQ((i32::MIN() + 1_i32).wrapping_add_unsigned(u32::MAX() - 1_u32),
            i32::MAX());
  EXPECT_EQ((i32::MAX() - 2_i32).wrapping_add_unsigned(3u), i32::MIN());
}

// ** Signed only.
TEST(i32, CheckedSubUnsigned) {
  constexpr auto a = (1_i32).checked_sub_unsigned(3u);
  EXPECT_EQ(a, Option<i32>::some(-2_i32));

  EXPECT_EQ((-1_i32).checked_sub_unsigned(2u), Option<i32>::some(-3_i32));
  EXPECT_EQ((i32::MAX()).checked_sub_unsigned(u32::MAX()),
            Option<i32>::some(i32::MIN()));
  EXPECT_EQ((i32::MAX() - 1_i32).checked_sub_unsigned(u32::MAX()), None);
  EXPECT_EQ((i32::MAX() - 1_i32).checked_sub_unsigned(u32::MAX() - 1_u32),
            Option<i32>::some(i32::MIN()));
  EXPECT_EQ((i32::MIN() + 2_i32).checked_sub_unsigned(3u), None);
}

// ** Signed only.
TEST(i32, OverflowingSubUnsigned) {
  constexpr auto a = (1_i32).overflowing_sub_unsigned(3u);
  EXPECT_EQ(a, (Tuple<i32, bool>::with(-2_i32, false)));

  EXPECT_EQ((-1_i32).overflowing_sub_unsigned(2u),
            (Tuple<i32, bool>::with(-3_i32, false)));
  EXPECT_EQ((i32::MAX()).overflowing_sub_unsigned(u32::MAX()),
            (Tuple<i32, bool>::with(i32::MIN(), false)));
  EXPECT_EQ((i32::MAX() - 1_i32).overflowing_sub_unsigned(u32::MAX()),
            (Tuple<i32, bool>::with(i32::MAX(), true)));
  EXPECT_EQ((i32::MAX() - 1_i32).overflowing_sub_unsigned(u32::MAX() - 1_u32),
            (Tuple<i32, bool>::with(i32::MIN(), false)));
  EXPECT_EQ((i32::MIN() + 2_i32).overflowing_sub_unsigned(3u),
            (Tuple<i32, bool>::with(i32::MAX(), true)));
}

// ** Signed only.
TEST(i32, SaturatingSubUnsigned) {
  constexpr auto a = (1_i32).saturating_sub_unsigned(3u);
  EXPECT_EQ(a, -2_i32);

  EXPECT_EQ((-1_i32).saturating_sub_unsigned(2u), -3_i32);
  EXPECT_EQ((i32::MAX()).saturating_sub_unsigned(u32::MAX()), i32::MIN());
  EXPECT_EQ((i32::MAX() - 1_i32).saturating_sub_unsigned(u32::MAX()),
            i32::MIN());
  EXPECT_EQ((i32::MAX() - 1_i32).saturating_sub_unsigned(u32::MAX() - 1_u32),
            i32::MIN());
  EXPECT_EQ((i32::MIN() + 2_i32).saturating_sub_unsigned(3u), i32::MIN());
}

// ** Signed only.
TEST(i32, WrappingSubUnsigned) {
  constexpr auto a = (1_i32).wrapping_sub_unsigned(3u);
  EXPECT_EQ(a, -2_i32);

  EXPECT_EQ((-1_i32).wrapping_sub_unsigned(2u), -3_i32);
  EXPECT_EQ((i32::MAX()).wrapping_sub_unsigned(u32::MAX()), i32::MIN());
  EXPECT_EQ((i32::MAX() - 1_i32).wrapping_sub_unsigned(u32::MAX()), i32::MAX());
  EXPECT_EQ((i32::MAX() - 1_i32).wrapping_sub_unsigned(u32::MAX() - 1_u32),
            i32::MIN());
  EXPECT_EQ((i32::MIN() + 2_i32).wrapping_sub_unsigned(3u), i32::MAX());
}

TEST(i32, DivEuclid) {
  constexpr auto a = (7_i32).div_euclid(4_i32);
  EXPECT_EQ(a, 1_i32);

  EXPECT_EQ((7_i32).div_euclid(4_i32), 1_i32);    // 7 >= 4 * 1
  EXPECT_EQ((7_i32).div_euclid(-4_i32), -1_i32);  // 7 >= -4 * -1
  EXPECT_EQ((-7_i32).div_euclid(4_i32), -2_i32);  // -7 >= 4 * -2
  EXPECT_EQ((-7_i32).div_euclid(-4_i32), 2_i32);  // -7 >= -4 * 2
}

TEST(i32DeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_i32).div_euclid(0_i32), "");
  EXPECT_DEATH((i32::MIN()).div_euclid(-1_i32), "");
#endif
}

TEST(i32, CheckedDivEuclid) {
  constexpr auto a = (7_i32).checked_div_euclid(4_i32);
  EXPECT_EQ(a, Option<i32>::some(1_i32));

  EXPECT_EQ((7_i32).checked_div_euclid(4_i32), Option<i32>::some(1_i32));
  EXPECT_EQ((7_i32).checked_div_euclid(0_i32), None);
  EXPECT_EQ((i32::MIN()).checked_div_euclid(-1_i32), None);
}

TEST(i32, OverflowingDivEuclid) {
  constexpr auto a = (7_i32).overflowing_div_euclid(4_i32);
  EXPECT_EQ(a, (Tuple<i32, bool>::with(1_i32, false)));

  EXPECT_EQ((7_i32).overflowing_div_euclid(4_i32),
            (Tuple<i32, bool>::with(1_i32, false)));
  EXPECT_EQ((i32::MIN()).overflowing_div_euclid(-1_i32),
            (Tuple<i32, bool>::with(i32::MIN(), true)));
}

TEST(i32DeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_i32).overflowing_div_euclid(0_i32), "");
#endif
}

TEST(i32, WrappingDivEuclid) {
  constexpr auto a = (7_i32).wrapping_div_euclid(4_i32);
  EXPECT_EQ(a, 1_i32);

  EXPECT_EQ((7_i32).wrapping_div_euclid(4_i32), 1_i32);
  EXPECT_EQ((i32::MIN()).wrapping_div_euclid(-1_i32), i32::MIN());
}

TEST(i32DeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_i32).wrapping_div_euclid(0_i32), "");
#endif
}

TEST(i32, RemEuclid) {
  constexpr auto a = (7_i32).rem_euclid(4_i32);
  EXPECT_EQ(a, 3_i32);

  EXPECT_EQ((7_i32).rem_euclid(4_i32), 3_i32);
  EXPECT_EQ((-7_i32).rem_euclid(4_i32), 1_i32);
  EXPECT_EQ((7_i32).rem_euclid(-4_i32), 3_i32);
  EXPECT_EQ((-7_i32).rem_euclid(-4_i32), 1_i32);
}

TEST(i32DeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_i32).rem_euclid(0_i32), "");
  EXPECT_DEATH((i32::MIN()).rem_euclid(-1_i32), "");
#endif
}

TEST(i32, CheckedRemEuclid) {
  constexpr auto a = (7_i32).checked_rem_euclid(4_i32);
  EXPECT_EQ(a, Option<i32>::some(3_i32));

  EXPECT_EQ((7_i32).checked_rem_euclid(4_i32), Option<i32>::some(3_i32));
  EXPECT_EQ((7_i32).checked_rem_euclid(0_i32), None);
  EXPECT_EQ((i32::MIN()).checked_rem_euclid(-1_i32), None);
}

TEST(i32, OverflowingRemEuclid) {
  constexpr auto a = (7_i32).overflowing_rem_euclid(4_i32);
  EXPECT_EQ(a, (Tuple<i32, bool>::with(3_i32, false)));

  EXPECT_EQ((7_i32).overflowing_rem_euclid(4_i32),
            (Tuple<i32, bool>::with(3_i32, false)));
  EXPECT_EQ((i32::MIN()).overflowing_rem_euclid(-1_i32),
            (Tuple<i32, bool>::with(0_i32, true)));
}

TEST(i32DeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_i32).overflowing_rem_euclid(0_i32), "");
#endif
}

TEST(i32, WrappingRemEuclid) {
  constexpr auto a = (7_i32).wrapping_rem_euclid(4_i32);
  EXPECT_EQ(a, 3_i32);

  EXPECT_EQ((7_i32).wrapping_rem_euclid(4_i32), 3_i32);
  EXPECT_EQ((i32::MIN()).wrapping_rem_euclid(-1_i32), 0_i32);
}

TEST(i32DeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_i32).wrapping_rem_euclid(0_i32), "");
#endif
}

}  // namespace
