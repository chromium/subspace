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
#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(!std::is_signed_v<decltype(u64::primitive_value)>);
static_assert(sizeof(decltype(u64::primitive_value)) == 8);
static_assert(sizeof(u64) == sizeof(decltype(u64::primitive_value)));

namespace behaviour {
using T = u64;
using From = decltype(u64::primitive_value);
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

// u64::MAX()
static_assert(u64::MAX().primitive_value == 0xffffffff'ffffffff);
template <class T>
concept MaxInRange = requires {
                       { 0xffffffff'ffffffff_u64 } -> std::same_as<T>;
                       { u64(0xffffffff'ffffffff) } -> std::same_as<T>;
                     };
static_assert(MaxInRange<u64>);

TEST(u64, Traits) {
  // ** Unsigned only
  static_assert(!sus::num::Neg<u64>);

  static_assert(sus::num::Add<u64, u64>);
  static_assert(sus::num::AddAssign<u64, u64>);
  static_assert(sus::num::Sub<u64, u64>);
  static_assert(sus::num::SubAssign<u64, u64>);
  static_assert(sus::num::Mul<u64, u64>);
  static_assert(sus::num::MulAssign<u64, u64>);
  static_assert(sus::num::Div<u64, u64>);
  static_assert(sus::num::DivAssign<u64, u64>);
  static_assert(sus::num::Rem<u64, u64>);
  static_assert(sus::num::RemAssign<u64, u64>);
  static_assert(sus::num::BitAnd<u64, u64>);
  static_assert(sus::num::BitAndAssign<u64, u64>);
  static_assert(sus::num::BitOr<u64, u64>);
  static_assert(sus::num::BitOrAssign<u64, u64>);
  static_assert(sus::num::BitXor<u64, u64>);
  static_assert(sus::num::BitXorAssign<u64, u64>);
  static_assert(sus::num::BitNot<u64>);
  static_assert(sus::num::Shl<u64>);
  static_assert(sus::num::ShlAssign<u64>);
  static_assert(sus::num::Shr<u64>);
  static_assert(sus::num::ShrAssign<u64>);

  static_assert(sus::ops::Ord<u64>);
  static_assert(1_u64 >= 1_u64);
  static_assert(2_u64 > 1_u64);
  static_assert(1_u64 <= 1_u64);
  static_assert(1_u64 < 2_u64);
  static_assert(sus::ops::Eq<u64>);
  static_assert(1_u64 == 1_u64);
  static_assert(!(1_u64 == 2_u64));
  static_assert(1_u64 != 2_u64);
  static_assert(!(1_u64 != 1_u64));

  // Verify constexpr.
  constexpr u64 c =
      1_u64 + 2_u64 - 3_u64 * 4_u64 / 5_u64 % 6_u64 & 7_u64 | 8_u64 ^ 9_u64;
  constexpr std::strong_ordering o = 2_u64 <=> 3_u64;
}

TEST(u64, Literals) {
  // Hex.
  static_assert((0x123abC_u64).primitive_value == 0x123abC);
  static_assert((0X123abC_u64).primitive_value == 0X123abC);
  static_assert((0X00123abC_u64).primitive_value == 0X123abC);
  EXPECT_EQ((0x123abC_u64).primitive_value, 0x123abC);
  EXPECT_EQ((0X123abC_u64).primitive_value, 0X123abC);
  EXPECT_EQ((0X00123abC_u64).primitive_value, 0X123abC);
  // Binary.
  static_assert((0b101_u64).primitive_value == 0b101);
  static_assert((0B101_u64).primitive_value == 0B101);
  static_assert((0b00101_u64).primitive_value == 0b101);
  EXPECT_EQ((0b101_u64).primitive_value, 0b101);
  EXPECT_EQ((0B101_u64).primitive_value, 0B101);
  EXPECT_EQ((0b00101_u64).primitive_value, 0b101);
  // Octal.
  static_assert((0123_u64).primitive_value == 0123);
  static_assert((000123_u64).primitive_value == 0123);
  EXPECT_EQ((0123_u64).primitive_value, 0123);
  EXPECT_EQ((000123_u64).primitive_value, 0123);
  // Decimal.
  static_assert((0_u64).primitive_value == 0);
  static_assert((1_u64).primitive_value == 1);
  static_assert((12_u64).primitive_value == 12);
  static_assert((123_u64).primitive_value == 123);
  static_assert((1234_u64).primitive_value == 1234);
  static_assert((12345_u64).primitive_value == 12345);
  static_assert((123456_u64).primitive_value == 123456);
  static_assert((1234567_u64).primitive_value == 1234567);
  static_assert((12345678_u64).primitive_value == 12345678);
  static_assert((123456789_u64).primitive_value == 123456789);
  static_assert((1234567891_u64).primitive_value == 1234567891);
}

TEST(u64, Constants) {
  constexpr auto max = u64::MAX();
  static_assert(std::same_as<decltype(max), const u64>);
  EXPECT_EQ(max.primitive_value, 0xffffffff'ffffffffu);
  constexpr auto min = u64::MIN();
  static_assert(std::same_as<decltype(min), const u64>);
  EXPECT_EQ(min.primitive_value, 0u);
  constexpr auto bits = u64::BITS();
  static_assert(std::same_as<decltype(bits), const u32>);
  EXPECT_EQ(bits, 64u);
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

TEST(u64, FromPrimitive) {
  static_assert(NotConvertible<int8_t, u64>);
  static_assert(NotConvertible<int16_t, u64>);
  static_assert(NotConvertible<int32_t, u64>);
  static_assert(NotConvertible<int64_t, u64>);
  static_assert(IsImplicitlyConvertible<uint8_t, u64>);
  static_assert(IsImplicitlyConvertible<uint16_t, u64>);
  static_assert(IsImplicitlyConvertible<uint32_t, u64>);
  static_assert(IsImplicitlyConvertible<uint64_t, u64>);
  static_assert(IsImplicitlyConvertible<size_t, u64>);
  static_assert(sizeof(size_t) <= sizeof(u64));
}

TEST(u64, ToPrimitive) {
  static_assert(NotConvertible<u64, int8_t>);
  static_assert(NotConvertible<u64, int16_t>);
  static_assert(NotConvertible<u64, int32_t>);
  static_assert(NotConvertible<u64, int64_t>);
  static_assert(NotConvertible<u64, uint8_t>);
  static_assert(NotConvertible<u64, uint16_t>);
  static_assert(NotConvertible<u64, uint32_t>);
  static_assert(IsExplicitlyConvertible<u64, uint64_t>);
  static_assert(sizeof(u64) <= sizeof(size_t)
                    ? IsExplicitlyConvertible<u64, size_t>
                    : NotConvertible<u64, size_t>);
}

TEST(u64, From) {
  static_assert(sus::construct::From<u64, char>);
  static_assert(sus::construct::From<u64, size_t>);
  static_assert(sus::construct::From<u64, int8_t>);
  static_assert(sus::construct::From<u64, int16_t>);
  static_assert(sus::construct::From<u64, int32_t>);
  static_assert(sus::construct::From<u64, int64_t>);
  static_assert(sus::construct::From<u64, uint8_t>);
  static_assert(sus::construct::From<u64, uint16_t>);
  static_assert(sus::construct::From<u64, uint32_t>);
  static_assert(sus::construct::From<u64, uint64_t>);

  EXPECT_EQ(u64::from(char{2}), 2_u64);
  EXPECT_EQ(u64::from(size_t{2}), 2_u64);
  EXPECT_EQ(u64::from(int8_t{2}), 2_u64);
  EXPECT_EQ(u64::from(int16_t{2}), 2_u64);
  EXPECT_EQ(u64::from(int32_t{2}), 2_u64);
  EXPECT_EQ(u64::from(int64_t{2}), 2_u64);
  EXPECT_EQ(u64::from(uint8_t{2}), 2_u64);
  EXPECT_EQ(u64::from(uint16_t{2}), 2_u64);
  EXPECT_EQ(u64::from(uint32_t{2}), 2_u64);
  EXPECT_EQ(u64::from(uint64_t{2}), 2_u64);

  static_assert(sus::construct::From<u64, i8>);
  static_assert(sus::construct::From<u64, i16>);
  static_assert(sus::construct::From<u64, i32>);
  static_assert(sus::construct::From<u64, i64>);
  static_assert(sus::construct::From<u64, isize>);
  static_assert(sus::construct::From<u64, u8>);
  static_assert(sus::construct::From<u64, u16>);
  static_assert(sus::construct::From<u64, u32>);
  static_assert(sus::construct::From<u64, u64>);
  static_assert(sus::construct::From<u64, usize>);

  EXPECT_EQ(u64::from(2_i8), 2_u64);
  EXPECT_EQ(u64::from(2_i16), 2_u64);
  EXPECT_EQ(u64::from(2_i32), 2_u64);
  EXPECT_EQ(u64::from(2_i64), 2_u64);
  EXPECT_EQ(u64::from(2_isize), 2_u64);
  EXPECT_EQ(u64::from(2_u8), 2_u64);
  EXPECT_EQ(u64::from(2_u16), 2_u64);
  EXPECT_EQ(u64::from(2_u32), 2_u64);
  EXPECT_EQ(u64::from(2_u64), 2_u64);
  EXPECT_EQ(u64::from(2_usize), 2_u64);
}

TEST(u64DeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u64::from(int64_t{-1}), "");
  EXPECT_DEATH(u64::from(int64_t{-1 - 0x7fff'ffff'ffff'ffff}), "");

  EXPECT_DEATH(u64::from(-1_i8), "");
  EXPECT_DEATH(u64::from(-1_i16), "");
  EXPECT_DEATH(u64::from(-1_i32), "");
  EXPECT_DEATH(u64::from(-1_i64), "");
  EXPECT_DEATH(u64::from(-1_isize), "");
#endif
}

TEST(u64, InvokeEverything) {
  auto i = 10_u64, j = 11_u64;
  auto s = 3_i64;
  auto a = sus::Array<u8, sizeof(u64)>::with_default();

  i.abs_diff(j);

  i.checked_add(j);
  i.checked_add_signed(s);
  i.overflowing_add(j);
  i.overflowing_add_signed(s);
  i.saturating_add(j);
  i.saturating_add_signed(s);
  i.unchecked_add(unsafe_fn, j);
  i.wrapping_add(j);
  i.wrapping_add_signed(s);

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
  i.overflowing_sub(j);
  i.saturating_sub(j);
  i.unchecked_sub(unsafe_fn, j);
  i.wrapping_sub(j);

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

  i.next_power_of_two();
  i.checked_next_power_of_two();
  i.wrapping_next_power_of_two();

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
