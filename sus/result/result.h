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
#include "sus/assertions/check.h"
#include "sus/assertions/unreachable.h"
#include "sus/fn/fn_ref.h"
#include "sus/iter/into_iterator.h"
#include "sus/iter/iterator_defn.h"
#include "sus/iter/once.h"
#include "sus/macros/lifetimebound.h"
#include "sus/macros/no_unique_address.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/__private/ref_concepts.h"
#include "sus/mem/clone.h"
#include "sus/mem/copy.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/mem/take.h"
#include "sus/ops/__private/void_concepts.h"
#include "sus/option/option.h"
#include "sus/result/__private/marker.h"
#include "sus/result/__private/result_state.h"
#include "sus/result/__private/storage.h"
#include "sus/string/__private/format_to_stream.h"

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
  ///
  /// # Const References
  ///
  /// For `Result<const T&, E>` it is possible to bind to a temporary which
  /// would create a memory safety bug. The `[[clang::lifetimebound]]` attribute
  /// is used to prevent this via Clang. But additionally, the incoming type is
  /// required to match with `sus::construct::SafelyConstructibleFromReference`
  /// to prevent conversions that would construct a temporary.
  ///
  /// To force accepting a const reference anyway in cases where a type can
  /// convert to a reference without constructing a temporary, use an unsafe
  /// `static_cast<const T&>()` at the callsite (and document it =)).
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
  template <std::convertible_to<TUnlessVoid> U>
  sus_pure static inline constexpr Result with(U&& t sus_lifetimebound) noexcept
    requires(!std::is_void_v<T> && std::is_reference_v<T> &&
             sus::construct::SafelyConstructibleFromReference<T, U &&>)
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

  constexpr Result(const Result& rhs) noexcept
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
          std::construct_at(&storage_.ok_, rhs.storage_.ok_);
        break;
      case ResultState::IsErr:
        std::construct_at(&storage_.err_, rhs.storage_.err_);
        break;
      case ResultState::IsMoved:
        // SAFETY: The state_ is verified to be Ok or Err at the top of the
        // function.
        ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
    }
  }

  constexpr Result(const Result&)
    requires(!((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
               ::sus::mem::Copy<E>))
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

  constexpr Result& operator=(const Result& o) noexcept
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
            std::construct_at(&storage_.err_, o.storage_.err_);
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
              std::construct_at(&storage_.ok_, o.storage_.ok_);
            break;
          // SAFETY: This condition is check()'d at the top of the function.
          case ResultState::IsMoved:
            ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
        break;
      case ResultState::IsMoved:
        switch (state_ = o.state_) {
          case ResultState::IsErr:
            std::construct_at(&storage_.err_, o.storage_.err_);
            break;
          case ResultState::IsOk:
            if constexpr (!std::is_void_v<T>)
              std::construct_at(&storage_.ok_, o.storage_.ok_);
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
    requires(!((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
               ::sus::mem::Copy<E>))
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

  constexpr Result(Result&& rhs) noexcept
    requires(
        // clang-format off
      (std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) && ::sus::mem::Move<E> &&
      !((std::is_void_v<T> || IsTrivialMoveCtorOrRef<T>) &&
        IsTrivialMoveCtorOrRef<E>)
        // clang-format on
        )
      : state_(::sus::mem::replace(rhs.state_, ResultState::IsMoved)) {
    ::sus::check(state_ != ResultState::IsMoved);
    switch (state_) {
      case ResultState::IsOk:
        std::construct_at(&storage_.ok_, ::sus::move(rhs.storage_.ok_));
        break;
      case ResultState::IsErr:
        std::construct_at(&storage_.err_, ::sus::move(rhs.storage_.err_));
        break;
      case ResultState::IsMoved:
        // SAFETY: The state_ is verified to be Ok or Err at the top of the
        // function.
        ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
    }
  }

  constexpr Result(Result&&)
    requires(!((std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) &&
               ::sus::mem::Move<E>))
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

  constexpr Result& operator=(Result&& o) noexcept
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
        switch (state_ = ::sus::mem::replace(o.state_, ResultState::IsMoved)) {
          case ResultState::IsOk:
            storage_.ok_ = ::sus::move(o.storage_.ok_);
            break;
          case ResultState::IsErr:
            storage_.destroy_ok();
            std::construct_at(&storage_.err_, ::sus::move(o.storage_.err_));
            break;
          // SAFETY: This condition is check()'d at the top of the function.
          case ResultState::IsMoved:
            ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
        break;
      case ResultState::IsErr:
        switch (state_ = ::sus::mem::replace(o.state_, ResultState::IsMoved)) {
          case ResultState::IsErr:
            storage_.err_ = ::sus::move(o.storage_.err_);
            break;
          case ResultState::IsOk:
            storage_.destroy_err();
            std::construct_at(&storage_.ok_, ::sus::move(o.storage_.ok_));
            break;
          // SAFETY: This condition is check()'d at the top of the function.
          case ResultState::IsMoved:
            ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
        break;
      case ResultState::IsMoved:
        switch (state_ = ::sus::mem::replace(o.state_, ResultState::IsMoved)) {
          case ResultState::IsErr:
            std::construct_at(&storage_.err_, ::sus::move(o.storage_.err_));
            break;
          case ResultState::IsOk:
            std::construct_at(&storage_.ok_, ::sus::move(o.storage_.ok_));
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
             !((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
               ::sus::mem::Copy<E>))
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

  constexpr void clone_from(const Result& source) &
    requires((std::is_void_v<T> || ::sus::mem::Clone<T>) &&
             ::sus::mem::Clone<E> &&
             !((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
               ::sus::mem::Copy<E>))
  {
    ::sus::check(source.state_ != ResultState::IsMoved);
    if (&source == this) [[unlikely]]
      return;
    if (state_ == source.state_) {
      switch (state_) {
        case ResultState::IsOk:
          if constexpr (!std::is_void_v<T>)
            ::sus::clone_into(storage_.ok_, source.storage_.ok_);
          break;
        case ResultState::IsErr:
          ::sus::clone_into(storage_.err_, source.storage_.err_);
          break;
        case ResultState::IsMoved:
          ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
      }
    } else {
      *this = source.clone();
    }
  }

  /// Computes the product of an iterator over `Result<T, E>` as long as there
  /// is no `Err` found. If an `Err` is found, the function returns the first
  /// `Err`.
  ///
  /// Implements sus::iter::Product<Result<T, E>, Result<T, E>>.
  ///
  /// The product is computed using the implementation of the inner type `T`
  /// which also satisfies `sus::iter::Product<T, T>`.
  template <::sus::iter::Iterator<Result<T, E>> Iter>
    requires ::sus::iter::Product<T>
  static constexpr Result from_product(Iter&& it) noexcept {
    class IterUntilNone final
        : public ::sus::iter::IteratorBase<IterUntilNone, T> {
     public:
      IterUntilNone(Iter& iter, Option<E>& err) : iter_(iter), err_(err) {}

      Option<T> next() noexcept {
        Option<Result<T, E>> next = iter_.next();
        Option<T> out;
        if (next.is_some()) {
          Result<T, E> r =
              ::sus::move(next).unwrap_unchecked(::sus::marker::unsafe_fn);
          if (r.is_err()) {
            err_ = ::sus::move(r).err();
          } else {
            out = ::sus::move(r).ok();
          }
        }
        return out;
      }
      ::sus::iter::SizeHint size_hint() const noexcept {
        return ::sus::iter::SizeHint(0u, iter_.size_hint().upper);
      }

     private:
      Iter& iter_;
      Option<E>& err_;
    };
    static_assert(::sus::iter::Iterator<IterUntilNone, T>);

    Option<E> err;
    auto out = Result::with(IterUntilNone(it, err).product());
    if (err.is_some()) {
      out = Result::with_err(
          ::sus::move(err).unwrap_unchecked(::sus::marker::unsafe_fn));
    }
    return out;
  }

  /// Computes the sum of an iterator over `Result<T, E>` as long as there
  /// is no `Err` found. If an `Err` is found, the function returns the first
  /// `Err`.
  ///
  /// Implements sus::iter::Sum<Result<T, E>, Result<T, E>>.
  ///
  /// The sum is computed using the implementation of the inner type `T`
  /// which also satisfies `sus::iter::Sum<T, T>`.
  template <::sus::iter::Iterator<Result<T, E>> Iter>
    requires ::sus::iter::Sum<T>
  static constexpr Result from_sum(Iter&& it) noexcept {
    class IterUntilNone final
        : public ::sus::iter::IteratorBase<IterUntilNone, T> {
     public:
      IterUntilNone(Iter& iter, Option<E>& err) : iter_(iter), err_(err) {}

      Option<T> next() noexcept {
        Option<Result<T, E>> next = iter_.next();
        Option<T> out;
        if (next.is_some()) {
          Result<T, E> r =
              ::sus::move(next).unwrap_unchecked(::sus::marker::unsafe_fn);
          if (r.is_err()) {
            err_ = ::sus::move(r).err();
          } else {
            out = ::sus::move(r).ok();
          }
        }
        return out;
      }
      ::sus::iter::SizeHint size_hint() const noexcept {
        return ::sus::iter::SizeHint(0u, iter_.size_hint().upper);
      }

     private:
      Iter& iter_;
      Option<E>& err_;
    };
    static_assert(::sus::iter::Iterator<IterUntilNone, T>);

    Option<E> err;
    auto out = Result::with(IterUntilNone(it, err).sum());
    if (err.is_some()) {
      out = Result::with_err(
          ::sus::move(err).unwrap_unchecked(::sus::marker::unsafe_fn));
    }
    return out;
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
    switch (::sus::mem::replace(state_, ResultState::IsMoved)) {
      case ResultState::IsOk:
        // SAFETY: The static_cast is needed to convert the pointer storage type
        // to a `const T&`, which does not create a temporary as it's converting
        // a pointer to a reference.
        return Option<T>::with(static_cast<T>(::sus::mem::take_and_destruct(
            ::sus::marker::unsafe_fn, storage_.ok_)));
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
    switch (::sus::mem::replace(state_, ResultState::IsMoved)) {
      case ResultState::IsOk: storage_.destroy_ok(); return Option<E>();
      case ResultState::IsErr:
        return Option<E>::with(::sus::mem::take_and_destruct(
            ::sus::marker::unsafe_fn, storage_.err_));
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
  constexpr const std::remove_reference_t<TUnlessVoid>& as_ok() const&
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ == ResultState::IsOk);
    return storage_.ok_;
  }

  /// Returns a const reference to the contained `Err` value.
  ///
  /// # Panic
  /// Panics if the value is an `Ok` or the Result is moved from.
  constexpr const std::remove_reference_t<E>& as_err() const& {
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
  /// Panics if the value is an `Err` or the Result is moved from.
  constexpr inline T unwrap() && noexcept {
    ResultState was = ::sus::mem::replace(state_, ResultState::IsMoved);
    check_with_message(
        was != ResultState::IsMoved,
        *"called `Result::unwrap_or_default()` on a moved Result");
    check_with_message(was == ResultState::IsOk,
                       *"called `Result::unwrap()` on an `Err` value");
    return ::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn);
  }

  /// Returns the contained Ok value or a default.
  ///
  /// Consumes the Result and, if it held an Ok value, the value is returned.
  /// Otherwise the default value of the Ok value's type is returned.
  constexpr T unwrap_or_default() && noexcept
    requires(!std::is_reference_v<T> &&
             (std::is_void_v<T> || ::sus::construct::Default<T>))
  {
    ResultState was = ::sus::mem::replace(state_, ResultState::IsMoved);
    check_with_message(
        was != ResultState::IsMoved,
        *"called `Result::unwrap_or_default()` on a moved Result");
    if constexpr (!std::is_void_v<T>) {
      if (was == ResultState::IsOk) {
        return ::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn);
      } else {
        return T();
      }
    }
  }

  /// Returns the contained `Ok` value, consuming the self value, without
  /// checking that the value is not an `Err`.
  ///
  /// # Safety
  /// Calling this method on an `Err` or a moved-from Result is Undefined
  /// Behavior.
  constexpr inline T unwrap_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    if constexpr (!std::is_void_v<T>) {
      return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                           storage_.ok_);
    }
  }

  /// Returns the contained `Err` value, consuming the self value.
  ///
  /// # Panics
  /// Panics if the value is an `Ok` or the Result is moved from.
  constexpr inline E unwrap_err() && noexcept {
    check_with_message(
        ::sus::mem::replace(state_, ResultState::IsMoved) == ResultState::IsErr,
        *"called `Result::unwrap_err()` on an `Ok` value");
    return ::sus::move(*this).unwrap_err_unchecked(::sus::marker::unsafe_fn);
  }

  /// Returns the contained `Err` value, consuming the self value, without
  /// checking that the value is not an `Ok`.
  ///
  /// # Safety
  /// Calling this method on an `Ok` or a moved-from Result is Undefined
  /// Behavior.
  constexpr inline E unwrap_err_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                         storage_.err_);
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
      return ::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn);
    } else {
      return ::sus::fn::call_once(
          ::sus::move(op),
          ::sus::move(*this).unwrap_err_unchecked(::sus::marker::unsafe_fn));
    }
  }

  constexpr Once<const std::remove_reference_t<TUnlessVoid>&> iter()
      const& noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (state_ == ResultState::IsOk) {
      return ::sus::iter::once<const std::remove_reference_t<T>&>(
          // SAFETY: The static_cast is needed to convert the pointer storage
          // type to a `const T&`, which does not create a temporary as it's
          // converting a pointer to a reference.
          Option<const std::remove_reference_t<T>&>::with(
              static_cast<const std::remove_reference_t<T>&>(storage_.ok_)));
    } else {
      return ::sus::iter::once<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>());
    }
  }
  constexpr Once<const std::remove_reference_t<TUnlessVoid>&> iter() && noexcept
    requires(!std::is_void_v<T> && std::is_reference_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (::sus::mem::replace(state_, ResultState::IsMoved) ==
        ResultState::IsOk) {
      return ::sus::iter::once<const std::remove_reference_t<T>&>(
          // SAFETY: The static_cast is needed to convert the pointer storage
          // type to a `const T&`, which does not create a temporary as it's
          // converting a pointer to a reference.
          Option<const std::remove_reference_t<T>&>::with(
              static_cast<const std::remove_reference_t<T>&>(storage_.ok_)));
    } else {
      storage_.destroy_err();
      return ::sus::iter::once<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>());
    }
  }

  constexpr Once<TUnlessVoid&> iter_mut() & noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (state_ == ResultState::IsOk) {
      return ::sus::iter::once<T&>(Option<T&>::with(storage_.ok_));
    } else {
      return ::sus::iter::once<T&>(Option<T&>());
    }
  }
  constexpr Once<TUnlessVoid&> iter_mut() && noexcept
    requires(!std::is_void_v<T> && std::is_reference_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (::sus::mem::replace(state_, ResultState::IsMoved) ==
        ResultState::IsOk) {
      return ::sus::iter::once<T&>(Option<T&>::with(storage_.ok_));
    } else {
      storage_.destroy_err();
      return ::sus::iter::once<T&>(Option<T&>());
    }
  }

  constexpr Once<T> into_iter() && noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (::sus::mem::replace(state_, ResultState::IsMoved) ==
        ResultState::IsOk) {
      return ::sus::iter::once<T>(Option<T>::with(::sus::mem::take_and_destruct(
          ::sus::marker::unsafe_fn, storage_.ok_)));
    } else {
      storage_.destroy_err();
      return ::sus::iter::once<T>(Option<T>());
    }
  }

  /// sus::ops::Eq<Result<T, E>> trait.
  ///
  /// The non-template overload allows ok/err marker types to convert to
  /// Option for comparison.
  friend constexpr bool operator==(const Result& l, const Result& r) noexcept
    requires(VoidOrEq<T> && ::sus::ops::Eq<E>)
  {
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
  ///
  /// The non-template overloads allow ok/err marker types to convert to
  /// Option for comparison.
  //
  // sus::ops::Ord<Result<T, E>> trait.
  friend constexpr auto operator<=>(const Result& l, const Result& r) noexcept
    requires(VoidOrOrd<T> && ::sus::ops::Ord<E>)
  {
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

  // sus::ops::WeakOrd<Result<T, E>> trait.
  friend constexpr auto operator<=>(const Result& l, const Result& r) noexcept
    requires((!VoidOrOrd<T> || !::sus::ops::Ord<E>) && VoidOrWeakOrd<T> &&
             ::sus::ops::WeakOrd<E>)
  {
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

  // sus::ops::PartialOrd<Result<T, E>> trait.
  friend constexpr auto operator<=>(const Result& l, const Result& r) noexcept
    requires((!VoidOrWeakOrd<T> || !::sus::ops::WeakOrd<E>) &&
             VoidOrPartialOrd<T> && ::sus::ops::PartialOrd<E>)
  {
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

/// Used to construct a Result<T, E> with an Ok(void) value.
///
/// Calling ok() produces a hint to make a Result<void, E> but does not actually
/// construct Result<void, E>. This is to deduce the actual type `E`
/// when it is constructed, avoid specifying it here, and support
/// conversions.
///
/// #[doc.overloads=ok.marker.void]
[[nodiscard]] inline constexpr auto ok() noexcept {
  return __private::OkVoidMarker();
}

/// Used to construct a Result<T, E> with an Ok(t) value.
///
/// Calling ok() produces a hint to make a Result<T, E> but does not actually
/// construct Result<T, E>. This is to deduce the actual types `T` and `E`
/// when it is constructed, avoid specifying them both here, and support
/// conversions.
///
/// #[doc.overloads=ok.marker]
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

// Implements sus::ops::Try for Result.
template <class T, class E>
struct sus::ops::TryImpl<::sus::result::Result<T, E>> {
  using Output = T;
  constexpr static bool is_success(const ::sus::result::Result<T, E>& t) {
    return t.is_ok();
  }
  constexpr static Output into_output(::sus::result::Result<T, E> t) {
    // SAFETY: The Result is verified to be holding Ok(T) by
    // `::sus::ops::try_into_output()` before it calls here.
    return ::sus::move(t).unwrap_unchecked(::sus::marker::unsafe_fn);
  }
  constexpr static ::sus::result::Result<T, E> from_output(Output t) {
    return ::sus::result::Result<T, E>::with(::sus::move(t));
  }
};

// Implements sus::iter::FromIterator for Result.
template <class T, class E>
struct sus::iter::FromIteratorImpl<::sus::result::Result<T, E>> {
  // TODO: Subdoc doesn't split apart template instantiations so comments
  // collide. This should be able to appear in the docs.
  //
  // Takes each element in the Iterator: if it is an Err, no further elements
  // are taken, and the Err is returned. Should no Err occur, a container with
  // the values of each Result is returned.
  template <class IntoIter, int&...,
            class Iter =
                std::decay_t<decltype(std::declval<IntoIter&&>().into_iter())>,
            class R = typename Iter::Item,
            class U = ::sus::result::__private::IsResultType<R>::ok_type,
            class F = ::sus::result::__private::IsResultType<R>::err_type>
    requires(::sus::result::__private::IsResultType<R>::value &&
             std::same_as<E, F> &&
             ::sus::iter::IntoIterator<IntoIter, ::sus::result::Result<U, E>>)
  static constexpr ::sus::result::Result<T, E> from_iter(
      IntoIter&& result_iter) noexcept
    requires(!std::is_void_v<T> && !std::is_reference_v<T> &&
             ::sus::iter::FromIterator<T, U>)
  {
    struct Unwrapper final : public ::sus::iter::IteratorBase<Unwrapper, U> {
      Unwrapper(Iter&& iter, Option<E>& err) : iter(iter), err(err) {}

      // sus::iter::Iterator trait.
      Option<U> next() noexcept {
        Option<::sus::result::Result<U, E>> try_item = iter.next();
        if (try_item.is_none()) return Option<U>();
        ::sus::result::Result<U, E> result =
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
    auto iter = Unwrapper(::sus::move(result_iter).into_iter(), err);
    auto success_out = ::sus::result::Result<T, E>::with(
        ::sus::iter::from_iter<T>(::sus::move(iter)));
    return ::sus::move(err).map_or_else(
        [&]() { return ::sus::move(success_out); },
        [](E e) { return ::sus::result::Result<T, E>::with_err(e); });
  }
};

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
