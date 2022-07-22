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
#include "concepts/make_default.h"
#include "mem/__private/relocate.h"
#include "num/i32.h"
#include "num/num_concepts.h"
#include "num/u32.h"
#include "option/option.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"
#include "tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(!std::is_signed_v<decltype(usize::primitive_value)>);
static_assert(sizeof(decltype(usize::primitive_value)) == sizeof(uintptr_t));
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
static_assert(sus::concepts::MakeDefault<T>::has_concept, "");
static_assert(sus::mem::__private::relocate_one_by_memcpy_v<T>, "");
static_assert(sus::mem::__private::relocate_array_by_memcpy_v<T>, "");
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
  if (sizeof(usize) != sizeof(u64))
    EXPECT_EQ(max.primitive_value, 0xffffffffu);
  else
    EXPECT_EQ(max.primitive_value, 0xffffffff'ffffffffu);
  constexpr auto min = usize::MIN();
  static_assert(std::same_as<decltype(min), const usize>);
  EXPECT_EQ(min.primitive_value, 0u);
  constexpr auto bits = usize::BITS();
  static_assert(std::same_as<decltype(bits), const u32>);
  if (sizeof(usize) != sizeof(u64))
    EXPECT_EQ(bits, 32u);
  else
    EXPECT_EQ(bits, 64u);
}

TEST(u32, From) {
  static_assert(sus::concepts::from::From<u32, char>);
  static_assert(sus::concepts::from::From<u32, size_t>);
  static_assert(sus::concepts::from::From<u32, int8_t>);
  static_assert(sus::concepts::from::From<u32, int16_t>);
  static_assert(sus::concepts::from::From<u32, int32_t>);
  static_assert(sus::concepts::from::From<u32, int64_t>);
  static_assert(sus::concepts::from::From<u32, uint8_t>);
  static_assert(sus::concepts::from::From<u32, uint16_t>);
  static_assert(sus::concepts::from::From<u32, uint32_t>);
  static_assert(sus::concepts::from::From<u32, uint64_t>);

  EXPECT_EQ(u32::from(char{2}), 2_u32);
  EXPECT_EQ(u32::from(size_t{2}), 2_u32);
  EXPECT_EQ(u32::from(int8_t{2}), 2_u32);
  EXPECT_EQ(u32::from(int16_t{2}), 2_u32);
  EXPECT_EQ(u32::from(int32_t{2}), 2_u32);
  EXPECT_EQ(u32::from(int64_t{2}), 2_u32);
  EXPECT_EQ(u32::from(uint8_t{2}), 2_u32);
  EXPECT_EQ(u32::from(uint16_t{2}), 2_u32);
  EXPECT_EQ(u32::from(uint32_t{2}), 2_u32);
  EXPECT_EQ(u32::from(uint64_t{2}), 2_u32);

  // TODO: Add all the integer types as they exist.
  static_assert(sus::concepts::from::From<usize, i32>);
  static_assert(sus::concepts::from::From<usize, u32>);
  static_assert(sus::concepts::from::From<usize, usize>);

  // TODO: Add all the integer types as they exist.
  EXPECT_EQ(usize::from(2_i32), 2_usize);
  EXPECT_EQ(usize::from(2_u32), 2_usize);
  EXPECT_EQ(usize::from(2_usize), 2_usize);
}

TEST(u32DeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::from(int64_t{-1}), "");
  EXPECT_DEATH(u32::from(int64_t{-1 - 0x7fff'ffff'ffff'ffff}), "");
  if (sizeof(usize) != sizeof(u64)) {
    EXPECT_DEATH(u32::from(uint64_t{0xffff'ffff'ffff'ffff}), "");
  }

  // TODO: Add all the signed integer types as they exist.
  EXPECT_DEATH(u32::from(-1_i32), "");
#endif
}

}  // namespace
