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
// Include it into the body of your class.
//
// Define `_self` to the name of the integer type.
// Define `_primitive` to the primitive it holds inside.
// Define `_signed` to the signed integer type of the same size.
// Define `_pointer` to true if the type is `uptr` or false otherwise.
// Define `_pointer_sized` to the same-size safe unsigned integer, when _pointer
//   is true.
///////////////////////////////////////////////////////////////////////////

/// The inner primitive value. Prefer to cast to the desired primitive type,
/// such as with `uint32_t{n}` for a numeric value `n`.
_primitive primitive_value{0u};

/// The smallest value that can be represented by an
/// [`@doc.self`]($sus::num::@doc.self), as a native C++ primitive.
static constexpr auto MIN_PRIMITIVE = __private::min_value<_primitive>();
/// The largest value that can be represented by an
/// [`@doc.self`]($sus::num::@doc.self), as a native C++ primitive.
static constexpr auto MAX_PRIMITIVE = __private::max_value<_primitive>();
/// The smallest value that can be represented by an
/// [`@doc.self`]($sus::num::@doc.self).
static const _self MIN;

#if _pointer
/// The largest address, the same value as
/// [`usize::MAX`]($sus::num::usize::MAX) but represented as a
/// [`uptr`]($sus::num::uptr).
static const _self MAX_ADDR;
/// The largest value that can be represented by an
/// [`@doc.self`]($sus::num::@doc.self), if the size of a pointer is larger
/// than the size of an address, this value will be larger than
/// [`MAX_ADDR`]($sus::num::@doc.self::MAX_ADDR) and will always be the maximum
/// value for the integer type.
static const _self MAX_BIT_PATTERN;
#else
/// The largest value that can be represented by an
/// [`@doc.self`]($sus::num::@doc.self).
static const _self MAX;
#endif

/// The size of and [`@doc.self`]($sus::num::@doc.self) in bits.
static const u32 BITS;

/// Default constructor, which sets the integer to 0.
///
/// Satisfies the [`Default`]($sus::construct::Default) concept.
///
/// #[doc.overloads=ctor.default]
constexpr inline _self() noexcept = default;

#if _pointer  // Pointers need pointer provenance to be constructed.

/// Construction from a pointer.
///
/// #[doc.overloads=ctor.unsigned.pointer]
template <class T>
explicit _self(T* v) noexcept
    : primitive_value(reinterpret_cast<_primitive>(v)) {}

/// Construction from a null pointer.
///
/// This produces the same value as the default constructor.
///
/// # Implementation notes
///
/// The function receives std::same_as<nullptr_t> to avoid implicit conversions
/// from integer types on some compilers (like GCC 13).
///
/// #[doc.overloads=ctor.unsigned.nullptr]
explicit constexpr _self(std::same_as<std::nullptr_t> auto) noexcept
    : primitive_value(0u) {}

/// Construction from unsigned types of the same size as a pointer.
///
/// The construction of a `uptr` from an integer must be explicit, unlike for
/// other integer types.
///
/// #[doc.overloads=ctor.unsigned.ptrsized]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() == ::sus::mem::size_of<_primitive>())
explicit constexpr inline _self(U v) noexcept
    : primitive_value(v.primitive_value) {}

/// Construction from unsigned primitive types the same size as a pointer.
///
/// The construction of a `uptr` from a primitive integer must be explicit,
/// unlike for other integer types.
///
/// #[doc.overloads=ctor.unsigned.ptrsized.primitive]
template <UnsignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() == ::sus::mem::size_of<_primitive>())
explicit constexpr inline _self(P v) noexcept : primitive_value(v) {}

#else

/// Construction from unsigned types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned). For lossy conversions
/// use the [`Transmogrify`]($sus::construct::Transmogrify) concept with
/// [`sus::mog<@doc.self>(x)`]($sus::construct::mog).
///
/// #[doc.overloads=ctor.unsigned.small.typed]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(U v) noexcept : primitive_value(v.primitive_value) {}

/// Construction from unsigned primitive types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.primitive). For lossy
/// conversions use the [`Transmogrify`]($sus::construct::Transmogrify) concept
/// with [`sus::mog<@doc.self>(x)`]($sus::construct::mog).
///
/// #[doc.overloads=ctor.unsigned.small.primitive]
template <UnsignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from unsigned enum types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.enum). For lossy
/// conversions use the [`Transmogrify`]($sus::construct::Transmogrify) concept
/// with [`sus::mog<@doc.self>(x)`]($sus::construct::mog).
///
/// #[doc.overloads=ctor.unsigned.smallenum]
template <UnsignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self(P v) noexcept : primitive_value(v) {}

/// Construction from unsigned enum class types where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.enum). For lossy
/// conversions use the
/// [`Transmogrify`]($sus::construct::Transmogrify) concept with
/// [`sus::mog<@doc.self>(x)`]($sus::construct::mog).
///
/// #[doc.overloads=ctor.unsigned.smallenumclass]
template <UnsignedPrimitiveEnumClass P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
explicit constexpr inline _self(P v) noexcept
    : primitive_value(static_cast<_primitive>(v)) {}

#endif

#if _pointer  // Pointers need pointer provenance to be constructed.

/// Assignment from a pointer.
///
/// #[doc.overloads=assign.unsigned.pointer]
template <class T>
_self& operator=(T* v) noexcept {
  primitive_value = reinterpret_cast<_primitive>(v);
  return *this;
}

/// Assignment from a null pointer.
///
/// This sets the `uptr` to the same value as the default constructor.
///
/// # Implementation notes
///
/// The function receives std::same_as<nullptr_t> to avoid implicit conversions
/// from integer types on some compilers (like GCC 13).
///
/// #[doc.overloads=assign.unsigned.nullptr]
constexpr _self& operator=(std::same_as<std::nullptr_t> auto) noexcept {
  primitive_value = 0u;
  return *this;
}

/// Assignment from unsigned types with the same size as a pointer.
///
/// #[doc.overloads=assign.unsigned.small.typed]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() == ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(U v) noexcept {
  primitive_value = v.primitive_value;
  return *this;
}

/// Assignment from unsigned primitive types with the same size as a pointer.
///
/// #[doc.overloads=assign.unsigned.small.primitive]
template <UnsignedPrimitiveInteger P>
  requires(::sus::mem::size_of<P>() == ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(P v) noexcept {
  primitive_value = v;
  return *this;
}

#else

/// Assignment from unsigned types (u8, u16, etc.) where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned). For lossy conversions
/// use the [`Transmogrify`]($sus::construct::Transmogrify) concept with
/// [`sus::mog<@doc.self>(x)`]($sus::construct::mog).
///
/// #[doc.overloads=assign.unsigned.small.typed]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(U v) noexcept {
  primitive_value = v.primitive_value;
  return *this;
}

/// Assignment from unsigned primitive types (unsigned int, unsigned long, etc.)
/// where no bits are lost.
///
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.primitive). For lossy
/// conversions use the
/// [`Transmogrify`]($sus::construct::Transmogrify) concept with
/// [`sus::mog<@doc.self>(x)`]($sus::construct::mog).
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
/// For conversions from types with a larger range use
/// [`try_from`]($sus::num::@doc.self::try_from!unsigned.primitive). For lossy
/// conversions use the
/// [`Transmogrify`]($sus::construct::Transmogrify) concept with
/// [`sus::mog<@doc.self>(x)`]($sus::construct::mog).
///
/// #[doc.overloads=assign.unsigned.smallenum]
template <UnsignedPrimitiveEnum P>
  requires(::sus::mem::size_of<P>() <= ::sus::mem::size_of<_primitive>())
constexpr inline _self& operator=(P v) noexcept {
  primitive_value = static_cast<_primitive>(v);
  return *this;
}

#endif

#if _pointer

/// Creates a new pointer with the given address.
///
/// This converts the address into a pointer-sized integer, but copies the
/// address-space and provenance of the current `uptr`'s value to the new
/// pointer. This allows us to dynamically preserve and propagate this important
/// information in a way that is otherwise impossible with a cast.
_self with_addr(usize addr) const& noexcept {
  _primitive p = _primitive{addr};
  if constexpr (::sus::mem::size_of<_primitive>() !=
                ::sus::mem::size_of<usize>()) {
    constexpr _primitive mask =
        _self::MAX_PRIMITIVE - _primitive{usize::MAX_PRIMITIVE};
    p |= mask & primitive_value;
  }
  return _self(p);
}

/// Constructs a @doc.self from a pointer.
///
/// #[doc.overloads=uptr.from.pointer]
template <class U>
sus_pure static _self from(U* u) noexcept {
  return _self(reinterpret_cast<_primitive>(u));
}

/// Tries to construct a @doc.self from a pointer.
///
/// This operation can not fail, so it always returns success.
///
/// #[doc.overloads=ptr]
template <class U>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U* u) noexcept;

/// Constructs a @doc.self from an unsigned integer type (u8, u16, u32, etc)
/// with the same size as a pointer.
///
/// #[doc.overloads=unsigned.from.unsigned]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() == ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(U u) noexcept {
  if constexpr (MAX_PRIMITIVE < U::MAX_PRIMITIVE)
    ::sus::check(u.primitive_value <= MAX_PRIMITIVE);
  return _self(static_cast<_primitive>(u.primitive_value));
}

/// Tries to construct a @doc.self from an unsigned integer type (u8, u16, u32,
/// etc) with the same size as a pointer.
///
/// As the input has the same size, this function can not return an error.
///
/// #[doc.overloads=unsigned]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() == ::sus::mem::size_of<_primitive>())
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u) noexcept;

/// Constructs a @doc.self from an unsigned primitive integer type (unsigned
/// int, unsigned long, etc) with the same size as a pointer.
///
/// #[doc.overloads=unsigned.from.unsignedprimitive]
template <UnsignedPrimitiveInteger U>
  requires(::sus::mem::size_of<U>() == ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(U u) noexcept {
  if constexpr (MAX_PRIMITIVE < __private::max_value<U>())
    ::sus::check(u <= MAX_PRIMITIVE);
  return _self(static_cast<_primitive>(u));
}

/// Construct a @doc.self from an unsigned primitive integer type
/// (unsigned int, unsigned long, etc) with the same size as a pointer.
///
/// As the input has the same size, this function can not return an error.
///
/// #[doc.overloads=unsigned.primitive]
template <UnsignedPrimitiveInteger U>
  requires(::sus::mem::size_of<U>() == ::sus::mem::size_of<_primitive>())
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u) noexcept;

#else

/// Constructs a @doc.self from an unsigned integer type (u8, u16, u32, etc)
/// where no bits are lost.
///
/// #[doc.overloads=unsigned.from.unsigned]
template <Unsigned U>
  requires(::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(U u) noexcept {
  return _self(static_cast<_primitive>(u.primitive_value));
}

/// Tries to construct a @doc.self from an unsigned integer type (u8, u16, u32,
/// etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=unsigned]
template <Unsigned U>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u) noexcept;

/// Constructs a @doc.self from an unsigned primitive integer type (unsigned
/// int, unsigned long, etc) where no bits are lost.
///
/// #[doc.overloads=unsigned.from.unsignedprimitive]
template <UnsignedPrimitiveInteger U>
  requires(::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(U u) noexcept {
  return _self(static_cast<_primitive>(u));
}

/// Tries to construct a @doc.self from an unsigned primitive integer type
/// (unsigned int, unsigned long, etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=unsigned.primitive]
template <UnsignedPrimitiveInteger U>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u) noexcept;

/// Tries to construct a @doc.self from a signed integer type (i8, i16, i32,
/// etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signed]
template <Signed S>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(S s) noexcept;

/// Tries to construct a @doc.self from a signed primitive integer type (int,
/// long, etc).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signed.primitive]
template <SignedPrimitiveInteger S>
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(S s) noexcept;

/// Tries to construct a @doc.self from a signed enum type (or enum class).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=signed.enum]
template <class S>
  requires(SignedPrimitiveEnum<S> || SignedPrimitiveEnumClass<S>)
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(S s) noexcept;

/// Constructs a @doc.self from an unsigned enum type (or enum class) where no
/// bits are lost.
///
/// #[doc.overloads=unsigned.from.unsignedenum]
template <class U>
  requires((UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>) &&
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
sus_pure static constexpr _self from(U u) noexcept {
  return _self(static_cast<std::underlying_type_t<U>>(u));
}

/// Tries to construct a @doc.self from an unsigned enum type (or enum class).
///
/// Returns an error if the source value is outside of the range of @doc.self.
///
/// #[doc.overloads=unsigned.enum]
template <class U>
  requires(UnsignedPrimitiveEnum<U> || UnsignedPrimitiveEnumClass<U>)
sus_pure static constexpr ::sus::result::Result<_self,
                                                ::sus::num::TryFromIntError>
try_from(U u) noexcept;

#endif

/// Constructs a @doc.self from an `Iterator` by computing the product of all
/// elements in the iterator.
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
/// This method will panic if the product of all values overflows.
static constexpr _self from_product(
    ::sus::iter::Iterator<_self> auto&& it) noexcept
  requires(::sus::mem::IsMoveRef<decltype(it)>)
{
  auto p = _self(_primitive{1u});
  for (_self i : ::sus::move(it)) p *= i;
  return p;
}

/// Constructs a @doc.self from an `Iterator` by computing the sum of all
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
/// This method will panic if the sum of all values overflows.
static constexpr _self from_sum(::sus::iter::Iterator<_self> auto&& it) noexcept
  requires(::sus::mem::IsMoveRef<decltype(it)>)
{
  auto p = _self(_primitive{0u});
  for (_self i : ::sus::move(it)) p += i;
  return p;
}

#if _pointer

/// Conversion from the numeric type to a C++ primitive type.
///
/// This converts to `uintptr_t` but not to other signed or unsigned integer
/// types. Note that the C++ standard library may alias `uintptr_t` with
/// `uint32_t` or `uint64_t` which would then allow that specific conversion as
/// well.
///
/// ```
/// auto d = uintptr_t{uptr()};  // Compiles.
/// auto e = uintptr_t(uptr());  // Compiles.
/// uintptr_t f = uptr();  // Compiles.
///
/// auto d = int64_t{uptr()};  // Does not compile.
/// auto e = int64_t(uptr());  // Does not compile.
/// int64_t f = uptr();  // Does not compile.
/// ```
///
/// Potentially-lossy type conversions can be forced through the
/// [`Transmogrify`]($sus::construct::Transmogrify) concept, such as
/// `sus::mog<uint32_t>(uptr())` or `sus::mog<int32_t>(uptr())`, or even
/// `sus::mog<u32>(uptr())`.
template <std::same_as<uintptr_t> U>
sus_pure constexpr inline operator U() const noexcept {
  // SAFETY: The `primitive_value` in uptr has the same size as `uintptr_t` even
  // though it's not explicitly that type.
  return static_cast<uintptr_t>(primitive_value);
}

#else

/// Conversion from the numeric type to a C++ primitive type.
///
/// This converts to unsigned primitives which are at least as large as the
/// [`@doc.self`]($sus::num::@doc.self).
///
/// ```
/// auto d = uint32_t{3_u32};  // Compiles.
/// auto e = uint32_t(3_u32);  // Compiles.
/// uint32_t f = 3_u32;  // Compiles.
///
/// auto d = uint16_t{3_u32};  // Does not compile.
/// auto e = uint16_t(3_u32);  // Does not compile.
/// uint16_t f = 3_u32;  // Does not compile.
/// ```
///
/// Potentially-lossy type conversions can be forced through the
/// [`Transmogrify`]($sus::construct::Transmogrify) concept, such as
/// `sus::mog<uint16_t>(3_u32)` or `sus::mog<int32_t>(3_u32)`, or even
/// `sus::mog<i32>(3_u32)`.
template <UnsignedPrimitiveInteger U>
  requires(::sus::mem::size_of<U>() >= ::sus::mem::size_of<_primitive>())
sus_pure constexpr inline operator U() const noexcept {
  return primitive_value;
}

#endif

/// Satisfies the [`Eq`]($sus::ops::Eq) concept for unsigned integers.
/// #[doc.overloads=uint.eq]
[[nodiscard]] sus_pure friend constexpr inline bool operator==(
    _self l, _self r) noexcept = default;
/// #[doc.overloads=uint.eq]
[[nodiscard]] sus_pure friend constexpr inline bool operator==(
    _self l, Unsigned auto r) noexcept {
  return l.primitive_value == r.primitive_value;
}
/// #[doc.overloads=uint.eq]
[[nodiscard]] sus_pure friend constexpr inline bool operator==(
    _self l, UnsignedPrimitiveInteger auto r) noexcept {
  return l.primitive_value == r;
}
/// Satisfies the [`StrongOrd`]($sus::ops::StrongOrd) concept for unsigned
/// integers.
/// #[doc.overloads=uint.strongord]
[[nodiscard]] sus_pure friend constexpr inline std::strong_ordering operator<=>(
    _self l, _self r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}
/// #[doc.overloads=uint.strongord]
[[nodiscard]] sus_pure friend constexpr inline std::strong_ordering operator<=>(
    _self l, Unsigned auto r) noexcept {
  return l.primitive_value <=> r.primitive_value;
}
/// #[doc.overloads=uint.strongord]
[[nodiscard]] sus_pure friend constexpr inline std::strong_ordering operator<=>(
    _self l, UnsignedPrimitiveInteger auto r) noexcept {
  return l.primitive_value <=> r;
}

// sus::num::Neg trait intentionally omitted.

/// Satisfies the [`BitNot<@doc.self>`]($sus::num::BitNot) concept.
sus_pure constexpr inline _self operator~() const& noexcept {
  return _self(__private::unchecked_not(primitive_value));
}

/// Satisfies the [`Add`]($sus::num::Add) concept for unsigned integers.
///
/// This operation supports working with primitive unsigned integers that
/// convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=unsignedint.+]
[[nodiscard]] sus_pure friend constexpr inline _self operator+(
    _self l, _self r) noexcept {
  const auto out =
      __private::add_with_overflow(l.primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  return _self(out.value);
}
/// #[doc.overloads=unsignedint.+]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                  //
           (UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
[[nodiscard]] sus_pure_const friend constexpr inline _self operator+(
    _self l, U r) noexcept {
  return l + _self(static_cast<_primitive>(r));
}
/// #[doc.overloads=unsignedint.+]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                  //
           (UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
           ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>())
[[nodiscard]] sus_pure_const friend constexpr inline _self operator+(
    U l, _self r) noexcept {
  return _self(static_cast<_primitive>(l)) + r;
}
/// #[doc.overloads=unsignedint.+]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator+(_self l, U r) noexcept = delete;
/// #[doc.overloads=unsignedint.+]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator+(U l, _self r) noexcept = delete;
/// Satisfies the [`Sub`]($sus::num::Sub) concept for unsigned integers.
///
/// This operation supports working with primitive unsigned integers that
/// convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=unsignedint.-]
[[nodiscard]] sus_pure friend constexpr inline _self operator-(
    _self l, _self r) noexcept {
  const auto out =
      __private::sub_with_overflow(l.primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  return _self(out.value);
}
/// #[doc.overloads=unsignedint.-]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator-(
    _self l, U r) noexcept {
  return l - _self(static_cast<_primitive>(r));
}
/// #[doc.overloads=unsignedint.-]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator-(
    U l, _self r) noexcept {
  return _self(static_cast<_primitive>(l)) - r;
}
/// #[doc.overloads=unsignedint.-]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator-(_self l, U r) noexcept = delete;
/// #[doc.overloads=unsignedint.-]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator-(U l, _self r) noexcept = delete;
/// Satisfies the [`Mul`]($sus::num::Mul) concept for unsigned integers.
///
/// This operation supports working with primitive unsigned integers that
/// convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=unsignedint.*]
[[nodiscard]] sus_pure friend constexpr inline _self operator*(
    _self l, _self r) noexcept {
  const auto out =
      __private::mul_with_overflow(l.primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  return _self(out.value);
}
/// #[doc.overloads=unsignedint.*]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator*(
    _self l, U r) noexcept {
  return l * _self(static_cast<_primitive>(r));
}
/// #[doc.overloads=unsignedint.*]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator*(
    U l, _self r) noexcept {
  return _self(static_cast<_primitive>(l)) * r;
}
/// #[doc.overloads=unsignedint.*]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator*(_self l, U r) noexcept = delete;
/// #[doc.overloads=unsignedint.*]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator*(U l, _self r) noexcept = delete;
/// Satisfies the [`Div`]($sus::num::Div) concept for unsigned integers.
///
/// This operation supports working with primitive unsigned integers that
/// convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=unsignedint./]
[[nodiscard]] sus_pure friend constexpr inline _self operator/(
    _self l, _self r) noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r.primitive_value != 0u);
  return _self(__private::unchecked_div(l.primitive_value, r.primitive_value));
}
/// #[doc.overloads=unsignedint./]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator/(
    _self l, U r) noexcept {
  return l / _self(static_cast<_primitive>(r));
}
/// #[doc.overloads=unsignedint./]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator/(
    U l, _self r) noexcept {
  return _self(static_cast<_primitive>(l)) / r;
}
/// #[doc.overloads=unsignedint./]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator/(_self l, U r) noexcept = delete;
/// #[doc.overloads=unsignedint./]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator/(U l, _self r) noexcept = delete;
/// Satisfies the [`Rem`]($sus::num::Rem) concept for unsigned integers.
///
/// This operation supports working with primitive unsigned integers that
/// convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=unsignedint.%]
[[nodiscard]] sus_pure friend constexpr inline _self operator%(
    _self l, _self r) noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r.primitive_value != 0u);
  return _self(__private::unchecked_rem(l.primitive_value, r.primitive_value));
}
/// #[doc.overloads=unsignedint.%]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator%(
    _self l, U r) noexcept {
  return l % _self(static_cast<_primitive>(r));
}
/// #[doc.overloads=unsignedint.%]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator%(
    U l, _self r) noexcept {
  return _self(static_cast<_primitive>(l)) % r;
}
/// #[doc.overloads=unsignedint.%]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator%(_self l, U r) noexcept = delete;
/// #[doc.overloads=unsignedint.%]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator%(U l, _self r) noexcept = delete;

/// Satisfies the [`BitAnd`]($sus::num::BitAnd) concept for unsigned integers.
///
/// This operation supports working with primitive unsigned integers that
/// convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=unsignedint.&]
[[nodiscard]] sus_pure friend constexpr inline _self operator&(
    _self l, _self r) noexcept {
  return _self(__private::unchecked_and(l.primitive_value, r.primitive_value));
}
/// #[doc.overloads=unsignedint.&]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator&(
    _self l, U r) noexcept {
  return l & _self(static_cast<_primitive>(r));
}
/// #[doc.overloads=unsignedint.&]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator&(
    U l, _self r) noexcept {
  return _self(static_cast<_primitive>(l)) & r;
}
/// #[doc.overloads=unsignedint.&]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator&(_self l, U r) noexcept = delete;
/// #[doc.overloads=unsignedint.&]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator&(U l, _self r) noexcept = delete;
/// Satisfies the [`BitOr`]($sus::num::BitOr) concept for unsigned integers.
///
/// This operation supports working with primitive unsigned integers that
/// convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=unsignedint.|]
[[nodiscard]] sus_pure friend constexpr inline _self operator|(
    _self l, _self r) noexcept {
  return _self(__private::unchecked_or(l.primitive_value, r.primitive_value));
}
/// #[doc.overloads=unsignedint.|]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator|(
    _self l, U r) noexcept {
  return l | _self(static_cast<_primitive>(r));
}
/// #[doc.overloads=unsignedint.|]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator|(
    U l, _self r) noexcept {
  return _self(static_cast<_primitive>(l)) | r;
}
/// #[doc.overloads=unsignedint.|]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator|(_self l, U r) noexcept = delete;
/// #[doc.overloads=unsignedint.|]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator|(U l, _self r) noexcept = delete;
/// Satisfies the [`BitXor`]($sus::num::BitXor) concept for unsigned integers.
///
/// This operation supports working with primitive unsigned integers that
/// convert to the safe numeric, as well as enums.
/// However enum class is excluded as they require an explicit conversion to an
/// integer.
///
/// #[doc.overloads=unsignedint.^]
[[nodiscard]] sus_pure friend constexpr inline _self operator^(
    _self l, _self r) noexcept {
  return _self(__private::unchecked_xor(l.primitive_value, r.primitive_value));
}
/// #[doc.overloads=unsignedint.^]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator^(
    _self l, U r) noexcept {
  return l ^ _self(static_cast<_primitive>(r));
}
/// #[doc.overloads=unsignedint.^]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&                   //
           ((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
            ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
[[nodiscard]] sus_pure_const friend constexpr inline _self operator^(
    U l, _self r) noexcept {
  return _self(static_cast<_primitive>(l)) ^ r;
}
/// #[doc.overloads=unsignedint.^]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator^(_self l, U r) noexcept = delete;
/// #[doc.overloads=unsignedint.^]
template <class U>
  requires((PrimitiveInteger<U> || PrimitiveEnum<U>) &&
           !((UnsignedPrimitiveInteger<U> || UnsignedPrimitiveEnum<U>) &&  //
             ::sus::mem::size_of<U>() <= ::sus::mem::size_of<_primitive>()))
friend constexpr inline _self operator^(U l, _self r) noexcept = delete;
/// Satisfies the [`Shl`]($sus::num::Shl) concept for unsigned integers.
///
/// #[doc.overloads=unsignedint.<<]
[[nodiscard]] sus_pure friend constexpr inline _self operator<<(
    _self l, std::convertible_to<u32> auto r) noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r < u32(__private::num_bits<_primitive>()));
  return _self(
      __private::unchecked_shl(l.primitive_value, u32(r).primitive_value));
}
/// #[doc.overloads=unsignedint.<<]
template <class U>
  requires(!std::convertible_to<U, u32>)
[[nodiscard]] sus_pure friend constexpr inline _self operator<<(
    _self l, U r) noexcept = delete;
/// Satisfies the [`Shr`]($sus::num::Shr) concept for unsigned integers.
///
/// #[doc.overloads=unsignedint.>>]
[[nodiscard]] sus_pure friend constexpr inline _self operator>>(
    _self l, std::convertible_to<u32> auto r) noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r < u32(__private::num_bits<_primitive>()));
  return _self(
      __private::unchecked_shr(l.primitive_value, u32(r).primitive_value));
}
/// #[doc.overloads=unsignedint.<<]
template <class U>
  requires(!std::convertible_to<U, u32>)
[[nodiscard]] sus_pure friend constexpr inline _self operator>>(
    _self l, U r) noexcept = delete;

/// Satisfies the [`AddAssign<@doc.self>`]($sus::num::AddAssign) concept.
constexpr inline void operator+=(_self r) & noexcept {
  const auto out =
      __private::add_with_overflow(primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
#if _pointer  // uptr can't be created from other safe numerics.
constexpr inline void operator+=(_pointer_sized r) & noexcept {
  *this += _self(_primitive{r.primitive_value});
}
#endif
/// Satisfies the [`SubAssign<@doc.self>`]($sus::num::SubAssign) concept.
constexpr inline void operator-=(_self r) & noexcept {
  const auto out =
      __private::sub_with_overflow(primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
#if _pointer  // uptr can't be created from other safe numerics.
constexpr inline void operator-=(_pointer_sized r) & noexcept {
  *this -= _self(_primitive{r.primitive_value});
}
#endif
/// Satisfies the [`MulAssign<@doc.self>`]($sus::num::MulAssign) concept.
constexpr inline void operator*=(_self r) & noexcept {
  const auto out =
      __private::mul_with_overflow(primitive_value, r.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  primitive_value = out.value;
}
#if _pointer  // uptr can't be created from other safe numerics.
constexpr inline void operator*=(_pointer_sized r) & noexcept {
  *this *= _self(_primitive{r.primitive_value});
}
#endif
/// Satisfies the [`DivAssign<@doc.self>`]($sus::num::DivAssign) concept.
constexpr inline void operator/=(_self r) & noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r.primitive_value != 0u);
  primitive_value /= r.primitive_value;
}
#if _pointer  // uptr can't be created from other safe numerics.
constexpr inline void operator/=(_pointer_sized r) & noexcept {
  *this /= _self(_primitive{r.primitive_value});
}
#endif
/// Satisfies the [`RemAssign<@doc.self>`]($sus::num::RemAssign) concept.
constexpr inline void operator%=(_self r) & noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r.primitive_value != 0u);
  primitive_value %= r.primitive_value;
}
#if _pointer  // uptr can't be created from other safe numerics.
constexpr inline void operator%=(_pointer_sized r) & noexcept {
  *this %= _self(_primitive{r.primitive_value});
}
#endif

/// Satisfies the [`BitAndAssign<@doc.self>`]($sus::num::BitAndAssign)
/// concept.
constexpr inline void operator&=(_self r) & noexcept {
  primitive_value &= r.primitive_value;
}
#if _pointer  // uptr can't be created from other safe numerics.
constexpr inline void operator&=(_pointer_sized r) & noexcept {
  *this &= _self(_primitive{r.primitive_value});
}
#endif
/// Satisfies the [`BitOrAssign<@doc.self>`]($sus::num::BitOrAssign) concept.
constexpr inline void operator|=(_self r) & noexcept {
  primitive_value |= r.primitive_value;
}
#if _pointer  // uptr can't be created from other safe numerics.
constexpr inline void operator|=(_pointer_sized r) & noexcept {
  *this |= _self(_primitive{r.primitive_value});
}
#endif
/// Satisfies the [`BitXorAssign<@doc.self>`]($sus::num::BitXorAssign)
/// concept.
constexpr inline void operator^=(_self r) & noexcept {
  primitive_value ^= r.primitive_value;
}
#if _pointer  // uptr can't be created from other safe numerics.
constexpr inline void operator^=(_pointer_sized r) & noexcept {
  *this ^= _self(_primitive{r.primitive_value});
}
#endif
/// Satisfies the [`ShlAssign<@doc.self>`]($sus::num::ShlAssign) concept.
constexpr inline void operator<<=(u32 r) & noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r < u32(__private::num_bits<_primitive>()));
  primitive_value <<= r.primitive_value;
}
/// Satisfies the [`ShrAssign<@doc.self>`]($sus::num::ShrAssign) concept.
constexpr inline void operator>>=(u32 r) & noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(r < u32(__private::num_bits<_primitive>()));
  primitive_value >>= r.primitive_value;
}

/// Computes the absolute difference between self and other.
sus_pure constexpr _self abs_diff(_self r) const& noexcept {
  if (primitive_value >= r.primitive_value)
    return _self(__private::unchecked_sub(primitive_value, r.primitive_value));
  else
    return _self(__private::unchecked_sub(r.primitive_value, primitive_value));
}

/// Checked integer addition. Computes self + rhs, returning None if overflow
/// occurred.
sus_pure constexpr ::sus::option::Option<_self> checked_add(
    _self rhs) const& noexcept;

#if !_pointer

/// Checked integer addition with an unsigned rhs. Computes self + rhs,
/// returning None if overflow occurred.
sus_pure constexpr ::sus::option::Option<_self> checked_add_signed(
    _signed rhs) const& noexcept;

#endif

/// Calculates self + rhs
///
/// Returns a tuple of the addition along with a boolean indicating whether an
/// arithmetic overflow would occur. If an overflow would have occurred then the
/// wrapped value is returned.
sus_pure constexpr sus::tuple_type::Tuple<_self, bool> overflowing_add(
    _self rhs) const& noexcept;

#if !_pointer

/// Calculates self + rhs with an unsigned rhs
///
/// Returns a tuple of the addition along with a boolean indicating whether an
/// arithmetic overflow would occur. If an overflow would have occurred then the
/// wrapped value is returned.
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_add_signed(
    _signed rhs) const& noexcept;

#endif

/// Saturating integer addition. Computes self + rhs, saturating at the numeric
/// bounds instead of overflowing.
sus_pure constexpr _self saturating_add(_self rhs) const& noexcept {
  return _self(__private::saturating_add(primitive_value, rhs.primitive_value));
}

#if !_pointer

/// Saturating integer addition with an unsigned rhs. Computes self + rhs,
/// saturating at the numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_add_signed(_signed rhs) const& noexcept;
#endif

/// Unchecked integer addition. Computes self + rhs, assuming overflow cannot
/// occur.
///
/// # Safety
/// This function is allowed to result in undefined behavior when `self + rhs >
/// @doc.self::MAX` or `self + rhs < @doc.self::MIN`, i.e. when `checked_add()`
/// would return None.
sus_pure inline constexpr _self unchecked_add(::sus::marker::UnsafeFnMarker,
                                              _self rhs) const& noexcept {
  return _self(__private::unchecked_add(primitive_value, rhs.primitive_value));
}

/// Wrapping (modular) addition. Computes self + rhs, wrapping around at the
/// boundary of the type.
sus_pure constexpr _self wrapping_add(_self rhs) const& noexcept {
  return _self(__private::wrapping_add(primitive_value, rhs.primitive_value));
}

#if !_pointer

/// Wrapping (modular) addition with an unsigned rhs. Computes self + rhs,
/// wrapping around at the boundary of the type.
sus_pure constexpr _self wrapping_add_signed(_signed rhs) const& noexcept;

#endif

/// Checked integer division. Computes self / rhs, returning None if `rhs == 0`.
sus_pure constexpr ::sus::option::Option<_self> checked_div(
    _self rhs) const& noexcept;

/// Calculates the divisor when self is divided by rhs.
///
/// Returns a tuple of the divisor along with a boolean indicating whether an
/// arithmetic overflow would occur. Note that for unsigned integers overflow
/// never occurs, so the second value is always false.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_div(
    _self rhs) const& noexcept;

/// Saturating integer division. Computes self / rhs, saturating at the  numeric
/// bounds instead of overflowing.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self saturating_div(_self rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return _self(__private::unchecked_div(primitive_value, rhs.primitive_value));
}

/// Wrapping (modular) division. Computes self / rhs. Wrapped division on
/// unsigned types is just normal division. There's no way wrapping could ever
/// happen. This function exists, so that all operations are accounted for in
/// the wrapping operations.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self wrapping_div(_self rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return _self(__private::unchecked_div(primitive_value, rhs.primitive_value));
}

/// Checked integer multiplication. Computes `self * rhs`, returning None if
/// overflow occurred.
sus_pure constexpr ::sus::option::Option<_self> checked_mul(
    _self rhs) const& noexcept;

/// Calculates the multiplication of self and `rhs`.
///
/// Returns a tuple of the multiplication along with a boolean indicating
/// whether an arithmetic overflow would occur. If an overflow would have
/// occurred then the wrapped value is returned.
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_mul(
    _self rhs) const& noexcept;

/// Saturating integer multiplication. Computes `self * rhs`, saturating at the
/// numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_mul(_self rhs) const& noexcept {
  return _self(__private::saturating_mul(primitive_value, rhs.primitive_value));
}

/// Unchecked integer multiplication. Computes `self * rhs`, assuming overflow
/// cannot occur.
///
/// # Safety
/// This function is allowed to result in undefined behavior when `self * rhs >
/// @doc.self::MAX` or `self * rhs < @doc.self::MIN`, i.e. when `checked_mul()`
/// would return None.
sus_pure constexpr _self unchecked_mul(::sus::marker::UnsafeFnMarker,
                                       _self rhs) const& noexcept {
  return _self(__private::unchecked_mul(primitive_value, rhs.primitive_value));
}

/// Wrapping (modular) multiplication. Computes self * rhs, wrapping around
/// at the boundary of the type.
sus_pure constexpr _self wrapping_mul(_self rhs) const& noexcept {
  return _self(__private::wrapping_mul(primitive_value, rhs.primitive_value));
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
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_neg()
    const& noexcept;

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
    _self rhs) const& noexcept;

/// Calculates the remainder when self is divided by rhs.
///
/// Returns a tuple of the remainder after dividing along with a boolean
/// indicating whether an arithmetic overflow would occur. Note that for
/// unsigned integers overflow never occurs, so the second value is always
/// false.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_rem(
    _self rhs) const& noexcept;

/// Wrapping (modular) remainder. Computes self % rhs. Wrapped remainder
/// calculation on unsigned types is just the regular remainder calculation.
///
/// There's no way wrapping could ever happen. This function exists, so that
/// all operations are accounted for in the wrapping operations.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self wrapping_rem(_self rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return _self(__private::unchecked_rem(primitive_value, rhs.primitive_value));
}

/// Performs Euclidean division.
///
/// Since, for the positive integers, all common definitions of division are
/// equal, this is exactly equal to self / rhs.
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self div_euclid(_self rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return _self(__private::unchecked_div(primitive_value, rhs.primitive_value));
}

/// Checked Euclidean division. Computes self.div_euclid(rhs), returning
/// None if rhs == 0.
sus_pure constexpr ::sus::option::Option<_self> checked_div_euclid(
    _self rhs) const& noexcept;

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
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_div_euclid(
    _self rhs) const& noexcept;

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
sus_pure constexpr _self wrapping_div_euclid(_self rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return _self(__private::unchecked_div(primitive_value, rhs.primitive_value));
}

/// Calculates the least remainder of self (mod rhs).
///
/// Since, for the positive integers, all common definitions of division are
/// equal, this is exactly equal to self % rhs. \
///
/// # Panics
/// This function will panic if `rhs` is 0.
sus_pure constexpr _self rem_euclid(_self rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return _self(__private::unchecked_rem(primitive_value, rhs.primitive_value));
}

/// Checked Euclidean modulo. Computes self.rem_euclid(rhs), returning None if
/// `rhs == 0`.
sus_pure constexpr ::sus::option::Option<_self> checked_rem_euclid(
    _self rhs) const& noexcept;

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
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_rem_euclid(
    _self rhs) const& noexcept;

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
sus_pure constexpr _self wrapping_rem_euclid(_self rhs) const& noexcept {
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(rhs.primitive_value != 0u);
  return _self(__private::unchecked_rem(primitive_value, rhs.primitive_value));
}

/// Checked shift left. Computes `*this << rhs`, returning None if rhs is
/// larger than or equal to the number of bits in self.
sus_pure constexpr ::sus::option::Option<_self> checked_shl(
    u32 rhs) const& noexcept;

/// Shifts self left by rhs bits.
///
/// Returns a tuple of the shifted version of self along with a boolean
/// indicating whether the shift value was larger than or equal to the number of
/// bits. If the shift value is too large, then value is masked (N-1) where N is
/// the number of bits, and this value is then used to perform the shift.
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_shl(
    u32 rhs) const& noexcept;

/// Panic-free bitwise shift-left; yields `*this << mask(rhs)`, where mask
/// removes any high-order bits of `rhs` that would cause the shift to exceed
/// the bitwidth of the type.
///
/// Note that this is not the same as a rotate-left; the RHS of a wrapping
/// shift-left is restricted to the range of the type, rather than the bits
/// shifted out of the LHS being returned to the other end. The primitive
/// integer types all implement a rotate_left function, which may be what you
/// want instead.
sus_pure constexpr _self wrapping_shl(u32 rhs) const& noexcept {
  return _self(
      __private::shl_with_overflow(primitive_value, rhs.primitive_value).value);
}

/// Checked shift right. Computes `*this >> rhs`, returning None if rhs is
/// larger than or equal to the number of bits in self.
sus_pure constexpr ::sus::option::Option<_self> checked_shr(
    u32 rhs) const& noexcept;

/// Shifts self right by rhs bits.
///
/// Returns a tuple of the shifted version of self along with a boolean
/// indicating whether the shift value was larger than or equal to the number of
/// bits. If the shift value is too large, then value is masked (N-1) where N is
/// the number of bits, and this value is then used to perform the shift.
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_shr(
    u32 rhs) const& noexcept;

/// Panic-free bitwise shift-right; yields `*this >> mask(rhs)`, where mask
/// removes any high-order bits of `rhs` that would cause the shift to exceed
/// the bitwidth of the type.
///
/// Note that this is not the same as a rotate-right; the RHS of a wrapping
/// shift-right is restricted to the range of the type, rather than the bits
/// shifted out of the LHS being returned to the other end. The primitive
/// integer types all implement a rotate_right function, which may be what you
/// want instead.
sus_pure constexpr _self wrapping_shr(u32 rhs) const& noexcept {
  return _self(
      __private::shr_with_overflow(primitive_value, rhs.primitive_value).value);
}

/// Checked integer subtraction. Computes self - rhs, returning None if overflow
/// occurred.
sus_pure constexpr ::sus::option::Option<_self> checked_sub(_self rhs) const&;

/// Calculates `self - rhs`.
///
/// Returns a tuple of the subtraction along with a boolean indicating whether
/// an arithmetic overflow would occur. If an overflow would have occurred then
/// the wrapped value is returned.
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_sub(
    _self rhs) const& noexcept;

/// Saturating integer subtraction. Computes self - rhs, saturating at the
/// numeric bounds instead of overflowing.
sus_pure constexpr _self saturating_sub(_self rhs) const& {
  return _self(__private::saturating_sub(primitive_value, rhs.primitive_value));
}

/// Unchecked integer subtraction. Computes self - rhs, assuming overflow cannot
/// occur.
///
/// # Safety
/// This function is allowed to result in undefined behavior when `self
/// - rhs > @doc.self::MAX` or `self - rhs < @doc.self::MIN`, i.e. when
/// `checked_sub()` would return None.
sus_pure constexpr _self unchecked_sub(::sus::marker::UnsafeFnMarker,
                                       _self rhs) const& noexcept {
  return _self(__private::unchecked_sub(primitive_value, rhs.primitive_value));
}

/// Wrapping (modular) subtraction. Computes self - rhs, wrapping around at the
/// boundary of the type.
sus_pure constexpr _self wrapping_sub(_self rhs) const& noexcept {
  return _self(__private::wrapping_sub(primitive_value, rhs.primitive_value));
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
  return _self(__private::reverse_bits(primitive_value));
}

/// Shifts the bits to the left by a specified amount, `n`, wrapping the
/// truncated bits to the end of the resulting integer.
///
/// Please note this isn't the same operation as the `<<` shifting operator!
sus_pure constexpr _self rotate_left(u32 n) const& noexcept {
  return _self(__private::rotate_left(primitive_value, n.primitive_value));
}

/// Shifts the bits to the right by a specified amount, n, wrapping the
/// truncated bits to the beginning of the resulting integer.
///
/// Please note this isn't the same operation as the >> shifting operator!
sus_pure constexpr _self rotate_right(u32 n) const& noexcept {
  return _self(__private::rotate_right(primitive_value, n.primitive_value));
}

/// Reverses the byte order of the integer.
sus_pure constexpr _self swap_bytes() const& noexcept {
  return _self(__private::swap_bytes(primitive_value));
}

/// Raises self to the power of `exp`, using exponentiation by squaring.
sus_pure constexpr _self pow(u32 rhs) const& noexcept {
  const auto out =
      __private::pow_with_overflow(primitive_value, rhs.primitive_value);
  // TODO: Allow opting out of all overflow checks?
  ::sus::check(!out.overflow);
  return _self(out.value);
}

/// Checked exponentiation. Computes `@doc.self::pow(exp)`, returning None if
/// overflow occurred.
sus_pure constexpr ::sus::option::Option<_self> checked_pow(
    u32 rhs) const& noexcept;

/// Raises self to the power of `exp`, using exponentiation by squaring.
///
/// Returns a tuple of the exponentiation along with a bool indicating whether
/// an overflow happened.
sus_pure constexpr ::sus::tuple_type::Tuple<_self, bool> overflowing_pow(
    u32 exp) const& noexcept;

/// Wrapping (modular) exponentiation. Computes self.pow(exp), wrapping around
/// at the boundary of the type.
sus_pure constexpr _self wrapping_pow(u32 exp) const& noexcept {
  return _self(__private::wrapping_pow(primitive_value, exp.primitive_value));
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
    _self base) const& noexcept;

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
sus_pure constexpr u32 log(_self base) const& noexcept;

/// Returns `true` if and only if `self == 2^k` for some `k`.
sus_pure constexpr bool is_power_of_two() const& noexcept {
  return count_ones() == _self(_primitive{1u});
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
[[nodiscard]] sus_pure static constexpr _self from_be(const _self& x) noexcept {
  if constexpr (std::endian::native == std::endian::big)
    return x;
  else
    return x.swap_bytes();
}

/// Converts an integer from little endian to the target's endianness.
///
/// On little endian this is a no-op. On big endian the bytes are swapped.
[[nodiscard]] sus_pure static constexpr _self from_le(const _self& x) noexcept {
  if constexpr (std::endian::native == std::endian::little)
    return x;
  else
    return x.swap_bytes();
}

/// Converts self to big endian from the target's endianness.
///
/// On big endian this is a no-op. On little endian the bytes are swapped.
sus_pure constexpr _self to_be() const& noexcept {
  if constexpr (std::endian::native == std::endian::big)
    return *this;
  else
    return swap_bytes();
}

/// Converts self to little endian from the target's endianness.
///
/// On little endian this is a no-op. On big endian the bytes are swapped.
sus_pure constexpr _self to_le() const& noexcept {
  if constexpr (std::endian::native == std::endian::little)
    return *this;
  else
    return swap_bytes();
}

/// Return the memory representation of this integer as a byte array in
/// big-endian (network) byte order.
sus_pure constexpr ::sus::collections::Array<u8,
                                             ::sus::mem::size_of<_primitive>()>
to_be_bytes() const& noexcept;

/// Return the memory representation of this integer as a byte array in
/// little-endian byte order.
sus_pure constexpr ::sus::collections::Array<u8,
                                             ::sus::mem::size_of<_primitive>()>
to_le_bytes() const& noexcept;

/// Return the memory representation of this integer as a byte array in native
/// byte order.
///
/// As the target platform's native endianness is used, portable code should use
/// `to_be_bytes()` or `to_le_bytes()`, as appropriate, instead.
sus_pure constexpr ::sus::collections::Array<u8,
                                             ::sus::mem::size_of<_primitive>()>
to_ne_bytes() const& noexcept;

/// Create an integer value from its representation as a byte array in big
/// endian.
[[nodiscard]] sus_pure static constexpr _self from_be_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

/// Create an integer value from its representation as a byte array in little
/// endian.
[[nodiscard]] sus_pure static constexpr _self from_le_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

/// Create an integer value from its memory representation as a byte array in
/// native endianness.
///
/// As the target platform's native endianness is used, portable code likely
/// wants to use `from_be_bytes()` or `from_le_bytes()`, as appropriate instead.
[[nodiscard]] sus_pure static constexpr _self from_ne_bytes(
    const ::sus::collections::Array<u8, ::sus::mem::size_of<_primitive>()>&
        bytes) noexcept;

#if _pointer

/// Returns the address portion of the pointer.
///
/// In most systems the address is the entire pointer, but in cases where the
/// pointer is larger, this returns the low bits of the pointer up to the size
/// of an address.
sus_pure constexpr usize addr() const& noexcept {
  return static_cast<decltype(std::declval<usize>().primitive_value)>(
      primitive_value);
}

#endif

#undef _self
#undef _primitive
#undef _signed
#undef _pointer
#undef _pointer_sized
