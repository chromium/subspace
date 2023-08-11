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

/// The inner primitive value, in case it needs to be unwrapped from the
/// type. Avoid using this member except to convert when a consumer requires
/// it.
_primitive primitive_value{0};

/// The smallest value that can be represented by this integer type,
/// as a native C++ primitive.
static constexpr auto MIN_PRIMITIVE = __private::min_value<_primitive>();
/// The largest value that can be represented by this integer type,
/// as a native C++ primitive.
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
constexpr inline _self() noexcept = default;

/// Construction from signed types where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.ctor.signedint.typed]
template <Signed S>
  requires(::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(S v) noexcept : primitive_value(v.primitive_value) {}

/// Construction from signed primitive types where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.ctor.signedint.primitive]
template <SignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from signed enum types where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.ctor.signedenum]
template <SignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from signed enum class types where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.ctor.signedenumclass]
template <SignedPrimitiveEnumClass P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
explicit constexpr inline _self(P v) noexcept
    : primitive_value(static_cast<_primitive>(v)) {}

/// Construction from unsigned types where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.ctor.unsignedint.typed]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() < ::sus::mem::size_of<_primitive>())
constexpr inline _self(U v) noexcept : primitive_value(v.primitive_value) {}

/// Construction from unsigned primitive types where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.ctor.unsignedint.primitive]
template <UnsignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from unsigned enum types where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.ctor.unsignedenum]
template <UnsignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from unsigned enum class types where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.ctor.unsignedenumclass]
template <UnsignedPrimitiveEnumClass P>
  requires(::sus::mem::size_of<P>() < ::sus::mem::size_of<_primitive>())
explicit constexpr inline _self(P v) noexcept
    : primitive_value(static_cast<_primitive>(v)) {}

/// Assignment from signed types where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
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
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
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
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.assign.signedenum]
template <SignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(P v) noexcept {
  primitive_value = v;
  return *this;
}

/// Constructs a @doc.self from a signed integer type (i8, i16, i32, etc) where
/// no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=from.signed]
template <Signed S>
  requires(::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(S s) noexcept {
  return _self(s);
}

/// Try to construct a @doc.self from a signed integer type (i8, i16, i32, etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=0]
template <Signed S>
sus_pure static constexpr ::sus::result::Result<_self,
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
  return R::with(_self(static_cast<_primitive>(s.primitive_value)));
}

/// Try to construct a @doc.self from an unsigned integer type (u8, u16, u32,
/// etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=1]
template <Unsigned U>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);
  if constexpr (umax < U::MAX_PRIMITIVE) {
    if (u.primitive_value > umax) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R::with(_self(static_cast<_primitive>(u.primitive_value)));
}

/// Constructs a @doc.self from a signed primitive integer type (int, long, etc)
/// where no bits are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=from.signed.prim]
template <SignedPrimitiveInteger S>
  requires(::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(S s) noexcept {
  return _self(s);
}

/// Tries to construct a @doc.self from a signed primitive integer type (int,
/// long, etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signedint.tryfrom.signedint]
template <SignedPrimitiveInteger S>
sus_pure static constexpr ::sus::result::Result<_self,
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
  return R::with(_self(static_cast<_primitive>(s)));
}

/// Constructs a @doc.self from a signed enum type (or enum class) where no bits
/// are lost.
///
/// For conversions from types with a larger range use `try_from()`. For lossy
/// conversions use the `sus::construct::Transmogrify` concept with
/// `sus::mog<@doc.self>()`.
///
/// #[doc.overloads=signedint.from.signedenum]
template <class S>
  requires((SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>) &&
           ::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(S s) noexcept {
  using D = std::underlying_type_t<S>;
  return _self(static_cast<std::underlying_type_t<S>>(s));
}

/// Tries to construct a @doc.self from a signed enum type (or enum class).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signedint.tryfrom.signedenum]
template <class S>
  requires(SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>)
sus_pure static constexpr ::sus::result::Result<_self,
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
  return R::with(_self(static_cast<_primitive>(s)));
}

/// Constructs a @doc.self from an unsigned primitive integer type (unsigned
/// int, unsigned long, etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signedint.tryfrom.unsignedint]
template <UnsignedPrimitiveInteger U>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u) noexcept {
  using R = ::sus::result::Result<_self, ::sus::num::TryFromIntError>;
  constexpr auto umax = __private::into_unsigned(MAX_PRIMITIVE);
  if constexpr (umax < __private::max_value<U>()) {
    if (u > umax) {
      return R::with_err(::sus::num::TryFromIntError::with_out_of_bounds());
    }
  }
  return R::with(_self(static_cast<_primitive>(u)));
}

/// Constructs a @doc.self from an unsigned enum type (or enum class).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signedint.tryfrom.unsignedenum]
template <class U>
  requires(UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>)
sus_pure static constexpr ::sus::result::Result<_self,
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
  return R::with(_self(static_cast<_primitive>(u)));
}

/// Constructs a @doc.self from an `Iterator` by computing the product of all
/// elements in the itertor.
///
/// This method should rarely be called directly, as it is used to satisfy the
/// `sus::iter::Product` concept so that `Iterator::product()` can be called for
/// iterators over @doc.self.
///
/// To handle overflow without panicing, instead of `iter.product()`, use
/// `iter.product<OverflowInteger<@doc.self>>()`.
///
/// # Panics
/// This method will panic if the product of all values overflows.
static constexpr _self from_product(
    ::sus::iter::Iterator<_self> auto&& it) noexcept {
  auto p = _self(_primitive{1});
  for (_self i : ::sus::move(it)) p *= i;
  return p;
}

/// Constructs a @doc.self from an `Iterator` by computing the sum of all
/// elements in the itertor.
///
/// This method should rarely be called directly, as it is used to satisfy the
/// `sus::iter::Sum` concept so that `Iterator::sum()` can be called for
/// iterators over @doc.self.
///
/// To handle overflow without panicing, instead of `iter.sum()`, use
/// `iter.sum<OverflowInteger<@doc.self>>()`.
///
/// # Panics
/// This method will panic if the product of all values overflows.
static constexpr _self from_sum(
    ::sus::iter::Iterator<_self> auto&& it) noexcept {
  auto p = _self(_primitive{0});
  for (_self i : ::sus::move(it)) p += i;
  return p;
}

/// Explicit conversion from the numeric type to the inner primitive type.
///
/// Typically this should be used in a curly-brace conversion statement such as
/// `int32_t{3_i32}` which will ensure a compiler error if the conversion can
/// lose data. Lossy conversions can be done through the `sus::convert::Transmogrify`
/// concept, such as `sus::mog<int32_t>(3_i32)` or
/// `sus::mog<uint32_t>(3_i32)`.
template <SignedPrimitiveInteger U>
  requires(::sus::mem::size_of<U>() >= ::sus::mem::size_of<_primitive>())
sus_pure constexpr inline explicit operator U() const {
  return primitive_value;
}

/// Returns true if the current value is positive and false if the number is
/// zero or negative.
sus_pure constexpr bool is_negative() const& noexcept {
  return primitive_value < 0;
}
/// Returns true if the current value is negative and false if the number is
/// zero or positive.
sus_pure constexpr bool is_positive() const& noexcept {
  return primitive_value > 0;
}

/// Returns a number representing sign of the current value.
///
/// - 0 if the number is zero
/// - 1 if the number is positive
/// - -1 if the number is negative
sus_pure constexpr _self signum() const& noexcept {
  if (primitive_value < 0)
    return _primitive{-1};
  else
    return _primitive{primitive_value != 0};
}

/// sus::ops::Eq<@doc.self> trait.
/// #[doc.overloads=int.eq.self] */
[[nodiscard]] sus_pure friend constexpr inline bool operator==(
    const _self& l, const _self& r) noexcept = default;
/// sus::ops::Eq<@doc.self, SignedPrimitiveInteger> trait.
/// #[doc.overloads=int.eq.signedprimitive] */
template <SignedPrimitiveInteger P>
[[nodiscard]] sus_pure friend constexpr inline bool operator==(
    const _self& l, const P& r) noexcept {
  return l.primitive_value == r;
}
/// sus::ops::Eq<@doc.self, Signed> trait.
/// #[doc.overloads=int.eq.signed] */
template <Signed S>
[[nodiscard]] sus_pure friend constexpr inline bool operator==(
    const _self& l, const S& r) noexcept {
  return l.primitive_value == r.primitive_value;
}
/// sus::ops::StrongOrd<@doc.self> trait.
/// #[doc.overloads=int.ord.self] */
template <UnsignedPrimitiveInteger P>
[[nodiscard]] sus_pure friend constexpr inline std::strong_ordering operator<=>(
    const _self& l, const _self& r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}
/// sus::ops::StrongOrd<@doc.self, SignedPrimitiveInteger> trait.
/// #[doc.overloads=int.ord.signedprimitive] */
template <SignedPrimitiveInteger P>
[[nodiscard]] sus_pure friend constexpr inline std::strong_ordering operator<=>(
    const _self& l, const P& r) noexcept {
  return l.primitive_value <=> r;
}
/// sus::ops::StrongOrd<@doc.self, Signed> trait.
/// #[doc.overloads=int.ord.signed] */
template <Signed S>
[[nodiscard]] sus_pure friend constexpr inline std::strong_ordering operator<=>(
    const _self& l, const S& r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}

/// sus::num::Neg trait. */
sus_pure constexpr inline _self operator-() const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(primitive_value != MIN_PRIMITIVE);
  return __private::unchecked_neg(primitive_value);
}
/// sus::num::BitNot trait. */
sus_pure constexpr inline _self operator~() const& noexcept {
  return __private::into_signed(
      __private::unchecked_not(__private::into_unsigned(primitive_value)));
}

/// sus::num::Add<@doc.self> trait.
/// #[doc.overloads=self.+] */
[[nodiscard]] sus_pure friend constexpr inline _self operator+(
    const _self& l, const _self& r) noexcept {
  const auto out =
      __private::add_with_overflow(l.primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  return out.value;
}
/// sus::num::Sub<@doc.self> trait.
/// #[doc.overloads=self.-] */
[[nodiscard]] sus_pure friend constexpr inline _self operator-(
    const _self& l, const _self& r) noexcept {
  const auto out =
      __private::sub_with_overflow(l.primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  return out.value;
}
/// sus::num::Mul<@doc.self> trait.
/// #[doc.overloads=self.*] */
[[nodiscard]] sus_pure friend constexpr inline _self operator*(
    const _self& l, const _self& r) noexcept {
  const auto out =
      __private::mul_with_overflow(l.primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  return out.value;
}
/// sus::num::Div<@doc.self> trait.
/// #[doc.overloads=self./] */
[[nodiscard]] sus_pure friend constexpr inline _self operator/(
    const _self& l, const _self& r) noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(r.primitive_value != 0);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(l.primitive_value != MIN_PRIMITIVE || r.primitive_value != -1);
  return static_cast<_primitive>(l.primitive_value / r.primitive_value);
}
/// sus::num::Rem<@doc.self> trait.
/// #[doc.overloads=self.%] */
[[nodiscard]] sus_pure friend constexpr inline _self operator%(
    const _self& l, const _self& r) noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(r.primitive_value != 0);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(l.primitive_value != MIN_PRIMITIVE || r.primitive_value != -1);
  return static_cast<_primitive>(l.primitive_value % r.primitive_value);
}

/// sus::num::BitAnd<@doc.self> trait.
/// #[doc.overloads=self.&] */
[[nodiscard]] sus_pure friend constexpr inline _self operator&(
    const _self& l, const _self& r) noexcept {
  return static_cast<_primitive>(l.primitive_value & r.primitive_value);
}
/// sus::num::BitOr<@doc.self> trait.
/// #[doc.overloads=self.|] */
[[nodiscard]] sus_pure friend constexpr inline _self operator|(
    const _self& l, const _self& r) noexcept {
  return static_cast<_primitive>(l.primitive_value | r.primitive_value);
}
/// sus::num::BitXor<@doc.self> trait.
/// #[doc.overloads=self.^] */
[[nodiscard]] sus_pure friend constexpr inline _self operator^(
    const _self& l, const _self& r) noexcept {
  return static_cast<_primitive>(l.primitive_value ^ r.primitive_value);
}
/// sus::num::Shl trait.
/// #[doc.overloads=self.<<] */
[[nodiscard]] sus_pure friend constexpr inline _self operator<<(
    const _self& l, const u32& r) noexcept {
  const auto out =
      __private::shl_with_overflow(l.primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  return out.value;
}
/// sus::num::Shr trait.
/// #[doc.overloads=self.>>] */
[[nodiscard]] sus_pure friend constexpr inline _self operator>>(
    const _self& l, const u32& r) noexcept {
  const auto out =
      __private::shr_with_overflow(l.primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  return out.value;
}

/// sus::num::AddAssign<@doc.self> trait. */
constexpr inline void operator+=(_self r) & noexcept {
  const auto out =
      __private::add_with_overflow(primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
/// sus::num::SubAssign<@doc.self> trait. */
constexpr inline void operator-=(_self r) & noexcept {
  const auto out =
      __private::sub_with_overflow(primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
/// sus::num::MulAssign<@doc.self> trait. */
constexpr inline void operator*=(_self r) & noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
/// sus::num::DivAssign<@doc.self> trait. */
constexpr inline void operator/=(_self r) & noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(r.primitive_value != 0);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(primitive_value != MIN_PRIMITIVE || r.primitive_value != -1);
  primitive_value /= r.primitive_value;
}
/// sus::num::RemAssign<@doc.self> trait. */
constexpr inline void operator%=(_self r) & noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(r.primitive_value != 0);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(primitive_value != MIN_PRIMITIVE || r.primitive_value != -1);
  primitive_value %= r.primitive_value;
}

/// sus::num::BitAndAssign<@doc.self> trait. */
constexpr inline void operator&=(_self r) & noexcept {
  primitive_value &= r.primitive_value;
}
/// sus::num::BitOrAssign<@doc.self> trait. */
constexpr inline void operator|=(_self r) & noexcept {
  primitive_value |= r.primitive_value;
}
/// sus::num::BitXorAssign<@doc.self> trait. */
constexpr inline void operator^=(_self r) & noexcept {
  primitive_value ^= r.primitive_value;
}
/// sus::num::ShlAssign trait. */
constexpr inline void operator<<=(const u32& r) & noexcept {
  const auto out =
      __private::shl_with_overflow(primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
/// sus::num::ShrAssign trait. */
constexpr inline void operator>>=(const u32& r) & noexcept {
  const auto out =
      __private::shr_with_overflow(primitive_value, r.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}

/// Computes the absolute value of itself.
///
/// The absolute value of @doc.self::MIN cannot be represented as an @doc.self,
/// and attempting to calculate it will panic.
sus_pure constexpr inline _self abs() const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(primitive_value != MIN_PRIMITIVE);
  if (primitive_value >= 0)
    return primitive_value;
  else
    return __private::unchecked_neg(primitive_value);
}

/// Checked absolute value. Computes `abs()`, returning None if the current
/// value is MIN.
sus_pure constexpr Option<_self> checked_abs() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return Option<_self>::with(abs());
  else
    return Option<_self>();
}

/// Computes the absolute value of self.
///
/// Returns a tuple of the absolute version of self along with a boolean
/// indicating whether an overflow happened. If self is the minimum value
/// (e.g., @doc.self::MIN for values of type @doc.self), then the minimum value
/// will be returned again and true will be returned for an overflow happening.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_abs() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return Tuple::with(abs(), false);
  else
    return Tuple::with(MIN, true);
}

/// Saturating absolute value. Computes `abs()`, returning MAX if the
///  current value is MIN instead of overflowing.
sus_pure constexpr _self saturating_abs() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return abs();
  else
    return MAX;
}

/// Computes the absolute value of self without any wrapping or panicking.
sus_pure constexpr _unsigned unsigned_abs() const& noexcept {
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

/// Wrapping (modular) absolute value. Computes `this->abs()`, wrapping
/// around at the boundary of the type.
///
/// The only case where such wrapping can occur is when one takes the
/// absolute value of the negative minimal value for the type; this is a
/// positive value that is too large to represent in the type. In such a
/// case, this function returns MIN itself.
sus_pure constexpr _self wrapping_abs() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return abs();
  else
    return MIN;
}

/// Computes the absolute difference between self and other.
///
/// This function always returns the correct answer without overflow or
/// panics by returning an unsigned integer.
sus_pure constexpr _unsigned abs_diff(const _self& r) const& noexcept {
  if (primitive_value >= r.primitive_value) {
    return __private::sub_with_unsigned_positive_result(primitive_value,
                                                        r.primitive_value);
  } else {
    return __private::sub_with_unsigned_positive_result(r.primitive_value,
                                                        primitive_value);
  }
}

/// Checked integer addition. Computes self + rhs, returning None if
/// overflow occurred.
sus_pure constexpr Option<_self> checked_add(const _self& rhs) const& noexcept {
  const auto out =
      __private::add_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>::with(out.value);
  else
    return Option<_self>();
}

/// Checked integer addition with an unsigned rhs. Computes self + rhs,
/// returning None if overflow occurred.
sus_pure constexpr Option<_self> checked_add_unsigned(
    const _unsigned& rhs) const& noexcept {
  const auto out = __private::add_with_overflow_unsigned(primitive_value,
                                                         rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>::with(out.value);
  else
    return Option<_self>();
}

/// Calculates self + rhs
///
/// Returns a tuple of the addition along with a boolean indicating whether
/// an arithmetic overflow would occur. If an overflow would have occurred
/// then the wrapped value is returned.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_add(const _self& rhs) const& noexcept {
  const auto r =
      __private::add_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple::with(r.value, r.overflow);
}

/// Calculates self + rhs with an unsigned rhs
///
/// Returns a tuple of the addition along with a boolean indicating whether
/// an arithmetic overflow would occur. If an overflow would have occurred
/// then the wrapped value is returned.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_add_unsigned(
    const _unsigned& rhs) const& noexcept {
  const auto r = __private::add_with_overflow_unsigned(primitive_value,
                                                       rhs.primitive_value);
  return Tuple::with(r.value, r.overflow);
}

/// Saturating integer addition. Computes self + rhs, saturating at the
/// numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_add(const _self& rhs) const& noexcept {
  return __private::saturating_add(primitive_value, rhs.primitive_value);
}

/// Saturating integer addition with an unsigned rhs. Computes self + rhs,
/// saturating at the numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_add_unsigned(
    const _unsigned& rhs) const& noexcept {
  const auto r = __private::add_with_overflow_unsigned(primitive_value,
                                                       rhs.primitive_value);
  if (!r.overflow) [[likely]]
    return r.value;
  else
    return MAX;
}

/// Unchecked integer addition. Computes self + rhs, assuming overflow
/// cannot occur.
///
/// # Safety
/// This results in undefined behavior when self + rhs > @doc.self::MAX or self
/// + rhs < @doc.self::MIN, i.e. when checked_add() would return None.
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
sus_pure constexpr _self wrapping_add_unsigned(
    const _unsigned& rhs) const& noexcept {
  return __private::add_with_overflow_unsigned(primitive_value,
                                               rhs.primitive_value)
      .value;
}

/// Checked integer division. Computes self / rhs, returning None if rhs ==
/// 0 or the division results in overflow.
sus_pure constexpr Option<_self> checked_div(const _self& rhs) const& noexcept {
  if (__private::div_overflows(primitive_value, rhs.primitive_value))
      [[unlikely]]
    return Option<_self>();
  else
    return Option<_self>::with(
        __private::unchecked_div(primitive_value, rhs.primitive_value));
}

/// Calculates the divisor when self is divided by rhs.
///
/// Returns a tuple of the divisor along with a boolean indicating whether an
/// arithmetic overflow would occur. If an overflow would occur then self is
/// returned.
///
/// # Panics
/// This function will panic if rhs is 0.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_div(const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(rhs.primitive_value != 0);
  if (__private::div_overflows_nonzero(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Tuple::with(MIN, true);
  } else {
    return Tuple::with(
        __private::unchecked_div(primitive_value, rhs.primitive_value), false);
  }
}

/// Saturating integer division. Computes self / rhs, saturating at the
/// numeric bounds instead of overflowing.
///
/// # Panics
/// This function will panic if rhs is 0.
sus_pure constexpr _self saturating_div(const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(rhs.primitive_value != 0);
  if (__private::div_overflows_nonzero(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value))
      [[unlikely]] {
    /* Only overflows in the case of -MIN / -1, which gives MAX + 1,
     saturated to MAX. */
    return MAX;
  } else {
    return __private::unchecked_div(primitive_value, rhs.primitive_value);
  }
}

/// Wrapping (modular) division. Computes self / rhs, wrapping around at the
/// boundary of the type.
///
/// The only case where such wrapping can occur is when one divides MIN / -1
/// on a signed type (where MIN is the negative minimal value for the type);
/// this is equivalent to -MIN, a positive value that is too large to
/// represent in the type. In such a case, this function returns MIN itself.
///
/// # Panics
/// This function will panic if rhs is 0.
sus_pure constexpr _self wrapping_div(const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(rhs.primitive_value != 0);
  if (__private::div_overflows_nonzero(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value))
      [[unlikely]] {
    /* Only overflows in the case of -MIN / -1, which gives MAX + 1,
     that wraps around to MIN. */
    return MIN;
  } else {
    return __private::unchecked_div(primitive_value, rhs.primitive_value);
  }
}

/// Checked integer multiplication. Computes self * rhs, returning None if
/// overflow occurred.
sus_pure constexpr Option<_self> checked_mul(const _self& rhs) const& noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>::with(out.value);
  else
    return Option<_self>();
}

/// Calculates the multiplication of self and rhs.
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

/// Saturating integer multiplication. Computes self * rhs, saturating at
/// the numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_mul(const _self& rhs) const& noexcept {
  return __private::saturating_mul(primitive_value, rhs.primitive_value);
}

/// Unchecked integer multiplication. Computes self * rhs, assuming overflow
/// cannot occur.
///
/// # Safety
/// This results in undefined behavior when `self * rhs > @doc.self::MAX` or
/// `self
/// * rhs < @doc.self::MIN`, i.e. when `checked_mul()` would return None.
sus_pure constexpr inline _self unchecked_mul(
    ::sus::marker::UnsafeFnMarker, const _self& rhs) const& noexcept {
  return __private::unchecked_mul(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) multiplication. Computes self * rhs, wrapping around
/// at the boundary of the type.
sus_pure constexpr _self wrapping_mul(const _self& rhs) const& noexcept {
  return __private::wrapping_mul(primitive_value, rhs.primitive_value);
}

/// Checked negation. Computes -self, returning None if self == MIN.
sus_pure constexpr Option<_self> checked_neg() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return Option<_self>::with(__private::unchecked_neg(primitive_value));
  else
    return Option<_self>();
}

/// Negates self, overflowing if this is equal to the minimum value.
///
/// Returns a tuple of the negated version of self along with a boolean
/// indicating whether an overflow happened. If self is the minimum value
/// (e.g., @doc.self::MIN for values of type @doc.self), then the minimum value
/// will be returned again and true will be returned for an overflow happening.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_neg() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return Tuple::with(__private::unchecked_neg(primitive_value), false);
  else
    return Tuple::with(MIN, true);
}

/// Saturating integer negation. Computes -self, returning MAX if self ==
/// MIN instead of overflowing.
sus_pure constexpr _self saturating_neg() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return __private::unchecked_neg(primitive_value);
  else
    return MAX;
}

/// Wrapping (modular) negation. Computes -self, wrapping around at the
/// boundary of the type.
///
/// The only case where such wrapping can occur is when one negates MIN on
/// a signed type (where MIN is the negative minimal value for the type);
/// this is a positive value that is too large to represent in the type. In
/// such a case, this function returns MIN itself.
sus_pure constexpr _self wrapping_neg() const& noexcept {
  if (primitive_value != MIN_PRIMITIVE) [[likely]]
    return __private::unchecked_neg(primitive_value);
  else
    return MIN;
}

/// Checked integer remainder. Computes self % rhs, returning None if rhs ==
/// 0 or the division results in overflow.
sus_pure constexpr Option<_self> checked_rem(const _self& rhs) const& noexcept {
  if (__private::div_overflows(primitive_value, rhs.primitive_value))
      [[unlikely]]
    return Option<_self>();
  else
    return Option<_self>::with(
        static_cast<_primitive>(primitive_value % rhs.primitive_value));
}

/// Calculates the remainder when self is divided by rhs.
///
/// Returns a tuple of the remainder after dividing along with a boolean
/// indicating whether an arithmetic overflow would occur. If an overflow
/// would occur then 0 is returned.
///
/// # Panics
/// This function will panic if rhs is 0.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_rem(const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(rhs.primitive_value != 0);
  if (__private::div_overflows_nonzero(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Tuple::with(_primitive{0}, true);
  } else {
    return Tuple::with(
        static_cast<_primitive>(primitive_value % rhs.primitive_value), false);
  }
}

/// Wrapping (modular) remainder. Computes self % rhs, wrapping around at
/// the boundary of the type.
///
/// Such wrap-around never actually occurs mathematically; implementation
/// artifacts make x % y invalid for MIN / -1 on a signed type (where MIN
/// is the negative minimal value). In such a case, this function returns 0.
sus_pure constexpr _self wrapping_rem(const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(rhs.primitive_value != 0);
  if (__private::div_overflows_nonzero(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value))
      [[likely]] {
    return _primitive{0};
  } else {
    return static_cast<_primitive>(primitive_value % rhs.primitive_value);
  }
}

/// Calculates the quotient of Euclidean division of self by rhs.
///
/// This computes the integer q such that self = q * rhs + r, with r =
/// self.rem_euclid(rhs) and 0 <= r < abs(rhs).
///
/// In other words, the result is self / rhs rounded to the integer q such
/// that self >= q * rhs. If self > 0, this is equal to round towards zero
/// (the default in Rust); if self < 0, this is equal to round towards +/-
/// infinity.
///
/// # Panics
/// This function will panic if rhs is 0 or the division results in overflow.
sus_pure constexpr _self div_euclid(const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!__private::div_overflows(primitive_value, rhs.primitive_value));
  return __private::div_euclid(::sus::marker::unsafe_fn, primitive_value,
                               rhs.primitive_value);
}

/// Checked Euclidean division. Computes self.div_euclid(rhs), returning
/// None if rhs == 0 or the division results in overflow.
sus_pure constexpr Option<_self> checked_div_euclid(
    const _self& rhs) const& noexcept {
  if (__private::div_overflows(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Option<_self>();
  } else {
    return Option<_self>::with(__private::div_euclid(
        ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value));
  }
}

/// Calculates the quotient of Euclidean division self.div_euclid(rhs).
///
/// Returns a tuple of the divisor along with a boolean indicating whether an
/// arithmetic overflow would occur. If an overflow would occur then self is
/// returned.
///
/// # Panics
/// This function will panic if rhs is 0.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_div_euclid(
    const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(rhs.primitive_value != 0);
  if (__private::div_overflows_nonzero(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Tuple::with(MIN, true);
  } else {
    return Tuple::with(
        __private::div_euclid(::sus::marker::unsafe_fn, primitive_value,
                              rhs.primitive_value),
        false);
  }
}

/// Wrapping Euclidean division. Computes self.div_euclid(rhs), wrapping
/// around at the boundary of the type.
///
/// Wrapping will only occur in MIN / -1 on a signed type (where MIN is the
/// negative minimal value for the type). This is equivalent to -MIN, a
/// positive value that is too large to represent in the type. In this case,
/// this method returns MIN itself.
///
/// # Panics
/// This function will panic if rhs is 0.
sus_pure constexpr _self wrapping_div_euclid(const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(rhs.primitive_value != 0);
  if (__private::div_overflows_nonzero(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return MIN;
  } else {
    return __private::div_euclid(::sus::marker::unsafe_fn, primitive_value,
                                 rhs.primitive_value);
  }
}

/// Calculates the least nonnegative remainder of self (mod rhs).
///
/// This is done as if by the Euclidean division algorithm – given r =
/// self.rem_euclid(rhs), self = rhs * self.div_euclid(rhs) + r, and 0 <= r <
/// abs(rhs).
///
/// # Panics
/// This function will panic if rhs is 0 or the division results in overflow.
sus_pure constexpr _self rem_euclid(const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!__private::div_overflows(primitive_value, rhs.primitive_value));
  return __private::rem_euclid(::sus::marker::unsafe_fn, primitive_value,
                               rhs.primitive_value);
}

/// Checked Euclidean remainder. Computes self.rem_euclid(rhs), returning
/// None if rhs == 0 or the division results in overflow.
sus_pure constexpr Option<_self> checked_rem_euclid(
    const _self& rhs) const& noexcept {
  if (__private::div_overflows(primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Option<_self>();
  } else {
    return Option<_self>::with(__private::rem_euclid(
        ::sus::marker::unsafe_fn, primitive_value, rhs.primitive_value));
  }
}

/// Overflowing Euclidean remainder. Calculates self.rem_euclid(rhs).
///
/// Returns a tuple of the remainder after dividing along with a boolean
/// indicating whether an arithmetic overflow would occur. If an overflow
/// would occur then 0 is returned.
///
/// # Panics
/// This function will panic if rhs is 0.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_rem_euclid(
    const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(rhs.primitive_value != 0);
  if (__private::div_overflows_nonzero(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return Tuple::with(_primitive{0}, true);
  } else {
    return Tuple::with(
        __private::rem_euclid(::sus::marker::unsafe_fn, primitive_value,
                              rhs.primitive_value),
        false);
  }
}

/// Wrapping Euclidean remainder. Computes self.rem_euclid(rhs), wrapping
/// around at the boundary of the type.
///
/// Wrapping will only occur in MIN % -1 on a signed type (where MIN is the
/// negative minimal value for the type). In this case, this method returns
/// 0.
///
/// # Panics
/// This function will panic if rhs is 0.
sus_pure constexpr _self wrapping_rem_euclid(const _self& rhs) const& noexcept {
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(rhs.primitive_value != 0);
  if (__private::div_overflows_nonzero(::sus::marker::unsafe_fn,
                                       primitive_value, rhs.primitive_value))
      [[unlikely]] {
    return _primitive{0};
  } else {
    return __private::rem_euclid(::sus::marker::unsafe_fn, primitive_value,
                                 rhs.primitive_value);
  }
}

/// Checked shift left. Computes `*this << rhs`, returning None if rhs is
/// larger than or equal to the number of bits in self.
sus_pure constexpr Option<_self> checked_shl(const u32& rhs) const& noexcept {
  const auto out =
      __private::shl_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>::with(out.value);
  else
    return Option<_self>();
}

/// Shifts self left by rhs bits.
///
/// Returns a tuple of the shifted version of self along with a boolean
/// indicating whether the shift value was larger than or equal to the number
/// of bits. If the shift value is too large, then value is masked (N-1)
/// where N is the number of bits, and this value is then used to perform the
/// shift.
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
sus_pure constexpr Option<_self> checked_shr(const u32& rhs) const& noexcept {
  const auto out =
      __private::shr_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>::with(out.value);
  else
    return Option<_self>();
}

/// Shifts self right by rhs bits.
///
/// Returns a tuple of the shifted version of self along with a boolean
/// indicating whether the shift value was larger than or equal to the number
/// of bits. If the shift value is too large, then value is masked (N-1)
/// where N is the number of bits, and this value is then used to perform the
/// shift.
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
/// integer types all implement a rotate_right function, which may be what
/// you want instead.
sus_pure constexpr _self wrapping_shr(const u32& rhs) const& noexcept {
  return __private::shr_with_overflow(primitive_value, rhs.primitive_value)
      .value;
}

/// Checked integer subtraction. Computes self - rhs, returning None if
/// overflow occurred.
sus_pure constexpr Option<_self> checked_sub(const _self& rhs) const& {
  const auto out =
      __private::sub_with_overflow(primitive_value, rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>::with(out.value);
  else
    return Option<_self>();
}

/// Checked integer subtraction with an unsigned rhs. Computes self - rhs,
/// returning None if overflow occurred.
sus_pure constexpr Option<_self> checked_sub_unsigned(
    const _unsigned& rhs) const& {
  const auto out = __private::sub_with_overflow_unsigned(primitive_value,
                                                         rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return Option<_self>::with(out.value);
  else
    return Option<_self>();
}

/// Calculates self - rhs
///
/// Returns a tuple of the subtraction along with a boolean indicating
/// whether an arithmetic overflow would occur. If an overflow would have
/// occurred then the wrapped value is returned.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_sub(const _self& rhs) const& noexcept {
  const auto out =
      __private::sub_with_overflow(primitive_value, rhs.primitive_value);
  return Tuple::with(out.value, out.overflow);
}

/// Calculates self - rhs with an unsigned rhs.
///
/// Returns a tuple of the subtraction along with a boolean indicating
/// whether an arithmetic overflow would occur. If an overflow would have
/// occurred then the wrapped value is returned.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_sub_unsigned(
    const _unsigned& rhs) const& noexcept {
  const auto out = __private::sub_with_overflow_unsigned(primitive_value,
                                                         rhs.primitive_value);
  return Tuple::with(out.value, out.overflow);
}

/// Saturating integer subtraction. Computes self - rhs, saturating at the
/// numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_sub(const _self& rhs) const& {
  return __private::saturating_sub(primitive_value, rhs.primitive_value);
}

/// Saturating integer subtraction with an unsigned rhs. Computes self -
/// rhs, saturating at the numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_sub_unsigned(const _unsigned& rhs) const& {
  const auto out = __private::sub_with_overflow_unsigned(primitive_value,
                                                         rhs.primitive_value);
  if (!out.overflow) [[likely]]
    return out.value;
  else
    return MIN;
}

/// Unchecked integer subtraction. Computes self - rhs, assuming overflow
/// cannot occur.
sus_pure inline constexpr _self unchecked_sub(::sus::marker::UnsafeFnMarker,
                                              const _self& rhs) const& {
  return static_cast<_primitive>(primitive_value - rhs.primitive_value);
}

/// Wrapping (modular) subtraction. Computes self - rhs, wrapping around at
/// the boundary of the type.
constexpr _self wrapping_sub(const _self& rhs) const& {
  return __private::wrapping_sub(primitive_value, rhs.primitive_value);
}

/// Wrapping (modular) subtraction with an unsigned rhs. Computes self -
/// rhs, wrapping around at the boundary of the type.
sus_pure constexpr _self wrapping_sub_unsigned(const _unsigned& rhs) const& {
  return __private::sub_with_overflow_unsigned(primitive_value,
                                               rhs.primitive_value)
      .value;
}

/// Returns the number of ones in the binary representation of the current
/// value.
sus_pure constexpr u32 count_ones() const& noexcept {
  return __private::count_ones(__private::into_unsigned(primitive_value));
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
  return __private::leading_zeros(__private::into_unsigned(primitive_value));
}

/// Returns the number of trailing ones in the binary representation of the
/// current value.
sus_pure constexpr u32 trailing_ones() const& noexcept {
  return (~(*this)).trailing_zeros();
}

/// Returns the number of trailing zeros in the binary representation of the
/// current value.
sus_pure constexpr u32 trailing_zeros() const& noexcept {
  return __private::trailing_zeros(__private::into_unsigned(primitive_value));
}

/// Reverses the order of bits in the integer. The least significant bit
/// becomes the most significant bit, second least-significant bit becomes
/// second most-significant bit, etc.
sus_pure constexpr _self reverse_bits() const& noexcept {
  return __private::into_signed(
      __private::reverse_bits(__private::into_unsigned(primitive_value)));
}

/// Shifts the bits to the left by a specified amount, `n`, wrapping the
/// truncated bits to the end of the resulting integer.
///
/// Please note this isn't the same operation as the `<<` shifting operator!
sus_pure constexpr _self rotate_left(const u32& n) const& noexcept {
  return __private::into_signed(__private::rotate_left(
      __private::into_unsigned(primitive_value), n.primitive_value));
}

/// Shifts the bits to the right by a specified amount, n, wrapping the
/// truncated bits to the beginning of the resulting integer.
///
/// Please note this isn't the same operation as the >> shifting operator!
sus_pure constexpr _self rotate_right(const u32& n) const& noexcept {
  return __private::into_signed(__private::rotate_right(
      __private::into_unsigned(primitive_value), n.primitive_value));
}

/// Reverses the byte order of the integer.
sus_pure constexpr _self swap_bytes() const& noexcept {
  return __private::into_signed(
      __private::swap_bytes(__private::into_unsigned(primitive_value)));
}

///  Raises self to the power of `exp`, using exponentiation by squaring. */
sus_pure constexpr inline _self pow(const u32& rhs) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, rhs.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  ::sus::check(!out.overflow);
  return out.value;
}

/// Checked exponentiation. Computes `@doc.self::pow(exp)`, returning None if
/// overflow occurred.
sus_pure constexpr Option<_self> checked_pow(const u32& rhs) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, rhs.primitive_value);
  /* TODO: Allow opting out of all overflow checks? */
  if (!out.overflow) [[likely]]
    return Option<_self>::with(out.value);
  else
    return Option<_self>();
}

/// Raises self to the power of `exp`, using exponentiation by squaring.
///
/// Returns a tuple of the exponentiation along with a bool indicating
/// whether an overflow happened.
template <int&..., class Tuple = ::sus::tuple_type::Tuple<_self, bool>>
sus_pure constexpr Tuple overflowing_pow(const u32& exp) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, exp.primitive_value);
  return Tuple::with(out.value, out.overflow);
}

/// Wrapping (modular) exponentiation. Computes self.pow(exp), wrapping
/// around at the boundary of the type.
sus_pure constexpr _self wrapping_pow(const u32& exp) const& noexcept {
  return __private::wrapping_pow(primitive_value, exp.primitive_value);
}

/// Returns the base 2 logarithm of the number, rounded down.
///
/// Returns None if the number is negative or zero.
sus_pure constexpr Option<u32> checked_log2() const& {
  if (primitive_value <= 0) [[unlikely]] {
    return Option<u32>();
  } else {
    uint32_t zeros = __private::leading_zeros_nonzero(
        ::sus::marker::unsafe_fn, __private::into_unsigned(primitive_value));
    return Option<u32>::with(BITS - 1_u32 - u32(zeros));
  }
}

/// Returns the base 2 logarithm of the number, rounded down.
///
/// # Panics
/// When the number is zero or negative the function will panic.
sus_pure constexpr u32 log2() const& {
  /* TODO: Allow opting out of all overflow checks? */
  return checked_log2().unwrap();
}

/// Returns the base 10 logarithm of the number, rounded down.
///
/// Returns None if the number is negative or zero.
sus_pure constexpr Option<u32> checked_log10() const& {
  if (primitive_value <= 0) [[unlikely]] {
    return Option<u32>();
  } else {
    return Option<u32>::with(__private::int_log10::_self(primitive_value));
  }
}

/// Returns the base 10 logarithm of the number, rounded down.
///
/// # Panics
/// When the number is zero or negative the function will panic.
sus_pure constexpr u32 log10() const& {
  /* TODO: Allow opting out of all overflow checks? */
  return checked_log10().unwrap();
}

/// Returns the logarithm of the number with respect to an arbitrary base,
/// rounded down.
///
/// Returns None if the number is negative or zero, or if the base is not at
/// least 2.
///
/// This method might not be optimized owing to implementation details;
/// `checked_log2` can produce results more efficiently for base 2, and
/// `checked_log10` can produce results more efficiently for base 10.
sus_pure constexpr Option<u32> checked_log(const _self& base) const& noexcept {
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
    return Option<u32>::with(n);
  }
}

/// Returns the logarithm of the number with respect to an arbitrary base,
/// rounded down.
///
/// This method might not be optimized owing to implementation details; log2
/// can produce results more efficiently for base 2, and log10 can produce
/// results more efficiently for base 10.
///
/// # Panics
/// When the number is negative, zero, or if the base is not at least 2.
sus_pure constexpr u32 log(const _self& base) const& noexcept {
  return checked_log(base).unwrap();
}

/// Converts an integer from big endian to the target's endianness.
///
/// On big endian this is a no-op. On little endian the bytes are swapped.
sus_pure static constexpr _self from_be(const _self& x) noexcept {
  if constexpr (::sus::assertions::is_big_endian())
    return x;
  else
    return x.swap_bytes();
}

/// Converts an integer from little endian to the target's endianness.
///
/// On little endian this is a no-op. On big endian the bytes are swapped.
sus_pure static constexpr _self from_le(const _self& x) noexcept {
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

/// Return the memory representation of this integer as a byte array in
/// native byte order.
///
/// As the target platform's native endianness is used, portable code should
/// use `to_be_bytes()` or `to_le_bytes()`, as appropriate, instead.
sus_pure constexpr ::sus::containers::Array<u8,
                                            ::sus::mem::size_of<_primitive>()>
to_ne_bytes() const& noexcept;

/// Create an integer value from its representation as a byte array in big
/// endian.
sus_pure static constexpr _self from_be_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

/// Create an integer value from its representation as a byte array in
/// little endian.
sus_pure static constexpr _self from_le_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

/// Create an integer value from its memory representation as a byte array
/// in native endianness.
///
/// As the target platform's native endianness is used, portable code likely
/// wants to use `from_be_bytes()` or `from_le_bytes()`, as appropriate
/// instead.
sus_pure static constexpr _self from_ne_bytes(
    const ::sus::containers::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

#undef _self
#undef _primitive
#undef _unsigned
