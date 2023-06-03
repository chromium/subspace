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
// Define `_signed` to the signed integer type of the same size.
///////////////////////////////////////////////////////////////////////////

namespace sus::num {

template <Signed S>
sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(S s) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if (s.primitive_value < decltype(S::primitive_value){0}) {
    return R::with_err(::sus::num::TryFromIntError(
        ::sus::num::TryFromIntError::Kind::OutOfBounds));
  }
  constexpr auto umax = __private::into_unsigned(S::MAX_PRIMITIVE);
  if constexpr (MAX_PRIMITIVE < umax) {
    if (__private::into_unsigned(s.primitive_value) > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError(
          ::sus::num::TryFromIntError::Kind::OutOfBounds));
    }
  }
  return R::with(_self(static_cast<_primitive>(s.primitive_value)));
}

template <Unsigned U>
sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(U u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MAX_PRIMITIVE < U::MAX_PRIMITIVE) {
    if (u.primitive_value > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError(
          ::sus::num::TryFromIntError::Kind::OutOfBounds));
    }
  }
  return R::with(_self(static_cast<_primitive>(u.primitive_value)));
}

template <SignedPrimitiveInteger S>
sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(S s) {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if (s < 0) {
    return R::with_err(::sus::num::TryFromIntError(
        ::sus::num::TryFromIntError::Kind::OutOfBounds));
  }
  constexpr auto umax = __private::into_unsigned(__private::max_value<S>());
  if constexpr (MAX_PRIMITIVE < umax) {
    if (__private::into_unsigned(s) > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError(
          ::sus::num::TryFromIntError::Kind::OutOfBounds));
    }
  }
  return R::with(_self(static_cast<_primitive>(s)));
}

template <class S>
  requires(SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>)
sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(S s) {
  using D = std::underlying_type_t<S>;
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if (static_cast<D>(s) < 0) {
    return R::with_err(::sus::num::TryFromIntError(
        ::sus::num::TryFromIntError::Kind::OutOfBounds));
  }
  constexpr auto umax = __private::into_unsigned(__private::max_value<D>());
  if constexpr (MAX_PRIMITIVE < umax) {
    if (__private::into_unsigned(static_cast<D>(s)) > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError(
          ::sus::num::TryFromIntError::Kind::OutOfBounds));
    }
  }
  return R::with(_self(static_cast<_primitive>(s)));
}

template <UnsignedPrimitiveInteger U>
sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(U u) {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MAX_PRIMITIVE < __private::max_value<U>()) {
    if (u > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError(
          ::sus::num::TryFromIntError::Kind::OutOfBounds));
    }
  }
  return R::with(_self(static_cast<_primitive>(u)));
}

template <class U>
  requires(UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>)
sus_pure constexpr ::sus::result::Result<_self, ::sus::num::TryFromIntError>
_self::try_from(U u) {
  using D = std::underlying_type_t<U>;
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MAX_PRIMITIVE < __private::max_value<D>()) {
    if (static_cast<D>(u) > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError(
          ::sus::num::TryFromIntError::Kind::OutOfBounds));
    }
  }
  return R::with(_self(static_cast<_primitive>(u)));
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_add(
    const _self& rhs) const& noexcept {
  const auto out =
      __private::add_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>::some(out.value);
  else
    return ::sus::option::Option<_self>::none();
}

template <std::convertible_to<_signed> S>
sus_pure constexpr ::sus::option::Option<_self> _self::checked_add_signed(
    const S& rhs) const& noexcept {
  const auto out =
      __private::add_with_overflow_signed(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>::some(out.value);
  else
    return ::sus::option::Option<_self>::none();
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_div(
    const _self& rhs) const& noexcept {
  if (rhs.primitive_value != 0u) [[likely]]
    return ::sus::option::Option<_self>::some(
        __private::unchecked_div(primitive_value, rhs.primitive_value));
  else
    return ::sus::option::Option<_self>::none();
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_mul(
    const _self& rhs) const& noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>::some(out.value);
  else
    return ::sus::option::Option<_self>::none();
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_neg()
    const& noexcept {
  if (primitive_value == 0u)
    return ::sus::option::Option<_self>::some(_self(_primitive{0u}));
  else
    return ::sus::option::Option<_self>::none();
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_rem(
    const _self& rhs) const& noexcept {
  if (rhs.primitive_value != 0u) [[likely]]
    return ::sus::option::Option<_self>::some(
        __private::unchecked_rem(primitive_value, rhs.primitive_value));
  else
    return ::sus::option::Option<_self>::none();
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_div_euclid(
    const _self& rhs) const& noexcept {
  if (rhs.primitive_value == 0u) [[unlikely]] {
    return ::sus::option::Option<_self>::none();
  } else {
    return ::sus::option::Option<_self>::some(
        __private::unchecked_div(primitive_value, rhs.primitive_value));
  }
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_rem_euclid(
    const _self& rhs) const& noexcept {
  if (rhs.primitive_value == 0u) [[unlikely]] {
    return ::sus::option::Option<_self>::none();
  } else {
    return ::sus::option::Option<_self>::some(
        __private::unchecked_rem(primitive_value, rhs.primitive_value));
  }
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_shl(
    const u32& rhs) const& noexcept {
  const auto out =
      __private::shl_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>::some(out.value);
  else
    return ::sus::option::Option<_self>::none();
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_shr(
    const u32& rhs) const& noexcept {
  const auto out =
      __private::shr_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>::some(out.value);
  else
    return ::sus::option::Option<_self>::none();
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_sub(
    const _self& rhs) const& {
  const auto out =
      __private::sub_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>::some(out.value);
  else
    return ::sus::option::Option<_self>::none();
}

sus_pure constexpr ::sus::option::Option<_self> _self::checked_pow(
    const u32& rhs) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, rhs.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  if (!out.overflow) [[likely]]
    return ::sus::option::Option<_self>::some(out.value);
  else
    return ::sus::option::Option<_self>::none();
}

sus_pure constexpr ::sus::option::Option<u32> _self::checked_log2()
    const& noexcept {
  if (primitive_value == 0u) [[unlikely]] {
    return ::sus::option::Option<u32>::none();
  } else {
    uint32_t zeros = __private::leading_zeros_nonzero(::sus::marker::unsafe_fn,
                                                      primitive_value);
    return ::sus::option::Option<u32>::some(BITS - u32(1u) - u32(zeros));
  }
}

sus_pure constexpr u32 _self::log2() const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  return checked_log2().unwrap();
}

sus_pure constexpr ::sus::option::Option<u32> _self::checked_log10()
    const& noexcept {
  if (primitive_value == 0u) [[unlikely]] {
    return ::sus::option::Option<u32>::none();
  } else {
    return ::sus::option::Option<u32>::some(
        __private::int_log10::_self(primitive_value));
  }
}

sus_pure constexpr u32 _self::log10() const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  return checked_log10().unwrap();
}

sus_pure constexpr ::sus::option::Option<u32> _self::checked_log(
    const _self& base) const& noexcept {
  if (primitive_value == 0u || base.primitive_value <= 1u) [[unlikely]] {
    return ::sus::option::Option<u32>::none();
  } else {
    auto n = uint32_t{0u};
    auto r = primitive_value;
    const auto b = base.primitive_value;
    while (r >= b) {
      r /= b;
      n += 1u;
    }
    return ::sus::option::Option<u32>::some(n);
  }
}

sus_pure constexpr u32 _self::log(const _self& base) const& noexcept {
  return checked_log(base).unwrap();
}

sus_pure constexpr ::sus::option::Option<_self>
_self::checked_next_power_of_two() const& noexcept {
  const auto one_less =
      __private::one_less_than_next_power_of_two(primitive_value);
  return _self(one_less).checked_add(_self(_primitive{1u}));
}

sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
_self::to_be_bytes() const& noexcept {
  return to_be().to_ne_bytes();
}

sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
_self::to_le_bytes() const& noexcept {
  return to_le().to_ne_bytes();
}

sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
_self::to_ne_bytes() const& noexcept {
  auto bytes =
      ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>();
  if (std::is_constant_evaluated()) {
    auto uval = primitive_value;
    for (auto i = size_t{0}; i < ::sus::mem::size_of<_primitive>(); ++i) {
      const auto last_byte = static_cast<uint8_t>(uval & 0xff);
      if (sus::assertions::is_little_endian())
        bytes[i] = last_byte;
      else
        bytes[::sus::mem::size_of<_primitive>() - 1 - i] = last_byte;
      // If _self is one byte, this shift would be UB. But it's also not needed
      //   since the loop will not run again.
      if constexpr (::sus::mem::size_of<_primitive>() > 1) uval >>= 8u;
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
sus_pure constexpr _self _self::from_be_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return from_be(from_ne_bytes(bytes));
}

sus_pure constexpr _self _self::from_le_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return from_le(from_ne_bytes(bytes));
}

sus_pure constexpr _self _self::from_ne_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  _primitive val;
  if (std::is_constant_evaluated()) {
    val = 0u;
    for (auto i = size_t{0}; i < ::sus::mem::size_of<_primitive>(); ++i) {
      val |= bytes[i].primitive_value
             << (::sus::mem::size_of<_primitive>() - 1 - i);
    }
  } else {
    ::sus::ptr::copy_nonoverlapping(
        ::sus::marker::unsafe_fn, reinterpret_cast<const char*>(bytes.as_ptr()),
        reinterpret_cast<char*>(&val), ::sus::mem::size_of<_primitive>());
  }
  return val;
}

}  // namespace sus::num

// std hash support.
template <>
struct std::hash<::sus::num::_self> {
  sus_pure auto operator()(::sus::num::_self u) const noexcept {
    return std::hash<_primitive>()(u.primitive_value);
  }
};
template <>
struct std::equal_to<::sus::num::_self> {
  sus_pure constexpr auto operator()(::sus::num::_self l,
                                     ::sus::num::_self r) const noexcept {
    return l == r;
  }
};

// fmt support.
template <class Char>
struct fmt::formatter<::sus::num::_self, Char> {
  template <class ParseContext>
  constexpr decltype(auto) parse(ParseContext& ctx) {
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
