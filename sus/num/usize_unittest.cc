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
#include "sus/collections/array.h"
#include "sus/iter/__private/step.h"
#include "sus/num/num_concepts.h"
#include "sus/num/signed_integer.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ops/eq.h"
#include "sus/ops/ord.h"
#include "sus/prelude.h"
#include "sus/tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(!std::is_signed_v<decltype(usize::primitive_value)>);
static_assert(sizeof(decltype(usize::primitive_value)) == sizeof(void*));
static_assert(sizeof(usize) == sizeof(decltype(usize::primitive_value)));

static_assert(sus::mem::Copy<usize>);
static_assert(sus::mem::TrivialCopy<usize>);
static_assert(sus::mem::Clone<usize>);
static_assert(sus::mem::relocate_by_memcpy<usize>);
static_assert(sus::mem::Move<usize>);

namespace behaviour {
using T = usize;
using From = decltype(usize::primitive_value);
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
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
static_assert(sus::construct::Default<T>);
static_assert(sus::mem::relocate_by_memcpy<T>);
}  // namespace behaviour

// usize::MAX
static_assert(sizeof(usize) != sizeof(u64)
                  ? usize::MAX.primitive_value == 0xffffffff
                  : usize::MAX.primitive_value == 0xffffffff'ffffffff);
template <class T>
concept MaxInRange = requires {
  {
    sizeof(usize) != sizeof(u64) ? 0xffffffff_usize : 0xffffffff'ffffffff_usize
  } -> std::same_as<T>;
  {
    usize(sizeof(usize) != sizeof(u64) ? 0xffffffff : 0xffffffff'ffffffff)
  } -> std::same_as<T>;
};
static_assert(MaxInRange<usize>);

// std hashing
static_assert(std::same_as<decltype(std::hash<usize>()(0_usize)), size_t>);
static_assert(
    std::same_as<decltype(std::equal_to<usize>()(0_usize, 1_usize)), bool>);

TEST(usize, Traits) {
  static_assert(sus::iter::__private::Step<usize>);

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

  static_assert(sus::ops::StrongOrd<usize, uint8_t>);
  static_assert(sus::ops::StrongOrd<usize, uint16_t>);
  static_assert(sus::ops::StrongOrd<usize, uint64_t>);
  static_assert(sus::ops::StrongOrd<usize, uint64_t>);
  static_assert(sus::ops::StrongOrd<usize, size_t>);
  static_assert(sus::ops::StrongOrd<usize, u8>);
  static_assert(sus::ops::StrongOrd<usize, u16>);
  static_assert(sus::ops::StrongOrd<usize, u32>);
  static_assert(sus::ops::StrongOrd<usize, u64>);
  static_assert(sus::ops::StrongOrd<usize, usize>);
  static_assert(1_usize >= 1_usize);
  static_assert(2_usize > 1_usize);
  static_assert(1_usize <= 1_usize);
  static_assert(1_usize < 2_usize);
  static_assert(sus::ops::Eq<usize, uint8_t>);
  static_assert(sus::ops::Eq<usize, uint16_t>);
  static_assert(sus::ops::Eq<usize, uint64_t>);
  static_assert(sus::ops::Eq<usize, uint64_t>);
  static_assert(sus::ops::Eq<usize, size_t>);
  static_assert(sus::ops::Eq<usize, u8>);
  static_assert(sus::ops::Eq<usize, u16>);
  static_assert(sus::ops::Eq<usize, u32>);
  static_assert(sus::ops::Eq<usize, u64>);
  static_assert(sus::ops::Eq<usize, usize>);
  static_assert(1_usize == 1_usize);
  static_assert(!(1_usize == 2_usize));
  static_assert(1_usize != 2_usize);
  static_assert(!(1_usize != 1_usize));

  // Verify constexpr.
  [[maybe_unused]] constexpr usize c =
      1_usize + 2_usize - 3_usize * 4_usize / 5_usize % 6_usize & 7_usize |
      8_usize ^ 9_usize;
  [[maybe_unused]] constexpr std::strong_ordering o = 2_usize <=> 3_usize;
}

TEST(usize, Literals) {
  // Hex.
  static_assert((0x123abC_usize).primitive_value == 0x123abC);
  static_assert((0X123abC_usize).primitive_value == 0X123abC);
  static_assert((0X00123abC_usize).primitive_value == 0X123abC);
  EXPECT_EQ((0x123abC_usize).primitive_value, 0x123abCu);
  EXPECT_EQ((0X123abC_usize).primitive_value, 0X123abCu);
  EXPECT_EQ((0X00123abC_usize).primitive_value, 0X123abCu);
  // Binary.
  static_assert((0b101_usize).primitive_value == 0b101);
  static_assert((0B101_usize).primitive_value == 0B101);
  static_assert((0b00101_usize).primitive_value == 0b101);
  EXPECT_EQ((0b101_usize).primitive_value, 0b101u);
  EXPECT_EQ((0B101_usize).primitive_value, 0B101u);
  EXPECT_EQ((0b00101_usize).primitive_value, 0b101u);
  // Octal.
  static_assert((0123_usize).primitive_value == 0123);
  static_assert((000123_usize).primitive_value == 0123);
  EXPECT_EQ((0123_usize).primitive_value, 0123u);
  EXPECT_EQ((000123_usize).primitive_value, 0123u);
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
  constexpr auto max = usize::MAX;
  static_assert(std::same_as<decltype(max), const usize>);
  if constexpr (sizeof(usize) != sizeof(u64))
    EXPECT_EQ(max.primitive_value, 0xffffffffu);
  else
    EXPECT_EQ(max.primitive_value, 0xffffffff'ffffffffu);
  constexpr auto min = usize::MIN;
  static_assert(std::same_as<decltype(min), const usize>);
  EXPECT_EQ(min.primitive_value, 0u);
  constexpr auto bits = usize::BITS;
  static_assert(std::same_as<decltype(bits), const u32>);
  if constexpr (sizeof(usize) != sizeof(u64))
    EXPECT_EQ(bits, 32u);
  else
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

TEST(usize, CompileTimeConversion) {
  using Self = usize;

  static_assert(is_constexpr_convertible<0_u8, Self>(0));
  static_assert(is_constexpr_convertible<0_u16, Self>(0));
  static_assert(is_constexpr_convertible<0_u32, Self>(0));
  static_assert(sizeof(Self) >= sizeof(u64) ==
                is_constexpr_convertible<0_u64, Self>(0));
  static_assert(is_constexpr_convertible<0_usize, Self>(0));
  static_assert(is_constexpr_convertible<uint8_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<uint16_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<uint32_t{0}, Self>(0));
  static_assert(sizeof(Self) >= sizeof(u64) ==
                is_constexpr_convertible<uint64_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<size_t{0}, Self>(0));

  static_assert(IsConstexprAssignable<0_u8, Self>);
  static_assert(IsConstexprAssignable<0_u16, Self>);
  static_assert(IsConstexprAssignable<0_u32, Self>);
  static_assert(sizeof(Self) >= sizeof(u64) ==
                IsConstexprAssignable<0_u64, Self>);
  static_assert(IsConstexprAssignable<0_usize, Self>);
  static_assert(IsConstexprAssignable<uint8_t{0}, Self>);
  static_assert(IsConstexprAssignable<uint16_t{0}, Self>);
  static_assert(IsConstexprAssignable<uint32_t{0}, Self>);
  static_assert(sizeof(Self) >= sizeof(u64) ==
                IsConstexprAssignable<uint64_t{0}, Self>);
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

TEST(usize, CompileTimeConversionEnum) {
  using Self = usize;

  static_assert(is_constexpr_convertible<ENUM(, uint8_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint16_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, uint32_t)::X, Self>(0));
  static_assert(sizeof(Self) >= sizeof(u64) ==
                is_constexpr_convertible<ENUM(, uint64_t)::X, Self>(0));

  // Conversion from enum class is explicit (constructible) instead of implicit
  // (convertible).
  static_assert(!is_constexpr_convertible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint64_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(sizeof(Self) >= sizeof(u64) ==
                is_constexpr_constructible<ENUM(class, uint64_t)::X, Self>(0));
}

TEST(usize, ToPrimitive) {
  static_assert(NotConvertible<usize, int8_t>);
  static_assert(NotConvertible<usize, int16_t>);
  static_assert(NotConvertible<usize, int32_t>);
  static_assert(NotConvertible<usize, int64_t>);
  static_assert(NotConvertible<usize, uint8_t>);
  static_assert(NotConvertible<usize, uint16_t>);
  static_assert(sizeof(usize) <= sizeof(uint32_t)
                    ? IsExplicitlyConvertible<usize, uint32_t>
                    : NotConvertible<usize, uint32_t>);
  static_assert(sizeof(usize) <= sizeof(uint64_t)
                    ? IsExplicitlyConvertible<usize, uint64_t>
                    : NotConvertible<usize, uint64_t>);
  static_assert(IsExplicitlyConvertible<usize, size_t>);
}

TEST(usize, From) {
  using unsigned_char = unsigned char;

  static_assert(sus::construct::From<usize, bool>);
  static_assert(sus::construct::From<usize, unsigned_char>);
  static_assert(sus::construct::From<usize, size_t>);
  static_assert(!sus::construct::From<usize, int8_t>);
  static_assert(!sus::construct::From<usize, int16_t>);
  static_assert(!sus::construct::From<usize, int32_t>);
  static_assert(!sus::construct::From<usize, int64_t>);
  static_assert(sus::construct::From<usize, uint8_t>);
  static_assert(sus::construct::From<usize, uint16_t>);
  static_assert(sus::construct::From<usize, uint32_t>);
  static_assert(sus::construct::From<usize, uint64_t>);
  static_assert(sus::construct::TryFrom<usize, unsigned_char>);
  static_assert(sus::construct::TryFrom<usize, size_t>);
  static_assert(sus::construct::TryFrom<usize, int8_t>);
  static_assert(sus::construct::TryFrom<usize, int16_t>);
  static_assert(sus::construct::TryFrom<usize, int32_t>);
  static_assert(sus::construct::TryFrom<usize, int64_t>);
  static_assert(sus::construct::TryFrom<usize, uint8_t>);
  static_assert(sus::construct::TryFrom<usize, uint16_t>);
  static_assert(sus::construct::TryFrom<usize, uint32_t>);
  static_assert(sus::construct::TryFrom<usize, uint64_t>);

  static_assert(sus::construct::From<usize, ENUM(, unsigned_char)>);
  static_assert(sus::construct::From<usize, ENUM(, size_t)>);
  static_assert(!sus::construct::From<usize, ENUM(, int8_t)>);
  static_assert(!sus::construct::From<usize, ENUM(, int16_t)>);
  static_assert(!sus::construct::From<usize, ENUM(, int32_t)>);
  static_assert(!sus::construct::From<usize, ENUM(, int64_t)>);
  static_assert(sus::construct::From<usize, ENUM(, uint8_t)>);
  static_assert(sus::construct::From<usize, ENUM(, uint16_t)>);
  static_assert(sus::construct::From<usize, ENUM(, uint32_t)>);
  static_assert(sus::construct::From<usize, ENUM(, uint64_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, unsigned_char)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, size_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, int8_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, int16_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, int32_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, int64_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, uint8_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, uint16_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, uint32_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(, uint64_t)>);

  static_assert(sus::construct::From<usize, ENUM(class, unsigned_char)>);
  static_assert(sus::construct::From<usize, ENUM(class, size_t)>);
  static_assert(!sus::construct::From<usize, ENUM(class, int8_t)>);
  static_assert(!sus::construct::From<usize, ENUM(class, int16_t)>);
  static_assert(!sus::construct::From<usize, ENUM(class, int32_t)>);
  static_assert(!sus::construct::From<usize, ENUM(class, int64_t)>);
  static_assert(sus::construct::From<usize, ENUM(class, uint8_t)>);
  static_assert(sus::construct::From<usize, ENUM(class, uint16_t)>);
  static_assert(sus::construct::From<usize, ENUM(class, uint32_t)>);
  static_assert(sus::construct::From<usize, ENUM(class, uint64_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, unsigned_char)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, size_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, int8_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, int16_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, int32_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, int64_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, uint8_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, uint16_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, uint32_t)>);
  static_assert(sus::construct::TryFrom<usize, ENUM(class, uint64_t)>);

  EXPECT_EQ(usize::from(unsigned_char{2}), 2_usize);
  EXPECT_EQ(usize::from(size_t{2}), 2_usize);
  EXPECT_EQ(usize::from(uint8_t{2}), 2_usize);
  EXPECT_EQ(usize::from(uint16_t{2}), 2_usize);
  EXPECT_EQ(usize::from(uint32_t{2}), 2_usize);
  EXPECT_EQ(usize::from(uint64_t{2}), 2_usize);

  EXPECT_EQ(usize::try_from(unsigned_char{2}).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(size_t{2}).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(int8_t{2}).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(int16_t{2}).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(int32_t{2}).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(int64_t{2}).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(uint8_t{2}).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(uint16_t{2}).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(uint32_t{2}).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(uint64_t{2}).unwrap(), 2_usize);

  if constexpr (sizeof(usize) == sizeof(u32)) {
    EXPECT_TRUE(usize::try_from(uint32_t{u32::MAX}).is_ok());
    EXPECT_TRUE(usize::try_from(uint64_t{u64::MAX}).is_err());
  }
  EXPECT_TRUE(usize::try_from(int64_t{i64::MIN}).is_err());
  EXPECT_TRUE(usize::try_from(int64_t{i64::MAX}).is_ok());

  EXPECT_EQ(usize::from(ENUM(, unsigned_char)::Z), 2_usize);
  EXPECT_EQ(usize::from(ENUM(, size_t)::Z), 2_usize);
  EXPECT_EQ(usize::from(ENUM(, uint8_t)::Z), 2_usize);
  EXPECT_EQ(usize::from(ENUM(, uint16_t)::Z), 2_usize);
  EXPECT_EQ(usize::from(ENUM(, uint32_t)::Z), 2_usize);
  EXPECT_EQ(usize::from(ENUM(, uint64_t)::Z), 2_usize);
  EXPECT_EQ(usize::from(ENUM(class, uint64_t)::Z), 2_usize);

  EXPECT_EQ(usize::try_from(ENUM(,unsigned  char)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(, size_t)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(, int8_t)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(, int16_t)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(, int32_t)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(, int64_t)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(, uint8_t)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(, uint16_t)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(, uint32_t)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(, uint64_t)::Z).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(ENUM(class, uint64_t)::Z).unwrap(), 2_usize);

  if constexpr (sizeof(usize) == sizeof(u32)) {
    EXPECT_TRUE(usize::try_from(ENUM(, uint32_t)::MAX).is_ok());
    EXPECT_TRUE(usize::try_from(ENUM(, uint64_t)::MAX).is_err());
    EXPECT_TRUE(usize::try_from(ENUM(class, uint64_t)::MAX).is_err());
  }
  EXPECT_TRUE(usize::try_from(ENUM(, int64_t)::MIN).is_err());
  EXPECT_TRUE(usize::try_from(ENUM(, int64_t)::MAX).is_ok());
  EXPECT_TRUE(usize::try_from(ENUM(class, int64_t)::MAX).is_ok());

  static_assert(!sus::construct::From<usize, i8>);
  static_assert(!sus::construct::From<usize, i16>);
  static_assert(!sus::construct::From<usize, i32>);
  static_assert(!sus::construct::From<usize, i64>);
  static_assert(!sus::construct::From<usize, isize>);
  static_assert(sus::construct::From<usize, u8>);
  static_assert(sus::construct::From<usize, u16>);
  static_assert(sus::construct::From<usize, u32>);
  static_assert(sus::construct::From<usize, u64>);
  static_assert(sus::construct::From<usize, usize>);
  static_assert(sizeof(uptr) <= sizeof(usize) == sus::construct::From<usize, uptr>);
  static_assert(sus::construct::TryFrom<usize, i8>);
  static_assert(sus::construct::TryFrom<usize, i16>);
  static_assert(sus::construct::TryFrom<usize, i32>);
  static_assert(sus::construct::TryFrom<usize, i64>);
  static_assert(sus::construct::TryFrom<usize, isize>);
  static_assert(sus::construct::TryFrom<usize, u8>);
  static_assert(sus::construct::TryFrom<usize, u16>);
  static_assert(sus::construct::TryFrom<usize, u32>);
  static_assert(sus::construct::TryFrom<usize, u64>);
  static_assert(sus::construct::TryFrom<usize, usize>);
  static_assert(sus::construct::TryFrom<usize, uptr>);

  EXPECT_EQ(usize::from(2_u8), 2_usize);
  EXPECT_EQ(usize::from(2_u16), 2_usize);
  EXPECT_EQ(usize::from(2_u32), 2_usize);
  EXPECT_EQ(usize::from(2_u64), 2_usize);
  EXPECT_EQ(usize::from(2_usize), 2_usize);

  EXPECT_EQ(usize::try_from(2_i8).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(2_i16).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(2_i32).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(2_i64).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(2_isize).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(2_u8).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(2_u16).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(2_u32).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(2_u64).unwrap(), 2_usize);
  EXPECT_EQ(usize::try_from(2_usize).unwrap(), 2_usize);

  if constexpr (sizeof(usize) == sizeof(u32)) {
    EXPECT_TRUE(usize::try_from(u32::MAX).is_ok());
    EXPECT_TRUE(usize::try_from(u64::MAX).is_err());
  }
  EXPECT_TRUE(usize::try_from(i64::MIN).is_err());
  EXPECT_TRUE(usize::try_from(i64::MAX).is_ok());
}

TEST(usize, InvokeEverything) {
  auto i = 10_usize, j = 11_usize;
  auto s = 3_isize;
  auto a = sus::Array<u8, sizeof(usize)>();

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

  [[maybe_unused]] bool b = i == j;
  [[maybe_unused]] auto z = i >= j;
}

TEST(usize, PointerArithmetic) {
  i32 x[] = {1, 2, 3, 4, 5, 6, 7, 8};
  i32* p = &x[0];
  EXPECT_EQ(*p, 1);
  p = p + 1_usize;
  EXPECT_EQ(*p, 2);
  p += 3_usize;
  EXPECT_EQ(*p, 5);
  p = p - 1_usize;
  EXPECT_EQ(*p, 4);
  p -= 3_usize;
  EXPECT_EQ(*p, 1);

  // operator+= returns the lhs.
  p = (p += size_t{1}) + size_t{1};
  EXPECT_EQ(*p, 3);
  p = (p += 1_usize) + 1_usize;
  EXPECT_EQ(*p, 5);

  // operator-= returns the lhs.
  p = (p -= size_t{1}) - size_t{1};
  EXPECT_EQ(*p, 3);
  p = (p -= 1_usize) - 1_usize;
  EXPECT_EQ(*p, 1);
}

TEST(usize, fmt) {
  static_assert(fmt::is_formattable<usize, char>::value);
  EXPECT_EQ(fmt::format("{}", 1234567_usize), "1234567");
  EXPECT_EQ(fmt::format("{:#x}", 1234567_usize), "0x12d687");
}

}  // namespace
