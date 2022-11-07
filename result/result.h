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
#include "iter/once.h"
#include "marker/unsafe.h"
#include "mem/mref.h"
#include "mem/relocate.h"
#include "mem/replace.h"
#include "mem/take.h"
#include "option/option.h"

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
class Result final {
 public:
  /// Construct an Result that is holding the given success value.
  static constexpr inline Result with(const T& t) noexcept
    requires(std::is_copy_constructible_v<T>)
  {
    return Result(WithOk, t);
  }
  /// Construct an Result that is holding the given success value.
  static constexpr inline Result with(Mref<T&> t) noexcept
    requires(std::is_copy_constructible_v<T>)
  {
    return Result(WithOk, t);
  }
  /// Construct an Result that is holding the given success value.
  static constexpr inline Result with(T&& t) noexcept
    requires(std::is_move_constructible_v<T>)
  {
    return Result(WithOk, static_cast<T&&>(t));
  }

  /// Construct an Result that is holding the given error value.
  static constexpr inline Result with_err(const E& e) noexcept
    requires(std::is_copy_constructible_v<E>)
  {
    return Result(WithErr, e);
  }
  /// Construct an Result that is holding the given error value.
  static constexpr inline Result with_err(Mref<E&> e) noexcept
    requires(std::is_copy_constructible_v<E>)
  {
    return Result(WithErr, e);
  }
  /// Construct an Result that is holding the given error value.
  static constexpr inline Result with_err(E&& e) noexcept
    requires(std::is_move_constructible_v<E>)
  {
    return Result(WithErr, static_cast<E&&>(e));
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
      case IsOk: ok_.~T(); break;
      case IsErr: err_.~E(); break;
    }
  }

  /// If T and E can be trivially copy-constructed, Result<T, E> can also be
  /// trivially copy-constructed.
  constexpr Result(const Result&)
    requires(std::is_trivially_copy_constructible_v<T> &&
             std::is_trivially_copy_constructible_v<E>)
  = default;
  inline Result(const Result& rhs) noexcept
    requires((!std::is_trivially_copy_constructible_v<T> ||
              !std::is_trivially_copy_constructible_v<E>) &&
             std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E>)
  : state_(rhs.state_) {
    check(state_ != IsMoved);
    switch (state_) {
      case IsOk: new (&ok_) T(rhs.ok_); break;
      case IsErr: new (&err_) E(rhs.err_); break;
      case IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the function.
    unreachable_unchecked(unsafe_fn);
  }

  constexpr Result(const Result&)
    requires(!std::is_copy_constructible_v<T> ||
             !std::is_copy_constructible_v<E>)
  = delete;

  /// If T and E can be trivially move-constructed, Result<T, E> can also be
  /// trivially
  /// move-constructed.
  constexpr Result(Result&&)
    requires(std::is_trivially_move_constructible_v<T> &&
             std::is_trivially_move_constructible_v<E>)
  = default;
  inline Result(Result&& rhs) noexcept
    requires((!std::is_trivially_move_constructible_v<T> ||
              !std::is_trivially_move_constructible_v<E>) &&
             std::is_move_constructible_v<T> && std::is_move_constructible_v<E>)
  : state_(replace(mref(rhs.state_), IsMoved)) {
    check(state_ != IsMoved);
    switch (state_) {
      case IsOk: new (&ok_) T(static_cast<T&&>(rhs.ok_)); break;
      case IsErr: new (&err_) E(static_cast<E&&>(rhs.err_)); break;
      case IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the function.
    unreachable_unchecked(unsafe_fn);
  }

  constexpr Result(Result&&)
    requires(!std::is_move_constructible_v<T> ||
             !std::is_move_constructible_v<E>)
  = delete;

  /// If T and E can be trivially copy-assigned, Result<T, E> can also be
  /// trivially copy-assigned.
  constexpr Result& operator=(const Result& o)
    requires(std::is_trivially_copy_assignable_v<T> &&
             std::is_trivially_copy_assignable_v<E>)
  = default;

  Result& operator=(const Result& o) noexcept
    requires((!std::is_trivially_copy_assignable_v<T> ||
              !std::is_trivially_copy_assignable_v<E>) &&
             std::is_copy_assignable_v<T> && std::is_copy_assignable_v<E>)
  {
    switch (state_) {
      case IsOk:
        switch (state_ = replace(mref(o.state_), IsMoved)) {
          case IsOk: mem::replace_and_discard(mref(ok_), o.ok_); break;
          case IsErr:
            ok_.~T();
            new (&err_) E(o.err_);
            break;
          case IsMoved: unreachable();
        }
        break;
      case IsErr:
        switch (state_ = replace(mref(o.state_), IsMoved)) {
          case IsErr: mem::replace_and_discard(mref(err_), o.err_); break;
          case IsOk:
            err_.~T();
            new (&ok_) T(o.ok_);
            break;
          case IsMoved: unreachable();
        }
        break;
      case IsMoved:
        switch (state_ = replace(mref(o.state_), IsMoved)) {
          case IsErr: new (&err_) T(o.err_); break;
          case IsOk: new (&ok_) T(o.ok_); break;
          case IsMoved: unreachable();
        }
        break;
    }
    return *this;
  }

  constexpr Result& operator=(const Result& o)
    requires(!std::is_copy_assignable_v<T> || !std::is_copy_assignable_v<E>)
  = delete;

  /// If T and E can be trivially move-assigned, Result<T, E> can also be
  /// trivially move-assigned.
  constexpr Result& operator=(Result&& o)
    requires(std::is_trivially_move_assignable_v<T> &&
             std::is_trivially_move_assignable_v<E>)
  = default;

  Result& operator=(Result&& o) noexcept
    requires((!std::is_trivially_move_assignable_v<T> ||
              !std::is_trivially_move_assignable_v<E>) &&
             std::is_move_assignable_v<T> && std::is_move_assignable_v<E>)
  {
    switch (state_) {
      case IsOk:
        switch (state_ = replace(mref(o.state_), IsMoved)) {
          case IsOk:
            mem::replace_and_discard(mref(ok_), static_cast<T&&>(o.ok_));
            break;
          case IsErr:
            ok_.~T();
            new (&err_) E(static_cast<E&&>(o.err_));
            break;
          case IsMoved: unreachable();
        }
        break;
      case IsErr:
        switch (state_ = replace(mref(o.state_), IsMoved)) {
          case IsErr:
            mem::replace_and_discard(mref(err_), static_cast<E&&>(o.err_));
            break;
          case IsOk:
            err_.~T();
            new (&ok_) T(static_cast<T&&>(o.ok_));
            break;
          case IsMoved: unreachable();
        }
        break;
      case IsMoved:
        switch (state_ = replace(mref(o.state_), IsMoved)) {
          case IsErr: new (&err_) T(static_cast<E&&>(o.err_)); break;
          case IsOk: new (&ok_) T(static_cast<T&&>(o.ok_)); break;
          case IsMoved: unreachable();
        }
        break;
    }
    return *this;
  }

  constexpr Result& operator=(Result&& o)
    requires(!std::is_move_assignable_v<T> || !std::is_move_assignable_v<E>)
  = delete;

  /// Returns true if the result is `Ok`.
  constexpr inline bool is_ok() const& noexcept {
    check(state_ != IsMoved);
    return state_ == IsOk;
  }

  /// Returns true if the result is `Err`.
  constexpr inline bool is_err() const& noexcept {
    check(state_ != IsMoved);
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
  ///   return sus::move(x).unwrap_unchecked(unsafe_fn);
  ///  case Err:
  ///   return -1;
  /// }
  /// ```
  constexpr inline operator State() const& noexcept {
    check(state_ != IsMoved);
    return static_cast<State>(state_);
  }

  /// Converts from `Result<T, E>` to `Option<T>`.
  ///
  /// Converts self into an `Option<T>`, consuming self, and discarding the
  /// error, if any.
  constexpr inline Option<T> ok() && noexcept {
    check(state_ != IsMoved);
    switch (replace(mref(state_), IsMoved)) {
      case IsOk:
        return Option<T>::some(take_and_destruct(unsafe_fn, mref(ok_)));
      case IsErr: err_.~E(); return Option<T>::none();
      case IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the function.
    unreachable_unchecked(unsafe_fn);
  }

  /// Converts from `Result<T, E>` to `Option<E>`.
  ///
  /// Converts self into an `Option<E>`, consuming self, and discarding the
  /// success value, if any.
  constexpr inline Option<E> err() && noexcept {
    check(state_ != IsMoved);
    switch (replace(mref(state_), IsMoved)) {
      case IsOk: ok_.~T(); return Option<E>::none();
      case IsErr:
        return Option<E>::some(take_and_destruct(unsafe_fn, mref(err_)));
      case IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the function.
    unreachable_unchecked(unsafe_fn);
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
    check_with_message(replace(mref(state_), IsMoved) == IsOk,
                       *"called `Result::unwrap()` on an `Err` value");
    return take_and_destruct(unsafe_fn, mref(ok_));
  }

  /// Returns the contained `Ok` value, consuming the self value, without
  /// checking that the value is not an `Err`.
  ///
  /// # Safety
  /// Calling this method on an `Err` is Undefined Behavior.
  constexpr inline T unwrap_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    return take_and_destruct(unsafe_fn, mref(ok_));
  }

  /// Returns the contained `Err` value, consuming the self value.
  ///
  /// # Panics
  /// Panics if the value is an `Ok`.
  constexpr inline E unwrap_err() && noexcept {
    check_with_message(replace(mref(state_), IsMoved) == IsErr,
                       *"called `Result::unwrap_err()` on an `Ok` value");
    return take_and_destruct(unsafe_fn, mref(err_));
  }

  /// Returns the contained `Err` value, consuming the self value, without
  /// checking that the value is not an `Ok`.
  ///
  /// # Safety
  /// Calling this method on an `Ok` is Undefined Behavior.
  constexpr inline E unwrap_err_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    return take_and_destruct(unsafe_fn, mref(err_));
  }

  constexpr Iterator<Once<const T&>> iter() const& noexcept {
    check(state_ != IsMoved);
    if (state_ == IsOk)
      return sus::iter::once(Option<const T&>::some(ok_));
    else
      return sus::iter::once(Option<const T&>::none());
  }
  Iterator<Once<const T&>> iter() const&& = delete;

  constexpr Iterator<Once<T&>> iter_mut() & noexcept {
    check(state_ != IsMoved);
    if (state_ == IsOk)
      return sus::iter::once(Option<T&>::some(mref(ok_)));
    else
      return sus::iter::once(Option<T&>::none());
  }

  constexpr Iterator<Once<T>> into_iter() && noexcept {
    check(state_ != IsMoved);
    if (replace(mref(state_), IsMoved) == IsOk) {
      return sus::iter::once(
          Option<T>::some(take_and_destruct(unsafe_fn, mref(ok_))));
    } else {
      err_.~E();
      return sus::iter::once(Option<T>::none());
    }
  }

 private:
  enum WithOkType { WithOk };
  constexpr inline Result(WithOkType, const T& t) noexcept
      : state_(IsOk), ok_(t) {}
  constexpr inline Result(WithOkType, T&& t) noexcept
      : state_(IsOk), ok_(static_cast<T&&>(t)) {}
  enum WithErrType { WithErr };
  constexpr inline Result(WithErrType, const E& e) noexcept
      : state_(IsErr), err_(e) {}
  constexpr inline Result(WithErrType, E&& e) noexcept
      : state_(IsErr), err_(static_cast<E&&>(e)) {}

  enum FullState { IsErr = 0, IsOk = 1, IsMoved = 2 } state_;
  union {
    T ok_;
    E err_;
  };

  sus_class_maybe_trivial_relocatable_types(unsafe_fn, T, E);
};

}  // namespace sus::result
