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
#include "subspace/construct/into.h"
#include "subspace/containers/array.h"
#include "subspace/iter/__private/step.h"
#include "subspace/num/num_concepts.h"
#include "subspace/num/signed_integer.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"
#include "subspace/prelude.h"
#include "subspace/test/ensure_use.h"
#include "subspace/tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(!std::is_signed_v<decltype(u8::primitive_value)>);
static_assert(sizeof(decltype(u8::primitive_value)) == 1);
static_assert(sizeof(u8) == sizeof(decltype(u8::primitive_value)));

static_assert(sus::mem::Copy<u8>);
static_assert(sus::mem::TrivialCopy<u8>);
static_assert(sus::mem::Clone<u8>);
static_assert(sus::mem::relocate_by_memcpy<u8>);
static_assert(sus::mem::Move<u8>);

namespace behaviour {
using T = u8;
using From = decltype(u8::primitive_value);
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

// u8::MAX
static_assert(u8::MAX.primitive_value == 0xff);
template <class T>
concept MaxInRange = requires {
  { 0xff_u8 } -> std::same_as<T>;
  { u8(uint8_t{0xff}) } -> std::same_as<T>;
};
static_assert(MaxInRange<u8>);

// std hashing
static_assert(std::same_as<decltype(std::hash<u8>()(0_u8)), size_t>);
static_assert(std::same_as<decltype(std::equal_to<u8>()(0_u8, 1_u8)), bool>);

TEST(u8, Traits) {
  static_assert(sus::iter::__private::Step<u8>);

  // ** Unsigned only
  static_assert(!sus::num::Neg<u8>);

  static_assert(sus::num::Add<u8, u8>);
  static_assert(sus::num::AddAssign<u8, u8>);
  static_assert(sus::num::Sub<u8, u8>);
  static_assert(sus::num::SubAssign<u8, u8>);
  static_assert(sus::num::Mul<u8, u8>);
  static_assert(sus::num::MulAssign<u8, u8>);
  static_assert(sus::num::Div<u8, u8>);
  static_assert(sus::num::DivAssign<u8, u8>);
  static_assert(sus::num::Rem<u8, u8>);
  static_assert(sus::num::RemAssign<u8, u8>);
  static_assert(sus::num::BitAnd<u8, u8>);
  static_assert(sus::num::BitAndAssign<u8, u8>);
  static_assert(sus::num::BitOr<u8, u8>);
  static_assert(sus::num::BitOrAssign<u8, u8>);
  static_assert(sus::num::BitXor<u8, u8>);
  static_assert(sus::num::BitXorAssign<u8, u8>);
  static_assert(sus::num::BitNot<u8>);
  static_assert(sus::num::Shl<u8>);
  static_assert(sus::num::ShlAssign<u8>);
  static_assert(sus::num::Shr<u8>);
  static_assert(sus::num::ShrAssign<u8>);

  static_assert(sus::ops::Ord<u8, uint8_t>);
  static_assert(sus::ops::Ord<u8, uint16_t>);
  static_assert(sus::ops::Ord<u8, uint32_t>);
  static_assert(sus::ops::Ord<u8, uint64_t>);
  static_assert(sus::ops::Ord<u8, size_t>);
  static_assert(sus::ops::Ord<u8, u8>);
  static_assert(sus::ops::Ord<u8, u16>);
  static_assert(sus::ops::Ord<u8, u32>);
  static_assert(sus::ops::Ord<u8, u64>);
  static_assert(sus::ops::Ord<u8, usize>);
  static_assert(1_u8 >= 1_u8);
  static_assert(2_u8 > 1_u8);
  static_assert(1_u8 <= 1_u8);
  static_assert(1_u8 < 2_u8);
  static_assert(sus::ops::Eq<u8, uint8_t>);
  static_assert(sus::ops::Eq<u8, uint16_t>);
  static_assert(sus::ops::Eq<u8, uint32_t>);
  static_assert(sus::ops::Eq<u8, uint64_t>);
  static_assert(sus::ops::Eq<u8, size_t>);
  static_assert(sus::ops::Eq<u8, u8>);
  static_assert(sus::ops::Eq<u8, u16>);
  static_assert(sus::ops::Eq<u8, u32>);
  static_assert(sus::ops::Eq<u8, u64>);
  static_assert(sus::ops::Eq<u8, usize>);
  static_assert(1_u8 == 1_u8);
  static_assert(!(1_u8 == 2_u8));
  static_assert(1_u8 != 2_u8);
  static_assert(!(1_u8 != 1_u8));

  // Verify constexpr.
  [[maybe_unused]] constexpr u8 c =
      1_u8 + 2_u8 - 3_u8 * 4_u8 / 5_u8 % 6_u8 & 7_u8 | 8_u8 ^ 9_u8;
  [[maybe_unused]] constexpr std::strong_ordering o = 2_u8 <=> 3_u8;
}

TEST(u8, Literals) {
  // Hex.
  static_assert((0x1C_u8).primitive_value == 0x1C);
  static_assert((0X1C_u8).primitive_value == 0X1C);
  static_assert((0X001C_u8).primitive_value == 0X1C);
  EXPECT_EQ((0x1C_u8).primitive_value, 0x1C);
  EXPECT_EQ((0X1C_u8).primitive_value, 0X1C);
  EXPECT_EQ((0X001C_u8).primitive_value, 0X1C);
  // Binary.
  static_assert((0b101_u8).primitive_value == 0b101);
  static_assert((0B101_u8).primitive_value == 0B101);
  static_assert((0b00101_u8).primitive_value == 0b101);
  EXPECT_EQ((0b101_u8).primitive_value, 0b101);
  EXPECT_EQ((0B101_u8).primitive_value, 0B101);
  EXPECT_EQ((0b00101_u8).primitive_value, 0b101);
  // Octal.
  static_assert((0123_u8).primitive_value == 0123);
  static_assert((000123_u8).primitive_value == 0123);
  EXPECT_EQ((0123_u8).primitive_value, 0123);
  EXPECT_EQ((000123_u8).primitive_value, 0123);
  // Decimal.
  static_assert((0_u8).primitive_value == 0);
  static_assert((1_u8).primitive_value == 1);
  static_assert((12_u8).primitive_value == 12);
  static_assert((123_u8).primitive_value == 123);
}

TEST(u8, Constants) {
  constexpr auto max = u8::MAX;
  static_assert(std::same_as<decltype(max), const u8>);
  EXPECT_EQ(max.primitive_value, 0xffu);
  constexpr auto min = u8::MIN;
  static_assert(std::same_as<decltype(min), const u8>);
  EXPECT_EQ(min.primitive_value, 0u);
  constexpr auto bits = u8::BITS;
  static_assert(std::same_as<decltype(bits), const u32>);
  EXPECT_EQ(bits, 8u);
}

template <class From, class To>
concept IsImplicitlyConvertible =
    (std::is_convertible_v<From, To> && std::is_assignable_v<To, From>);
template <class From, class To>
concept IsExplicitlyConvertible =
    (std::constructible_from<To, From> && !std::is_convertible_v<From, To> &&
     !std::is_assignable_v<To, From>);
template <class From, class To>
concept NotConvertible =
    (!std::constructible_from<To, From> && !std::is_convertible_v<From, To> &&
     !std::is_assignable_v<To, From>);

template <class T>
auto make_enum() {
  enum E : T {
    X,
    Y,
    Z,
    MIN = std::numeric_limits<T>::min(),
    MAX = std::numeric_limits<T>::max()
  };
  return E::Z;
}

template <class T>
auto make_enumclass() {
  enum class E : T {
    X,
    Y,
    Z,
    MIN = std::numeric_limits<T>::min(),
    MAX = std::numeric_limits<T>::max()
  };
  return E::Z;
}

#define ENUM(kind, T) decltype([]() { return make_enum##kind<T>(); }())

template <auto From, class To, int = [](To) constexpr { return 0; }(From)>
constexpr bool is_constexpr_convertible(int) {
  return true;
}
template <auto From, class To>
constexpr bool is_constexpr_convertible(...) {
  return false;
}

template <auto From, class To, int = [](To) constexpr { return 0; }(To(From))>
constexpr bool is_constexpr_constructible(int) {
  return true;
}
template <auto From, class To>
constexpr bool is_constexpr_constructible(...) {
  return false;
}

template <auto From, class To>
concept IsConstexprAssignable = requires(To to) {
  { to = From };
};

TEST(u8, CompileTimeConversion) {
  using Self = u8;

  static_assert(is_constexpr_convertible<0_u8, Self>(0));
  static_assert(!is_constexpr_convertible<0_u16, Self>(0));
  static_assert(!is_constexpr_convertible<0_u32, Self>(0));
  static_assert(!is_constexpr_convertible<0_u64, Self>(0));
  static_assert(!is_constexpr_convertible<0_usize, Self>(0));
  static_assert(is_constexpr_convertible<uint8_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<uint16_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<uint32_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<uint64_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<size_t{0}, Self>(0));

  static_assert(IsConstexprAssignable<0_u8, Self>);
  static_assert(!IsConstexprAssignable<0_u16, Self>);
  static_assert(!IsConstexprAssignable<0_u32, Self>);
  static_assert(!IsConstexprAssignable<0_u64, Self>);
  static_assert(!IsConstexprAssignable<0_usize, Self>);
  static_assert(IsConstexprAssignable<uint8_t{0}, Self>);
  static_assert(!IsConstexprAssignable<uint16_t{0}, Self>);
  static_assert(!IsConstexprAssignable<uint32_t{0}, Self>);
  static_assert(!IsConstexprAssignable<uint64_t{0}, Self>);
  static_assert(!IsConstexprAssignable<size_t{0}, Self>);

  static_assert(!IsExplicitlyConvertible<int8_t, Self>);
  static_assert(!IsExplicitlyConvertible<int16_t, Self>);
  static_assert(!IsExplicitlyConvertible<int32_t, Self>);
  static_assert(!IsExplicitlyConvertible<int64_t, Self>);
  static_assert(!IsExplicitlyConvertible<i8, Self>);
  static_assert(!IsExplicitlyConvertible<i16, Self>);
  static_assert(!IsExplicitlyConvertible<i32, Self>);
  static_assert(!IsExplicitlyConvertible<i64, Self>);
  static_assert(!IsExplicitlyConvertible<isize, Self>);
}

TEST(u8, CompileTimeConversionEnum) {
  using Self = u8;

  static_assert(is_constexpr_convertible<ENUM(, uint8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, uint16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, uint32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, uint64_t)::X, Self>(0));

  // Conversion from enum class is explicit (constructible) instead of implicit
  // (convertible).
  static_assert(!is_constexpr_convertible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint64_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, uint64_t)::X, Self>(0));
}

TEST(u8, ToPrimitive) {
  static_assert(NotConvertible<u8, int8_t>);
  static_assert(NotConvertible<u8, int16_t>);
  static_assert(NotConvertible<u8, int32_t>);
  static_assert(NotConvertible<u8, int64_t>);
  static_assert(IsExplicitlyConvertible<u8, uint8_t>);
  static_assert(IsExplicitlyConvertible<u8, uint16_t>);
  static_assert(IsExplicitlyConvertible<u8, uint32_t>);
  static_assert(IsExplicitlyConvertible<u8, uint64_t>);
  static_assert(IsExplicitlyConvertible<u8, size_t>);
  static_assert(sizeof(u8) < sizeof(size_t));
}

TEST(u8, From) {
  static_assert(sus::construct::From<u8, bool>);
  static_assert(sus::construct::From<u8, unsigned char>);
  static_assert(!sus::construct::From<u8, size_t>);
  static_assert(!sus::construct::From<u8, int8_t>);
  static_assert(!sus::construct::From<u8, int16_t>);
  static_assert(!sus::construct::From<u8, int32_t>);
  static_assert(!sus::construct::From<u8, int64_t>);
  static_assert(sus::construct::From<u8, uint8_t>);
  static_assert(!sus::construct::From<u8, uint16_t>);
  static_assert(!sus::construct::From<u8, uint32_t>);
  static_assert(!sus::construct::From<u8, uint64_t>);
  static_assert(sus::construct::TryFrom<u8, unsigned char>);
  static_assert(sus::construct::TryFrom<u8, size_t>);
  static_assert(sus::construct::TryFrom<u8, int8_t>);
  static_assert(sus::construct::TryFrom<u8, int16_t>);
  static_assert(sus::construct::TryFrom<u8, int32_t>);
  static_assert(sus::construct::TryFrom<u8, int64_t>);
  static_assert(sus::construct::TryFrom<u8, uint8_t>);
  static_assert(sus::construct::TryFrom<u8, uint16_t>);
  static_assert(sus::construct::TryFrom<u8, uint32_t>);
  static_assert(sus::construct::TryFrom<u8, uint64_t>);

  static_assert(sus::construct::From<u8, ENUM(, unsigned char)>);
  static_assert(!sus::construct::From<u8, ENUM(, size_t)>);
  static_assert(!sus::construct::From<u8, ENUM(, int8_t)>);
  static_assert(!sus::construct::From<u8, ENUM(, int16_t)>);
  static_assert(!sus::construct::From<u8, ENUM(, int32_t)>);
  static_assert(!sus::construct::From<u8, ENUM(, int64_t)>);
  static_assert(sus::construct::From<u8, ENUM(, uint8_t)>);
  static_assert(!sus::construct::From<u8, ENUM(, uint16_t)>);
  static_assert(!sus::construct::From<u8, ENUM(, uint32_t)>);
  static_assert(!sus::construct::From<u8, ENUM(, uint64_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, unsigned char)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, size_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, int8_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, int16_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, int32_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, int64_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, uint8_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, uint16_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, uint32_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(, uint64_t)>);

  static_assert(sus::construct::From<u8, ENUM(class, unsigned char)>);
  static_assert(!sus::construct::From<u8, ENUM(class, size_t)>);
  static_assert(!sus::construct::From<u8, ENUM(class, int8_t)>);
  static_assert(!sus::construct::From<u8, ENUM(class, int16_t)>);
  static_assert(!sus::construct::From<u8, ENUM(class, int32_t)>);
  static_assert(!sus::construct::From<u8, ENUM(class, int64_t)>);
  static_assert(sus::construct::From<u8, ENUM(class, uint8_t)>);
  static_assert(!sus::construct::From<u8, ENUM(class, uint16_t)>);
  static_assert(!sus::construct::From<u8, ENUM(class, uint32_t)>);
  static_assert(!sus::construct::From<u8, ENUM(class, uint64_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, unsigned char)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, size_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, int8_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, int16_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, int32_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, int64_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, uint8_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, uint16_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, uint32_t)>);
  static_assert(sus::construct::TryFrom<u8, ENUM(class, uint64_t)>);

  EXPECT_EQ(u8::from(unsigned char{2}), 2_u8);
  EXPECT_EQ(u8::from(uint8_t{2}), 2_u8);

  EXPECT_EQ(u8::try_from(unsigned char{2}).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(size_t{2}).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(int8_t{2}).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(int16_t{2}).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(int32_t{2}).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(int64_t{2}).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(uint8_t{2}).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(uint16_t{2}).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(uint32_t{2}).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(uint64_t{2}).unwrap(), 2_u8);

  EXPECT_TRUE(u8::try_from(int8_t{i8::MIN}).is_err());
  EXPECT_TRUE(u8::try_from(int8_t{i8::MAX}).is_ok());
  EXPECT_TRUE(u8::try_from(int16_t{i16::MIN}).is_err());
  EXPECT_TRUE(u8::try_from(int16_t{i16::MAX}).is_err());
  EXPECT_TRUE(u8::try_from(uint16_t{u16::MAX}).is_err());

  EXPECT_EQ(u8::from(ENUM(, unsigned char)::Z), 2_u8);
  EXPECT_EQ(u8::from(ENUM(, uint8_t)::Z), 2_u8);

  EXPECT_EQ(u8::try_from(ENUM(, unsigned char)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(, size_t)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(, int8_t)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(, int16_t)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(, int32_t)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(, int64_t)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(, uint8_t)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(, uint16_t)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(, uint32_t)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(, uint64_t)::Z).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(ENUM(class, uint64_t)::Z).unwrap(), 2_u8);

  EXPECT_TRUE(u8::try_from(ENUM(, int8_t)::MIN).is_err());
  EXPECT_TRUE(u8::try_from(ENUM(, int8_t)::MAX).is_ok());
  EXPECT_TRUE(u8::try_from(ENUM(, int16_t)::MIN).is_err());
  EXPECT_TRUE(u8::try_from(ENUM(, int16_t)::MAX).is_err());
  EXPECT_TRUE(u8::try_from(ENUM(, uint16_t)::MAX).is_err());
  EXPECT_TRUE(u8::try_from(ENUM(class, uint16_t)::MAX).is_err());

  static_assert(!sus::construct::From<u8, i8>);
  static_assert(!sus::construct::From<u8, i16>);
  static_assert(!sus::construct::From<u8, i32>);
  static_assert(!sus::construct::From<u8, i64>);
  static_assert(!sus::construct::From<u8, isize>);
  static_assert(sus::construct::From<u8, u8>);
  static_assert(!sus::construct::From<u8, u16>);
  static_assert(!sus::construct::From<u8, u32>);
  static_assert(!sus::construct::From<u8, u64>);
  static_assert(!sus::construct::From<u8, usize>);
  static_assert(!sus::construct::From<u8, uptr>);
  static_assert(sus::construct::TryFrom<u8, i8>);
  static_assert(sus::construct::TryFrom<u8, i16>);
  static_assert(sus::construct::TryFrom<u8, i32>);
  static_assert(sus::construct::TryFrom<u8, i64>);
  static_assert(sus::construct::TryFrom<u8, isize>);
  static_assert(sus::construct::TryFrom<u8, u8>);
  static_assert(sus::construct::TryFrom<u8, u16>);
  static_assert(sus::construct::TryFrom<u8, u32>);
  static_assert(sus::construct::TryFrom<u8, u64>);
  static_assert(sus::construct::TryFrom<u8, usize>);
  static_assert(sus::construct::TryFrom<u8, uptr>);

  EXPECT_EQ(u8::from(2_u8), 2_u8);

  EXPECT_EQ(u8::try_from(2_i8).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(2_i16).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(2_i32).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(2_i64).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(2_isize).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(2_u8).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(2_u16).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(2_u32).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(2_u64).unwrap(), 2_u8);
  EXPECT_EQ(u8::try_from(2_usize).unwrap(), 2_u8);

  EXPECT_TRUE(u8::try_from(i8::MIN).is_err());
  EXPECT_TRUE(u8::try_from(i8::MAX).is_ok());
  EXPECT_TRUE(u8::try_from(i16::MIN).is_err());
  EXPECT_TRUE(u8::try_from(i16::MAX).is_err());
  EXPECT_TRUE(u8::try_from(u16::MAX).is_err());
}

TEST(u8, InvokeEverything) {
  auto i = 10_u8, j = 11_u8;
  auto s = 3_i8;
  auto a = sus::Array<u8, sizeof(u8)>();

  (void)i.abs_diff(j);

  (void)i.checked_add(j);
  (void)i.checked_add_signed(s);
  (void)i.overflowing_add(j);
  (void)i.overflowing_add_signed(s);
  (void)i.saturating_add(j);
  (void)i.saturating_add_signed(s);
  (void)i.unchecked_add(unsafe_fn, j);
  (void)i.wrapping_add(j);
  (void)i.wrapping_add_signed(s);

  (void)i.checked_div(j);
  (void)i.overflowing_div(j);
  (void)i.saturating_div(j);
  (void)i.wrapping_div(j);

  (void)i.checked_mul(j);
  (void)i.overflowing_mul(j);
  (void)i.saturating_mul(j);
  (void)i.unchecked_mul(unsafe_fn, j);
  (void)i.wrapping_mul(j);

  (void)i.checked_neg();
  (void)i.overflowing_neg();
  (void)i.wrapping_neg();

  (void)i.checked_rem(j);
  (void)i.overflowing_rem(j);
  (void)i.wrapping_rem(j);

  (void)i.div_euclid(j);
  (void)i.checked_div_euclid(j);
  (void)i.overflowing_div_euclid(j);
  (void)i.wrapping_div_euclid(j);
  (void)i.rem_euclid(j);
  (void)i.checked_rem_euclid(j);
  (void)i.overflowing_rem_euclid(j);
  (void)i.wrapping_rem_euclid(j);

  (void)i.checked_shl(1_u32);
  (void)i.overflowing_shl(1_u32);
  (void)i.wrapping_shl(1_u32);
  (void)i.checked_shr(1_u32);
  (void)i.overflowing_shr(1_u32);
  (void)i.wrapping_shr(1_u32);

  (void)i.checked_sub(j);
  (void)i.overflowing_sub(j);
  (void)i.saturating_sub(j);
  (void)i.unchecked_sub(unsafe_fn, j);
  (void)i.wrapping_sub(j);

  (void)i.count_ones();
  (void)i.count_zeros();
  (void)i.leading_ones();
  (void)i.leading_zeros();
  (void)i.trailing_ones();
  (void)i.trailing_zeros();
  (void)i.reverse_bits();
  (void)i.rotate_left(1_u32);
  (void)i.rotate_right(1_u32);
  (void)i.swap_bytes();

  (void)i.pow(1_u32);
  (void)i.checked_pow(1_u32);
  (void)i.overflowing_pow(1_u32);
  (void)i.wrapping_pow(1_u32);

  (void)i.checked_log2();
  (void)i.log2();
  (void)i.checked_log10();
  (void)i.log10();
  (void)i.checked_log(j);
  (void)i.log(j);

  (void)i.next_power_of_two();
  (void)i.checked_next_power_of_two();
  (void)i.wrapping_next_power_of_two();

  (void)i.from_be(j);
  (void)i.from_le(j);
  (void)i.to_be();
  (void)i.to_le();
  (void)i.to_be_bytes();
  (void)i.to_le_bytes();
  (void)i.to_ne_bytes();
  (void)i.from_be_bytes(a);
  (void)i.from_le_bytes(a);
  (void)i.from_ne_bytes(a);

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

TEST(u8, fmt) {
  static_assert(fmt::is_formattable<u8, char>::value);
  EXPECT_EQ(fmt::format("{}", 123_u8), "123");
  EXPECT_EQ(fmt::format("{:#x}", 123_u8), "0x7b");
}

}  // namespace
