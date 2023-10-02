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
#include "sus/cmp/__private/void_concepts.h"
#include "sus/fn/fn_concepts.h"
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
#include "sus/option/option.h"
#include "sus/result/__private/marker.h"
#include "sus/result/__private/result_state.h"
#include "sus/result/__private/storage.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus {
/// The [`Result`]($sus::result::Result) type, and the
/// [`ok`]($sus::result::ok) and [`err`]($sus::result::err)
/// type-deduction constructor functions.
namespace result {}
}  // namespace sus

namespace sus::result {

using ::sus::cmp::__private::VoidOrEq;
using ::sus::cmp::__private::VoidOrOrd;
using ::sus::cmp::__private::VoidOrPartialOrd;
using ::sus::cmp::__private::VoidOrWeakOrd;
using ::sus::iter::Once;
using ::sus::mem::__private::IsTrivialCopyAssignOrRef;
using ::sus::mem::__private::IsTrivialCopyCtorOrRef;
using ::sus::mem::__private::IsTrivialDtorOrRef;
using ::sus::mem::__private::IsTrivialMoveAssignOrRef;
using ::sus::mem::__private::IsTrivialMoveCtorOrRef;
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

  // A helper type used for `T&` references since `T` can be void which then
  // makes `T&` ill-formed. So using this allows code to compile for functions
  // that are excluded when `T` is void without templating them. Why can't
  // `void` be a type.
  using TUnlessVoid = std::conditional_t<std::is_void_v<T>, OkVoid, T>;

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
  /// #[doc.overloads=ctor.ok]
  explicit constexpr Result(OkVoid) noexcept
    requires(std::is_void_v<T>)
      : Result(WITH_OK) {}
  /// #[doc.overloads=ctor.ok]
  explicit constexpr Result(const TUnlessVoid& t) noexcept
    requires(!std::is_void_v<T> &&       //
             !std::is_reference_v<T> &&  //
             ::sus::mem::Copy<T>)
      : Result(WITH_OK, t) {}
  /// #[doc.overloads=ctor.ok]
  explicit constexpr Result(TUnlessVoid&& t) noexcept
    requires(!std::is_void_v<T> &&       //
             !std::is_reference_v<T> &&  //
             ::sus::mem::Move<T>)
      : Result(WITH_OK, move_ok_to_storage(t)) {}
  /// #[doc.overloads=ctor.ok]
  template <std::convertible_to<TUnlessVoid> U>
  explicit constexpr Result(U&& t sus_lifetimebound) noexcept
    requires(!std::is_void_v<T> &&      //
             std::is_reference_v<T> &&  //
             sus::construct::SafelyConstructibleFromReference<T, U &&>)
      : Result(WITH_OK, move_ok_to_storage(t)) {}

  /// Construct an Result that is holding the given error value.
  static constexpr inline Result with_err(const E& e) noexcept
    requires(::sus::mem::Copy<E>)
  {
    return Result(WITH_ERR, e);
  }
  static constexpr inline Result with_err(E&& e) noexcept
    requires(::sus::mem::Move<E>)
  {
    return Result(WITH_ERR, ::sus::move(e));
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

  /// Copy constructor for `Result<T, E>` which satisfies
  /// [`sus::mem::Copy<Result<T, E>>`](sus-mem-Copy.html) if
  /// [`Copy<T>`](sus-mem-Copy.html) and
  /// [`Copy<E>`](sus-mem-Copy.html) are satisfied.
  ///
  /// If `T` and `E` can be trivially copy-constructed, then `Result<T, E>` can
  /// also be trivially copy-constructed.
  ///
  /// #[doc.overloads=copy]
  constexpr Result(const Result&)
    requires((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
             ::sus::mem::Copy<E> &&
             (std::is_void_v<T> || IsTrivialCopyCtorOrRef<T>) &&
             IsTrivialCopyCtorOrRef<E>)
  = default;

  /// #[doc.overloads=copy]
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

  /// #[doc.overloads=copy]
  constexpr Result(const Result&)
    requires(!((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
               ::sus::mem::Copy<E>))
  = delete;

  /// Copy assignment for `Result<T, E>` which satisfies
  /// [`sus::mem::Copy<Result<T, E>>`](sus-mem-Copy.html) if
  /// [`Copy<T>`](sus-mem-Copy.html) and
  /// [`Copy<E>`](sus-mem-Copy.html) are satisfied.
  ///
  /// If `T` and `E` can be trivially copy-assigned, then `Result<T, E>` can
  /// also be trivially copy-assigned.
  ///
  /// #[doc.overloads=copy]
  constexpr Result& operator=(const Result& o)
    requires((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
             ::sus::mem::Copy<E> &&
             (std::is_void_v<T> || IsTrivialCopyAssignOrRef<T>) &&
             IsTrivialCopyAssignOrRef<E>)
  = default;

  /// #[doc.overloads=copy]
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

  /// #[doc.overloads=copy]
  constexpr Result& operator=(const Result&)
    requires(!((std::is_void_v<T> || ::sus::mem::CopyOrRef<T>) &&
               ::sus::mem::Copy<E>))
  = delete;

  /// Move constructor for `Result<T, E>` which satisfies
  /// [`sus::mem::Move<Result<T, E>>`](sus-mem-Move.html) if
  /// [`Move<T>`](sus-mem-Move.html) and
  /// [`Move<E>`](sus-mem-Move.html) are satisfied.
  ///
  /// If `T` and `E` can be trivially move-constructed, then `Result<T, E>` can
  /// also be trivially move-constructed. When trivially-moved, the `Result` is
  /// copied on move, and the moved-from Result is unchanged but should still
  /// not be used thereafter without reinitializing it.
  ///
  /// #[doc.overloads=move]
  constexpr Result(Result&&)
    requires((std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) &&
             ::sus::mem::Move<E> &&
             (std::is_void_v<T> || IsTrivialMoveCtorOrRef<T>) &&
             IsTrivialMoveCtorOrRef<E>)
  = default;

  /// #[doc.overloads=move]
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

  /// #[doc.overloads=move]
  constexpr Result(Result&&)
    requires(!((std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) &&
               ::sus::mem::Move<E>))
  = delete;

  /// Move assignment for `Result<T, E>` which satisfies
  /// [`sus::mem::Move<Result<T, E>>`](sus-mem-Move.html) if
  /// [`Move<T>`](sus-mem-Move.html) and
  /// [`Move<E>`](sus-mem-Move.html) are satisfied.
  ///
  /// If `T` and `E` can be trivially move-assigned, then `Result<T, E>` can
  /// also be trivially move-assigned. When trivially-moved, the `Result` is
  /// copied on move, and the moved-from Result is unchanged but should still
  /// not be used thereafter without reinitializing it.
  ///
  /// #[doc.overloads=move]
  constexpr Result& operator=(Result&& o)
    requires((std::is_void_v<T> || ::sus::mem::MoveOrRef<T>) &&
             ::sus::mem::Move<E> &&
             (std::is_void_v<T> || IsTrivialMoveAssignOrRef<T>) &&
             IsTrivialMoveAssignOrRef<E>)
  = default;

  /// #[doc.overloads=move]
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

  /// #[doc.overloads=move]
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
          return Result(WITH_OK);
        else
          return Result(WITH_OK, ::sus::clone(storage_.ok_));
      case ResultState::IsErr:
        return Result(WITH_ERR, ::sus::clone(storage_.err_));
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
  /// Prefer to call `product()` on the iterator rather than calling
  /// `from_product()` directly.
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
    auto out = Result(IterUntilNone(it, err).product());
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
  /// Prefer to call `sum()` on the iterator rather than calling `from_sum()`
  /// directly.
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
    auto out = Result(IterUntilNone(it, err).sum());
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
  /// A reimplementation of `Result::unwrap_or()`.
  /// ```cpp
  /// auto x = Result<int, char>(2);
  /// switch (x) {
  ///  case Ok:
  ///   return sus::move(x).unwrap();
  ///  case Err:
  ///   return -1;
  /// }
  /// ```
  constexpr inline operator State() const& noexcept {
    ::sus::check(state_ != ResultState::IsMoved);
    return static_cast<State>(state_);
  }

  /// Calls `op` if the result is `Ok`, otherwise returns the `Err` value of
  /// self.
  ///
  ///  This function can be used for control flow based on `Result` values.
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(TUnlessVoid&&)> AndFn>
    requires(
        !std::is_void_v<T> &&  //
        __private::IsResultWithErrType<sus::fn::ReturnOnce<AndFn, T &&>, E>)
  constexpr sus::fn::ReturnOnce<AndFn, TUnlessVoid&&> and_then(
      AndFn op) && noexcept {
    ::sus::check(state_ != ResultState::IsMoved);
    switch (::sus::mem::replace(state_, ResultState::IsMoved)) {
      case ResultState::IsOk:
        return ::sus::fn::call_once(
            ::sus::move(op), ::sus::mem::take_and_destruct(
                                 ::sus::marker::unsafe_fn, storage_.ok_));
      case ResultState::IsErr:
        return sus::fn::ReturnOnce<AndFn, T&&>::with_err(
            ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                          storage_.err_));
      case ResultState::IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the
    // function.
    sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> AndFn>
    requires(std::is_void_v<T> &&  //
             __private::IsResultWithErrType<sus::fn::ReturnOnce<AndFn>, E>)
  constexpr sus::fn::ReturnOnce<AndFn> and_then(AndFn op) && noexcept {
    ::sus::check(state_ != ResultState::IsMoved);
    switch (::sus::mem::replace(state_, ResultState::IsMoved)) {
      case ResultState::IsOk: return ::sus::fn::call_once(::sus::move(op));
      case ResultState::IsErr:
        return sus::fn::ReturnOnce<AndFn>::with_err(
            ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                          storage_.err_));
      case ResultState::IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the
    // function.
    sus::unreachable_unchecked(::sus::marker::unsafe_fn);
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
        return Option<T>(static_cast<T>(::sus::mem::take_and_destruct(
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
        return Option<E>(::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                                       storage_.err_));
      case ResultState::IsMoved: break;
    }
    // SAFETY: The state_ is verified to be Ok or Err at the top of the
    // function.
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  /// Returns a const reference to the contained `Ok` value.
  ///
  /// # Panics
  /// Panics if the value is an `Err`.
  constexpr const std::remove_reference_t<TUnlessVoid>& as_value() const&
    requires(!std::is_void_v<T>)
  {
    if (state_ != ResultState::IsOk) [[unlikely]] {
      if (state_ == ResultState::IsErr) {
        if constexpr (fmt::is_formattable<E>::value) {
          panic_with_message(fmt::to_string(storage_.err_));
        } else {
          panic_with_message("Result has error state");
        }
      } else {
        panic_with_message("Result used after move");
      }
    }
    return storage_.ok_;
  }
  constexpr const std::remove_reference_t<TUnlessVoid>& as_value() && = delete;

  /// Returns a mutable reference to the contained `Ok` value.
  ///
  /// # Panics
  /// Panics if the value is an `Err`.
  constexpr std::remove_reference_t<TUnlessVoid>& as_value_mut() &
    requires(!std::is_void_v<T>)
  {
    if (state_ != ResultState::IsOk) [[unlikely]] {
      if (state_ == ResultState::IsErr) {
        if constexpr (fmt::is_formattable<E>::value) {
          panic_with_message(fmt::to_string(storage_.err_));
        } else {
          panic_with_message("Result has error state");
        }
      } else {
        panic_with_message("Result used after move");
      }
    }
    return storage_.ok_;
  }
  /// Returns a const reference to the contained `Err` value.
  ///
  /// # Panics
  /// Panics if the value is an `Ok` or the Result is moved from.
  constexpr const std::remove_reference_t<E>& as_err() const& {
    if (state_ != ResultState::IsErr) [[unlikely]] {
      if (state_ == ResultState::IsOk) {
        if constexpr (!std::is_void_v<T>) {
          if constexpr (fmt::is_formattable<T>::value) {
            panic_with_message(fmt::to_string(storage_.ok_));
          } else {
            panic_with_message("Result has ok state");
          }
        } else {
          panic_with_message("Result has ok state");
        }
      } else {
        panic_with_message("Result used after move");
      }
    }
    return storage_.err_;
  }
  constexpr const std::remove_reference_t<E>& as_err() && = delete;

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
    if (was != ResultState::IsOk) [[unlikely]] {
      if (was == ResultState::IsErr) {
        if constexpr (fmt::is_formattable<E>::value) {
          panic_with_message(fmt::to_string(storage_.err_));
        } else {
          panic_with_message("Result has error state");
        }
      } else {
        panic_with_message("Result used after move");
      }
    }
    if constexpr (!std::is_void_v<T>) {
      return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                           storage_.ok_);
    }
  }

  /// Returns the contained `Ok` value, consuming the self value.
  ///
  /// Because this function may panic, its use is generally discouraged.
  /// Instead, prefer to use pattern matching and handle the `Err` case
  /// explicitly, or call `unwrap_or`, `unwrap_or_else`, or `unwrap_or_default`.
  ///
  /// # Panics
  /// Panics if the value is an Err, with a panic message including the passed
  /// message, and the content of the Err.
  constexpr inline T expect(const char* msg) && noexcept {
    ResultState was = ::sus::mem::replace(state_, ResultState::IsMoved);
    if (was != ResultState::IsOk) [[unlikely]] {
      if (was == ResultState::IsErr) {
        if constexpr (fmt::is_formattable<E>::value) {
          panic_with_message(fmt::format("{}: {}", msg, storage_.err_));
        } else {
          panic_with_message(msg);
        }
      } else {
        panic_with_message("Result used after move");
      }
    }
    if constexpr (!std::is_void_v<T>) {
      return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                           storage_.ok_);
    }
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
        "called `Result::unwrap_or_default()` on a moved Result");
    if constexpr (!std::is_void_v<T>) {
      if (was == ResultState::IsOk) {
        return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                             storage_.ok_);
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
    if (state_ != ResultState::IsOk) {
      // This enables Clang to drop any branches that were used to construct
      // the Result. Without this, the optimizer keeps the whole Result
      // construction, possibly because the `state_` gets clobbered below?
      // The signed code version at https://godbolt.org/z/Gax47shsb improves
      // greatly when the compiler is informed about the UB here.
      sus::unreachable_unchecked(::sus::marker::unsafe_fn);
    }
    state_ = ResultState::IsMoved;
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
    ResultState was = ::sus::mem::replace(state_, ResultState::IsMoved);
    if (was != ResultState::IsErr) [[unlikely]] {
      if (was == ResultState::IsOk) {
        if constexpr (!std::is_void_v<T>) {
          if constexpr (fmt::is_formattable<T>::value) {
            panic_with_message(fmt::to_string(storage_.ok_));
          } else {
            panic_with_message("Result has ok state");
          }
        } else {
          panic_with_message("Result has ok state");
        }
      } else {
        panic_with_message("Result used after move");
      }
    }
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                         storage_.err_);
  }

  /// Returns the contained `Err` value, consuming the self value, without
  /// checking that the value is not an `Ok`.
  ///
  /// # Safety
  /// Calling this method on an `Ok` or a moved-from Result is Undefined
  /// Behavior.
  constexpr inline E unwrap_err_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    if (state_ != ResultState::IsErr) {
      // Match the code in unwrap_unchecked, and tell the compiler that the
      // `state_` is an Err before clobbering it.
      sus::unreachable_unchecked(::sus::marker::unsafe_fn);
    }
    state_ = ResultState::IsMoved;
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
  /// auto ok = sus::Result<i32, ECode>(2);
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

  constexpr ::sus::option::OptionIter<
      const std::remove_reference_t<TUnlessVoid>&>
  iter() const& noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (state_ == ResultState::IsOk) {
      return ::sus::option::OptionIter<const std::remove_reference_t<T>&>(
          // SAFETY: The static_cast is needed to convert the pointer storage
          // type to a `const T&`, which does not create a temporary as it's
          // converting a pointer to a reference.
          Option<const std::remove_reference_t<T>&>(
              static_cast<const std::remove_reference_t<T>&>(storage_.ok_)));
    } else {
      return ::sus::option::OptionIter<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>());
    }
  }
  constexpr ::sus::option::OptionIter<
      const std::remove_reference_t<TUnlessVoid>&>
  iter() && noexcept
    requires(!std::is_void_v<T> && std::is_reference_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (::sus::mem::replace(state_, ResultState::IsMoved) ==
        ResultState::IsOk) {
      return ::sus::option::OptionIter<const std::remove_reference_t<T>&>(
          // SAFETY: The static_cast is needed to convert the pointer storage
          // type to a `const T&`, which does not create a temporary as it's
          // converting a pointer to a reference.
          Option<const std::remove_reference_t<T>&>(
              static_cast<const std::remove_reference_t<T>&>(storage_.ok_)));
    } else {
      storage_.destroy_err();
      return ::sus::option::OptionIter<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>());
    }
  }

  constexpr ::sus::option::OptionIter<TUnlessVoid&> iter_mut() & noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (state_ == ResultState::IsOk) {
      return ::sus::option::OptionIter<T&>(Option<T&>(storage_.ok_));
    } else {
      return ::sus::option::OptionIter<T&>(Option<T&>());
    }
  }
  constexpr ::sus::option::OptionIter<TUnlessVoid&> iter_mut() && noexcept
    requires(!std::is_void_v<T> &&  //
             std::is_reference_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (::sus::mem::replace(state_, ResultState::IsMoved) ==
        ResultState::IsOk) {
      return ::sus::option::OptionIter<T&>(Option<T&>(storage_.ok_));
    } else {
      storage_.destroy_err();
      return ::sus::option::OptionIter<T&>(Option<T&>());
    }
  }

  constexpr ::sus::option::OptionIter<T> into_iter() && noexcept
    requires(!std::is_void_v<T>)
  {
    ::sus::check(state_ != ResultState::IsMoved);
    if (::sus::mem::replace(state_, ResultState::IsMoved) ==
        ResultState::IsOk) {
      return ::sus::option::OptionIter<T>(
          Option<T>(::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                                  storage_.ok_)));
    } else {
      storage_.destroy_err();
      return ::sus::option::OptionIter<T>(Option<T>());
    }
  }

  /// Compares two [`Result`]($sus::result::Result)s for equality if the types
  /// inside satisfy `Eq`.
  ///
  /// Satisfies the [`Eq`]($sus::cmp::Eq) concept.
  ///
  /// # Implementation Note
  /// The non-template overload allows ok/err marker types to convert to
  /// Option for comparison.
  friend constexpr bool operator==(const Result& l, const Result& r) noexcept
    requires(VoidOrEq<T> && ::sus::cmp::Eq<E>)
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
    requires(VoidOrEq<T, U> && ::sus::cmp::Eq<E, F>)
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
    requires(!(VoidOrEq<T, U> && ::sus::cmp::Eq<E, F>))
  friend constexpr bool operator==(const Result& l,
                                   const Result<U, F>& r) = delete;

  /// Compares two [`Result`]($sus::result::Result)s for their ordering if the
  /// types inside can be compared.
  ///
  /// Satisfies the [`StrongOrd`]($sus::cmp::StrongOrd),
  /// [`Ord`]($sus::cmp::Ord), or
  /// [`PartialOrd`]($sus::cmp::PartialOrd) concept, depending on whether the
  /// internal types do. The `Result` will satisfy the strongest possible
  /// ordering.
  ///
  /// # Implementation Note
  /// The non-template overloads allow ok/err marker types to convert to
  /// Option for comparison.
  friend constexpr std::strong_ordering operator<=>(const Result& l,
                                                    const Result& r) noexcept
    requires(VoidOrOrd<T> && ::sus::cmp::StrongOrd<E>)
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
    requires(VoidOrOrd<T, U> && ::sus::cmp::StrongOrd<E, F>)
  friend constexpr std::strong_ordering operator<=>(
      const Result& l, const Result<U, F>& r) noexcept {
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

  // sus::cmp::Ord<Result<T, E>> trait.
  friend constexpr std::weak_ordering operator<=>(const Result& l,
                                                  const Result& r) noexcept
    requires((!VoidOrOrd<T> || !::sus::cmp::StrongOrd<E>) && VoidOrWeakOrd<T> &&
             ::sus::cmp::Ord<E>)
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
    requires((!VoidOrOrd<T, U> || !::sus::cmp::StrongOrd<E, F>) &&
             VoidOrWeakOrd<T, U> && ::sus::cmp::Ord<E, F>)
  friend constexpr std::weak_ordering operator<=>(
      const Result& l, const Result<U, F>& r) noexcept {
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

  // sus::cmp::PartialOrd<Result<T, E>> trait.
  friend constexpr std::partial_ordering operator<=>(const Result& l,
                                                     const Result& r) noexcept
    requires((!VoidOrWeakOrd<T> || !::sus::cmp::Ord<E>) &&
             VoidOrPartialOrd<T> && ::sus::cmp::PartialOrd<E>)
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
    requires((!VoidOrWeakOrd<T, U> || !::sus::cmp::Ord<E, F>) &&
             VoidOrPartialOrd<T, U> && ::sus::cmp::PartialOrd<E, F>)
  friend constexpr std::partial_ordering operator<=>(
      const Result& l, const Result<U, F>& r) noexcept {
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
    requires(!VoidOrPartialOrd<T, U> || !::sus::cmp::PartialOrd<E, F>)
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
  enum WithOk { WITH_OK };
  constexpr inline Result(WithOk) noexcept
    requires(std::is_void_v<T>)
      : storage_(), state_(ResultState::IsOk) {}
  template <std::convertible_to<T> U>
  constexpr inline Result(WithOk, const U& t) noexcept
      : storage_(__private::kWithT, t), state_(ResultState::IsOk) {}
  template <std::convertible_to<T> U>
  constexpr inline Result(WithOk, U&& t) noexcept
    requires(::sus::mem::Move<T>)
      : storage_(__private::kWithT, ::sus::move(t)),
        state_(ResultState::IsOk) {}
  // Constructors for `Err`.
  enum WithErr { WITH_ERR };
  constexpr inline Result(WithErr, const E& e) noexcept
      : storage_(__private::kWithE, e), state_(ResultState::IsErr) {}
  constexpr inline Result(WithErr, E&& e) noexcept
    requires(::sus::mem::Move<E>)
      : storage_(__private::kWithE, ::sus::move(e)),
        state_(ResultState::IsErr) {}

  struct VoidStorage {};
  using OkStorageType =
      std::conditional_t<std::is_reference_v<T>, StoragePointer<T>,
                         std::conditional_t<std::is_void_v<T>, VoidStorage, T>>;

  [[sus_no_unique_address]] Storage<OkStorageType, E> storage_;
  ResultState state_;

  sus_class_trivially_relocatable_if_types(
      ::sus::marker::unsafe_fn,
      std::remove_const_t<std::remove_reference_t<decltype(storage_.ok_)>>,
      std::remove_const_t<std::remove_reference_t<decltype(storage_.err_)>>,
      decltype(state_));
};

/// Implicit for-ranged loop iteration via [`Result::iter()`](
/// $sus::result::Result::iter).
using sus::iter::begin;
/// Implicit for-ranged loop iteration via [`Result::iter()`](
/// $sus::result::Result::iter).
using sus::iter::end;

/// Used to construct a [`Result<T, E>`]($sus::result::Result) with an `Ok(t)`
/// value.
///
/// Calling `ok()` produces a hint to make a Result<T, E> but does not actually
/// construct [`Result<T, E>`]($sus::result::Result).
/// This is to deduce the actual types `T` and `E`
/// when it is constructed, avoid specifying them both here, and support
/// conversions.
///
/// [`Result<void, E>`]($sus::result::Result) can also be constructed from
/// calling `ok()` without an argument.
template <class T>
[[nodiscard]] inline constexpr auto ok(T&& t sus_lifetimebound) noexcept {
  return __private::OkMarker<T&&>(::sus::forward<T>(t));
}

[[nodiscard]] inline constexpr auto ok() noexcept {
  return __private::OkVoidMarker();
}

/// Used to construct a [`Result<T, E>`]($sus::result::Result) with an `Err(e)`
/// value.
///
/// Calling `err()` produces a hint to make a Result<T, E> but does not actually
/// construct [`Result<T, E>`]($sus::result::Result).
/// This is to deduce the actual types `T` and `E`
/// when it is constructed, avoid specifying them both here, and support
/// conversions.
template <class E>
[[nodiscard]] inline constexpr auto err(E&& e sus_lifetimebound) noexcept {
  return __private::ErrMarker<E&&>(::sus::forward<E>(e));
}

}  // namespace sus::result

// Implements sus::ops::Try for Result<T, E>
template <class T, class E>
  requires(!std::is_void_v<T>)
struct sus::ops::TryImpl<::sus::result::Result<T, E>> {
  using Output = T;
  template <class U>
  using RemapOutput = ::sus::result::Result<U, E>;
  constexpr static bool is_success(const ::sus::result::Result<T, E>& t) {
    return t.is_ok();
  }
  constexpr static Output into_output(::sus::result::Result<T, E> t) {
    // SAFETY: The Result is verified to be holding Ok(T) by
    // `sus::ops::try_into_output()` before it calls here.
    return ::sus::move(t).unwrap_unchecked(::sus::marker::unsafe_fn);
  }
  constexpr static ::sus::result::Result<T, E> from_output(Output t) {
    return ::sus::result::Result<T, E>(::sus::move(t));
  }
  template <class U>
  constexpr static ::sus::result::Result<T, E> preserve_error(
      ::sus::result::Result<U, E> t) noexcept {
    // SAFETY: The Result is verified to be holding Err(T) by
    // `sus::ops::try_preserve_error()` before it calls here.
    return ::sus::result::Result<T, E>::with_err(
        ::sus::move(t).unwrap_err_unchecked(::sus::marker::unsafe_fn));
  }
  // Implements sus::ops::TryDefault for `Result<T, E>` if `T` satisfies
  // `Default`.
  constexpr static ::sus::result::Result<T, E> from_default() noexcept
    requires(sus::construct::Default<T>)
  {
    return ::sus::result::Result<T, E>(T());
  }
};

// Implements sus::ops::Try for Result<void, E>
template <class E>
struct sus::ops::TryImpl<::sus::result::Result<void, E>> {
  using Output = void;
  template <class U>
  using RemapOutput = ::sus::result::Result<U, E>;
  constexpr static bool is_success(
      const ::sus::result::Result<void, E>& t) noexcept {
    return t.is_ok();
  }
  constexpr static void into_output(::sus::result::Result<void, E> t) noexcept {
  }
  constexpr static ::sus::result::Result<void, E> preserve_error(
      ::sus::result::Result<void, E> t) noexcept {
    ::sus::move(t);  // Preserve the Err by returning it.
  }
  // Implements sus::ops::TryDefault for `Result<void, E>`.
  constexpr static ::sus::result::Result<void, E> from_default() noexcept {
    return ::sus::result::Result<void, E>(::sus::result::OkVoid());
  }
};

// Implements sus::iter::FromIterator for Result.
template <class T, class E>
struct sus::iter::FromIteratorImpl<::sus::result::Result<T, E>> {
  // TODO: Subdoc doesn't split apart template instantiations so comments
  // collide. This should be able to appear in the docs.
  //
  // Takes each element in the Iterator: if it is an Err, no further elements
  // are taken, and the Err is returned. Should no Err occur, a collection with
  // the values of each Result is returned.
  template <class IntoIter, int&...,
            class Iter =
                std::decay_t<decltype(std::declval<IntoIter&&>().into_iter())>,
            class Item = typename Iter::Item,
            class U = ::sus::result::__private::IsResultType<Item>::ok_type,
            class F = ::sus::result::__private::IsResultType<Item>::err_type>
    requires(::sus::result::__private::IsResult<Item> && std::same_as<E, F> &&
             ::sus::iter::IntoIterator<IntoIter, ::sus::result::Result<U, E>>)
  static constexpr ::sus::result::Result<T, E> from_iter(
      IntoIter&& result_iter) noexcept
    requires(!std::is_void_v<T> && !std::is_reference_v<T> &&
             ::sus::iter::FromIterator<T, U>)
  {
    struct Unwrapper final : public ::sus::iter::IteratorBase<Unwrapper, U> {
      constexpr Unwrapper(Iter&& iter, Option<E>& err) : iter(iter), err(err) {}

      // sus::iter::Iterator trait.
      constexpr Option<U> next() noexcept {
        Option<::sus::result::Result<U, E>> try_item = iter.next();
        if (try_item.is_none()) return Option<U>();
        ::sus::result::Result<U, E> result =
            ::sus::move(try_item).unwrap_unchecked(::sus::marker::unsafe_fn);
        if (result.is_ok())
          return Option<U>(
              ::sus::move(result).unwrap_unchecked(::sus::marker::unsafe_fn));
        err.insert(
            ::sus::move(result).unwrap_err_unchecked(::sus::marker::unsafe_fn));
        return Option<U>();
      }
      constexpr ::sus::iter::SizeHint size_hint() const noexcept {
        return ::sus::iter::SizeHint(0u, iter.size_hint().upper);
      }

      Iter& iter;
      Option<E>& err;
    };

    auto err = Option<E>();
    auto iter = Unwrapper(::sus::move(result_iter).into_iter(), err);
    auto out = ::sus::result::Result<T, E>(
        ::sus::iter::from_iter<T>(::sus::move(iter)));
    if (err.is_some())
      out = ::sus::result::Result<T, E>::with_err(::sus::move(err).unwrap());
    return out;
  }
};

// std hash support.
template <class T, class E>
struct std::hash<::sus::result::Result<T, E>> {
  auto operator()(const ::sus::result::Result<T, E>& u) const noexcept {
    if (u.is_ok())
      return std::hash<T>()(u.as_value());
    else
      return std::hash<T>()(u.as_err());
  }
};
template <class T, class E>
  requires(::sus::cmp::Eq<::sus::result::Result<T, E>>)
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
        out = underlying_ok_.format(t.as_value(), ctx);
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
