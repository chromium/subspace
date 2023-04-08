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

#include "googletest/include/gtest/gtest.h"
#include "subspace/construct/default.h"
#include "subspace/construct/into.h"
#include "subspace/containers/array.h"
#include "subspace/iter/__private/step.h"
#include "subspace/mem/relocate.h"
#include "subspace/num/num_concepts.h"
#include "subspace/num/signed_integer.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"
#include "subspace/tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(!std::is_signed_v<decltype(u32::primitive_value)>);
static_assert(sizeof(decltype(u32::primitive_value)) == 4);
static_assert(sizeof(u32) == sizeof(decltype(u32::primitive_value)));

static_assert(sus::mem::Copy<u32>);
static_assert(sus::mem::TrivialCopy<u32>);
static_assert(sus::mem::Clone<u32>);
static_assert(sus::mem::relocate_by_memcpy<u32>);
static_assert(sus::mem::Move<u32>);

namespace behaviour {
using T = u32;
using From = decltype(u32::primitive_value);
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
static_assert(sus::mem::relocate_by_memcpy<T>, "");
}  // namespace behaviour

// u32::MAX
static_assert(u32::MAX.primitive_value == 0xffffffff);
template <class T>
concept MaxInRange = requires {
  { 0xffffffff_u32 } -> std::same_as<T>;
  { u32(0xffffffff) } -> std::same_as<T>;
};
static_assert(MaxInRange<u32>);

// std hashing
static_assert(std::same_as<decltype(std::hash<u32>()(0_u32)), size_t>);
static_assert(std::same_as<decltype(std::equal_to<u32>()(0_u32, 1_u32)), bool>);

TEST(u32, Traits) {
  static_assert(sus::iter::__private::Step<u32>);

  // ** Unsigned only
  static_assert(!sus::num::Neg<u32>);

  static_assert(sus::num::Add<u32, u32>);
  static_assert(sus::num::AddAssign<u32, u32>);
  static_assert(sus::num::Sub<u32, u32>);
  static_assert(sus::num::SubAssign<u32, u32>);
  static_assert(sus::num::Mul<u32, u32>);
  static_assert(sus::num::MulAssign<u32, u32>);
  static_assert(sus::num::Div<u32, u32>);
  static_assert(sus::num::DivAssign<u32, u32>);
  static_assert(sus::num::Rem<u32, u32>);
  static_assert(sus::num::RemAssign<u32, u32>);
  static_assert(sus::num::BitAnd<u32, u32>);
  static_assert(sus::num::BitAndAssign<u32, u32>);
  static_assert(sus::num::BitOr<u32, u32>);
  static_assert(sus::num::BitOrAssign<u32, u32>);
  static_assert(sus::num::BitXor<u32, u32>);
  static_assert(sus::num::BitXorAssign<u32, u32>);
  static_assert(sus::num::BitNot<u32>);
  static_assert(sus::num::Shl<u32>);
  static_assert(sus::num::ShlAssign<u32>);
  static_assert(sus::num::Shr<u32>);
  static_assert(sus::num::ShrAssign<u32>);

  static_assert(sus::ops::Ord<u32, uint8_t>);
  static_assert(sus::ops::Ord<u32, uint16_t>);
  static_assert(sus::ops::Ord<u32, uint32_t>);
  static_assert(sus::ops::Ord<u32, uint64_t>);
  static_assert(sus::ops::Ord<u32, size_t>);
  static_assert(sus::ops::Ord<u32, u8>);
  static_assert(sus::ops::Ord<u32, u16>);
  static_assert(sus::ops::Ord<u32, u32>);
  static_assert(sus::ops::Ord<u32, u64>);
  static_assert(sus::ops::Ord<u32, usize>);
  static_assert(1_u32 >= 1_u32);
  static_assert(2_u32 > 1_u32);
  static_assert(1_u32 <= 1_u32);
  static_assert(1_u32 < 2_u32);
  static_assert(sus::ops::Eq<u32, uint8_t>);
  static_assert(sus::ops::Eq<u32, uint16_t>);
  static_assert(sus::ops::Eq<u32, uint32_t>);
  static_assert(sus::ops::Eq<u32, uint64_t>);
  static_assert(sus::ops::Eq<u32, size_t>);
  static_assert(sus::ops::Eq<u32, u8>);
  static_assert(sus::ops::Eq<u32, u16>);
  static_assert(sus::ops::Eq<u32, u32>);
  static_assert(sus::ops::Eq<u32, u64>);
  static_assert(sus::ops::Eq<u32, usize>);
  static_assert(1_u32 == 1_u32);
  static_assert(!(1_u32 == 2_u32));
  static_assert(1_u32 != 2_u32);
  static_assert(!(1_u32 != 1_u32));

  // Verify constexpr.
  [[maybe_unused]] constexpr u32 c =
      1_u32 + 2_u32 - 3_u32 * 4_u32 / 5_u32 % 6_u32 & 7_u32 | 8_u32 ^ 9_u32;
  [[maybe_unused]] constexpr std::strong_ordering o = 2_u32 <=> 3_u32;
}

TEST(u32, Literals) {
  // Hex.
  static_assert((0x123abC_u32).primitive_value == 0x123abC);
  static_assert((0X123abC_u32).primitive_value == 0X123abC);
  static_assert((0X00123abC_u32).primitive_value == 0X123abC);
  EXPECT_EQ((0x123abC_u32).primitive_value, 0x123abC);
  EXPECT_EQ((0X123abC_u32).primitive_value, 0X123abC);
  EXPECT_EQ((0X00123abC_u32).primitive_value, 0X123abC);
  // Binary.
  static_assert((0b101_u32).primitive_value == 0b101);
  static_assert((0B101_u32).primitive_value == 0B101);
  static_assert((0b00101_u32).primitive_value == 0b101);
  EXPECT_EQ((0b101_u32).primitive_value, 0b101);
  EXPECT_EQ((0B101_u32).primitive_value, 0B101);
  EXPECT_EQ((0b00101_u32).primitive_value, 0b101);
  // Octal.
  static_assert((0123_u32).primitive_value == 0123);
  static_assert((000123_u32).primitive_value == 0123);
  EXPECT_EQ((0123_u32).primitive_value, 0123);
  EXPECT_EQ((000123_u32).primitive_value, 0123);
  // Decimal.
  static_assert((0_u32).primitive_value == 0);
  static_assert((1_u32).primitive_value == 1);
  static_assert((12_u32).primitive_value == 12);
  static_assert((123_u32).primitive_value == 123);
  static_assert((1234_u32).primitive_value == 1234);
  static_assert((12345_u32).primitive_value == 12345);
  static_assert((123456_u32).primitive_value == 123456);
  static_assert((1234567_u32).primitive_value == 1234567);
  static_assert((12345678_u32).primitive_value == 12345678);
  static_assert((123456789_u32).primitive_value == 123456789);
  static_assert((1234567891_u32).primitive_value == 1234567891);
}

TEST(u32, Constants) {
  constexpr auto max = u32::MAX;
  static_assert(std::same_as<decltype(max), const u32>);
  EXPECT_EQ(max.primitive_value, 0xffffffffu);
  constexpr auto min = u32::MIN;
  static_assert(std::same_as<decltype(min), const u32>);
  EXPECT_EQ(min.primitive_value, 0u);
  constexpr auto bits = u32::BITS;
  static_assert(std::same_as<decltype(bits), const u32>);
  EXPECT_EQ(bits, 32u);
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

#define ENUM(kind, T)                      \
  decltype([]() {                          \
    enum kind E : T {                      \
      X,                                   \
      Y,                                   \
      Z,                                   \
      MIN = std::numeric_limits<T>::min(), \
      MAX = std::numeric_limits<T>::max()  \
    };                                     \
    return E::Z;                           \
  }())

TEST(u32, FromPrimitive) {
  // They all register as convertible, but will fail at compile time if the
  // value itself won't fit.
  static_assert(IsImplicitlyConvertible<int8_t, u32>);
  static_assert(IsImplicitlyConvertible<int16_t, u32>);
  static_assert(IsImplicitlyConvertible<int32_t, u32>);
  static_assert(IsImplicitlyConvertible<int64_t, u32>);
  static_assert(IsImplicitlyConvertible<uint8_t, u32>);
  static_assert(IsImplicitlyConvertible<uint16_t, u32>);
  static_assert(IsImplicitlyConvertible<uint32_t, u32>);
  static_assert(IsImplicitlyConvertible<uint64_t, u32>);
  static_assert(IsImplicitlyConvertible<size_t, u32>);

  static_assert(IsImplicitlyConvertible<ENUM(, int8_t), u32>);
  static_assert(IsImplicitlyConvertible<ENUM(, int16_t), u32>);
  static_assert(IsImplicitlyConvertible<ENUM(, int32_t), u32>);
  static_assert(IsImplicitlyConvertible<ENUM(, int64_t), u32>);
  static_assert(IsImplicitlyConvertible<ENUM(, uint8_t), u32>);
  static_assert(IsImplicitlyConvertible<ENUM(, uint16_t), u32>);
  static_assert(IsImplicitlyConvertible<ENUM(, uint32_t), u32>);
  static_assert(IsImplicitlyConvertible<ENUM(, uint64_t), u32>);
  static_assert(IsImplicitlyConvertible<ENUM(, size_t), u32>);

  static_assert(IsExplicitlyConvertible<ENUM(class, int8_t), u32>);
  static_assert(IsExplicitlyConvertible<ENUM(class, int16_t), u32>);
  static_assert(IsExplicitlyConvertible<ENUM(class, int32_t), u32>);
  static_assert(IsExplicitlyConvertible<ENUM(class, int64_t), u32>);
  static_assert(IsExplicitlyConvertible<ENUM(class, uint8_t), u32>);
  static_assert(IsExplicitlyConvertible<ENUM(class, uint16_t), u32>);
  static_assert(IsExplicitlyConvertible<ENUM(class, uint32_t), u32>);
  static_assert(IsExplicitlyConvertible<ENUM(class, uint64_t), u32>);
  static_assert(IsExplicitlyConvertible<ENUM(class, size_t), u32>);
}

TEST(u32, ToPrimitive) {
  static_assert(NotConvertible<u32, int8_t>);
  static_assert(NotConvertible<u32, int16_t>);
  static_assert(NotConvertible<u32, int32_t>);
  static_assert(IsExplicitlyConvertible<u32, int64_t>);
  static_assert(NotConvertible<u32, uint8_t>);
  static_assert(NotConvertible<u32, uint16_t>);
  static_assert(IsExplicitlyConvertible<u32, uint32_t>);
  static_assert(IsExplicitlyConvertible<u32, uint64_t>);
  static_assert(IsExplicitlyConvertible<u32, size_t>);
  static_assert(sizeof(u32) <= sizeof(size_t));
}

TEST(u32, From) {
  static_assert(sus::construct::From<u32, char>);
  static_assert(sus::construct::From<u32, size_t>);
  static_assert(sus::construct::From<u32, int8_t>);
  static_assert(sus::construct::From<u32, int16_t>);
  static_assert(sus::construct::From<u32, int32_t>);
  static_assert(sus::construct::From<u32, int64_t>);
  static_assert(sus::construct::From<u32, uint8_t>);
  static_assert(sus::construct::From<u32, uint16_t>);
  static_assert(sus::construct::From<u32, uint32_t>);
  static_assert(sus::construct::From<u32, uint64_t>);
  static_assert(sus::construct::TryFrom<u32, char>);
  static_assert(sus::construct::TryFrom<u32, size_t>);
  static_assert(sus::construct::TryFrom<u32, int8_t>);
  static_assert(sus::construct::TryFrom<u32, int16_t>);
  static_assert(sus::construct::TryFrom<u32, int32_t>);
  static_assert(sus::construct::TryFrom<u32, int64_t>);
  static_assert(sus::construct::TryFrom<u32, uint8_t>);
  static_assert(sus::construct::TryFrom<u32, uint16_t>);
  static_assert(sus::construct::TryFrom<u32, uint32_t>);
  static_assert(sus::construct::TryFrom<u32, uint64_t>);

  static_assert(sus::construct::From<u32, ENUM(, char)>);
  static_assert(sus::construct::From<u32, ENUM(, size_t)>);
  static_assert(sus::construct::From<u32, ENUM(, int8_t)>);
  static_assert(sus::construct::From<u32, ENUM(, int16_t)>);
  static_assert(sus::construct::From<u32, ENUM(, int32_t)>);
  static_assert(sus::construct::From<u32, ENUM(, int64_t)>);
  static_assert(sus::construct::From<u32, ENUM(, uint8_t)>);
  static_assert(sus::construct::From<u32, ENUM(, uint16_t)>);
  static_assert(sus::construct::From<u32, ENUM(, uint32_t)>);
  static_assert(sus::construct::From<u32, ENUM(, uint64_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, char)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, size_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, int8_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, int16_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, int32_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, int64_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, uint8_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, uint16_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, uint32_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(, uint64_t)>);

  static_assert(sus::construct::From<u32, ENUM(class, char)>);
  static_assert(sus::construct::From<u32, ENUM(class, size_t)>);
  static_assert(sus::construct::From<u32, ENUM(class, int8_t)>);
  static_assert(sus::construct::From<u32, ENUM(class, int16_t)>);
  static_assert(sus::construct::From<u32, ENUM(class, int32_t)>);
  static_assert(sus::construct::From<u32, ENUM(class, int64_t)>);
  static_assert(sus::construct::From<u32, ENUM(class, uint8_t)>);
  static_assert(sus::construct::From<u32, ENUM(class, uint16_t)>);
  static_assert(sus::construct::From<u32, ENUM(class, uint32_t)>);
  static_assert(sus::construct::From<u32, ENUM(class, uint64_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, char)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, size_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, int8_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, int16_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, int32_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, int64_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, uint8_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, uint16_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, uint32_t)>);
  static_assert(sus::construct::TryFrom<u32, ENUM(class, uint64_t)>);

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

  EXPECT_EQ(u32::try_from(char{2}).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(size_t{2}).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(int8_t{2}).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(int16_t{2}).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(int32_t{2}).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(int64_t{2}).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(uint8_t{2}).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(uint16_t{2}).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(uint32_t{2}).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(uint64_t{2}).unwrap(), 2_u32);

  EXPECT_TRUE(u32::try_from(int32_t{i32::MIN}).is_err());
  EXPECT_TRUE(u32::try_from(int32_t{i32::MAX}).is_ok());
  EXPECT_TRUE(u32::try_from(int64_t{i64::MIN}).is_err());
  EXPECT_TRUE(u32::try_from(int64_t{i64::MAX}).is_err());
  EXPECT_TRUE(u32::try_from(uint64_t{u64::MAX}).is_err());

  EXPECT_EQ(u32::from(ENUM(, char)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(, size_t)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(, int8_t)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(, int16_t)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(, int32_t)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(, int64_t)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(, uint8_t)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(, uint16_t)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(, uint32_t)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(, uint64_t)::Z), 2_u32);
  EXPECT_EQ(u32::from(ENUM(class, uint64_t)::Z), 2_u32);

  EXPECT_EQ(u32::try_from(ENUM(, char)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(, size_t)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(, int8_t)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(, int16_t)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(, int32_t)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(, int64_t)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(, uint8_t)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(, uint16_t)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(, uint32_t)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(, uint64_t)::Z).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(ENUM(class, uint64_t)::Z).unwrap(), 2_u32);

  EXPECT_TRUE(u32::try_from(ENUM(, int32_t)::MIN).is_err());
  EXPECT_TRUE(u32::try_from(ENUM(, int32_t)::MAX).is_ok());
  EXPECT_TRUE(u32::try_from(ENUM(, int64_t)::MIN).is_err());
  EXPECT_TRUE(u32::try_from(ENUM(, int64_t)::MAX).is_err());
  EXPECT_TRUE(u32::try_from(ENUM(, uint64_t)::MAX).is_err());
  EXPECT_TRUE(u32::try_from(ENUM(class, uint64_t)::MAX).is_err());

  EXPECT_EQ(u32::from_unchecked(unsafe_fn, char{2}), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, size_t{2}), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, int8_t{2}), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, int16_t{2}), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, int32_t{2}), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, int64_t{2}), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, uint8_t{2}), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, uint16_t{2}), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, uint32_t{2}), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, uint64_t{2}), 2_u32);

  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, char)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, size_t)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, int8_t)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, int16_t)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, int32_t)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, int64_t)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, uint8_t)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, uint16_t)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, uint32_t)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(, uint64_t)::Z), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, ENUM(class, uint64_t)::Z), 2_u32);

  static_assert(sus::construct::From<u32, i8>);
  static_assert(sus::construct::From<u32, i16>);
  static_assert(sus::construct::From<u32, i32>);
  static_assert(sus::construct::From<u32, i64>);
  static_assert(sus::construct::From<u32, isize>);
  static_assert(sus::construct::From<u32, u8>);
  static_assert(sus::construct::From<u32, u16>);
  static_assert(sus::construct::From<u32, u32>);
  static_assert(sus::construct::From<u32, u64>);
  static_assert(sus::construct::From<u32, usize>);
  static_assert(sus::construct::TryFrom<u32, i8>);
  static_assert(sus::construct::TryFrom<u32, i16>);
  static_assert(sus::construct::TryFrom<u32, i32>);
  static_assert(sus::construct::TryFrom<u32, i64>);
  static_assert(sus::construct::TryFrom<u32, isize>);
  static_assert(sus::construct::TryFrom<u32, u8>);
  static_assert(sus::construct::TryFrom<u32, u16>);
  static_assert(sus::construct::TryFrom<u32, u32>);
  static_assert(sus::construct::TryFrom<u32, u64>);
  static_assert(sus::construct::TryFrom<u32, usize>);

  EXPECT_EQ(u32::from(2_i8), 2_u32);
  EXPECT_EQ(u32::from(2_i16), 2_u32);
  EXPECT_EQ(u32::from(2_i32), 2_u32);
  EXPECT_EQ(u32::from(2_i64), 2_u32);
  EXPECT_EQ(u32::from(2_isize), 2_u32);
  EXPECT_EQ(u32::from(2_u8), 2_u32);
  EXPECT_EQ(u32::from(2_u16), 2_u32);
  EXPECT_EQ(u32::from(2_u32), 2_u32);
  EXPECT_EQ(u32::from(2_u64), 2_u32);
  EXPECT_EQ(u32::from(2_usize), 2_u32);

  EXPECT_EQ(u32::try_from(2_i8).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(2_i16).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(2_i32).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(2_i64).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(2_isize).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(2_u8).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(2_u16).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(2_u32).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(2_u64).unwrap(), 2_u32);
  EXPECT_EQ(u32::try_from(2_usize).unwrap(), 2_u32);

  EXPECT_TRUE(u32::try_from(i32::MIN).is_err());
  EXPECT_TRUE(u32::try_from(i32::MAX).is_ok());
  EXPECT_TRUE(u32::try_from(i64::MIN).is_err());
  EXPECT_TRUE(u32::try_from(i64::MAX).is_err());
  EXPECT_TRUE(u32::try_from(u64::MAX).is_err());

  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_i8), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_i16), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_i32), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_i64), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_isize), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_u8), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_u16), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_u32), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_u64), 2_u32);
  EXPECT_EQ(u32::from_unchecked(unsafe_fn, 2_usize), 2_u32);
}

TEST(u32DeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::from(int64_t{-1}), "");
  EXPECT_DEATH(u32::from(int64_t{-1 - 0x7fff'ffff'ffff'ffff}), "");
  EXPECT_DEATH(u32::from(uint64_t{0xffff'ffff'ffff'ffff}), "");

  EXPECT_DEATH(u32::from(ENUM(, int64_t)::MIN), "");
  EXPECT_DEATH(u32::from(ENUM(, int64_t)::MAX), "");
  EXPECT_DEATH(u32::from(ENUM(, uint64_t)::MAX), "");
  EXPECT_DEATH(u32::from(ENUM(class, int64_t)::MIN), "");
  EXPECT_DEATH(u32::from(ENUM(class, int64_t)::MAX), "");
  EXPECT_DEATH(u32::from(ENUM(class, uint64_t)::MAX), "");

  EXPECT_DEATH(u32::from(-1_i8), "");
  EXPECT_DEATH(u32::from(-1_i16), "");
  EXPECT_DEATH(u32::from(-1_i32), "");
  EXPECT_DEATH(u32::from(-1_i64), "");
  EXPECT_DEATH(u32::from(-1_isize), "");
#endif
}

TEST(u32, TryFromBoundaries) {
  // Signed primitives.
  EXPECT_TRUE(u32::try_from(int64_t{i64::from(u32::MAX)}).is_ok());
  EXPECT_TRUE(u32::try_from(int64_t{i64::from(u32::MAX) + 1}).is_err());
  EXPECT_TRUE(u32::try_from(int64_t{0}).is_ok());
  EXPECT_TRUE(u32::try_from(int64_t{-1}).is_err());

  // Signed integers.
  EXPECT_TRUE(u32::try_from(i64::from(u32::MAX)).is_ok());
  EXPECT_TRUE(u32::try_from(i64::from(u32::MAX) + 1).is_err());
  EXPECT_TRUE(u32::try_from(0_i32).is_ok());
  EXPECT_TRUE(u32::try_from(-1_i32).is_err());

  // Unsigned primitives.
  EXPECT_TRUE(u32::try_from(uint64_t{u64::from(u32::MAX)}).is_ok());
  EXPECT_TRUE(u32::try_from(uint64_t{u64::from(u32::MAX) + 1u}).is_err());

  // Unsigned integers.
  EXPECT_TRUE(u32::try_from(u64::from(u32::MAX)).is_ok());
  EXPECT_TRUE(u32::try_from(u64::from(u32::MAX) + 1u).is_err());
}

TEST(u32, AbsDiff) {
  [[maybe_unused]] constexpr auto a = (1_u32).abs_diff(10_u32);

  EXPECT_EQ((0_u32).abs_diff(0_u32), 0u);
  EXPECT_EQ((0_u32).abs_diff(123456_u32), 123456u);
  EXPECT_EQ((123456_u32).abs_diff(0_u32), 123456u);
  EXPECT_EQ((123456_u32).abs_diff(123456_u32), 0u);
  EXPECT_EQ(u32::MAX.abs_diff(u32::MIN), 0xffffffffu);
  EXPECT_EQ(u32::MIN.abs_diff(u32::MAX), 0xffffffffu);

  // lvalue.
  auto i = 9000_u32;
  auto j = 1000_u32;
  EXPECT_EQ(i.abs_diff(j), 8000u);
}

TEST(u32, Add) {
  constexpr auto a = 1_u32 + 3_u32;
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ(0_u32 + 0_u32, 0_u32);
  EXPECT_EQ(12345_u32 + 1_u32, 12346_u32);
  EXPECT_EQ(u32::MAX + 0_u32, u32::MAX);
  EXPECT_EQ(u32::MIN + 0_u32, u32::MIN);
  EXPECT_EQ(u32::MIN + 1_u32, u32(u32::MIN_PRIMITIVE + 1));

  auto x = 0_u32;
  x += 0_u32;
  EXPECT_EQ(x, 0_u32);
  x = 12345_u32;
  x += 1_u32;
  EXPECT_EQ(x, 12346_u32);
  x = u32::MAX;
  x += 0_u32;
  EXPECT_EQ(x, u32::MAX);
  x = u32::MIN;
  x += 0_u32;
  EXPECT_EQ(x, u32::MIN);
  x = u32::MIN;
  x += 1_u32;
  EXPECT_EQ(x, u32(u32::MIN_PRIMITIVE + 1));
}

TEST(u32DeathTest, AddOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX + 1_u32, "");
  EXPECT_DEATH(u32::MAX + u32::MAX, "");
#endif
}

TEST(u32, CheckedAdd) {
  constexpr auto a = (1_u32).checked_add(3_u32);
  EXPECT_EQ(a, Option<u32>::some(4_u32));

  EXPECT_EQ((0_u32).checked_add(0_u32).unwrap(), 0_u32);

  EXPECT_EQ(u32::MAX.checked_add(1_u32), None);
  EXPECT_EQ((1_u32).checked_add(u32::MAX), None);
  EXPECT_EQ(u32::MAX.checked_add(u32::MAX), None);
}

TEST(u32, OverflowingAdd) {
  constexpr auto a = (1_u32).overflowing_add(3_u32);
  EXPECT_EQ(a, (Tuple<u32, bool>::with(4_u32, false)));

  EXPECT_EQ((0_u32).overflowing_add(0_u32),
            (Tuple<u32, bool>::with(0_u32, false)));

  EXPECT_EQ(u32::MAX.overflowing_add(1_u32),
            (Tuple<u32, bool>::with(u32::MIN, true)));
  EXPECT_EQ(u32::MAX.overflowing_add(2_u32),
            (Tuple<u32, bool>::with(u32::MIN + 1_u32, true)));
  EXPECT_EQ((2_u32).overflowing_add(u32::MAX),
            (Tuple<u32, bool>::with(u32::MIN + 1_u32, true)));
  EXPECT_EQ(u32::MAX.overflowing_add(u32::MAX),
            (Tuple<u32, bool>::with(u32::MIN + u32::MAX - 1_u32, true)));
}

TEST(u32, SaturatingAdd) {
  constexpr auto a = (1_u32).saturating_add(3_u32);
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((0_u32).saturating_add(0_u32), 0_u32);

  EXPECT_EQ(u32::MAX.saturating_add(1_u32), u32::MAX);
  EXPECT_EQ((1_u32).saturating_add(u32::MAX), u32::MAX);
  EXPECT_EQ(u32::MAX.saturating_add(u32::MAX), u32::MAX);
}

TEST(u32, UncheckedAdd) {
  constexpr auto a = (1_u32).unchecked_add(unsafe_fn, 3_u32);
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((0_u32).unchecked_add(unsafe_fn, 0_u32), 0_u32);
  EXPECT_EQ((12345_u32).unchecked_add(unsafe_fn, 1_u32), 12346_u32);
  EXPECT_EQ(u32::MAX.unchecked_add(unsafe_fn, 0_u32), u32::MAX);
  EXPECT_EQ(u32::MIN.unchecked_add(unsafe_fn, 0_u32), u32::MIN);
  EXPECT_EQ(u32::MIN.unchecked_add(unsafe_fn, 1_u32),
            u32(u32::MIN_PRIMITIVE + 1));
  EXPECT_EQ(u32::MIN.unchecked_add(unsafe_fn, u32::MAX), u32::MIN + u32::MAX);
  EXPECT_EQ(u32::MAX.unchecked_add(unsafe_fn, u32::MIN), u32::MIN + u32::MAX);
}

TEST(u32, WrappingAdd) {
  constexpr auto a = (1_u32).wrapping_add(3_u32);
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((0_u32).wrapping_add(0_u32), 0_u32);

  EXPECT_EQ(u32::MAX.wrapping_add(1_u32), u32::MIN);
  EXPECT_EQ(u32::MAX.wrapping_add(2_u32), u32::MIN + 1_u32);
  EXPECT_EQ((2_u32).wrapping_add(u32::MAX), u32::MIN + 1_u32);
  EXPECT_EQ(u32::MAX.wrapping_add(u32::MAX), u32::MIN + u32::MAX - 1_u32);
}

TEST(u32, Div) {
  constexpr auto a = 4_u32 / 2_u32;
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ(0_u32 / 123_u32, 0_u32);
  EXPECT_EQ(2345_u32 / 1_u32, 2345_u32);
  EXPECT_EQ(2222_u32 / 2_u32, 1111_u32);
  EXPECT_EQ(5_u32 / 2_u32, 2_u32);

  auto x = 0_u32;
  x /= 123_u32;
  EXPECT_EQ(x, 0_u32);
  x = 2345_u32;
  x /= 1_u32;
  EXPECT_EQ(x, 2345_u32);
  x = 2222_u32;
  x /= 2_u32;
  EXPECT_EQ(x, 1111_u32);
  x = 5_u32;
  x /= 2_u32;
  EXPECT_EQ(x, 2_u32);
}

TEST(u32DeathTest, DivOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX / 0_u32, "");
  EXPECT_DEATH(0_u32 / 0_u32, "");
  EXPECT_DEATH(1_u32 / 0_u32, "");
  EXPECT_DEATH(u32::MIN / 0_u32, "");

  auto x = u32::MAX;
  EXPECT_DEATH(x /= 0_u32, "");
  x = 0_u32;
  EXPECT_DEATH(x /= 0_u32, "");
  x = 1_u32;
  EXPECT_DEATH(x /= 0_u32, "");
  x = u32::MIN;
  EXPECT_DEATH(x /= 0_u32, "");
#endif
}

TEST(u32, CheckedDiv) {
  constexpr auto a = (4_u32).checked_div(2_u32);
  EXPECT_EQ(a, Option<u32>::some(2_u32));

  EXPECT_EQ((0_u32).checked_div(123_u32), Option<u32>::some(0_u32));
  EXPECT_EQ((2345_u32).checked_div(1_u32), Option<u32>::some(2345_u32));

  EXPECT_EQ(u32::MAX.checked_div(0_u32), None);
  EXPECT_EQ((0_u32).checked_div(0_u32), None);
  EXPECT_EQ((1_u32).checked_div(0_u32), None);
  EXPECT_EQ(u32::MIN.checked_div(0_u32), None);
}

TEST(u32, OverflowingDiv) {
  constexpr auto a = (4_u32).overflowing_div(2_u32);
  EXPECT_EQ(a, (Tuple<u32, bool>::with(2_u32, false)));

  EXPECT_EQ((0_u32).overflowing_div(123_u32),
            (Tuple<u32, bool>::with(0_u32, false)));
}

TEST(u32DeathTest, OverflowingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX.overflowing_div(0_u32), "");
  EXPECT_DEATH((0_u32).overflowing_div(0_u32), "");
  EXPECT_DEATH((1_u32).overflowing_div(0_u32), "");
  EXPECT_DEATH(u32::MIN.overflowing_div(0_u32), "");
#endif
}

TEST(u32, SaturatingDiv) {
  constexpr auto a = (4_u32).saturating_div(2_u32);
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ((0_u32).saturating_div(123_u32), 0_u32);
  EXPECT_EQ((2345_u32).saturating_div(1_u32), 2345_u32);
}

TEST(u32DeathTest, SaturatingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX.saturating_div(0_u32), "");
  EXPECT_DEATH((0_u32).saturating_div(0_u32), "");
  EXPECT_DEATH((1_u32).saturating_div(0_u32), "");
  EXPECT_DEATH(u32::MIN.saturating_div(0_u32), "");
#endif
}

TEST(u32, WrappingDiv) {
  constexpr auto a = (4_u32).wrapping_div(2_u32);
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ((0_u32).wrapping_div(123_u32), 0_u32);
  EXPECT_EQ((2345_u32).wrapping_div(1_u32), 2345_u32);
}

TEST(u32DeathTest, WrappingDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX.wrapping_div(0_u32), "");
  EXPECT_DEATH((0_u32).wrapping_div(0_u32), "");
  EXPECT_DEATH((1_u32).wrapping_div(0_u32), "");
  EXPECT_DEATH(u32::MIN.wrapping_div(0_u32), "");
#endif
}

TEST(u32, Mul) {
  constexpr auto a = (1_u32) * (3_u32);
  EXPECT_EQ(a, 3_u32);

  EXPECT_EQ(0_u32 * 21_u32, 0_u32);
  EXPECT_EQ(21_u32 * 0_u32, 0_u32);
  EXPECT_EQ(1_u32 * 21_u32, 21_u32);
  EXPECT_EQ(21_u32 * 1_u32, 21_u32);
  EXPECT_EQ(100_u32 * 21_u32, 2100_u32);
  EXPECT_EQ(21_u32 * 100_u32, 2100_u32);
  EXPECT_EQ(1_u32 * u32::MAX, u32::MAX);
  EXPECT_EQ(u32::MIN * 1_u32, u32::MIN);

  auto x = 5_u32;
  x *= 20_u32;
  EXPECT_EQ(x, 100_u32);
}

TEST(u32DeathTest, MulOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX * 2_u32, "");
#endif
}

TEST(u32, CheckedMul) {
  constexpr auto a = (1_u32).checked_mul(3_u32).unwrap();
  EXPECT_EQ(a, 3_u32);

  EXPECT_EQ((100_u32).checked_mul(21_u32), sus::some(2100_u32).construct());
  EXPECT_EQ((21_u32).checked_mul(100_u32), sus::some(2100_u32).construct());
  EXPECT_EQ((123456_u32).checked_mul(234567_u32), sus::None);
}

TEST(u32, OverflowingMul) {
  constexpr auto a = (123456_u32).overflowing_mul(234567_u32);
  EXPECT_EQ(
      a,
      (Tuple<u32, bool>::with(
          u32(static_cast<decltype(u32::primitive_value)>(123456u * 234567u)),
          true)));

  EXPECT_EQ((100_u32).overflowing_mul(21_u32),
            (Tuple<u32, bool>::with(2100_u32, false)));
  EXPECT_EQ((21_u32).overflowing_mul(100_u32),
            (Tuple<u32, bool>::with(2100_u32, false)));
  EXPECT_EQ(
      (123456_u32).overflowing_mul(234567_u32),
      (Tuple<u32, bool>::with(
          u32(static_cast<decltype(u32::primitive_value)>(123456u * 234567u)),
          true)));
  EXPECT_EQ((1'000'000'000_u32).overflowing_mul(10_u32),
            (Tuple<u32, bool>::with(1410065408_u32, true)));
}

TEST(u32, SaturatedMul) {
  constexpr auto a = (1_u32).saturating_mul(3_u32);
  EXPECT_EQ(a, 3_u32);

  EXPECT_EQ((100_u32).saturating_mul(21_u32), 2100_u32);
  EXPECT_EQ((21_u32).saturating_mul(100_u32), 2100_u32);
  EXPECT_EQ((123456_u32).saturating_mul(234567_u32), u32::MAX);
}

TEST(u32, UncheckedMul) {
  constexpr auto a = (1_u32).unchecked_mul(unsafe_fn, 3_u32);
  EXPECT_EQ(a, 3_u32);

  EXPECT_EQ((100_u32).unchecked_mul(unsafe_fn, 21_u32), 2100_u32);
  EXPECT_EQ((21_u32).unchecked_mul(unsafe_fn, 100_u32), 2100_u32);
}

TEST(u32, WrappingMul) {
  [[maybe_unused]] constexpr auto a = (123456_u32).wrapping_mul(23456_u32);

  EXPECT_EQ((100_u32).wrapping_mul(21_u32), 2100_u32);
  EXPECT_EQ((21_u32).wrapping_mul(100_u32), 2100_u32);
  EXPECT_EQ(
      (123456_u32).wrapping_mul(234567_u32),
      u32(static_cast<decltype(u32::primitive_value)>(123456u * 234567u)));
}

TEST(u32, CheckedNeg) {
  constexpr auto a = (0_u32).checked_neg();
  EXPECT_EQ(a, Option<u32>::some(0_u32));

  EXPECT_EQ((0_u32).checked_neg(), Option<u32>::some(0_u32));

  // ** Unsigned only.
  EXPECT_EQ((123_u32).checked_neg(), None);
}

TEST(u32, OverflowingNeg) {
  constexpr auto a = (0_u32).overflowing_neg();
  EXPECT_EQ(a, (Tuple<u32, bool>::with(0_u32, false)));

  EXPECT_EQ((0_u32).overflowing_neg(), (Tuple<u32, bool>::with(0_u32, false)));

  // ** Unsigned only.
  EXPECT_EQ((123_u32).overflowing_neg(),
            (Tuple<u32, bool>::with(
                static_cast<decltype(u32::primitive_value)>(-123), true)));
}

TEST(u32, WrappingNeg) {
  [[maybe_unused]] constexpr auto a = (123456_u32).wrapping_neg();

  EXPECT_EQ((0_u32).wrapping_neg(), 0_u32);

  EXPECT_EQ(u32::MIN.wrapping_neg(), u32::MIN);
  EXPECT_EQ(u32::MAX.wrapping_neg(), u32::MIN + 1_u32);
}

TEST(u32, Rem) {
  constexpr auto a = 5_u32 % 3_u32;
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ(0_u32 % 123_u32, 0_u32);
  EXPECT_EQ(5_u32 % 2_u32, 1_u32);
  EXPECT_EQ(5_u32 % 1_u32, 0_u32);

  auto x = 0_u32;
  x %= 123_u32;
  EXPECT_EQ(x, 0_u32);
  x = 5_u32;
  x %= 2_u32;
  EXPECT_EQ(x, 1_u32);
  x = 5_u32;
  x %= 1_u32;
  EXPECT_EQ(x, 0_u32);
}

TEST(u32DeathTest, RemOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX % 0_u32, "");
  EXPECT_DEATH(0_u32 % 0_u32, "");
  EXPECT_DEATH(1_u32 % 0_u32, "");
  EXPECT_DEATH(u32::MIN % 0_u32, "");

  auto x = u32::MAX;
  EXPECT_DEATH(x %= 0_u32, "");
  x = 0_u32;
  EXPECT_DEATH(x %= 0_u32, "");
  x = 1_u32;
  EXPECT_DEATH(x %= 0_u32, "");
  x = u32::MIN;
  EXPECT_DEATH(x %= 0_u32, "");
#endif
}

TEST(u32, CheckedRem) {
  constexpr auto a = (5_u32).checked_rem(3_u32);
  EXPECT_EQ(a, Option<u32>::some(2_u32));

  EXPECT_EQ((0_u32).checked_rem(123_u32), Option<u32>::some(0_u32));
  EXPECT_EQ((2345_u32).checked_rem(4_u32), Option<u32>::some(1_u32));

  EXPECT_EQ(u32::MAX.checked_rem(0_u32), None);
  EXPECT_EQ((0_u32).checked_rem(0_u32), None);
  EXPECT_EQ((1_u32).checked_rem(0_u32), None);
  EXPECT_EQ(u32::MIN.checked_rem(0_u32), None);
}

TEST(u32, OverflowingRem) {
  constexpr auto a = (5_u32).overflowing_rem(3_u32);
  EXPECT_EQ(a, (Tuple<u32, bool>::with(2_u32, false)));

  EXPECT_EQ((0_u32).overflowing_rem(123_u32),
            (Tuple<u32, bool>::with(0_u32, false)));
  EXPECT_EQ((2345_u32).overflowing_rem(4_u32),
            (Tuple<u32, bool>::with(1_u32, false)));
}

TEST(u32DeathTest, OverflowingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX.overflowing_rem(0_u32), "");
  EXPECT_DEATH((0_u32).overflowing_rem(0_u32), "");
  EXPECT_DEATH((1_u32).overflowing_rem(0_u32), "");
#endif
}

TEST(u32, WrappingRem) {
  constexpr auto a = (5_u32).wrapping_rem(3_u32);
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ((0_u32).wrapping_rem(123_u32), 0_u32);
  EXPECT_EQ((2345_u32).wrapping_rem(4_u32), 1_u32);
}

TEST(u32DeathTest, WrappingRemByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX.wrapping_rem(0_u32), "");
  EXPECT_DEATH((0_u32).wrapping_rem(0_u32), "");
  EXPECT_DEATH((1_u32).wrapping_rem(0_u32), "");
  EXPECT_DEATH(u32::MIN.wrapping_rem(0_u32), "");
#endif
}

TEST(u32, Shl) {
  constexpr auto a = (5_u32) << 1_u32;
  EXPECT_EQ(a, 10_u32);

  EXPECT_EQ(2_u32 << 1_u32, 4_u32);
  EXPECT_EQ(1_u32 << 31_u32, u32(static_cast<decltype(u32::primitive_value)>(
                                 static_cast<uint32_t>(1u) << 31)));

  auto x = 2_u32;
  x <<= 1_u32;
  EXPECT_EQ(x, 4_u32);
}

TEST(u32DeathTest, ShlOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(0_u32 << 32_u32, "");
  EXPECT_DEATH(1_u32 << 33_u32, "");
  EXPECT_DEATH(2_u32 << 64_u32, "");
#endif
}

TEST(u32, OverflowingShl) {
  [[maybe_unused]] constexpr auto a = (5_u32).overflowing_shl(1_u32);

  EXPECT_EQ((2_u32).overflowing_shl(1_u32),
            (Tuple<u32, bool>::with(4_u32, false)));

  // Masks out everything.
  EXPECT_EQ((2_u32).overflowing_shl(32_u32),
            (Tuple<u32, bool>::with(2_u32, true)));
  // Masks out everything but the 1.
  EXPECT_EQ((2_u32).overflowing_shl(33_u32),
            (Tuple<u32, bool>::with(4_u32, true)));
}

TEST(u32, CheckedShl) {
  constexpr auto a = (5_u32).checked_shl(1_u32);
  EXPECT_EQ(a, Option<u32>::some(10_u32));

  EXPECT_EQ((2_u32).checked_shl(1_u32), Option<u32>::some(4_u32));

  EXPECT_EQ((0_u32).checked_shl(32_u32), None);
  EXPECT_EQ((1_u32).checked_shl(33_u32), None);
  EXPECT_EQ((2_u32).checked_shl(64_u32), None);
}

TEST(u32, WrappingShl) {
  constexpr auto a = (5_u32).wrapping_shl(1_u32);
  EXPECT_EQ(a, 10_u32);

  EXPECT_EQ((2_u32).wrapping_shl(1_u32), 4_u32);

  EXPECT_EQ((2_u32).wrapping_shl(32_u32), 2_u32);  // Masks out everything.
  EXPECT_EQ((2_u32).wrapping_shl(33_u32),
            4_u32);  // Masks out everything but the 1.
}

TEST(u32, Shr) {
  constexpr auto a = (5_u32) >> 1_u32;
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ(4_u32 >> 1_u32, 2_u32);

  auto x = 4_u32;
  x >>= 1_u32;
  EXPECT_EQ(x, 2_u32);
}

TEST(u32DeathTest, ShrOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(0_u32 >> 33_u32, "");
  EXPECT_DEATH(1_u32 >> 64_u32, "");
#endif
}

TEST(u32, CheckedShr) {
  constexpr auto a = (5_u32).checked_shr(1_u32);
  EXPECT_EQ(a, Option<u32>::some(2_u32));

  EXPECT_EQ((4_u32).checked_shr(1_u32), Option<u32>::some(2_u32));
  EXPECT_EQ((0_u32).checked_shr(33_u32), None);
  EXPECT_EQ((1_u32).checked_shr(64_u32), None);
}

TEST(u32, OverflowingShr) {
  constexpr auto a = (5_u32).overflowing_shr(1_u32);
  EXPECT_EQ(a, (Tuple<u32, bool>::with(2_u32, false)));

  EXPECT_EQ((4_u32).overflowing_shr(1_u32),
            (Tuple<u32, bool>::with(2_u32, false)));

  // Masks out everything.
  EXPECT_EQ((4_u32).overflowing_shr(32_u32),
            (Tuple<u32, bool>::with(4_u32, true)));
  // Masks out everything but the 1.
  EXPECT_EQ((4_u32).overflowing_shr(33_u32),
            (Tuple<u32, bool>::with(2_u32, true)));
}

TEST(u32, WrappingShr) {
  constexpr auto a = (5_u32).wrapping_shr(1_u32);
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ((4_u32).wrapping_shr(1_u32), 2_u32);

  EXPECT_EQ((4_u32).wrapping_shr(32_u32), 4_u32);  // Masks out everything.
  EXPECT_EQ((4_u32).wrapping_shr(33_u32),
            2_u32);  // Masks out everything but the 1.
}

TEST(u32, Sub) {
  constexpr auto a = 5_u32 - 3_u32;
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ(0_u32 - 0_u32, 0_u32);
  EXPECT_EQ(12345_u32 - 12345_u32, 0_u32);
  EXPECT_EQ(12345_u32 - 1_u32, 12344_u32);
  EXPECT_EQ(u32::MAX - u32::MAX, 0_u32);
  EXPECT_EQ(u32::MIN - u32::MIN, 0_u32);

  auto x = 0_u32;
  x -= 0_u32;
  EXPECT_EQ(x, 0_u32);
  x = 12345_u32;
  x -= 345_u32;
  EXPECT_EQ(x, 12000_u32);
}

TEST(u32DeathTest, SubOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MIN - 1_u32, "");
  EXPECT_DEATH(u32::MIN - u32::MAX, "");
#endif
}

TEST(u32, CheckedSub) {
  constexpr auto a = (5_u32).checked_sub(3_u32);
  EXPECT_EQ(a, Option<u32>::some(2_u32));

  EXPECT_EQ((0_u32).checked_sub(0_u32).unwrap(), 0_u32);
  EXPECT_EQ((12345_u32).checked_sub(12345_u32).unwrap(), 0_u32);

  EXPECT_EQ(u32::MIN.checked_sub(1_u32), None);
  EXPECT_EQ(u32::MIN.checked_sub(2_u32), None);
  EXPECT_EQ(u32::MIN.checked_sub(u32::MAX), None);
}

TEST(u32, OverflowingSub) {
  constexpr auto a = (5_u32).overflowing_sub(3_u32);
  EXPECT_EQ(a, (Tuple<u32, bool>::with(2_u32, false)));

  EXPECT_EQ((0_u32).overflowing_sub(0_u32),
            (Tuple<u32, bool>::with(0_u32, false)));
  EXPECT_EQ((12345_u32).overflowing_sub(12345_u32),
            (Tuple<u32, bool>::with(0_u32, false)));

  EXPECT_EQ(u32::MIN.overflowing_sub(1_u32),
            (Tuple<u32, bool>::with(u32::MAX, true)));
  EXPECT_EQ(u32::MIN.overflowing_sub(2_u32),
            (Tuple<u32, bool>::with(u32::MAX - 1_u32, true)));
  EXPECT_EQ(u32::MIN.overflowing_sub(u32::MAX),
            (Tuple<u32, bool>::with(1_u32, true)));
}

TEST(u32, SaturatingSub) {
  constexpr auto a = (5_u32).saturating_sub(3_u32);
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ((0_u32).saturating_sub(0_u32), 0_u32);
  EXPECT_EQ((12345_u32).saturating_sub(12345_u32), 0_u32);

  EXPECT_EQ(u32::MIN.saturating_sub(1_u32), u32::MIN);
  EXPECT_EQ(u32::MIN.saturating_sub(2_u32), u32::MIN);
  EXPECT_EQ(u32::MIN.saturating_sub(u32::MAX), u32::MIN);
}

TEST(u32, UncheckedSub) {
  constexpr auto a = (5_u32).unchecked_sub(unsafe_fn, 3_u32);
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ((0_u32).unchecked_sub(unsafe_fn, 0_u32), 0_u32);
  EXPECT_EQ((12345_u32).unchecked_sub(unsafe_fn, 12345_u32), 0_u32);
  EXPECT_EQ((12345_u32).unchecked_sub(unsafe_fn, 1_u32), 12344_u32);
  EXPECT_EQ(u32::MAX.unchecked_sub(unsafe_fn, u32::MAX), 0_u32);
  EXPECT_EQ(u32::MIN.unchecked_sub(unsafe_fn, u32::MIN), 0_u32);
  EXPECT_EQ((0_u32).unchecked_sub(unsafe_fn, u32::MIN + 1_u32), u32::MAX);
}

TEST(u32, WrappingSub) {
  constexpr auto a = (5_u32).wrapping_sub(3_u32);
  EXPECT_EQ(a, 2_u32);

  EXPECT_EQ((0_u32).wrapping_sub(0_u32), 0_u32);
  EXPECT_EQ((12345_u32).wrapping_sub(12345_u32), 0_u32);

  EXPECT_EQ(u32::MIN.wrapping_sub(1_u32), u32::MAX);
  EXPECT_EQ(u32::MIN.wrapping_sub(2_u32), u32::MAX - 1_u32);
  EXPECT_EQ(u32::MIN.wrapping_sub(u32::MAX), 1_u32);
}

TEST(u32, CountOnes) {
  constexpr auto a1 = (7_u32).count_ones();
  EXPECT_EQ(a1, 3_u32);
  constexpr auto a2 = (0_u32).count_ones();
  EXPECT_EQ(a2, 0_u32);

  EXPECT_EQ((7_u32).count_ones(), 3_u32);
  EXPECT_EQ((0_u32).count_ones(), 0_u32);
}

TEST(u32, CountZeros) {
  constexpr auto a1 = (7_u32).count_zeros();
  EXPECT_EQ(a1, 32_u32 - 3_u32);
  constexpr auto a2 = (0_u32).count_zeros();
  EXPECT_EQ(a2, 32_u32);

  EXPECT_EQ((7_u32).count_zeros(), 32_u32 - 3_u32);
  EXPECT_EQ((0_u32).count_zeros(), 32_u32);
}

TEST(u32, LeadingZeros) {
  constexpr auto a1 = (0_u32).leading_zeros();
  EXPECT_EQ(a1, 32_u32);
  constexpr auto a2 = (1_u32).leading_zeros();
  EXPECT_EQ(a2, 31_u32);
  constexpr auto a3 = (3_u32).leading_zeros();
  EXPECT_EQ(a3, 30_u32);

  // ** Unsigned only.
  constexpr auto a4 = (u32::MAX).leading_zeros();
  EXPECT_EQ(a4, 0_u32);

  EXPECT_EQ((0_u32).leading_zeros(), 32_u32);
  EXPECT_EQ((1_u32).leading_zeros(), 31_u32);
  EXPECT_EQ((3_u32).leading_zeros(), 30_u32);

  // ** Unsigned only.
  EXPECT_EQ((u32::MAX).leading_zeros(), 0_u32);
}

TEST(u32, LeadingOnes) {
  constexpr auto a1 = (0_u32).leading_ones();
  EXPECT_EQ(a1, 0_u32);
  constexpr auto a2 = (1_u32).leading_ones();
  EXPECT_EQ(a2, 0_u32);

  // ** Unsigned only.
  constexpr auto a3 = (u32::MAX).leading_ones();
  EXPECT_EQ(a3, 32_u32);

  EXPECT_EQ((0_u32).leading_ones(), 0_u32);
  EXPECT_EQ((1_u32).leading_ones(), 0_u32);

  // ** Unsigned only.
  EXPECT_EQ((u32::MAX).leading_ones(), 32_u32);
}

TEST(u32, TrailingZeros) {
  constexpr auto a1 = (0_u32).trailing_zeros();
  EXPECT_EQ(a1, 32_u32);
  constexpr auto a2 = (1_u32).trailing_zeros();
  EXPECT_EQ(a2, 0_u32);
  constexpr auto a3 = (2_u32).trailing_zeros();
  EXPECT_EQ(a3, 1_u32);

  EXPECT_EQ((0_u32).trailing_zeros(), 32_u32);
  EXPECT_EQ((1_u32).trailing_zeros(), 0_u32);
  EXPECT_EQ((2_u32).trailing_zeros(), 1_u32);
}

TEST(u32, TrailingOnes) {
  constexpr auto a1 = (0_u32).trailing_ones();
  EXPECT_EQ(a1, 0_u32);
  constexpr auto a2 = (1_u32).trailing_ones();
  EXPECT_EQ(a2, 1_u32);
  constexpr auto a3 = (3_u32).trailing_ones();
  EXPECT_EQ(a3, 2_u32);

  // ** Unsigned only.
  constexpr auto a4 = (u32::MAX).trailing_ones();
  EXPECT_EQ(a4, 32_u32);

  EXPECT_EQ((0_u32).trailing_ones(), 0_u32);
  EXPECT_EQ((1_u32).trailing_ones(), 1_u32);
  EXPECT_EQ((3_u32).trailing_ones(), 2_u32);

  // ** Unsigned only.
  EXPECT_EQ((u32::MAX).trailing_ones(), 32_u32);
}

TEST(u32, Pow) {
  [[maybe_unused]] constexpr auto a = (2_u32).pow(5_u32);

  EXPECT_EQ((2_u32).pow(5_u32), 32_u32);
  EXPECT_EQ((2_u32).pow(0_u32), 1_u32);
  EXPECT_EQ((2_u32).pow(1_u32), 2_u32);
  EXPECT_EQ((2_u32).pow(30_u32), 1_u32 << 30_u32);
  EXPECT_EQ((1_u32).pow(u32::MAX), 1_u32);
  EXPECT_EQ((u32::MAX).pow(1_u32), u32::MAX);
  EXPECT_EQ((u32::MAX).pow(0_u32), 1_u32);
}

TEST(u32DeathTest, PowOverflow) {
#if GTEST_HAS_DEATH_TEST
  // Crashes on the final acc * base.
  EXPECT_DEATH((3_u32).pow(31_u32), "");
  // Crashes on base * base.
  EXPECT_DEATH((u32::MAX / 2_u32).pow(31_u32), "");
  // Crashes on acc * base inside the exponent loop.
  EXPECT_DEATH((4_u32).pow((1_u32 << 30_u32) - 1_u32), "");
#endif
}

TEST(u32, OverflowingPow) {
  [[maybe_unused]] constexpr auto a = (2_u32).overflowing_pow(5_u32);

  EXPECT_EQ((2_u32).overflowing_pow(5_u32),
            (Tuple<u32, bool>::with(32_u32, false)));
  EXPECT_EQ((2_u32).overflowing_pow(0_u32),
            (Tuple<u32, bool>::with(1_u32, false)));
  EXPECT_EQ((u32::MAX).overflowing_pow(1_u32),
            (Tuple<u32, bool>::with(u32::MAX, false)));
  EXPECT_EQ((u32::MAX).overflowing_pow(2_u32),
            (Tuple<u32, bool>::with(1_u32, true)));
}

TEST(u32, CheckedPow) {
  constexpr auto a = (2_u32).checked_pow(5_u32);
  EXPECT_EQ(a, Option<u32>::some(32_u32));

  EXPECT_EQ((2_u32).checked_pow(5_u32), Option<u32>::some(32_u32));
  EXPECT_EQ((2_u32).checked_pow(0_u32), Option<u32>::some(1_u32));
  EXPECT_EQ((2_u32).checked_pow(1_u32), Option<u32>::some(2_u32));
  EXPECT_EQ((2_u32).checked_pow(30_u32), Option<u32>::some(1_u32 << 30_u32));
  EXPECT_EQ((1_u32).checked_pow(u32::MAX), Option<u32>::some(1_u32));
  EXPECT_EQ((u32::MAX).checked_pow(1_u32), Option<u32>::some(u32::MAX));
  EXPECT_EQ((u32::MAX).checked_pow(0_u32), Option<u32>::some(1_u32));

  // Fails on the final acc * base.
  EXPECT_EQ((3_u32).checked_pow(31_u32), None);
  // Fails on base * base.
  EXPECT_EQ((u32::MAX / 2_u32).checked_pow(31_u32), None);
  // Fails on acc * base inside the exponent loop.
  EXPECT_EQ((4_u32).checked_pow((1_u32 << 30_u32) - 1_u32), None);
}

TEST(u32, WrappingPow) {
  constexpr auto a = (2_u32).wrapping_pow(5_u32);
  EXPECT_EQ(a, 32_u32);

  EXPECT_EQ((2_u32).wrapping_pow(5_u32), 32_u32);
  EXPECT_EQ((2_u32).wrapping_pow(0_u32), 1_u32);
  EXPECT_EQ((u32::MAX).wrapping_pow(1_u32), u32::MAX);
  EXPECT_EQ((u32::MAX).wrapping_pow(2_u32), 1_u32);
}

TEST(u32, ReverseBits) {
  constexpr auto a1 = (0_u32).reverse_bits();
  EXPECT_EQ(a1, 0_u32);
  constexpr auto a2 = (2_u32).reverse_bits();
  EXPECT_EQ(a2, 1_u32 << 30_u32);
  constexpr auto a3 = (0xf8f800_u32).reverse_bits();
  EXPECT_EQ(a3, 0x1f1f00_u32);
  constexpr auto a5 = (1_u32).reverse_bits();
  EXPECT_EQ(a5, 1_u32 << (sus::into(sizeof(u32)) * 8_u32 - 1_u32));

  EXPECT_EQ((0_u32).reverse_bits(), 0_u32);
  EXPECT_EQ((2_u32).reverse_bits(), 1_u32 << 30_u32);
  EXPECT_EQ((0xf8f800_u32).reverse_bits(), 0x1f1f00_u32);
  EXPECT_EQ((1_u32).reverse_bits().primitive_value,
            1_u32 << (sus::into(sizeof(u32)) * 8_u32 - 1_u32));
}

TEST(u32, RotateLeft) {
  constexpr auto a = (3_u32).rotate_left(2_u32);
  EXPECT_EQ(a, 12_u32);

  EXPECT_EQ((1_u32).rotate_left(1_u32), 2_u32);
  EXPECT_EQ((1_u32).rotate_left(4_u32), 16_u32);
  EXPECT_EQ((1_u32).rotate_left(31_u32), 1_u32 << 31_u32);
  EXPECT_EQ((1_u32).rotate_left(32_u32), 1_u32);
  EXPECT_EQ((1_u32).rotate_left(63_u32), 1_u32 << 31_u32);
  EXPECT_EQ((1_u32).rotate_left(64_u32), 1_u32);
}

TEST(u32, RotateRight) {
  constexpr auto a = (2_u32).rotate_right(1_u32);
  EXPECT_EQ(a, 1_u32);

  EXPECT_EQ((2_u32).rotate_right(1_u32), 1_u32);
  EXPECT_EQ((16_u32).rotate_right(4_u32), 1_u32);
  EXPECT_EQ((1_u32).rotate_right(1_u32), 1_u32 << 31_u32);
  EXPECT_EQ((1_u32).rotate_right(32_u32), 1_u32);
  EXPECT_EQ((1_u32).rotate_right(33_u32), 1_u32 << 31_u32);
  EXPECT_EQ((1_u32).rotate_right(64_u32), 1_u32);
  EXPECT_EQ((1_u32).rotate_right(65_u32), 1_u32 << 31_u32);
}

TEST(u32, SwapBytes) {
  constexpr auto a = (0x12345678_u32).swap_bytes();
  EXPECT_EQ(a, 0x78563412_u32);

  EXPECT_EQ((0x12345678_u32).swap_bytes(), 0x78563412_u32);
  EXPECT_EQ((0_u32).swap_bytes(), 0_u32);
  EXPECT_EQ((1_u32 << 31_u32).swap_bytes(), (0x80_u32));
}

TEST(u32, Log2) {
  constexpr auto a = (2_u32).log2();
  EXPECT_EQ(a, 1_u32);

  EXPECT_EQ((2_u32).log2(), 1_u32);
  EXPECT_EQ((55555_u32).log2(), 15_u32);

  // ** Unsigned only.
  EXPECT_EQ((u32::MAX / 2_u32).log2(), 30_u32);
}

TEST(u32DeathTest, Log2NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((0_u32).log2(), "");
#endif
}

TEST(u32, CheckedLog2) {
  constexpr auto a = (2_u32).checked_log2();
  EXPECT_EQ(a, Option<u32>::some(1_u32));

  EXPECT_EQ((2_u32).checked_log2(), Option<u32>::some(1_u32));
  EXPECT_EQ((55555_u32).checked_log2(), Option<u32>::some(15_u32));
  EXPECT_EQ((0_u32).checked_log2(), None);

  // ** Unsigned only.
  EXPECT_EQ((u32::MAX / 2_u32).checked_log2(), Option<u32>::some(30_u32));
}

TEST(u32, Log10) {
  constexpr auto a = (55555_u32).log10();
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((2_u32).log10(), 0_u32);
  EXPECT_EQ((55555_u32).log10(), 4_u32);
  EXPECT_EQ((u32::MAX).log10(), 9_u32);
}

TEST(u32DeathTest, Log10NonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((0_u32).log10(), "");
#endif
}

TEST(u32, CheckedLog10) {
  constexpr auto a = (55555_u32).checked_log10();
  EXPECT_EQ(a, Option<u32>::some(4_u32));

  EXPECT_EQ((2_u32).checked_log10(), Option<u32>::some(0_u32));
  EXPECT_EQ((55555_u32).checked_log10(), Option<u32>::some(4_u32));
  EXPECT_EQ((u32::MAX).checked_log10(), Option<u32>::some(9_u32));
  EXPECT_EQ((0_u32).checked_log10(), None);
}

TEST(u32, Log) {
  constexpr auto a = (55555_u32).log(10_u32);
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((2_u32).log(10_u32), 0_u32);
  EXPECT_EQ((55555_u32).log(10_u32), 4_u32);
  EXPECT_EQ((u32::MAX).log(10_u32), 9_u32);
}

TEST(u32DeathTest, LogNonPositive) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((0_u32).log(10_u32), "");
  EXPECT_DEATH((2_u32).log(0_u32), "");
  EXPECT_DEATH((2_u32).log(1_u32), "");
#endif
}

TEST(u32, CheckedLog) {
  constexpr auto a = (55555_u32).checked_log(10_u32);
  EXPECT_EQ(a, Option<u32>::some(4u));

  EXPECT_EQ((2_u32).checked_log(10_u32), Option<u32>::some(0_u32));
  EXPECT_EQ((55555_u32).checked_log(10_u32), Option<u32>::some(4_u32));
  EXPECT_EQ(u32::MAX.checked_log(10_u32), Option<u32>::some(9_u32));
  EXPECT_EQ((0_u32).checked_log(10_u32), None);
}

TEST(u32, ToBe) {
  if constexpr (sus::assertions::is_little_endian()) {
    constexpr auto a = (0x12345678_u32).to_be();
    EXPECT_EQ(a, 0x78563412_u32);

    EXPECT_EQ((0x12345678_u32).to_be(), 0x78563412_u32);
    EXPECT_EQ((0_u32).to_be(), 0_u32);
    EXPECT_EQ((1_u32 << 31_u32).to_be(), (1_u32 << 7_u32));
  } else {
    constexpr auto a = (0x12345678_u32).to_be();
    EXPECT_EQ(a, 0x12345678_u32);

    EXPECT_EQ((0x12345678_u32).to_be(), 0x12345678_u32);
    EXPECT_EQ((0_u32).to_be(), 0_u32);
    EXPECT_EQ((1_u32 << 31_u32).to_be(), (1_u32 << 31_u32));
  }
}

TEST(u32, FromBe) {
  if constexpr (sus::assertions::is_little_endian()) {
    constexpr auto a = u32::from_be(0x12345678_u32);
    EXPECT_EQ(a, 0x78563412_u32);

    EXPECT_EQ(u32::from_be(0x12345678_u32), 0x78563412_u32);
    EXPECT_EQ(u32::from_be(0_u32), 0_u32);
    EXPECT_EQ(u32::from_be(1_u32 << 31_u32), 1_u32 << 7_u32);
  } else {
    constexpr auto a = u32::from_be(0x12345678_u32);
    EXPECT_EQ(a, 0x12345678_u32);

    EXPECT_EQ(u32::from_be(0x12345678_u32), 0x12345678_u32);
    EXPECT_EQ(u32::from_be(0_u32), 0_u32);
    EXPECT_EQ(u32::from_be(1_u32 << 31_u32), 1_u32 << 31_u32);
  }
}

TEST(u32, ToLe) {
  if constexpr (sus::assertions::is_big_endian()) {
    constexpr auto a = (0x12345678_u32).to_le();
    EXPECT_EQ(a, 0x78563412_u32);

    EXPECT_EQ((0x12345678_u32).to_le(), 0x78563412_u32);
    EXPECT_EQ((0_u32).to_le(), 0_u32);
    EXPECT_EQ(u32::MIN.to_le(), 0x80_u32);
  } else {
    constexpr auto a = (0x12345678_u32).to_le();
    EXPECT_EQ(a, 0x12345678_u32);

    EXPECT_EQ((0x12345678_u32).to_le(), 0x12345678_u32);
    EXPECT_EQ((0_u32).to_le(), 0_u32);
    EXPECT_EQ(u32::MIN.to_le(), u32::MIN);
  }
}

TEST(u32, FromLe) {
  if constexpr (sus::assertions::is_big_endian()) {
    constexpr auto a = u32::from_le(0x12345678_u32);
    EXPECT_EQ(a, 0x78563412_u32);

    EXPECT_EQ(u32::from_le(0x12345678_u32), 0x78563412_u32);
    EXPECT_EQ(u32::from_le(0_u32), 0_u32);
    EXPECT_EQ(u32::from_le(u32::MIN), 0x80_u32);
  } else {
    constexpr auto a = u32::from_le(0x12345678_u32);
    EXPECT_EQ(a, 0x12345678_u32);

    EXPECT_EQ(u32::from_le(0x12345678_u32), 0x12345678_u32);
    EXPECT_EQ(u32::from_le(0_u32), 0_u32);
    EXPECT_EQ(u32::from_le(u32::MIN), u32::MIN);
  }
}

TEST(u32, ToBeBytes) {
  {
    constexpr auto a = (0x12345678_u32).to_be_bytes();
    EXPECT_EQ(a, (sus::Array<u8, 4>::with_values(0x12_u8, 0x34_u8, 0x56_u8,
                                                 0x78_u8)));
  }
  {
    auto a = (0x12345678_u32).to_be_bytes();
    EXPECT_EQ(a, (sus::Array<u8, 4>::with_values(0x12_u8, 0x34_u8, 0x56_u8,
                                                 0x78_u8)));
  }
}

TEST(u32, ToLeBytes) {
  {
    constexpr auto a = (0x12345678_u32).to_le_bytes();
    EXPECT_EQ(a, (sus::Array<u8, 4>::with_values(0x78_u8, 0x56_u8, 0x34_u8,
                                                 0x12_u8)));
  }
  {
    auto a = (0x12345678_u32).to_le_bytes();
    EXPECT_EQ(a, (sus::Array<u8, 4>::with_values(0x78_u8, 0x56_u8, 0x34_u8,
                                                 0x12_u8)));
  }
}

TEST(u32, ToNeBytes) {
  if constexpr (sus::assertions::is_big_endian()) {
    {
      constexpr auto a = (0x12345678_u32).to_ne_bytes();
      EXPECT_EQ(a, (sus::Array<u8, 4>::with_values(0x12_u8, 0x34_u8, 0x56_u8,
                                                   0x78_u8)));
    }
    {
      auto a = (0x12345678_u32).to_ne_bytes();
      EXPECT_EQ(a, (sus::Array<u8, 4>::with_values(0x12_u8, 0x34_u8, 0x56_u8,
                                                   0x78_u8)));
    }
  } else {
    {
      constexpr auto a = (0x12345678_u32).to_ne_bytes();
      EXPECT_EQ(a, (sus::Array<u8, 4>::with_values(0x78_u8, 0x56_u8, 0x34_u8,
                                                   0x12_u8)));
    }
    {
      auto a = (0x12345678_u32).to_ne_bytes();
      EXPECT_EQ(a, (sus::Array<u8, 4>::with_values(0x78_u8, 0x56_u8, 0x34_u8,
                                                   0x12_u8)));
    }
  }
}

// ** Unsigned only.
TEST(u32, CheckedAddSigned) {
  constexpr auto a = (1_u32).checked_add_signed(3_i32);
  EXPECT_EQ(a, Option<u32>::some(4_u32));

  EXPECT_EQ((1_u32).checked_add_signed(2_i32), Option<u32>::some(3_u32));
  EXPECT_EQ((u32::MIN + 1_u32).checked_add_signed(-1_i32),
            Option<u32>::some(u32::MIN));
  EXPECT_EQ((u32::MIN).checked_add_signed(-1_i32), None);
  EXPECT_EQ((u32::MAX - 2_u32).checked_add_signed(3_i32), None);
}

// ** Unsigned only.
TEST(u32, OverflowingAddSigned) {
  constexpr auto a = (1_u32).overflowing_add_signed(3_i32);
  EXPECT_EQ(a, (Tuple<u32, bool>::with(4_u32, false)));

  EXPECT_EQ((1_u32).overflowing_add_signed(2_i32),
            (Tuple<u32, bool>::with(3_u32, false)));
  EXPECT_EQ((u32::MIN + 1_u32).overflowing_add_signed(-1_i32),
            (Tuple<u32, bool>::with(u32::MIN, false)));
  EXPECT_EQ((u32::MIN).overflowing_add_signed(-1_i32),
            (Tuple<u32, bool>::with(u32::MAX, true)));
  EXPECT_EQ((u32::MAX - 2_u32).overflowing_add_signed(3_i32),
            (Tuple<u32, bool>::with(u32::MIN, true)));
}

// ** Unsigned only.
TEST(u32, SaturatingAddSigned) {
  constexpr auto a = (1_u32).saturating_add_signed(3_i32);
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((1_u32).saturating_add_signed(2_i32), 3_u32);
  EXPECT_EQ((u32::MIN + 1_u32).saturating_add_signed(-1_i32), u32::MIN);
  EXPECT_EQ((u32::MIN).saturating_add_signed(-1_i32), u32::MIN);
  EXPECT_EQ((u32::MAX - 2_u32).saturating_add_signed(3_i32), u32::MAX);
}

// ** Unsigned only.
TEST(u32, WrappingAddSigned) {
  constexpr auto a = (1_u32).wrapping_add_signed(3_i32);
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((1_u32).wrapping_add_signed(2_i32), 3_u32);
  EXPECT_EQ((u32::MIN + 1_u32).wrapping_add_signed(-1_i32), u32::MIN);
  EXPECT_EQ((u32::MIN).wrapping_add_signed(-1_i32), u32::MAX);
  EXPECT_EQ((u32::MAX - 2_u32).wrapping_add_signed(3_i32), u32::MIN);
}

TEST(u32, NextPowerOfTwo) {
  constexpr auto a = (3_u32).next_power_of_two();
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((2_u32).next_power_of_two(), 2_u32);
  EXPECT_EQ((3_u32).next_power_of_two(), 4_u32);
  EXPECT_EQ((4_u32).next_power_of_two(), 4_u32);
  EXPECT_EQ((1000_u32).next_power_of_two(), 1024_u32);
}

TEST(u32DeathTest, NextPowerOfTwoOutOfBounds) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(u32::MAX.next_power_of_two(), "");
#endif
}

TEST(u32, CheckedNextPowerOfTwo) {
  constexpr auto a = (3_u32).checked_next_power_of_two();
  EXPECT_EQ(a, Option<u32>::some(4_u32));

  EXPECT_EQ((2_u32).checked_next_power_of_two(), Option<u32>::some(2_u32));
  EXPECT_EQ((3_u32).checked_next_power_of_two(), Option<u32>::some(4_u32));
  EXPECT_EQ((4_u32).checked_next_power_of_two(), Option<u32>::some(4_u32));
  EXPECT_EQ((1000_u32).checked_next_power_of_two(),
            Option<u32>::some(1024_u32));

  EXPECT_EQ(u32::MAX.checked_next_power_of_two(), None);
}

TEST(u32, WrappingNextPowerOfTwo) {
  constexpr auto a = (3_u32).wrapping_next_power_of_two();
  EXPECT_EQ(a, 4_u32);

  EXPECT_EQ((2_u32).wrapping_next_power_of_two(), 2_u32);
  EXPECT_EQ((3_u32).wrapping_next_power_of_two(), 4_u32);
  EXPECT_EQ((4_u32).wrapping_next_power_of_two(), 4_u32);
  EXPECT_EQ((1000_u32).wrapping_next_power_of_two(), 1024_u32);

  EXPECT_EQ(u32::MAX.wrapping_next_power_of_two(), 0_u32);
}

TEST(u32, DivEuclid) {
  constexpr auto a = (7_u32).div_euclid(4_u32);
  EXPECT_EQ(a, 1_u32);

  EXPECT_EQ((7_u32).div_euclid(4_u32), 1_u32);  // 7 >= 4 * 1
}

TEST(u32DeathTest, DivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_u32).div_euclid(0_u32), "");
#endif
}

TEST(u32, CheckedDivEuclid) {
  constexpr auto a = (7_u32).checked_div_euclid(4_u32);
  EXPECT_EQ(a, Option<u32>::some(1_u32));

  EXPECT_EQ((7_u32).checked_div_euclid(4_u32), Option<u32>::some(1_u32));
  EXPECT_EQ((7_u32).checked_div_euclid(0_u32), None);
}

TEST(u32, OverflowingDivEuclid) {
  constexpr auto a = (7_u32).overflowing_div_euclid(4_u32);
  EXPECT_EQ(a, (Tuple<u32, bool>::with(1_u32, false)));

  EXPECT_EQ((7_u32).overflowing_div_euclid(4_u32),
            (Tuple<u32, bool>::with(1_u32, false)));
}

TEST(u32DeathTest, OverflowingDivEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_u32).overflowing_div_euclid(0_u32), "");
#endif
}

TEST(u32, WrappingDivEuclid) {
  constexpr auto a = (7_u32).wrapping_div_euclid(4_u32);
  EXPECT_EQ(a, 1_u32);

  EXPECT_EQ((7_u32).wrapping_div_euclid(4_u32), 1_u32);
}

TEST(u32DeathTest, WrappingDivEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_u32).wrapping_div_euclid(0_u32), "");
#endif
}

TEST(u32, RemEuclid) {
  constexpr auto a = (7_u32).rem_euclid(4_u32);
  EXPECT_EQ(a, 3_u32);

  EXPECT_EQ((7_u32).rem_euclid(4_u32), 3_u32);
}

TEST(u32DeathTest, RemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_u32).rem_euclid(0_u32), "");
#endif
}

TEST(u32, CheckedRemEuclid) {
  constexpr auto a = (7_u32).checked_rem_euclid(4_u32);
  EXPECT_EQ(a, Option<u32>::some(3_u32));

  EXPECT_EQ((7_u32).checked_rem_euclid(4_u32), Option<u32>::some(3_u32));
  EXPECT_EQ((7_u32).checked_rem_euclid(0_u32), None);
}

TEST(u32, OverflowingRemEuclid) {
  constexpr auto a = (7_u32).overflowing_rem_euclid(4_u32);
  EXPECT_EQ(a, (Tuple<u32, bool>::with(3_u32, false)));

  EXPECT_EQ((7_u32).overflowing_rem_euclid(4_u32),
            (Tuple<u32, bool>::with(3_u32, false)));
}

TEST(u32DeathTest, OverflowingRemEuclidDivByZero) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_u32).overflowing_rem_euclid(0_u32), "");
#endif
}

TEST(u32, WrappingRemEuclid) {
  constexpr auto a = (7_u32).wrapping_rem_euclid(4_u32);
  EXPECT_EQ(a, 3_u32);

  EXPECT_EQ((7_u32).wrapping_rem_euclid(4_u32), 3_u32);
}

TEST(u32DeathTest, WrappingRemEuclidOverflow) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH((7_u32).wrapping_rem_euclid(0_u32), "");
#endif
}

}  // namespace
