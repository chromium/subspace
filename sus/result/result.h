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
      : Result(WITH_OK, ::sus::move(t)) {}
  /// #[doc.overloads=ctor.ok]
  template <std::convertible_to<TUnlessVoid> U>
  explicit constexpr Result(U&& t sus_lifetimebound) noexcept
    requires(!std::is_void_v<T> &&      //
             std::is_reference_v<T> &&  //
             sus::construct::SafelyConstructibleFromReference<T, U &&>)
      : Result(WITH_OK, make_ok_storage(::sus::forward<U>(t))) {}

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

  /// Destructor for the `Result`.
  ///
  /// Destroys the Ok or Err value contained within the `Result`.
  ///
  /// If T and E can be trivially destroyed, `Result<T, E>` can also be
  /// trivially destroyed.
  constexpr ~Result() = default;

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
    requires(::sus::mem::CopyOrRefOrVoid<T> && ::sus::mem::Copy<E>)
  = default;

  /// #[doc.overloads=copy]
  constexpr Result(const Result&)
    requires(!::sus::mem::CopyOrRefOrVoid<T> || !::sus::mem::Copy<E>)
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
    requires(::sus::mem::CopyOrRefOrVoid<T> && ::sus::mem::Copy<E>)
  = default;

  /// #[doc.overloads=copy]
  constexpr Result& operator=(const Result&)
    requires(!::sus::mem::CopyOrRefOrVoid<T> || !::sus::mem::Copy<E>)
  = delete;

  /// Move constructor for `Result<T, E>` which satisfies
  /// [`Move`]($sus::mem::Move), if `T` and `E` both satisfy
  /// [`Move`]($sus::mem::Move).
  ///
  /// If `T` and `E` can be trivially move-constructed, then `Result<T, E>` can
  /// also be trivially move-constructed. When trivially-moved, the `Result` is
  /// copied on move, and the moved-from `Result` is unchanged but should still
  /// not be used thereafter without reinitializing it.
  ///
  /// #[doc.overloads=move]
  constexpr Result(Result&&)
    requires(::sus::mem::MoveOrRefOrVoid<T> && ::sus::mem::Move<E>)
  = default;

  /// #[doc.overloads=move]
  constexpr Result(Result&&)
    requires(!::sus::mem::MoveOrRefOrVoid<T> || !::sus::mem::Move<E>)
  = delete;

  /// Move assignment for `Result<T, E>` which satisfies
  /// [`Move`]($sus::mem::Move), if `T` and `E` both satisfy
  ///  [`Move`]($sus::mem::Move).
  ///
  /// If `T` and `E` can be trivially move-assigned, then `Result<T, E>` can
  /// also be trivially move-assigned. When trivially-moved, the `Result` is
  /// copied on move, and the moved-from Result is unchanged but should still
  /// not be used thereafter without reinitializing it.
  ///
  /// #[doc.overloads=move]
  constexpr Result& operator=(Result&&)
    requires(::sus::mem::MoveOrRefOrVoid<T> && ::sus::mem::Move<E>)
  = default;

  /// #[doc.overloads=move]
  constexpr Result& operator=(Result&&)
    requires(!::sus::mem::MoveOrRefOrVoid<T> || !::sus::mem::Move<E>)
  = delete;

  constexpr Result clone() const& noexcept
    requires((std::is_void_v<T> || ::sus::mem::Clone<T>) &&
             ::sus::mem::Clone<E> &&
             !(::sus::mem::CopyOrRefOrVoid<T> && ::sus::mem::Copy<E>))
  {
    if (storage_.is_ok()) {
      if constexpr (std::is_void_v<T>)
        return Result(WITH_OK);
      else
        return Result(WITH_OK, ::sus::clone(storage_.template get_ok<T>()));
    } else if (storage_.is_err()) {
      return Result(WITH_ERR, ::sus::clone(storage_.get_err()));
    } else {
      sus_panic_with_message("Result used after move");
    }
  }

  constexpr void clone_from(const Result& source) &
    requires((std::is_void_v<T> || ::sus::mem::Clone<T>) &&
             ::sus::mem::Clone<E> &&
             !(::sus::mem::CopyOrRefOrVoid<T> && ::sus::mem::Copy<E>))
  {
    if (source.storage_.is_moved()) [[unlikely]] {
      sus_panic_with_message("Result used after move");
    } else if (&source == this) [[unlikely]] {
      // Nothing to do.
    } else if (storage_.is_moved()) {
      *this = source.clone();  // Replace the moved self.
    } else if (storage_.is_ok() != source.storage_.is_ok()) {
      // Moving to a different state, replace everything.
      *this = source.clone();
    } else {
      // Both are in the same state. So recursively clone_into.
      if (storage_.is_ok()) {
        if constexpr (!std::is_void_v<T>)
          ::sus::clone_into(storage_.template get_ok_mut<T>(),
                            source.storage_.template get_ok<T>());
      } else {
        ::sus::clone_into(storage_.get_err_mut(), source.storage_.get_err());
      }
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
    sus_check(!storage_.is_moved());
    return storage_.is_ok();
  }

  /// Returns true if the result is `Err`.
  constexpr inline bool is_err() const& noexcept {
    sus_check(!storage_.is_moved());
    return storage_.is_err();
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
    sus_check(!storage_.is_moved());
    if (storage_.is_ok())
      return State::Ok;
    else
      return State::Err;
  }

  /// Calls `op` if the result is `Ok`, otherwise returns the `Err` value of
  /// self.
  ///
  ///  This function can be used for control flow based on `Result` values.
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(TUnlessVoid&&)> AndFn>
    requires(
        // TODO: FnOnce should let us plug in our own type check instead of
        // NonVoid. Then this becomes part of the FnOnce constraint.
        __private::IsResultWithErrType<sus::fn::ReturnOnce<AndFn, T &&>, E>)
  constexpr sus::fn::ReturnOnce<AndFn, TUnlessVoid&&> and_then(
      AndFn op) && noexcept {
    if (storage_.is_ok()) {
      return ::sus::fn::call_once(::sus::move(op),
                                  storage_.template take_ok<T>());
    } else if (storage_.is_err()) {
      return sus::fn::ReturnOnce<AndFn>::with_err(storage_.take_err());
    } else {
      sus_panic_with_message("Result used after move");
    }
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> AndFn>
    requires(std::is_void_v<T> &&  //
             // TODO: FnOnce should let us plug in our own type check instead of
             // NonVoid. Then this becomes part of the FnOnce constraint.
             __private::IsResultWithErrType<sus::fn::ReturnOnce<AndFn>, E>)
  constexpr sus::fn::ReturnOnce<AndFn> and_then(AndFn op) && noexcept {
    if (storage_.is_ok()) {
      storage_.drop_ok();
      return ::sus::fn::call_once(::sus::move(op));
    } else if (storage_.is_err()) {
      return sus::fn::ReturnOnce<AndFn>::with_err(storage_.take_err());
    } else {
      sus_panic_with_message("Result used after move");
    }
  }
  /// Converts from `Result<T, E>` to [`Option<T>`]($sus::option::Option).
  ///
  /// Converts self into an `Option<T>`, consuming self, and discarding the
  /// error, if any.
  ///
  /// [`Option`]($sus::option::Option) can not hold `void`, so this method is
  /// not present on `Result<void, E>`.
  constexpr inline Option<T> ok() && noexcept
    requires(!std::is_void_v<T>)
  {
    if (storage_.is_ok()) {
      return Option<T>(storage_.template take_ok<T>());
    } else if (storage_.is_err()) {
      storage_.drop_err();
      return Option<T>();
    } else {
      sus_panic_with_message("Result used after move");
    }
  }

  /// Converts from `Result<T, E>` to `Option<E>`.
  ///
  /// Converts self into an `Option<E>`, consuming self, and discarding the
  /// success value, if any.
  constexpr inline Option<E> err() && noexcept {
    if (storage_.is_ok()) {
      storage_.drop_ok();
      return Option<E>();
    } else if (storage_.is_err()) {
      return Option<E>(storage_.take_err());
    } else {
      sus_panic_with_message("Result used after move");
    }
  }

  /// Returns a const reference to the contained `Ok` value.
  ///
  /// # Panics
  /// Panics if the value is an `Err`.
  constexpr const std::remove_reference_t<TUnlessVoid>& as_value() const&
    requires(!std::is_void_v<T>)
  {
    if (storage_.is_ok()) [[likely]] {
      return storage_.template get_ok<T>();
    } else if (storage_.is_err()) {
      if constexpr (fmt::is_formattable<E>::value) {
        sus_panic_with_message(fmt::to_string(storage_.get_err()));
      } else {
        sus_panic_with_message("Result has error state");
      }
    } else {
      sus_panic_with_message("Result used after move");
    }
  }
  constexpr const std::remove_reference_t<TUnlessVoid>& as_value() && = delete;

  /// Returns a mutable reference to the contained `Ok` value.
  ///
  /// # Panics
  /// Panics if the value is an `Err`.
  constexpr std::remove_reference_t<TUnlessVoid>& as_value_mut() &
    requires(!std::is_void_v<T>)
  {
    if (storage_.is_ok()) [[likely]] {
      return storage_.template get_ok_mut<T>();
    } else if (storage_.is_err()) {
      if constexpr (fmt::is_formattable<E>::value) {
        sus_panic_with_message(fmt::to_string(storage_.get_err()));
      } else {
        sus_panic_with_message("Result has error state");
      }
    } else {
      sus_panic_with_message("Result used after move");
    }
  }
  /// Returns a const reference to the contained `Err` value.
  ///
  /// # Panics
  /// Panics if the value is an `Ok` or the Result is moved from.
  constexpr const E& as_err() const& {
    if (storage_.is_err()) [[likely]] {
      return storage_.get_err();
    } else if (storage_.is_ok()) {
      if constexpr (std::is_void_v<T>) {
        sus_panic_with_message("Result has ok state");
      } else if constexpr (!fmt::is_formattable<T>::value) {
        sus_panic_with_message("Result has ok state");
      } else {
        sus_panic_with_message(fmt::to_string(storage_.template get_ok<T>()));
      }
    } else {
      sus_panic_with_message("Result used after move");
    }
  }
  constexpr const E& as_err() && = delete;

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
    if (storage_.is_ok()) [[likely]] {
      return storage_.template take_ok<T>();
    } else if (storage_.is_err()) {
      if constexpr (fmt::is_formattable<E>::value) {
        sus_panic_with_message(fmt::to_string(storage_.get_err()));
      } else {
        sus_panic_with_message("Result has error state");
      }
    } else {
      sus_panic_with_message("Result used after move");
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
    if (storage_.is_ok()) [[likely]] {
      return storage_.template take_ok<T>();
    } else if (storage_.is_err()) {
      if constexpr (fmt::is_formattable<E>::value) {
        sus_panic_with_message(fmt::format("{}: {}", msg, storage_.get_err()));
      } else {
        sus_panic_with_message(msg);
      }
    } else {
      sus_panic_with_message("Result used after move");
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
    if (storage_.is_ok()) {
      return storage_.template take_ok<T>();
    } else if (storage_.is_err()) {
      storage_.drop_err();
      if constexpr (!std::is_void_v<T>)
        return T();
      else
        return;
    } else {
      sus_panic_with_message("Result used after move");
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
    if (!storage_.is_ok()) {
      // This enables Clang to drop any branches that were used to construct
      // the Result. Without this, the optimizer keeps the whole Result
      // construction, possibly because the `state_` gets clobbered below?
      // The signed code version at https://godbolt.org/z/Gax47shsb improves
      // greatly when the compiler is informed about the UB here.
      sus_unreachable_unchecked(::sus::marker::unsafe_fn);
    }
    return storage_.template take_ok<T>();
  }

  /// Returns the contained `Err` value, consuming the self value.
  ///
  /// # Panics
  /// Panics if the value is an `Ok` or the Result is moved from.
  constexpr inline E unwrap_err() && noexcept {
    if (storage_.is_err()) [[likely]] {
      return storage_.take_err();
    } else if (storage_.is_ok()) {
      if constexpr (std::is_void_v<T>) {
        sus_panic_with_message("Result has ok state");
      } else if constexpr (!fmt::is_formattable<T>::value) {
        sus_panic_with_message("Result has ok state");
      } else {
        sus_panic_with_message(fmt::to_string(storage_.template get_ok<T>()));
      }
    } else {
      sus_panic_with_message("Result used after move");
    }
  }

  /// Returns the contained `Err` value, consuming the self value, without
  /// checking that the value is not an `Ok`.
  ///
  /// # Safety
  /// Calling this method on an `Ok` or a moved-from Result is Undefined
  /// Behavior.
  constexpr inline E unwrap_err_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    if (!storage_.is_err()) {
      // Match the code in unwrap_unchecked, and tell the compiler that the
      // `state_` is an Err before clobbering it.
      sus_unreachable_unchecked(::sus::marker::unsafe_fn);
    }
    return storage_.take_err();
  }

  /// Returns the contained `Ok` value or computes it from a closure.
  ///
  /// # Examples
  /// Basic usage:
  /// ```
  /// enum ECode { ItsHappening = -1 };
  /// auto conv = [](ECode e) { return static_cast<i32>(e); };
  /// auto ok = sus::Result<i32, ECode>(2);
  /// sus_check(sus::move(ok).unwrap_or_else(conv) == 2);
  /// auto err = sus::Result<i32, ECode>::with_err(ItsHappening);
  /// sus_check(sus::move(err).unwrap_or_else(conv) == -1);
  /// ```
  constexpr T unwrap_or_else(::sus::fn::FnOnce<T(E&&)> auto op) && noexcept {
    if (storage_.is_ok()) {
      return storage_.template take_ok<T>();
    } else if (storage_.is_err()) {
      return ::sus::fn::call_once(::sus::move(op), storage_.take_err());
    } else {
      sus_panic_with_message("Result used after move");
    }
  }

  constexpr ::sus::option::OptionIter<
      const std::remove_reference_t<TUnlessVoid>&>
  iter() const& noexcept
    requires(!std::is_void_v<T>)
  {
    if (storage_.is_ok()) {
      return ::sus::option::OptionIter<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>(
              storage_.template get_ok<T>()));
    } else if (storage_.is_err()) {
      return ::sus::option::OptionIter<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>());
    } else {
      sus_panic_with_message("Result used after move");
    }
  }
  constexpr ::sus::option::OptionIter<
      const std::remove_reference_t<TUnlessVoid>&>
  iter() && noexcept
    requires(!std::is_void_v<T> && std::is_reference_v<T>)
  {
    if (storage_.is_ok()) {
      return ::sus::option::OptionIter<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>(
              storage_.template take_ok<T>()));
    } else if (storage_.is_err()) {
      storage_.drop_err();
      return ::sus::option::OptionIter<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>());
    } else {
      sus_panic_with_message("Result used after move");
    }
  }

  constexpr ::sus::option::OptionIter<TUnlessVoid&> iter_mut() & noexcept
    requires(!std::is_void_v<T>)
  {
    if (storage_.is_ok()) {
      return ::sus::option::OptionIter<T&>(
          Option<T&>(storage_.template get_ok_mut<T>()));
    } else if (storage_.is_err()) {
      return ::sus::option::OptionIter<T&>(Option<T&>());
    } else {
      sus_panic_with_message("Result used after move");
    }
  }
  constexpr ::sus::option::OptionIter<TUnlessVoid&> iter_mut() && noexcept
    requires(!std::is_void_v<T> &&  //
             std::is_reference_v<T>)
  {
    if (storage_.is_ok()) {
      return ::sus::option::OptionIter<T&>(
          Option<T&>(storage_.template take_ok<T>()));
    } else if (storage_.is_err()) {
      storage_.drop_err();
      return ::sus::option::OptionIter<T&>(Option<T&>());
    } else {
      sus_panic_with_message("Result used after move");
    }
  }

  constexpr ::sus::option::OptionIter<T> into_iter() && noexcept
    requires(!std::is_void_v<T>)
  {
    if (storage_.is_ok()) {
      return ::sus::option::OptionIter<T>(
          Option<T>(storage_.template take_ok<T>()));
    } else if (storage_.is_err()) {
      storage_.drop_err();
      return ::sus::option::OptionIter<T>(Option<T>());
    } else {
      sus_panic_with_message("Result used after move");
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
    return eq(l, r);
  }
  template <class U, class F>
    requires(VoidOrEq<T, U> && ::sus::cmp::Eq<E, F>)
  friend constexpr bool operator==(const Result& l,
                                   const Result<U, F>& r) noexcept {
    return eq(l, r);
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
    return cmp<std::strong_ordering>(l, r);
  }
  template <class U, class F>
    requires(VoidOrOrd<T, U> && ::sus::cmp::StrongOrd<E, F>)
  friend constexpr std::strong_ordering operator<=>(
      const Result& l, const Result<U, F>& r) noexcept {
    return cmp<std::strong_ordering>(l, r);
  }

  // sus::cmp::Ord<Result<T, E>> trait.
  friend constexpr std::weak_ordering operator<=>(const Result& l,
                                                  const Result& r) noexcept
    requires((!VoidOrOrd<T> || !::sus::cmp::StrongOrd<E>) && VoidOrWeakOrd<T> &&
             ::sus::cmp::Ord<E>)
  {
    return cmp<std::weak_ordering>(l, r);
  }
  template <class U, class F>
    requires((!VoidOrOrd<T, U> || !::sus::cmp::StrongOrd<E, F>) &&
             VoidOrWeakOrd<T, U> && ::sus::cmp::Ord<E, F>)
  friend constexpr std::weak_ordering operator<=>(
      const Result& l, const Result<U, F>& r) noexcept {
    return cmp<std::weak_ordering>(l, r);
  }

  // sus::cmp::PartialOrd<Result<T, E>> trait.
  friend constexpr std::partial_ordering operator<=>(const Result& l,
                                                     const Result& r) noexcept
    requires((!VoidOrWeakOrd<T> || !::sus::cmp::Ord<E>) &&
             VoidOrPartialOrd<T> && ::sus::cmp::PartialOrd<E>)
  {
    return cmp<std::partial_ordering>(l, r);
  }
  template <class U, class F>
    requires((!VoidOrWeakOrd<T, U> || !::sus::cmp::Ord<E, F>) &&
             VoidOrPartialOrd<T, U> && ::sus::cmp::PartialOrd<E, F>)
  friend constexpr std::partial_ordering operator<=>(
      const Result& l, const Result<U, F>& r) noexcept {
    return cmp<std::partial_ordering>(l, r);
  }

  template <class U, class F>
    requires(!VoidOrPartialOrd<T, U> || !::sus::cmp::PartialOrd<E, F>)
  friend constexpr auto operator<=>(const Result& l,
                                    const Result<U, F>& r) noexcept = delete;

 private:
  template <class U, class V>
  friend class Result;

  // Since `T` may be a reference or a value type, this constructs the correct
  // storage type for `T`.
  template <class U>
    requires(!std::is_void_v<U>)
  static constexpr inline decltype(auto) make_ok_storage(U&& t) {
    if constexpr (std::is_reference_v<T>)
      return StoragePointer<T>(t);
    else
      return ::sus::forward<U>(t);
  }

  template <class U, class F>
  static constexpr bool eq(const Result& l, const Result<U, F>& r) noexcept {
    sus_check(!l.storage_.is_moved() && !r.storage_.is_moved());
    if (l.storage_.is_ok()) {
      if constexpr (std::is_void_v<T>) {
        return r.storage_.is_ok();
      } else if (r.storage_.is_ok()) {
        return l.storage_.template get_ok<T>() ==
               r.storage_.template get_ok<U>();
      } else {
        return false;
      }
    } else {
      if (r.storage_.is_err()) {
        return l.storage_.get_err() == r.storage_.get_err();
      } else {
        return false;
      }
    }
  }

  template <::sus::cmp::Ordering O, class U, class F>
  static constexpr O cmp(const Result& l, const Result<U, F>& r) noexcept {
    sus_check(!l.storage_.is_moved() && !r.storage_.is_moved());
    if (l.storage_.is_ok()) {
      if (r.storage_.is_ok()) {
        if constexpr (std::is_void_v<T>) {
          return O::equivalent;
        } else {
          return l.storage_.template get_ok<T>() <=>
                 r.storage_.template get_ok<U>();
        }
      } else {
        return O::greater;
      }
    } else {
      if (r.storage_.is_err()) {
        return l.storage_.get_err() <=> r.storage_.get_err();
      } else {
        return O::less;
      }
    }
  }

  // Constructors for `Ok`.
  enum WithOk { WITH_OK };
  constexpr inline Result(WithOk) noexcept
    requires(std::is_void_v<T>)
      : storage_(__private::WITH_T) {}
  template <class U>
  constexpr inline Result(WithOk, U&& u) noexcept
    requires(!std::is_void_v<T>)
      : storage_(__private::WITH_T, ::sus::forward<U>(u)) {}
  // Constructors for `Err`.
  enum WithErr { WITH_ERR };
  template <class F>
  constexpr inline Result(WithErr, F&& f) noexcept
      : storage_(__private::WITH_E, ::sus::forward<F>(f)) {}

  using OkStorageType =
      std::conditional_t<std::is_reference_v<T>, StoragePointer<T>, T>;
  using Storage = std::conditional_t<  //
      std::is_void_v<T>,               //
      __private::StorageVoid<E>,       //
      __private::StorageNonVoid<OkStorageType, E>>;
  [[no_unique_address]] Storage storage_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(storage_));
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
_sus_format_to_stream(sus::result, Result, T, E);

namespace sus {
using ::sus::result::Err;
using ::sus::result::err;
using ::sus::result::Ok;
using ::sus::result::ok;
using ::sus::result::Result;
}  // namespace sus
