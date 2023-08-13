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

sus_pure constexpr ::sus::collections::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
_self::to_be_bytes() const& noexcept {
  return to_be().to_ne_bytes();
}

sus_pure constexpr ::sus::collections::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
_self::to_le_bytes() const& noexcept {
  return to_le().to_ne_bytes();
}

sus_pure constexpr ::sus::collections::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
_self::to_ne_bytes() const& noexcept {
  auto bytes =
      ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>();
  if (std::is_constant_evaluated()) {
    auto uval = __private::into_unsigned(primitive_value);
    for (auto i = size_t{0}; i < ::sus::mem::size_of<_primitive>(); ++i) {
      const auto last_byte = static_cast<uint8_t>(uval & 0xff);
      if (sus::assertions::is_little_endian())
        bytes[i] = last_byte;
      else
        bytes[::sus::mem::size_of<_primitive>() - 1 - i] = last_byte;
      /* If _self is one byte, this shift would be UB. But it's also not needed
         since the loop will not run again. */
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

sus_pure inline constexpr _self _self::from_be_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return from_be(from_ne_bytes(bytes));
}

sus_pure inline constexpr _self _self::from_le_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return from_le(from_ne_bytes(bytes));
}

sus_pure inline constexpr _self _self::from_ne_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  using Unsigned = decltype(__private::into_unsigned(primitive_value));
  Unsigned val;
  if (std::is_constant_evaluated()) {
    val = Unsigned{0};
    for (auto i = size_t{0}; i < ::sus::mem::size_of<_primitive>(); ++i) {
      val |= bytes[i].primitive_value
             << (::sus::mem::size_of<_primitive>() - size_t{1} - i);
    }
  } else {
    ::sus::ptr::copy_nonoverlapping(
        ::sus::marker::unsafe_fn, reinterpret_cast<const char*>(bytes.as_ptr()),
        reinterpret_cast<char*>(&val), ::sus::mem::size_of<_primitive>());
  }
  return __private::into_signed(val);
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
sus__format_to_stream(sus::num, _self);

#undef _self
#undef _primitive
#undef _unsigned
