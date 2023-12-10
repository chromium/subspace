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
// Declares (and defines) methods of signed integer types
//
// TO USE THIS INC FILE:
//
// Include it in the top level namespace.
//
// Define `_self` to the name of the integer type.
// Define `_primitive` to the primitive it holds inside.
// Define `_unsigned` to the unsigned integer type of the same size.
///////////////////////////////////////////////////////////////////////////

namespace sus::num {

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_abs() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return Tuple(abs(), false);
  else
    return Tuple(MIN, true);
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_add(_self rhs) const& noexcept {
  const auto r =
      __private::add_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple(r.value, r.overflow);
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_add_unsigned(_unsigned rhs) const& noexcept {
  const auto r = __private::add_with_overflow_unsigned(primitive_value,
                                                       rhs.primitive_value);
  return Tuple(r.value, r.overflow);
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_div(_self rhs) const& noexcept {
  sus_check_with_message(rhs.primitive_value != _primitive{0},
                         "attempt to divide by zero");
  if (__private::div_overflows_nonzero(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Tuple(MIN, true);
  } else {
    return Tuple(__private::unchecked_div(primitive_value, rhs.primitive_value),
                 false);
  }
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_mul(_self rhs) const& noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple(out.value, out.overflow);
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_neg() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return Tuple(__private::unchecked_neg(primitive_value), false);
  else
    return Tuple(MIN, true);
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_rem(_self rhs) const& noexcept {
  sus_check_with_message(
      rhs.primitive_value != _primitive{0},
      "attempt to calculate the remainder with a divisor of zero");
  if (__private::div_overflows_nonzero(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Tuple(_primitive{0}, true);
  } else {
    return Tuple(static_cast<_primitive>(primitive_value % rhs.primitive_value),
                 false);
  }
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_div_euclid(_self rhs) const& noexcept {
  sus_check_with_message(rhs.primitive_value != _primitive{0},
                         "attempt to divide by zero");
  if (__private::div_overflows_nonzero(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Tuple(MIN, true);
  } else {
    return Tuple(__private::div_euclid(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value),
                 false);
  }
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_rem_euclid(_self rhs) const& noexcept {
  sus_check_with_message(
      rhs.primitive_value != _primitive{0},
      "attempt to calculate the remainder with a divisor of zero");
  if (__private::div_overflows_nonzero(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Tuple(_primitive{0}, true);
  } else {
    return Tuple(__private::rem_euclid(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value),
                 false);
  }
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_shl(u64 rhs) const& noexcept {
  const auto out =
      __private::shl_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple(out.value, out.overflow);
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_shr(u64 rhs) const& noexcept {
  const auto out =
      __private::shr_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple(out.value, out.overflow);
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_sub(_self rhs) const& noexcept {
  const auto out =
      __private::sub_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple(out.value, out.overflow);
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_sub_unsigned(const _unsigned& rhs) const& noexcept {
  const auto out = __private::sub_with_overflow_unsigned(primitive_value,
                                                         rhs.primitive_value);
  return Tuple(out.value, out.overflow);
}

_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
_self::overflowing_pow(u32 exp) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, exp.primitive_value);
  return Tuple(out.value, out.overflow);
}

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
    auto uval = __private::into_unsigned(primitive_value);
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
        ::sus::mem::data_size_of<_primitive>());
    return bytes;
  }
}

_sus_pure inline constexpr _self _self::from_be_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return from_be(from_ne_bytes(bytes));
}

_sus_pure inline constexpr _self _self::from_le_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return from_le(from_ne_bytes(bytes));
}

_sus_pure inline constexpr _self _self::from_ne_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  using Unsigned = decltype(__private::into_unsigned(primitive_value));
  Unsigned val;
  if (std::is_constant_evaluated()) {
    val = Unsigned{0};
    for (usize i; i < ::sus::mem::size_of<_primitive>(); i += 1u) {
      // size_of<_primitive>() is <= u32::MAX so the cast<u32>() is not lossy.
      static_assert(::sus::mem::size_of<_primitive>() <= u32::MAX);
      val |= uint8_t{bytes[i] << sus::cast<u32>(
                         ::sus::mem::size_of<_primitive>() - 1u - i)};
    }
  } else {
    ::sus::ptr::copy_nonoverlapping(
        ::sus::marker::unsafe_fn, reinterpret_cast<const char*>(bytes.as_ptr()),
        reinterpret_cast<char*>(&val), ::sus::mem::data_size_of<_primitive>());
  }
  return __private::into_signed(val);
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

// Stream support.
_sus_format_to_stream(sus::num, _self);

#undef _self
#undef _primitive
#undef _unsigned
