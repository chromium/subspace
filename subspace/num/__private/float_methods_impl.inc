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
// WI_selfHOU_self WARRAN_selfIES OR CONDI_selfIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions
// and limitations under the License.

///////////////////////////////////////////////////////////////////////////
//
// Declares (and defines) methods of floating point types
//
// _selfO USE _selfHIS INC FILE:
//
// Include it into the body of your class.
//
// Define `_self` to the name of the floating point type.
// Define `_primitive` to the primitive it holds inside.
// Define `_unsigned` to the unsigned integer type of the same size.
///////////////////////////////////////////////////////////////////////////

namespace sus::num {

sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
_self::to_be_bytes() const& noexcept {
  return to_bits().to_be_bytes();
}
sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
_self::to_le_bytes() const& noexcept {
  return to_bits().to_le_bytes();
}
sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
_self::to_ne_bytes() const& noexcept {
  return to_bits().to_ne_bytes();
}
sus_pure inline _self _self::from_be_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return _self::from_bits(_unsigned::from_be_bytes(bytes));
}
sus_pure inline _self _self::from_le_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return _self::from_bits(_unsigned::from_le_bytes(bytes));
}
sus_pure inline _self _self::from_ne_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept {
  return _self::from_bits(_unsigned::from_ne_bytes(bytes));
}

}  // namespace sus::num

// std hash support.
template <>
struct std::hash<::sus::num::_self> {
  sus_pure auto operator()(::sus::num::_self u) const {
    return std::hash<_primitive>()(u.primitive_value);
  }
};
template <>
struct std::equal_to<::sus::num::_self> {
  sus_pure constexpr auto operator()(::sus::num::_self l,
                                     ::sus::num::_self r) const {
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
#undef _unsigned
