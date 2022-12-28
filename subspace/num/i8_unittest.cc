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
#include "containers/array.h"
#include "num/num_concepts.h"
#include "num/signed_integer.h"
#include "num/unsigned_integer.h"
#include "ops/eq.h"
#include "ops/ord.h"
#include "prelude.h"
#include "googletest/include/gtest/gtest.h"
#include "tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(std::is_signed_v<decltype(i8::primitive_value)>);
static_assert(sizeof(decltype(i8::primitive_value)) == 1);
static_assert(sizeof(i8) == sizeof(decltype(i8::primitive_value)));

namespace behaviour {
using T = i8;
using From = decltype(i8::primitive_value);
static_assert(!std::is_trivial_v<T>, "");
static_assert(!std::is_aggregate_v<T>, "");
static_assert(std::is_standard_layout_v<T>, "");
static_assert(!std::is_trivially_default_constructible_v<T>, "");
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
static_assert(sus::construct::Default<T>, "");
static_assert(sus::mem::relocate_one_by_memcpy<T>, "");
static_assert(sus::mem::relocate_array_by_memcpy<T>, "");
}  // namespace behaviour

// i8::MAX()
static_assert(i8::MAX().primitive_value == 0x7f);
template <class T>
concept MaxInRange = requires {
  { 0x7f_i8 } -> std::same_as<T>;
  { i8(int8_t{0x7f}) } -> std::same_as<T>;
};
static_assert(MaxInRange<i8>);

// std hashing
static_assert(std::same_as<decltype(std::hash<i8>()(0_i8)), size_t>);
static_assert(std::same_as<decltype(std::equal_to<i8>()(0_i8, 1_i8)), bool>);

TEST(i8, Traits) {
  // ** Signed only **
  static_assert(sus::num::Neg<i8>);

  static_assert(sus::num::Add<i8, i8>);
  static_assert(sus::num::AddAssign<i8, i8>);
  static_assert(sus::num::Sub<i8, i8>);
  static_assert(sus::num::SubAssign<i8, i8>);
  static_assert(sus::num::Mul<i8, i8>);
  static_assert(sus::num::MulAssign<i8, i8>);
  static_assert(sus::num::Div<i8, i8>);
  static_assert(sus::num::DivAssign<i8, i8>);
  static_assert(sus::num::Rem<i8, i8>);
  static_assert(sus::num::RemAssign<i8, i8>);
  static_assert(sus::num::BitAnd<i8, i8>);
  static_assert(sus::num::BitAndAssign<i8, i8>);
  static_assert(sus::num::BitOr<i8, i8>);
  static_assert(sus::num::BitOrAssign<i8, i8>);
  static_assert(sus::num::BitXor<i8, i8>);
  static_assert(sus::num::BitXorAssign<i8, i8>);
  static_assert(sus::num::BitNot<i8>);
  static_assert(sus::num::Shl<i8>);
  static_assert(sus::num::ShlAssign<i8>);
  static_assert(sus::num::Shr<i8>);
  static_assert(sus::num::ShrAssign<i8>);

  static_assert(sus::ops::Ord<i8, int8_t>);
  static_assert(sus::ops::Ord<i8, int16_t>);
  static_assert(sus::ops::Ord<i8, int32_t>);
  static_assert(sus::ops::Ord<i8, int64_t>);
  static_assert(sus::ops::Ord<i8, i8>);
  static_assert(sus::ops::Ord<i8, i16>);
  static_assert(sus::ops::Ord<i8, i32>);
  static_assert(sus::ops::Ord<i8, i64>);
  static_assert(sus::ops::Ord<i8, isize>);
  static_assert(1_i8 >= 1_i8);
  static_assert(2_i8 > 1_i8);
  static_assert(1_i8 <= 1_i8);
  static_assert(1_i8 < 2_i8);
  static_assert(sus::ops::Eq<i8, int8_t>);
  static_assert(sus::ops::Eq<i8, int16_t>);
  static_assert(sus::ops::Eq<i8, int32_t>);
  static_assert(sus::ops::Eq<i8, int64_t>);
  static_assert(sus::ops::Eq<i8, i8>);
  static_assert(sus::ops::Eq<i8, i16>);
  static_assert(sus::ops::Eq<i8, i32>);
  static_assert(sus::ops::Eq<i8, i64>);
  static_assert(sus::ops::Eq<i8, isize>);
  static_assert(1_i8 == 1_i8);
  static_assert(!(1_i8 == 2_i8));
  static_assert(1_i8 != 2_i8);
  static_assert(!(1_i8 != 1_i8));

  // Verify constexpr.
  [[maybe_unused]] constexpr i8 c =
      1_i8 + 2_i8 - 3_i8 * 4_i8 / 5_i8 % 6_i8 & 7_i8 | 8_i8 ^ -9_i8;
  [[maybe_unused]] constexpr std::strong_ordering o = 2_i8 <=> 3_i8;
}

TEST(i8, Literals) {
  // Hex.
  static_assert((0x1C_i8).primitive_value == 0x1C);
  static_assert((0X1C_i8).primitive_value == 0X1C);
  static_assert((0X001C_i8).primitive_value == 0X1C);
  EXPECT_EQ((0x1C_i8).primitive_value, 0x1C);
  EXPECT_EQ((0X1C_i8).primitive_value, 0X1C);
  EXPECT_EQ((0X001C_i8).primitive_value, 0X1C);
  // Binary.
  static_assert((0b101_i8).primitive_value == 0b101);
  static_assert((0B101_i8).primitive_value == 0B101);
  static_assert((0b00101_i8).primitive_value == 0b101);
  EXPECT_EQ((0b101_i8).primitive_value, 0b101);
  EXPECT_EQ((0B101_i8).primitive_value, 0B101);
  EXPECT_EQ((0b00101_i8).primitive_value, 0b101);
  // Octal.
  static_assert((0123_i8).primitive_value == 0123);
  static_assert((000123_i8).primitive_value == 0123);
  EXPECT_EQ((0123_i8).primitive_value, 0123);
  EXPECT_EQ((000123_i8).primitive_value, 0123);
  // Decimal.
  static_assert((0_i8).primitive_value == 0);
  static_assert((1_i8).primitive_value == 1);
  static_assert((12_i8).primitive_value == 12);
  static_assert((123_i8).primitive_value == 123);
}

TEST(i8, Constants) {
  constexpr auto max = i8::MAX();
  static_assert(std::same_as<decltype(max), const i8>);
  EXPECT_EQ(max.primitive_value, 0x7f);
  constexpr auto min = i8::MIN();
  static_assert(std::same_as<decltype(min), const i8>);
  EXPECT_EQ(min.primitive_value, -0x7f - 1);
  constexpr auto bits = i8::BITS();
  static_assert(std::same_as<decltype(bits), const u32>);
  EXPECT_EQ(bits, 8u);
}

template <class From, class To>
concept IsImplicitlyConvertible = (std::is_convertible_v<From, To> &&
                                   std::is_assignable_v<To, From>);
template <class From, class To>
concept IsExplicitlyConvertible = (std::constructible_from<To, From> &&
                                   !std::is_convertible_v<From, To> &&
                                   !std::is_assignable_v<To, From>);
template <class From, class To>
concept NotConvertible = (!std::constructible_from<To, From> &&
                          !std::is_convertible_v<From, To> &&
                          !std::is_assignable_v<To, From>);

TEST(i8, FromPrimitive) {
  static_assert(IsImplicitlyConvertible<int8_t, i8>);
  static_assert(NotConvertible<int16_t, i8>);
  static_assert(NotConvertible<int32_t, i8>);
  static_assert(NotConvertible<int64_t, i8>);
  static_assert(NotConvertible<uint8_t, i8>);
  static_assert(NotConvertible<uint16_t, i8>);
  static_assert(NotConvertible<uint32_t, i8>);
  static_assert(NotConvertible<uint64_t, i8>);
  static_assert(NotConvertible<size_t, i8>);
  static_assert(sizeof(size_t) >= sizeof(i8));
}

TEST(i8, ToPrimitive) {
  static_assert(IsExplicitlyConvertible<i8, int8_t>);
  static_assert(IsExplicitlyConvertible<i8, int16_t>);
  static_assert(IsExplicitlyConvertible<i8, int32_t>);
  static_assert(IsExplicitlyConvertible<i8, int64_t>);
  static_assert(NotConvertible<i8, uint8_t>);
  static_assert(NotConvertible<i8, uint16_t>);
  static_assert(NotConvertible<i8, uint32_t>);
  static_assert(NotConvertible<i8, uint64_t>);
  static_assert(NotConvertible<i8, size_t>);
}

TEST(i8, From) {
  static_assert(sus::construct::From<i8, char>);
  static_assert(sus::construct::From<i8, size_t>);
  static_assert(sus::construct::From<i8, int8_t>);
  static_assert(sus::construct::From<i8, int16_t>);
  static_assert(sus::construct::From<i8, int32_t>);
  static_assert(sus::construct::From<i8, int64_t>);
  static_assert(sus::construct::From<i8, uint8_t>);
  static_assert(sus::construct::From<i8, uint16_t>);
  static_assert(sus::construct::From<i8, uint32_t>);
  static_assert(sus::construct::From<i8, uint64_t>);

  EXPECT_EQ(i8::from(char{2}), 2_i8);
  EXPECT_EQ(i8::from(size_t{2}), 2_i8);
  EXPECT_EQ(i8::from(int8_t{2}), 2_i8);
  EXPECT_EQ(i8::from(int16_t{2}), 2_i8);
  EXPECT_EQ(i8::from(int32_t{2}), 2_i8);
  EXPECT_EQ(i8::from(int64_t{2}), 2_i8);
  EXPECT_EQ(i8::from(uint8_t{2}), 2_i8);
  EXPECT_EQ(i8::from(uint16_t{2}), 2_i8);
  EXPECT_EQ(i8::from(uint32_t{2}), 2_i8);
  EXPECT_EQ(i8::from(uint64_t{2}), 2_i8);

  static_assert(sus::construct::From<i8, i8>);
  static_assert(sus::construct::From<i8, i16>);
  static_assert(sus::construct::From<i8, i32>);
  static_assert(sus::construct::From<i8, i64>);
  static_assert(sus::construct::From<i8, isize>);
  static_assert(sus::construct::From<i8, u8>);
  static_assert(sus::construct::From<i8, u16>);
  static_assert(sus::construct::From<i8, u32>);
  static_assert(sus::construct::From<i8, u64>);
  static_assert(sus::construct::From<i8, usize>);

  EXPECT_EQ(i8::from(2_i8), 2_i8);
  EXPECT_EQ(i8::from(2_i16), 2_i8);
  EXPECT_EQ(i8::from(2_i32), 2_i8);
  EXPECT_EQ(i8::from(2_i64), 2_i8);
  EXPECT_EQ(i8::from(2_isize), 2_i8);
  EXPECT_EQ(i8::from(2_u8), 2_i8);
  EXPECT_EQ(i8::from(2_u16), 2_i8);
  EXPECT_EQ(i8::from(2_u32), 2_i8);
  EXPECT_EQ(i8::from(2_u64), 2_i8);
  EXPECT_EQ(i8::from(2_usize), 2_i8);
}

TEST(i8DeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i8::from(int64_t{-1 - 0x7fff'ffff'ffff'ffff}), "");
  EXPECT_DEATH(i8::from(uint64_t{0xffff'ffff'ffff'ffff}), "");

  EXPECT_DEATH(i8::from(i16::MAX()), "");
  EXPECT_DEATH(i8::from(i32::MAX()), "");
  EXPECT_DEATH(i8::from(i64::MAX()), "");
  EXPECT_DEATH(i8::from(u8::MAX()), "");
  EXPECT_DEATH(i8::from(u16::MAX()), "");
  EXPECT_DEATH(i8::from(u32::MAX()), "");
  EXPECT_DEATH(i8::from(u64::MAX()), "");
  EXPECT_DEATH(i8::from(usize::MAX()), "");
#endif
}

TEST(i8, InvokeEverything) {
  auto i = 10_i8, j = 11_i8;
  auto s = 3_u8;
  auto a = sus::Array<u8, sizeof(i8)>();

  i.is_negative();
  i.is_positive();
  i.signum();

  i.abs();
  i.checked_abs();
  i.overflowing_abs();
  i.saturating_abs();
  i.unsigned_abs();
  i.wrapping_abs();
  i.abs_diff(j);

  i.checked_add(j);
  i.checked_add_unsigned(s);
  i.overflowing_add(j);
  i.overflowing_add_unsigned(s);
  i.saturating_add(j);
  i.saturating_add_unsigned(s);
  i.unchecked_add(unsafe_fn, j);
  i.wrapping_add(j);
  i.wrapping_add_unsigned(s);

  i.checked_div(j);
  i.overflowing_div(j);
  i.saturating_div(j);
  i.wrapping_div(j);

  i.checked_mul(j);
  i.overflowing_mul(j);
  i.saturating_mul(j);
  i.unchecked_mul(unsafe_fn, j);
  i.wrapping_mul(j);

  i.checked_neg();
  i.overflowing_neg();
  i.wrapping_neg();

  i.checked_rem(j);
  i.overflowing_rem(j);
  i.wrapping_rem(j);

  i.div_euclid(j);
  i.checked_div_euclid(j);
  i.overflowing_div_euclid(j);
  i.wrapping_div_euclid(j);
  i.rem_euclid(j);
  i.checked_rem_euclid(j);
  i.overflowing_rem_euclid(j);
  i.wrapping_rem_euclid(j);

  i.checked_shl(1_u32);
  i.overflowing_shl(1_u32);
  i.wrapping_shl(1_u32);
  i.checked_shr(1_u32);
  i.overflowing_shr(1_u32);
  i.wrapping_shr(1_u32);

  i.checked_sub(j);
  i.checked_sub_unsigned(s);
  i.overflowing_sub(j);
  i.overflowing_sub_unsigned(s);
  i.saturating_sub(j);
  i.saturating_sub_unsigned(s);
  i.unchecked_sub(unsafe_fn, j);
  i.wrapping_sub(j);
  i.wrapping_sub_unsigned(s);

  i.count_ones();
  i.count_zeros();
  i.leading_ones();
  i.leading_zeros();
  i.trailing_ones();
  i.trailing_zeros();
  i.reverse_bits();
  i.rotate_left(1_u32);
  i.rotate_right(1_u32);
  i.swap_bytes();

  i.pow(1_u32);
  i.checked_pow(1_u32);
  i.overflowing_pow(1_u32);
  i.wrapping_pow(1_u32);

  i.checked_log2();
  i.log2();
  i.checked_log10();
  i.log10();
  i.checked_log(j);
  i.log(j);

  i.from_be(j);
  i.from_le(j);
  i.to_be();
  i.to_le();
  i.to_be_bytes();
  i.to_le_bytes();
  i.to_ne_bytes();
  i.from_be_bytes(a);
  i.from_le_bytes(a);
  i.from_ne_bytes(a);

  i = -j;
  i = ~j;

  i = j + j;
  i = j - j;
  i = j * j;
  i = j / j;
  i = j % j;
  i = j & j;
  i = j | j;
  i = j ^ j;
  i = j << 1_u32;
  i = j >> 1_u32;

  i += j;
  i -= j;
  i *= j;
  i /= j;
  i %= j;
  i &= j;
  i |= j;
  i ^= j;
  i <<= 1_u32;
  i >>= 1_u32;

  [[maybe_unused]] auto b = i == j;
  [[maybe_unused]] auto z = i >= j;
}
}  // namespace
