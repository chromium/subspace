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
#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(std::is_signed_v<decltype(isize::primitive_value)>);
static_assert(sizeof(decltype(isize::primitive_value)) == sizeof(void*));
static_assert(sizeof(isize) == sizeof(decltype(isize::primitive_value)));

// `isize` can be explicitly converted to an unsigned int of the same size.
static_assert(sizeof(isize) != sizeof(uint16_t) ||
              std::is_constructible_v<int16_t, isize>);
static_assert(sizeof(isize) != sizeof(uint32_t) ||
              std::is_constructible_v<int32_t, isize>);
static_assert(sizeof(isize) != sizeof(uint64_t) ||
              std::is_constructible_v<int64_t, isize>);

namespace behaviour {
using T = isize;
using From = decltype(isize::primitive_value);
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
static_assert(std::is_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T>);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_assignable_v<T>);
static_assert(std::is_trivially_destructible_v<T>);
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_copy_assignable_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_move_assignable_v<T>);
static_assert(std::is_nothrow_swappable_v<T>);
static_assert(std::is_constructible_v<T, From&&>);
static_assert(std::is_assignable_v<T, From&&>);
static_assert(std::is_constructible_v<T, const From&>);
static_assert(std::is_assignable_v<T, const From&>);
static_assert(std::is_constructible_v<T, From>);
static_assert(!std::is_trivially_constructible_v<T, From>);
static_assert(std::is_assignable_v<T, From>);
static_assert(std::is_nothrow_destructible_v<T>);
static_assert(sus::construct::MakeDefault<T>);
static_assert(sus::mem::relocate_one_by_memcpy<T>);
static_assert(sus::mem::relocate_array_by_memcpy<T>);
}  // namespace behaviour

// isize::MAX()
static_assert(sizeof(isize) != sizeof(i64)
                  ? isize::MAX().primitive_value == 0x7fffffff
                  : isize::MAX().primitive_value == 0x7fffffff'ffffffff);
template <class T>
concept MaxInRange =
    requires {
      {
        sizeof(isize) != sizeof(i64) ? 0x7fffffff_isize
                                     : 0x7fffffff'ffffffff_isize
        } -> std::same_as<T>;
      {
        isize(sizeof(isize) != sizeof(i64) ? 0x7fffffff : 0x7fffffff'ffffffff)
        } -> std::same_as<T>;
    };
static_assert(MaxInRange<isize>);

TEST(isize, Traits) {
  // ** Signed only **
  static_assert(sus::num::Neg<isize>);

  static_assert(sus::num::Add<isize, isize>);
  static_assert(sus::num::AddAssign<isize, isize>);
  static_assert(sus::num::Sub<isize, isize>);
  static_assert(sus::num::SubAssign<isize, isize>);
  static_assert(sus::num::Mul<isize, isize>);
  static_assert(sus::num::MulAssign<isize, isize>);
  static_assert(sus::num::Div<isize, isize>);
  static_assert(sus::num::DivAssign<isize, isize>);
  static_assert(sus::num::Rem<isize, isize>);
  static_assert(sus::num::RemAssign<isize, isize>);
  static_assert(sus::num::BitAnd<isize, isize>);
  static_assert(sus::num::BitAndAssign<isize, isize>);
  static_assert(sus::num::BitOr<isize, isize>);
  static_assert(sus::num::BitOrAssign<isize, isize>);
  static_assert(sus::num::BitXor<isize, isize>);
  static_assert(sus::num::BitXorAssign<isize, isize>);
  static_assert(sus::num::BitNot<isize>);
  static_assert(sus::num::Shl<isize>);
  static_assert(sus::num::ShlAssign<isize>);
  static_assert(sus::num::Shr<isize>);
  static_assert(sus::num::ShrAssign<isize>);

  static_assert(sus::ops::Ord<isize>);
  static_assert(1_isize >= 1_isize);
  static_assert(2_isize > 1_isize);
  static_assert(1_isize <= 1_isize);
  static_assert(1_isize < 2_isize);
  static_assert(sus::ops::Eq<isize>);
  static_assert(1_isize == 1_isize);
  static_assert(!(1_isize == 2_isize));
  static_assert(1_isize != 2_isize);
  static_assert(!(1_isize != 1_isize));

  // Verify constexpr.
  constexpr isize c =
      1_isize + 2_isize - 3_isize * 4_isize / 5_isize % 6_isize & 7_isize |
      8_isize ^ -9_isize;
  constexpr std::strong_ordering o = 2_isize <=> 3_isize;
}

TEST(isize, Literals) {
  // Hex.
  static_assert((0x12bC_isize).primitive_value == 0x12bC);
  static_assert((0X12bC_isize).primitive_value == 0X12bC);
  static_assert((0X0012bC_isize).primitive_value == 0X12bC);
  EXPECT_EQ((0x12bC_isize).primitive_value, 0x12bC);
  EXPECT_EQ((0X12bC_isize).primitive_value, 0X12bC);
  EXPECT_EQ((0X0012bC_isize).primitive_value, 0X12bC);
  // Binary.
  static_assert((0b101_isize).primitive_value == 0b101);
  static_assert((0B101_isize).primitive_value == 0B101);
  static_assert((0b00101_isize).primitive_value == 0b101);
  EXPECT_EQ((0b101_isize).primitive_value, 0b101);
  EXPECT_EQ((0B101_isize).primitive_value, 0B101);
  EXPECT_EQ((0b00101_isize).primitive_value, 0b101);
  // Octal.
  static_assert((0123_isize).primitive_value == 0123);
  static_assert((000123_isize).primitive_value == 0123);
  EXPECT_EQ((0123_isize).primitive_value, 0123);
  EXPECT_EQ((000123_isize).primitive_value, 0123);
  // Decimal.
  static_assert((0_isize).primitive_value == 0);
  static_assert((1_isize).primitive_value == 1);
  static_assert((12_isize).primitive_value == 12);
  static_assert((123_isize).primitive_value == 123);
  static_assert((1234_isize).primitive_value == 1234);
  static_assert((12345_isize).primitive_value == 12345);
}

TEST(isize, Constants) {
  constexpr auto max = isize::MAX();
  static_assert(std::same_as<decltype(max), const isize>);
  if constexpr (sizeof(isize) != sizeof(i64))
    EXPECT_EQ(max.primitive_value, 0x7fffffff);
  else
    EXPECT_EQ(max.primitive_value, 0x7fffffff'ffffffff);
  constexpr auto min = isize::MIN();
  static_assert(std::same_as<decltype(min), const isize>);
  if constexpr (sizeof(isize) != sizeof(i64))
    EXPECT_EQ(min.primitive_value, -0x7fffffff - 1);
  else
    EXPECT_EQ(min.primitive_value, -0x7fffffff'ffffffff - 1);
  constexpr auto bits = isize::BITS();
  static_assert(std::same_as<decltype(bits), const u32>);
  if constexpr (sizeof(isize) != sizeof(i64))
    EXPECT_EQ(bits, 32u);
  else
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

TEST(isize, FromPrimitive) {
  static_assert(sizeof(isize) > sizeof(int16_t));
  static_assert(sizeof(isize) <= sizeof(int64_t));
  static_assert(IsImplicitlyConvertible<int8_t, isize>);
  static_assert(IsImplicitlyConvertible<int16_t, isize>);
  static_assert(sizeof(int32_t) > sizeof(isize)
                    ? NotConvertible<int32_t, isize>
                    : IsImplicitlyConvertible<int32_t, isize>);
  static_assert(sizeof(int64_t) > sizeof(isize)
                    ? NotConvertible<int64_t, isize>
                    : IsImplicitlyConvertible<int64_t, isize>);
  static_assert(IsImplicitlyConvertible<uint8_t, isize>);
  static_assert(IsImplicitlyConvertible<uint16_t, isize>);
  static_assert(sizeof(uint32_t) >= sizeof(isize)
                    ? NotConvertible<uint32_t, isize>
                    : IsImplicitlyConvertible<uint32_t, isize>);
  static_assert(NotConvertible<uint64_t, isize>);
  static_assert(sizeof(size_t) >= sizeof(isize)
                    ? NotConvertible<size_t, isize>
                    : IsImplicitlyConvertible<size_t, isize>);
}

TEST(isize, ToPrimitive) {
  static_assert(sizeof(isize) > sizeof(int8_t)
                    ? NotConvertible<i64, int8_t>
                    : IsExplicitlyConvertible<i64, int8_t>);
  static_assert(sizeof(isize) > sizeof(int16_t)
                    ? NotConvertible<i64, int16_t>
                    : IsExplicitlyConvertible<i64, int16_t>);
  static_assert(sizeof(isize) > sizeof(int32_t)
                    ? NotConvertible<i64, int32_t>
                    : IsExplicitlyConvertible<i64, int32_t>);
  static_assert(sizeof(isize) > sizeof(int64_t)
                    ? NotConvertible<i64, int64_t>
                    : IsExplicitlyConvertible<i64, int64_t>);
  static_assert(NotConvertible<i64, uint8_t>);
  static_assert(NotConvertible<i64, uint16_t>);
  static_assert(NotConvertible<i8, uint32_t>);
  static_assert(NotConvertible<i64, uint64_t>);
  static_assert(NotConvertible<i64, size_t>);
}

TEST(isize, From) {
  static_assert(sus::construct::From<isize, char>);
  static_assert(sus::construct::From<isize, size_t>);
  static_assert(sus::construct::From<isize, int8_t>);
  static_assert(sus::construct::From<isize, int16_t>);
  static_assert(sus::construct::From<isize, int32_t>);
  static_assert(sus::construct::From<isize, int64_t>);
  static_assert(sus::construct::From<isize, uint8_t>);
  static_assert(sus::construct::From<isize, uint16_t>);
  static_assert(sus::construct::From<isize, uint32_t>);
  static_assert(sus::construct::From<isize, uint64_t>);

  EXPECT_EQ(isize::from(char{2}), 2_isize);
  EXPECT_EQ(isize::from(size_t{2}), 2_isize);
  EXPECT_EQ(isize::from(int8_t{2}), 2_isize);
  EXPECT_EQ(isize::from(int16_t{2}), 2_isize);
  EXPECT_EQ(isize::from(int32_t{2}), 2_isize);
  EXPECT_EQ(isize::from(int64_t{2}), 2_isize);
  EXPECT_EQ(isize::from(uint8_t{2}), 2_isize);
  EXPECT_EQ(isize::from(uint16_t{2}), 2_isize);
  EXPECT_EQ(isize::from(uint32_t{2}), 2_isize);
  EXPECT_EQ(isize::from(uint64_t{2}), 2_isize);

  static_assert(sus::construct::From<isize, i8>);
  static_assert(sus::construct::From<isize, i16>);
  static_assert(sus::construct::From<isize, i32>);
  static_assert(sus::construct::From<isize, i64>);
  static_assert(sus::construct::From<isize, isize>);
  static_assert(sus::construct::From<isize, u8>);
  static_assert(sus::construct::From<isize, u16>);
  static_assert(sus::construct::From<isize, u32>);
  static_assert(sus::construct::From<isize, u64>);
  static_assert(sus::construct::From<isize, usize>);

  EXPECT_EQ(isize::from(2_i8), 2_isize);
  EXPECT_EQ(isize::from(2_i16), 2_isize);
  EXPECT_EQ(isize::from(2_i32), 2_isize);
  EXPECT_EQ(isize::from(2_i64), 2_isize);
  EXPECT_EQ(isize::from(2_isize), 2_isize);
  EXPECT_EQ(isize::from(2_u8), 2_isize);
  EXPECT_EQ(isize::from(2_u16), 2_isize);
  EXPECT_EQ(isize::from(2_u32), 2_isize);
  EXPECT_EQ(isize::from(2_u64), 2_isize);
  EXPECT_EQ(isize::from(2_usize), 2_isize);
}

TEST(isizeDeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(isize::from(uint64_t{0xffff'ffff'ffff'ffff}), "");

  bool not64 = sizeof(isize) != sizeof(i64);
  if (not64) {
    EXPECT_DEATH(isize::from(i64::MAX()), "");
    EXPECT_DEATH(isize::from(u32::MAX()), "");
  }
  EXPECT_DEATH(isize::from(u64::MAX()), "");
  EXPECT_DEATH(isize::from(usize::MAX()), "");
#endif
}

TEST(isize, InvokeEverything) {
  auto i = 10_isize, j = 11_isize;
  auto s = 3_usize;
  auto a = sus::Array<u8, sizeof(isize)>::with_default();

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

  [[maybe_unused]] bool b = i == j;
  [[maybe_unused]] auto z = i >= j;
}
}  // namespace
