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

static_assert(!std::is_signed_v<decltype(u16::primitive_value)>);
static_assert(sizeof(decltype(u16::primitive_value)) == 2);
static_assert(sizeof(u16) == sizeof(decltype(u16::primitive_value)));

static_assert(sus::mem::Copy<u16>);
static_assert(sus::mem::TrivialCopy<u16>);
static_assert(sus::mem::Clone<u16>);
static_assert(sus::mem::relocate_by_memcpy<u16>);
static_assert(sus::mem::Move<u16>);

namespace behaviour {
using T = u16;
using From = decltype(u16::primitive_value);
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

// u16::MAX
static_assert(u16::MAX.primitive_value == 0xffff);
template <class T>
concept MaxInRange = requires {
  { 0xffff_u16 } -> std::same_as<T>;
  { u16(uint16_t{0xffff}) } -> std::same_as<T>;
};
static_assert(MaxInRange<u16>);

// std hashing
static_assert(std::same_as<decltype(std::hash<u16>()(0_u16)), size_t>);
static_assert(std::same_as<decltype(std::equal_to<u16>()(0_u16, 1_u16)), bool>);

TEST(u16, Traits) {
  static_assert(sus::iter::__private::Step<u16>);

  // ** Unsigned only
  static_assert(!sus::num::Neg<u16>);

  static_assert(sus::num::Add<u16, u16>);
  static_assert(sus::num::AddAssign<u16, u16>);
  static_assert(sus::num::Sub<u16, u16>);
  static_assert(sus::num::SubAssign<u16, u16>);
  static_assert(sus::num::Mul<u16, u16>);
  static_assert(sus::num::MulAssign<u16, u16>);
  static_assert(sus::num::Div<u16, u16>);
  static_assert(sus::num::DivAssign<u16, u16>);
  static_assert(sus::num::Rem<u16, u16>);
  static_assert(sus::num::RemAssign<u16, u16>);
  static_assert(sus::num::BitAnd<u16, u16>);
  static_assert(sus::num::BitAndAssign<u16, u16>);
  static_assert(sus::num::BitOr<u16, u16>);
  static_assert(sus::num::BitOrAssign<u16, u16>);
  static_assert(sus::num::BitXor<u16, u16>);
  static_assert(sus::num::BitXorAssign<u16, u16>);
  static_assert(sus::num::BitNot<u16>);
  static_assert(sus::num::Shl<u16>);
  static_assert(sus::num::ShlAssign<u16>);
  static_assert(sus::num::Shr<u16>);
  static_assert(sus::num::ShrAssign<u16>);

  static_assert(sus::ops::Ord<u16, uint8_t>);
  static_assert(sus::ops::Ord<u16, uint16_t>);
  static_assert(sus::ops::Ord<u16, uint32_t>);
  static_assert(sus::ops::Ord<u16, uint64_t>);
  static_assert(sus::ops::Ord<u16, size_t>);
  static_assert(sus::ops::Ord<u16, u8>);
  static_assert(sus::ops::Ord<u16, u16>);
  static_assert(sus::ops::Ord<u16, u32>);
  static_assert(sus::ops::Ord<u16, u64>);
  static_assert(sus::ops::Ord<u16, usize>);
  static_assert(1_u16 >= 1_u16);
  static_assert(2_u16 > 1_u16);
  static_assert(1_u16 <= 1_u16);
  static_assert(1_u16 < 2_u16);
  static_assert(sus::ops::Eq<u16, uint8_t>);
  static_assert(sus::ops::Eq<u16, uint16_t>);
  static_assert(sus::ops::Eq<u16, uint32_t>);
  static_assert(sus::ops::Eq<u16, uint64_t>);
  static_assert(sus::ops::Eq<u16, size_t>);
  static_assert(sus::ops::Eq<u16, u8>);
  static_assert(sus::ops::Eq<u16, u16>);
  static_assert(sus::ops::Eq<u16, u32>);
  static_assert(sus::ops::Eq<u16, u64>);
  static_assert(sus::ops::Eq<u16, usize>);
  static_assert(1_u16 == 1_u16);
  static_assert(!(1_u16 == 2_u16));
  static_assert(1_u16 != 2_u16);
  static_assert(!(1_u16 != 1_u16));

  // Verify constexpr.
  [[maybe_unused]] constexpr u16 c =
      1_u16 + 2_u16 - 3_u16 * 4_u16 / 5_u16 % 6_u16 & 7_u16 | 8_u16 ^ 9_u16;
  [[maybe_unused]] constexpr std::strong_ordering o = 2_u16 <=> 3_u16;
}

TEST(u16, Literals) {
  // Hex.
  static_assert((0x12bC_u16).primitive_value == 0x12bC);
  static_assert((0X12bC_u16).primitive_value == 0X12bC);
  static_assert((0X0012bC_u16).primitive_value == 0X12bC);
  EXPECT_EQ((0x12bC_u16).primitive_value, 0x12bC);
  EXPECT_EQ((0X12bC_u16).primitive_value, 0X12bC);
  EXPECT_EQ((0X0012bC_u16).primitive_value, 0X12bC);
  // Binary.
  static_assert((0b101_u16).primitive_value == 0b101);
  static_assert((0B101_u16).primitive_value == 0B101);
  static_assert((0b00101_u16).primitive_value == 0b101);
  EXPECT_EQ((0b101_u16).primitive_value, 0b101);
  EXPECT_EQ((0B101_u16).primitive_value, 0B101);
  EXPECT_EQ((0b00101_u16).primitive_value, 0b101);
  // Octal.
  static_assert((0123_u16).primitive_value == 0123);
  static_assert((000123_u16).primitive_value == 0123);
  EXPECT_EQ((0123_u16).primitive_value, 0123);
  EXPECT_EQ((000123_u16).primitive_value, 0123);
  // Decimal.
  static_assert((0_u16).primitive_value == 0);
  static_assert((1_u16).primitive_value == 1);
  static_assert((12_u16).primitive_value == 12);
  static_assert((123_u16).primitive_value == 123);
  static_assert((1234_u16).primitive_value == 1234);
  static_assert((12345_u16).primitive_value == 12345);
}

TEST(u16, Constants) {
  constexpr auto max = u16::MAX;
  static_assert(std::same_as<decltype(max), const u16>);
  EXPECT_EQ(max.primitive_value, 0xffffu);
  constexpr auto min = u16::MIN;
  static_assert(std::same_as<decltype(min), const u16>);
  EXPECT_EQ(min.primitive_value, 0u);
  constexpr auto bits = u16::BITS;
  static_assert(std::same_as<decltype(bits), const u32>);
  EXPECT_EQ(bits, 16u);
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

TEST(u16, CompileTimeConversion) {
  using Self = u16;

  static_assert(is_constexpr_convertible<0_i8, Self>(0));
  static_assert(is_constexpr_convertible<0_i16, Self>(0));
  static_assert(is_constexpr_convertible<0_i32, Self>(0));
  static_assert(is_constexpr_convertible<0_i64, Self>(0));
  static_assert(is_constexpr_convertible<0_isize, Self>(0));
  static_assert(is_constexpr_convertible<int8_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<int16_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<int32_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<int64_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<-1_i8, Self>(0));
  static_assert(!is_constexpr_convertible<-1_i16, Self>(0));
  static_assert(!is_constexpr_convertible<-1_i32, Self>(0));
  static_assert(!is_constexpr_convertible<-1_i64, Self>(0));
  static_assert(!is_constexpr_convertible<-1_isize, Self>(0));
  static_assert(!is_constexpr_convertible<int8_t{-1}, Self>(0));
  static_assert(!is_constexpr_convertible<int16_t{-1}, Self>(0));
  static_assert(!is_constexpr_convertible<int32_t{-1}, Self>(0));
  static_assert(!is_constexpr_convertible<int64_t{-1}, Self>(0));

  static_assert(is_constexpr_convertible<i64{Self::MAX}, Self>(0));
  static_assert(is_constexpr_convertible<int64_t{Self::MAX}, Self>(0));
  static_assert(!is_constexpr_convertible<i64{Self::MAX} + 1u, Self>(0));
  static_assert(!is_constexpr_convertible<int64_t{Self::MAX} + 1u, Self>(0));
  static_assert(!is_constexpr_convertible<i64::MAX, Self>(0));
  static_assert(!is_constexpr_convertible<int64_t{i64::MAX}, Self>(0));

  static_assert(is_constexpr_convertible<0_u8, Self>(0));
  static_assert(is_constexpr_convertible<0_u16, Self>(0));
  static_assert(is_constexpr_convertible<0_u32, Self>(0));
  static_assert(is_constexpr_convertible<0_u64, Self>(0));
  static_assert(is_constexpr_convertible<0_usize, Self>(0));
  static_assert(is_constexpr_convertible<uint8_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<uint16_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<uint32_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<uint64_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<size_t{0}, Self>(0));

  static_assert(is_constexpr_convertible<u64{Self::MAX}, Self>(0));
  static_assert(is_constexpr_convertible<uint64_t{Self::MAX}, Self>(0));
  static_assert(!is_constexpr_convertible<u64{Self::MAX} + 1u, Self>(0));
  static_assert(!is_constexpr_convertible<uint64_t{Self::MAX} + 1u, Self>(0));
  static_assert(!is_constexpr_convertible<u64::MAX, Self>(0));
  static_assert(!is_constexpr_convertible<uint64_t{u64::MAX}, Self>(0));
}

TEST(u16, CompileTimeConversionEnum) {
  using Self = u16;

  static_assert(is_constexpr_convertible<ENUM(, int8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, int8_t)::MIN, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, int8_t)::MAX, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, int16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, int16_t)::MIN, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, int16_t)::MAX, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, int32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, int32_t)::MIN, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, int32_t)::MAX, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, int64_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, int64_t)::MIN, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, int64_t)::MAX, Self>(0));

  static_assert(!is_constexpr_convertible<ENUM(class, int8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, int16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, int32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, int64_t)::X, Self>(0));

  static_assert(is_constexpr_constructible<ENUM(class, int8_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, int8_t)::MIN, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int8_t)::MAX, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int16_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, int16_t)::MIN, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int16_t)::MAX, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int32_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, int32_t)::MIN, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, int32_t)::MAX, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int64_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, int64_t)::MIN, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, int64_t)::MAX, Self>(0));

  static_assert(is_constexpr_convertible<ENUM(, uint8_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint8_t)::MIN, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint8_t)::MAX, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint16_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint16_t)::MIN, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint16_t)::MAX, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint32_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint32_t)::MIN, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, uint32_t)::MAX, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint64_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint64_t)::MIN, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, uint64_t)::MAX, Self>(0));

  static_assert(!is_constexpr_convertible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint64_t)::X, Self>(0));

  static_assert(is_constexpr_constructible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint8_t)::MIN, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint8_t)::MAX, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint16_t)::MIN, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint16_t)::MAX, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint32_t)::MIN, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, uint32_t)::MAX, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint64_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint64_t)::MIN, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, uint64_t)::MAX, Self>(0));
}

TEST(u16, ToPrimitive) {
  static_assert(NotConvertible<u16, int8_t>);
  static_assert(NotConvertible<u16, int16_t>);
  static_assert(IsExplicitlyConvertible<u16, int32_t>);
  static_assert(IsExplicitlyConvertible<u16, int64_t>);
  static_assert(NotConvertible<u16, uint8_t>);
  static_assert(IsExplicitlyConvertible<u16, uint16_t>);
  static_assert(IsExplicitlyConvertible<u16, uint32_t>);
  static_assert(IsExplicitlyConvertible<u16, uint64_t>);
  static_assert(IsExplicitlyConvertible<u16, size_t>);
  static_assert(sizeof(u16) < sizeof(size_t));
}

TEST(u16, From) {
  static_assert(sus::construct::From<u16, char>);
  static_assert(sus::construct::From<u16, size_t>);
  static_assert(sus::construct::From<u16, int8_t>);
  static_assert(sus::construct::From<u16, int16_t>);
  static_assert(sus::construct::From<u16, int32_t>);
  static_assert(sus::construct::From<u16, int64_t>);
  static_assert(sus::construct::From<u16, uint8_t>);
  static_assert(sus::construct::From<u16, uint16_t>);
  static_assert(sus::construct::From<u16, uint32_t>);
  static_assert(sus::construct::From<u16, uint64_t>);
  static_assert(sus::construct::TryFrom<u16, char>);
  static_assert(sus::construct::TryFrom<u16, size_t>);
  static_assert(sus::construct::TryFrom<u16, int8_t>);
  static_assert(sus::construct::TryFrom<u16, int16_t>);
  static_assert(sus::construct::TryFrom<u16, int32_t>);
  static_assert(sus::construct::TryFrom<u16, int64_t>);
  static_assert(sus::construct::TryFrom<u16, uint8_t>);
  static_assert(sus::construct::TryFrom<u16, uint16_t>);
  static_assert(sus::construct::TryFrom<u16, uint32_t>);
  static_assert(sus::construct::TryFrom<u16, uint64_t>);

  static_assert(sus::construct::From<u16, ENUM(, char)>);
  static_assert(sus::construct::From<u16, ENUM(, size_t)>);
  static_assert(sus::construct::From<u16, ENUM(, int8_t)>);
  static_assert(sus::construct::From<u16, ENUM(, int16_t)>);
  static_assert(sus::construct::From<u16, ENUM(, int32_t)>);
  static_assert(sus::construct::From<u16, ENUM(, int64_t)>);
  static_assert(sus::construct::From<u16, ENUM(, uint8_t)>);
  static_assert(sus::construct::From<u16, ENUM(, uint16_t)>);
  static_assert(sus::construct::From<u16, ENUM(, uint32_t)>);
  static_assert(sus::construct::From<u16, ENUM(, uint64_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, char)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, size_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, int8_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, int16_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, int32_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, int64_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, uint8_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, uint16_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, uint32_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(, uint64_t)>);

  static_assert(sus::construct::From<u16, ENUM(class, char)>);
  static_assert(sus::construct::From<u16, ENUM(class, size_t)>);
  static_assert(sus::construct::From<u16, ENUM(class, int8_t)>);
  static_assert(sus::construct::From<u16, ENUM(class, int16_t)>);
  static_assert(sus::construct::From<u16, ENUM(class, int32_t)>);
  static_assert(sus::construct::From<u16, ENUM(class, int64_t)>);
  static_assert(sus::construct::From<u16, ENUM(class, uint8_t)>);
  static_assert(sus::construct::From<u16, ENUM(class, uint16_t)>);
  static_assert(sus::construct::From<u16, ENUM(class, uint32_t)>);
  static_assert(sus::construct::From<u16, ENUM(class, uint64_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, char)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, size_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, int8_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, int16_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, int32_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, int64_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, uint8_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, uint16_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, uint32_t)>);
  static_assert(sus::construct::TryFrom<u16, ENUM(class, uint64_t)>);

  EXPECT_EQ(u16::from(char{2}), 2_u16);
  EXPECT_EQ(u16::from(size_t{2}), 2_u16);
  EXPECT_EQ(u16::from(int8_t{2}), 2_u16);
  EXPECT_EQ(u16::from(int16_t{2}), 2_u16);
  EXPECT_EQ(u16::from(int32_t{2}), 2_u16);
  EXPECT_EQ(u16::from(int64_t{2}), 2_u16);
  EXPECT_EQ(u16::from(uint8_t{2}), 2_u16);
  EXPECT_EQ(u16::from(uint16_t{2}), 2_u16);
  EXPECT_EQ(u16::from(uint32_t{2}), 2_u16);
  EXPECT_EQ(u16::from(uint64_t{2}), 2_u16);

  EXPECT_EQ(u16::try_from(char{2}).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(size_t{2}).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(int8_t{2}).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(int16_t{2}).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(int32_t{2}).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(int64_t{2}).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(uint8_t{2}).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(uint16_t{2}).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(uint32_t{2}).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(uint64_t{2}).unwrap(), 2_u16);

  EXPECT_TRUE(u16::try_from(int16_t{i16::MIN}).is_err());
  EXPECT_TRUE(u16::try_from(int16_t{i16::MAX}).is_ok());
  EXPECT_TRUE(u16::try_from(int32_t{i32::MIN}).is_err());
  EXPECT_TRUE(u16::try_from(int32_t{i32::MAX}).is_err());
  EXPECT_TRUE(u16::try_from(uint32_t{u32::MAX}).is_err());

  EXPECT_EQ(u16::from(ENUM(, char)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(, size_t)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(, int8_t)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(, int16_t)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(, int32_t)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(, int64_t)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(, uint8_t)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(, uint16_t)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(, uint32_t)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(, uint64_t)::Z), 2_u16);
  EXPECT_EQ(u16::from(ENUM(class, uint64_t)::Z), 2_u16);

  EXPECT_EQ(u16::try_from(ENUM(, char)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(, size_t)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(, int8_t)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(, int16_t)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(, int32_t)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(, int64_t)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(, uint8_t)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(, uint16_t)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(, uint32_t)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(, uint64_t)::Z).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(ENUM(class, uint64_t)::Z).unwrap(), 2_u16);

  EXPECT_TRUE(u16::try_from(ENUM(, int16_t)::MIN).is_err());
  EXPECT_TRUE(u16::try_from(ENUM(, int16_t)::MAX).is_ok());
  EXPECT_TRUE(u16::try_from(ENUM(, int32_t)::MIN).is_err());
  EXPECT_TRUE(u16::try_from(ENUM(, int32_t)::MAX).is_err());
  EXPECT_TRUE(u16::try_from(ENUM(, uint32_t)::MAX).is_err());
  EXPECT_TRUE(u16::try_from(ENUM(class, uint32_t)::MAX).is_err());

  EXPECT_EQ(u16::from_unchecked(unsafe_fn, char{2}), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, size_t{2}), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, int8_t{2}), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, int16_t{2}), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, int32_t{2}), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, int64_t{2}), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, uint8_t{2}), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, uint16_t{2}), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, uint32_t{2}), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, uint64_t{2}), 2_u16);

  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, char)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, size_t)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, int8_t)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, int16_t)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, int32_t)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, int64_t)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, uint8_t)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, uint16_t)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, uint32_t)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(, uint64_t)::Z), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, ENUM(class, uint64_t)::Z), 2_u16);

  static_assert(sus::construct::From<u16, i8>);
  static_assert(sus::construct::From<u16, i16>);
  static_assert(sus::construct::From<u16, i32>);
  static_assert(sus::construct::From<u16, i64>);
  static_assert(sus::construct::From<u16, isize>);
  static_assert(sus::construct::From<u16, u8>);
  static_assert(sus::construct::From<u16, u16>);
  static_assert(sus::construct::From<u16, u32>);
  static_assert(sus::construct::From<u16, u64>);
  static_assert(sus::construct::From<u16, usize>);
  static_assert(sus::construct::TryFrom<u16, i8>);
  static_assert(sus::construct::TryFrom<u16, i16>);
  static_assert(sus::construct::TryFrom<u16, i32>);
  static_assert(sus::construct::TryFrom<u16, i64>);
  static_assert(sus::construct::TryFrom<u16, isize>);
  static_assert(sus::construct::TryFrom<u16, u8>);
  static_assert(sus::construct::TryFrom<u16, u16>);
  static_assert(sus::construct::TryFrom<u16, u32>);
  static_assert(sus::construct::TryFrom<u16, u64>);
  static_assert(sus::construct::TryFrom<u16, usize>);

  EXPECT_EQ(u16::from(2_i8), 2_u16);
  EXPECT_EQ(u16::from(2_i16), 2_u16);
  EXPECT_EQ(u16::from(2_i32), 2_u16);
  EXPECT_EQ(u16::from(2_i64), 2_u16);
  EXPECT_EQ(u16::from(2_isize), 2_u16);
  EXPECT_EQ(u16::from(2_u8), 2_u16);
  EXPECT_EQ(u16::from(2_u16), 2_u16);
  EXPECT_EQ(u16::from(2_u32), 2_u16);
  EXPECT_EQ(u16::from(2_u64), 2_u16);
  EXPECT_EQ(u16::from(2_usize), 2_u16);

  EXPECT_EQ(u16::try_from(2_i8).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(2_i16).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(2_i32).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(2_i64).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(2_isize).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(2_u8).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(2_u16).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(2_u32).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(2_u64).unwrap(), 2_u16);
  EXPECT_EQ(u16::try_from(2_usize).unwrap(), 2_u16);

  EXPECT_TRUE(u16::try_from(i16::MIN).is_err());
  EXPECT_TRUE(u16::try_from(i16::MAX).is_ok());
  EXPECT_TRUE(u16::try_from(i32::MIN).is_err());
  EXPECT_TRUE(u16::try_from(i32::MAX).is_err());
  EXPECT_TRUE(u16::try_from(u32::MAX).is_err());

  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_i8), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_i16), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_i32), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_i64), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_isize), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_u8), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_u16), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_u32), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_u64), 2_u16);
  EXPECT_EQ(u16::from_unchecked(unsafe_fn, 2_usize), 2_u16);
}

TEST(u16DeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = u16::from(int64_t{-1});
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(int64_t{-1 - 0x7fff'ffff'ffff'ffff});
        ensure_use(&x);
      },
      "");

  EXPECT_DEATH(
      {
        auto x = u16::from(ENUM(, int64_t)::MIN);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(ENUM(, int64_t)::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(ENUM(, uint64_t)::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(ENUM(class, int64_t)::MIN);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(ENUM(class, int64_t)::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(ENUM(class, uint64_t)::MAX);
        ensure_use(&x);
      },
      "");

  EXPECT_DEATH(
      {
        auto x = u16::from(-1_i8);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(-1_i16);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(-1_i32);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(-1_i64);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = u16::from(-1_isize);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(u16, InvokeEverything) {
  auto i = 10_u16, j = 11_u16;
  auto s = 3_i16;
  auto a = sus::Array<u8, sizeof(u16)>();

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

TEST(u16, fmt) {
  static_assert(fmt::is_formattable<u16, char>::value);
  EXPECT_EQ(fmt::format("{}", 12345_u16), "12345");
}

}  // namespace
