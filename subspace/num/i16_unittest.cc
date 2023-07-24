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

static_assert(std::is_signed_v<decltype(i16::primitive_value)>);
static_assert(sizeof(decltype(i16::primitive_value)) == 2);
static_assert(sizeof(i16) == sizeof(decltype(i16::primitive_value)));

static_assert(sus::mem::Copy<i16>);
static_assert(sus::mem::TrivialCopy<i16>);
static_assert(sus::mem::Clone<i16>);
static_assert(sus::mem::relocate_by_memcpy<i16>);
static_assert(sus::mem::Move<i16>);

namespace behaviour {
using T = i16;
using From = decltype(i16::primitive_value);
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

// i16::MAX
static_assert(i16::MAX.primitive_value == 0x7fff);
template <class T>
concept MaxInRange = requires {
  { 0x7fff_i16 } -> std::same_as<T>;
  { i16(int16_t{0x7fff}) } -> std::same_as<T>;
};
static_assert(MaxInRange<i16>);

// std hashing
static_assert(std::same_as<decltype(std::hash<i16>()(0_i16)), size_t>);
static_assert(std::same_as<decltype(std::equal_to<i16>()(0_i16, 1_i16)), bool>);

TEST(i16, Traits) {
  static_assert(sus::iter::__private::Step<i16>);

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

  static_assert(sus::ops::Ord<i16, int8_t>);
  static_assert(sus::ops::Ord<i16, int16_t>);
  static_assert(sus::ops::Ord<i16, int32_t>);
  static_assert(sus::ops::Ord<i16, int64_t>);
  static_assert(sus::ops::Ord<i16, i8>);
  static_assert(sus::ops::Ord<i16, i16>);
  static_assert(sus::ops::Ord<i16, i32>);
  static_assert(sus::ops::Ord<i16, i64>);
  static_assert(sus::ops::Ord<i16, isize>);
  static_assert(1_i16 >= 1_i16);
  static_assert(2_i16 > 1_i16);
  static_assert(1_i16 <= 1_i16);
  static_assert(1_i16 < 2_i16);
  static_assert(sus::ops::Eq<i16, int8_t>);
  static_assert(sus::ops::Eq<i16, int16_t>);
  static_assert(sus::ops::Eq<i16, int32_t>);
  static_assert(sus::ops::Eq<i16, int64_t>);
  static_assert(sus::ops::Eq<i16, i8>);
  static_assert(sus::ops::Eq<i16, i16>);
  static_assert(sus::ops::Eq<i16, i32>);
  static_assert(sus::ops::Eq<i16, i64>);
  static_assert(sus::ops::Eq<i16, isize>);
  static_assert(1_i16 == 1_i16);
  static_assert(!(1_i16 == 2_i16));
  static_assert(1_i16 != 2_i16);
  static_assert(!(1_i16 != 1_i16));

  // Verify constexpr.
  [[maybe_unused]] constexpr i16 c =
      1_i16 + 2_i16 - 3_i16 * 4_i16 / 5_i16 % 6_i16 & 7_i16 | 8_i16 ^ -9_i16;
  [[maybe_unused]] constexpr std::strong_ordering o = 2_i16 <=> 3_i16;
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
  constexpr auto max = i16::MAX;
  static_assert(std::same_as<decltype(max), const i16>);
  EXPECT_EQ(max.primitive_value, 0x7fff);
  constexpr auto min = i16::MIN;
  static_assert(std::same_as<decltype(min), const i16>);
  EXPECT_EQ(min.primitive_value, -0x7fff - 1);
  constexpr auto bits = i16::BITS;
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

template <auto From, class To>
concept IsConstexprAssignable = requires(To to) {
  { to = From };
};

TEST(i16, CompileTimeConversion) {
  using Self = i16;

  static_assert(is_constexpr_convertible<0_i8, Self>(0));
  static_assert(is_constexpr_convertible<0_i16, Self>(0));
  static_assert(!is_constexpr_convertible<0_i32, Self>(0));
  static_assert(!is_constexpr_convertible<0_i64, Self>(0));
  static_assert(!is_constexpr_convertible<0_isize, Self>(0));
  static_assert(is_constexpr_convertible<int8_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<int16_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<int32_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<int64_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<ptrdiff_t{0}, Self>(0));

  static_assert(IsConstexprAssignable<0_i8, Self>);
  static_assert(IsConstexprAssignable<0_i16, Self>);
  static_assert(!IsConstexprAssignable<0_i32, Self>);
  static_assert(!IsConstexprAssignable<0_i64, Self>);
  static_assert(!IsConstexprAssignable<0_isize, Self>);
  static_assert(IsConstexprAssignable<int8_t{0}, Self>);
  static_assert(IsConstexprAssignable<int16_t{0}, Self>);
  static_assert(!IsConstexprAssignable<int32_t{0}, Self>);
  static_assert(!IsConstexprAssignable<int64_t{0}, Self>);
  static_assert(!IsConstexprAssignable<ptrdiff_t{0}, Self>);

  static_assert(!IsExplicitlyConvertible<uint8_t, Self>);
  static_assert(!IsExplicitlyConvertible<uint16_t, Self>);
  static_assert(!IsExplicitlyConvertible<uint32_t, Self>);
  static_assert(!IsExplicitlyConvertible<uint64_t, Self>);
  static_assert(!IsExplicitlyConvertible<u8, Self>);
  static_assert(!IsExplicitlyConvertible<u16, Self>);
  static_assert(!IsExplicitlyConvertible<u32, Self>);
  static_assert(!IsExplicitlyConvertible<u64, Self>);
  static_assert(!IsExplicitlyConvertible<usize, Self>);
}

TEST(i16, CompileTimeConversionEnum) {
  using Self = i16;

  static_assert(is_constexpr_convertible<ENUM(, int8_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, int16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, int32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, int64_t)::X, Self>(0));

  // Conversion from enum class is explicit (constructible) instead of implicit
  // (convertible).
  static_assert(!is_constexpr_convertible<ENUM(class, int8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, int16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, int32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, int64_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int8_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int16_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, int32_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, int64_t)::X, Self>(0));
}

TEST(i16, ToPrimitive) {
  static_assert(NotConvertible<i16, int8_t>);
  static_assert(IsExplicitlyConvertible<i16, int16_t>);
  static_assert(IsExplicitlyConvertible<i16, int32_t>);
  static_assert(IsExplicitlyConvertible<i16, int64_t>);
  static_assert(NotConvertible<i16, uint8_t>);
  static_assert(IsExplicitlyConvertible<i16, uint16_t>);
  static_assert(IsExplicitlyConvertible<i16, uint32_t>);
  static_assert(IsExplicitlyConvertible<i16, uint64_t>);
  static_assert(IsExplicitlyConvertible<i16, size_t>);
}

TEST(i16, From) {
  static_assert(sus::construct::From<i16, char>);
  static_assert(sus::construct::From<i16, size_t>);
  static_assert(sus::construct::From<i16, int8_t>);
  static_assert(sus::construct::From<i16, int16_t>);
  static_assert(sus::construct::From<i16, int32_t>);
  static_assert(sus::construct::From<i16, int64_t>);
  static_assert(sus::construct::From<i16, uint8_t>);
  static_assert(sus::construct::From<i16, uint16_t>);
  static_assert(sus::construct::From<i16, uint32_t>);
  static_assert(sus::construct::From<i16, uint64_t>);
  static_assert(sus::construct::TryFrom<i16, char>);
  static_assert(sus::construct::TryFrom<i16, size_t>);
  static_assert(sus::construct::TryFrom<i16, int8_t>);
  static_assert(sus::construct::TryFrom<i16, int16_t>);
  static_assert(sus::construct::TryFrom<i16, int32_t>);
  static_assert(sus::construct::TryFrom<i16, int64_t>);
  static_assert(sus::construct::TryFrom<i16, uint8_t>);
  static_assert(sus::construct::TryFrom<i16, uint16_t>);
  static_assert(sus::construct::TryFrom<i16, uint32_t>);
  static_assert(sus::construct::TryFrom<i16, uint64_t>);

  static_assert(sus::construct::From<i16, ENUM(, char)>);
  static_assert(sus::construct::From<i16, ENUM(, size_t)>);
  static_assert(sus::construct::From<i16, ENUM(, int8_t)>);
  static_assert(sus::construct::From<i16, ENUM(, int16_t)>);
  static_assert(sus::construct::From<i16, ENUM(, int32_t)>);
  static_assert(sus::construct::From<i16, ENUM(, int64_t)>);
  static_assert(sus::construct::From<i16, ENUM(, uint8_t)>);
  static_assert(sus::construct::From<i16, ENUM(, uint16_t)>);
  static_assert(sus::construct::From<i16, ENUM(, uint32_t)>);
  static_assert(sus::construct::From<i16, ENUM(, uint64_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, char)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, size_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, int8_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, int16_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, int32_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, int64_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, uint8_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, uint16_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, uint32_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(, uint64_t)>);

  static_assert(sus::construct::From<i16, ENUM(class, char)>);
  static_assert(sus::construct::From<i16, ENUM(class, size_t)>);
  static_assert(sus::construct::From<i16, ENUM(class, int8_t)>);
  static_assert(sus::construct::From<i16, ENUM(class, int16_t)>);
  static_assert(sus::construct::From<i16, ENUM(class, int32_t)>);
  static_assert(sus::construct::From<i16, ENUM(class, int64_t)>);
  static_assert(sus::construct::From<i16, ENUM(class, uint8_t)>);
  static_assert(sus::construct::From<i16, ENUM(class, uint16_t)>);
  static_assert(sus::construct::From<i16, ENUM(class, uint32_t)>);
  static_assert(sus::construct::From<i16, ENUM(class, uint64_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, char)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, size_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, int8_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, int16_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, int32_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, int64_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, uint8_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, uint16_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, uint32_t)>);
  static_assert(sus::construct::TryFrom<i16, ENUM(class, uint64_t)>);

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

  EXPECT_EQ(i16::try_from(char{2}).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(size_t{2}).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(int8_t{2}).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(int16_t{2}).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(int32_t{2}).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(int64_t{2}).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(uint8_t{2}).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(uint16_t{2}).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(uint32_t{2}).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(uint64_t{2}).unwrap(), 2_i16);

  EXPECT_TRUE(i16::try_from(int32_t{i32::MIN}).is_err());
  EXPECT_TRUE(i16::try_from(int32_t{i32::MAX}).is_err());
  EXPECT_TRUE(i16::try_from(uint16_t{u16::MAX}).is_err());
  EXPECT_TRUE(i16::try_from(uint32_t{u32::MAX}).is_err());

  EXPECT_EQ(i16::from(ENUM(, char)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(, size_t)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(, int8_t)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(, int16_t)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(, int32_t)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(, int64_t)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(, uint8_t)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(, uint16_t)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(, uint32_t)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(, uint64_t)::Z), 2_i16);
  EXPECT_EQ(i16::from(ENUM(class, uint64_t)::Z), 2_i16);

  EXPECT_EQ(i16::try_from(ENUM(, char)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(, size_t)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(, int8_t)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(, int16_t)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(, int32_t)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(, int64_t)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(, uint8_t)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(, uint16_t)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(, uint32_t)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(, uint64_t)::Z).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(ENUM(class, uint64_t)::Z).unwrap(), 2_i16);

  EXPECT_TRUE(i16::try_from(ENUM(, int32_t)::MIN).is_err());
  EXPECT_TRUE(i16::try_from(ENUM(, int32_t)::MAX).is_err());
  EXPECT_TRUE(i16::try_from(ENUM(, uint16_t)::MAX).is_err());
  EXPECT_TRUE(i16::try_from(ENUM(, uint32_t)::MAX).is_err());
  EXPECT_TRUE(i16::try_from(ENUM(class, uint32_t)::MAX).is_err());

  EXPECT_EQ(i16::from_unchecked(unsafe_fn, char{2}), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, size_t{2}), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, int8_t{2}), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, int16_t{2}), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, int32_t{2}), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, int64_t{2}), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, uint8_t{2}), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, uint16_t{2}), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, uint32_t{2}), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, uint64_t{2}), 2_i16);

  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, char)::Z), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, size_t)::Z), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, int8_t)::Z), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, int16_t)::Z), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, int32_t)::Z), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, int64_t)::Z), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, uint8_t)::Z), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, uint16_t)::Z), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, uint32_t)::Z), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, ENUM(, uint64_t)::Z), 2_i16);

  static_assert(sus::construct::From<i16, i8>);
  static_assert(sus::construct::From<i16, i16>);
  static_assert(sus::construct::From<i16, i32>);
  static_assert(sus::construct::From<i16, i64>);
  static_assert(sus::construct::From<i16, isize>);
  static_assert(sus::construct::From<i16, u8>);
  static_assert(sus::construct::From<i16, u16>);
  static_assert(sus::construct::From<i16, u32>);
  static_assert(sus::construct::From<i16, u64>);
  static_assert(sus::construct::From<i16, usize>);
  static_assert(sus::construct::TryFrom<i16, i8>);
  static_assert(sus::construct::TryFrom<i16, i16>);
  static_assert(sus::construct::TryFrom<i16, i32>);
  static_assert(sus::construct::TryFrom<i16, i64>);
  static_assert(sus::construct::TryFrom<i16, isize>);
  static_assert(sus::construct::TryFrom<i16, u8>);
  static_assert(sus::construct::TryFrom<i16, u16>);
  static_assert(sus::construct::TryFrom<i16, u32>);
  static_assert(sus::construct::TryFrom<i16, u64>);
  static_assert(sus::construct::TryFrom<i16, usize>);

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

  EXPECT_EQ(i16::try_from(2_i8).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(2_i16).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(2_i32).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(2_i64).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(2_isize).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(2_u8).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(2_u16).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(2_u32).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(2_u64).unwrap(), 2_i16);
  EXPECT_EQ(i16::try_from(2_usize).unwrap(), 2_i16);

  EXPECT_TRUE(i16::try_from(i32::MIN).is_err());
  EXPECT_TRUE(i16::try_from(i32::MAX).is_err());
  EXPECT_TRUE(i16::try_from(u16::MAX).is_err());
  EXPECT_TRUE(i16::try_from(u32::MAX).is_err());

  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_i8), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_i16), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_i32), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_i64), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_isize), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_u8), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_u16), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_u32), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_u64), 2_i16);
  EXPECT_EQ(i16::from_unchecked(unsafe_fn, 2_usize), 2_i16);
}

TEST(i16DeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = i16::from(int64_t{-1 - 0x7fff'ffff'ffff'ffff});
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(uint64_t{0xffff'ffff'ffff'ffff});
        ensure_use(&x);
      },
      "");

  EXPECT_DEATH(
      {
        auto x = i16::from(ENUM(, int64_t)::MIN);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(ENUM(, int64_t)::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(ENUM(, uint64_t)::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(ENUM(class, int64_t)::MIN);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(ENUM(class, int64_t)::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(ENUM(class, uint64_t)::MAX);
        ensure_use(&x);
      },
      "");

  EXPECT_DEATH(
      {
        auto x = i16::from(i32::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(i64::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(u16::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(u32::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(u64::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = i16::from(usize::MAX);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(i16, AsBits) {
  using Self = i16;

  static_assert(sus::construct::AsBits<Self, char>);
  static_assert(sus::construct::AsBits<Self, signed char>);
  static_assert(sus::construct::AsBits<Self, unsigned char>);
  static_assert(sus::construct::AsBits<Self, int8_t>);
  static_assert(sus::construct::AsBits<Self, int16_t>);
  static_assert(sus::construct::AsBits<Self, int32_t>);
  static_assert(sus::construct::AsBits<Self, int64_t>);
  static_assert(sus::construct::AsBits<Self, uint8_t>);
  static_assert(sus::construct::AsBits<Self, uint16_t>);
  static_assert(sus::construct::AsBits<Self, uint32_t>);
  static_assert(sus::construct::AsBits<Self, uint64_t>);
  static_assert(sus::construct::AsBits<Self, size_t>);
  static_assert(sus::construct::AsBits<Self, intptr_t>);
  static_assert(sus::construct::AsBits<Self, uintptr_t>);
  static_assert(sus::construct::AsBits<Self, std::byte>);

  static_assert(sus::construct::AsBits<Self, i8>);
  static_assert(sus::construct::AsBits<Self, i16>);
  static_assert(sus::construct::AsBits<Self, i32>);
  static_assert(sus::construct::AsBits<Self, i64>);
  static_assert(sus::construct::AsBits<Self, isize>);
  static_assert(sus::construct::AsBits<Self, u8>);
  static_assert(sus::construct::AsBits<Self, u16>);
  static_assert(sus::construct::AsBits<Self, u32>);
  static_assert(sus::construct::AsBits<Self, u64>);
  static_assert(sus::construct::AsBits<Self, usize>);
  static_assert(sus::construct::AsBits<Self, uptr>);

  static_assert(sus::construct::AsBits<char, Self>);
  static_assert(sus::construct::AsBits<signed char, Self>);
  static_assert(sus::construct::AsBits<unsigned char, Self>);
  static_assert(sus::construct::AsBits<int8_t, Self>);
  static_assert(sus::construct::AsBits<int16_t, Self>);
  static_assert(sus::construct::AsBits<int32_t, Self>);
  static_assert(sus::construct::AsBits<int64_t, Self>);
  static_assert(sus::construct::AsBits<uint8_t, Self>);
  static_assert(sus::construct::AsBits<uint16_t, Self>);
  static_assert(sus::construct::AsBits<uint32_t, Self>);
  static_assert(sus::construct::AsBits<uint64_t, Self>);
  static_assert(sus::construct::AsBits<size_t, Self>);
  static_assert(sus::construct::AsBits<intptr_t, Self>);
  static_assert(sus::construct::AsBits<uintptr_t, Self>);
  static_assert(sus::construct::AsBits<std::byte, Self>);

  // Smaller unsigned to larger signed self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::as_bits<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, i16::MAX_PRIMITIVE);
  }
}

TEST(i16, InvokeEverything) {
  auto i = 10_i16, j = 11_i16;
  auto s = 3_u16;
  auto a = sus::Array<u8, sizeof(i16)>();

  (void)i.is_negative();
  (void)i.is_positive();
  (void)i.signum();

  (void)i.abs();
  (void)i.checked_abs();
  (void)i.overflowing_abs();
  (void)i.saturating_abs();
  (void)i.unsigned_abs();
  (void)i.wrapping_abs();
  (void)i.abs_diff(j);

  (void)i.checked_add(j);
  (void)i.checked_add_unsigned(s);
  (void)i.overflowing_add(j);
  (void)i.overflowing_add_unsigned(s);
  (void)i.saturating_add(j);
  (void)i.saturating_add_unsigned(s);
  (void)i.unchecked_add(unsafe_fn, j);
  (void)i.wrapping_add(j);
  (void)i.wrapping_add_unsigned(s);

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
  (void)i.checked_sub_unsigned(s);
  (void)i.overflowing_sub(j);
  (void)i.overflowing_sub_unsigned(s);
  (void)i.saturating_sub(j);
  (void)i.saturating_sub_unsigned(s);
  (void)i.unchecked_sub(unsafe_fn, j);
  (void)i.wrapping_sub(j);
  (void)i.wrapping_sub_unsigned(s);

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

TEST(i16, fmt) {
  static_assert(fmt::is_formattable<i16, char>::value);
  EXPECT_EQ(fmt::format("{}", -4321_i16), "-4321");
  EXPECT_EQ(fmt::format("{}", 12345_i16), "12345");
  EXPECT_EQ(fmt::format("{:+#x}", 12345_i16), "+0x3039");
}

}  // namespace
