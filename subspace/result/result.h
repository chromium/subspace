// Copyright 2022 Google LLC
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

// TODO: Support for Result<T&, E>.

// TODO: and, and_then, as_mut, as_ref, cloned, copied, expect, expect_err,
// etc...
// https://doc.rust-lang.org/std/result/enum.Result.html

#include "assertions/check.h"
#include "assertions/unreachable.h"
#include "iter/__private/adaptors.h"
#include "iter/iterator_defn.h"
#include "iter/once.h"
#include "macros/no_unique_address.h"
#include "marker/unsafe.h"
#include "mem/clone.h"
#include "mem/copy.h"
#include "mem/move.h"
#include "mem/mref.h"
#include "mem/relocate.h"
#include "mem/replace.h"
#include "mem/take.h"
#include "option/option.h"
#include "result/__private/marker.h"
#include "result/__private/storage.h"

namespace sus::result {

using sus::iter::Iterator;
using sus::iter::Once;

/// The representation of an Result's state, which can either be #Ok to
/// represent it has a success value, or #Err for when it is holding an error
/// value.
enum class State : bool {
  /// The Result is holding an error value.
  Err = 0,
  /// The Result is holding a success value.
  Ok = 1,
};
using State::Err;
using State::Ok;

template <class T, class E>
class [[nodiscard]] Result final {
  static_assert(!std::is_reference_v<T>,
                "References in Result are not yet supported.");
  static_assert(!std::is_reference_v<E>,
                "References in Result are not yet supported.");

 public:
  using OkType = T;
  using ErrType = E;

  /// Construct an Result that is holding the given success value.
  static constexpr inline Result with(const T& t) noexcept
    requires(::sus::mem::Copy<T>)
  {
    return Result(WithOk, t);
  }
  /// Construct an Result that is holding the given success value.
  static constexpr inline Result with(T&& t) noexcept
    requires(::sus::mem::Move<T>)
  {
    return Result(WithOk, ::sus::move(t));
  }

  /// Construct an Result that is holding the given error value.
  static constexpr inline Result with_err(const E& e) noexcept
    requires(::sus::mem::Copy<E>)
  {
    return Result(WithErr, e);
  }
  /// Construct an Result that is holding the given error value.
  static constexpr inline Result with_err(E&& e) noexcept
    requires(::sus::mem::Move<E>)
  {
    return Result(WithErr, ::sus::move(e));
  }

  /// Takes each element in the Iterator: if it is an Err, no further elements
  /// are taken, and the Err is returned. Should no Err occur, a container with
  /// the values of each Result is returned.
  ///
  /// sus::iter::FromIterator trait.
  template <class U>
  static constexpr Result from_iter(
      ::sus::iter::IteratorBase<Result<U, E>>&& iter) noexcept
    requires(::sus::iter::FromIterator<T, U>)
  {
    auto err = Option<E>::none();
    auto success_out =
        Result::with(T::from_iter(::sus::iter::__private::Unwrapper(
            ::sus::move(iter), mref(err),
            [](Result<U, E>&& r) { return static_cast<Result<U, E>&&>(r); })));
    return ::sus::move(err).map_or_else(
        [&]() { return ::sus::move(success_out); },
        [](E e) { return Result::with_err(e); });
  }

  /// Destructor for the Result.
  ///
  /// If T and E can be trivially destroyed, we don't need to explicitly destroy
  /// them, so we can use the default destructor, which allows Result<T, E> to
  /// also be trivially destroyed.
  constexpr ~Result()
    requires(std::is_trivially_destructible_v<T> &&
             std::is_trivially_destructible_v<E>)
  = default;

  /// Destructor for the Result.
  ///
  /// Destroys the Ok or Err value contained within the Result.
  constexpr inline ~Result() noexcept
    requires(!std::is_trivially_destructible_v<T> ||
             !std::is_trivially_destructible_v<E>)
  {
    switch (state_) {
      case IsMoved: break;
      case IsOk: storage_.ok_.~T(); break;
      case IsErr: storage_.err_.~E(); break;
    }
  }

  /// If T and E can be trivially move-constructed, Result<T, E> can also be
  /// trivially move-constructed.
  constexpr Result(Result&&)
    requires(::sus::mem::Move<T> && ::sus::mem::Move<E> &&
             std::is_trivially_move_constructible_v<T> &&
             std::is_trivially_move_constructible_v<E>)
  = default;

  Result(Result&& rhs) noexcept
    requires(::sus::mem::Move<T> && ::sus::mem::Move<E> &&
             !(std::is_trivially_move_constructible_v<T> &&
               std::is_trivially_move_constructible_v<E>))
      : state_(::sus::mem::replace(mref(rhs.state_), IsMoved)) {
    ::sus::check(state_ != IsMoved);
    switch (state_) {
      case IsOk: new (&storage_.ok_) T(::sus::move(rhs.storage_.ok_)); break;
      case IsErr: new (&storage_.err_) E(::sus::move(rhs.storage_.err_)); break;
      case IsMoved:
        // SAFETY: The state_ is verified to be Ok or Err at the top of the
        // function.
        ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
    }
  }

  constexpr Result(Result&&)
    requires(!::sus::mem::Move<T> || !::sus::mem::Move<E>)
  = delete;

  /// If T and E can be trivially move-assigned, Result<T, E> can also be
  /// trivially move-assigned.
  constexpr Result& operator=(Result&& o)
    requires(::sus::mem::Move<T> && ::sus::mem::Move<E> &&
             std::is_trivially_move_assignable_v<T> &&
             std::is_trivially_move_assignable_v<E>)
  = default;

  Result& operator=(Result&& o) noexcept
    requires(::sus::mem::Move<T> && ::sus::mem::Move<E> &&
             !(std::is_trivially_move_assignable_v<T> &&
               std::is_trivially_move_assignable_v<E>))
  {
    switch (state_) {
      case IsOk:
        switch (state_ = replace(mref(o.state_), IsMoved)) {
          case IsOk:
            mem::replace_and_discard(mref(storage_.ok_),
                                     ::sus::move(o.storage_.ok_));
            break;
          case IsErr:
            storage_.ok_.~T();
            new (&storage_.err_) E(::sus::move(o.storage_.err_));
            break;
          case IsMoved: unreachable();
        }
        break;
      case IsErr:
        switch (state_ = replace(mref(o.state_), IsMoved)) {
          case IsErr:
            mem::replace_and_discard(mref(storage_.err_),
                                     ::sus::move(o.storage_.err_));
            break;
          case IsOk:
            storage_.err_.~T();
            new (&storage_.ok_) T(::sus::move(o.storage_.ok_));
            break;
          case IsMoved: unreachable();
        }
        break;
      case IsMoved:
        switch (state_ = replace(mref(o.state_), IsMoved)) {
          case IsErr:
            new (&storage_.err_) T(::sus::move(o.storage_.err_));
            break;
          case IsOk: new (&storage_.ok_) T(::sus::move(o.storage_.ok_)); break;
          case IsMoved: unreachable();
        }
        break;
    }
    return *this;
  }

  constexpr Result& operator=(Result&& o)
    requires(!::sus::mem::Move<T> || !::sus::mem::Move<E>)
  = delete;

  constexpr Result clone() const& noexcept
    requires(::sus::mem::Clone<T> && ::sus::mem::Clone<E>)
  {
    ::sus::check(state_ != IsMoved);
    switch (state_) {
      case IsOk: return Result::with(::sus::clone(storage_.ok_));
      case IsErr: return Result::with_err(::sus::clone(storage_.err_));
      case IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the
    // function.
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  void clone_from(const Result& source) &
        requires(::sus::mem::Clone<T> && ::sus::mem::Clone<E>)
  {
    ::sus::check(source.state_ != IsMoved);
    if (state_ == source.state_) {
      switch (state_) {
        case IsOk:
          ::sus::clone_into(mref(storage_.ok_), source.storage_.ok_);
          break;
        case IsErr:
          ::sus::clone_into(mref(storage_.err_), source.storage_.err_);
          break;
        case IsMoved: ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
      }
    } else {
      *this = source.clone();
    }
  }

  /// Returns true if the result is `Ok`.
  constexpr inline bool is_ok() const& noexcept {
    ::sus::check(state_ != IsMoved);
    return state_ == IsOk;
  }

  /// Returns true if the result is `Err`.
  constexpr inline bool is_err() const& noexcept {
    ::sus::check(state_ != IsMoved);
    return state_ == IsErr;
  }

  /// An operator which returns the state of the Result, either #Ok or #Err.
  ///
  /// This supports the use of an Result in a `switch()`, allowing it to act as
  /// a tagged union between "success" and "error".
  ///
  /// # Example
  ///
  /// ```cpp
  /// auto x = Result<int, char>::with(2);
  /// switch (x) {
  ///  case Ok:
  ///   return sus::move(x).unwrap_unchecked(::sus::marker::unsafe_fn);
  ///  case Err:
  ///   return -1;
  /// }
  /// ```
  constexpr inline operator State() const& noexcept {
    ::sus::check(state_ != IsMoved);
    return static_cast<State>(state_);
  }

  /// Converts from `Result<T, E>` to `Option<T>`.
  ///
  /// Converts self into an `Option<T>`, consuming self, and discarding the
  /// error, if any.
  constexpr inline Option<T> ok() && noexcept {
    ::sus::check(state_ != IsMoved);
    switch (::sus::mem::replace(mref(state_), IsMoved)) {
      case IsOk:
        return Option<T>::some(::sus::mem::take_and_destruct(
            ::sus::marker::unsafe_fn, mref(storage_.ok_)));
      case IsErr: storage_.err_.~E(); return Option<T>::none();
      case IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the
    // function.
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  /// Converts from `Result<T, E>` to `Option<E>`.
  ///
  /// Converts self into an `Option<E>`, consuming self, and discarding the
  /// success value, if any.
  constexpr inline Option<E> err() && noexcept {
    ::sus::check(state_ != IsMoved);
    switch (::sus::mem::replace(mref(state_), IsMoved)) {
      case IsOk: storage_.ok_.~T(); return Option<E>::none();
      case IsErr:
        return Option<E>::some(::sus::mem::take_and_destruct(
            ::sus::marker::unsafe_fn, mref(storage_.err_)));
      case IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the
    // function.
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  /// Returns the contained `Ok` value, consuming the self value.
  ///
  /// Because this function may panic, its use is generally discouraged.
  /// Instead, prefer to use pattern matching and handle the `Err` case
  /// explicitly, or call `unwrap_or()`, `unwrap_or_else()`, or
  /// `unwrap_or_default()`.
  ///
  /// # Panics
  /// Panics if the value is an `Err`.
  constexpr inline T unwrap() && noexcept {
    check_with_message(::sus::mem::replace(mref(state_), IsMoved) == IsOk,
                       *"called `Result::unwrap()` on an `Err` value");
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                         mref(storage_.ok_));
  }

  /// Returns the contained `Ok` value, consuming the self value, without
  /// checking that the value is not an `Err`.
  ///
  /// # Safety
  /// Calling this method on an `Err` is Undefined Behavior.
  constexpr inline T unwrap_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                         mref(storage_.ok_));
  }

  /// Returns the contained `Err` value, consuming the self value.
  ///
  /// # Panics
  /// Panics if the value is an `Ok`.
  constexpr inline E unwrap_err() && noexcept {
    check_with_message(::sus::mem::replace(mref(state_), IsMoved) == IsErr,
                       *"called `Result::unwrap_err()` on an `Ok` value");
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                         mref(storage_.err_));
  }

  /// Returns the contained `Err` value, consuming the self value, without
  /// checking that the value is not an `Ok`.
  ///
  /// # Safety
  /// Calling this method on an `Ok` is Undefined Behavior.
  constexpr inline E unwrap_err_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                         mref(storage_.err_));
  }

  constexpr Iterator<Once<const T&>> iter() const& noexcept {
    ::sus::check(state_ != IsMoved);
    if (state_ == IsOk)
      return sus::iter::once(Option<const T&>::some(storage_.ok_));
    else
      return sus::iter::once(Option<const T&>::none());
  }
  Iterator<Once<const T&>> iter() const&& = delete;

  constexpr Iterator<Once<T&>> iter_mut() & noexcept {
    ::sus::check(state_ != IsMoved);
    if (state_ == IsOk)
      return sus::iter::once(Option<T&>::some(mref(storage_.ok_)));
    else
      return sus::iter::once(Option<T&>::none());
  }

  constexpr Iterator<Once<T>> into_iter() && noexcept {
    ::sus::check(state_ != IsMoved);
    if (::sus::mem::replace(mref(state_), IsMoved) == IsOk) {
      return sus::iter::once(Option<T>::some(::sus::mem::take_and_destruct(
          ::sus::marker::unsafe_fn, mref(storage_.ok_))));
    } else {
      storage_.err_.~E();
      return sus::iter::once(Option<T>::none());
    }
  }

 private:
  enum WithOkType { WithOk };
  constexpr inline Result(WithOkType, const T& t) noexcept
      : state_(IsOk), storage_(__private::kWithT, t) {}
  constexpr inline Result(WithOkType, T&& t) noexcept
    requires(::sus::mem::Move<T>)
      : state_(IsOk), storage_(__private::kWithT, ::sus::move(t)) {}
  enum WithErrType { WithErr };
  constexpr inline Result(WithErrType, const E& e) noexcept
      : state_(IsErr), storage_(__private::kWithE, e) {}
  constexpr inline Result(WithErrType, E&& e) noexcept
    requires(::sus::mem::Move<E>)
      : state_(IsErr), storage_(__private::kWithE, ::sus::move(e)) {}

  [[sus_no_unique_address]] __private::Storage<T, E> storage_;
  enum FullState { IsErr = 0, IsOk = 1, IsMoved = 2 } state_;

  sus_class_maybe_trivial_relocatable_types(::sus::marker::unsafe_fn, T, E);
};

template <class T>
[[nodiscard]] inline constexpr auto ok(
    T&& t sus_if_clang([[clang::lifetimebound]])) noexcept {
  return __private::OkMarker<T&&>(::sus::forward<T>(t));
}

template <class E>
[[nodiscard]] inline constexpr auto err(
    E&& e sus_if_clang([[clang::lifetimebound]])) noexcept {
  return __private::ErrMarker<E&&>(::sus::forward<E>(e));
}

}  // namespace sus::result
