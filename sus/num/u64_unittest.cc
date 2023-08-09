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
#include "sus/construct/into.h"
#include "sus/containers/array.h"
#include "sus/iter/__private/step.h"
#include "sus/num/num_concepts.h"
#include "sus/num/signed_integer.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ops/eq.h"
#include "sus/ops/ord.h"
#include "sus/prelude.h"
#include "sus/test/ensure_use.h"
#include "sus/tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(!std::is_signed_v<decltype(u64::primitive_value)>);
static_assert(sizeof(decltype(u64::primitive_value)) == 8);
static_assert(sizeof(u64) == sizeof(decltype(u64::primitive_value)));

static_assert(sus::mem::Copy<u64>);
static_assert(sus::mem::TrivialCopy<u64>);
static_assert(sus::mem::Clone<u64>);
static_assert(sus::mem::relocate_by_memcpy<u64>);
static_assert(sus::mem::Move<u64>);

namespace behaviour {
using T = u64;
using From = decltype(u64::primitive_value);
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

// u64::MAX
static_assert(u64::MAX.primitive_value == 0xffffffff'ffffffff);
template <class T>
concept MaxInRange = requires {
  { 0xffffffff'ffffffff_u64 } -> std::same_as<T>;
  { u64(0xffffffff'ffffffff) } -> std::same_as<T>;
};
static_assert(MaxInRange<u64>);

// std hashing
static_assert(std::same_as<decltype(std::hash<u64>()(0_u64)), size_t>);
static_assert(std::same_as<decltype(std::equal_to<u64>()(0_u64, 1_u64)), bool>);

TEST(u64, Traits) {
  static_assert(sus::iter::__private::Step<u64>);

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

  static_assert(sus::ops::StrongOrd<u64, uint8_t>);
  static_assert(sus::ops::StrongOrd<u64, uint16_t>);
  static_assert(sus::ops::StrongOrd<u64, uint32_t>);
  static_assert(sus::ops::StrongOrd<u64, uint64_t>);
  static_assert(sus::ops::StrongOrd<u64, size_t>);
  static_assert(sus::ops::StrongOrd<u64, u8>);
  static_assert(sus::ops::StrongOrd<u64, u16>);
  static_assert(sus::ops::StrongOrd<u64, u32>);
  static_assert(sus::ops::StrongOrd<u64, u64>);
  static_assert(sus::ops::StrongOrd<u64, usize>);
  static_assert(1_u64 >= 1_u64);
  static_assert(2_u64 > 1_u64);
  static_assert(1_u64 <= 1_u64);
  static_assert(1_u64 < 2_u64);
  static_assert(sus::ops::Eq<u64, uint8_t>);
  static_assert(sus::ops::Eq<u64, uint16_t>);
  static_assert(sus::ops::Eq<u64, uint32_t>);
  static_assert(sus::ops::Eq<u64, uint64_t>);
  static_assert(sus::ops::Eq<u64, size_t>);
  static_assert(sus::ops::Eq<u64, u8>);
  static_assert(sus::ops::Eq<u64, u16>);
  static_assert(sus::ops::Eq<u64, u32>);
  static_assert(sus::ops::Eq<u64, u64>);
  static_assert(sus::ops::Eq<u64, usize>);
  static_assert(1_u64 == 1_u64);
  static_assert(!(1_u64 == 2_u64));
  static_assert(1_u64 != 2_u64);
  static_assert(!(1_u64 != 1_u64));

  // Verify constexpr.
  [[maybe_unused]] constexpr u64 c =
      1_u64 + 2_u64 - 3_u64 * 4_u64 / 5_u64 % 6_u64 & 7_u64 | 8_u64 ^ 9_u64;
  [[maybe_unused]] constexpr std::strong_ordering o = 2_u64 <=> 3_u64;
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
  constexpr auto max = u64::MAX;
  static_assert(std::same_as<decltype(max), const u64>);
  EXPECT_EQ(max.primitive_value, 0xffffffff'ffffffffu);
  constexpr auto min = u64::MIN;
  static_assert(std::same_as<decltype(min), const u64>);
  EXPECT_EQ(min.primitive_value, 0u);
  constexpr auto bits = u64::BITS;
  static_assert(std::same_as<decltype(bits), const u32>);
  EXPECT_EQ(bits, 64u);
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

TEST(u64, CompileTimeConversion) {
  using Self = u64;

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

  static_assert(IsConstexprAssignable<0_u8, Self>);
  static_assert(IsConstexprAssignable<0_u16, Self>);
  static_assert(IsConstexprAssignable<0_u32, Self>);
  static_assert(IsConstexprAssignable<0_u64, Self>);
  static_assert(IsConstexprAssignable<0_usize, Self>);
  static_assert(IsConstexprAssignable<uint8_t{0}, Self>);
  static_assert(IsConstexprAssignable<uint16_t{0}, Self>);
  static_assert(IsConstexprAssignable<uint32_t{0}, Self>);
  static_assert(IsConstexprAssignable<uint64_t{0}, Self>);
  static_assert(IsConstexprAssignable<size_t{0}, Self>);

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

TEST(u64, CompileTimeConversionEnum) {
  using Self = u64;

  static_assert(is_constexpr_convertible<ENUM(, uint8_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint16_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint32_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint64_t)::X, Self>(0));

  // Conversion from enum class is explicit (constructible) instead of implicit
  // (convertible).
  static_assert(!is_constexpr_convertible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint64_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint64_t)::X, Self>(0));
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
  using unsigned_char = unsigned char;

  static_assert(sus::construct::From<u64, bool>);
  static_assert(sus::construct::From<u64, unsigned_char>);
  static_assert(sus::construct::From<u64, size_t>);
  static_assert(!sus::construct::From<u64, int8_t>);
  static_assert(!sus::construct::From<u64, int16_t>);
  static_assert(!sus::construct::From<u64, int32_t>);
  static_assert(!sus::construct::From<u64, int64_t>);
  static_assert(sus::construct::From<u64, uint8_t>);
  static_assert(sus::construct::From<u64, uint16_t>);
  static_assert(sus::construct::From<u64, uint32_t>);
  static_assert(sus::construct::From<u64, uint64_t>);
  static_assert(sus::construct::TryFrom<u64,unsigned  char>);
  static_assert(sus::construct::TryFrom<u64, size_t>);
  static_assert(sus::construct::TryFrom<u64, int8_t>);
  static_assert(sus::construct::TryFrom<u64, int16_t>);
  static_assert(sus::construct::TryFrom<u64, int32_t>);
  static_assert(sus::construct::TryFrom<u64, int64_t>);
  static_assert(sus::construct::TryFrom<u64, uint8_t>);
  static_assert(sus::construct::TryFrom<u64, uint16_t>);
  static_assert(sus::construct::TryFrom<u64, uint32_t>);
  static_assert(sus::construct::TryFrom<u64, uint64_t>);

  static_assert(sus::construct::From<u64, ENUM(,unsigned  char)>);
  static_assert(sus::construct::From<u64, ENUM(, size_t)>);
  static_assert(!sus::construct::From<u64, ENUM(, int8_t)>);
  static_assert(!sus::construct::From<u64, ENUM(, int16_t)>);
  static_assert(!sus::construct::From<u64, ENUM(, int32_t)>);
  static_assert(!sus::construct::From<u64, ENUM(, int64_t)>);
  static_assert(sus::construct::From<u64, ENUM(, uint8_t)>);
  static_assert(sus::construct::From<u64, ENUM(, uint16_t)>);
  static_assert(sus::construct::From<u64, ENUM(, uint32_t)>);
  static_assert(sus::construct::From<u64, ENUM(, uint64_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(,unsigned  char)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(, size_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(, int8_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(, int16_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(, int32_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(, int64_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(, uint8_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(, uint16_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(, uint32_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(, uint64_t)>);

  static_assert(sus::construct::From<u64, ENUM(class, unsigned_char)>);
  static_assert(sus::construct::From<u64, ENUM(class, size_t)>);
  static_assert(!sus::construct::From<u64, ENUM(class, int8_t)>);
  static_assert(!sus::construct::From<u64, ENUM(class, int16_t)>);
  static_assert(!sus::construct::From<u64, ENUM(class, int32_t)>);
  static_assert(!sus::construct::From<u64, ENUM(class, int64_t)>);
  static_assert(sus::construct::From<u64, ENUM(class, uint8_t)>);
  static_assert(sus::construct::From<u64, ENUM(class, uint16_t)>);
  static_assert(sus::construct::From<u64, ENUM(class, uint32_t)>);
  static_assert(sus::construct::From<u64, ENUM(class, uint64_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class,unsigned  char)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class, size_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class, int8_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class, int16_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class, int32_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class, int64_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class, uint8_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class, uint16_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class, uint32_t)>);
  static_assert(sus::construct::TryFrom<u64, ENUM(class, uint64_t)>);

  EXPECT_EQ(u64::from(unsigned_char{2}), 2_u64);
  EXPECT_EQ(u64::from(size_t{2}), 2_u64);
  EXPECT_EQ(u64::from(uint8_t{2}), 2_u64);
  EXPECT_EQ(u64::from(uint16_t{2}), 2_u64);
  EXPECT_EQ(u64::from(uint32_t{2}), 2_u64);
  EXPECT_EQ(u64::from(uint64_t{2}), 2_u64);

  EXPECT_EQ(u64::try_from(unsigned_char{2}).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(size_t{2}).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(int8_t{2}).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(int16_t{2}).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(int32_t{2}).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(int64_t{2}).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(uint8_t{2}).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(uint16_t{2}).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(uint32_t{2}).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(uint64_t{2}).unwrap(), 2_u64);

  EXPECT_TRUE(u64::try_from(int64_t{i64::MIN}).is_err());
  EXPECT_TRUE(u64::try_from(int64_t{i64::MAX}).is_ok());

  EXPECT_EQ(u64::from(ENUM(, unsigned_char)::Z), 2_u64);
  EXPECT_EQ(u64::from(ENUM(, size_t)::Z), 2_u64);
  EXPECT_EQ(u64::from(ENUM(, uint8_t)::Z), 2_u64);
  EXPECT_EQ(u64::from(ENUM(, uint16_t)::Z), 2_u64);
  EXPECT_EQ(u64::from(ENUM(, uint32_t)::Z), 2_u64);
  EXPECT_EQ(u64::from(ENUM(, uint64_t)::Z), 2_u64);
  EXPECT_EQ(u64::from(ENUM(class, uint64_t)::Z), 2_u64);

  EXPECT_EQ(u64::try_from(ENUM(, unsigned_char)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(, size_t)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(, int8_t)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(, int16_t)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(, int32_t)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(, int64_t)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(, uint8_t)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(, uint16_t)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(, uint32_t)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(, uint64_t)::Z).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(ENUM(class, uint64_t)::Z).unwrap(), 2_u64);

  EXPECT_TRUE(u64::try_from(ENUM(, int64_t)::MIN).is_err());
  EXPECT_TRUE(u64::try_from(ENUM(, int64_t)::MAX).is_ok());
  EXPECT_TRUE(u64::try_from(ENUM(class, int64_t)::MAX).is_ok());

  static_assert(!sus::construct::From<u64, i8>);
  static_assert(!sus::construct::From<u64, i16>);
  static_assert(!sus::construct::From<u64, i32>);
  static_assert(!sus::construct::From<u64, i64>);
  static_assert(!sus::construct::From<u64, isize>);
  static_assert(sus::construct::From<u64, u8>);
  static_assert(sus::construct::From<u64, u16>);
  static_assert(sus::construct::From<u64, u32>);
  static_assert(sus::construct::From<u64, u64>);
  static_assert(sus::construct::From<u64, usize>);
  static_assert(sizeof(uptr) <= sizeof(u64) == sus::construct::From<u64, uptr>);
  static_assert(sus::construct::TryFrom<u64, i8>);
  static_assert(sus::construct::TryFrom<u64, i16>);
  static_assert(sus::construct::TryFrom<u64, i32>);
  static_assert(sus::construct::TryFrom<u64, i64>);
  static_assert(sus::construct::TryFrom<u64, isize>);
  static_assert(sus::construct::TryFrom<u64, u8>);
  static_assert(sus::construct::TryFrom<u64, u16>);
  static_assert(sus::construct::TryFrom<u64, u32>);
  static_assert(sus::construct::TryFrom<u64, u64>);
  static_assert(sus::construct::TryFrom<u64, usize>);
  static_assert(sus::construct::TryFrom<u64, uptr>);

  EXPECT_EQ(u64::from(2_u8), 2_u64);
  EXPECT_EQ(u64::from(2_u16), 2_u64);
  EXPECT_EQ(u64::from(2_u32), 2_u64);
  EXPECT_EQ(u64::from(2_u64), 2_u64);
  EXPECT_EQ(u64::from(2_usize), 2_u64);

  EXPECT_EQ(u64::try_from(2_i8).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(2_i16).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(2_i32).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(2_i64).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(2_isize).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(2_u8).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(2_u16).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(2_u32).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(2_u64).unwrap(), 2_u64);
  EXPECT_EQ(u64::try_from(2_usize).unwrap(), 2_u64);

  EXPECT_TRUE(u64::try_from(i64::MIN).is_err());
  EXPECT_TRUE(u64::try_from(i64::MAX).is_ok());
}

TEST(u64, CheckedMul) {
  constexpr auto a = (1_u64).checked_mul(3_u64).unwrap();
  EXPECT_EQ(a, 3_u64);

  EXPECT_EQ((100_u64).checked_mul(21_u64), sus::some(2100_u64).construct());
  EXPECT_EQ((21_u64).checked_mul(100_u64), sus::some(2100_u64).construct());
  EXPECT_EQ((u64::MAX).checked_mul(2_u64), sus::None);
}

TEST(u64, InvokeEverything) {
  auto i = 10_u64, j = 11_u64;
  auto s = 3_i64;
  auto a = sus::Array<u8, sizeof(u64)>();

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

TEST(u64, fmt) {
  static_assert(fmt::is_formattable<u64, char>::value);
  EXPECT_EQ(fmt::format("{}", 123456789_u64), "123456789");
  EXPECT_EQ(fmt::format("{:#x}", 123456789_u64), "0x75bcd15");
}

}  // namespace
