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

static_assert(std::is_signed_v<decltype(isize::primitive_value)>);
static_assert(sizeof(decltype(isize::primitive_value)) == sizeof(void*));
static_assert(sizeof(isize) == sizeof(decltype(isize::primitive_value)));

static_assert(sus::mem::Copy<isize>);
static_assert(sus::mem::TrivialCopy<isize>);
static_assert(sus::mem::Clone<isize>);
static_assert(sus::mem::relocate_by_memcpy<isize>);
static_assert(sus::mem::Move<isize>);

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

// isize::MAX
static_assert(sizeof(isize) != sizeof(i64)
                  ? isize::MAX.primitive_value == 0x7fffffff
                  : isize::MAX.primitive_value == 0x7fffffff'ffffffff);
template <class T>
concept MaxInRange = requires {
  {
    sizeof(isize) != sizeof(i64) ? 0x7fffffff_isize : 0x7fffffff'ffffffff_isize
  } -> std::same_as<T>;
  {
    isize(sizeof(isize) != sizeof(i64) ? 0x7fffffff : 0x7fffffff'ffffffff)
  } -> std::same_as<T>;
};
static_assert(MaxInRange<isize>);

// std hashing
static_assert(std::same_as<decltype(std::hash<isize>()(0_isize)), size_t>);
static_assert(
    std::same_as<decltype(std::equal_to<isize>()(0_isize, 1_isize)), bool>);

TEST(isize, Traits) {
  static_assert(sus::iter::__private::Step<isize>);

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

  static_assert(sus::ops::Ord<isize, int8_t>);
  static_assert(sus::ops::Ord<isize, int16_t>);
  static_assert(sus::ops::Ord<isize, int32_t>);
  static_assert(sus::ops::Ord<isize, int64_t>);
  static_assert(sus::ops::Ord<isize, i8>);
  static_assert(sus::ops::Ord<isize, i16>);
  static_assert(sus::ops::Ord<isize, i32>);
  static_assert(sus::ops::Ord<isize, i64>);
  static_assert(sus::ops::Ord<isize, isize>);
  static_assert(1_isize >= 1_isize);
  static_assert(2_isize > 1_isize);
  static_assert(1_isize <= 1_isize);
  static_assert(1_isize < 2_isize);
  static_assert(sus::ops::Eq<isize, int8_t>);
  static_assert(sus::ops::Eq<isize, int16_t>);
  static_assert(sus::ops::Eq<isize, int32_t>);
  static_assert(sus::ops::Eq<isize, int64_t>);
  static_assert(sus::ops::Eq<isize, i8>);
  static_assert(sus::ops::Eq<isize, i16>);
  static_assert(sus::ops::Eq<isize, i32>);
  static_assert(sus::ops::Eq<isize, i64>);
  static_assert(sus::ops::Eq<isize, isize>);
  static_assert(1_isize == 1_isize);
  static_assert(!(1_isize == 2_isize));
  static_assert(1_isize != 2_isize);
  static_assert(!(1_isize != 1_isize));

  // Verify constexpr.
  [[maybe_unused]] constexpr isize c =
      1_isize + 2_isize - 3_isize * 4_isize / 5_isize % 6_isize & 7_isize |
      8_isize ^ -9_isize;
  [[maybe_unused]] constexpr std::strong_ordering o = 2_isize <=> 3_isize;
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
  constexpr auto max = isize::MAX;
  static_assert(std::same_as<decltype(max), const isize>);
  if constexpr (sizeof(isize) != sizeof(i64))
    EXPECT_EQ(max.primitive_value, 0x7fffffff);
  else
    EXPECT_EQ(max.primitive_value, 0x7fffffff'ffffffff);
  constexpr auto min = isize::MIN;
  static_assert(std::same_as<decltype(min), const isize>);
  if constexpr (sizeof(isize) != sizeof(i64))
    EXPECT_EQ(min.primitive_value, -0x7fffffff - 1);
  else
    EXPECT_EQ(min.primitive_value, -0x7fffffff'ffffffff - 1);
  constexpr auto bits = isize::BITS;
  static_assert(std::same_as<decltype(bits), const u32>);
  if constexpr (sizeof(isize) != sizeof(i64))
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

TEST(isize, CompileTimeConversion) {
  using Self = isize;

  static_assert(is_constexpr_convertible<0_i8, Self>(0));
  static_assert(is_constexpr_convertible<0_i16, Self>(0));
  static_assert(is_constexpr_convertible<0_i32, Self>(0));
  static_assert(sizeof(Self) >= sizeof(i64) ==
                is_constexpr_convertible<0_i64, Self>(0));
  static_assert(is_constexpr_convertible<0_isize, Self>(0));
  static_assert(is_constexpr_convertible<int8_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<int16_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<int32_t{0}, Self>(0));
  static_assert(sizeof(Self) >= sizeof(64) ==
                is_constexpr_convertible<int64_t{0}, Self>(0));
  static_assert(is_constexpr_convertible<ptrdiff_t{0}, Self>(0));

  static_assert(IsConstexprAssignable<0_i8, Self>);
  static_assert(IsConstexprAssignable<0_i16, Self>);
  static_assert(IsConstexprAssignable<0_i32, Self>);
  static_assert(sizeof(Self) >= sizeof(i64) ==
                IsConstexprAssignable<0_i64, Self>);
  static_assert(IsConstexprAssignable<0_isize, Self>);
  static_assert(IsConstexprAssignable<int8_t{0}, Self>);
  static_assert(IsConstexprAssignable<int16_t{0}, Self>);
  static_assert(IsConstexprAssignable<int32_t{0}, Self>);
  static_assert(sizeof(Self) >= sizeof(i64) ==
                IsConstexprAssignable<int64_t{0}, Self>);
  static_assert(IsConstexprAssignable<ptrdiff_t{0}, Self>);

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

TEST(isize, CompileTimeConversionEnum) {
  using Self = isize;

  static_assert(is_constexpr_convertible<ENUM(, int8_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, int16_t)::X, Self>(0));
  static_assert(is_constexpr_convertible<ENUM(, int32_t)::X, Self>(0));
  static_assert(sizeof(Self) >= sizeof(u64) ==
                is_constexpr_convertible<ENUM(, int64_t)::X, Self>(0));

  // Conversion from enum class is explicit (constructible) instead of implicit
  // (convertible).
  static_assert(!is_constexpr_convertible<ENUM(class, int8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, int16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, int32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, int64_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int8_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int16_t)::X, Self>(0));
  static_assert(is_constexpr_constructible<ENUM(class, int32_t)::X, Self>(0));
  static_assert(sizeof(Self) >= sizeof(i64) ==
                is_constexpr_constructible<ENUM(class, int64_t)::X, Self>(0));
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
  static_assert(IsExplicitlyConvertible<i8, uint32_t>);
  static_assert(IsExplicitlyConvertible<i64, uint64_t>);
  static_assert(IsExplicitlyConvertible<i64, size_t>);
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
  static_assert(sus::construct::TryFrom<i32, char>);
  static_assert(sus::construct::TryFrom<i32, size_t>);
  static_assert(sus::construct::TryFrom<i32, int8_t>);
  static_assert(sus::construct::TryFrom<i32, int16_t>);
  static_assert(sus::construct::TryFrom<i32, int32_t>);
  static_assert(sus::construct::TryFrom<i32, int64_t>);
  static_assert(sus::construct::TryFrom<i32, uint8_t>);
  static_assert(sus::construct::TryFrom<i32, uint16_t>);
  static_assert(sus::construct::TryFrom<i32, uint32_t>);
  static_assert(sus::construct::TryFrom<i32, uint64_t>);

  static_assert(sus::construct::From<isize, ENUM(, char)>);
  static_assert(sus::construct::From<isize, ENUM(, size_t)>);
  static_assert(sus::construct::From<isize, ENUM(, int8_t)>);
  static_assert(sus::construct::From<isize, ENUM(, int16_t)>);
  static_assert(sus::construct::From<isize, ENUM(, int32_t)>);
  static_assert(sus::construct::From<isize, ENUM(, int64_t)>);
  static_assert(sus::construct::From<isize, ENUM(, uint8_t)>);
  static_assert(sus::construct::From<isize, ENUM(, uint16_t)>);
  static_assert(sus::construct::From<isize, ENUM(, uint32_t)>);
  static_assert(sus::construct::From<isize, ENUM(, uint64_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, char)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, size_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, int8_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, int16_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, int32_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, int64_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, uint8_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, uint16_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, uint32_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(, uint64_t)>);

  static_assert(sus::construct::From<isize, ENUM(class, char)>);
  static_assert(sus::construct::From<isize, ENUM(class, size_t)>);
  static_assert(sus::construct::From<isize, ENUM(class, int8_t)>);
  static_assert(sus::construct::From<isize, ENUM(class, int16_t)>);
  static_assert(sus::construct::From<isize, ENUM(class, int32_t)>);
  static_assert(sus::construct::From<isize, ENUM(class, int64_t)>);
  static_assert(sus::construct::From<isize, ENUM(class, uint8_t)>);
  static_assert(sus::construct::From<isize, ENUM(class, uint16_t)>);
  static_assert(sus::construct::From<isize, ENUM(class, uint32_t)>);
  static_assert(sus::construct::From<isize, ENUM(class, uint64_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, char)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, size_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, int8_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, int16_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, int32_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, int64_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, uint8_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, uint16_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, uint32_t)>);
  static_assert(sus::construct::TryFrom<isize, ENUM(class, uint64_t)>);

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

  EXPECT_EQ(isize::try_from(char{2}).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(size_t{2}).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(int8_t{2}).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(int16_t{2}).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(int32_t{2}).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(int64_t{2}).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(uint8_t{2}).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(uint16_t{2}).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(uint32_t{2}).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(uint64_t{2}).unwrap(), 2_isize);

  if constexpr (sizeof(isize) == sizeof(i32)) {
    EXPECT_TRUE(isize::try_from(uint32_t{u32::MAX}).is_err());
    EXPECT_TRUE(isize::try_from(int64_t{i64::MIN}).is_err());
    EXPECT_TRUE(isize::try_from(int64_t{i64::MAX}).is_err());
  }
  EXPECT_TRUE(isize::try_from(uint64_t{u64::MAX}).is_err());

  EXPECT_EQ(isize::from(ENUM(, char)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(, size_t)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(, int8_t)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(, int16_t)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(, int32_t)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(, int64_t)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(, uint8_t)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(, uint16_t)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(, uint32_t)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(, uint64_t)::Z), 2_isize);
  EXPECT_EQ(isize::from(ENUM(class, uint64_t)::Z), 2_isize);

  EXPECT_EQ(isize::try_from(ENUM(, char)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(, size_t)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(, int8_t)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(, int16_t)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(, int32_t)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(, int64_t)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(, uint8_t)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(, uint16_t)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(, uint32_t)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(, uint64_t)::Z).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(ENUM(class, uint64_t)::Z).unwrap(), 2_isize);

  if constexpr (sizeof(isize) == sizeof(i32)) {
    EXPECT_TRUE(isize::try_from(ENUM(, uint32_t)::MAX).is_err());
    EXPECT_TRUE(isize::try_from(ENUM(, int64_t)::MIN).is_err());
    EXPECT_TRUE(isize::try_from(ENUM(, int64_t)::MAX).is_err());
    EXPECT_TRUE(isize::try_from(ENUM(class, int64_t)::MAX).is_err());
  }
  EXPECT_TRUE(isize::try_from(ENUM(, uint64_t)::MAX).is_err());
  EXPECT_TRUE(isize::try_from(ENUM(class, uint64_t)::MAX).is_err());

  EXPECT_EQ(isize::from_unchecked(unsafe_fn, char{2}), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, size_t{2}), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, int8_t{2}), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, int16_t{2}), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, int32_t{2}), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, int64_t{2}), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, uint8_t{2}), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, uint16_t{2}), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, uint32_t{2}), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, uint64_t{2}), 2_isize);

  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, char)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, size_t)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, int8_t)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, int16_t)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, int32_t)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, int64_t)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, uint8_t)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, uint16_t)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, uint32_t)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(, uint64_t)::Z), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, ENUM(class, uint64_t)::Z),
            2_isize);

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
  static_assert(sus::construct::TryFrom<i32, i8>);
  static_assert(sus::construct::TryFrom<i32, i16>);
  static_assert(sus::construct::TryFrom<i32, i32>);
  static_assert(sus::construct::TryFrom<i32, i64>);
  static_assert(sus::construct::TryFrom<i32, isize>);
  static_assert(sus::construct::TryFrom<i32, u8>);
  static_assert(sus::construct::TryFrom<i32, u16>);
  static_assert(sus::construct::TryFrom<i32, u32>);
  static_assert(sus::construct::TryFrom<i32, u64>);
  static_assert(sus::construct::TryFrom<i32, usize>);

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

  EXPECT_EQ(isize::try_from(2_i8).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(2_i16).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(2_i32).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(2_i64).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(2_isize).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(2_u8).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(2_u16).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(2_u32).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(2_u64).unwrap(), 2_isize);
  EXPECT_EQ(isize::try_from(2_usize).unwrap(), 2_isize);

  if constexpr (sizeof(isize) == sizeof(i32)) {
    EXPECT_TRUE(isize::try_from(u32::MAX).is_err());
    EXPECT_TRUE(isize::try_from(u64::MIN).is_err());
    EXPECT_TRUE(isize::try_from(u64::MAX).is_err());
  }
  EXPECT_TRUE(isize::try_from(u64::MAX).is_err());

  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_i8), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_i16), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_i32), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_i64), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_isize), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_u8), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_u16), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_u32), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_u64), 2_isize);
  EXPECT_EQ(isize::from_unchecked(unsafe_fn, 2_usize), 2_isize);
}

TEST(isizeDeathTest, FromOutOfRange) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(
      {
        auto x = isize::from(uint64_t{0xffff'ffff'ffff'ffff});
        ensure_use(&x);
      },
      "");

  EXPECT_DEATH(
      {
        auto x = isize::from(ENUM(, uint64_t)::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = isize::from(ENUM(class, uint64_t)::MAX);
        ensure_use(&x);
      },
      "");

  bool not64 = sizeof(isize) != sizeof(i64);
  if (not64) {
    EXPECT_DEATH(
        {
          auto x = isize::from(ENUM(, int64_t)::MIN);
          ensure_use(&x);
        },
        "");
    EXPECT_DEATH(
        {
          auto x = isize::from(ENUM(, int64_t)::MAX);
          ensure_use(&x);
        },
        "");
    EXPECT_DEATH(
        {
          auto x = isize::from(ENUM(, uint32_t)::MAX);
          ensure_use(&x);
        },
        "");
    EXPECT_DEATH(
        {
          auto x = isize::from(ENUM(class, int64_t)::MIN);
          ensure_use(&x);
        },
        "");
    EXPECT_DEATH(
        {
          auto x = isize::from(ENUM(class, int64_t)::MAX);
          ensure_use(&x);
        },
        "");
    EXPECT_DEATH(
        {
          auto x = isize::from(ENUM(class, uint32_t)::MAX);
          ensure_use(&x);
        },
        "");

    EXPECT_DEATH(
        {
          auto x = isize::from(i64::MIN);
          ensure_use(&x);
        },
        "");
    EXPECT_DEATH(
        {
          auto x = isize::from(i64::MAX);
          ensure_use(&x);
        },
        "");
    EXPECT_DEATH(
        {
          auto x = isize::from(u32::MAX);
          ensure_use(&x);
        },
        "");
  }
  EXPECT_DEATH(
      {
        auto x = isize::from(u64::MAX);
        ensure_use(&x);
      },
      "");
  EXPECT_DEATH(
      {
        auto x = isize::from(usize::MAX);
        ensure_use(&x);
      },
      "");
#endif
}

TEST(isize, InvokeEverything) {
  auto i = 10_isize, j = 11_isize;
  auto s = 3_usize;
  auto a = sus::Array<u8, sizeof(isize)>();

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

  [[maybe_unused]] bool b = i == j;
  [[maybe_unused]] auto z = i >= j;
}

TEST(isize, PointerArithmetic) {
  i32 x[] = {1, 2, 3, 4, 5, 6, 7, 8};
  i32* p = &x[0];
  EXPECT_EQ(*p, 1);
  p = p + 1_isize;
  EXPECT_EQ(*p, 2);
  p += 3_isize;
  EXPECT_EQ(*p, 5);
  p = p - 1_isize;
  EXPECT_EQ(*p, 4);
  p -= 3_isize;
  EXPECT_EQ(*p, 1);

  // operator+= returns the lhs.
  p = (p += size_t{1}) + size_t{1};
  EXPECT_EQ(*p, 3);
  p = (p += 1_isize) + 1_isize;
  EXPECT_EQ(*p, 5);

  // operator-= returns the lhs.
  p = (p -= size_t{1}) - size_t{1};
  EXPECT_EQ(*p, 3);
  p = (p -= 1_isize) - 1_isize;
  EXPECT_EQ(*p, 1);

  // Negative offsets.
  EXPECT_EQ(*p, 1);
  p = p - -1_isize;
  EXPECT_EQ(*p, 2);
  p -= -3_isize;
  EXPECT_EQ(*p, 5);
  p = p + -1_isize;
  EXPECT_EQ(*p, 4);
  p += -3_isize;
  EXPECT_EQ(*p, 1);
}

TEST(isize, fmt) {
  static_assert(fmt::is_formattable<isize, char>::value);
  EXPECT_EQ(fmt::format("{}", -654321_isize), "-654321");
  EXPECT_EQ(fmt::format("{}", 1234567_isize), "1234567");
  EXPECT_EQ(fmt::format("{:+#x}", 1234567_isize), "+0x12d687");
}

}  // namespace
