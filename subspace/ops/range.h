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

#include "subspace/iter/__private/step.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/mem/move.h"
#include "subspace/ops/ord.h"
#include "subspace/option/option.h"

namespace sus::ops {

/// `RangeBounds` is implemented by Subspace's range types, and produced by
/// range syntax like `..`, `a..`, `..b`, `..=c`, `d..e`, or `f..=g`.
template <class T, class I>
concept RangeBounds = requires(const T& t, T v, I i) {
  { t.start_bound() } -> std::same_as<::sus::option::Option<const I&>>;
  { t.end_bound() } -> std::same_as<::sus::option::Option<const I&>>;
  // Rvalue overloads must not exist as they would return a reference to a
  // temporary.
  requires !requires { ::sus::move(v).start_bound(); };
  requires !requires { ::sus::move(v).end_bound(); };
  { t.contains(i) } -> std::same_as<bool>;
  // These should return a RangeBounds, but we're unable to check that here as
  // the type may return itself and the concept would be cyclical and thus
  // become false.
  { ::sus::move(v).start_at(i) };
  { ::sus::move(v).end_at(i) };
  // start_at() and end_at() are rvalue methods.
  requires !requires { t.start_at(i); };
  requires !requires { t.end_at(i); };
  requires !requires { v.start_at(i); };
  requires !requires { v.end_at(i); };
};

namespace __private {

template <class Final, class T, bool = ::sus::iter::__private::Step<T>>
class RangeIter;

template <class Final, class T>
class RangeIter<Final, T, true> : public ::sus::iter::IteratorImpl<Final, T> {
 public:
  // sus::iter::Iterator trait.
  Option<T> next() noexcept final {
    if (static_cast<Final*>(this)->start == static_cast<Final*>(this)->finish)
      return Option<T>::none();
    return Option<T>::some(
        ::sus::mem::replace(static_cast<Final*>(this)->start,
                            ::sus::iter::__private::step_forward(
                                static_cast<Final*>(this)->start)));
  }

  // sus::iter::Iterator trait optional method.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    Option<::sus::num::usize> steps =
        ::sus::iter::__private::steps_between(
            static_cast<const Final*>(this)->start,
            static_cast<const Final*>(this)->finish);
    const ::sus::num::usize lower = steps.is_some() ? *steps : 0_usize;
    return ::sus::iter::SizeHint(lower, ::sus::move(steps));
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<T> next_back() noexcept {
    if (static_cast<Final*>(this)->start == static_cast<Final*>(this)->finish)
      return Option<T>::none();
    static_cast<Final*>(this)->finish = ::sus::iter::__private::step_backward(
        static_cast<Final*>(this)->finish);
    return Option<T>::some(static_cast<Final*>(this)->finish);
  }

  // TODO: Provide and test overrides of Iterator min(), max(), count(),
  // advance_by(), etc that can be done efficiently here.
};

template <class Final, class T>
class RangeIter<Final, T, false> {};

template <class Final, class T, bool = ::sus::iter::__private::Step<T>>
class RangeFromIter;

template <class Final, class T>
class RangeFromIter<Final, T, true>
    : public ::sus::iter::IteratorImpl<Final, T> {
 public:
  // sus::iter::Iterator trait.
  Option<T> next() noexcept final {
    return Option<T>::some(
        ::sus::mem::replace(static_cast<Final*>(this)->start,
                            ::sus::iter::__private::step_forward(
                                static_cast<Final*>(this)->start)));
  }

  // TODO: Provide and test overrides of Iterator min(), max(), count(),
  // advance_by(), etc that can be done efficiently here.
};

template <class Final, class T>
class RangeFromIter<Final, T, false> {};

}  // namespace __private

/// A (half-open) range bounded inclusively below and exclusively above
/// (`start..end`).
///
/// The range `start..end` contains all values with `start <= x < end`. It is
/// empty if `start >= end`.
///
/// A Range<usize> can be constructed as a literal as `"start..end"_r`.
template <class T>
  requires(::sus::ops::Ord<T>)
class Range final : public __private::RangeIter<Range<T>, T> {
 public:
  /// The beginning of the range, inclusive of the given value.
  T start;
  /// The end of the range, exclusive of the given value.
  //
  // Not named `end` to avoid shadowing IteratorImpl::end(), which then breaks
  // for loops on Range.
  T finish;

  constexpr Range() noexcept
    requires(::sus::construct::Default<T>)
  = default;
  constexpr Range(T start, T finish) noexcept
      : start(::sus::move(start)), finish(::sus::move(finish)) {}

  /// Returns true if `item` is contained in the range.
  //
  // sus::ops::RangeBounds<T> trait.
  constexpr bool contains(const T& item) const noexcept {
    return start <= item && item < finish;
  }
  /// Returns the beginning of the RangeBounds, inclusive of its own value.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr ::sus::option::Option<const T&> start_bound() const& noexcept {
    return ::sus::option::Option<const T&>::some(start);
  }
  constexpr ::sus::option::Option<const T&> start_bound() && = delete;
  /// Returns the end of the RangeBounds, exclusive of its own value.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr ::sus::option::Option<const T&> end_bound() const& noexcept {
    return ::sus::option::Option<const T&>::some(finish);
  }
  constexpr ::sus::option::Option<const T&> end_bound() && = delete;

  /// Returns true if the range contains no items.
  ///
  /// The range is empty if either side is incomparable, such as `f32::NAN`.
  constexpr bool is_empty() const noexcept { return !(start < finish); }

  /// Return a new Range that starts at `t` and ends where the original Range
  /// did.
  constexpr Range start_at(T t) && noexcept {
    return Range(::sus::move(t), ::sus::move(finish));
  }
  /// Return a new Range that starts at where the original Range did and ends at
  /// `t`.
  constexpr Range end_at(T t) && noexcept {
    return Range(::sus::move(start), ::sus::move(t));
  }

  // sus::ops::Eq trait
  constexpr bool operator==(const Range& rhs) const noexcept
    requires Eq<T>
  {
    return start == rhs.start && finish == rhs.finish;
  }
};

/// A range only bounded inclusively below (`start..`).
///
/// The RangeFrom `start..` contains all values with `x >= start`.
///
/// A RangeFrom<usize> can be constructed as a literal as `"start.."_r`.
///
/// Note: Overflow in the `Iterator` implementation returned by `iter()` and
/// `into_iter()` (when the contained data type reaches its numerical limit) is
/// allowed to panic, wrap, or saturate. For integer types like `usize`
/// integers, this follows the normal rules will panic if `usize + 1_usize`
/// would otherwise panic in the build configuration. Note also that overflow
/// happens earlier than you might assume: the overflow happens in the call to
/// next that yields the maximum value, as the range must be set to a state to
/// yield the next value.
template <class T>
  requires(::sus::ops::Ord<T>)
class RangeFrom final : public __private::RangeFromIter<RangeFrom<T>, T> {
 public:
  /// The beginning of the range, inclusive of the given value.
  T start;

  constexpr RangeFrom() noexcept
    requires(::sus::construct::Default<T>)
  = default;
  constexpr RangeFrom(T start) noexcept : start(::sus::move(start)) {}

  /// Returns true if `item` is contained in the range.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr bool contains(const T& item) const noexcept {
    return item >= start;
  }
  /// Returns the beginning of the RangeBounds, inclusive of its own value.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr ::sus::option::Option<const T&> start_bound() const& noexcept {
    return ::sus::option::Option<const T&>::some(start);
  }
  constexpr ::sus::option::Option<const T&> start_bound() && = delete;
  /// Returns `None` for the end of the RangeBounds.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr ::sus::option::Option<const T&> end_bound() const& noexcept {
    return ::sus::option::Option<const T&>::none();
  }
  constexpr ::sus::option::Option<const T&> end_bound() && = delete;

  /// Return a new RangeFrom that starts at `t` and still has no end.
  constexpr RangeFrom start_at(T t) && noexcept {
    return RangeFrom(::sus::move(t));
  }
  /// Return a new Range that starts at where the original Range did and ends at
  /// `t`.
  constexpr Range<T> end_at(T t) && noexcept {
    return Range<T>(::sus::move(start), ::sus::move(t));
  }

  // sus::ops::Eq trait
  constexpr bool operator==(const RangeFrom& rhs) const noexcept
    requires Eq<T>
  {
    return start == rhs.start;
  }
};

/// A range only bounded exclusively above (`..end`).
///
/// The RangeTo `..end` contains all values with `x < end`. It cannot serve as
/// an Iterator because it doesn't have a starting point.
///
/// A RangeTo<usize> can be constructed as a literal as `"..end"_r`.
template <class T>
  requires(::sus::ops::Ord<T>)
class RangeTo final {
 public:
  /// The end of the range, exclusive of the given value.
  T finish;

  constexpr RangeTo() noexcept
    requires(::sus::construct::Default<T>)
  = default;
  constexpr RangeTo(T finish) noexcept : finish(::sus::move(finish)) {}

  /// Returns true if `item` is contained in the range.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr bool contains(const T& item) const noexcept {
    return item < finish;
  }
  /// Returns `None` for the beginning of the RangeBounds.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr ::sus::option::Option<const T&> start_bound() const& noexcept {
    return ::sus::option::Option<const T&>::none();
  }
  constexpr ::sus::option::Option<const T&> start_bound() && = delete;
  /// Returns the end of the RangeBounds, exclusive of its own value.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr ::sus::option::Option<const T&> end_bound() const& noexcept {
    return ::sus::option::Option<const T&>::some(finish);
  }
  constexpr ::sus::option::Option<const T&> end_bound() && = delete;

  /// Return a new Range that starts at `t` and ends where the original Range
  /// did.
  constexpr Range<T> start_at(T t) && noexcept {
    return Range<T>(::sus::move(t), ::sus::move(finish));
  }
  /// Return a new Range that still has no start and ends at `t`.
  constexpr RangeTo end_at(T t) && noexcept { return RangeTo(::sus::move(t)); }

  // sus::ops::Eq trait
  constexpr bool operator==(const RangeTo& rhs) const noexcept
    requires Eq<T>
  {
    return finish == rhs.finish;
  }
};

/// An unbounded range (`..`).
///
/// RangeFull is primarily used as a slicing index. It cannot serve as
/// an Iterator because it doesn't have a starting point.
///
/// A RangeFull<usize> can be constructed as a literal as `".."_r`.
template <class T>
  requires(::sus::ops::Ord<T>)
class RangeFull final {
 public:
  RangeFull() = default;

  /// Returns true if `item` is contained in the range. For RangeFull it is
  /// always true.
  //
  // sus::ops::RangeBounds<T> trait.
  constexpr bool contains(const T&) const noexcept { return true; }
  /// Returns `None` for the start of the RangeBounds.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr ::sus::option::Option<const T&> start_bound() const& noexcept {
    return ::sus::option::Option<const T&>::none();
  }
  constexpr ::sus::option::Option<const T&> start_bound() && = delete;
  /// Returns `None` for the end of the RangeBounds.
  ///
  /// Part of the sus::ops::RangeBounds<T> trait.
  constexpr ::sus::option::Option<const T&> end_bound() const& noexcept {
    return ::sus::option::Option<const T&>::none();
  }
  constexpr ::sus::option::Option<const T&> end_bound() && = delete;

  /// Return a new Range that starts at `t` and has no end.
  constexpr RangeFrom<T> start_at(T t) && noexcept {
    return RangeFrom<T>(::sus::move(t));
  }
  /// Return a new Range that has no start and ends at `t`.
  constexpr RangeTo<T> end_at(T t) && noexcept {
    return RangeTo<T>(::sus::move(t));
  }

  // sus::ops::Eq trait
  constexpr bool operator==(const RangeFull& rhs) const noexcept
    requires Eq<T>
  {
    return true;
  }
};

}  // namespace sus::ops
