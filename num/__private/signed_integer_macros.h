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

#pragma once

namespace sus::num::__private {

template <class T>
constexpr inline bool can_add_without_overflow(const T& l, const T& r) {
  if (r.primitive_value >= 0)
    return l.primitive_value <= T::MAX_PRIMITIVE - r.primitive_value;
  else
    return l.primitive_value >= T::MIN_PRIMITIVE - r.primitive_value;
}

template <class T>
constexpr inline bool can_sub_without_overflow(const T& l, const T& r) {
  if (r.primitive_value <= 0)
    return l.primitive_value <= T::MAX_PRIMITIVE + r.primitive_value;
  else
    return l.primitive_value >= T::MIN_PRIMITIVE + r.primitive_value;
}

template <class T>
constexpr inline bool can_mul_without_overflow(const T& l, const T& r) {
  // TODO: Is this really correct always? False positives?
  if (l.primitive_value == 0 || r.primitive_value == 0) {
    return true;
  } else if (l.primitive_value > 0 && r.primitive_value > 0) {
    return l.primitive_value <= T::MAX_PRIMITIVE / r.primitive_value;
  } else if (r.primitive_value > 0) {
    return l.primitive_value >= T::MIN_PRIMITIVE / r.primitive_value;
  } else {
    return r.primitive_value >= T::MIN_PRIMITIVE / l.primitive_value;
  }
}

template <class T>
constexpr inline bool can_div_without_overflow(const T& l, const T& r) {
  if (r.primitive_value == 0) {
    // Divide by zero.
    return false;
  } else if (r.primitive_value == -1) {
    // Overflow: MIN / -1 => MAX + 1.
    return l.primitive_value > T::MIN();
  } else {
    return true;
  }
}

}  // namespace sus::num::__private

#define _sus__signed_constants(T, Max, Bits)                         \
  static constexpr auto MIN_PRIMITIVE = primitive_type{-Max - 1};    \
  static constexpr auto MAX_PRIMITIVE = primitive_type{Max};         \
  static constexpr inline T MIN() noexcept { return MIN_PRIMITIVE; } \
  static constexpr inline T MAX() noexcept { return MAX_PRIMITIVE; } \
  static constexpr inline /* TODO: u32 */ uint32_t BITS() noexcept { \
    return /* TODO: u32(*/ Bits /*)*/;                               \
  }                                                                  \
  static_assert(true)

#define _sus__signed_impl(T, LargerT, UnsignedT) \
  _sus__signed_integer_comparison(T);            \
  _sus__signed_unary_ops(T);                     \
  _sus__signed_binary_logic_ops(T);              \
  _sus__signed_binary_bit_ops(T, UnsignedT);     \
  _sus__signed_mutable_logic_ops(T);             \
  _sus__signed_mutable_bit_ops(T, UnsignedT);    \
  _sus__signed_abs(T, UnsignedT);                \
  _sus__signed_add(T);                           \
  _sus__signed_div(T);                           \
  _sus__signed_mul(T, LargerT);                  \
  _sus__signed_neg(T);                           \
  _sus__signed_rem(T);                           \
  _sus__signed_shift(T, UnsignedT)

#define _sus__signed_integer_comparison(T)                                    \
  /** sus::concepts::Eq<##T##> trait. */                                      \
  friend constexpr inline bool operator==(const T& l, const T& r) noexcept {  \
    return (l.primitive_value <=> r.primitive_value) == 0;                    \
  }                                                                           \
  /** sus::concepts::Ord<##T##> trait. */                                     \
  friend constexpr inline auto operator<=>(const T& l, const T& r) noexcept { \
    return l.primitive_value <=> r.primitive_value;                           \
  }                                                                           \
  static_assert(true)

#define _sus__signed_unary_ops(T)                                        \
  /** sus::concepts::Neg trait. */                                       \
  constexpr inline T operator-()&& noexcept {                            \
    ::sus::check(primitive_value != MIN_PRIMITIVE);                      \
    return -primitive_value;                                             \
  }                                                                      \
  /** sus::concepts::BitNot trait. */                                    \
  constexpr inline T operator~()&& noexcept { return ~primitive_value; } \
  static_assert(true)

#define _sus__signed_binary_logic_ops(T)                                 \
  /** sus::concepts::Add<##T##> trait. */                                \
  friend constexpr inline T operator+(const T& l, const T& r) noexcept { \
    ::sus::check(__private::can_add_without_overflow(l, r));             \
    return l.primitive_value + r.primitive_value;                        \
  }                                                                      \
  /** sus::concepts::Sub<##T##> trait. */                                \
  friend constexpr inline T operator-(const T& l, const T& r) noexcept { \
    ::sus::check(__private::can_sub_without_overflow(l, r));             \
    return l.primitive_value - r.primitive_value;                        \
  }                                                                      \
  /** sus::concepts::Mul<##T##> trait. */                                \
  friend constexpr inline T operator*(const T& l, const T& r) noexcept { \
    ::sus::check(__private::can_mul_without_overflow(l, r));             \
    return l.primitive_value * r.primitive_value;                        \
  }                                                                      \
  /** sus::concepts::Div<##T##> trait. */                                \
  friend constexpr inline T operator/(const T& l, const T& r) noexcept { \
    ::sus::check(__private::can_div_without_overflow(l, r));             \
    return l.primitive_value / r.primitive_value;                        \
  }                                                                      \
  /** sus::concepts::Rem<##T##> trait. */                                \
  friend constexpr inline T operator%(const T& l, const T& r) noexcept { \
    ::sus::check(__private::can_div_without_overflow(l, r));             \
    return l.primitive_value % r.primitive_value;                        \
  }                                                                      \
  static_assert(true)

#define _sus__signed_binary_bit_ops(T, UnsignedT)                             \
  /** sus::concepts::BitAnd<##T##> trait. */                                  \
  friend constexpr inline T operator&(const T& l, const T& r) noexcept {      \
    return l.primitive_value & r.primitive_value;                             \
  }                                                                           \
  /** sus::concepts::BitOr<##T##> trait. */                                   \
  friend constexpr inline T operator|(const T& l, const T& r) noexcept {      \
    return l.primitive_value | r.primitive_value;                             \
  }                                                                           \
  /** sus::concepts::BitXor<##T##> trait. */                                  \
  friend constexpr inline T operator^(const T& l, const T& r) noexcept {      \
    return l.primitive_value ^ r.primitive_value;                             \
  }                                                                           \
  /** sus::concepts::Shl trait. */                                            \
  friend constexpr inline T operator<<(const T& l,                            \
                                       /* TODO: u32 */ uint32_t r) noexcept { \
    ::sus::check(r < BITS());                                                 \
    return static_cast<primitive_type>(                                       \
        static_cast<UnsignedT>(l.primitive_value) << r);                      \
  }                                                                           \
  /** sus::concepts::Shr trait. */                                            \
  friend constexpr inline T operator>>(const T& l,                            \
                                       /* TODO: u32 */ uint32_t r) noexcept { \
    ::sus::check(r < BITS());                                                 \
    return static_cast<primitive_type>(                                       \
        static_cast<UnsignedT>(l.primitive_value) >> r);                      \
  }                                                                           \
  static_assert(true)

#define _sus__signed_mutable_logic_ops(T)                        \
  /** sus::concepts::AddAssign<##T##> trait. */                  \
  inline void operator+=(T r)& noexcept {                        \
    ::sus::check(__private::can_add_without_overflow(*this, r)); \
    primitive_value += r.primitive_value;                        \
  }                                                              \
  /** sus::concepts::SubAssign<##T##> trait. */                  \
  inline void operator-=(T r)& noexcept {                        \
    ::sus::check(__private::can_sub_without_overflow(*this, r)); \
    primitive_value -= r.primitive_value;                        \
  }                                                              \
  /** sus::concepts::MulAssign<##T##> trait. */                  \
  inline void operator*=(T r)& noexcept {                        \
    ::sus::check(__private::can_mul_without_overflow(*this, r)); \
    primitive_value *= r.primitive_value;                        \
  }                                                              \
  /** sus::concepts::DivAssign<##T##> trait. */                  \
  inline void operator/=(T r)& noexcept {                        \
    ::sus::check(__private::can_div_without_overflow(*this, r)); \
    primitive_value /= r.primitive_value;                        \
  }                                                              \
  /** sus::concepts::RemAssign<##T##> trait. */                  \
  inline void operator%=(T r)& noexcept {                        \
    ::sus::check(__private::can_div_without_overflow(*this, r)); \
    primitive_value %= r.primitive_value;                        \
  }                                                              \
  static_assert(true)

#define _sus__signed_mutable_bit_ops(T, UnsignedT)                \
  /** sus::concepts::BitAndAssign<##T##> trait. */                \
  inline void operator&=(T r)& noexcept {                         \
    primitive_value &= r.primitive_value;                         \
  }                                                               \
  /** sus::concepts::BitOrAssign<##T##> trait. */                 \
  inline void operator|=(T r)& noexcept {                         \
    primitive_value |= r.primitive_value;                         \
  }                                                               \
  /** sus::concepts::BitXorAssign<##T##> trait. */                \
  inline void operator^=(T r)& noexcept {                         \
    primitive_value ^= r.primitive_value;                         \
  }                                                               \
  /** sus::concepts::ShlAssign trait. */                          \
  inline void operator<<=(/* TODO: u32 */ uint32_t r)& noexcept { \
    ::sus::check(r < BITS());                                     \
    primitive_value = static_cast<primitive_type>(                \
        static_cast<UnsignedT>(primitive_value) << r);            \
  }                                                               \
  /** sus::concepts::ShrAssign trait. */                          \
  inline void operator>>=(/* TODO: u32 */ uint32_t r)& noexcept { \
    ::sus::check(r < BITS());                                     \
    primitive_value = static_cast<primitive_type>(                \
        static_cast<UnsignedT>(primitive_value) >> r);            \
  }                                                               \
  static_assert(true)

#define _sus__signed_abs(T, UnsignedT)                                         \
  /** Computes the absolute value of itself.                                   \
   *                                                                           \
   * The absolute value of ##T##::MIN() cannot be represented as an ##T##, and \
   * attempting to calculate it will panic.                                    \
   */                                                                          \
  constexpr inline T abs() const& noexcept {                                   \
    ::sus::check(primitive_value != MIN_PRIMITIVE);                            \
    if (primitive_value >= 0)                                                  \
      return primitive_value;                                                  \
    else                                                                       \
      return -primitive_value;                                                 \
  }                                                                            \
                                                                               \
  /** Checked absolute value. Computes `abs()`, returning None if the current  \
   * value is MIN().                                                           \
   */                                                                          \
  constexpr Option<T> checked_abs() const& noexcept {                          \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                           \
      return Option<T>::some(abs());                                           \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Saturating absolute value. Computes `abs()`, returning MAX if the        \
   *  current value is MIN() instead of overflowing.                           \
   */                                                                          \
  constexpr T saturating_abs() const& noexcept {                               \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                           \
      return abs();                                                            \
    else                                                                       \
      return MAX();                                                            \
  }                                                                            \
                                                                               \
  /** Computes the absolute value of self without any wrapping or panicking.   \
   */                                                                          \
  constexpr /* TODO: u32 */ UnsignedT unsigned_abs() const& noexcept {         \
    if (primitive_value >= 0)                                                  \
      return static_cast<UnsignedT>(primitive_value);                          \
    else                                                                       \
      return static_cast<UnsignedT>(-(primitive_value + 1)) + uint32_t{1};     \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) absolute value. Computes self.abs(), wrapping around  \
   * at the boundary of the type.                                              \
   *                                                                           \
   * The only case where such wrapping can occur is when one takes the         \
   * absolute value of the negative minimal value for the type; this is a      \
   * positive value that is too large to represent in the type. In such a      \
   * case, this function returns MIN itself.                                   \
   */                                                                          \
  constexpr T wrapping_abs() const& noexcept {                                 \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                           \
      return abs();                                                            \
    else                                                                       \
      return MIN();                                                            \
  }                                                                            \
                                                                               \
  /** Computes the absolute difference between self and other.                 \
   *                                                                           \
   * This function always returns the correct answer without overflow or       \
   * panics by returning an unsigned integer.                                  \
   */                                                                          \
  constexpr /* TODO: u32 */ UnsignedT abs_diff(const T& r) const& noexcept {   \
    if (primitive_value >= r.primitive_value)                                  \
      return static_cast<UnsignedT>(primitive_value - r.primitive_value);      \
    else                                                                       \
      return static_cast<UnsignedT>(r.primitive_value - primitive_value);      \
  }                                                                            \
  static_assert(true)

#define _sus__signed_add(T)                                                    \
  /** Checked integer addition. Computes self + rhs, returning None if         \
   * overflow occurred.                                                        \
   */                                                                          \
  constexpr Option<T> checked_add(const T& rhs) const& noexcept {              \
    if (__private::can_add_without_overflow(*this, rhs)) [[likely]]            \
      return Option<T>::some(unchecked_add(unsafe_fn, rhs));                   \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Saturating integer addition. Computes self + rhs, saturating at the      \
   * numeric bounds instead of overflowing.                                    \
   */                                                                          \
  constexpr T saturating_add(const T& rhs) const& noexcept {                   \
    if (__private::can_add_without_overflow(*this, rhs)) [[likely]] {          \
      return unchecked_add(unsafe_fn, rhs);                                    \
    } else if (rhs.primitive_value >= 0) {                                     \
      return MAX();                                                            \
    } else {                                                                   \
      return MIN();                                                            \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Unchecked integer addition. Computes self + rhs, assuming overflow       \
   * cannot occur.                                                             \
   *                                                                           \
   * # Safety                                                                  \
   * This results in undefined behavior when self + rhs > ##T##::MAX() or self \
   * + rhs < ##T##::MIN(), i.e. when checked_add() would return None.          \
   */                                                                          \
  inline constexpr T unchecked_add(::sus::marker::UnsafeFnMarker,              \
                                   const T& rhs) const& noexcept {             \
    return primitive_value + rhs.primitive_value;                              \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) addition. Computes self + rhs, wrapping around at the \
   * boundary of the type.                                                     \
   */                                                                          \
  constexpr T wrapping_add(const T& rhs) const& noexcept {                     \
    if (__private::can_add_without_overflow(*this, rhs)) [[likely]]            \
      return unchecked_add(unsafe_fn, rhs);                                    \
    if (rhs.primitive_value >= 0) {                                            \
      return MIN_PRIMITIVE + rhs.primitive_value + primitive_value -           \
             MAX_PRIMITIVE - 1;                                                \
    } else {                                                                   \
      return rhs.primitive_value - MIN_PRIMITIVE + primitive_value +           \
             MAX_PRIMITIVE + 1;                                                \
    }                                                                          \
  }                                                                            \
  static_assert(true)

#define _sus__signed_div(T)                                                    \
  /** Checked integer division. Computes self / rhs, returning None if rhs ==  \
   * 0 or the division results in overflow.                                    \
   */                                                                          \
  constexpr Option<T> checked_div(const T& rhs) const& noexcept {              \
    if (__private::can_div_without_overflow(*this, rhs)) [[likely]]            \
      return Option<T>::some(primitive_value / rhs.primitive_value);           \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Saturating integer division. Computes self / rhs, saturating at the      \
   * numeric bounds instead of overflowing.                                    \
   *                                                                           \
   * #Panics                                                                   \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  constexpr T saturating_div(const T& rhs) const& noexcept {                   \
    ::sus::check(rhs != 0);                                                    \
    if (__private::can_div_without_overflow(*this, rhs)) [[likely]]            \
      return primitive_value / rhs.primitive_value;                            \
    /* Only overflows in the case of -MIN() / -1, which gives MAX() + 1,       \
     saturated to MAX(). */                                                    \
    return MAX();                                                              \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) division. Computes self / rhs, wrapping around at the \
   * boundary of the type.                                                     \
   *                                                                           \
   * The only case where such wrapping can occur is when one divides MIN / -1  \
   * on a signed type (where MIN is the negative minimal value for the type);  \
   * this is equivalent to -MIN, a positive value that is too large to         \
   * represent in the type. In such a case, this function returns MIN itself.  \
   *                                                                           \
   * #Panics                                                                   \
   * This function will panic if rhs is 0.                                     \
   */                                                                          \
  constexpr T wrapping_div(const T& rhs) const& noexcept {                     \
    ::sus::check(rhs != 0);                                                    \
    if (__private::can_div_without_overflow(*this, rhs)) [[likely]]            \
      return primitive_value / rhs.primitive_value;                            \
    /* Only overflows in the case of -MIN() / -1, which gives MAX() + 1,       \
     that wraps around to MIN(). */                                            \
    return MIN();                                                              \
  }                                                                            \
  static_assert(true)

#define _sus__signed_mul(T, LargerT)                                           \
  /** Checked integer multiplication. Computes self * rhs, returning None if   \
   * overflow occurred.                                                        \
   */                                                                          \
  constexpr Option<T> checked_mul(const T& rhs) const& noexcept {              \
    if (__private::can_mul_without_overflow(*this, rhs)) [[likely]]            \
      return Option<T>::some(unchecked_mul(unsafe_fn, rhs));                   \
    else                                                                       \
      return Option<T>::none();                                                \
  }                                                                            \
                                                                               \
  /** Saturating integer division. Computes self / rhs, saturating at the      \
   * numeric bounds instead of overflowing.                                    \
   */                                                                          \
  constexpr T saturating_mul(const T& rhs) const& noexcept {                   \
    if (__private::can_mul_without_overflow(*this, rhs)) [[likely]]            \
      return unchecked_mul(unsafe_fn, rhs);                                    \
    else if (primitive_value >= 0 == rhs.primitive_value >= 0)                 \
      return MAX(); /* Same sign, so the outcome is positive. */               \
    else                                                                       \
      return MIN(); /* Different signs, so the outcome is negative. */         \
  }                                                                            \
                                                                               \
  /** Unchecked integer multiplication. Computes self * rhs, assuming overflow \
   * cannot occur.                                                             \
   *                                                                           \
   * # Safety                                                                  \
   * This results in undefined behavior when `self * rhs > ##T##::MAX()` or    \
   * `self                                                                     \
   * * rhs < ##T##::MIN()`, i.e. when `checked_mul()` would return None.       \
   */                                                                          \
  constexpr inline T unchecked_mul(::sus::marker::UnsafeFnMarker,              \
                                   const T& rhs) const& noexcept {             \
    return primitive_value * rhs.primitive_value;                              \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) multiplication. Computes self * rhs, wrapping around  \
   * at the boundary of the type.                                              \
   */                                                                          \
  constexpr T wrapping_mul(const T& rhs) const& noexcept {                     \
    if (__private::can_mul_without_overflow(*this, rhs)) [[likely]]            \
      return unchecked_mul(unsafe_fn, rhs);                                    \
    if constexpr (sizeof(T) == 8) {                                            \
      /* For i64 GCC/Clang, use __int128:                                      \
       https://quuxplusone.github.io/blog/2019/02/28/is-int128-integral/       \
       \                                                                       \
       For i64 MSVC, use _mult128, but what about constexpr?? If we can't do   \
       it then make the whole function non-constexpr:                          \
       https://docs.microsoft.com/en-us/cpp/intrinsics/mul128?view=msvc-170    \
       */                                                                      \
    }                                                                          \
                                                                               \
    static_assert(sizeof(LargerT) == 2 * sizeof(T));                           \
    auto out = LargerT{primitive_value} * LargerT{rhs.primitive_value};        \
    while (out > LargerT{MAX_PRIMITIVE})                                       \
      out -= LargerT{MAX_PRIMITIVE} - LargerT{MIN_PRIMITIVE} + 1;              \
    while (out < LargerT{MIN_PRIMITIVE})                                       \
      out += LargerT{MAX_PRIMITIVE} - LargerT{MIN_PRIMITIVE} + 1;              \
    return static_cast<primitive_type>(out);                                   \
  }                                                                            \
  static_assert(true)

#define _sus__signed_neg(T)                                                   \
  /** Checked negation. Computes -self, returning None if self == MIN.        \
   */                                                                         \
  constexpr Option<T> checked_neg() const& noexcept {                         \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return Option<T>::some(T(-primitive_value));                            \
    else                                                                      \
      return Option<T>::none();                                               \
  }                                                                           \
                                                                              \
  /** Saturating integer negation. Computes -self, returning MAX if self ==   \
   * MIN instead of overflowing.                                              \
   */                                                                         \
  constexpr T saturating_neg() const& noexcept {                              \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return T(-primitive_value);                                             \
    else                                                                      \
      return MAX();                                                           \
  }                                                                           \
                                                                              \
  /** Wrapping (modular) negation. Computes -self, wrapping around at the     \
   * boundary of the type.                                                    \
   *                                                                          \
   * The only case where such wrapping can occur is when one negates MIN() on \
   * a signed type (where MIN() is the negative minimal value for the type);  \
   * this is a positive value that is too large to represent in the type. In  \
   * such a case, this function returns MIN() itself.                         \
   */                                                                         \
  constexpr T wrapping_neg() const& noexcept {                                \
    if (primitive_value != MIN_PRIMITIVE) [[likely]]                          \
      return T(-primitive_value);                                             \
    else                                                                      \
      return MIN();                                                           \
  }                                                                           \
  static_assert(true)

#define _sus__signed_rem(T)                                                    \
  /** Checked integer remainder. Computes self % rhs, returning None if rhs == \
   * 0 or the division results in overflow.                                    \
   */                                                                          \
  constexpr Option<i32> checked_rem(const i32& rhs) const& noexcept {          \
    if (__private::can_div_without_overflow(*this, rhs)) [[likely]]            \
      return Option<i32>::some(i32(primitive_value % rhs.primitive_value));    \
    else                                                                       \
      return Option<i32>::none();                                              \
  }                                                                            \
                                                                               \
  /** Wrapping (modular) remainder. Computes self % rhs, wrapping around at    \
   * the boundary of the type.                                                 \
   *                                                                           \
   * Such wrap-around never actually occurs mathematically; implementation     \
   * artifacts make x % y invalid for MIN() / -1 on a signed type (where MIN() \
   * is the negative minimal value). In such a case, this function returns 0.  \
   */                                                                          \
  constexpr i32 wrapping_rem(const i32& rhs) const& noexcept {                 \
    ::sus::check(rhs != 0);                                                    \
    if (__private::can_div_without_overflow(*this, rhs)) [[likely]]            \
      return i32(primitive_value % rhs.primitive_value);                       \
    else                                                                       \
      return i32(0);                                                           \
  }                                                                            \
  static_assert(true)

#define _sus__signed_shift(T, UnsignedT)                                       \
  /** Checked shift left. Computes self << rhs, returning None if rhs is       \
   * larger than or equal to the number of bits in self.                       \
   */                                                                          \
  constexpr Option<i32> checked_shl(/* TODO: u32 */ uint32_t rhs)              \
      const& noexcept {                                                        \
    if (rhs < BITS()) [[likely]] {                                             \
      return Option<i32>::some(i32(static_cast<primitive_type>(                \
          static_cast<UnsignedT>(primitive_value) << rhs)));                   \
    } else {                                                                   \
      return Option<i32>::none();                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Panic-free bitwise shift-left; yields self << mask(rhs), where mask      \
   * removes any high-order bits of rhs that would cause the shift to exceed   \
   * the bitwidth of the type.                                                 \
   *                                                                           \
   * Note that this is not the same as a rotate-left; the RHS of a wrapping    \
   * shift-left is restricted to the range of the type, rather than the bits   \
   * shifted out of the LHS being returned to the other end. The primitive     \
   * integer types all implement a rotate_left function, which may be what you \
   * want instead.                                                             \
   */                                                                          \
  constexpr i32 wrapping_shl(/* TODO: u32 */ uint32_t rhs) const& noexcept {   \
    if (rhs < BITS()) [[likely]] {                                             \
      return i32(static_cast<primitive_type>(                                  \
          static_cast<UnsignedT>(primitive_value) << rhs));                    \
    } else {                                                                   \
      /* Using `BITS() - 1` as a mask only works if BITS() is a power of two,  \
      so we look for that for some values here. */                             \
      static_assert((BITS() >> 3) == 1    /* 8 bits */                         \
                    || (BITS() >> 4) == 1 /* 16 bits */                        \
                    || (BITS() >> 5) == 1 /* 32 bits */                        \
                    || (BITS() >> 6) == 1 /* 64 bits */                        \
                    || (BITS() >> 7) == 1 /* 128 bits */                       \
      );                                                                       \
      return i32(static_cast<primitive_type>(                                  \
          static_cast<UnsignedT>(primitive_value) << (rhs & (BITS() - 1))));   \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Checked shift right. Computes self >> rhs, returning None if rhs is      \
   * larger than or equal to the number of bits in self.                       \
   */                                                                          \
  constexpr Option<i32> checked_shr(/* TODO: u32 */ uint32_t rhs)              \
      const& noexcept {                                                        \
    if (rhs < BITS()) [[likely]] {                                             \
      return Option<i32>::some(i32(static_cast<primitive_type>(                \
          static_cast<UnsignedT>(primitive_value) >> rhs)));                   \
    } else {                                                                   \
      return Option<i32>::none();                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  /** Panic-free bitwise shift-right; yields self >> mask(rhs), where mask     \
   * removes any high-order bits of rhs that would cause the shift to exceed   \
   * the bitwidth of the type.                                                 \
   *                                                                           \
   * Note that this is not the same as a rotate-right; the RHS of a wrapping   \
   * shift-right is restricted to the range of the type, rather than the bits  \
   * shifted out of the LHS being returned to the other end. The primitive     \
   * integer types all implement a rotate_right function, which may be what    \
   * you want instead.                                                         \
   */                                                                          \
  constexpr i32 wrapping_shr(/* TODO: u32 */ uint32_t rhs) const& noexcept {   \
    if (rhs < BITS()) [[likely]] {                                             \
      return i32(static_cast<primitive_type>(                                  \
          static_cast<UnsignedT>(primitive_value) >> rhs));                    \
    } else {                                                                   \
      /* Using `BITS() - 1` as a mask only works if BITS() is a power of two,  \
      so we look for that for some values here. */                             \
      static_assert((BITS() >> 3) == 1    /* 8 bits */                         \
                    || (BITS() >> 4) == 1 /* 16 bits */                        \
                    || (BITS() >> 5) == 1 /* 32 bits */                        \
                    || (BITS() >> 6) == 1 /* 64 bits */                        \
                    || (BITS() >> 7) == 1 /* 128 bits */                       \
      );                                                                       \
      return i32(static_cast<primitive_type>(                                  \
          static_cast<UnsignedT>(primitive_value) >> (rhs & (BITS() - 1))));   \
    }                                                                          \
  }                                                                            \
  static_assert(true)
