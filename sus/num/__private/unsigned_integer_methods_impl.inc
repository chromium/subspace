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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"

///////////////////////////////////////////////////////////////////////////
//
// Declares (and defines) methods of unsigned integer types
//
// TO USE THIS INC FILE:
//
// Include it in the top level namespace.
//
// Define `_self` to the name of the integer type.
// Define `_primitive` to the primitive it holds inside.
// Define `_signed` to the signed integer type of the same size (except when
//   _pointer is true).
// Define `_pointer` to true if the type is `uptr` or false otherwise.
///////////////////////////////////////////////////////////////////////////

namespace sus::num {

#if _pointer

template <class U>
_sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(U* u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  return R(_self(reinterpret_cast<_primitive>(u)));
}

template <Unsigned U>
  requires(::sus::mem::size_of<U>() == ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(U u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MAX_PRIMITIVE < U::MAX_PRIMITIVE) {
    if (u.primitive_value > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(u.primitive_value)));
}

template <UnsignedPrimitiveInteger U>
  requires(::sus::mem::size_of<U>() == ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(U u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MAX_PRIMITIVE < __private::max_value<U>()) {
    if (u > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(u)));
}

#else

template <Unsigned U>
_sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(U u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MAX_PRIMITIVE < U::MAX_PRIMITIVE) {
    if (u.primitive_value > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(u.primitive_value)));
}

template <UnsignedPrimitiveInteger U>
_sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(U u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MAX_PRIMITIVE < __private::max_value<U>()) {
    if (u > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(u)));
}

template <Signed S>
_sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(S s) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if (s.primitive_value < decltype(S::primitive_value){0}) {
    return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
  }
  constexpr auto umax = __private::into_unsigned(S::MAX_PRIMITIVE);
  if constexpr (MAX_PRIMITIVE < umax) {
    if (__private::into_unsigned(s.primitive_value) > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(s.primitive_value)));
}

template <SignedPrimitiveInteger S>
_sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(S s) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if (s < 0) {
    return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
  }
  constexpr auto umax = __private::into_unsigned(__private::max_value<S>());
  if constexpr (MAX_PRIMITIVE < umax) {
    if (__private::into_unsigned(s) > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(s)));
}

template <class S>
  requires(SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>)
_sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(S s) noexcept {
  using D = std::underlying_type_t<S>;
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if (static_cast<D>(s) < 0) {
    return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
  }
  constexpr auto umax = __private::into_unsigned(__private::max_value<D>());
  if constexpr (MAX_PRIMITIVE < umax) {
    if (__private::into_unsigned(static_cast<D>(s)) > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(s)));
}

template <class U>
  requires(UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>)
_sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(U u) noexcept {
  using D = std::underlying_type_t<U>;
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MAX_PRIMITIVE < __private::max_value<D>()) {
    if (static_cast<D>(u) > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(u)));
}

#endif

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_add(
    _self rhs) const& noexcept {
  const auto out =
      __private::add_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>(_self(out.value));
  else
    return ::sus::option::Option<_self>();
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::option::Option<_self> _self::checked_add(
    U rhs) const& noexcept {
  return checked_add(_self(_primitive{rhs}));
}
#endif

#if !_pointer  // No `_signed` for uptr.

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_add_signed(
    _signed rhs) const& noexcept {
  const auto out =
      __private::add_with_overflow_signed(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>(out.value);
  else
    return ::sus::option::Option<_self>();
}

#endif

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_add(
    _self rhs) const& noexcept {
  const auto out =
      __private::add_with_overflow(primitive_value, rhs.primitive_value);
  return ::sus::tuple_type::Tuple<_self, bool>(_self(out.value), out.overflow);
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr sus::tuple_type::Tuple<_self, bool> _self::overflowing_add(
    U rhs) const& noexcept {
  return overflowing_add(_self(_primitive{rhs}));
}
#endif

#if !_pointer  // No `_signed` for uptr.

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_add_signed(_signed rhs) const& noexcept {
  const auto r =
      __private::add_with_overflow_signed(primitive_value, rhs.primitive_value);
  return ::sus::tuple_type::Tuple<_self, bool>(r.value, r.overflow);
}

_sus_pure constexpr _self _self::saturating_add_signed(
    _signed rhs) const& noexcept {
  const auto r =
      __private::add_with_overflow_signed(primitive_value, rhs.primitive_value);
  if (!r.overflow) [[likely]]
    return r.value;
  else {
    // TODO: Can this be done without a branch? If it's complex or uses compiler
    // stuff, move into intrinsics.
    if (rhs.primitive_value >= 0)
      return MAX;
    else
      return MIN;
  }
}

_sus_pure constexpr _self _self::wrapping_add_signed(
    _signed rhs) const& noexcept {
  return __private::add_with_overflow_signed(primitive_value,
                                             rhs.primitive_value)
      .value;
}

#endif

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_div(
    _self rhs) const& noexcept {
  if (rhs.primitive_value != 0u) [[likely]]
    return ::sus::option::Option<_self>(
        _self(__private::unchecked_div(primitive_value, rhs.primitive_value)));
  else
    return ::sus::option::Option<_self>();
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::option::Option<_self> _self::checked_div(
    U rhs) const& noexcept {
  return checked_div(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_div(
    _self rhs) const& noexcept {
  sus_check_with_message(rhs.primitive_value != _primitive{0},
                            "attempt to divide by zero");
  return ::sus::tuple_type::Tuple<_self, bool>(
      _self(__private::unchecked_div(primitive_value, rhs.primitive_value)),
      false);
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_div(
    U rhs) const& noexcept {
  return overflowing_div(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_mul(
    _self rhs) const& noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>(_self(out.value));
  else
    return ::sus::option::Option<_self>();
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::option::Option<_self> _self::checked_mul(
    U rhs) const& noexcept {
  return checked_mul(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_mul(
    _self rhs) const& noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, rhs.primitive_value);
  return ::sus::tuple_type::Tuple<_self, bool>(_self(out.value), out.overflow);
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_mul(
    U rhs) const& noexcept {
  return overflowing_mul(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_neg()
    const& noexcept {
  if (primitive_value == 0u)
    return ::sus::option::Option<_self>(_self(_primitive{0u}));
  else
    return ::sus::option::Option<_self>();
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_neg() const& noexcept {
  return ::sus::tuple_type::Tuple<_self, bool>(
      (~(*this)).wrapping_add(_self(_primitive{1u})), primitive_value != 0u);
}

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_rem(
    _self rhs) const& noexcept {
  if (rhs.primitive_value != 0u) [[likely]]
    return ::sus::option::Option<_self>(
        _self(__private::unchecked_rem(primitive_value, rhs.primitive_value)));
  else
    return ::sus::option::Option<_self>();
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::option::Option<_self> _self::checked_rem(
    U rhs) const& noexcept {
  return checked_rem(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_rem(
    _self rhs) const& noexcept {
  sus_check_with_message(
      rhs.primitive_value != _primitive{0},
      "attempt to calculate the remainder with a divisor of zero");
  return ::sus::tuple_type::Tuple<_self, bool>(
      _self(__private::unchecked_rem(primitive_value, rhs.primitive_value)),
      false);
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_rem(
    U rhs) const& noexcept {
  return overflowing_rem(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_div_euclid(
    _self rhs) const& noexcept {
  if (rhs.primitive_value == 0u) [[unlikely]] {
    return ::sus::option::Option<_self>();
  } else {
    return ::sus::option::Option<_self>(
        _self(__private::unchecked_div(primitive_value, rhs.primitive_value)));
  }
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::option::Option<_self> _self::checked_div_euclid(
    U rhs) const& noexcept {
  return checked_div_euclid(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_div_euclid(_self rhs) const& noexcept {
  sus_check_with_message(rhs.primitive_value != _primitive{0},
                            "attempt to divide by zero");
  return ::sus::tuple_type::Tuple<_self, bool>(
      _self(__private::unchecked_div(primitive_value, rhs.primitive_value)),
      false);
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_div_euclid(U rhs) const& noexcept {
  return overflowing_div_euclid(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_rem_euclid(
    _self rhs) const& noexcept {
  if (rhs.primitive_value == 0u) [[unlikely]] {
    return ::sus::option::Option<_self>();
  } else {
    return ::sus::option::Option<_self>(
        _self(__private::unchecked_rem(primitive_value, rhs.primitive_value)));
  }
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::option::Option<_self> _self::checked_rem_euclid(
    U rhs) const& noexcept {
  return checked_rem_euclid(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_rem_euclid(_self rhs) const& noexcept {
  sus_check_with_message(
      rhs.primitive_value != _primitive{0},
      "attempt to calculate the remainder with a divisor of zero");
  return ::sus::tuple_type::Tuple<_self, bool>(
      _self(__private::unchecked_rem(primitive_value, rhs.primitive_value)),
      false);
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_rem_euclid(U rhs) const& noexcept {
  return overflowing_rem_euclid(_self(_primitive{rhs}));
}
#endif

constexpr inline void _self::operator<<=(u64 r) & noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    sus_check_with_message(r < u64(__private::num_bits<_primitive>()),
                              "attempt to shift left with overflow");
    primitive_value <<= r.primitive_value;
  } else {
    primitive_value = wrapping_shl(r).primitive_value;
  }
}

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_shl(
    u64 rhs) const& noexcept {
  const auto out =
      __private::shl_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>(_self(out.value));
  else
    return ::sus::option::Option<_self>();
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_shl(
    u64 rhs) const& noexcept {
  const auto out =
      __private::shl_with_overflow(primitive_value, rhs.primitive_value);
  return ::sus::tuple_type::Tuple<_self, bool>(_self(out.value), out.overflow);
}

_sus_pure constexpr _self _self::wrapping_shl(u64 rhs) const& noexcept {
  return _self(
      __private::shl_with_overflow(primitive_value, rhs.primitive_value).value);
}

constexpr inline void _self::operator>>=(u64 r) & noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    sus_check_with_message(r < u64(__private::num_bits<_primitive>()),
                              "attempt to shift right with overflow");
    primitive_value >>= r.primitive_value;
  } else {
    primitive_value = wrapping_shr(r).primitive_value;
  }
}

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_shr(
    u64 rhs) const& noexcept {
  const auto out =
      __private::shr_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>(_self(out.value));
  else
    return ::sus::option::Option<_self>();
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_shr(
    u64 rhs) const& noexcept {
  const auto out =
      __private::shr_with_overflow(primitive_value, rhs.primitive_value);
  return ::sus::tuple_type::Tuple<_self, bool>(_self(out.value), out.overflow);
}

_sus_pure constexpr _self _self::wrapping_shr(u64 rhs) const& noexcept {
  return _self(
      __private::shr_with_overflow(primitive_value, rhs.primitive_value).value);
}

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_sub(
    _self rhs) const& {
  const auto out =
      __private::sub_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>(_self(out.value));
  else
    return ::sus::option::Option<_self>();
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::option::Option<_self> _self::checked_sub(
    U rhs) const& noexcept {
  return checked_sub(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_sub(
    _self rhs) const& noexcept {
  const auto out =
      __private::sub_with_overflow(primitive_value, rhs.primitive_value);
  return ::sus::tuple_type::Tuple<_self, bool>(_self(out.value), out.overflow);
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_sub(
    U rhs) const& noexcept {
  return overflowing_sub(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_pow(
    u32 rhs) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>(_self(out.value));
  else
    return ::sus::option::Option<_self>();
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> _self::overflowing_pow(
    u32 exp) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, exp.primitive_value);
  return ::sus::tuple_type::Tuple<_self, bool>(_self(out.value), out.overflow);
}

_sus_pure constexpr _self _self::rotate_left(u64 n) const& noexcept {
  return _self(__private::rotate_left(primitive_value, n.primitive_value));
}

_sus_pure constexpr _self _self::rotate_right(u64 n) const& noexcept {
  return _self(__private::rotate_right(primitive_value, n.primitive_value));
}

_sus_pure constexpr ::sus::option::Option<u32> _self::checked_log2()
    const& noexcept {
  if (primitive_value == 0u) [[unlikely]] {
    return ::sus::option::Option<u32>();
  } else {
    uint32_t zeros = __private::leading_zeros_nonzero(::sus::marker::unsafe_fn,
                                                      primitive_value);
    return ::sus::option::Option<u32>(BITS - u32(1u) - u32(zeros));
  }
}

_sus_pure constexpr u32 _self::log2() const& noexcept {
  return checked_log2().expect(
      "argument of integer logarithm must be positive");
}

_sus_pure constexpr ::sus::option::Option<u32> _self::checked_log10()
    const& noexcept {
  if (primitive_value == 0u) [[unlikely]] {
    return ::sus::option::Option<u32>();
  } else {
    return ::sus::option::Option<u32>(
        __private::int_log10::_self(primitive_value));
  }
}

_sus_pure constexpr u32 _self::log10() const& noexcept {
  return checked_log10().expect(
      "argument of integer logarithm must be positive");
}

_sus_pure constexpr ::sus::option::Option<u32> _self::checked_log(
    _self base) const& noexcept {
  if (primitive_value == 0u || base.primitive_value <= 1u) [[unlikely]] {
    return ::sus::option::Option<u32>();
  } else {
    auto n = uint32_t{0u};
    auto r = primitive_value;
    const auto b = base.primitive_value;
    while (r >= b) {
      r /= b;
      n += 1u;
    }
    return ::sus::option::Option<u32>(n);
  }
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::option::Option<u32> _self::checked_log(
    U rhs) const& noexcept {
  return checked_log(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr u32 _self::log(_self base) const& noexcept {
  return checked_log(base).expect(
      "argument of integer logarithm must be positive");
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr u32 _self::log(U rhs) const& noexcept {
  return log(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::option::Option<_self>
_self::checked_next_power_of_two() const& noexcept {
  const auto one_less =
      __private::one_less_than_next_power_of_two(primitive_value);
  return _self(one_less).checked_add(_self(_primitive{1u}));
}

_sus_pure constexpr ::sus::option::Option<_self> _self::checked_next_multiple_of(
    _self rhs) const& noexcept {
  return checked_rem(rhs).and_then([s = *this, rhs](_self r) {
    return r == _primitive{0}
               ? Option<_self>(s)
               // SAFETY: rhs - r cannot overflow because `r` is smaller than
               // `rhs`, as it's the remainder of `*this / rhs`.
               : s.checked_add(__private::unchecked_sub(rhs.primitive_value,
                                                        r.primitive_value));
  });
}

#if _pointer
template <class U>
  requires((UnsignedNumeric<U> || UnsignedPrimitiveInteger<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr ::sus::option::Option<_self> _self::checked_next_multiple_of(
    U rhs) const& noexcept {
  return checked_next_multiple_of(_self(_primitive{rhs}));
}
#endif

_sus_pure constexpr ::sus::collections::Array<u8,
                                             ::sus::mem::size_of<_primitive>()>
_self::to_be_bytes() const& noexcept {
  return to_be().to_ne_bytes();
}

_sus_pure constexpr ::sus::collections::Array<u8,
                                             ::sus::mem::size_of<_primitive>()>
_self::to_le_bytes() const& noexcept {
  return to_le().to_ne_bytes();
}

_sus_pure constexpr ::sus::collections::Array<u8,
                                             ::sus::mem::size_of<_primitive>()>
_self::to_ne_bytes() const& noexcept {
  auto bytes =
      ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>();
  if (std::is_constant_evaluated()) {
    auto uval = primitive_value;
    for (usize i; i < ::sus::mem::size_of<_primitive>(); i += 1u) {
      const auto last_byte = static_cast<uint8_t>(uval & 0xff);
      if constexpr (std::endian::native == std::endian::little)
        bytes[i] = last_byte;
      else
        bytes[::sus::mem::size_of<_primitive>() - 1u - i] = last_byte;
      // If _self is one byte, this shift would be UB. But it's also not needed
      // since the loop will not run again.
      if constexpr (::sus::mem::size_of<_primitive>() > 1u) uval >>= 8u;
    }
    return bytes;
  } else {
    ::sus::ptr::copy_nonoverlapping(
        ::sus::marker::unsafe_fn,
        reinterpret_cast<const char*>(&primitive_value),
        reinterpret_cast<char*>(bytes.as_mut_ptr()),
        ::sus::mem::size_of<_primitive>());
    return bytes;
  }
}
_sus_pure constexpr _self _self::from_be_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return from_be(from_ne_bytes(bytes));
}

_sus_pure constexpr _self _self::from_le_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return from_le(from_ne_bytes(bytes));
}

_sus_pure constexpr _self _self::from_ne_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  _self val;
  if (std::is_constant_evaluated()) {
    val = _primitive{0};
    // size_of<_primitive>() < u32::MAX` so casting to u32 is not lossy.
    for (u32 i; i < ::sus::cast<u32>(::sus::mem::size_of<_primitive>());
         i += 1u) {
      val |= _primitive{bytes[i]} << (i * 8u);
    }
  } else {
    ::sus::ptr::copy_nonoverlapping(
        ::sus::marker::unsafe_fn, reinterpret_cast<const char*>(bytes.as_ptr()),
        reinterpret_cast<char*>(&val.primitive_value),
        ::sus::mem::size_of<_primitive>());
  }
  return _self(val);
}

_sus_pure constexpr u32 _self::count_ones() const& noexcept {
  return __private::count_ones(primitive_value);
}

_sus_pure constexpr u32 _self::count_zeros() const& noexcept {
  return (~(*this)).count_ones();
}

_sus_pure constexpr u32 _self::leading_ones() const& noexcept {
  return (~(*this)).leading_zeros();
}

_sus_pure constexpr u32 _self::leading_zeros() const& noexcept {
  return __private::leading_zeros(primitive_value);
}

_sus_pure constexpr u32 _self::trailing_ones() const& noexcept {
  return (~(*this)).trailing_zeros();
}

_sus_pure constexpr u32 _self::trailing_zeros() const& noexcept {
  return __private::trailing_zeros(primitive_value);
}

_sus_pure constexpr _self _self::pow(u32 rhs) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, rhs.primitive_value);
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    sus_check_with_message(!out.overflow,
                              "attempt to multiply with overflow");
  }
  return _self(out.value);
}

_sus_pure constexpr _self _self::wrapping_pow(u32 exp) const& noexcept {
  return _self(__private::wrapping_pow(primitive_value, exp.primitive_value));
}

_sus_pure constexpr bool _self::is_power_of_two() const& noexcept {
  return count_ones() == _self(_primitive{1u});
}

}  // namespace sus::num

// std hash support.
template <>
struct std::hash<::sus::num::_self> {
  _sus_pure auto operator()(::sus::num::_self u) const noexcept {
    return std::hash<_primitive>()(u.primitive_value);
  }
};
template <>
struct std::equal_to<::sus::num::_self> {
  _sus_pure constexpr auto operator()(::sus::num::_self l,
                                     ::sus::num::_self r) const noexcept {
    return l == r;
  }
};

// fmt support.
template <class Char>
struct fmt::formatter<::sus::num::_self, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(::sus::num::_self t, FormatContext& ctx) const {
    return underlying_.format(t.primitive_value, ctx);
  }

 private:
  formatter<_primitive, Char> underlying_;
};

#undef _self
#undef _primitive
#undef _signed
#undef _pointer
