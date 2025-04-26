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
// Include it into the body of your class.
//
// Define `_self` to the name of the integer type.
// Define `_primitive` to the primitive it holds inside.
// Define `_unsigned` to the unsigned integer type of the same size.
///////////////////////////////////////////////////////////////////////////

/// The inner primitive value. Prefer to cast to the desired primitive type,
/// such as with `int32_t{n}` for a numeric value `n`.
_primitive primitive_value{0};

/// The smallest value that can be represented by an
/// [`@doc.self`]($sus::num::@doc.self), as a native C++ primitive.
static constexpr auto MIN_PRIMITIVE = __private::min_value<_primitive>();
/// The largest value that can be represented by an
/// [`@doc.self`]($sus::num::@doc.self), as a native C++ primitive.
static constexpr auto MAX_PRIMITIVE = __private::max_value<_primitive>();
/// The smallest value that can be represented by an
/// [`@doc.self`]($sus::num::@doc.self).
static const _self MIN;
/// The largest value that can be represented by an
/// [`@doc.self`]($sus::num::@doc.self).
static const _self MAX;
/// The size of and [`@doc.self`]($sus::num::@doc.self) in bits.
static const u32 BITS;

/// Default constructor, which sets the integer to 0.
///
/// Satisfies the [`Default`]($sus::construct::Default) concept.
///
/// The trivial copy and move constructors are implicitly declared, as is the
/// trivial destructor.
constexpr inline _self() noexcept = default;

/// Construction from signed types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed). For lossy conversions use the
/// [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.ctor.signedint.typed]
template <Signed S>
  requires(::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(S v) noexcept : primitive_value(v.primitive_value) {}

/// Construction from signed primitive types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed.primitive). For lossy conversions use
/// the [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.ctor.signedint.primitive]
template <SignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from signed enum types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed.enum). For lossy conversions use the
/// [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.ctor.signedenum]
template <SignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from signed enum class types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed.enum). For lossy conversions use
/// the [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.ctor.signedenumclass]
template <SignedPrimitiveEnumClass P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
explicit constexpr inline _self(P v) noexcept
    : primitive_value(static_cast<_primitive>(v)) {}

/// Construction from unsigned types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned). For lossy conversions use the
/// [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.ctor.unsignedint.typed]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() < ::sus::mem::size_of<_primitive>())
constexpr inline _self(U v) noexcept : primitive_value(v.primitive_value) {}

/// Construction from unsigned primitive types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.primitive). For lossy conversions use
/// the [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.ctor.unsignedint.primitive]
template <UnsignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from unsigned enum types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.enum). For lossy conversions use the
/// [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.ctor.unsignedenum]
template <UnsignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from unsigned enum class types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.enum). For lossy conversions
/// use the [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.ctor.unsignedenumclass]
template <UnsignedPrimitiveEnumClass P>
  requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<_primitive>())
explicit constexpr inline _self(P v) noexcept
    : primitive_value(static_cast<_primitive>(v)) {}

/// Assignment from signed types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed). For lossy conversions use the
/// [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.assign.signedint.typed]
template <Signed S>
  requires(::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(S v) noexcept {
  primitive_value = v.primitive_value;
  return *this;
}

/// Assignment from signed primitive types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed.primitive). For lossy conversions use
/// the [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.assign.signedint.primitive]
template <SignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(P v) noexcept {
  primitive_value = v;
  return *this;
}

/// Assignment from signed enum types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed.enum). For lossy conversions use the
/// [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.assign.signedenum]
template <SignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(P v) noexcept {
  primitive_value = v;
  return *this;
}

/// Constructs a `@doc.self` from a signed integer type (i8, i16, i32, etc)
/// where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed). For lossy conversions use the
/// [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=from.signed]
template <Signed S>
  requires(::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>())
_sus_pure static constexpr _self from(S s) noexcept {
  return _self(s);
}

/// Try to construct a @doc.self from a signed integer type (i8, i16, i32, etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signed]
template <Signed S>
_sus_pure static constexpr ::sus::result::Result<_self,
                                                 ::sus::num::TryFromIntError>
try_from(S s) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MIN_PRIMITIVE > S::MIN_PRIMITIVE) {
    if (s.primitive_value < MIN_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  if constexpr (MAX_PRIMITIVE < S::MAX_PRIMITIVE) {
    if (s.primitive_value > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(s.primitive_value)));
}

/// Constructs a `@doc.self` from an unsigned integer type (unsigned
/// int, unsigned long, etc) where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned). For lossy conversions use the
/// [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=from.unsigned]
template <Unsigned S>
  requires(::sus::mem::size_of<S>() < ::sus::mem::size_of<_primitive>())
_sus_pure static constexpr _self from(S s) noexcept {
  return _self(s);
}

/// Try to construct a `@doc.self` from an unsigned integer type (`u8`, `u16`,
/// `u32`, etc).
///
/// Returns an error if the source value is outside of the range of `@doc.self`.
///
/// #[doc.overloads=unsigned]
template <Unsigned U>
_sus_pure static constexpr ::sus::result::Result<_self,
                                                 ::sus::num::TryFromIntError>
try_from(U u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);
  if constexpr (umax < U::MAX_PRIMITIVE) {
    if (u.primitive_value > umax) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(u.primitive_value)));
}

/// Constructs a `@doc.self` from a signed primitive integer type (int, long, etc)
/// where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed.primitive). For lossy
/// conversions use
/// the [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=from.signed.prim]
template <SignedPrimitiveInteger S>
  requires(::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>())
_sus_pure static constexpr _self from(S s) noexcept {
  return _self(s);
}

/// Tries to construct a `@doc.self` from a signed primitive integer type (int,
/// long, etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signed.primitive]
template <SignedPrimitiveInteger S>
_sus_pure static constexpr ::sus::result::Result<_self,
                                                 ::sus::num::TryFromIntError>
try_from(S s) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MIN_PRIMITIVE > __private::min_value<S>()) {
    if (s < MIN_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  if constexpr (MAX_PRIMITIVE < __private::max_value<S>()) {
    if (s > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(s)));
}

/// Constructs a `@doc.self` from a signed enum type (or enum class) where no bits
/// are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!signed.enum). For lossy conversions use
/// the [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.from.signedenum]
template <class S>
  requires((SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>) &&
           ::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>())
_sus_pure static constexpr _self from(S s) noexcept {
  using D = std::underlying_type_t<S>;
  return _self(static_cast<std::underlying_type_t<S>>(s));
}

/// Tries to construct a @doc.self from a signed enum type (or enum class).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signed.enum]
template <class S>
  requires(SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>)
_sus_pure static constexpr ::sus::result::Result<_self,
                                                 ::sus::num::TryFromIntError>
try_from(S s) noexcept {
  using D = std::underlying_type_t<S>;
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  if constexpr (MIN_PRIMITIVE > __private::min_value<D>()) {
    if (static_cast<D>(s) < MIN_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  if constexpr (MAX_PRIMITIVE < __private::max_value<D>()) {
    if (static_cast<D>(s) > MAX_PRIMITIVE) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(s)));
}

/// Constructs a `@doc.self` from an unsigned primitive integer type (unsigned
/// int, unsigned long, etc) where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.primitive). For lossy conversions use
/// the [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=from.unsigned.prim]
template <UnsignedPrimitiveInteger S>
  requires(::sus::mem::size_of<S>() < ::sus::mem::size_of<_primitive>())
_sus_pure static constexpr _self from(S s) noexcept {
  return _self(s);
}

/// Constructs a `@doc.self` from an unsigned primitive integer type (unsigned
/// int, unsigned long, etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=unsigned.primitive]
template <UnsignedPrimitiveInteger U>
_sus_pure static constexpr ::sus::result::Result<_self,
                                                 ::sus::num::TryFromIntError>
try_from(U u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);
  if constexpr (umax < __private::max_value<U>()) {
    if (u > umax) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(u)));
}

/// Constructs a `@doc.self` from an unsigned enum type (or enum class) where no
/// bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.enum). For lossy conversions use the
/// [`Cast`]($sus::construct::Cast) concept with
/// [`sus::cast<@doc.self>(x)`]($sus::construct::cast).
///
/// #[doc.overloads=signedint.from.unsignedenum]
template <class S>
  requires((UnsignedPrimitiveEnum<S> || UnsignedPrimitiveEnumClass<S>) &&
           ::sus::mem::size_of<S>() < ::sus::mem::size_of<_primitive>())
_sus_pure static constexpr _self from(S s) noexcept {
  using D = std::underlying_type_t<S>;
  return _self(static_cast<std::underlying_type_t<S>>(s));
}

/// Constructs a `@doc.self` from an unsigned enum type (or enum class).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=unsigned.enum]
template <class U>
  requires(UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>)
_sus_pure static constexpr ::sus::result::Result<_self,
                                                 ::sus::num::TryFromIntError>
try_from(U u) noexcept {
  using D = std::underlying_type_t<U>;
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);
  if constexpr (umax < __private::max_value<D>()) {
    if (static_cast<D>(u) > umax) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R(_self(static_cast<_primitive>(u)));
}

/// Constructs a `@doc.self` from an `Iterator` by computing the product of all
/// elements in the itertor.
///
/// This method should rarely be called directly, as it is used to satisfy the
/// [`Product`]($sus::iter::Product) concept so that
/// [`Iterator::product`]($sus::iter::IteratorBase::product) can be
/// called for iterators over @doc.self.
///
/// To handle overflow without panicing, instead of `iter.product()`, use
/// `iter.product<OverflowInteger<@doc.self>>()`.
///
/// # Panics
/// This method will panic if the product of all values overflows and overflow
/// checks are enabled (they are by default), and will wrap if overflow checks
/// are disabled (not the default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
static constexpr _self from_product(
    ::sus::iter::Iterator<_self> auto&& it) noexcept
  requires(::sus::mem::IsMoveRef<decltype(it)>)
{
  auto p = _self(_primitive{1});
  for (_self i : ::sus::move(it)) p *= i;
  return p;
}

/// Constructs a `@doc.self` from an `Iterator` by computing the sum of all
/// elements in the itertor.
///
/// This method should rarely be called directly, as it is used to satisfy the
/// [`Sum`]($sus::iter::Sum) concept so that
/// [`Iterator::sum`]($sus::iter::IteratorBase::sum) can be called for
/// iterators over @doc.self.
///
/// To handle overflow without panicing, instead of `iter.sum()`, use
/// `iter.sum<OverflowInteger<@doc.self>>()`.
///
/// # Panics
/// This method will panic if the sum of all values overflows and overflow
/// checks are enabled (they are by default), and will wrap if overflow checks
/// are disabled (not the default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
static constexpr _self from_sum(::sus::iter::Iterator<_self> auto&& it) noexcept
  requires(::sus::mem::IsMoveRef<decltype(it)>)
{
  auto p = _self(_primitive{0});
  for (_self i : ::sus::move(it)) p += i;
  return p;
}

/// Conversion from the numeric type to a C++ primitive type.
///
/// This converts to signed primitives which are at least as large as the
/// [`@doc.self`]($sus::num::@doc.self).
///
/// ```
/// auto d = int32_t{3_i32};  // Compiles.
/// auto e = int32_t(3_i32);  // Compiles.
/// int32_t f = 3_i32;  // Compiles.
///
/// auto d = uint16_t{3_i32};  // Does not compile.
/// auto e = uint16_t(3_i32);  // Does not compile.
/// uint16_t f = 3_i32;  // Does not compile.
/// ```
///
/// Potentially-lossy type conversions can be forced through the
/// [`Cast`]($sus::construct::Cast) concept, such as
/// `sus::cast<uint32_t>(3_i32)` or `sus::cast<int16_t>(3_i32)`, or even
/// `sus::cast<u32>(3_i32)`.
template <SignedPrimitiveInteger U>
  requires(::sus::mem::size_of<U>() >= ::sus::mem::size_of<_primitive>())
_sus_pure constexpr inline operator U() const noexcept {
  return primitive_value;
}

/// Returns a mutable pointer to the underlying C++ primitive value type.
///
/// This allows Subspace numerics be used with APIs that expect a pointer to a
/// C++ primitive type.
_sus_pure constexpr _primitive* as_mut_ptr() & noexcept {
  return &primitive_value;
}

/// Returns a const pointer to the underlying C++ primitive value type.
///
/// This allows Subspace numerics be used with APIs that expect a pointer to a
/// C++ primitive type.
_sus_pure constexpr const _primitive* as_ptr() const& noexcept {
  return &primitive_value;
}
_sus_pure constexpr const _primitive* as_ptr() && noexcept = delete;

/// Returns true if the current value is positive and false if the number is
/// zero or negative.
_sus_pure constexpr bool is_negative() const& noexcept {
  return primitive_value < 0;
}
/// Returns true if the current value is negative and false if the number is
/// zero or positive.
_sus_pure constexpr bool is_positive() const& noexcept {
  return primitive_value > 0;
}

/// Returns a number representing sign of the current value.
///
/// - 0 if the number is zero
/// - 1 if the number is positive
/// - -1 if the number is negative
_sus_pure constexpr _self signum() const& noexcept {
  if (primitive_value < 0)
    return _primitive{-1};
  else
    return _primitive{primitive_value != 0};
}

/// Returns whether `l` and `r` are equal for signed integers.
///
/// Satisfies the [`Eq`]($sus::cmp::Eq) concept.
///
/// #[doc.overloads=int.eq]
[[nodiscard]] __sus_pure_const friend constexpr inline bool operator==(
    _self l, _self r) noexcept = default;
/// #[doc.overloads=int.eq]
[[nodiscard]] __sus_pure_const friend constexpr inline bool operator==(
    _self l, Signed auto r) noexcept {
  return l.primitive_value == r.primitive_value;
}
/// #[doc.overloads=int.eq]
[[nodiscard]] __sus_pure_const friend constexpr inline bool operator==(
    _self l, SignedPrimitiveInteger auto r) noexcept {
  return l.primitive_value == r;
}
/// Returns the ordering of `l` and `r` for signed integers.
///
/// Equivalent values also compare as equal and are replaceable so this
/// satisfies the [`StrongOrd`]($sus::cmp::StrongOrd) concept.
///
/// #[doc.overloads=int.strongord]
[[nodiscard]] __sus_pure_const friend constexpr inline std::strong_ordering
operator<=>(_self l, _self r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}
/// #[doc.overloads=int.strongord]
[[nodiscard]] __sus_pure_const friend constexpr inline std::strong_ordering
operator<=>(_self l, Signed auto r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}
/// #[doc.overloads=int.strongord]
[[nodiscard]] __sus_pure_const friend constexpr inline std::strong_ordering
operator<=>(_self l, SignedPrimitiveInteger auto r) noexcept {
  return l.primitive_value <=> r;
}

/// Computes the negation of the current value.
///
/// Satisfies the [`Neg<@doc.self>`]($sus::num::Neg) concept.
///
/// # Panics
/// This operation will panic on overflow (`self == @doc.self::MIN`) if overflow
/// checks are enabled (they are by default) and will wrap if overflow checks
/// are disabled (not the default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
_sus_pure constexpr inline _self operator-() const& noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    sus_check_with_message(primitive_value != MIN_PRIMITIVE,
                           "attempt to negate with overflow");
    return __private::unchecked_neg(primitive_value);
  } else {
    return wrapping_neg();
  }
}
/// Computes the bitwise complement (bitwise inverse) of the current value.
///
/// Satisfies the [`BitNot<@doc.self>`]($sus::num::BitNot) concept.
_sus_pure constexpr inline _self operator~() const& noexcept {
  return __private::into_signed(
      __private::unchecked_not(__private::into_unsigned(primitive_value)));
}

/// Computes `l` summed with `r`.
///
/// Satisfies the [`Add`]($sus::num::Add) concept for signed integers.
///
/// This operation supports shifting with primitive signed or unsigned integers
/// that convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// # Panics
/// This operation will panic on overflow if overflow checks are enabled (they
/// are by default) and will wrap if overflow checks are disabled (not the
/// default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
///
/// #[doc.overloads=signedint.+]
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator+(
    _self l, _self r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::add_with_overflow(l.primitive_value, r.primitive_value);
    sus_check_with_message(!out.overflow, "attempt to add with overflow");
    return out.value;
  } else {
    return l.wrapping_add(r);
  }
}
/// #[doc.overloads=signedint.+]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator+(
    _self l, U r) noexcept {
  return l + _self(r);
}
/// #[doc.overloads=signedint.+]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator+(
    U l, _self r) noexcept {
  return _self(l) + r;
}
/// #[doc.overloads=signedint.+]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator+(_self l, U r) noexcept = delete;
/// #[doc.overloads=signedint.+]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator+(U l, _self r) noexcept = delete;
/// Computes `l` subtracted by `r`.
///
/// Satisfies the [`Sub`]($sus::num::Sub) concept for signed integers.
///
/// This operation supports shifting with primitive signed or unsigned integers
/// that convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// # Panics
/// This operation will panic on overflow if overflow checks are enabled (they
/// are by default) and will wrap if overflow checks are disabled (not the
/// default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
///
/// #[doc.overloads=signedint.-]
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator-(
    _self l, _self r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::sub_with_overflow(l.primitive_value, r.primitive_value);
    sus_check_with_message(!out.overflow, "attempt to subtract with overflow");
    return out.value;
  } else {
    return l.wrapping_sub(r);
  }
}
/// #[doc.overloads=signedint.-]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator-(
    _self l, U r) noexcept {
  return l - _self(r);
}
/// #[doc.overloads=signedint.-]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator-(
    U l, _self r) noexcept {
  return _self(l) - r;
}
/// #[doc.overloads=signedint.-]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator-(_self l, U r) noexcept = delete;
/// #[doc.overloads=signedint.-]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator-(U l, _self r) noexcept = delete;
/// Computes `l` multiplied by `r`.
///
/// Satisfies the [`Mul`]($sus::num::Mul) concept for signed integers.
///
/// This operation supports shifting with primitive signed or unsigned integers
/// that convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// # Panics
/// This operation will panic on overflow if overflow checks are enabled (they
/// are by default) and will wrap if overflow checks are disabled (not the
/// default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
///
/// #[doc.overloads=signedint.*]
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator*(
    _self l, _self r) noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::mul_with_overflow(l.primitive_value, r.primitive_value);
    sus_check_with_message(!out.overflow, "attempt to multiply with overflow");
    return out.value;
  } else {
    return l.wrapping_mul(r);
  }
}
/// #[doc.overloads=signedint.*]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator*(
    _self l, U r) noexcept {
  return l * _self(r);
}
/// #[doc.overloads=signedint.*]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator*(
    U l, _self r) noexcept {
  return _self(l) * r;
}
/// #[doc.overloads=signedint.*]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator*(_self l, U r) noexcept = delete;
/// #[doc.overloads=signedint.*]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator*(U l, _self r) noexcept = delete;
/// Computes `l` divided by `r`.
///
/// Satisfies the [`Div`]($sus::num::Div) concept for signed integers.
///
/// This operation supports shifting with primitive signed or unsigned integers
/// that convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// # Panics
/// This operator will panic when dividing by zero or when division will
/// overflow.
///
/// #[doc.overloads=signedint./]
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator/(
    _self l, _self r) noexcept {
  sus_check_with_message(r.primitive_value != _primitive{0},
                         "attempt to divide by zero");
  sus_check_with_message(
      !__private::div_overflows_nonzero(l.primitive_value, r.primitive_value),
      "attempt to divide with overflow");
  return static_cast<_primitive>(l.primitive_value / r.primitive_value);
}
/// #[doc.overloads=signedint./]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator/(
    _self l, U r) noexcept {
  return l / _self(r);
}
/// #[doc.overloads=signedint./]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator/(
    U l, _self r) noexcept {
  return _self(l) / r;
}
/// #[doc.overloads=signedint./]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator/(_self l, U r) noexcept = delete;
/// #[doc.overloads=signedint./]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator/(U l, _self r) noexcept = delete;
/// Computes the remainder of `l` divided by `r`, also known as the modulo
/// operation.
///
/// Satisfies the [`Rem`]($sus::num::Rem) concept for signed integers.
///
/// This operation supports shifting with primitive signed or unsigned integers
/// that convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// # Panics
/// This operator will panic when dividing by zero or when division will
/// overflow.
///
/// #[doc.overloads=signedint.%]
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator%(
    _self l, _self r) noexcept {
  sus_check_with_message(
      r.primitive_value != _primitive{0},
      "attempt to calculate the remainder with a divisor of zero");
  sus_check_with_message(
      !__private::div_overflows_nonzero(l.primitive_value, r.primitive_value),
      "attempt to calculate the remainder with overflow");
  return static_cast<_primitive>(l.primitive_value % r.primitive_value);
}
/// #[doc.overloads=signedint.%]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator%(
    _self l, U r) noexcept {
  return l % _self(r);
}
/// #[doc.overloads=signedint.%]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator%(
    U l, _self r) noexcept {
  return _self(l) % r;
}
/// #[doc.overloads=signedint.%]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator%(_self l, U r) noexcept = delete;
/// #[doc.overloads=signedint.%]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator%(U l, _self r) noexcept = delete;

/// Performs the bitwise operation `l` AND `r`.
///
/// Satisfies the [`BitAnd`]($sus::num::BitAnd) concept for signed integers.
///
/// This operation supports shifting with primitive signed or unsigned integers
/// that convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=signedint.&]
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator&(
    _self l, _self r) noexcept {
  return static_cast<_primitive>(l.primitive_value & r.primitive_value);
}
/// #[doc.overloads=signedint.&]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator&(
    _self l, U r) noexcept {
  return l & _self(r);
}
/// #[doc.overloads=signedint.&]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator&(
    U l, _self r) noexcept {
  return _self(l) & r;
}
/// #[doc.overloads=signedint.&]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator&(_self l, U r) noexcept = delete;
/// #[doc.overloads=signedint.&]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator&(U l, _self r) noexcept = delete;
/// Performs the bitwise operation `l` OR `r`.
///
/// Satisfies the [`BitOr`]($sus::num::BitOr) concept for signed integers.
///
/// This operation supports shifting with primitive signed or unsigned integers
/// that convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=signedint.|]
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator|(
    _self l, _self r) noexcept {
  return static_cast<_primitive>(l.primitive_value | r.primitive_value);
}
/// #[doc.overloads=signedint.|]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator|(
    _self l, U r) noexcept {
  return l | _self(r);
}
/// #[doc.overloads=signedint.|]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator|(
    U l, _self r) noexcept {
  return _self(l) | r;
}
/// #[doc.overloads=signedint.|]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator|(_self l, U r) noexcept = delete;
/// #[doc.overloads=signedint.|]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator|(U l, _self r) noexcept = delete;
/// Performs the bitwise operation `l` XOR `r`.
///
/// Satisfies the [`BitXor`]($sus::num::BitXor) concept for signed integers.
///
/// This operation supports shifting with primitive signed or unsigned integers
/// that convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=signedint.^]
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator^(
    _self l, _self r) noexcept {
  return static_cast<_primitive>(l.primitive_value ^ r.primitive_value);
}
/// #[doc.overloads=signedint.^]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator^(
    _self l, U r) noexcept {
  return l ^ _self(r);
}
/// #[doc.overloads=signedint.^]
template <std::convertible_to<_self> U>
  requires(PrimitiveInteger<U> || PrimitiveEnum<U>)
[[nodiscard]] __sus_pure_const friend constexpr inline _self operator^(
    U l, _self r) noexcept {
  return _self(l) ^ r;
}
/// #[doc.overloads=signedint.^]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator^(_self l, U r) noexcept = delete;
/// #[doc.overloads=signedint.^]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !std::convertible_to<U, _self>)
friend constexpr inline _self operator^(U l, _self r) noexcept = delete;

/// Adds `r` to itself.
///
/// Satisfies the [`AddAssign<@doc.self>`]($sus::num::AddAssign) concept.
///
/// # Panics
/// This operation will panic on overflow if overflow checks are enabled (they
/// are by default) and will wrap if overflow checks are disabled (not the
/// default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
constexpr inline void operator+=(_self r) & noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::add_with_overflow(primitive_value, r.primitive_value);
    sus_check_with_message(!out.overflow, "attempt to add with overflow");
    primitive_value = out.value;
  } else {
    primitive_value = wrapping_add(r).primitive_value;
  }
}
/// Subtracts `r` from itself.
///
/// Satisfies the [`SubAssign<@doc.self>`]($sus::num::SubAssign) concept.
///
/// # Panics
/// This operation will panic on overflow if overflow checks are enabled (they
/// are by default) and will wrap if overflow checks are disabled (not the
/// default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
constexpr inline void operator-=(_self r) & noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::sub_with_overflow(primitive_value, r.primitive_value);
    sus_check_with_message(!out.overflow, "attempt to subtract with overflow");
    primitive_value = out.value;
  } else {
    primitive_value = wrapping_sub(r).primitive_value;
  }
}
/// Multiplies itself by `r`.
///
/// Satisfies the [`MulAssign<@doc.self>`]($sus::num::MulAssign) concept.
///
/// # Panics
/// This operation will panic on overflow if overflow checks are enabled (they
/// are by default) and will wrap if overflow checks are disabled (not the
/// default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
constexpr inline void operator*=(_self r) & noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::mul_with_overflow(primitive_value, r.primitive_value);
    sus_check_with_message(!out.overflow, "attempt to multiply with overflow");
    primitive_value = out.value;
  } else {
    primitive_value = wrapping_mul(r).primitive_value;
  }
}
/// Divides itself by `r`.
///
/// Satisfies the [`DivAssign<@doc.self>`]($sus::num::DivAssign) concept.
///
/// # Panics
/// This operator will panic when dividing by zero or when division will
/// overflow.
constexpr inline void operator/=(_self r) & noexcept {
  sus_check_with_message(r.primitive_value != _primitive{0},
                         "attempt to divide by zero");
  sus_check_with_message(
      !__private::div_overflows_nonzero(primitive_value, r.primitive_value),
      "attempt to divide with overflow");
  primitive_value /= r.primitive_value;
}
/// Assigns the remainder of dividing itself by `r`.
///
/// Satisfies the [`RemAssign<@doc.self>`]($sus::num::RemAssign) concept.
///
/// # Panics
/// This operator will panic when dividing by zero or when division will
/// overflow.
constexpr inline void operator%=(_self r) & noexcept {
  sus_check_with_message(
      r.primitive_value != _primitive{0},
      "attempt to calculate the remainder with a divisor of zero");
  sus_check_with_message(
      !__private::div_overflows_nonzero(primitive_value, r.primitive_value),
      "attempt to calculate the remainder with overflow");
  primitive_value %= r.primitive_value;
}

/// Assigns the bitwise AND of itself and `r`.
///
/// Satisfies the [`BitAndAssign<@doc.self>`]($sus::num::BitAndAssign)
/// concept.
///
/// # Panics
/// This operator will panic when dividing by zero or when division will
/// overflow.
constexpr inline void operator&=(_self r) & noexcept {
  primitive_value &= r.primitive_value;
}
/// Assigns the bitwise OR of itself and `r`.
///
/// Satisfies the [`BitOrAssign<@doc.self>`]($sus::num::BitOrAssign) concept.
constexpr inline void operator|=(_self r) & noexcept {
  primitive_value |= r.primitive_value;
}
/// Assigns the bitwise XOR of itself and `r`.
///
/// Satisfies the [`BitXorAssign<@doc.self>`]($sus::num::BitXorAssign)
/// concept.
constexpr inline void operator^=(_self r) & noexcept {
  primitive_value ^= r.primitive_value;
}
/// Shifts itself left by `r` bits.
///
/// Satisfies the [`ShlAssign<@doc.self>`]($sus::num::ShlAssign) concept.
///
/// # Panics
/// This function will panic when `r` is not less than the number of bits in
/// `@doc.self` if overflow checks are enabled (they are by default) and will
/// perform a wrapping shift if overflow checks are disabled (not the default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
constexpr inline void operator<<=(u64 r) & noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shl_with_overflow(primitive_value, r.primitive_value);
    sus_check_with_message(!out.overflow,
                           "attempt to shift left with overflow");
    primitive_value = out.value;
  } else {
    primitive_value = wrapping_shl(r).primitive_value;
  }
}
/// Shifts itself right by `r` bits.
///
/// Performs sign extension, copying the sign bit to the right if its set.
///
/// Satisfies the [`ShrAssign<@doc.self>`]($sus::num::ShrAssign) concept.
///
/// # Panics
/// This function will panic when `r` is not less than the number of bits in
/// `@doc.self` if overflow checks are enabled (they are by default) and will
/// perform a wrapping shift if overflow checks are disabled (not the default).
constexpr inline void operator>>=(u64 r) & noexcept {
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    const auto out =
        __private::shr_with_overflow(primitive_value, r.primitive_value);
    sus_check_with_message(!out.overflow,
                           "attempt to shift right with overflow");
    primitive_value = out.value;
  } else {
    primitive_value = wrapping_shr(r).primitive_value;
  }
}

/// Computes the absolute value of itself.
///
/// # Panics
/// The absolute value of `@doc.self::MIN` cannot be represented as an
/// `@doc.self`, and attempting to calculate it will panic when overflow checks
/// are enabled. When disabled (not the default), wrapping abs is performed.
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
_sus_pure constexpr inline _self abs() const& noexcept {
  if (primitive_value >= 0)
    return *this;
  else
    return -*this;
}

/// Checked absolute value. Computes [`abs`](
/// $sus::num::@doc.self::abs), returning `None` if the current
/// value is [`MIN`]($sus::num::@doc.self::MIN).
_sus_pure constexpr Option<_self> checked_abs() const& noexcept {
  if (primitive_value >= 0)
    return Option<_self>(*this);
  else if (primitive_value != MIN_PRIMITIVE)
    return Option<_self>(-*this);
  else [[unlikely]]
    return Option<_self>();
}

/// Computes the absolute value of `self`.
///
/// Returns a tuple of the absolute version of `self` along with a boolean
/// indicating whether an overflow happened. If `self` is the minimum value
/// (i.e. [`@doc.self::MIN`]($sus::num::@doc.self::MIN), then the minimum value
/// will be returned again and true will be returned for an overflow happening.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_abs()
    const& noexcept;

/// Saturating absolute value. Computes [`abs`](
/// $sus::num::@doc.self::abs), returning [`MAX`](
/// $sus::num::@doc.self::MAX) if the current value is [`MIN`](
/// $sus::num::@doc.self::MIN) instead of overflowing.
_sus_pure constexpr _self saturating_abs() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return abs();
  else
    return MAX;
}

/// Computes the absolute value of `self` without any wrapping or panicking.
_sus_pure constexpr _unsigned unsigned_abs() const& noexcept {
  if (primitive_value >= 0) {
    return __private::into_unsigned(primitive_value);
  } else {
    const auto neg_plus_one =
        __private::unchecked_add(primitive_value, _primitive{1});
    const auto pos_minus_one =
        __private::into_unsigned(__private::unchecked_neg(neg_plus_one));
    return __private::unchecked_add(pos_minus_one, decltype(pos_minus_one){1});
  }
}

/// Wrapping (modular) absolute value. Computes [`abs`](
/// $sus::num::@doc.self::abs), wrapping around at the boundary of the type.
///
/// The only case where such wrapping can occur is when one takes the
/// absolute value of the negative minimal value for the type; this is a
/// positive value that is too large to represent in the type. In such a
/// case, this function returns MIN itself.
_sus_pure constexpr _self wrapping_abs() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return abs();
  else
    return MIN;
}

/// Computes the absolute difference between `self` and `other`.
///
/// This function always returns the correct answer without overflow or
/// panics by returning an unsigned integer.
_sus_pure constexpr _unsigned abs_diff(_self other) const& noexcept {
  if (primitive_value >= other.primitive_value) {
    return __private::sub_with_unsigned_positive_result(primitive_value,
                                                        other.primitive_value);
  } else {
    return __private::sub_with_unsigned_positive_result(other.primitive_value,
                                                        primitive_value);
  }
}

/// Checked integer addition. Computes `self + rhs`, returning `None` if
/// overflow occurred.
_sus_pure constexpr Option<_self> checked_add(_self rhs) const& noexcept {
  const auto out =
      __private::add_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>(out.value);
  else
    return Option<_self>();
}

/// Checked integer addition with an unsigned `rhs`. Computes `self + rhs`,
/// returning `None` if overflow occurred.
_sus_pure constexpr Option<_self> checked_add_unsigned(
    _unsigned rhs) const& noexcept {
  const auto out = __private::add_with_overflow_unsigned(primitive_value,
                                                         rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>(out.value);
  else
    return Option<_self>();
}

/// Calculates `self + rhs`.
///
/// Returns a tuple of the addition along with a boolean indicating whether
/// an arithmetic overflow would occur. If an overflow would have occurred
/// then the wrapped value is returned.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_add(
    _self rhs) const& noexcept;

/// Calculates `self + rhs` with an unsigned `rhs`.
///
/// Returns a tuple of the addition along with a boolean indicating whether
/// an arithmetic overflow would occur. If an overflow would have occurred
/// then the wrapped value is returned.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
overflowing_add_unsigned(_unsigned rhs) const& noexcept;

/// Saturating integer addition. Computes `self + rhs`, saturating at the
/// numeric bounds instead of overflowing.
_sus_pure constexpr _self saturating_add(_self rhs) const& noexcept {
  return __private::saturating_add(primitive_value, rhs.primitive_value);
}

/// Saturating integer addition with an unsigned `rhs`. Computes `self + rhs`,
/// saturating at the numeric bounds instead of overflowing.
_sus_pure constexpr _self saturating_add_unsigned(
    _unsigned rhs) const& noexcept {
  const auto r = __private::add_with_overflow_unsigned(primitive_value,
                                                       rhs.primitive_value);
  if (!r.overflow) [[likely]]
    return r.value;
  else
    return MAX;
}

/// Unchecked integer addition. Computes `self + rhs`, assuming overflow
/// cannot occur.
///
/// # Safety
/// This results in undefined behavior when `self + rhs > `[`@doc.self::MAX`](
/// $sus::num::@doc.self::MAX) or
/// `self + rhs < `[`@doc.self::MIN`](
/// $sus::num::@doc.self::MIN), i.e. when [`checked_add`](
/// $sus::num::@doc.self::checked_add) would return `None`.
_sus_pure inline constexpr _self unchecked_add(::sus::marker::UnsafeFnMarker,
                                               _self rhs) const& noexcept {
  return __private::unchecked_add(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) addition. Computes `self + rhs`, wrapping around at the
/// boundary of the type.
_sus_pure constexpr _self wrapping_add(_self rhs) const& noexcept {
  return __private::wrapping_add(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) addition with an unsigned `rhs`. Computes `self + rhs`,
/// wrapping around at the boundary of the type.
_sus_pure constexpr _self wrapping_add_unsigned(_unsigned rhs) const& noexcept {
  return __private::add_with_overflow_unsigned(primitive_value,
                                               rhs.primitive_value)
      .value;
}

/// Checked integer division. Computes `self / rhs`, returning `None` if
/// `rhs == 0` or the division results in overflow.
_sus_pure constexpr Option<_self> checked_div(_self rhs) const& noexcept {
  if (__private::div_overflows(primitive_value, rhs.primitive_value))
      [[unlikely]]
    return Option<_self>();
  else
    return Option<_self>(
        __private::unchecked_div(primitive_value, rhs.primitive_value));
}

/// Calculates the divisor when `self` is divided by `rhs`.
///
/// Returns a tuple of the divisor along with a boolean indicating whether an
/// arithmetic overflow would occur. If an overflow would occur then `self` is
/// returned.
///
/// # Panics
/// This function will panic if `rhs` is 0.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_div(
    _self rhs) const& noexcept;

/// Saturating integer division. Computes `self / rhs`, saturating at the
/// numeric bounds instead of overflowing.
///
/// # Panics
/// This function will panic if `rhs` is 0.
_sus_pure constexpr _self saturating_div(_self rhs) const& noexcept {
  sus_check_with_message(rhs.primitive_value != _primitive{0},
                         "attempt to divide by zero");
  if (__private::div_overflows_nonzero(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    // Only overflows in the case of -MIN / -1, which gives MAX + 1,
    // saturated to MAX.
    return MAX;
  } else {
    return __private::unchecked_div(primitive_value, rhs.primitive_value);
  }
}

/// Wrapping (modular) division. Computes `self / rhs`, wrapping around at the
/// boundary of the type.
///
/// The only case where such wrapping can occur is when one divides `MIN / -1`
/// on a signed type (where `MIN` is the negative minimal value for the type);
/// this is equivalent to `-MIN`, a positive value that is too large to
/// represent in the type. In such a case, this function returns [`MIN`](
/// $sus::num::@doc.self::MIN) itself.
///
/// # Panics
/// This function will panic if `rhs` is 0.
_sus_pure constexpr _self wrapping_div(_self rhs) const& noexcept {
  sus_check_with_message(rhs.primitive_value != _primitive{0},
                         "attempt to divide by zero");
  if (__private::div_overflows_nonzero(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    // Only overflows in the case of -MIN / -1, which gives MAX + 1,
    // that wraps around to MIN.
    return MIN;
  } else {
    return __private::unchecked_div(primitive_value, rhs.primitive_value);
  }
}

/// Checked integer multiplication. Computes `self * rhs`, returning None if
/// overflow occurred.
_sus_pure constexpr Option<_self> checked_mul(_self rhs) const& noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>(out.value);
  else
    return Option<_self>();
}

/// Calculates the multiplication of `self` and `rhs`.
///
/// Returns a tuple of the multiplication along with a boolean indicating
/// whether an arithmetic overflow would occur. If an overflow would have
/// occurred then the wrapped value is returned.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_mul(
    _self rhs) const& noexcept;

/// Saturating integer multiplication. Computes `self * rhs`, saturating at
/// the numeric bounds instead of overflowing.
_sus_pure constexpr _self saturating_mul(_self rhs) const& noexcept {
  return __private::saturating_mul(primitive_value, rhs.primitive_value);
}

/// Unchecked integer multiplication. Computes `self * rhs`, assuming overflow
/// cannot occur.
///
/// # Safety
/// This results in undefined behavior when `self * rhs > `[`@doc.self::MAX`](
/// $sus::num::@doc.self::MAX) or `self * rhs < `[`@doc.self::MIN`](
/// $sus::num::@doc.self::MIN), i.e. when [`checked_mul`](
/// $sus::num::@doc.self::checked_mul) would return `None`.
_sus_pure constexpr inline _self unchecked_mul(::sus::marker::UnsafeFnMarker,
                                               _self rhs) const& noexcept {
  return __private::unchecked_mul(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) multiplication. Computes `self * rhs`, wrapping around
/// at the boundary of the type.
_sus_pure constexpr _self wrapping_mul(_self rhs) const& noexcept {
  return __private::wrapping_mul(primitive_value, rhs.primitive_value);
}

/// Checked negation. Computes `-self`, returning `None` if `self == `[`MIN`](
/// $sus::num::@doc.self::MIN).
_sus_pure constexpr Option<_self> checked_neg() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return Option<_self>(__private::unchecked_neg(primitive_value));
  else
    return Option<_self>();
}

/// Negates `self`, overflowing if this is equal to the minimum value.
///
/// Returns a tuple of the negated version of self along with a boolean
/// indicating whether an overflow happened. If `self` is the minimum value
/// (i.e. [`@doc.self::MIN`]($sus::num::@doc.self::MIN), then the minimum value
/// will be returned again and true will be returned for an overflow happening.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_neg()
    const& noexcept;

/// Saturating integer negation. Computes `-self`, returning [`MAX`](
/// $sus::num::@doc.self::MAX) if `self == `[`MIN`]($sus::num::@doc.self::MIN)
/// instead of overflowing.
_sus_pure constexpr _self saturating_neg() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return __private::unchecked_neg(primitive_value);
  else
    return MAX;
}

/// Wrapping (modular) negation. Computes `-self`, wrapping around at the
/// boundary of the type.
///
/// The only case where such wrapping can occur is when one negates `MIN` on
/// a signed type (where `MIN` is the negative minimal value for the type);
/// this is a positive value that is too large to represent in the type. In
/// such a case, this function returns [`MIN`](
/// $sus::num::@doc.self::MIN) itself.
_sus_pure constexpr _self wrapping_neg() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return __private::unchecked_neg(primitive_value);
  else
    return MIN;
}

/// Checked integer remainder. Computes `self % rhs`, returning `None` if
/// `rhs == 0` or the division results in overflow.
_sus_pure constexpr Option<_self> checked_rem(_self rhs) const& noexcept {
  if (__private::div_overflows(primitive_value, rhs.primitive_value))
      [[unlikely]]
    return Option<_self>();
  else
    return Option<_self>(
        static_cast<_primitive>(primitive_value % rhs.primitive_value));
}

/// Calculates the remainder when `self` is divided by `rhs`.
///
/// Returns a tuple of the remainder after dividing along with a boolean
/// indicating whether an arithmetic overflow would occur. If an overflow
/// would occur then 0 is returned.
///
/// # Panics
/// This function will panic if `rhs` is 0.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_rem(
    _self rhs) const& noexcept;

/// Wrapping (modular) remainder. Computes `self % rhs`, wrapping around at
/// the boundary of the type.
///
/// Such wrap-around never actually occurs mathematically; implementation
/// artifacts make `x % y` invalid for `MIN / -1` on a signed type (where `MIN`
/// is the negative minimal value). In such a case, this function returns 0.
_sus_pure constexpr _self wrapping_rem(_self rhs) const& noexcept {
  sus_check_with_message(
      rhs.primitive_value != _primitive{0},
      "attempt to calculate the remainder with a divisor of zero");
  if (__private::div_overflows_nonzero(primitive_value, rhs.primitive_value))
      [[likely]] {
    return _primitive{0};
  } else {
    return static_cast<_primitive>(primitive_value % rhs.primitive_value);
  }
}

/// Calculates the quotient of Euclidean division of `self` by `rhs`.
///
/// This computes the integer `q` such that `self = q * rhs + r`, with
/// `r = self.rem_euclid(rhs)` and `0 <= r < abs(rhs)`.
///
/// In other words, the result is `self / rhs` rounded to the integer `q` such
/// that `self >= q * rhs`. If `self > 0`, this is equal to round towards zero;
/// if `self < 0`, this is equal to round towards +/- infinity.
///
/// # Panics
/// This function will panic if `rhs` is 0 or the division results in overflow.
_sus_pure constexpr _self div_euclid(_self rhs) const& noexcept {
  sus_check_with_message(rhs.primitive_value != _primitive{0},
                         "attempt to divide by zero");
  sus_check_with_message(
      !__private::div_overflows_nonzero(primitive_value, rhs.primitive_value),
      "attempt to divide with overflow");
  return __private::div_euclid(::sus::marker::unsafe_fn, primitive_value,
                               rhs.primitive_value);
}

/// Checked Euclidean division. Computes `self.`[`div_euclid`](
/// $sus::num::@doc.self::div_euclid)`(rhs)`, returning
/// `None` if `rhs == 0` or the division results in overflow.
_sus_pure constexpr Option<_self> checked_div_euclid(
    _self rhs) const& noexcept {
  if (__private::div_overflows(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Option<_self>();
  } else {
    return Option<_self>(__private::div_euclid(
        ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value));
  }
}

/// Calculates the quotient of Euclidean division `self.`[`div_euclid`](
/// $sus::num::@doc.self::div_euclid)`(rhs)`.
///
/// Returns a tuple of the divisor along with a boolean indicating whether an
/// arithmetic overflow would occur. If an overflow would occur then self is
/// returned.
///
/// # Panics
/// This function will panic if `rhs` is 0.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
overflowing_div_euclid(_self rhs) const& noexcept;

/// Wrapping Euclidean division. Computes `self.`[`div_euclid`](
/// $sus::num::@doc.self::div_euclid)`(rhs)`, wrapping around at the boundary of the
/// type.
///
/// Wrapping will only occur in `MIN / -1` on a signed type (where `MIN` is the
/// negative minimal value for the type). This is equivalent to `-MIN`, a
/// positive value that is too large to represent in the type. In this case,
/// this method returns [`MIN`]($sus::num::@doc.self::MIN) itself.
///
/// # Panics
/// This function will panic if `rhs` is 0.
_sus_pure constexpr _self wrapping_div_euclid(_self rhs) const& noexcept {
  sus_check_with_message(rhs.primitive_value != _primitive{0},
                         "attempt to divide by zero");
  if (__private::div_overflows_nonzero(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return MIN;
  } else {
    return __private::div_euclid(::sus::marker::unsafe_fn, primitive_value,
                                 rhs.primitive_value);
  }
}

/// Calculates the least nonnegative remainder of `self` (mod `rhs`).
///
/// This is done as if by the Euclidean division algorithm – given
/// `r = self.rem_euclid(rhs)`, `self = rhs * self.div_euclid(rhs) + r`, and
/// `0 <= r < abs(rhs)`.
///
/// # Panics
/// This function will panic if `rhs` is 0 or the division results in overflow.
_sus_pure constexpr _self rem_euclid(_self rhs) const& noexcept {
  sus_check_with_message(
      rhs.primitive_value != _primitive{0},
      "attempt to calculate the remainder with a divisor of zero");
  sus_check_with_message(
      !__private::div_overflows_nonzero(primitive_value, rhs.primitive_value),
      "attempt to calculate the remainder with overflow");
  return __private::rem_euclid(::sus::marker::unsafe_fn, primitive_value,
                               rhs.primitive_value);
}

/// Checked Euclidean remainder. Computes `self.`[`rem_euclid`](
/// $sus::num::@doc.self::rem_euclid)`(rhs)`, returning
/// `None` if `rhs == 0` or the division results in overflow.
_sus_pure constexpr Option<_self> checked_rem_euclid(
    _self rhs) const& noexcept {
  if (__private::div_overflows(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Option<_self>();
  } else {
    return Option<_self>(__private::rem_euclid(
        ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value));
  }
}

/// Overflowing Euclidean remainder. Calculates `self.`[`rem_euclid`](
/// $sus::num::@doc.self::rem_euclid)`(rhs)`.
///
/// Returns a tuple of the remainder after dividing along with a boolean
/// indicating whether an arithmetic overflow would occur. If an overflow
/// would occur then 0 is returned.
///
/// # Panics
/// This function will panic if `rhs` is 0.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
overflowing_rem_euclid(_self rhs) const& noexcept;

/// Wrapping Euclidean remainder. Computes `self.`[`rem_euclid`](
/// $sus::num::@doc.self::rem_euclid)`(rhs)`, wrapping
/// around at the boundary of the type.
///
/// Wrapping will only occur in `MIN % -1` on a signed type (where `MIN` is the
/// negative minimal value for the type). In this case, this method returns
/// 0.
///
/// # Panics
/// This function will panic if `rhs` is 0.
_sus_pure constexpr _self wrapping_rem_euclid(_self rhs) const& noexcept {
  sus_check_with_message(
      rhs.primitive_value != _primitive{0},
      "attempt to calculate the remainder with a divisor of zero");
  if (__private::div_overflows_nonzero(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return _primitive{0};
  } else {
    return __private::rem_euclid(::sus::marker::unsafe_fn, primitive_value,
                                 rhs.primitive_value);
  }
}

/// Checked shift left. Computes `self << rhs`, returning `None` if `rhs` is
/// larger than or equal to the number of bits in `self`.
_sus_pure constexpr Option<_self> checked_shl(u64 rhs) const& noexcept {
  const auto out =
      __private::shl_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>(out.value);
  else
    return Option<_self>();
}

/// Shifts `self` left by `rhs` bits.
///
/// Returns a tuple of the shifted version of `self` along with a boolean
/// indicating whether the shift value was larger than or equal to the number
/// of bits. If the shift value is too large, then `value` is masked by `(N-1)`
/// where `N` is the number of bits, and this value is then used to perform the
/// shift.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_shl(
    u64 rhs) const& noexcept;

/// Panic-free bitwise shift-left; yields `self << mask(rhs)`, where mask
/// removes any high-order bits of `rhs` that would cause the shift to exceed
/// the bitwidth of the type.
///
/// Note that this is not the same as a rotate-left; the RHS of a wrapping
/// shift-left is restricted to the range of the type, rather than the bits
/// shifted out of the LHS being returned to the other end. The Subspace
/// integer types all implement a
// [`rotate_left`]($sus::num::@doc.self::rotate_left) function, which may be
/// what you want instead.
_sus_pure constexpr _self wrapping_shl(u64 rhs) const& noexcept {
  return __private::shl_with_overflow(primitive_value, rhs.primitive_value)
      .value;
}

/// Checked shift right. Computes `self >> rhs`, returning `None` if `rhs` is
/// larger than or equal to the number of bits in `self`.
_sus_pure constexpr Option<_self> checked_shr(u64 rhs) const& noexcept {
  const auto out =
      __private::shr_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>(out.value);
  else
    return Option<_self>();
}

/// Shifts `self` right by `rhs` bits.
///
/// Returns a tuple of the shifted version of self along with a boolean
/// indicating whether the shift value was larger than or equal to the number
/// of bits. If the shift value is too large, then value is masked by `(N-1)`
/// where `N` is the number of bits, and this value is then used to perform the
/// shift.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_shr(
    u64 rhs) const& noexcept;

/// Panic-free bitwise shift-right; yields `self >> mask(rhs)`, where mask
/// removes any high-order bits of `rhs` that would cause the shift to exceed
/// the bitwidth of the type.
///
/// Note that this is not the same as a rotate-right; the RHS of a wrapping
/// shift-right is restricted to the range of the type, rather than the bits
/// shifted out of the LHS being returned to the other end. The Subspace
/// integer types all implement a
/// [`rotate_right`]($sus::num::@doc.self::rotate_right) function, which may be
/// what you want instead.
_sus_pure constexpr _self wrapping_shr(u64 rhs) const& noexcept {
  return __private::shr_with_overflow(primitive_value, rhs.primitive_value)
      .value;
}

/// Checked integer subtraction. Computes `self - rhs`, returning `None` if
/// overflow occurred.
_sus_pure constexpr Option<_self> checked_sub(_self rhs) const& {
  const auto out =
      __private::sub_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>(out.value);
  else
    return Option<_self>();
}

/// Checked integer subtraction with an unsigned `rhs`. Computes `self - rhs`,
/// returning `None` if overflow occurred.
_sus_pure constexpr Option<_self> checked_sub_unsigned(_unsigned rhs) const& {
  const auto out = __private::sub_with_overflow_unsigned(primitive_value,
                                                         rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>(out.value);
  else
    return Option<_self>();
}

/// Calculates `self - rhs`.
///
/// Returns a tuple of the subtraction along with a boolean indicating
/// whether an arithmetic overflow would occur. If an overflow would have
/// occurred then the wrapped value is returned.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_sub(
    _self rhs) const& noexcept;

/// Calculates `self - rhs` with an unsigned `rhs`.
///
/// Returns a tuple of the subtraction along with a boolean indicating
/// whether an arithmetic overflow would occur. If an overflow would have
/// occurred then the wrapped value is returned.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool>
overflowing_sub_unsigned(const _unsigned& rhs) const& noexcept;

/// Saturating integer subtraction. Computes `self - rhs`, saturating at the
/// numeric bounds instead of overflowing.
_sus_pure constexpr _self saturating_sub(_self rhs) const& {
  return __private::saturating_sub(primitive_value, rhs.primitive_value);
}

/// Saturating integer subtraction with an unsigned `rhs`. Computes
/// `self - rhs`, saturating at the numeric bounds instead of overflowing.
_sus_pure constexpr _self saturating_sub_unsigned(const _unsigned& rhs) const& {
  const auto out = __private::sub_with_overflow_unsigned(primitive_value,
                                                         rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return out.value;
  else
    return MIN;
}

/// Unchecked integer subtraction. Computes `self - rhs`, assuming overflow
/// cannot occur.
///
/// # Safety
/// This results in undefined behavior when `self - rhs > `[`@doc.self::MAX`](
/// $sus::num::@doc.self::MAX) or
/// `self - rhs < `[`@doc.self::MIN`](
/// $sus::num::@doc.self::MIN), i.e. when [`checked_sub`](
/// $sus::num::@doc.self::checked_sub) would return `None`.
_sus_pure inline constexpr _self unchecked_sub(::sus::marker::UnsafeFnMarker,
                                               _self rhs) const& {
  return static_cast<_primitive>(primitive_value - rhs.primitive_value);
}

/// Wrapping (modular) subtraction. Computes `self - rhs`, wrapping around at
/// the boundary of the type.
constexpr _self wrapping_sub(_self rhs) const& {
  return __private::wrapping_sub(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) subtraction with an unsigned `rhs`. Computes
/// `self - rhs`, wrapping around at the boundary of the type.
_sus_pure constexpr _self wrapping_sub_unsigned(const _unsigned& rhs) const& {
  return __private::sub_with_overflow_unsigned(primitive_value,
                                               rhs.primitive_value)
      .value;
}

/// Returns the number of ones in the binary representation of the current
/// value.
_sus_pure constexpr u32 count_ones() const& noexcept {
  return __private::count_ones(__private::into_unsigned(primitive_value));
}

/// Returns the number of zeros in the binary representation of the current
/// value.
_sus_pure constexpr u32 count_zeros() const& noexcept {
  return (~(*this)).count_ones();
}

/// Returns the number of leading ones in the binary representation of the
/// current value.
_sus_pure constexpr u32 leading_ones() const& noexcept {
  return (~(*this)).leading_zeros();
}

/// Returns the number of leading zeros in the binary representation of the
/// current value.
_sus_pure constexpr u32 leading_zeros() const& noexcept {
  return __private::leading_zeros(__private::into_unsigned(primitive_value));
}

/// Returns the number of trailing ones in the binary representation of the
/// current value.
_sus_pure constexpr u32 trailing_ones() const& noexcept {
  return (~(*this)).trailing_zeros();
}

/// Returns the number of trailing zeros in the binary representation of the
/// current value.
_sus_pure constexpr u32 trailing_zeros() const& noexcept {
  return __private::trailing_zeros(__private::into_unsigned(primitive_value));
}

/// Reverses the order of bits in the integer. The least significant bit
/// becomes the most significant bit, second least-significant bit becomes
/// second most-significant bit, etc.
_sus_pure constexpr _self reverse_bits() const& noexcept {
  return __private::into_signed(
      __private::reverse_bits(__private::into_unsigned(primitive_value)));
}

/// Shifts the bits to the left by a specified amount, `n`, wrapping the
/// truncated bits to the end of the resulting integer.
///
/// Please note this isn't the same operation as the `<<` shifting operator!
_sus_pure constexpr _self rotate_left(u64 n) const& noexcept {
  return __private::into_signed(__private::rotate_left(
      __private::into_unsigned(primitive_value), n.primitive_value));
}

/// Shifts the bits to the right by a specified amount, n, wrapping the
/// truncated bits to the beginning of the resulting integer.
///
/// Please note this isn't the same operation as the >> shifting operator!
_sus_pure constexpr _self rotate_right(u64 n) const& noexcept {
  return __private::into_signed(__private::rotate_right(
      __private::into_unsigned(primitive_value), n.primitive_value));
}

/// Reverses the byte order of the integer.
_sus_pure constexpr _self swap_bytes() const& noexcept {
  return __private::into_signed(
      __private::swap_bytes(__private::into_unsigned(primitive_value)));
}

/// Raises self to the power of `exp`, using exponentiation by squaring.
///
/// # Panics
/// This function will panic on overflow if overflow checks are enabled (they
/// are by default) and will wrap if overflow checks are disabled (not the
/// default).
///
/// See [overflow checks]($sus::num#overflow-behaviour) for controlling this
/// behaviour.
_sus_pure constexpr inline _self pow(u32 rhs) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, rhs.primitive_value);
  if constexpr (SUS_CHECK_INTEGER_OVERFLOW) {
    sus_check_with_message(!out.overflow, "attempt to multiply with overflow");
  }
  return out.value;
}

/// Checked exponentiation. Computes [`pow`]($sus::num::@doc.self::pow),
/// returning `None` if overflow occurred.
_sus_pure constexpr Option<_self> checked_pow(u32 rhs) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>(out.value);
  else
    return Option<_self>();
}

/// Raises self to the power of `exp`, using exponentiation by squaring.
///
/// Returns a tuple of the exponentiation along with a bool indicating
/// whether an overflow happened.
_sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_pow(
    u32 exp) const& noexcept;

/// Wrapping (modular) exponentiation. Computes
/// [`pow`]($sus::num::@doc.self::pow), wrapping around at the boundary of the
/// type.
_sus_pure constexpr _self wrapping_pow(u32 exp) const& noexcept {
  return __private::wrapping_pow(primitive_value, exp.primitive_value);
}

/// Returns the base 2 logarithm of the number, rounded down.
///
/// Returns `None` if the number is negative or zero.
_sus_pure constexpr Option<u32> checked_log2() const& noexcept {
  if (primitive_value <= 0) [[unlikely]] {
    return Option<u32>();
  } else {
    uint32_t zeros = __private::leading_zeros_nonzero(
        ::sus::marker::unsafe_fn, __private::into_unsigned(primitive_value));
    return Option<u32>(u32(__private::num_bits<_primitive>()) - 1_u32 -
                       u32(zeros));
  }
}

/// Returns the base 2 logarithm of the number, rounded down.
///
/// # Panics
/// When the number is zero or negative the function will panic.
_sus_pure constexpr u32 log2() const& noexcept {
  return checked_log2().expect(
      "argument of integer logarithm must be positive");
}

/// Returns the base 10 logarithm of the number, rounded down.
///
/// Returns `None` if the number is negative or zero.
_sus_pure constexpr Option<u32> checked_log10() const& noexcept {
  if (primitive_value <= 0) [[unlikely]] {
    return Option<u32>();
  } else {
    return Option<u32>(__private::int_log10::_self(primitive_value));
  }
}

/// Returns the base 10 logarithm of the number, rounded down.
///
/// # Panics
/// When the number is zero or negative the function will panic.
_sus_pure constexpr u32 log10() const& noexcept {
  return checked_log10().expect(
      "argument of integer logarithm must be positive");
}

/// Returns the logarithm of the number with respect to an arbitrary base,
/// rounded down.
///
/// Returns None if the number is negative or zero, or if the base is not at
/// least 2.
///
/// This method might not be optimized owing to implementation details;
/// [`checked_log2`]($sus::num::@doc.self::checked_log2) can produce results
/// more efficiently for base 2, and
/// [`checked_log10`]($sus::num::@doc.self::checked_log10) can produce results
/// more efficiently for base 10.
_sus_pure constexpr Option<u32> checked_log(_self base) const& noexcept {
  if (primitive_value <= 0 || base.primitive_value <= 1) [[unlikely]] {
    return Option<u32>();
  } else {
    auto n = uint32_t{0};
    auto r = primitive_value;
    const auto b = base.primitive_value;
    while (r >= b) {
      r /= b;
      n += 1u;
    }
    return Option<u32>(n);
  }
}

/// Returns the logarithm of the number with respect to an arbitrary `base`,
/// rounded down.
///
/// This method might not be optimized owing to implementation details;
/// [`log2`]($sus::num::@doc.self::log2)
/// can produce results more efficiently for base 2, and
/// [`log10`]($sus::num::@doc.self::log10) can produce
/// results more efficiently for base 10.
///
/// # Panics
/// When the number is negative, zero, or if the `base` is not at least 2.
_sus_pure constexpr u32 log(_self base) const& noexcept {
  return checked_log(base).expect(
      "argument of integer logarithm must be positive");
}

/// Converts an integer from big endian to the target's endianness.
///
/// On big endian this is a no-op. On little endian the bytes are swapped.
_sus_pure static constexpr _self from_be(_self x) noexcept {
  if constexpr (std::endian::native == std::endian::big)
    return x;
  else
    return x.swap_bytes();
}

/// Converts an integer from little endian to the target's endianness.
///
/// On little endian this is a no-op. On big endian the bytes are swapped.
_sus_pure static constexpr _self from_le(_self x) noexcept {
  if constexpr (std::endian::native == std::endian::little)
    return x;
  else
    return x.swap_bytes();
}

/// Converts self to big endian from the target's endianness.
///
/// On big endian this is a no-op. On little endian the bytes are swapped.
_sus_pure constexpr _self to_be() const& noexcept {
  if constexpr (std::endian::native == std::endian::big)
    return *this;
  else
    return swap_bytes();
}

/// Converts self to little endian from the target's endianness.
///
/// On little endian this is a no-op. On big endian the bytes are swapped.
_sus_pure constexpr _self to_le() const& noexcept {
  if constexpr (std::endian::native == std::endian::little)
    return *this;
  else
    return swap_bytes();
}

/// Return the memory representation of this integer as a byte array in
/// big-endian (network) byte order.
_sus_pure constexpr ::sus::collections::Array<u8,
                                              ::sus::mem::size_of<_primitive>()>
to_be_bytes() const& noexcept;

/// Return the memory representation of this integer as a byte array in
/// little-endian byte order.
_sus_pure constexpr ::sus::collections::Array<u8,
                                              ::sus::mem::size_of<_primitive>()>
to_le_bytes() const& noexcept;

/// Return the memory representation of this integer as a byte array in
/// native byte order.
///
/// As the target platform's native endianness is used, portable code should
/// use [`to_be_bytes`]($sus::num::@doc.self::to_be_bytes) or
/// [`to_le_bytes`]($sus::num::@doc.self::to_le_bytes), as appropriate, instead.
_sus_pure constexpr ::sus::collections::Array<u8,
                                              ::sus::mem::size_of<_primitive>()>
to_ne_bytes() const& noexcept;

/// Create an integer value from its representation as a byte array in big
/// endian.
_sus_pure static constexpr _self from_be_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

/// Create an integer value from its representation as a byte array in
/// little endian.
_sus_pure static constexpr _self from_le_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

/// Create an integer value from its memory representation as a byte array
/// in native endianness.
///
/// As the target platform's native endianness is used, portable code likely
/// wants to use [`from_be_bytes`]($sus::num::@doc.self::from_be_bytes) or
/// [`from_le_bytes`]($sus::num::@doc.self::from_le_bytes), as appropriate
/// instead.
_sus_pure static constexpr _self from_ne_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

// Stream support.
_sus_format_to_stream(_self)

#undef _self
#undef _primitive
#undef _unsigned
