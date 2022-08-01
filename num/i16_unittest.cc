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

#include "concepts/into.h"
#include "num/num_concepts.h"
#include "num/types.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(std::is_signed_v<decltype(i16::primitive_value)>);
static_assert(sizeof(decltype(i16::primitive_value)) == 2);
static_assert(sizeof(i16) == sizeof(decltype(i16::primitive_value)));

namespace behaviour {
using T = i16;
using From = decltype(i16::primitive_value);
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

// i16::MAX()
static_assert(i16::MAX().primitive_value == 0x7fff);
template <class T>
concept MaxInRange = requires {
                       { 0x7fff_i16 } -> std::same_as<T>;
                       { i16(int16_t{0x7fff}) } -> std::same_as<T>;
                     };
static_assert(MaxInRange<i16>);

TEST(i16, Traits) {
  // ** Signed only **
  static_assert(sus::num::Neg<i16>);

  static_assert(sus::num::Add<i16, i16>);
  static_assert(sus::num::AddAssign<i16, i16>);
  static_assert(sus::num::Sub<i16, i16>);
  static_assert(sus::num::SubAssign<i16, i16>);
  static_assert(sus::num::Mul<i16, i16>);
  static_assert(sus::num::MulAssign<i16, i16>);
  static_assert(sus::num::Div<i16, i16>);
  static_assert(sus::num::DivAssign<i16, i16>);
  static_assert(sus::num::Rem<i16, i16>);
  static_assert(sus::num::RemAssign<i16, i16>);
  static_assert(sus::num::BitAnd<i16, i16>);
  static_assert(sus::num::BitAndAssign<i16, i16>);
  static_assert(sus::num::BitOr<i16, i16>);
  static_assert(sus::num::BitOrAssign<i16, i16>);
  static_assert(sus::num::BitXor<i16, i16>);
  static_assert(sus::num::BitXorAssign<i16, i16>);
  static_assert(sus::num::BitNot<i16>);
  static_assert(sus::num::Shl<i16>);
  static_assert(sus::num::ShlAssign<i16>);
  static_assert(sus::num::Shr<i16>);
  static_assert(sus::num::ShrAssign<i16>);

  static_assert(sus::num::Ord<i16, i16>);
  static_assert(1_i16 >= 1_i16);
  static_assert(2_i16 > 1_i16);
  static_assert(1_i16 <= 1_i16);
  static_assert(1_i16 < 2_i16);
  static_assert(sus::num::Eq<i16, i16>);
  static_assert(1_i16 == 1_i16);
  static_assert(!(1_i16 == 2_i16));
  static_assert(1_i16 != 2_i16);
  static_assert(!(1_i16 != 1_i16));

  // Verify constexpr.
  constexpr i16 c =
      1_i16 + 2_i16 - 3_i16 * 4_i16 / 5_i16 % 6_i16 & 7_i16 | 8_i16 ^ -9_i16;
  constexpr std::strong_ordering o = 2_i16 <=> 3_i16;
}

TEST(i16, Literals) {
  // Hex.
  static_assert((0x12bC_i16).primitive_value == 0x12bC);
  static_assert((0X12bC_i16).primitive_value == 0X12bC);
  static_assert((0X0012bC_i16).primitive_value == 0X12bC);
  EXPECT_EQ((0x12bC_i16).primitive_value, 0x12bC);
  EXPECT_EQ((0X12bC_i16).primitive_value, 0X12bC);
  EXPECT_EQ((0X0012bC_i16).primitive_value, 0X12bC);
  // Binary.
  static_assert((0b101_i16).primitive_value == 0b101);
  static_assert((0B101_i16).primitive_value == 0B101);
  static_assert((0b00101_i16).primitive_value == 0b101);
  EXPECT_EQ((0b101_i16).primitive_value, 0b101);
  EXPECT_EQ((0B101_i16).primitive_value, 0B101);
  EXPECT_EQ((0b00101_i16).primitive_value, 0b101);
  // Octal.
  static_assert((0123_i16).primitive_value == 0123);
  static_assert((000123_i16).primitive_value == 0123);
  EXPECT_EQ((0123_i16).primitive_value, 0123);
  EXPECT_EQ((000123_i16).primitive_value, 0123);
  // Decimal.
  static_assert((0_i16).primitive_value == 0);
  static_assert((1_i16).primitive_value == 1);
  static_assert((12_i16).primitive_value == 12);
  static_assert((123_i16).primitive_value == 123);
  static_assert((1234_i16).primitive_value == 1234);
  static_assert((12345_i16).primitive_value == 12345);
}

TEST(i16, Constants) {
  constexpr auto max = i16::MAX();
  static_assert(std::same_as<decltype(max), const i16>);
  EXPECT_EQ(max.primitive_value, 0x7fff);
  constexpr auto min = i16::MIN();
  static_assert(std::same_as<decltype(min), const i16>);
  EXPECT_EQ(min.primitive_value, -0x7fff - 1);
  constexpr auto bits = i16::BITS();
  static_assert(std::same_as<decltype(bits), const u32>);
  EXPECT_EQ(bits, 16u);
}

TEST(i16, From) {
  static_assert(sus::concepts::from::From<i16, char>);
  static_assert(sus::concepts::from::From<i16, size_t>);
  static_assert(sus::concepts::from::From<i16, int8_t>);
  static_assert(sus::concepts::from::From<i16, int16_t>);
  static_assert(sus::concepts::from::From<i16, int32_t>);
  static_assert(sus::concepts::from::From<i16, int64_t>);
  static_assert(sus::concepts::from::From<i16, uint8_t>);
  static_assert(sus::concepts::from::From<i16, uint16_t>);
  static_assert(sus::concepts::from::From<i16, uint32_t>);
  static_assert(sus::concepts::from::From<i16, uint64_t>);

  EXPECT_EQ(i16::from(char{2}), 2_i16);
  EXPECT_EQ(i16::from(size_t{2}), 2_i16);
  EXPECT_EQ(i16::from(int8_t{2}), 2_i16);
  EXPECT_EQ(i16::from(int16_t{2}), 2_i16);
  EXPECT_EQ(i16::from(int32_t{2}), 2_i16);
  EXPECT_EQ(i16::from(int64_t{2}), 2_i16);
  EXPECT_EQ(i16::from(uint8_t{2}), 2_i16);
  EXPECT_EQ(i16::from(uint16_t{2}), 2_i16);
  EXPECT_EQ(i16::from(uint32_t{2}), 2_i16);
  EXPECT_EQ(i16::from(uint64_t{2}), 2_i16);

  static_assert(sus::concepts::from::From<i16, i8>);
  static_assert(sus::concepts::from::From<i16, i16>);
  static_assert(sus::concepts::from::From<i16, i32>);
  static_assert(sus::concepts::from::From<i16, i64>);
  static_assert(sus::concepts::from::From<i16, isize>);
  static_assert(sus::concepts::from::From<i16, u8>);
  static_assert(sus::concepts::from::From<i16, u16>);
  static_assert(sus::concepts::from::From<i16, u32>);
  static_assert(sus::concepts::from::From<i16, u64>);
  static_assert(sus::concepts::from::From<i16, usize>);

  EXPECT_EQ(i16::from(2_i8), 2_i16);
  EXPECT_EQ(i16::from(2_i16), 2_i16);
  EXPECT_EQ(i16::from(2_i32), 2_i16);
  EXPECT_EQ(i16::from(2_i64), 2_i16);
  EXPECT_EQ(i16::from(2_isize), 2_i16);
  EXPECT_EQ(i16::from(2_u8), 2_i16);
  EXPECT_EQ(i16::from(2_u16), 2_i16);
  EXPECT_EQ(i16::from(2_u32), 2_i16);
  EXPECT_EQ(i16::from(2_u64), 2_i16);
  EXPECT_EQ(i16::from(2_usize), 2_i16);
}

TEST(i16DeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(i16::from(int64_t{-1 - 0x7fff'ffff'ffff'ffff}), "");
  EXPECT_DEATH(i16::from(uint64_t{0xffff'ffff'ffff'ffff}), "");

  EXPECT_DEATH(i16::from(i32::MAX()), "");
  EXPECT_DEATH(i16::from(i64::MAX()), "");
  EXPECT_DEATH(i16::from(u16::MAX()), "");
  EXPECT_DEATH(i16::from(u32::MAX()), "");
  EXPECT_DEATH(i16::from(u64::MAX()), "");
  EXPECT_DEATH(i16::from(usize::MAX()), "");
#endif
}

TEST(i16, InvokeEverything) {
  auto i = 10_i16, j = 11_i16;
  auto s = 3_u16;
  auto a = sus::Array<u8, sizeof(i16)>::with_default();

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

  i == j;
  [[maybe_unused]] auto z = i >= j;
}
}  // namespace
