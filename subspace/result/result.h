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

#include "fmt/format.h"
#include "subspace/assertions/check.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/fn/fn_ref.h"
#include "subspace/iter/into_iterator.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/once.h"
#include "subspace/macros/lifetimebound.h"
#include "subspace/macros/no_unique_address.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/__private/ref_concepts.h"
#include "subspace/mem/clone.h"
#include "subspace/mem/copy.h"
#include "subspace/mem/move.h"
#include "subspace/mem/mref.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/replace.h"
#include "subspace/mem/take.h"
#include "subspace/ops/__private/void_concepts.h"
#include "subspace/option/option.h"
#include "subspace/result/__private/marker.h"
#include "subspace/result/__private/result_state.h"
#include "subspace/result/__private/storage.h"
#include "subspace/string/__private/format_to_stream.h"

namespace sus::result {

using ::sus::iter::Once;
using ::sus::mem::__private::IsTrivialCopyAssignOrRef;
using ::sus::mem::__private::IsTrivialCopyCtorOrRef;
using ::sus::mem::__private::IsTrivialDtorOrRef;
using ::sus::mem::__private::IsTrivialMoveAssignOrRef;
using ::sus::mem::__private::IsTrivialMoveCtorOrRef;
using ::sus::ops::__private::VoidOrEq;
using ::sus::ops::__private::VoidOrOrd;
using ::sus::ops::__private::VoidOrPartialOrd;
using ::sus::ops::__private::VoidOrWeakOrd;
using ::sus::option::__private::StoragePointer;
using ::sus::result::__private::ResultState;
using ::sus::result::__private::Storage;

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
  static_assert(!std::is_reference_v<E>,
                "A reference Error type is not supported.");
  static_assert(
      !std::is_rvalue_reference_v<T>,
      "`Option<T&&> is not supported, use Option<T&> or Option<const T&.");
  static_assert(!std::is_void_v<E>,
                "A void Error type is not supported. Use Option<T> instead.");

  struct VoidType;
  // A helper type used for `T&` references since `T` can be void which then
  // makes `T&` ill-formed. So using this allows code to compile for functions
  // that are excluded when `T` is void without templating them. Why can't
  // `void` be a type.
  using TUnlessVoid = std::conditional_t<std::is_void_v<T>, VoidType, T>;

 public:
  using OkType = T;
  using ErrType = E;

  /// Construct an Result that is holding the given success value.
  static constexpr inline Result with() noexcept
    requires(std::is_void_v<T>)
  {
    return Result(WithOk);
  }
  static constexpr inline Result with(const TUnlessVoid& t) noexcept
    requires(!std::is_void_v<T> && !std::is_reference_v<T> &&
             ::sus::mem::Copy<T>)
  {
    return Result(WithOk, t);
  }
  static constexpr inline Result with(TUnlessVoid&& t) noexcept
    requires(!std::is_void_v<T> && !std::is_reference_v<T>)
  {
    if constexpr (::sus::mem::Move<T>) {
      return Result(WithOk, move_ok_to_storage(t));
    } else {
      static_assert(::sus::mem::Copy<T>, "All types should be Copy or Move.");
      return Result(WithOk, t);
    }
  }
  static constexpr inline Result with(TUnlessVoid t sus_lifetimebound) noexcept
    requires(!std::is_void_v<T> && std::is_reference_v<T>)
  {
    return Result(WithOk, move_ok_to_storage(t));
  }

  /// Construct an Result that is holding the given error value.
  static constexpr inline Result with_err(const E& e) noexcept
    requires(::sus::mem::Copy<E>)
  {
    return Result(WithErr, e);
  }
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
  template <class IntoIter, int&...,
            class Iter =
                std::decay_t<decltype(std::declval<IntoIter&&>().into_iter())>,
            class R = typename Iter::Item,
            class U = __private::IsResultType<R>::ok_type,
            class F = __private::IsResultType<R>::err_type>
    requires(__private::IsResultType<R>::value && std::same_as<E, F> &&
             ::sus::iter::IntoIterator<IntoIter, Result<U, E>>)
  static constexpr Result from_iter(IntoIter&& result_iter) noexcept
    requires(!std::is_void_v<T> && !std::is_reference_v<T> &&
             ::sus::iter::FromIterator<T, U>)
  {
    struct Unwrapper final : public ::sus::iter::IteratorBase<Unwrapper, U> {
      Unwrapper(Iter&& iter, Option<E>& err) : iter(iter), err(err) {}

      // sus::iter::Iterator trait.
      Option<U> next() noexcept {
        Option<Result<U, E>> try_item = iter.next();
        if (try_item.is_none()) return Option<U>();
        Result<U, E> result =
            ::sus::move(try_item).unwrap_unchecked(::sus::marker::unsafe_fn);
        if (result.is_ok())
          return Option<U>::with(
              ::sus::move(result).unwrap_unchecked(::sus::marker::unsafe_fn));
        err.insert(
            ::sus::move(result).unwrap_err_unchecked(::sus::marker::unsafe_fn));
        return Option<U>();
      }
      ::sus::iter::SizeHint size_hint() const noexcept {
        return ::sus::iter::SizeHint(0u, iter.size_hint().upper);
      }

      Iter& iter;
      Option<E>& err;
    };

    auto err = Option<E>();
    auto iter = Unwrapper(::sus::move(result_iter).into_iter(), mref(err));
    auto success_out = Result::with(T::from_iter(::sus::move(iter)));
    return ::sus::move(err).map_or_else(
        [&]() { return ::sus::move(success_out); },
        [](E e) { return Result::with_err(e); });
  }

  /// Destructor for the Result.
  ///
  /// Destroys the Ok or Err value contained within the Result.
  ///
  /// If T and E can be trivially destroyed, we don't need to explicitly destroy
  /// them, so we can use the default destructor, which allows Result<T, E> to
  /// also be trivially destroyed.
  constexpr ~Result()
    requires((std::is_void_v<T> || IsTrivialDtorOrRef<T>) &&
             IsTrivialDtorOrRef<E>)
  = default;

  constexpr inline ~Result() noexcept
    requires(
        // clang-format off
      !((std::is_void_v<T> || IsTrivialDtorOrRef<T>) &&
        IsTrivialDtorOrRef<E>)
        // clang-format on
    )
  {
    switch (state_) {
      case ResultState::IsMoved: break;
      case ResultState::IsOk: storage_.destroy_ok(); break;
      case ResultState::IsErr: storage_.destroy_err(); break;
    }
  }

  /// If T and E can be trivially copy-constructed, Result<T, E> can also be
  /// trivially copy-constructed.
  ///
  /// #[doc.overloads=copy]
  constexpr Result(const Result&)
    requires((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
             ::sus::mem::Copy<E> &&
             (std::is_void_v<T> || IsTrivialCopyCtorOrRef<T>) &&
             IsTrivialCopyCtorOrRef<E>)
  = default;

  Result(const Result& rhs) noexcept
    requires(
        // clang-format off
        (std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
        ::sus::mem::Copy<E> &&
        !((std::is_void_v<T> || IsTrivialCopyCtorOrRef<T>) &&
          IsTrivialCopyCtorOrRef<E>)
        // clang-format on
        )
      : state_(rhs.state_) {
    ::sus::check(state_ != ResultState::IsMoved);
    switch (state_) {
      case ResultState::IsOk:
        if constexpr (!std::is_void_v<T>)
          new (&storage_.ok_) OkStorageType(rhs.storage_.ok_);
        break;
      case ResultState::IsErr: new (&storage_.err_) E(rhs.storage_.err_); break;
      case ResultState::IsMoved:
        // SAFETY: The state_ is verified to be Ok or Err at the top of the
        // function.
        ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
    }
  }

  constexpr Result(const Result&)
    requires(!((std::is_void_v<T> ||
                ::sus::mem::CopyOrRef<T>)&&::sus::mem::Copy<E>))
  = delete;

  /// If T and E can be trivially copy-assigned, Result<T, E> can also be
  /// trivially copy-assigned.
  ///
  /// #[doc.overloads=copy]
  constexpr Result& operator=(const Result& o)
    requires((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
             ::sus::mem::Copy<E> &&
             (std::is_void_v<T> || IsTrivialCopyAssignOrRef<T>) &&
             IsTrivialCopyAssignOrRef<E>)
  = default;

  Result& operator=(const Result& o) noexcept
    requires(
        // clang-format off
      (std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) && ::sus::mem::Copy<E> &&
       !((std::is_void_v<T> || IsTrivialCopyAssignOrRef<T>) &&
         IsTrivialCopyAssignOrRef<E>)
        // clang-format on
    )
  {
    check(o.state_ != ResultState::IsMoved);
    switch (state_) {
      case ResultState::IsOk:
        switch (state_ = o.state_) {
          case ResultState::IsOk:
            if constexpr (!std::is_void_v<T>) {
              storage_.ok_ = o.storage_.ok_;
            }
            break;
          case ResultState::IsErr:
            storage_.destroy_ok();
            new (&storage_.err_) E(o.storage_.err_);
            break;
          // SAFETY: This condition is check()'d at the top of the function.
          case ResultState::IsMoved:
            ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
        break;
      case ResultState::IsErr:
        switch (state_ = o.state_) {
          case ResultState::IsErr: storage_.err_ = o.storage_.err_; break;
          case ResultState::IsOk:
            storage_.destroy_err();
            if constexpr (!std::is_void_v<T>)
              new (&storage_.ok_) OkStorageType(o.storage_.ok_);
            break;
          // SAFETY: This condition is check()'d at the top of the function.
          case ResultState::IsMoved:
            ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
        break;
      case ResultState::IsMoved:
        switch (state_ = o.state_) {
          case ResultState::IsErr:
            new (&storage_.err_) E(o.storage_.err_);
            break;
          case ResultState::IsOk:
            if constexpr (!std::is_void_v<T>)
              new (&storage_.ok_) OkStorageType(o.storage_.ok_);
            break;
            // SAFETY: This condition is check()'d at the top of the function.
          case ResultState::IsMoved:
            ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
        break;
    }
    return *this;
  }

  constexpr Result& operator=(const Result&)
    requires(!((std::is_void_v<T> ||
                ::sus::mem::CopyOrRef<T>)&&::sus::mem::Copy<E>))
  = delete;

  /// If T and E can be trivially move-constructed, Result<T, E> can also be
  /// trivially move-constructed.
  ///
  /// #[doc.overloads=move]
  constexpr Result(Result&&)
    requires((std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) &&
             ::sus::mem::Move<E> &&
             (std::is_void_v<T> || IsTrivialMoveCtorOrRef<T>) &&
             IsTrivialMoveCtorOrRef<E>)
  = default;

  Result(Result&& rhs) noexcept
    requires(
        // clang-format off
      (std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) && ::sus::mem::Move<E> &&
      !((std::is_void_v<T> || IsTrivialMoveCtorOrRef<T>) &&
        IsTrivialMoveCtorOrRef<E>)
        // clang-format on
        )
      : state_(::sus::mem::replace(mref(rhs.state_), ResultState::IsMoved)) {
    ::sus::check(state_ != ResultState::IsMoved);
    switch (state_) {
      case ResultState::IsOk:
        new (&storage_.ok_) OkStorageType(::sus::move(rhs.storage_.ok_));
        break;
      case ResultState::IsErr:
        new (&storage_.err_) E(::sus::move(rhs.storage_.err_));
        break;
      case ResultState::IsMoved:
        // SAFETY: The state_ is verified to be Ok or Err at the top of the
        // function.
        ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
    }
  }

  constexpr Result(Result&&)
    requires(!((std::is_void_v<T> ||
                ::sus::mem::MoveOrRef<T>)&&::sus::mem::Move<E>))
  = delete;

  /// If T and E can be trivially move-assigned, Result<T, E> can also be
  /// trivially move-assigned.
  ///
  /// #[doc.overloads=move]
  constexpr Result& operator=(Result&& o)
    requires((std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) &&
             ::sus::mem::Move<E> &&
             (std::is_void_v<T> || IsTrivialMoveAssignOrRef<T>) &&
             IsTrivialMoveAssignOrRef<E>)
  = default;

  Result& operator=(Result&& o) noexcept
    requires(
        // clang-format off
      (std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) && ::sus::mem::Move<E> &&
      !((std::is_void_v<T> || IsTrivialMoveAssignOrRef<T>) &&
        IsTrivialMoveAssignOrRef<E>)
        // clang-format on
    )
  {
    check(o.state_ != ResultState::IsMoved);
    switch (state_) {
      case ResultState::IsOk:
        switch (state_ =
                    ::sus::mem::replace(mref(o.state_), ResultState::IsMoved)) {
          case ResultState::IsOk:
            storage_.ok_ = ::sus::move(o.storage_.ok_);
            break;
          case ResultState::IsErr:
            storage_.destroy_ok();
            new (&storage_.err_) E(::sus::move(o.storage_.err_));
            break;
          // SAFETY: This condition is check()'d at the top of the function.
          case ResultState::IsMoved:
            ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
        break;
      case ResultState::IsErr:
        switch (state_ =
                    ::sus::mem::replace(mref(o.state_), ResultState::IsMoved)) {
          case ResultState::IsErr:
            storage_.err_ = ::sus::move(o.storage_.err_);
            break;
          case ResultState::IsOk:
            storage_.destroy_err();
            new (&storage_.ok_) OkStorageType(::sus::move(o.storage_.ok_));
            break;
          // SAFETY: This condition is check()'d at the top of the function.
          case ResultState::IsMoved:
            ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
        break;
      case ResultState::IsMoved:
        switch (state_ =
                    ::sus::mem::replace(mref(o.state_), ResultState::IsMoved)) {
          case ResultState::IsErr:
            new (&storage_.err_) E(::sus::move(o.storage_.err_));
            break;
          case ResultState::IsOk:
            new (&storage_.ok_) OkStorageType(::sus::move(o.storage_.ok_));
            break;
            // SAFETY: This condition is check()'d at the top of the function.
          case ResultState::IsMoved:
            ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
        break;
    }
    return *this;
  }

  constexpr Result& operator=(Result&& o)
    requires(!(std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) ||
             !::sus::mem::Move<E>)
  = delete;

  constexpr Result clone() const& noexcept
    requires((std::is_void_v<T> || ::sus::mem::Clone<T>) &&
             ::sus::mem::Clone<E> &&
             !((std::is_void_v<T> ||
                ::sus::mem::CopyOrRef<T>)&&::sus::mem::Copy<E>))
  {
    ::sus::check(state_ != ResultState::IsMoved);
    switch (state_) {
      case ResultState::IsOk:
        if constexpr (std::is_void_v<T>)
          return Result(WithOk);
        else
          return Result(WithOk, ::sus::clone(storage_.ok_));
      case ResultState::IsErr:
        return Result(WithErr, ::sus::clone(storage_.err_));
      case ResultState::IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the
    // function.
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  void clone_from(const Result& source) &
        requires((std::is_void_v<T> || ::sus::mem::Clone<T>) &&
                 ::sus::mem::Clone<E> &&
                 !((std::is_void_v<T> ||
                    ::sus::mem::CopyOrRef<T>)&&::sus::mem::Copy<E>))
  {
    ::sus::check(source.state_ != ResultState::IsMoved);
    if (&source == this) [[unlikely]]
      return;
    if (state_ == source.state_) {
      switch (state_) {
        case ResultState::IsOk:
          if constexpr (!std::is_void_v<T>)
            ::sus::clone_into(mref(storage_.ok_), source.storage_.ok_);
          break;
        case ResultState::IsErr:
          ::sus::clone_into(mref(storage_.err_), source.storage_.err_);
          break;
        case ResultState::IsMoved:
          ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
      }
    } else {
      *this = source.clone();
    }
  }

  /// Returns true if the result is `Ok`.
  constexpr inline bool is_ok() const& noexcept {
    ::sus::check(state_ != ResultState::IsMoved);
    return state_ == ResultState::IsOk;
  }

  /// Returns true if the result is `Err`.
  constexpr inline bool is_err() const& noexcept {
    ::sus::check(state_ != ResultState::IsMoved);
    return state_ == ResultState::IsErr;
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
    ::sus::check(state_ != ResultState::IsMoved);
    return static_cast<State>(state_);
  }

  /// Converts from `Result<T, E>` to `Option<T>`.
  ///
  /// Converts self into an `Option<T>`, consuming self, and discarding the
  /// error, if any.
  constexpr inline Option<T> ok() && noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    switch (::sus::mem::replace(mref(state_), ResultState::IsMoved)) {
      case ResultState::IsOk:
        return Option<T>::with(::sus::mem::take_and_destruct(
            ::sus::marker::unsafe_fn, mref(storage_.ok_)));
      case ResultState::IsErr: storage_.destroy_err(); return Option<T>();
      case ResultState::IsMoved: break;
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
    ::sus::check(state_ != ResultState::IsMoved);
    switch (::sus::mem::replace(mref(state_), ResultState::IsMoved)) {
      case ResultState::IsOk: storage_.destroy_ok(); return Option<E>();
      case ResultState::IsErr:
        return Option<E>::with(::sus::mem::take_and_destruct(
            ::sus::marker::unsafe_fn, mref(storage_.err_)));
      case ResultState::IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the
    // function.
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  /// Returns a const reference to the contained `Ok` value.
  ///
  /// # Panic
  /// Panics if the value is an `Err`.
  const std::remove_reference_t<TUnlessVoid>& as_ok() const&
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ == ResultState::IsOk);
    return storage_.ok_;
  }

  /// Returns a const reference to the contained `Err` value.
  ///
  /// # Panic
  /// Panics if the value is an `Ok`.
  const std::remove_reference_t<E>& as_err() const& {
    ::sus::check(state_ == ResultState::IsErr);
    return storage_.err_;
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
    check_with_message(
        ::sus::mem::replace(mref(state_), ResultState::IsMoved) ==
            ResultState::IsOk,
        *"called `Result::unwrap()` on an `Err` value");
    if constexpr (!std::is_void_v<T>) {
      return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                           mref(storage_.ok_));
    }
  }

  /// Returns the contained `Ok` value, consuming the self value, without
  /// checking that the value is not an `Err`.
  ///
  /// # Safety
  /// Calling this method on an `Err` is Undefined Behavior.
  constexpr inline T unwrap_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    if constexpr (!std::is_void_v<T>) {
      return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                           mref(storage_.ok_));
    }
  }

  /// Returns the contained `Err` value, consuming the self value.
  ///
  /// # Panics
  /// Panics if the value is an `Ok`.
  constexpr inline E unwrap_err() && noexcept {
    check_with_message(
        ::sus::mem::replace(mref(state_), ResultState::IsMoved) ==
            ResultState::IsErr,
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

  /// Returns the contained `Ok` value or computes it from a closure.
  ///
  /// # Examples
  /// Basic usage:
  /// ```
  /// enum ECode { ItsHappening = -1 };
  /// auto conv = [](ECode e) { return static_cast<i32>(e); };
  /// auto ok = sus::Result<i32, ECode>::with(2);
  /// sus::check(sus::move(ok).unwrap_or_else(conv) == 2);
  /// auto err = sus::Result<i32, ECode>::with_err(ItsHappening);
  /// sus::check(sus::move(err).unwrap_or_else(conv) == -1);
  /// ```
  template <::sus::fn::FnOnce<T(E&&)> F>
  constexpr T unwrap_or_else(F&& op) && noexcept {
    if (is_ok()) {
      return sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn);
    } else {
      return ::sus::move(op)(
          sus::move(*this).unwrap_err_unchecked(::sus::marker::unsafe_fn));
    }
  }

  constexpr Once<const std::remove_reference_t<TUnlessVoid>&> iter()
      const& noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (state_ == ResultState::IsOk) {
      return Once<const std::remove_reference_t<T>&>::with(
          Option<const std::remove_reference_t<T>&>::with(storage_.ok_));
    } else {
      return Once<const std::remove_reference_t<T>&>::with(
          Option<const std::remove_reference_t<T>&>());
    }
  }
  constexpr Once<const std::remove_reference_t<TUnlessVoid>&> iter() && noexcept
    requires(!std::is_void_v<T> && std::is_rvalue_reference_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (::sus::mem::replace(state_, ResultState::IsMoved) ==
        ResultState::IsOk) {
      return Once<const std::remove_reference_t<T>&>::with(
          Option<const std::remove_reference_t<T>&>::with(storage_.ok_));
    } else {
      storage_.destroy_err();
      return Once<const std::remove_reference_t<T>&>::with(
          Option<const std::remove_reference_t<T>&>());
    }
  }

  constexpr Once<TUnlessVoid&> iter_mut() & noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (state_ == ResultState::IsOk) {
      return Once<T&>::with(Option<T&>::with(mref(storage_.ok_)));
    } else {
      return Once<T&>::with(Option<T&>());
    }
  }
  constexpr Once<TUnlessVoid&> iter_mut() && noexcept
    requires(!std::is_void_v<T> && std::is_reference_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (::sus::mem::replace(state_, ResultState::IsMoved) ==
        ResultState::IsOk) {
      return Once<T&>::with(Option<T&>::with(mref(storage_.ok_)));
    } else {
      storage_.destroy_err();
      return Once<T&>::with(Option<T&>());
    }
  }

  constexpr Once<T> into_iter() && noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (::sus::mem::replace(mref(state_), ResultState::IsMoved) ==
        ResultState::IsOk) {
      return Once<T>::with(Option<T>::with(::sus::mem::take_and_destruct(
          ::sus::marker::unsafe_fn, mref(storage_.ok_))));
    } else {
      storage_.destroy_err();
      return Once<T>::with(Option<T>());
    }
  }

  /// sus::ops::Eq<Result<T, E>> trait.
  template <class U, class F>
    requires(VoidOrEq<T, U> && ::sus::ops::Eq<E, F>)
  friend constexpr bool operator==(const Result& l,
                                   const Result<U, F>& r) noexcept {
    ::sus::check(l.state_ != ResultState::IsMoved);
    switch (l.state_) {
      case ResultState::IsOk:
        if constexpr (!std::is_void_v<T>)
          return r.is_ok() && l.storage_.ok_ == r.storage_.ok_;
        else
          return r.is_ok();
      case ResultState::IsErr:
        return r.is_err() && l.storage_.err_ == r.storage_.err_;
      case ResultState::IsMoved: break;
    }
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  template <class U, class F>
    requires(!(VoidOrEq<T, U> && ::sus::ops::Eq<E, F>))
  friend constexpr bool operator==(const Result& l,
                                   const Result<U, F>& r) = delete;

  /// Compares two Result.
  ///
  /// Satisfies sus::ops::Ord<Result<T, E>> if sus::ops::Ord<T> and
  /// sus::ops::Ord<E>.
  ///
  /// Satisfies sus::ops::WeakOrd<Result<T, E>> if sus::ops::WeakOrd<T> and
  /// sus::ops::WeakOrd<E>.
  ///
  /// Satisfies sus::ops::PartialOrd<Result<T, E>> if sus::ops::PartialOrd<T>
  /// and sus::ops::PartialOrd<E>.
  //
  // sus::ops::Ord<Result<T, E>> trait.
  // sus::ops::WeakOrd<Result<T, E>> trait.
  // sus::ops::PartialOrd<Result<T, E>> trait.
  template <class U, class F>
    requires(VoidOrOrd<T, U> && ::sus::ops::Ord<E, F>)
  friend constexpr auto operator<=>(const Result& l,
                                    const Result<U, F>& r) noexcept {
    ::sus::check(l.state_ != ResultState::IsMoved);
    switch (l.state_) {
      case ResultState::IsOk:
        if (r.is_ok()) {
          if constexpr (!std::is_void_v<T>)
            return std::strong_order(l.storage_.ok_, r.storage_.ok_);
          else
            return std::strong_ordering::equivalent;
        } else {
          return std::strong_ordering::greater;
        }
      case ResultState::IsErr:
        if (r.is_err())
          return std::strong_order(l.storage_.err_, r.storage_.err_);
        else
          return std::strong_ordering::less;
      case ResultState::IsMoved: break;
    }
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  template <class U, class F>
    requires((!VoidOrOrd<T, U> || !::sus::ops::Ord<E, F>) &&
             VoidOrWeakOrd<T, U> && ::sus::ops::WeakOrd<E, F>)
  friend constexpr auto operator<=>(const Result& l,
                                    const Result<U, F>& r) noexcept {
    ::sus::check(l.state_ != ResultState::IsMoved);
    switch (l.state_) {
      case ResultState::IsOk:
        if (r.is_ok()) {
          if constexpr (!std::is_void_v<T>)
            return std::weak_order(l.storage_.ok_, r.storage_.ok_);
          else
            return std::weak_ordering::equivalent;
        } else {
          return std::weak_ordering::greater;
        }
      case ResultState::IsErr:
        if (r.is_err())
          return std::weak_order(l.storage_.err_, r.storage_.err_);
        else
          return std::weak_ordering::less;
      case ResultState::IsMoved: break;
    }
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  template <class U, class F>
    requires((!VoidOrWeakOrd<T, U> || !::sus::ops::WeakOrd<E, F>) &&
             VoidOrPartialOrd<T, U> && ::sus::ops::PartialOrd<E, F>)
  friend constexpr auto operator<=>(const Result& l,
                                    const Result<U, F>& r) noexcept {
    ::sus::check(l.state_ != ResultState::IsMoved);
    switch (l.state_) {
      case ResultState::IsOk:
        if (r.is_ok()) {
          if constexpr (!std::is_void_v<T>)
            return std::partial_order(l.storage_.ok_, r.storage_.ok_);
          else
            return std::partial_ordering::equivalent;
        } else {
          return std::partial_ordering::greater;
        }
      case ResultState::IsErr:
        if (r.is_err())
          return std::partial_order(l.storage_.err_, r.storage_.err_);
        else
          return std::partial_ordering::less;
      case ResultState::IsMoved: break;
    }
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  template <class U, class F>
    requires(!VoidOrPartialOrd<T, U> || !::sus::ops::PartialOrd<E, F>)
  friend constexpr auto operator<=>(const Result& l,
                                    const Result<U, F>& r) noexcept = delete;

 private:
  template <class U, class V>
  friend class Result;

  // Since `T` may be a reference or a value type, this constructs the correct
  // storage from a `T` object or a `T&&` (which is received as `T&`).
  template <class U>
  static constexpr inline decltype(auto) copy_ok_to_storage(const U& t) {
    if constexpr (std::is_reference_v<T>)
      return StoragePointer<T>(t);
    else
      return t;
  }
  // Since `T` may be a reference or a value type, this constructs the correct
  // storage from a `T` object or a `T&&` (which is received as `T&`).
  template <class U>
  static constexpr inline decltype(auto) move_ok_to_storage(U&& t) {
    if constexpr (std::is_reference_v<T>)
      return StoragePointer<T>(t);
    else
      return ::sus::move(t);
  }

  // Constructors for `Ok`.
  enum WithOkType { WithOk };
  constexpr inline Result(WithOkType) noexcept
    requires(std::is_void_v<T>)
      : storage_(), state_(ResultState::IsOk) {}
  template <std::convertible_to<T> U>
  constexpr inline Result(WithOkType, const U& t) noexcept
      : storage_(__private::kWithT, t), state_(ResultState::IsOk) {}
  template <std::convertible_to<T> U>
  constexpr inline Result(WithOkType, U&& t) noexcept
    requires(::sus::mem::Move<T>)
      : storage_(__private::kWithT, ::sus::move(t)),
        state_(ResultState::IsOk) {}
  // Constructors for `Err`.
  enum WithErrType { WithErr };
  constexpr inline Result(WithErrType, const E& e) noexcept
      : storage_(__private::kWithE, e), state_(ResultState::IsErr) {}
  constexpr inline Result(WithErrType, E&& e) noexcept
    requires(::sus::mem::Move<E>)
      : storage_(__private::kWithE, ::sus::move(e)),
        state_(ResultState::IsErr) {}

  using OkStorageType =
      std::conditional_t<std::is_reference_v<T>, StoragePointer<T>,
                         std::conditional_t<std::is_void_v<T>, char, T>>;

  [[sus_no_unique_address]] Storage<OkStorageType, E> storage_;
  ResultState state_;

  sus_class_trivially_relocatable_if_types(
      ::sus::marker::unsafe_fn,
      std::remove_const_t<std::remove_reference_t<decltype(storage_.ok_)>>,
      std::remove_const_t<std::remove_reference_t<decltype(storage_.err_)>>,
      decltype(state_));
};

// Implicit for-ranged loop iteration via `Result::iter()`.
using sus::iter::__private::begin;
using sus::iter::__private::end;

/// Used to construct a Result<T, E> with an Ok(t) value.
///
/// Calling ok() produces a hint to make a Result<T, E> but does not actually
/// construct Result<T, E>. This is to deduce the actual types `T` and `E`
/// when it is constructed, avoid specifying them both here, and support
/// conversions.
template <class T>
[[nodiscard]] inline constexpr auto ok(T&& t sus_lifetimebound) noexcept {
  return __private::OkMarker<T&&>(::sus::forward<T>(t));
}

/// Used to construct a Result<T, E> with an Err(e) value.
///
/// Calling err() produces a hint to make a Result<T, E> but does not actually
/// construct Result<T, E>. This is to deduce the actual types `T` and `E`
/// when it is constructed, avoid specifying them both here, and support
/// conversions.
template <class E>
[[nodiscard]] inline constexpr auto err(E&& e sus_lifetimebound) noexcept {
  return __private::ErrMarker<E&&>(::sus::forward<E>(e));
}

}  // namespace sus::result

// std hash support.
template <class T, class E>
struct std::hash<::sus::result::Result<T, E>> {
  auto operator()(const ::sus::result::Result<T, E>& u) const noexcept {
    if (u.is_ok())
      return std::hash<T>()(u.as_ok());
    else
      return std::hash<T>()(u.as_err());
  }
};
template <class T, class E>
  requires(::sus::ops::Eq<::sus::result::Result<T, E>>)
struct std::equal_to<::sus::result::Result<T, E>> {
  constexpr auto operator()(
      const ::sus::result::Result<T, E>& l,
      const ::sus::result::Result<T, E>& r) const noexcept {
    return l == r;
  }
};

// fmt support.
template <class T, class E, class Char>
struct fmt::formatter<::sus::result::Result<T, E>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_ok_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::result::Result<T, E>& t,
                        FormatContext& ctx) const {
    auto out = ctx.out();
    if (t.is_err()) {
      out = fmt::format_to(ctx.out(), "Err(");
      ctx.advance_to(out);
      out = underlying_err_.format(t.as_err(), ctx);
    } else {
      out = fmt::format_to(out, "Ok(");
      ctx.advance_to(out);
      if constexpr (std::is_void_v<T>)
        out = underlying_ok_.format(ctx);
      else
        out = underlying_ok_.format(t.as_ok(), ctx);
    }
    return fmt::format_to(out, ")");
  }

 private:
  ::sus::string::__private::AnyOrVoidFormatter<T, Char> underlying_ok_;
  ::sus::string::__private::AnyFormatter<E, Char> underlying_err_;
};

// Stream support.
sus__format_to_stream(sus::result, Result, T, E);

namespace sus {
using ::sus::result::Err;
using ::sus::result::err;
using ::sus::result::Ok;
using ::sus::result::ok;
using ::sus::result::Result;
}  // namespace sus
