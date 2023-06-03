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
// Include it into the body of your class.
//
// Define `_self` to the name of the integer type.
// Define `_primitive` to the primitive it holds inside.
// Define `_signed` to the signed integer type of the same size.
///////////////////////////////////////////////////////////////////////////

/// The inner primitive value, in case it needs to be unwrapped from the type.
/// Avoid using this member except to convert when a consumer requires it.
_primitive primitive_value{0u};

/// The smallest value that can be represented by this integer type, as a native
/// C++ primitive.
static constexpr auto MIN_PRIMITIVE = __private::min_value<_primitive>();
/// The largest value that can be represented by this integer type, as a native
/// C++ primitive.
static constexpr auto MAX_PRIMITIVE = __private::max_value<_primitive>();
/// The smallest value that can be represented by this integer type.
static const _self MIN;
/// The largest value that can be represented by this integer type.
static const _self MAX;
/// The size of this integer type in bits.
static const u32 BITS;

/// Default constructor, which sets the integer to 0.
///
/// The trivial copy and move constructors are implicitly declared, as is the
/// trivial destructor.
///
/// #[doc.overloads=ctor.default]
constexpr inline _self() noexcept = default;

/// Construction from unsigned types where no bits are lost.
///
/// #[doc.overloads=ctor.unsigned.small.typed]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(U v) noexcept : primitive_value(v.primitive_value) {}

/// Construction from unsigned primitive types where no bits are lost.
///
/// #[doc.overloads=ctor.unsigned.small.primitive]
template <UnsignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from unsigned enum types where no bits are lost.
///
/// #[doc.overloads=ctor.unsigned.smallenum]
template <UnsignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from unsigned enum class types where no bits are lost.
///
/// #[doc.overloads=ctor.unsigned.smallenumclass]
template <UnsignedPrimitiveEnumClass P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
explicit constexpr inline _self(P v) noexcept
    : primitive_value(static_cast<_primitive>(v)) {}

/// Construction from unsigned types, where it can be checked at compile time
/// that no bits are lost.
///
/// For runtime conversion of larger values, use `from()`.
///
/// #[doc.overloads=ctor.unsigned.large.typed]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() > ::sus::mem::size_of<_primitive>())
consteval inline _self(U v) noexcept
    : primitive_value(  // SAFETY: We check below that v <= _self::MAX, so we
                        // can cast `v` to a `_self`.
          static_cast<_primitive>(v.primitive_value)) {
  // SAFETY: P has more bits than _self, so converting an unsigned _self
  // to P will fit. When removing the sign bit, there's still at least as many
  // bits as _self.
  if (v > static_cast<decltype(U::primitive_value)>(_self::MAX_PRIMITIVE)) {
    ::sus::panic_with_message(                                             
            *"Cannot assign to unsigned integer from value outside its "       
             "range.");
  }
}

/// Construction from unsigned primitive types, where it can be checked at
/// compile time that no bits are lost.
///
/// For runtime conversion of larger values, use `from()`.
///
/// #[doc.overloads=ctor.unsigned.large.primitive]
template <UnsignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>())
consteval inline _self(P v) noexcept
    : primitive_value(  // SAFETY: We check below that v <= _self::MAX, so we
                        // can cast `v` to a `_self`.
          static_cast<_primitive>(v)) {
  // SAFETY: P has more bits than _self, so converting an unsigned _self
  // to P will fit. When removing the sign bit, there's still at least as many
  // bits as _self.
  if (v > static_cast<P>(_self::MAX_PRIMITIVE)) {
    ::sus::panic_with_message(                                             
            *"Cannot assign to unsigned integer from value outside its "       
             "range.");
  }
}

/// Construction from unsigned enum types, where it can be checked at compile
/// time that no bits are lost.
///
/// For runtime conversion of larger values, use `from()`.
///
/// #[doc.overloads=ctor.unsigned.largeenum]
template <UnsignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>())
consteval inline _self(P v) noexcept
    : primitive_value(  // SAFETY: We check above that v <= _self::MAX, so we
                        // can
                        // cast `v` to a `_self`.
          static_cast<_primitive>(v)) {
  // SAFETY: P has more bits than _self, so converting an unsigned _self
  // to P will fit. When removing the sign bit, there's still at least as many
  // bits as _self.
  if (v > static_cast<std::underlying_type_t<P>>(_self::MAX_PRIMITIVE)) {
    ::sus::panic_with_message(                                           
            *"Cannot assign to unsigned integer from value outside its "     
             "range.");
  }
}

/// Construction from unsigned enum class types, where it can be checked at
/// compile time that no bits are lost.
///
/// For runtime conversion of larger values, use `from()`.
///
/// #[doc.overloads=ctor.unsigned.largeenumclass]
template <UnsignedPrimitiveEnumClass P>
  requires(::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>())
explicit consteval inline _self(P v) noexcept
    : primitive_value(  // SAFETY: We check below that v <= _self::MAX, so we
                        // can
                        // cast `v` to a `_self`.
          static_cast<_primitive>(v)) {
  // SAFETY: P has more bits than _self, so converting an unsigned _self
  // to P will fit. When removing the sign bit, there's still at least as many
  // bits as _self.
  if (static_cast<std::underlying_type_t<P>>(v) >
      static_cast<std::underlying_type_t<P>>(_self::MAX_PRIMITIVE)) {
    ::sus::panic_with_message(                                           
            *"Cannot assign to unsigned integer from value outside its "     
             "range.");
  }
}

/// Construction from signed types, where it can be checked at compile time that
/// no bits are lost.
///
/// For runtime conversion, use `from()`.
///
/// #[doc.overloads=ctor.signed.typed]
template <Signed S>
consteval inline _self(S v) noexcept
    : primitive_value(  // SAFETY: We check below that v <= _self::MAX, so we
                        // can cast `v` to a `_self`.
          static_cast<_primitive>(v.primitive_value)) {
  if (v < decltype(S::primitive_value){0}) {
    ::sus::panic_with_message(
        *"Cannot construct unsigned integer from negative value.");
  }
  if constexpr (::sus::mem::size_of<S>() > ::sus::mem::size_of<_primitive>()) {
    // SAFETY: S has more bits than _self, so converting an unsigned _self
    // to S will fit. When removing the sign bit, there's still at least as many
    // bits as _self.
    if (v > static_cast<decltype(S::primitive_value)>(_self::MAX_PRIMITIVE)) {
      ::sus::panic_with_message(                                             
            *"Cannot construct unsigned integer from value outside its "       
             "range.");
    }
  }
}

/// Construction from signed primitive types, where it can be checked at compile
/// time that no bits are lost.
///
/// For runtime conversion, use `from()`.
///
/// #[doc.overloads=ctor.signed.primitive]
template <SignedPrimitiveInteger P>
consteval inline _self(P v) noexcept
    : primitive_value(  // SAFETY: We check above that v <= _self::MAX, so we
                        // can cast `v` to a `_self`.
          static_cast<_primitive>(v)) {
  if (v < P{0}) {
    ::sus::panic_with_message(
        *"Cannot construct unsigned integer from negative value.");
  }
  if constexpr (::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>()) {
    // SAFETY: P has more bits than _self, so converting an unsigned _self
    // to P will fit. When removing the sign bit, there's still at least as many
    // bits as _self.
    if (v > static_cast<P>(_self::MAX_PRIMITIVE)) {
      ::sus::panic_with_message(                                             
            *"Cannot construct unsigned integer from value outside its "       
             "range.");
    }
  }
}

/// Construction from signed enum types, where it can be checked at compile time
/// that no bits are lost.
///
/// For runtime conversion, use `from()`.
///
/// #[doc.overloads=ctor.signedenum]
template <SignedPrimitiveEnum P>
consteval inline _self(P v) noexcept
    : primitive_value(  // SAFETY: We check above that v <= _self::MAX, so we
                        // can cast `v` to a `_self`.
          static_cast<_primitive>(v)) {
  const auto u = static_cast<std::underlying_type_t<P>>(v);
  if (u < std::underlying_type_t<P>{0}) {
    ::sus::panic_with_message(
        *"Cannot construct unsigned integer from negative value.");
  }
  if constexpr (::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>()) {
    // SAFETY: P has more bits than _self, so converting an unsigned _self
    // to P will fit. When removing the sign bit, there's still at least as many
    // bits as _self.
    if (u > static_cast<std::underlying_type_t<P>>(_self::MAX_PRIMITIVE)) {
      ::sus::panic_with_message(                                             
            *"Cannot construct unsigned integer from value outside its "       
             "range.");
    }
  }
}

/// Construction from signed enum types, where it can be checked at compile time
/// that no bits are lost.
///
/// For runtime conversion, use `from()`.
///
/// #[doc.overloads=ctor.signedenumclass]
template <SignedPrimitiveEnumClass P>
explicit consteval inline _self(P v) noexcept
    : primitive_value(  // SAFETY: We check above that v <= _self::MAX, so we
                        // can cast `v` to a `_self`.
          static_cast<_primitive>(v)) {
  const auto u = static_cast<std::underlying_type_t<P>>(v);
  if (u < std::underlying_type_t<P>{0}) {
    ::sus::panic_with_message(
        *"Cannot construct unsigned integer from negative value.");
  }
  if constexpr (::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>()) {
    // SAFETY: P has more bits than _self, so converting an unsigned _self
    // to P will fit. When removing the sign bit, there's still at least as many
    // bits as _self.
    if (u > static_cast<std::underlying_type_t<P>>(_self::MAX_PRIMITIVE)) {
      ::sus::panic_with_message(                                             
            *"Cannot construct unsigned integer from value outside its "       
             "range.");
    }
  }
}

/// Assignment from unsigned types where no bits are lost.
///
/// #[doc.overloads=assign.unsigned.small.typed]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(U v) noexcept {
  primitive_value = v.primitive_value;
  return *this;
}

/// Assignment from unsigned primitive types where no bits are lost.
///
/// #[doc.overloads=assign.unsigned.small.primitive]
template <UnsignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(P v) noexcept {
  primitive_value = v;
  return *this;
}

/// Assignment from unsigned enum types where no bits are lost.
///
/// #[doc.overloads=assign.unsigned.smallenum]
template <UnsignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(P v) noexcept {
  primitive_value = static_cast<_primitive>(v);
  return *this;
}

/// Assignment from unsigned types, where it can be checked at compile time that
/// no bits are lost.
///
/// For runtime conversion of larger values, use `from()`.
///
/// #[doc.overloads=assign.unsigned.large.typed]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() > ::sus::mem::size_of<_primitive>())
consteval inline _self& operator=(U v) noexcept {
  // SAFETY: U has more bits than _self, so converting an unsigned _self
  // to U will fit. When removing the sign bit, there's still at least as many
  // bits as _self.
  if (v > static_cast<decltype(U::primitive_value)>(_self::MAX_PRIMITIVE)) {
    ::sus::panic_with_message(                                             
            *"Cannot assign to unsigned integer from value outside its "       
             "range.");
  }
  // SAFETY: We check above that v <= _self::MAX, so we can cast `v` to a
  // `_self`.
  primitive_value = static_cast<_primitive>(v.primitive_value);
  return *this;
}

/// Assignment from unsigned primitive types, where it can be checked at compile
/// time that no bits are lost.
///
/// For runtime conversion of larger values, use `from()`.
///
/// #[doc.overloads=assign.unsigned.large.primitive]
template <UnsignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>())
consteval inline _self& operator=(P v) noexcept {
  // SAFETY: P has more bits than _self, so converting an unsigned _self
  // to P will fit. When removing the sign bit, there's still at least as many
  // bits as _self.
  if (v > static_cast<P>(_self::MAX_PRIMITIVE)) {
    ::sus::panic_with_message(                                             
            *"Cannot assign to unsigned integer from value outside its "       
             "range.");
  }
  // SAFETY: We check above that v <= _self::MAX, so we can cast `v` to a
  // `_self`.

  primitive_value = static_cast<_primitive>(v);
  return *this;
}

/// Assignment from unsigned enum types, where it can be checked at compile time
/// that no bits are lost.
///
/// For runtime conversion of larger values, use `from()`.
///
/// #[doc.overloads=assign.unsigned.largeenum]
template <UnsignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>())
consteval inline _self& operator=(P v) noexcept {
  // SAFETY: P has more bits than _self, so converting an unsigned _self
  // to P will fit. When removing the sign bit, there's still at least as many
  // bits as _self.
  if (v > static_cast<std::underlying_type_t<P>>(_self::MAX_PRIMITIVE)) {
    ::sus::panic_with_message(                                             
            *"Cannot assign to unsigned integer from value outside its "       
             "range.");
  }
  // SAFETY: We check above that v <= _self::MAX, so we can cast `v` to a
  // `_self`.
  primitive_value = static_cast<_primitive>(v);
  return *this;
}

/// Assignment from signed types, where it can be checked at compile time that
/// no bits are lost.
///
/// For runtime conversion, use `from()`.
///
/// #[doc.overloads=assign.signed.typed]
template <Signed S>
consteval inline _self& operator=(S v) noexcept {
  if (v < decltype(S::primitive_value){0}) {
    ::sus::panic_with_message(
        *"Cannot assign to unsigned integer from negative value.");
  }
  if constexpr (::sus::mem::size_of<S>() > ::sus::mem::size_of<_primitive>()) {
    // SAFETY: P has more bits than _self, so converting an unsigned _self
    // to P will fit. When removing the sign bit, there's still at least as many
    // bits as _self.
    if (v > static_cast<decltype(S::primitive_value)>(_self::MAX_PRIMITIVE)) {
      ::sus::panic_with_message(                                             
            *"Cannot assign to unsigned integer from value outside its "       
             "range.");
    }
  }
  // SAFETY: We check above that v <= _self::MAX, so we can cast `v` to a
  // `_self`.

  primitive_value = static_cast<_primitive>(v.primitive_value);
  return *this;
}

/// Assignment from signed primitive types, where it can be checked at compile
/// time that no bits are lost.
///
/// For runtime conversion, use `from()`.
///
/// #[doc.overloads=assign.signed.primitive]
template <SignedPrimitiveInteger P>
consteval inline _self& operator=(P v) noexcept {
  if (static_cast<std::underlying_type_t<P>>(v) <
      std::underlying_type_t<P>{0}) {
    ::sus::panic_with_message(
        *"Cannot assign to unsigned integer from negative value.");
  }
  if constexpr (::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>()) {
    // SAFETY: P has more bits than _self, so converting an unsigned _self
    // to P will fit. When removing the sign bit, there's still at least as many
    // bits as _self.
    if (v > static_cast<P>(_self::MAX_PRIMITIVE)) {
      ::sus::panic_with_message(                                             
            *"Cannot assign to unsigned integer from value outside its "       
             "range.");
    }
  }
  // SAFETY: We check above that v <= _self::MAX, so we can cast `v` to a
  // `_self`.
  primitive_value = static_cast<_primitive>(v);
  return *this;
}

/// Assignment from signed enum types, where it can be checked at compile time
/// that no bits are lost.
///
/// For runtime conversion, use `from()`.
///
/// #[doc.overloads=assign.signedenum]
template <SignedPrimitiveEnum P>
consteval inline _self& operator=(P v) noexcept {
  const auto u = static_cast<std::underlying_type_t<P>>(v);
  if (u < std::underlying_type_t<P>{0}) {
    ::sus::panic_with_message(
        *"Cannot assign to unsigned integer from negative value.");
  }
  if constexpr (::sus::mem::size_of<P>() > ::sus::mem::size_of<_primitive>()) {
    // SAFETY: P has more bits than _self, so converting an unsigned _self
    // to P will fit. When removing the sign bit, there's still at least as many
    // bits as _self.
    if (u > static_cast<std::underlying_type_t<P>>(_self::MAX_PRIMITIVE)) {
      ::sus::panic_with_message(                                             
            *"Cannot assign to unsigned integer from value outside its "       
             "range.");
    }
  }
  // SAFETY: We check above that v <= _self::MAX, so we can cast `v` to a
  // `_self`.
  primitive_value = static_cast<_primitive>(v);
  return *this;
}

/// Constructs a _self from a signed integer type (i8, i16, i32, etc).
///
/// # Panics
/// The function will panic if the input value is out of range for _self.
///
/// #[doc.overloads=unsigned.from.signed]
template <Signed S>
sus_pure static constexpr _self from(S s) noexcept {
  ::sus::check(s.primitive_value >= decltype(S::primitive_value){0});
  constexpr auto umax = __private::into_unsigned(S::MAX_PRIMITIVE);
  if constexpr (MAX_PRIMITIVE < umax)
    ::sus::check(__private::into_unsigned(s.primitive_value) <= MAX_PRIMITIVE);
  return _self(static_cast<_primitive>(s.primitive_value));
}

/// Tries to construct a _self from a signed integer type (i8, i16, i32,
/// etc).
///
/// Returns an error if the source value is outside of the range of _self.
///
/// #[doc.overloads=unsigned.tryfrom.signed]
template <Signed S>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(S s) noexcept;

/// Constructs a _self from an unsigned integer type (u8, u16, u32, etc).
///
/// # Panics
/// The function will panic if the input value is out of range for _self.
///
/// #[doc.overloads=unsigned.from.unsigned]
template <Unsigned U>
sus_pure static constexpr _self from(U u) noexcept {
  if constexpr (MAX_PRIMITIVE < U::MAX_PRIMITIVE)
    ::sus::check(u.primitive_value <= MAX_PRIMITIVE);
  return _self(static_cast<_primitive>(u.primitive_value));
}

/// Tries to construct a _self from an unsigned integer type (u8, u16, u32,
/// etc).
///
/// Returns an error if the source value is outside of the range of _self.
///
/// #[doc.overloads=unsigned.tryfrom.unsigned]
template <Unsigned U>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u) noexcept;

/// Constructs a _self from a signed primitive integer type (int, long,
/// etc).
///
/// # Panics
/// The function will panic if the input value is out of range for _self.
///
/// #[doc.overloads=unsigned.from.signedprimitive]
template <SignedPrimitiveInteger S>
sus_pure static constexpr _self from(S s) {
  ::sus::check(s >= 0);
  constexpr auto umax = __private::into_unsigned(__private::max_value<S>());
  if constexpr (MAX_PRIMITIVE < umax)
    ::sus::check(__private::into_unsigned(s) <= MAX_PRIMITIVE);
  return _self(static_cast<_primitive>(s));
}

/// Tries to construct a _self from a signed primitive integer type (int,
/// long, etc).
///
/// Returns an error if the source value is outside of the range of _self.
///
/// #[doc.overloads=unsigned.tryfrom.signedprimitive]
template <SignedPrimitiveInteger S>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(S s);

/// Constructs a _self from a signed enum type (or enum class).
///
/// # Panics
/// The function will panic if the input value is out of range for _self.
///
/// #[doc.overloads=unsigned.from.signedenum]
template <class S>
  requires(SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>)
sus_pure static constexpr _self from(S s) {
  using D = std::underlying_type_t<S>;
  ::sus::check(static_cast<D>(s) >= 0);
  constexpr auto umax = __private::into_unsigned(__private::max_value<D>());
  if constexpr (MAX_PRIMITIVE < umax) {
    ::sus::check(__private::into_unsigned(static_cast<D>(s)) <= MAX_PRIMITIVE);
  }
  return _self(static_cast<_primitive>(s));
}

/// Tries to construct a _self from a signed enum type (or enum class).
///
/// Returns an error if the source value is outside of the range of _self.
///
/// #[doc.overloads=unsigned.tryfrom.signedenum]
template <class S>
  requires(SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>)
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(S s);

/// Constructs a _self from an unsigned primitive integer type (unsigned
/// int, unsigned long, etc).
///
/// # Panics
/// The function will panic if the input value is out of range for _self.
///
/// #[doc.overloads=unsigned.from.unsignedprimitive]
template <UnsignedPrimitiveInteger U>
sus_pure static constexpr _self from(U u) {
  if constexpr (MAX_PRIMITIVE < __private::max_value<U>())
    ::sus::check(u <= MAX_PRIMITIVE);
  return _self(static_cast<_primitive>(u));
}

/// Tries to construct a _self from an unsigned primitive integer type
/// (unsigned int, unsigned long, etc).
///
/// Returns an error if the source value is outside of the range of _self.
///
/// #[doc.overloads=unsigned.tryfrom.unsignedprimitive]
template <UnsignedPrimitiveInteger U>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u);

/// Constructs a _self from an unsigned enum type (or enum class).
///
/// # Panics
/// The function will panic if the input value is out of range for _self.
///
/// #[doc.overloads=unsigned.from.unsignedenum]
template <class U>
  requires(UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>)
sus_pure static constexpr _self from(U u) {
  using D = std::underlying_type_t<U>;
  if constexpr (MAX_PRIMITIVE < __private::max_value<D>())
    ::sus::check(static_cast<D>(u) <= MAX_PRIMITIVE);
  return _self(static_cast<_primitive>(u));
}

/// Tries to construct a _self from an unsigned enum type (or enum class).
///
/// Returns an error if the source value is outside of the range of _self.
///
/// #[doc.overloads=unsigned.tryfrom.unsignedenum]
template <class U>
  requires(UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>)
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u);

/// Constructs a _self from a signed or unsigned integer type (i8, i16, u32,
/// u64 etc).
///
/// # Safety
/// If the input value is out of range for _self, the value will be
/// truncated, which may lead to application bugs and memory unsafety.
///
/// #[doc.overloads=unsigned.from.unchecked.int]
template <Integer I>
sus_pure static constexpr _self from_unchecked(::sus::marker::UnsafeFnMarker,
                                               I i) noexcept {
  return _self(static_cast<_primitive>(i.primitive_value));
}

/// Constructs a _self from a signed or unsigned integer primitive type
/// (int, long, unsigned int, etc).
///
/// # Safety
/// If the input value is out of range for _self, the value will be
/// truncated, which may lead to application bugs and memory unsafety.
///
/// #[doc.overloads=unsigned.from.unchecked.primitive]
template <PrimitiveInteger P>
sus_pure static constexpr _self from_unchecked(::sus::marker::UnsafeFnMarker,
                                               P p) noexcept {
  return _self(static_cast<_primitive>(p));
}

/// Constructs a _self from a signed or unsigned enum type
/// (or enum class).
///
/// # Safety
/// If the input value is out of range for _self, the value will be
/// truncated, which may lead to application bugs and memory unsafety.
///
/// #[doc.overloads=unsigned.from.unchecked.enum]
template <class P>
  requires(PrimitiveEnum<P> || PrimitiveEnumClass<P>)
sus_pure static constexpr _self from_unchecked(::sus::marker::UnsafeFnMarker,
                                               P p) noexcept {
  return _self(static_cast<_primitive>(p));
}

template <UnsignedPrimitiveInteger U>
  requires(::sus::mem::size_of<U>() >= ::sus::mem::size_of<_primitive>())
sus_pure constexpr inline explicit operator U() const {
  return primitive_value;
}
template <SignedPrimitiveInteger U>
  requires(::sus::mem::size_of<U>() > ::sus::mem::size_of<_primitive>())
sus_pure constexpr inline explicit operator U() const {
  return primitive_value;
}

/// sus::ops::Eq<##_self##> trait.
/// #[doc.overloads=uint.eq.self]
[[nodiscard]] friend sus_pure constexpr inline bool operator==(
    const _self& l, const _self& r) noexcept = default;
/// sus::ops::Eq<##_self##, UnsignedPrimitiveInteger> trait.
/// #[doc.overloads=uint.eq.unsignedprimitive]
template <UnsignedPrimitiveInteger P>
[[nodiscard]] friend sus_pure constexpr inline bool operator==(
    const _self& l, const P& r) noexcept {
  return l.primitive_value == r;
}
/// sus::ops::Eq<##_self##, Unsigned> trait.
/// #[doc.overloads=uint.eq.unsigned]
template <Unsigned U>
[[nodiscard]] friend sus_pure constexpr inline bool operator==(
    const _self& l, const U& r) noexcept {
  return l.primitive_value == r.primitive_value;
}
/// sus::ops::Ord<##_self##> trait.
/// #[doc.overloads=uint.ord.self]
template <UnsignedPrimitiveInteger P>
[[nodiscard]] friend sus_pure constexpr inline std::strong_ordering operator<=>(
    const _self& l, const _self& r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}
/// sus::ops::Ord<##_self##, UnsignedPrimitiveInteger> trait.
/// #[doc.overloads=uint.ord.unsignedprimitive]
template <UnsignedPrimitiveInteger P>
[[nodiscard]] friend sus_pure constexpr inline std::strong_ordering operator<=>(
    const _self& l, const P& r) noexcept {
  return l.primitive_value <=> r;
}
/// sus::ops::Ord<##_self##, Unsigned> trait.
/// #[doc.overloads=uint.ord.unsigned]
template <Unsigned U>
[[nodiscard]] friend sus_pure constexpr inline std::strong_ordering operator<=>(
    const _self& l, const U& r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}

// sus::num::Neg trait intentionally omitted.

/// sus::num::BitNot trait.
sus_pure constexpr inline _self operator~() const& noexcept {
  return __private::unchecked_not(primitive_value);
}

/// sus::num::Add<##_self##> trait.
///
/// #[doc.overloads=uint##_self##.+]
[[nodiscard]] friend sus_pure constexpr inline _self operator+(
    const _self& l, const _self& r) noexcept {
  const auto out =
      __private::add_with_overflow(l.primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  return out.value;
}
/// sus::num::Sub<##_self##> trait.
///
/// #[doc.overloads=uint##_self##.-]
[[nodiscard]] friend sus_pure constexpr inline _self operator-(
    const _self& l, const _self& r) noexcept {
  const auto out =
      __private::sub_with_overflow(l.primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  return out.value;
}
/// sus::num::Mul<##_self##> trait.
///
/// #[doc.overloads=uint##_self##.*]
[[nodiscard]] friend sus_pure constexpr inline _self operator*(
    const _self& l, const _self& r) noexcept {
  const auto out =
      __private::mul_with_overflow(l.primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  return out.value;
}
/// sus::num::Div<##_self##> trait.
///
/// #[doc.overloads=uint##_self##./]
[[nodiscard]] friend sus_pure constexpr inline _self operator/(
    const _self& l, const _self& r) noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r.primitive_value != 0u);
  return __private::unchecked_div(l.primitive_value, r.primitive_value);
}
/// sus::num::Rem<##_self##> trait.
///
/// #[doc.overloads=uint##_self##.%]
[[nodiscard]] friend sus_pure constexpr inline _self operator%(
    const _self& l, const _self& r) noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r.primitive_value != 0u);
  return __private::unchecked_rem(l.primitive_value, r.primitive_value);
}

/// sus::num::BitAnd<##_self##> trait.
///
/// #[doc.overloads=uint##_self##.&]
[[nodiscard]] friend sus_pure constexpr inline _self operator&(
    const _self& l, const _self& r) noexcept {
  return __private::unchecked_and(l.primitive_value, r.primitive_value);
}
/// sus::num::BitOr<##_self##> trait.
///
/// #[doc.overloads=uint##_self##.|]
[[nodiscard]] friend sus_pure constexpr inline _self operator|(
    const _self& l, const _self& r) noexcept {
  return __private::unchecked_or(l.primitive_value, r.primitive_value);
}
/// sus::num::BitXor<##_self##> trait.
///
/// #[doc.overloads=uint##_self##.^]
[[nodiscard]] friend sus_pure constexpr inline _self operator^(
    const _self& l, const _self& r) noexcept {
  return __private::unchecked_xor(l.primitive_value, r.primitive_value);
}
/// sus::num::Shl trait.
///
/// #[doc.overloads=uint##_self##.<<]
[[nodiscard]] friend sus_pure constexpr inline _self operator<<(
    const _self& l, const u32& r) noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r < BITS);
  return __private::unchecked_shl(l.primitive_value, r.primitive_value);
}
/// sus::num::Shr trait.
///
/// #[doc.overloads=uint##_self##>>]
[[nodiscard]] friend sus_pure constexpr inline _self operator>>(
    const _self& l, const u32& r) noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r < BITS);
  return __private::unchecked_shr(l.primitive_value, r.primitive_value);
}

/// sus::num::AddAssign<##_self##> trait.
constexpr inline void operator+=(_self r) & noexcept {
  const auto out =
      __private::add_with_overflow(primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
/// sus::num::SubAssign<##_self##> trait.
constexpr inline void operator-=(_self r) & noexcept {
  const auto out =
      __private::sub_with_overflow(primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
/// sus::num::MulAssign<##_self##> trait.
constexpr inline void operator*=(_self r) & noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
/// sus::num::DivAssign<##_self##> trait.
constexpr inline void operator/=(_self r) & noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r.primitive_value != 0u);
  primitive_value /= r.primitive_value;
}
/// sus::num::RemAssign<##_self##> trait.
constexpr inline void operator%=(_self r) & noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r.primitive_value != 0u);
  primitive_value %= r.primitive_value;
}

/// sus::num::BitAndAssign<##_self##> trait.
constexpr inline void operator&=(_self r) & noexcept {
  primitive_value &= r.primitive_value;
}
/// sus::num::BitOrAssign<##_self##> trait.
constexpr inline void operator|=(_self r) & noexcept {
  primitive_value |= r.primitive_value;
}
/// sus::num::BitXorAssign<##_self##> trait.
constexpr inline void operator^=(_self r) & noexcept {
  primitive_value ^= r.primitive_value;
}
/// sus::num::ShlAssign trait.
constexpr inline void operator<<=(const u32& r) & noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r < BITS);
  primitive_value <<= r.primitive_value;
}
/// sus::num::ShrAssign trait.
constexpr inline void operator>>=(const u32& r) & noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r < BITS);
  primitive_value >>= r.primitive_value;
}

/// Computes the absolute difference between self and other.
sus_pure constexpr _self abs_diff(const _self& r) const& noexcept {
  if (primitive_value >= r.primitive_value)
    return __private::unchecked_sub(primitive_value, r.primitive_value);
  else
    return __private::unchecked_sub(r.primitive_value, primitive_value);
}

/// Checked integer addition. Computes self + rhs, returning None if overflow
/// occurred.
sus_pure constexpr ::sus::option::Option<_self> checked_add(
    const _self& rhs) const& noexcept;

/// Checked integer addition with an unsigned rhs. Computes self + rhs,
/// returning None if overflow occurred.
template <std::convertible_to<_signed> S>
sus_pure constexpr ::sus::option::Option<_self> checked_add_signed(
    const S& rhs) const& noexcept;

/// Calculates self + rhs
///
/// Returns a tuple of the addition along with a boolean indicating whether an
/// arithmetic overflow would occur. If an overflow would have occurred then the
/// wrapped value is returned.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_add(const _self& rhs) const& noexcept {
  const auto out =
      __private::add_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple::with(out.value, out.overflow);
}

/// Calculates self + rhs with an unsigned rhs
///
/// Returns a tuple of the addition along with a boolean indicating whether an
/// arithmetic overflow would occur. If an overflow would have occurred then the
/// wrapped value is returned.
template <std::convertible_to<_signed> S, int&...,
          class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_add_signed(const S& rhs) const& noexcept {
  const auto r =
      __private::add_with_overflow_signed(primitive_value, rhs.primitive_value);
  return Tuple::with(r.value, r.overflow);
}

/// Saturating integer addition. Computes self + rhs, saturating at the numeric
/// bounds instead of overflowing.
sus_pure constexpr _self saturating_add(const _self& rhs) const& noexcept {
  return __private::saturating_add(primitive_value, rhs.primitive_value);
}

/// Saturating integer addition with an unsigned rhs. Computes self + rhs,
/// saturating at the numeric bounds instead of overflowing.
template <std::convertible_to<_signed> S>
sus_pure constexpr _self saturating_add_signed(const S& rhs) const& noexcept {
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

/// Unchecked integer addition. Computes self + rhs, assuming overflow cannot
/// occur.
///
/// # Safety
/// This function is allowed to result in undefined behavior when `self + rhs >
/// ##_self##::MAX` or `self + rhs < ##_self##::MIN`, i.e. when `checked_add()`
/// would return None.
sus_pure inline constexpr _self unchecked_add(
    ::sus::marker::UnsafeFnMarker, const _self& rhs) const& noexcept {
  return __private::unchecked_add(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) addition. Computes self + rhs, wrapping around at the
/// boundary of the type.
sus_pure constexpr _self wrapping_add(const _self& rhs) const& noexcept {
  return __private::wrapping_add(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) addition with an unsigned rhs. Computes self + rhs,
/// wrapping around at the boundary of the type.
template <std::convertible_to<_signed> S>
sus_pure constexpr _self wrapping_add_signed(const S& rhs) const& noexcept {
  return __private::add_with_overflow_signed(primitive_value,
                                             rhs.primitive_value)
      .value;
}

/// Checked integer division. Computes self / rhs, returning None if `rhs == 0`.
sus_pure constexpr ::sus::option::Option<_self> checked_div(
    const _self& rhs) const& noexcept;

/// Calculates the divisor when self is divided by rhs.
///
/// Returns a tuple of the divisor along with a boolean indicating whether an
/// arithmetic overflow would occur. Note that for unsigned integers overflow
/// never occurs, so the second value is always false.
///
/// # Panics
/// This function will panic if `rhs` is 0.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_div(const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return Tuple::with(
      __private::unchecked_div(primitive_value, rhs.primitive_value), false);
}

/// Saturating integer division. Computes self / rhs, saturating at the  numeric
/// bounds instead of overflowing.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self saturating_div(const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return __private::unchecked_div(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) division. Computes self / rhs. Wrapped division on
/// unsigned types is just normal division. There's no way wrapping could ever
/// happen. This function exists, so that all operations are accounted for in
/// the wrapping operations.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self wrapping_div(const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return __private::unchecked_div(primitive_value, rhs.primitive_value);
}

/// Checked integer multiplication. Computes `self * rhs`, returning None if
/// overflow occurred.
sus_pure constexpr ::sus::option::Option<_self> checked_mul(
    const _self& rhs) const& noexcept;

/// Calculates the multiplication of self and `rhs`.
///
/// Returns a tuple of the multiplication along with a boolean indicating
/// whether an arithmetic overflow would occur. If an overflow would have
/// occurred then the wrapped value is returned.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_mul(const _self& rhs) const& noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple::with(out.value, out.overflow);
}

/// Saturating integer multiplication. Computes `self * rhs`, saturating at the
/// numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_mul(const _self& rhs) const& noexcept {
  return __private::saturating_mul(primitive_value, rhs.primitive_value);
}

/// Unchecked integer multiplication. Computes `self * rhs`, assuming overflow
/// cannot occur.
///
/// # Safety
/// This function is allowed to result in undefined behavior when `self * rhs >
/// ##_self##::MAX` or `self * rhs < ##_self##::MIN`, i.e. when `checked_mul()`
/// would return None.
sus_pure constexpr _self unchecked_mul(::sus::marker::UnsafeFnMarker,
                                       const _self& rhs) const& noexcept {
  return __private::unchecked_mul(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) multiplication. Computes self * rhs, wrapping around
/// at the boundary of the type.
sus_pure constexpr _self wrapping_mul(const _self& rhs) const& noexcept {
  return __private::wrapping_mul(primitive_value, rhs.primitive_value);
}

/// Checked negation. Computes -self, returning None unless `self == 0`.
///
/// Note that negating any positive integer will overflow.
sus_pure constexpr ::sus::option::Option<_self> checked_neg() const& noexcept;

/// Negates self in an overflowing fashion.
///
/// Returns `~self + 1` using wrapping operations to return the value that
/// represents the negation of this unsigned value. Note that for positive
/// unsigned values overflow always occurs, but negating 0 does not overflow.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_neg() const& noexcept {
  return Tuple::with((~(*this)).wrapping_add(_self(_primitive{1u})),
                     primitive_value != 0u);
}

/// Wrapping (modular) negation. Computes `-self`, wrapping around at the
/// boundary of the type.
///
/// Since unsigned types do not have negative equivalents all applications
/// of this function will wrap (except for -0). For values smaller than the
/// corresponding signed type's maximum the result is the same as casting
/// the corresponding signed value. Any larger values are equivalent to
/// `MAX + 1 - (val - MAX - 1)` where MAX is the corresponding signed type's
/// maximum.
sus_pure constexpr _self wrapping_neg() const& noexcept {
  return (_self(_primitive{0u})).wrapping_sub(*this);
}

/// Checked integer remainder. Computes `self % rhs`, returning None if `rhs ==
/// 0`.
sus_pure constexpr ::sus::option::Option<_self> checked_rem(
    const _self& rhs) const& noexcept;

/// Calculates the remainder when self is divided by rhs.
///
/// Returns a tuple of the remainder after dividing along with a boolean
/// indicating whether an arithmetic overflow would occur. Note that for
/// unsigned integers overflow never occurs, so the second value is always
/// false.
///
/// # Panics
/// This function will panic if `rhs` is 0.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_rem(const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return Tuple::with(
      __private::unchecked_rem(primitive_value, rhs.primitive_value), false);
}

/// Wrapping (modular) remainder. Computes self % rhs. Wrapped remainder
/// calculation on unsigned types is just the regular remainder calculation.
///
/// There's no way wrapping could ever happen. This function exists, so that
/// all operations are accounted for in the wrapping operations.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self wrapping_rem(const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return __private::unchecked_rem(primitive_value, rhs.primitive_value);
}

/// Performs Euclidean division.
///
/// Since, for the positive integers, all common definitions of division are
/// equal, this is exactly equal to self / rhs.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self div_euclid(const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return __private::unchecked_div(primitive_value, rhs.primitive_value);
}

/// Checked Euclidean division. Computes self.div_euclid(rhs), returning
/// None if rhs == 0.
sus_pure constexpr ::sus::option::Option<_self> checked_div_euclid(
    const _self& rhs) const& noexcept;

/// Calculates the quotient of Euclidean division self.div_euclid(rhs).
///
/// Returns a tuple of the divisor along with a boolean indicating whether an
/// arithmetic overflow would occur. Note that for unsigned integers overflow
/// never occurs, so the second value is always false. Since, for the positive
/// integers, all common definitions of division are equal, this is exactly
/// equal to self.overflowing_div(rhs).
///
/// # Panics
/// This function will panic if `rhs` is 0.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_div_euclid(
    const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return Tuple::with(
      __private::unchecked_div(primitive_value, rhs.primitive_value), false);
}

/// Wrapping Euclidean division. Computes self.div_euclid(rhs). Wrapped division
/// on unsigned types is just normal division.
///
/// There's no way wrapping could ever happen. This function exists so that all
/// operations are accounted for in the wrapping operations. Since, for the
/// positive integers, all common definitions of division are equal, this is
/// exactly equal to self.wrapping_div(rhs).
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self wrapping_div_euclid(const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return __private::unchecked_div(primitive_value, rhs.primitive_value);
}

/// Calculates the least remainder of self (mod rhs).
///
/// Since, for the positive integers, all common definitions of division are
/// equal, this is exactly equal to self % rhs. \
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self rem_euclid(const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return __private::unchecked_rem(primitive_value, rhs.primitive_value);
}

/// Checked Euclidean modulo. Computes self.rem_euclid(rhs), returning None if
/// `rhs == 0`.
sus_pure constexpr ::sus::option::Option<_self> checked_rem_euclid(
    const _self& rhs) const& noexcept;

/// Calculates the remainder self.rem_euclid(rhs) as if by Euclidean division.
///
/// Returns a tuple of the modulo after dividing along with a boolean indicating
/// whether an arithmetic overflow would occur. Note that for unsigned integers
/// overflow never occurs, so the second value is always false. Since, for the
/// positive integers, all common definitions of division are equal, this
/// operation is exactly equal to self.overflowing_rem(rhs).
///
/// # Panics
/// This function will panic if `rhs` is 0.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_rem_euclid(
    const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return Tuple::with(
      __private::unchecked_rem(primitive_value, rhs.primitive_value), false);
}

/// Wrapping Euclidean modulo. Computes self.rem_euclid(rhs). Wrapped modulo
/// calculation on unsigned types is just the regular remainder calculation.
///
/// There’s no way wrapping could ever happen. This function exists, so that all
/// operations are accounted for in the wrapping operations. Since, for the
/// positive integers, all common definitions of division are equal, this is
/// exactly equal to self.wrapping_rem(rhs). \
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self wrapping_rem_euclid(const _self& rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return __private::unchecked_rem(primitive_value, rhs.primitive_value);
}

/// Checked shift left. Computes `*this << rhs`, returning None if rhs is
/// larger than or equal to the number of bits in self.
sus_pure constexpr ::sus::option::Option<_self> checked_shl(
    const u32& rhs) const& noexcept;

/// Shifts self left by rhs bits.
///
/// Returns a tuple of the shifted version of self along with a boolean
/// indicating whether the shift value was larger than or equal to the number of
/// bits. If the shift value is too large, then value is masked (N-1) where N is
/// the number of bits, and this value is then used to perform the shift.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_shl(const u32& rhs) const& noexcept {
  const auto out =
      __private::shl_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple::with(out.value, out.overflow);
}

/// Panic-free bitwise shift-left; yields `*this << mask(rhs)`, where mask
/// removes any high-order bits of `rhs` that would cause the shift to exceed
/// the bitwidth of the type.
///
/// Note that this is not the same as a rotate-left; the RHS of a wrapping
/// shift-left is restricted to the range of the type, rather than the bits
/// shifted out of the LHS being returned to the other end. The primitive
/// integer types all implement a rotate_left function, which may be what you
/// want instead.
sus_pure constexpr _self wrapping_shl(const u32& rhs) const& noexcept {
  return __private::shl_with_overflow(primitive_value, rhs.primitive_value)
      .value;
}

/// Checked shift right. Computes `*this >> rhs`, returning None if rhs is
/// larger than or equal to the number of bits in self.
sus_pure constexpr ::sus::option::Option<_self> checked_shr(
    const u32& rhs) const& noexcept;

/// Shifts self right by rhs bits.
///
/// Returns a tuple of the shifted version of self along with a boolean
/// indicating whether the shift value was larger than or equal to the number of
/// bits. If the shift value is too large, then value is masked (N-1) where N is
/// the number of bits, and this value is then used to perform the shift.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_shr(const u32& rhs) const& noexcept {
  const auto out =
      __private::shr_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple::with(out.value, out.overflow);
}

/// Panic-free bitwise shift-right; yields `*this >> mask(rhs)`, where mask
/// removes any high-order bits of `rhs` that would cause the shift to exceed
/// the bitwidth of the type.
///
/// Note that this is not the same as a rotate-right; the RHS of a wrapping
/// shift-right is restricted to the range of the type, rather than the bits
/// shifted out of the LHS being returned to the other end. The primitive
/// integer types all implement a rotate_right function, which may be what you
/// want instead.
sus_pure constexpr _self wrapping_shr(const u32& rhs) const& noexcept {
  return __private::shr_with_overflow(primitive_value, rhs.primitive_value)
      .value;
}

/// Checked integer subtraction. Computes self - rhs, returning None if overflow
/// occurred.
sus_pure constexpr ::sus::option::Option<_self> checked_sub(
    const _self& rhs) const&;

/// Calculates `self - rhs`.
///
/// Returns a tuple of the subtraction along with a boolean indicating whether
/// an arithmetic overflow would occur. If an overflow would have occurred then
/// the wrapped value is returned.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_sub(const _self& rhs) const& noexcept {
  const auto out =
      __private::sub_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple::with(out.value, out.overflow);
}

/// Saturating integer subtraction. Computes self - rhs, saturating at the
/// numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_sub(const _self& rhs) const& {
  return __private::saturating_sub(primitive_value, rhs.primitive_value);
}

/// Unchecked integer subtraction. Computes self - rhs, assuming overflow cannot
/// occur.
///
/// # Safety
/// This function is allowed to result in undefined behavior when `self
/// - rhs > ##_self##::MAX` or `self - rhs < ##_self##::MIN`, i.e. when
/// `checked_sub()` would return None.
sus_pure constexpr _self unchecked_sub(::sus::marker::UnsafeFnMarker,
                                       const _self& rhs) const& noexcept {
  return __private::unchecked_sub(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) subtraction. Computes self - rhs, wrapping around at the
/// boundary of the type.
sus_pure constexpr _self wrapping_sub(const _self& rhs) const& noexcept {
  return __private::wrapping_sub(primitive_value, rhs.primitive_value);
}

/// Returns the number of ones in the binary representation of the current
/// value.
sus_pure constexpr u32 count_ones() const& noexcept {
  return __private::count_ones(primitive_value);
}

/// Returns the number of zeros in the binary representation of the current
/// value.
sus_pure constexpr u32 count_zeros() const& noexcept {
  return (~(*this)).count_ones();
}

/// Returns the number of leading ones in the binary representation of the
/// current value.
sus_pure constexpr u32 leading_ones() const& noexcept {
  return (~(*this)).leading_zeros();
}

/// Returns the number of leading zeros in the binary representation of the
/// current value.
sus_pure constexpr u32 leading_zeros() const& noexcept {
  return __private::leading_zeros(primitive_value);
}

/// Returns the number of trailing ones in the binary representation of the
/// current value.
sus_pure constexpr u32 trailing_ones() const& noexcept {
  return (~(*this)).trailing_zeros();
}

/// Returns the number of trailing zeros in the binary representation of the
/// current value.
sus_pure constexpr u32 trailing_zeros() const& noexcept {
  return __private::trailing_zeros(primitive_value);
}

/// Reverses the order of bits in the integer. The least significant bit becomes
/// the most significant bit, second least-significant bit becomes second
/// most-significant bit, etc.
sus_pure constexpr _self reverse_bits() const& noexcept {
  return __private::reverse_bits(primitive_value);
}

/// Shifts the bits to the left by a specified amount, `n`, wrapping the
/// truncated bits to the end of the resulting integer.
///
/// Please note this isn't the same operation as the `<<` shifting operator!
sus_pure constexpr _self rotate_left(const u32& n) const& noexcept {
  return __private::rotate_left(primitive_value, n.primitive_value);
}

/// Shifts the bits to the right by a specified amount, n, wrapping the
/// truncated bits to the beginning of the resulting integer.
///
/// Please note this isn't the same operation as the >> shifting operator!
sus_pure constexpr _self rotate_right(const u32& n) const& noexcept {
  return __private::rotate_right(primitive_value, n.primitive_value);
}

/// Reverses the byte order of the integer.
sus_pure constexpr _self swap_bytes() const& noexcept {
  return __private::swap_bytes(primitive_value);
}

/// Raises self to the power of `exp`, using exponentiation by squaring.
sus_pure constexpr _self pow(const u32& rhs) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, rhs.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  return out.value;
}

/// Checked exponentiation. Computes `##_self##::pow(exp)`, returning None if
/// overflow occurred.
sus_pure constexpr ::sus::option::Option<_self> checked_pow(
    const u32& rhs) const& noexcept;

/// Raises self to the power of `exp`, using exponentiation by squaring.
///
/// Returns a tuple of the exponentiation along with a bool indicating whether
/// an overflow happened.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_pow(const u32& exp) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, exp.primitive_value);
  return Tuple::with(out.value, out.overflow);
}

/// Wrapping (modular) exponentiation. Computes self.pow(exp), wrapping around
/// at the boundary of the type.
sus_pure constexpr _self wrapping_pow(const u32& exp) const& noexcept {
  return __private::wrapping_pow(primitive_value, exp.primitive_value);
}

/// Returns the base 2 logarithm of the number, rounded down.
///
/// Returns None if the number is zero.
sus_pure constexpr ::sus::option::Option<u32> checked_log2() const& noexcept;

/// Returns the base 2 logarithm of the number, rounded down.
///
/// # Panics
/// When the number is zero the function will panic.
sus_pure constexpr u32 log2() const& noexcept;

/// Returns the base 10 logarithm of the number, rounded down.
///
/// Returns None if the number is zero.
sus_pure constexpr ::sus::option::Option<u32> checked_log10() const& noexcept;

/// Returns the base 10 logarithm of the number, rounded down.
///
/// # Panics
/// When the number is zero the function will panic.
sus_pure constexpr u32 log10() const& noexcept;

/// Returns the logarithm of the number with respect to an arbitrary base,
/// rounded down.
///
/// Returns None if the number is zero, or if the base is not at least 2.
///
/// This method might not be optimized owing to implementation details;
/// `checked_log2` can produce results more efficiently for base 2, and
/// `checked_log10` can produce results more efficiently for base 10.
sus_pure constexpr ::sus::option::Option<u32> checked_log(
    const _self& base) const& noexcept;

/// Returns the logarithm of the number with respect to an arbitrary base,
/// rounded down.
///
/// This method might not be optimized owing to implementation details; log2 can
/// produce results more efficiently for base 2, and log10 can produce results
/// more efficiently for base 10.
///
/// # Panics
/// When the number is zero, or if the base is not at least 2, the function will
/// panic.
sus_pure constexpr u32 log(const _self& base) const& noexcept;

/// Returns `true` if and only if `self == 2^k` for some `k`.
sus_pure constexpr bool is_power_of_two() const& noexcept {
  return count_ones() == 1;
}

/// Returns the smallest power of two greater than or equal to self.
///
/// # Panics
/// The function panics when the return value overflows (i.e.,
/// `self > (1 << (N-1))` for type uN).
sus_pure constexpr _self next_power_of_two() const& noexcept {
  const auto one_less =
      __private::one_less_than_next_power_of_two(primitive_value);
  return _self(one_less) + _self(_primitive{1u});
}

/// Returns the smallest power of two greater than or equal to n.
///
/// If the next power of two is greater than the type's maximum value, None is
/// returned, otherwise the power of two is wrapped in Some.
sus_pure constexpr ::sus::option::Option<_self> checked_next_power_of_two()
    const& noexcept;

/// Returns the smallest power of two greater than or equal to n.
///
/// If the next power of two is greater than the type's maximum value, the
/// return value is wrapped to 0.
sus_pure constexpr _self wrapping_next_power_of_two() const& noexcept {
  const auto one_less =
      __private::one_less_than_next_power_of_two(primitive_value);
  return _self(one_less).wrapping_add(_self(_primitive{1u}));
}

/// Converts an integer from big endian to the target's endianness.
///
/// On big endian this is a no-op. On little endian the bytes are swapped.
[[nodiscard]] static sus_pure constexpr _self from_be(const _self& x) noexcept {
  if constexpr (::sus::assertions::is_big_endian())
    return x;
  else
    return x.swap_bytes();
}

/// Converts an integer from little endian to the target's endianness.
///
/// On little endian this is a no-op. On big endian the bytes are swapped.
[[nodiscard]] static sus_pure constexpr _self from_le(const _self& x) noexcept {
  if constexpr (::sus::assertions::is_little_endian())
    return x;
  else
    return x.swap_bytes();
}

/// Converts self to big endian from the target's endianness.
///
/// On big endian this is a no-op. On little endian the bytes are swapped.
sus_pure constexpr _self to_be() const& noexcept {
  if constexpr (::sus::assertions::is_big_endian())
    return *this;
  else
    return swap_bytes();
}

/// Converts self to little endian from the target's endianness.
///
/// On little endian this is a no-op. On big endian the bytes are swapped.
sus_pure constexpr _self to_le() const& noexcept {
  if constexpr (::sus::assertions::is_little_endian())
    return *this;
  else
    return swap_bytes();
}

/// Return the memory representation of this integer as a byte array in
/// big-endian (network) byte order.
sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
to_be_bytes() const& noexcept;

/// Return the memory representation of this integer as a byte array in
/// little-endian byte order.
sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
to_le_bytes() const& noexcept;

/// Return the memory representation of this integer as a byte array in native
/// byte order.
///
/// As the target platform's native endianness is used, portable code should use
/// `to_be_bytes()` or `to_le_bytes()`, as appropriate, instead.
sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
to_ne_bytes() const& noexcept;

/// Create an integer value from its representation as a byte array in big
/// endian.
[[nodiscard]] static sus_pure constexpr _self from_be_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

/// Create an integer value from its representation as a byte array in little
/// endian.
[[nodiscard]] static sus_pure constexpr _self from_le_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

/// Create an integer value from its memory representation as a byte array in
/// native endianness.
///
/// As the target platform's native endianness is used, portable code likely
/// wants to use `from_be_bytes()` or `from_le_bytes()`, as appropriate instead.
[[nodiscard]] static sus_pure constexpr _self from_ne_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

#undef _self
#undef _primitive
#undef _signed
