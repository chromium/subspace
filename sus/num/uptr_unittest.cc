// Copyright 2023 Google LLC
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
#include "sus/cmp/eq.h"
#include "sus/cmp/ord.h"
#include "sus/collections/array.h"
#include "sus/construct/into.h"
#include "sus/iter/__private/step.h"
#include "sus/num/num_concepts.h"
#include "sus/num/signed_integer.h"
#include "sus/num/unsigned_integer.h"
#include "sus/prelude.h"
#include "sus/tuple/tuple.h"

namespace {

using sus::None;
using sus::Option;
using sus::Tuple;

static_assert(!std::is_signed_v<decltype(uptr::primitive_value)>);
static_assert(sizeof(decltype(uptr::primitive_value)) == sizeof(void*));
static_assert(sizeof(uptr) == sizeof(decltype(uptr::primitive_value)));

static_assert(sus::mem::Copy<uptr>);
static_assert(sus::mem::TrivialCopy<uptr>);
static_assert(sus::mem::Clone<uptr>);
static_assert(sus::mem::relocate_by_memcpy<uptr>);
static_assert(sus::mem::Move<uptr>);

namespace behaviour {
using T = uptr;
using From = decltype(uptr::primitive_value);
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

// uptr::MAX_BIT_PATTERN
static_assert(uptr::MAX_BIT_PATTERN.primitive_value == ~uintptr_t{0});
template <class T>
concept MaxInRange = requires {
  { uptr(~uintptr_t{0u}) } -> std::same_as<T>;
};
static_assert(MaxInRange<uptr>);

// std hashing
static_assert(std::same_as<decltype(std::hash<uptr>()(uptr())), size_t>);
static_assert(
    std::same_as<decltype(std::equal_to<uptr>()(uptr(), uptr())), bool>);

TEST(uptr, Traits) {
  static_assert(sus::iter::__private::Step<uptr>);

  // ** Unsigned only
  static_assert(!sus::num::Neg<uptr>);

  static_assert(sus::num::Add<uptr, uptr>);
  static_assert(sus::num::AddAssign<uptr, uptr>);
  static_assert(sus::num::Sub<uptr, uptr>);
  static_assert(sus::num::SubAssign<uptr, uptr>);
  static_assert(sus::num::Mul<uptr, uptr>);
  static_assert(sus::num::MulAssign<uptr, uptr>);
  static_assert(sus::num::Div<uptr, uptr>);
  static_assert(sus::num::DivAssign<uptr, uptr>);
  static_assert(sus::num::Rem<uptr, uptr>);
  static_assert(sus::num::RemAssign<uptr, uptr>);
  static_assert(sus::num::BitAnd<uptr, uptr>);
  static_assert(sus::num::BitAndAssign<uptr, uptr>);
  static_assert(sus::num::BitOr<uptr, uptr>);
  static_assert(sus::num::BitOrAssign<uptr, uptr>);
  static_assert(sus::num::BitXor<uptr, uptr>);
  static_assert(sus::num::BitXorAssign<uptr, uptr>);
  static_assert(sus::num::BitNot<uptr>);
  static_assert(sus::num::Shl<uptr>);
  static_assert(sus::num::ShlAssign<uptr>);
  static_assert(sus::num::Shr<uptr>);
  static_assert(sus::num::ShrAssign<uptr>);

  static_assert(sus::cmp::StrongOrd<uptr, uint8_t>);
  static_assert(sus::cmp::StrongOrd<uptr, uint16_t>);
  static_assert(sus::cmp::StrongOrd<uptr, uint64_t>);
  static_assert(sus::cmp::StrongOrd<uptr, uint64_t>);
  static_assert(sus::cmp::StrongOrd<uptr, size_t>);
  static_assert(sus::cmp::StrongOrd<uptr, u8>);
  static_assert(sus::cmp::StrongOrd<uptr, u16>);
  static_assert(sus::cmp::StrongOrd<uptr, u32>);
  static_assert(sus::cmp::StrongOrd<uptr, u64>);
  static_assert(sus::cmp::StrongOrd<uptr, uptr>);
  static_assert(uptr(uintptr_t{1}) >= uptr(uintptr_t{1}));
  static_assert(uptr(uintptr_t{2}) > uptr(uintptr_t{1}));
  static_assert(uptr(uintptr_t{1}) <= uptr(uintptr_t{1}));
  static_assert(uptr(uintptr_t{1}) < uptr(uintptr_t{2}));
  static_assert(sus::cmp::Eq<uptr, uint8_t>);
  static_assert(sus::cmp::Eq<uptr, uint16_t>);
  static_assert(sus::cmp::Eq<uptr, uint64_t>);
  static_assert(sus::cmp::Eq<uptr, uint64_t>);
  static_assert(sus::cmp::Eq<uptr, size_t>);
  static_assert(sus::cmp::Eq<uptr, u8>);
  static_assert(sus::cmp::Eq<uptr, u16>);
  static_assert(sus::cmp::Eq<uptr, u32>);
  static_assert(sus::cmp::Eq<uptr, u64>);
  static_assert(sus::cmp::Eq<uptr, uptr>);
  static_assert(uptr(uintptr_t{1}) == uptr(uintptr_t{1}));
  static_assert(!(uptr(uintptr_t{1}) == uptr(uintptr_t{2})));
  static_assert(uptr(uintptr_t{1}) != uptr(uintptr_t{2}));
  static_assert(!(uptr(uintptr_t{1}) != uptr(uintptr_t{1})));

  // Verify constexpr.
  [[maybe_unused]] constexpr uptr e = []() {
    uptr a = uptr(uintptr_t{1}) + uptr(uintptr_t{1}) -
                     uptr(uintptr_t{1}) * uptr(uintptr_t{1}) /
                         uptr(uintptr_t{1}) % uptr(uintptr_t{1}) &
                 uptr(uintptr_t{1}) |
             uptr(uintptr_t{1}) ^ uptr(uintptr_t{1});
    [[maybe_unused]] bool b = uptr(uintptr_t{1}) == uptr(uintptr_t{2});
    [[maybe_unused]] std::strong_ordering c =
        uptr(uintptr_t{1}) <=> uptr(uintptr_t{2});
    [[maybe_unused]] uptr d = a << 1_u32;
    [[maybe_unused]] uptr e = a >> 1_u32;
    a += uptr(uintptr_t{1});
    a -= uptr(uintptr_t{1});
    a *= uptr(uintptr_t{1});
    a /= uptr(uintptr_t{1});
    a %= uptr(uintptr_t{1});
    a &= uptr(uintptr_t{1});
    a |= uptr(uintptr_t{1});
    a ^= uptr(uintptr_t{1});
    a <<= 1_u32;
    a >>= 1_u32;
    return a;
  }();
}

TEST(uptr, Constants) {
  constexpr auto max = uptr::MAX_BIT_PATTERN;
  static_assert(std::same_as<decltype(max), const uptr>);
  EXPECT_EQ(max.primitive_value, ~uintptr_t{0});
  constexpr auto min = uptr::MIN;
  static_assert(std::same_as<decltype(min), const uptr>);
  EXPECT_EQ(min.primitive_value, 0u);
  constexpr auto bits = uptr::BITS;
  static_assert(std::same_as<decltype(bits), const u32>);
  if constexpr (sizeof(uptr) == 4)
    EXPECT_EQ(bits, 32u);
  else if constexpr (sizeof(uptr) == 8)
    EXPECT_EQ(bits, 64u);
  else
    EXPECT_EQ(bits, 128u);
}

template <class From, class To>
concept IsImplicitlyConvertible =
    std::is_convertible_v<From, To> && std::is_nothrow_assignable_v<To&, From>;
template <class From, class To>
concept IsExplicitlyConvertible =
    std::constructible_from<To, From> && !std::is_convertible_v<From, To> &&
    !std::is_assignable_v<To&, From>;
template <class From, class To>
concept NotConvertible =
    !std::constructible_from<To, From> && !std::is_convertible_v<From, To> &&
    !std::is_assignable_v<To&, From>;

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

TEST(uptr, CompileTimeConversion) {
  using Self = uptr;

  // uptr requires explicit construction.
  static_assert(!std::is_convertible_v<Self, i32*>);
  static_assert(std::is_constructible_v<Self, i32*>);
  static_assert(!is_constexpr_convertible<uintptr_t{0}, Self>(0));
  static_assert(is_constexpr_constructible<uintptr_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<nullptr, Self>(0));
  static_assert(is_constexpr_constructible<nullptr, Self>(0));

  static_assert(!is_constexpr_convertible<0_u8, Self>(0));
  static_assert(!is_constexpr_constructible<0_u8, Self>(0));
  static_assert(!is_constexpr_convertible<0_u16, Self>(0));
  static_assert(!is_constexpr_constructible<0_u16, Self>(0));
  static_assert(!is_constexpr_convertible<0_u32, Self>(0));
  static_assert(sizeof(Self) == sizeof(u32) ==
                is_constexpr_constructible<0_u32, Self>(0));
  static_assert(!is_constexpr_convertible<0_u64, Self>(0));
  static_assert(sizeof(Self) == sizeof(u64) ==
                is_constexpr_constructible<0_u64, Self>(0));
  static_assert(!is_constexpr_convertible<0_usize, Self>(0));
  static_assert(sizeof(Self) == sizeof(usize) ==
                is_constexpr_constructible<0_usize, Self>(0));

  static_assert(!is_constexpr_convertible<uint8_t{0}, Self>(0));
  static_assert(!is_constexpr_constructible<uint8_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<uint16_t{0}, Self>(0));
  static_assert(!is_constexpr_constructible<uint16_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<uint32_t{0}, Self>(0));
  static_assert(sizeof(Self) == sizeof(uint32_t) ==
                is_constexpr_constructible<uint32_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<uint64_t{0}, Self>(0));
  static_assert(sizeof(Self) == sizeof(uint64_t) ==
                is_constexpr_constructible<uint64_t{0}, Self>(0));
  static_assert(!is_constexpr_convertible<size_t{0}, Self>(0));
  static_assert(sizeof(Self) == sizeof(size_t) ==
                is_constexpr_constructible<size_t{0}, Self>(0));

  static_assert(std::is_assignable_v<Self, i32*>);
  static_assert(IsConstexprAssignable<uintptr_t{0}, Self>);
  static_assert(IsConstexprAssignable<nullptr, Self>);

  static_assert(!IsConstexprAssignable<0_u8, Self>);
  static_assert(!IsConstexprAssignable<0_u16, Self>);
  static_assert(sizeof(Self) == sizeof(u32) ==
                IsConstexprAssignable<0_u32, Self>);
  static_assert(sizeof(Self) == sizeof(u64) ==
                IsConstexprAssignable<0_u64, Self>);
  static_assert(sizeof(Self) == sizeof(usize) ==
                IsConstexprAssignable<0_usize, Self>);
  static_assert(!IsConstexprAssignable<uint8_t{0}, Self>);
  static_assert(!IsConstexprAssignable<uint16_t{0}, Self>);
  static_assert(sizeof(Self) == sizeof(uint32_t) ==
                IsConstexprAssignable<uint32_t{0}, Self>);
  static_assert(sizeof(Self) == sizeof(uint64_t) ==
                IsConstexprAssignable<uint64_t{0}, Self>);
  static_assert(sizeof(Self) == sizeof(size_t) ==
                IsConstexprAssignable<size_t{0}, Self>);
}

TEST(uptr, CompileTimeConversionEnum) {
  using Self = uptr;

  // Enums don't convert to pointer integers.
  static_assert(!is_constexpr_convertible<ENUM(, uint8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, uint16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(, uint32_t)::X, Self>(0));
  static_assert(sizeof(Self) >= sizeof(u64) ==
                !is_constexpr_convertible<ENUM(, uint64_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(!is_constexpr_convertible<ENUM(class, uint64_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, uint8_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, uint16_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, uint32_t)::X, Self>(0));
  static_assert(!is_constexpr_constructible<ENUM(class, uint64_t)::X, Self>(0));
}

TEST(uptr, ToPrimitive) {
  static_assert(NotConvertible<uptr, int8_t>);
  static_assert(NotConvertible<uptr, int16_t>);
  static_assert(NotConvertible<uptr, int32_t>);
  static_assert(NotConvertible<uptr, int64_t>);
  static_assert(NotConvertible<uptr, uint8_t>);
  static_assert(NotConvertible<uptr, uint16_t>);

  // If uint32_t matches with uintptr_t in std::same_as<> then it will convert,
  // and it's up to the implementation if this is true. Same for uint64_t, so
  // testing them is not super meaningful.

  static_assert(IsImplicitlyConvertible<uptr, uintptr_t>);
}

template <class Sus, class Prim>
concept CanOperator = requires(Sus sus, Prim prim) {
  { sus + prim } -> std::same_as<Sus>;
  { sus - prim } -> std::same_as<Sus>;
  { sus* prim } -> std::same_as<Sus>;
  { sus / prim } -> std::same_as<Sus>;
  { sus % prim } -> std::same_as<Sus>;
  { sus& prim } -> std::same_as<Sus>;
  { sus | prim } -> std::same_as<Sus>;
  { sus ^ prim } -> std::same_as<Sus>;
  { prim + sus } -> std::same_as<Sus>;
  { prim - sus } -> std::same_as<Sus>;
  { prim* sus } -> std::same_as<Sus>;
  { prim / sus } -> std::same_as<Sus>;
  { prim % sus } -> std::same_as<Sus>;
  { prim& sus } -> std::same_as<Sus>;
  { prim | sus } -> std::same_as<Sus>;
  { prim ^ sus } -> std::same_as<Sus>;
  { sus += prim };
  { sus -= prim };
  { sus *= prim };
  { sus /= prim };
  { sus %= prim };
  { sus &= prim };
  { sus |= prim };
  { sus ^= prim };
};

// clang-format off
template <class Sus, class Prim>
concept CantOperator =
  !requires(Sus sus, Prim prim) { { sus + prim }; } &&
  !requires(Sus sus, Prim prim) { { sus - prim }; } &&
  !requires(Sus sus, Prim prim) { { sus* prim }; } &&
  !requires(Sus sus, Prim prim) { { sus / prim }; } &&
  !requires(Sus sus, Prim prim) { { sus % prim }; } &&
  !requires(Sus sus, Prim prim) { { sus& prim }; } &&
  !requires(Sus sus, Prim prim) { { sus | prim }; } &&
  !requires(Sus sus, Prim prim) { { sus ^ prim }; } &&
  !requires(Sus sus, Prim prim) { { prim + sus }; } &&
  !requires(Sus sus, Prim prim) { { prim - sus }; } &&
  !requires(Sus sus, Prim prim) { { prim* sus }; } &&
  !requires(Sus sus, Prim prim) { { prim / sus }; } &&
  !requires(Sus sus, Prim prim) { { prim % sus }; } &&
  !requires(Sus sus, Prim prim) { { prim& sus }; } &&
  !requires(Sus sus, Prim prim) { { prim | sus }; } &&
  !requires(Sus sus, Prim prim) { { prim ^ sus }; } &&
  !requires(Sus sus, Prim prim) { { sus += prim }; } &&
  !requires(Sus sus, Prim prim) { { sus -= prim }; } &&
  !requires(Sus sus, Prim prim) { { sus *= prim }; } &&
  !requires(Sus sus, Prim prim) { { sus /= prim }; } &&
  !requires(Sus sus, Prim prim) { { sus %= prim }; } &&
  !requires(Sus sus, Prim prim) { { sus &= prim }; } &&
  !requires(Sus sus, Prim prim) { { sus |= prim }; } &&
  !requires(Sus sus, Prim prim) { { sus ^= prim }; };
// clang-format on

template <class Lhs, class Rhs>
concept CanShift = requires(Lhs l, Rhs r) {
  { l << r } -> std::same_as<Lhs>;
  { l >> r } -> std::same_as<Lhs>;
};

// clang-format off
template <class Lhs, class Rhs>
concept CantShift =
  !requires(Lhs l, Rhs r) { { l << r }; } &&
  !requires(Lhs l, Rhs r) { { l >> r }; };
// clang-format on

TEST(uptr, Operators) {
  static_assert(CanOperator<uptr, u8>);
  static_assert(CanOperator<uptr, u16>);
  static_assert(CanOperator<uptr, u32>);
  static_assert(!(sizeof(uptr) >= sizeof(u64)) || CanOperator<uptr, u64>);
  static_assert(!(sizeof(uptr) < sizeof(u64)) || CantOperator<uptr, u64>);

  static_assert(CantOperator<uptr, i8>);
  static_assert(CantOperator<uptr, i16>);
  static_assert(CantOperator<uptr, i32>);
  static_assert(CantOperator<uptr, i64>);
}

TEST(uptr, OperatorsWithPrimitives) {
  static_assert(CanOperator<uptr, uint8_t>);
  static_assert(CanOperator<uptr, uint16_t>);
  static_assert(CanOperator<uptr, uint32_t>);
  static_assert(!(sizeof(uptr) >= sizeof(uint64_t)) ||
                CanOperator<uptr, uint64_t>);
  static_assert(!(sizeof(uptr) < sizeof(uint64_t)) ||
                CantOperator<uptr, uint64_t>);

  static_assert(CantOperator<uptr, int8_t>);
  static_assert(CantOperator<uptr, int16_t>);
  static_assert(CantOperator<uptr, int32_t>);
  static_assert(CantOperator<uptr, int64_t>);

  static_assert(CanOperator<uptr, ENUM(, uint8_t)>);
  static_assert(CanOperator<uptr, ENUM(, uint16_t)>);
  static_assert(CanOperator<uptr, ENUM(, uint32_t)>);
  static_assert(!(sizeof(uptr) >= sizeof(uint64_t)) ||
                CanOperator<uptr, ENUM(, uint64_t)>);
  static_assert(!(sizeof(uptr) < sizeof(uint64_t)) ||
                CantOperator<uptr, ENUM(, uint64_t)>);

  static_assert(CantOperator<uptr, ENUM(, int8_t)>);
  static_assert(CantOperator<uptr, ENUM(, int16_t)>);
  static_assert(CantOperator<uptr, ENUM(, int32_t)>);
  static_assert(CantOperator<uptr, ENUM(, int64_t)>);

  static_assert(CantOperator<uptr, ENUM(class, uint8_t)>);
  static_assert(CantOperator<uptr, ENUM(class, uint16_t)>);
  static_assert(CantOperator<uptr, ENUM(class, uint32_t)>);
  static_assert(CantOperator<uptr, ENUM(class, uint64_t)>);

  static_assert(CantOperator<uptr, ENUM(class, int8_t)>);
  static_assert(CantOperator<uptr, ENUM(class, int16_t)>);
  static_assert(CantOperator<uptr, ENUM(class, int32_t)>);
  static_assert(CantOperator<uptr, ENUM(class, int64_t)>);

  static_assert(CanShift<uptr, uint8_t>);
  static_assert(CanShift<uptr, uint16_t>);
  static_assert(CanShift<uptr, uint32_t>);
  static_assert(CanShift<uptr, uint64_t>);
  static_assert(CanShift<uptr, ENUM(, uint8_t)>);
  static_assert(CanShift<uptr, ENUM(, uint16_t)>);
  static_assert(CanShift<uptr, ENUM(, uint32_t)>);
  static_assert(CanShift<uptr, ENUM(, uint64_t)>);
  static_assert(CantShift<uptr, ENUM(class, uint8_t)>);
  static_assert(CantShift<uptr, ENUM(class, uint16_t)>);
  static_assert(CantShift<uptr, ENUM(class, uint32_t)>);
  static_assert(CantShift<uptr, ENUM(class, uint64_t)>);

  static_assert(CantShift<uptr, int8_t>);

  static_assert(CanShift<int8_t, uptr>);
  static_assert(CanShift<uint8_t, uptr>);
  static_assert(CanShift<ENUM(, int8_t), uptr>);
  static_assert(CanShift<ENUM(, uint8_t), uptr>);
  static_assert(CantShift<ENUM(class, int8_t), uptr>);
  static_assert(CantShift<ENUM(class, uint8_t), uptr>);
}

TEST(uptr, From) {
  static_assert(!sus::construct::From<uptr, char>);
  static_assert(!sus::construct::From<uptr, int8_t>);
  static_assert(!sus::construct::From<uptr, int16_t>);
  static_assert(!sus::construct::From<uptr, int32_t>);
  static_assert(!sus::construct::From<uptr, int64_t>);
  static_assert(!sus::construct::From<uptr, uint8_t>);
  static_assert(!sus::construct::From<uptr, uint16_t>);
  static_assert(sizeof(uptr) == sizeof(uint32_t) ==
                sus::construct::From<uptr, uint32_t>);
  static_assert(sizeof(uptr) == sizeof(uint64_t) ==
                sus::construct::From<uptr, uint64_t>);
  static_assert(sizeof(uptr) == sizeof(size_t) ==
                sus::construct::From<uptr, size_t>);
  static_assert(sus::construct::From<uptr, uintptr_t>);
  static_assert(sus::construct::From<uptr, i32*>);
  static_assert(!sus::construct::TryFrom<uptr, char>);
  static_assert(!sus::construct::TryFrom<uptr, int8_t>);
  static_assert(!sus::construct::TryFrom<uptr, int16_t>);
  static_assert(!sus::construct::TryFrom<uptr, int32_t>);
  static_assert(!sus::construct::TryFrom<uptr, int64_t>);
  static_assert(!sus::construct::TryFrom<uptr, uint8_t>);
  static_assert(!sus::construct::TryFrom<uptr, uint16_t>);
  static_assert(sizeof(uptr) == sizeof(uint32_t) ==
                sus::construct::TryFrom<uptr, uint32_t>);
  static_assert(sizeof(uptr) == sizeof(uint64_t) ==
                sus::construct::TryFrom<uptr, uint64_t>);
  static_assert(sizeof(uptr) == sizeof(size_t) ==
                sus::construct::TryFrom<uptr, size_t>);
  static_assert(sus::construct::TryFrom<uptr, uintptr_t>);
  static_assert(sus::construct::TryFrom<uptr, i32*>);

  static_assert(!sus::construct::From<uptr, ENUM(, char)>);
  static_assert(!sus::construct::From<uptr, ENUM(, size_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(, int8_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(, int16_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(, int32_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(, int64_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(, uint8_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(, uint16_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(, uint32_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(, uint64_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, char)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, size_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, int8_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, int16_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, int32_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, int64_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, uint8_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, uint16_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, uint32_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(, uint64_t)>);

  static_assert(!sus::construct::From<uptr, ENUM(class, char)>);
  static_assert(!sus::construct::From<uptr, ENUM(class, size_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(class, int8_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(class, int16_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(class, int32_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(class, int64_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(class, uint8_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(class, uint16_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(class, uint32_t)>);
  static_assert(!sus::construct::From<uptr, ENUM(class, uint64_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, char)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, size_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, int8_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, int16_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, int32_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, int64_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, uint8_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, uint16_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, uint32_t)>);
  static_assert(!sus::construct::TryFrom<uptr, ENUM(class, uint64_t)>);

  EXPECT_EQ(uptr::from(uintptr_t{2}), uptr().with_addr(2u));
  EXPECT_EQ(uptr::try_from(uintptr_t{2}).unwrap(), uptr().with_addr(2u));

  static_assert(!sus::construct::From<uptr, i8>);
  static_assert(!sus::construct::From<uptr, i16>);
  static_assert(!sus::construct::From<uptr, i32>);
  static_assert(!sus::construct::From<uptr, i64>);
  static_assert(!sus::construct::From<uptr, isize>);
  static_assert(!sus::construct::From<uptr, u8>);
  static_assert(!sus::construct::From<uptr, u16>);
  static_assert(sizeof(uptr) == sizeof(u32) == sus::construct::From<uptr, u32>);
  static_assert(sizeof(uptr) == sizeof(u64) == sus::construct::From<uptr, u64>);
  static_assert(sizeof(uptr) == sizeof(usize) ==
                sus::construct::From<uptr, usize>);
  // TODO: u128
  static_assert(sus::construct::From<uptr, uptr>);
  static_assert(!sus::construct::TryFrom<uptr, i8>);
  static_assert(!sus::construct::TryFrom<uptr, i16>);
  static_assert(!sus::construct::TryFrom<uptr, i32>);
  static_assert(!sus::construct::TryFrom<uptr, i64>);
  static_assert(!sus::construct::TryFrom<uptr, isize>);
  static_assert(!sus::construct::TryFrom<uptr, u8>);
  static_assert(!sus::construct::TryFrom<uptr, u16>);
  static_assert(sizeof(uptr) == sizeof(u32) ==
                sus::construct::TryFrom<uptr, u32>);
  static_assert(sizeof(uptr) == sizeof(u64) ==
                sus::construct::TryFrom<uptr, u64>);
  static_assert(sizeof(uptr) == sizeof(usize) ==
                sus::construct::TryFrom<uptr, usize>);
  // TODO: u128
  static_assert(sus::construct::TryFrom<uptr, uptr>);
}

TEST(uptr, WithAddr) {
  i32 i = 9;
  usize a = 10u;
  auto p = uptr::from(&i).with_addr(a);
  EXPECT_EQ(p & ~0_usize, a);
  if constexpr (sizeof(uptr) > sizeof(usize)) {
    auto high_mask = ~(uptr().with_addr(~0_usize));
    EXPECT_EQ(p & high_mask, uptr(&i) & high_mask);
  }
}

TEST(uptr, Addr) {
  i32 i = 9;
  usize a = 10u;
  auto p = uptr::from(&i).with_addr(a);
  static_assert(std::same_as<decltype(p.addr()), usize>);
  EXPECT_EQ(p.addr(), a);
}

TEST(uptr, InvokeEverything) {
  auto i = uptr().with_addr(10u), j = uptr().with_addr(11u);
  auto a = sus::Array<u8, sizeof(uptr)>();

  (void)i.abs_diff(j);

  (void)i.checked_add(j);
  // Not on uptr: (void)i.checked_add_signed(s);
  (void)i.overflowing_add(j);
  // Not on uptr: (void)i.overflowing_add_signed(s);
  (void)i.saturating_add(j);
  // Not on uptr: (void)i.saturating_add_signed(s);
  (void)i.unchecked_add(unsafe_fn, j);
  (void)i.wrapping_add(j);
  // Not on uptr: (void)i.wrapping_add_signed(s);

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

  (void)i.div_ceil(j);

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

TEST(uptr, PointerArithmetic) {
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

// uptr can't convert from smaller integers, unlike other integer types. So it
// needs special overloads to do arithmetic with them.
TEST(uptr, ArithemticWithSmallerIntegers) {
  const auto i = uptr().with_addr(11u);
  const auto p = uint16_t{11u};
  const auto u = 11_u16;

  static_assert(std::same_as<uptr, decltype(i + i)>);
  static_assert(std::same_as<uptr, decltype(i + p)>);
  static_assert(std::same_as<uptr, decltype(i + u)>);
  static_assert(std::same_as<uptr, decltype(p + i)>);
  static_assert(std::same_as<uptr, decltype(u + i)>);
  EXPECT_EQ(i + p, i + i);
  EXPECT_EQ(i + u, i + i);
  EXPECT_EQ(p + i, i + i);
  EXPECT_EQ(u + i, i + i);

  static_assert(std::same_as<Option<uptr>, decltype(i.checked_add(i))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_add(p))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_add(u))>);
  EXPECT_EQ(i.checked_add(p), i.checked_add(i));
  EXPECT_EQ(i.checked_add(u), i.checked_add(i));

  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_add(i))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_add(p))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_add(u))>);
  EXPECT_EQ(i.overflowing_add(p), i.overflowing_add(i));
  EXPECT_EQ(i.overflowing_add(u), i.overflowing_add(i));

  static_assert(std::same_as<uptr, decltype(i.saturating_add(i))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_add(p))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_add(u))>);
  EXPECT_EQ(i.saturating_add(p), i.saturating_add(i));
  EXPECT_EQ(i.saturating_add(u), i.saturating_add(i));

  static_assert(std::same_as<uptr, decltype(i.unchecked_add(unsafe_fn, i))>);
  static_assert(std::same_as<uptr, decltype(i.unchecked_add(unsafe_fn, p))>);
  static_assert(std::same_as<uptr, decltype(i.unchecked_add(unsafe_fn, u))>);
  EXPECT_EQ(i.unchecked_add(unsafe_fn, p), i.unchecked_add(unsafe_fn, i));
  EXPECT_EQ(i.unchecked_add(unsafe_fn, u), i.unchecked_add(unsafe_fn, i));

  static_assert(std::same_as<uptr, decltype(i.wrapping_add(i))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_add(p))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_add(u))>);
  EXPECT_EQ(i.wrapping_add(p), i.wrapping_add(i));
  EXPECT_EQ(i.wrapping_add(u), i.wrapping_add(i));

  static_assert(std::same_as<uptr, decltype(i / i)>);
  static_assert(std::same_as<uptr, decltype(i / p)>);
  static_assert(std::same_as<uptr, decltype(i / u)>);
  static_assert(std::same_as<uptr, decltype(p / i)>);
  static_assert(std::same_as<uptr, decltype(u / i)>);
  EXPECT_EQ(i / p, i / i);
  EXPECT_EQ(i / u, i / i);
  EXPECT_EQ(p / i, i / i);
  EXPECT_EQ(u / i, i / i);

  static_assert(std::same_as<Option<uptr>, decltype(i.checked_div(i))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_div(p))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_div(u))>);
  EXPECT_EQ(i.checked_div(p), i.checked_div(i));
  EXPECT_EQ(i.checked_div(u), i.checked_div(i));

  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_div(i))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_div(p))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_div(u))>);
  EXPECT_EQ(i.overflowing_div(p), i.overflowing_div(i));
  EXPECT_EQ(i.overflowing_div(u), i.overflowing_div(i));

  static_assert(std::same_as<uptr, decltype(i.saturating_div(i))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_div(p))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_div(u))>);
  EXPECT_EQ(i.saturating_div(p), i.saturating_div(i));
  EXPECT_EQ(i.saturating_div(u), i.saturating_div(i));

  // No unchecked_div.

  static_assert(std::same_as<uptr, decltype(i.wrapping_div(i))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_div(p))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_div(u))>);
  EXPECT_EQ(i.wrapping_div(p), i.wrapping_div(i));
  EXPECT_EQ(i.wrapping_div(u), i.wrapping_div(i));

  static_assert(std::same_as<uptr, decltype(i * i)>);
  static_assert(std::same_as<uptr, decltype(i * p)>);
  static_assert(std::same_as<uptr, decltype(i * u)>);
  static_assert(std::same_as<uptr, decltype(p * i)>);
  static_assert(std::same_as<uptr, decltype(u * i)>);
  EXPECT_EQ(i * p, i * i);
  EXPECT_EQ(i * u, i * i);
  EXPECT_EQ(p * i, i * i);
  EXPECT_EQ(u * i, i * i);

  static_assert(std::same_as<Option<uptr>, decltype(i.checked_mul(i))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_mul(p))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_mul(u))>);
  EXPECT_EQ(i.checked_mul(p), i.checked_mul(i));
  EXPECT_EQ(i.checked_mul(u), i.checked_mul(i));

  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_mul(i))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_mul(p))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_mul(u))>);
  EXPECT_EQ(i.overflowing_mul(p), i.overflowing_mul(i));
  EXPECT_EQ(i.overflowing_mul(u), i.overflowing_mul(i));

  static_assert(std::same_as<uptr, decltype(i.saturating_mul(i))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_mul(p))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_mul(u))>);
  EXPECT_EQ(i.saturating_mul(p), i.saturating_mul(i));
  EXPECT_EQ(i.saturating_mul(u), i.saturating_mul(i));

  static_assert(std::same_as<uptr, decltype(i.unchecked_mul(unsafe_fn, i))>);
  static_assert(std::same_as<uptr, decltype(i.unchecked_mul(unsafe_fn, p))>);
  static_assert(std::same_as<uptr, decltype(i.unchecked_mul(unsafe_fn, u))>);
  EXPECT_EQ(i.unchecked_mul(unsafe_fn, p), i.unchecked_mul(unsafe_fn, i));
  EXPECT_EQ(i.unchecked_mul(unsafe_fn, u), i.unchecked_mul(unsafe_fn, i));

  static_assert(std::same_as<uptr, decltype(i.wrapping_mul(i))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_mul(p))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_mul(u))>);
  EXPECT_EQ(i.wrapping_mul(p), i.wrapping_mul(i));
  EXPECT_EQ(i.wrapping_mul(u), i.wrapping_mul(i));

  static_assert(std::same_as<uptr, decltype(i % i)>);
  static_assert(std::same_as<uptr, decltype(i % p)>);
  static_assert(std::same_as<uptr, decltype(i % u)>);
  static_assert(std::same_as<uptr, decltype(p % i)>);
  static_assert(std::same_as<uptr, decltype(u % i)>);
  EXPECT_EQ(i % p, i % i);
  EXPECT_EQ(i % u, i % i);
  EXPECT_EQ(p % i, i % i);
  EXPECT_EQ(u % i, i % i);

  static_assert(std::same_as<Option<uptr>, decltype(i.checked_rem(i))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_rem(p))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_rem(u))>);
  EXPECT_EQ(i.checked_rem(p), i.checked_rem(i));
  EXPECT_EQ(i.checked_rem(u), i.checked_rem(i));

  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_rem(i))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_rem(p))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_rem(u))>);
  EXPECT_EQ(i.overflowing_rem(p), i.overflowing_rem(i));
  EXPECT_EQ(i.overflowing_rem(u), i.overflowing_rem(i));

  // No saturating_rem.

  // No unchecked_rem.

  static_assert(std::same_as<uptr, decltype(i.wrapping_rem(i))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_rem(p))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_rem(u))>);
  EXPECT_EQ(i.wrapping_rem(p), i.wrapping_rem(i));
  EXPECT_EQ(i.wrapping_rem(u), i.wrapping_rem(i));

  static_assert(std::same_as<uptr, decltype(i << i)>);
  static_assert(std::same_as<uptr, decltype(i - p)>);
  static_assert(std::same_as<uptr, decltype(i - u)>);
  static_assert(std::same_as<uptr, decltype(p - i)>);
  static_assert(std::same_as<uptr, decltype(u - i)>);
  EXPECT_EQ(i - p, i - i);
  EXPECT_EQ(i - u, i - i);
  EXPECT_EQ(p - i, i - i);
  EXPECT_EQ(u - i, i - i);

  static_assert(std::same_as<Option<uptr>, decltype(i.checked_sub(i))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_sub(p))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_sub(u))>);
  EXPECT_EQ(i.checked_sub(p), i.checked_sub(i));
  EXPECT_EQ(i.checked_sub(u), i.checked_sub(i));

  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_sub(i))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_sub(p))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_sub(u))>);
  EXPECT_EQ(i.overflowing_sub(p), i.overflowing_sub(i));
  EXPECT_EQ(i.overflowing_sub(u), i.overflowing_sub(i));

  static_assert(std::same_as<uptr, decltype(i.saturating_sub(i))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_sub(p))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_sub(u))>);
  EXPECT_EQ(i.saturating_sub(p), i.saturating_sub(i));
  EXPECT_EQ(i.saturating_sub(u), i.saturating_sub(i));

  static_assert(std::same_as<uptr, decltype(i.unchecked_sub(unsafe_fn, i))>);
  static_assert(std::same_as<uptr, decltype(i.unchecked_sub(unsafe_fn, p))>);
  static_assert(std::same_as<uptr, decltype(i.unchecked_sub(unsafe_fn, u))>);
  EXPECT_EQ(i.unchecked_sub(unsafe_fn, p), i.unchecked_sub(unsafe_fn, i));
  EXPECT_EQ(i.unchecked_sub(unsafe_fn, u), i.unchecked_sub(unsafe_fn, i));

  static_assert(std::same_as<uptr, decltype(i.wrapping_sub(i))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_sub(p))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_sub(u))>);
  EXPECT_EQ(i.wrapping_sub(p), i.wrapping_sub(i));
  EXPECT_EQ(i.wrapping_sub(u), i.wrapping_sub(i));

  static_assert(std::same_as<uptr, decltype(i - i)>);
  static_assert(std::same_as<uptr, decltype(i - p)>);
  static_assert(std::same_as<uptr, decltype(i - u)>);
  static_assert(std::same_as<uptr, decltype(p - i)>);
  static_assert(std::same_as<uptr, decltype(u - i)>);
  EXPECT_EQ(i - p, i - i);
  EXPECT_EQ(i - u, i - i);
  EXPECT_EQ(p - i, i - i);
  EXPECT_EQ(u - i, i - i);

  static_assert(std::same_as<Option<uptr>, decltype(i.checked_sub(i))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_sub(p))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_sub(u))>);
  EXPECT_EQ(i.checked_sub(p), i.checked_sub(i));
  EXPECT_EQ(i.checked_sub(u), i.checked_sub(i));

  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_sub(i))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_sub(p))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_sub(u))>);
  EXPECT_EQ(i.overflowing_sub(p), i.overflowing_sub(i));
  EXPECT_EQ(i.overflowing_sub(u), i.overflowing_sub(i));

  static_assert(std::same_as<uptr, decltype(i.saturating_sub(i))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_sub(p))>);
  static_assert(std::same_as<uptr, decltype(i.saturating_sub(u))>);
  EXPECT_EQ(i.saturating_sub(p), i.saturating_sub(i));
  EXPECT_EQ(i.saturating_sub(u), i.saturating_sub(i));

  static_assert(std::same_as<uptr, decltype(i.unchecked_sub(unsafe_fn, i))>);
  static_assert(std::same_as<uptr, decltype(i.unchecked_sub(unsafe_fn, p))>);
  static_assert(std::same_as<uptr, decltype(i.unchecked_sub(unsafe_fn, u))>);
  EXPECT_EQ(i.unchecked_sub(unsafe_fn, p), i.unchecked_sub(unsafe_fn, i));
  EXPECT_EQ(i.unchecked_sub(unsafe_fn, u), i.unchecked_sub(unsafe_fn, i));

  static_assert(std::same_as<uptr, decltype(i.wrapping_sub(i))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_sub(p))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_sub(u))>);
  EXPECT_EQ(i.wrapping_sub(p), i.wrapping_sub(i));
  EXPECT_EQ(i.wrapping_sub(u), i.wrapping_sub(i));

  // Euclidean math.

  static_assert(std::same_as<uptr, decltype(i.div_euclid(i))>);
  static_assert(std::same_as<uptr, decltype(i.div_euclid(p))>);
  static_assert(std::same_as<uptr, decltype(i.div_euclid(u))>);
  EXPECT_EQ(i.div_euclid(p), i.div_euclid(i));
  EXPECT_EQ(i.div_euclid(u), i.div_euclid(i));
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_div_euclid(i))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_div_euclid(p))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_div_euclid(u))>);
  EXPECT_EQ(i.checked_div_euclid(p), i.checked_div_euclid(i));
  EXPECT_EQ(i.checked_div_euclid(u), i.checked_div_euclid(i));
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_div_euclid(i))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_div_euclid(p))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_div_euclid(u))>);
  EXPECT_EQ(i.overflowing_div_euclid(p), i.overflowing_div_euclid(i));
  EXPECT_EQ(i.overflowing_div_euclid(u), i.overflowing_div_euclid(i));
  static_assert(std::same_as<uptr, decltype(i.wrapping_div_euclid(i))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_div_euclid(p))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_div_euclid(u))>);
  EXPECT_EQ(i.wrapping_div_euclid(p), i.wrapping_div_euclid(i));
  EXPECT_EQ(i.wrapping_div_euclid(u), i.wrapping_div_euclid(i));

  static_assert(std::same_as<uptr, decltype(i.rem_euclid(i))>);
  static_assert(std::same_as<uptr, decltype(i.rem_euclid(p))>);
  static_assert(std::same_as<uptr, decltype(i.rem_euclid(u))>);
  EXPECT_EQ(i.rem_euclid(p), i.rem_euclid(i));
  EXPECT_EQ(i.rem_euclid(u), i.rem_euclid(i));
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_rem_euclid(i))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_rem_euclid(p))>);
  static_assert(std::same_as<Option<uptr>, decltype(i.checked_rem_euclid(u))>);
  EXPECT_EQ(i.checked_rem_euclid(p), i.checked_rem_euclid(i));
  EXPECT_EQ(i.checked_rem_euclid(u), i.checked_rem_euclid(i));
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_rem_euclid(i))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_rem_euclid(p))>);
  static_assert(
      std::same_as<Tuple<uptr, bool>, decltype(i.overflowing_rem_euclid(u))>);
  EXPECT_EQ(i.overflowing_rem_euclid(p), i.overflowing_rem_euclid(i));
  EXPECT_EQ(i.overflowing_rem_euclid(u), i.overflowing_rem_euclid(i));
  static_assert(std::same_as<uptr, decltype(i.wrapping_rem_euclid(i))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_rem_euclid(p))>);
  static_assert(std::same_as<uptr, decltype(i.wrapping_rem_euclid(u))>);
  EXPECT_EQ(i.wrapping_rem_euclid(p), i.wrapping_rem_euclid(i));
  EXPECT_EQ(i.wrapping_rem_euclid(u), i.wrapping_rem_euclid(i));

  // Ceil math.

  static_assert(std::same_as<uptr, decltype(i.div_ceil(i))>);
  static_assert(std::same_as<uptr, decltype(i.div_ceil(p))>);
  static_assert(std::same_as<uptr, decltype(i.div_ceil(u))>);
  EXPECT_EQ(i.div_ceil(p), i.div_ceil(i));
  EXPECT_EQ(i.div_ceil(u), i.div_ceil(i));

  // Log math.
  static_assert(std::same_as<u32, decltype(i.log(i))>);
  static_assert(std::same_as<u32, decltype(i.log(p))>);
  static_assert(std::same_as<u32, decltype(i.log(u))>);
  EXPECT_EQ(i.log(p), i.log(i));
  EXPECT_EQ(i.log(u), i.log(i));
  static_assert(std::same_as<Option<u32>, decltype(i.checked_log(i))>);
  static_assert(std::same_as<Option<u32>, decltype(i.checked_log(p))>);
  static_assert(std::same_as<Option<u32>, decltype(i.checked_log(u))>);
  EXPECT_EQ(i.checked_log(p), i.checked_log(i));
  EXPECT_EQ(i.checked_log(u), i.checked_log(i));
}

TEST(uptr, fmt) {
  static_assert(fmt::is_formattable<uptr, char>::value);
  i32 i;
  EXPECT_EQ(fmt::format("{}", uptr(uintptr_t{1234567})), "1234567");
  EXPECT_EQ(fmt::format("{:#x}", uptr(uintptr_t{1234567})), "0x12d687");
}

}  // namespace
