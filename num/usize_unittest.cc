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
#include "num/num_concepts.h"
#include "num/signed_integer.h"
#include "num/unsigned_integer.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(!std::is_signed_v<decltype(usize::primitive_value)>);
static_assert(sizeof(decltype(usize::primitive_value)) == sizeof(void*));
static_assert(sizeof(usize) == sizeof(decltype(usize::primitive_value)));

namespace behaviour {
using T = usize;
using From = decltype(usize::primitive_value);
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

// usize::MAX()
static_assert(sizeof(usize) != sizeof(u64)
                  ? usize::MAX().primitive_value == 0xffffffff
                  : usize::MAX().primitive_value == 0xffffffff'ffffffff);
template <class T>
concept MaxInRange =
    requires {
      {
        sizeof(usize) != sizeof(u64) ? 0xffffffff_usize
                                     : 0xffffffff'ffffffff_usize
        } -> std::same_as<T>;
      {
        usize(sizeof(usize) != sizeof(u64) ? 0xffffffff : 0xffffffff'ffffffff)
        } -> std::same_as<T>;
    };
static_assert(MaxInRange<usize>);

TEST(usize, Traits) {
  // ** Unsigned only
  static_assert(!sus::num::Neg<usize>);

  static_assert(sus::num::Add<usize, usize>);
  static_assert(sus::num::AddAssign<usize, usize>);
  static_assert(sus::num::Sub<usize, usize>);
  static_assert(sus::num::SubAssign<usize, usize>);
  static_assert(sus::num::Mul<usize, usize>);
  static_assert(sus::num::MulAssign<usize, usize>);
  static_assert(sus::num::Div<usize, usize>);
  static_assert(sus::num::DivAssign<usize, usize>);
  static_assert(sus::num::Rem<usize, usize>);
  static_assert(sus::num::RemAssign<usize, usize>);
  static_assert(sus::num::BitAnd<usize, usize>);
  static_assert(sus::num::BitAndAssign<usize, usize>);
  static_assert(sus::num::BitOr<usize, usize>);
  static_assert(sus::num::BitOrAssign<usize, usize>);
  static_assert(sus::num::BitXor<usize, usize>);
  static_assert(sus::num::BitXorAssign<usize, usize>);
  static_assert(sus::num::BitNot<usize>);
  static_assert(sus::num::Shl<usize>);
  static_assert(sus::num::ShlAssign<usize>);
  static_assert(sus::num::Shr<usize>);
  static_assert(sus::num::ShrAssign<usize>);

  static_assert(sus::num::Ord<usize, usize>);
  static_assert(1_usize >= 1_usize);
  static_assert(2_usize > 1_usize);
  static_assert(1_usize <= 1_usize);
  static_assert(1_usize < 2_usize);
  static_assert(sus::num::Eq<usize, usize>);
  static_assert(1_usize == 1_usize);
  static_assert(!(1_usize == 2_usize));
  static_assert(1_usize != 2_usize);
  static_assert(!(1_usize != 1_usize));

  // Verify constexpr.
  constexpr usize c =
      1_usize + 2_usize - 3_usize * 4_usize / 5_usize % 6_usize & 7_usize |
      8_usize ^ 9_usize;
  constexpr std::strong_ordering o = 2_usize <=> 3_usize;
}

TEST(usize, Literals) {
  // Hex.
  static_assert((0x123abC_usize).primitive_value == 0x123abC);
  static_assert((0X123abC_usize).primitive_value == 0X123abC);
  static_assert((0X00123abC_usize).primitive_value == 0X123abC);
  EXPECT_EQ((0x123abC_usize).primitive_value, 0x123abC);
  EXPECT_EQ((0X123abC_usize).primitive_value, 0X123abC);
  EXPECT_EQ((0X00123abC_usize).primitive_value, 0X123abC);
  // Binary.
  static_assert((0b101_usize).primitive_value == 0b101);
  static_assert((0B101_usize).primitive_value == 0B101);
  static_assert((0b00101_usize).primitive_value == 0b101);
  EXPECT_EQ((0b101_usize).primitive_value, 0b101);
  EXPECT_EQ((0B101_usize).primitive_value, 0B101);
  EXPECT_EQ((0b00101_usize).primitive_value, 0b101);
  // Octal.
  static_assert((0123_usize).primitive_value == 0123);
  static_assert((000123_usize).primitive_value == 0123);
  EXPECT_EQ((0123_usize).primitive_value, 0123);
  EXPECT_EQ((000123_usize).primitive_value, 0123);
  // Decimal.
  static_assert((0_usize).primitive_value == 0);
  static_assert((1_usize).primitive_value == 1);
  static_assert((12_usize).primitive_value == 12);
  static_assert((123_usize).primitive_value == 123);
  static_assert((1234_usize).primitive_value == 1234);
  static_assert((12345_usize).primitive_value == 12345);
  static_assert((123456_usize).primitive_value == 123456);
  static_assert((1234567_usize).primitive_value == 1234567);
  static_assert((12345678_usize).primitive_value == 12345678);
  static_assert((123456789_usize).primitive_value == 123456789);
  static_assert((1234567891_usize).primitive_value == 1234567891);
}

TEST(usize, Constants) {
  constexpr auto max = usize::MAX();
  static_assert(std::same_as<decltype(max), const usize>);
  if constexpr (sizeof(usize) != sizeof(u64))
    EXPECT_EQ(max.primitive_value, 0xffffffffu);
  else
    EXPECT_EQ(max.primitive_value, 0xffffffff'ffffffffu);
  constexpr auto min = usize::MIN();
  static_assert(std::same_as<decltype(min), const usize>);
  EXPECT_EQ(min.primitive_value, 0u);
  constexpr auto bits = usize::BITS();
  static_assert(std::same_as<decltype(bits), const u32>);
  if constexpr (sizeof(usize) != sizeof(u64))
    EXPECT_EQ(bits, 32u);
  else
    EXPECT_EQ(bits, 64u);
}

TEST(usize, From) {
  static_assert(sus::construct::From<usize, char>);
  static_assert(sus::construct::From<usize, size_t>);
  static_assert(sus::construct::From<usize, int8_t>);
  static_assert(sus::construct::From<usize, int16_t>);
  static_assert(sus::construct::From<usize, int32_t>);
  static_assert(sus::construct::From<usize, int64_t>);
  static_assert(sus::construct::From<usize, uint8_t>);
  static_assert(sus::construct::From<usize, uint16_t>);
  static_assert(sus::construct::From<usize, uint32_t>);
  static_assert(sus::construct::From<usize, uint64_t>);

  EXPECT_EQ(usize::from(char{2}), 2_usize);
  EXPECT_EQ(usize::from(size_t{2}), 2_usize);
  EXPECT_EQ(usize::from(int8_t{2}), 2_usize);
  EXPECT_EQ(usize::from(int16_t{2}), 2_usize);
  EXPECT_EQ(usize::from(int32_t{2}), 2_usize);
  EXPECT_EQ(usize::from(int64_t{2}), 2_usize);
  EXPECT_EQ(usize::from(uint8_t{2}), 2_usize);
  EXPECT_EQ(usize::from(uint16_t{2}), 2_usize);
  EXPECT_EQ(usize::from(uint32_t{2}), 2_usize);
  EXPECT_EQ(usize::from(uint64_t{2}), 2_usize);

  static_assert(sus::construct::From<usize, i8>);
  static_assert(sus::construct::From<usize, i16>);
  static_assert(sus::construct::From<usize, i32>);
  static_assert(sus::construct::From<usize, i64>);
  static_assert(sus::construct::From<usize, isize>);
  static_assert(sus::construct::From<usize, u8>);
  static_assert(sus::construct::From<usize, u16>);
  static_assert(sus::construct::From<usize, u32>);
  static_assert(sus::construct::From<usize, u64>);
  static_assert(sus::construct::From<usize, usize>);

  EXPECT_EQ(usize::from(2_i8), 2_usize);
  EXPECT_EQ(usize::from(2_i16), 2_usize);
  EXPECT_EQ(usize::from(2_i32), 2_usize);
  EXPECT_EQ(usize::from(2_i64), 2_usize);
  EXPECT_EQ(usize::from(2_isize), 2_usize);
  EXPECT_EQ(usize::from(2_u8), 2_usize);
  EXPECT_EQ(usize::from(2_u16), 2_usize);
  EXPECT_EQ(usize::from(2_u32), 2_usize);
  EXPECT_EQ(usize::from(2_u64), 2_usize);
  EXPECT_EQ(usize::from(2_usize), 2_usize);
}

TEST(usizeDeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(usize::from(int64_t{-1}), "");
  EXPECT_DEATH(usize::from(int64_t{-1 - 0x7fff'ffff'ffff'ffff}), "");
  bool not64 =
      sizeof(usize) != sizeof(u64);  // `if constexpr` breaks death tests.
  if (not64) {
    EXPECT_DEATH(usize::from(uint64_t{0xffff'ffff'ffff'ffff}), "");
  }

  EXPECT_DEATH(usize::from(-1_i8), "");
  EXPECT_DEATH(usize::from(-1_i16), "");
  EXPECT_DEATH(usize::from(-1_i32), "");
  EXPECT_DEATH(usize::from(-1_i64), "");
  EXPECT_DEATH(usize::from(-1_isize), "");
#endif
}

TEST(usize, InvokeEverything) {
  auto i = 10_usize, j = 11_usize;
  auto s = 3_isize;
  auto a = sus::Array<u8, sizeof(usize)>::with_default();

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

  i == j;
  [[maybe_unused]] auto z = i >= j;
}

}  // namespace
