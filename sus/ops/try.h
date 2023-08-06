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

#include "sus/mem/move.h"

namespace sus::ops {

/// Specializations of this struct are used to implement the Try concept.
template <class T>
struct TryImpl;

/// A concept for types that can indicate success and failure.
///
/// The Try concept is implemented by specializing `TryImpl` for a type `T`
/// with:
/// * A type `Output` that is the unwrapped success value type.
/// * A member `static bool is_success(const T& t)` that reports if the
///   given `t` is a success or failure value.
/// * A member `static Output into_output(T t)` that unwraps a successful T to
///   its success value.
/// * A member `static T from_output(Output t)` that constructs a successful T
///   from a success value.
///
/// Note that when the `Output` type is `void` then `from_output()` will not be
/// useable. To support `void`, the `TryDefault` concept can be used instead of
/// `Try`. The `TryDefault` concept allows construction of the `Try` type with a
/// default success value which can be `void`.
template <class T>
concept Try =
    requires {
      // The Try type is not a reference, conversions require concrete types.
      requires !std::is_reference_v<T>;
      // The Output type can be converted to/from the Try type.
      typename TryImpl<T>::Output;
      // The Output type is also not a reference.
      requires !std::is_reference_v<typename TryImpl<T>::Output>;
    }   //
    &&  //
    requires(const T& t, T&& tt) {
      // is_success() reports if the Try type is in a success state.
      { TryImpl<T>::is_success(t) } -> std::same_as<bool>;
      // into_output() unwraps from the Try type to its success type. It may
      // assume that the input is in a success state (`is_success()` would
      // return true), as the `try_into_output()` method verifies this.
      {
        TryImpl<T>::into_output(::sus::move(tt))
      } -> std::same_as<typename TryImpl<T>::Output>;
    }   //
    &&  //
    // from_output() is not needed (or possible) for void Output types. To
    // construct a `T` with a void Output type, require `TryDefault` instead and
    // use `from_default()`.
    (std::is_void_v<typename TryImpl<T>::Output> ||  //
     requires(typename TryImpl<T>::Output output) {
       // from_output() converts a success type to the Try type.
       { TryImpl<T>::from_output(::sus::move(output)) } -> std::same_as<T>;
     });

/// Identifies Try types which can be constructed with a default success value.
///
/// This takes the place of the void type in Rust types, such as `Option<()>`
/// and `Result<(), E>` as `void` is not a constructible type in C++. But by
/// satisfying `TryDefault`, `Result<void, E>` can be constructed with a default
/// success value of nothing.
template <class T>
concept TryDefault = Try<T> && requires {
  // from_default() construct the Try type with the default value for its
  // success type.
  { TryImpl<T>::from_default() } -> std::same_as<T>;
};

/// Determines if a type `T` that satisfies `Try` represents success in its
/// current state.
template <Try T>
constexpr inline bool try_is_success(const T& t) noexcept {
  return TryImpl<T>::is_success(t);
}

/// Unwraps from the Try type that is currently in its success state
/// (`is_success()` would return true) to produce its success value.
///
/// For instance, this unwraps an `Result<T, E>` which can represent success or
/// failure into `T` which represents only success in the type system.
///
/// # Panics
///
/// If the input is not in a success state (`is_success()` would return true)
/// the function will panic.
template <Try T>
  requires(::sus::mem::IsMoveRef<T &&>)
constexpr inline TryImpl<T>::Output try_into_output(T&& t) noexcept {
  ::sus::check(TryImpl<T>::is_success(t));
  return TryImpl<T>::into_output(::sus::move(t));
}

/// Constructs an object of type `T` that satisfies `Try` from a value that
/// represents success for `T`.
///
/// For instance, this constructs a `Result<T, E>` from a `T` since `Result`
/// satisfies `Try` and `T` is the type that represents its success values.
///
/// The template variable `T` must be specified as it can not be deduced here.
/// For example: `sus::ops::try_from_output<Result<T, E>>(T())`.
///
/// # Void success values
///
/// The `Output` type of `Try<T>` can not be void. To construct a type that has
/// an output of `void`, require `T` to be `TryDefault` and use
/// `try_from_default()`.
template <Try T>
  requires(!std::is_void_v<typename TryImpl<T>::Output>)
constexpr inline T try_from_output(typename TryImpl<T>::Output&& t) noexcept {
  return TryImpl<T>::from_output(::sus::move(t));
}

/// Constructs an object of type `T` that satisfies `TryDefault` (and `Try`)
/// with its default success value.
///
/// The template variable `T` must be specified as it can not be deduced here.
/// For example: `sus::ops::try_from_default<Result<void, E>>()`.
///
/// The default success value is specified by the type, but is typically the
/// success state containing the default constructed value of the inner type,
/// such as `Some(0_i32)` for `Option<i32>`.
template <TryDefault T>
constexpr inline T try_from_default() noexcept {
  return TryImpl<T>::from_default();
}

}  // namespace sus::ops
