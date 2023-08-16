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

#pragma once

#include "sus/construct/default.h"
#include "sus/construct/from.h"
#include "sus/iter/iterator_concept.h"
#include "sus/macros/pure.h"
#include "sus/marker/unsafe.h"
#include "sus/num/transmogrify.h"
#include "sus/option/option.h"
#include "sus/result/result.h"

namespace sus::num {

/// An integer type that handles overflow instead of panicing.
///
/// The value inside the integer can be accessed or unwrapped like with an
/// `Option`, which will panic if the integer has overflowed. Or it can be
/// converted into an `Option` that will represent the overflow state as `None`.
template <::sus::num::Integer I>
class OverflowInteger {
 public:
  // Default constructs OverflowInteger with the default value of the inner
  // integer type `I`.
  explicit constexpr OverflowInteger() noexcept
    requires(::sus::construct::Default<I>)
      : v_(Option<I>(I())) {}

  /// Constructs an OverflowInteger from the same subspace integer type.
  template <std::convertible_to<I> U>
  explicit constexpr OverflowInteger(U u) noexcept
      : v_(Option<I>(::sus::move(u))) {}

  /// Satisfies `sus::construct::From<OverflowInteger<I>, U>` if the inner
  /// integer type `I` satisfies `sus::construct::From<I, U>`.
  template <class U>
    requires(::sus::construct::From<I, U>)
  sus_pure static constexpr OverflowInteger from(U u) noexcept {
    return OverflowInteger(::sus::move_into(u));
  }

  /// Satisfies `sus::construct::TryFrom<OverflowInteger<I>, U>` if the inner
  /// integer type `I` satisfies `sus::construct::TryFrom<I, U>`.
  template <class U>
    requires(::sus::construct::TryFrom<I, U>)
  sus_pure static constexpr ::sus::result::Result<OverflowInteger,
                                                  ::sus::num::TryFromIntError>
  try_from(U u) noexcept {
    // TODO: Use map() when it exists.
    if (auto r = I::try_from(u); r.is_ok()) {
      return ::sus::result::ok(OverflowInteger(::sus::move(r).unwrap()));
    } else {
      return ::sus::result::err(::sus::move(r).unwrap_err());
    }
  }

  /// Constructs an `OverflowInteger` from an `Iterator` by computing the
  /// product of all elements in the iterator.
  ///
  /// This method should rarely be called directly, as it is used to satisfy the
  /// `sus::iter::Product` concept.
  ///
  /// This method satisfies `sus::iter::Product<OverflowInteger<T>, T>` as well
  /// as `sus::iter::Product<OverflowInteger<T>, OverflowInteger<T>>`, for a
  /// subspace integer type `T`.
  ///
  /// If an iterator yields a subspace integer type, `iter.product()` would
  /// panic on overflow. So instead `iter.product<OverflowInteger<T>>()` can be
  /// used (for integer type `T`) which will perform the product computation and
  /// return an OverflowInteger without ever panicking.
  static constexpr OverflowInteger from_product(
      ::sus::iter::Iterator<I> auto&& it) noexcept
    requires(::sus::mem::IsMoveRef<decltype(it)>)
  {
    // SAFETY: This is not lossy, as all integers can hold positive 1.
    auto p = OverflowInteger(::sus::mog<I>(1));
    for (I i : ::sus::move(it)) p *= i;
    return p;
  }

  static constexpr OverflowInteger from_product(
      ::sus::iter::Iterator<OverflowInteger> auto&& it) noexcept
    requires(::sus::mem::IsMoveRef<decltype(it)>)
  {
    // SAFETY: This is not lossy, as all integers can hold positive 1.
    auto p = OverflowInteger(::sus::mog<I>(1));
    for (OverflowInteger i : ::sus::move(it)) p *= i;
    return p;
  }

  sus_pure bool is_valid() const noexcept { return v_.is_some(); }
  sus_pure bool is_overflow() const noexcept { return v_.is_none(); }

  sus_pure constexpr I as_value() const& noexcept { return v_.as_value(); }
  sus_pure constexpr I as_value_unchecked(
      ::sus::marker::UnsafeFnMarker) const& noexcept {
    return v_.as_value_unchecked(::sus::marker::unsafe_fn);
  }
  sus_pure constexpr I& as_value_mut() & noexcept { return v_.as_value_mut(); }
  sus_pure constexpr I& as_value_unchecked_mut(
      ::sus::marker::UnsafeFnMarker) & noexcept {
    return v_.as_value_unchecked_mut(::sus::marker::unsafe_fn);
  }

  constexpr I unwrap() && noexcept { return ::sus::move(v_).unwrap(); }
  constexpr I unwrap_unchecked(::sus::marker::UnsafeFnMarker) && noexcept {
    return ::sus::move(v_).unwrap_unchecked(::sus::marker::unsafe_fn);
  }

  /// Converts the `OverflowInteger` into an `Option` that contains the integer
  /// value. If overflow has occured, returns `None`.
  sus_pure constexpr Option<I> to_option() const noexcept { return v_; }

  constexpr void operator+=(I rhs) noexcept {
    v_ = v_.and_then([=](I vi) { return vi.checked_add(rhs); });
  }
  constexpr void operator-=(I rhs) noexcept {
    v_ = v_.and_then([=](I vi) { return vi.checked_sub(rhs); });
  }
  constexpr void operator*=(I rhs) noexcept {
    v_ = v_.and_then([=](I vi) { return vi.checked_mul(rhs); });
  }
  constexpr void operator/=(I rhs) noexcept {
    v_ = v_.and_then([=](I vi) { return vi.checked_div(rhs); });
  }
  constexpr void operator%=(I rhs) noexcept {
    v_ = v_.and_then([=](I vi) { return vi.checked_rem(rhs); });
  }

  constexpr void operator+=(OverflowInteger rhs) noexcept {
    v_ = v_.and_then([=](I vi) {
      return rhs.v_.and_then([=](I rhsi) { return vi.checked_add(rhsi); });
    });
  }
  constexpr void operator-=(OverflowInteger rhs) noexcept {
    v_ = v_.and_then([=](I vi) {
      return rhs.v_.and_then([=](I rhsi) { return vi.checked_sub(rhsi); });
    });
  }
  constexpr void operator*=(OverflowInteger rhs) noexcept {
    v_ = v_.and_then([=](I vi) {
      return rhs.v_.and_then([=](I rhsi) { return vi.checked_mul(rhsi); });
    });
  }
  constexpr void operator/=(OverflowInteger rhs) noexcept {
    v_ = v_.and_then([=](I vi) {
      return rhs.v_.and_then([=](I rhsi) { return vi.checked_div(rhsi); });
    });
  }
  constexpr void operator%=(OverflowInteger rhs) noexcept {
    v_ = v_.and_then([=](I vi) {
      return rhs.v_.and_then([=](I rhsi) { return vi.checked_rem(rhsi); });
    });
  }

  sus_pure friend constexpr OverflowInteger operator+(OverflowInteger lhs,
                                                      I rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return lhsi.checked_add(rhs);
    }));
  }
  sus_pure friend constexpr OverflowInteger operator-(OverflowInteger lhs,
                                                      I rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return lhsi.checked_sub(rhs);
    }));
  }
  sus_pure friend constexpr OverflowInteger operator*(OverflowInteger lhs,
                                                      I rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return lhsi.checked_mul(rhs);
    }));
  }
  sus_pure friend constexpr OverflowInteger operator/(OverflowInteger lhs,
                                                      I rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return lhsi.checked_div(rhs);
    }));
  }
  sus_pure friend constexpr OverflowInteger operator%(OverflowInteger lhs,
                                                      I rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return lhsi.checked_rem(rhs);
    }));
  }

  sus_pure friend constexpr OverflowInteger operator+(
      I lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, rhs.v_.and_then([=](I rhsi) {
      return lhs.checked_add(rhsi);
    }));
  }
  sus_pure friend constexpr OverflowInteger operator-(
      I lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, rhs.v_.and_then([=](I rhsi) {
      return lhs.checked_sub(rhsi);
    }));
  }
  sus_pure friend constexpr OverflowInteger operator*(
      I lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, rhs.v_.and_then([=](I rhsi) {
      return lhs.checked_mul(rhsi);
    }));
  }
  sus_pure friend constexpr OverflowInteger operator/(
      I lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, rhs.v_.and_then([=](I rhsi) {
      return lhs.checked_div(rhsi);
    }));
  }
  sus_pure friend constexpr OverflowInteger operator%(
      I lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, rhs.v_.and_then([=](I rhsi) {
      return lhs.checked_rem(rhsi);
    }));
  }

  sus_pure friend constexpr OverflowInteger operator+(
      OverflowInteger lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return rhs.v_.and_then([=](I rhsi) { return lhsi.checked_add(rhsi); });
    }));
  }
  sus_pure friend constexpr OverflowInteger operator-(
      OverflowInteger lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return rhs.v_.and_then([=](I rhsi) { return lhsi.checked_sub(rhsi); });
    }));
  }
  sus_pure friend constexpr OverflowInteger operator*(
      OverflowInteger lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return rhs.v_.and_then([=](I rhsi) { return lhsi.checked_mul(rhsi); });
    }));
  }
  sus_pure friend constexpr OverflowInteger operator/(
      OverflowInteger lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return rhs.v_.and_then([=](I rhsi) { return lhsi.checked_div(rhsi); });
    }));
  }
  sus_pure friend constexpr OverflowInteger operator%(
      OverflowInteger lhs, OverflowInteger rhs) noexcept {
    return OverflowInteger(FROM_OPTION, lhs.v_.and_then([=](I lhsi) {
      return rhs.v_.and_then([=](I rhsi) { return lhsi.checked_rem(rhsi); });
    }));
  }

  // Once overflow (or underflow) occurs, the value is lost, so all overflow or
  // underflow values are considered equal.
  friend bool operator==(OverflowInteger lhs, OverflowInteger rhs) noexcept {
    return lhs.v_ == rhs.v_;
  }
  friend bool operator==(OverflowInteger lhs, I rhs) noexcept {
    return lhs.is_valid() &&
           lhs.v_.as_value_unchecked(::sus::marker::unsafe_fn) == rhs;
  }
  friend bool operator==(I lhs, OverflowInteger rhs) noexcept {
    return rhs.is_valid() &&
           lhs == rhs.v_.as_value_unchecked(::sus::marker::unsafe_fn);
  }

  // Overflow (and underflow) is treated as positive infinity.
  friend std::strong_ordering operator<=>(OverflowInteger lhs,
                                          OverflowInteger rhs) noexcept {
    if (lhs.is_overflow()) {
      if (rhs.is_overflow()) {
        return std::strong_ordering::equivalent;
      } else {
        return std::strong_ordering::greater;
      }
    } else if (rhs.is_overflow()) {
      return std::strong_ordering::less;
    } else {
      // SAFETY: Both lhs and rhs are not overflow, so they have a value in
      // their Options.
      return lhs.v_.as_value_unchecked(::sus::marker::unsafe_fn) <=>
             rhs.v_.as_value_unchecked(::sus::marker::unsafe_fn);
    }
  }
  friend std::strong_ordering operator<=>(OverflowInteger lhs, I rhs) noexcept {
    if (lhs.is_overflow()) {
      return std::strong_ordering::greater;
    } else {
      // SAFETY: lhs is not overflow, so it has a value in its Option.
      return lhs.v_.as_value_unchecked(::sus::marker::unsafe_fn) <=> rhs;
    }
  }
  friend std::strong_ordering operator<=>(I lhs, OverflowInteger rhs) noexcept {
    if (rhs.is_overflow()) {
      return std::strong_ordering::less;
    } else {
      // SAFETY: rhs is not overflow, so it has a value in its Option.
      return lhs <=> rhs.v_.as_value_unchecked(::sus::marker::unsafe_fn);
    }
  }

 private:
  enum FromOption { FROM_OPTION };
  explicit inline constexpr OverflowInteger(FromOption, Option<I> o) noexcept
      : v_(::sus::move(o)) {}

  Option<I> v_;
};

// TODO: Aliases for each integer type?
// using OverflowI32 = OverflowInteger<i32>; ?

}  // namespace sus::num
