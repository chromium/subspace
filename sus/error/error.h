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

#include "fmt/core.h"
#include "sus/macros/lifetimebound.h"
#include "sus/option/option.h"

namespace sus {
/// Interfaces for working with Errors.
///
/// # Error Handling With Subspace
/// The Subspace C++ library provides two complementary systems for
/// constructing/representing, reporting, propagating, and reacting to errors.
/// These responsibilities are collectively known as “error handling.” The
/// components of the first system, the panic handling interfaces, are most
/// commonly used to represent bugs that have been detected in your program. The
/// components of the second system,
/// [`Result`]($sus::result::Result), the [`Error`]($sus::error::Error) concept,
/// and user defined types, are used to represent anticipated runtime failure
/// modes of your program.
///
/// Unlike [exceptions](https://en.cppreference.com/w/cpp/error/exception), the
/// error handling facilities here are explicit at each layer of a call stack,
/// with a convenient way to generically propagate errors of any type without
/// templates.
///
/// ## The Panic Interfaces
/// The following are the primary interfaces of the panic system and the
/// responsibilities they cover:
///
/// * [`panic`]($sus::assertions::panic) (Constructing, Propagating)
/// * [`SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER`]($sus::assertions::panic)
///   (Reporting)
/// * [`SUS_PROVIDE_PANIC_HANDLER`]($sus::assertions::panic)
///   (Reacting)
///
/// The following are the primary interfaces of the error system and the
/// responsibilities they cover:
///
/// * [`Result`]($sus::result::Result) (Propagating, Reacting)
/// * The [`Error`]($sus::error::Error) concept (Reporting)
/// * The [`DynError`]($sus::error::DynError) type (Reporting)
/// * User defined types (Constructing / Representing)
/// * [`downcast`]($sus::boxed::Box::downcast) (Reacting)
/// * TODO: The [`TRY`](https://github.com/chromium/subspace/issues/299) macro
///   (Propagating)
/// * The [`Try`]($sus::ops::Try) concept (Propagating, Constructing)
///
/// ## Converting Errors into Panics
/// The panic and error systems are not entirely distinct. Often times errors
/// that are anticipated runtime failures in an API might instead represent bugs
/// to a caller. For these situations the Subspace C++ library provides APIs for
/// constructing panics with an [`Error`]($sus::error::Error) as it's source.
///
/// * [`Result::unwrap`]($sus::result::Result::unwrap)
/// * [`Result::expect`]($sus::result::Result::expect)
///
/// These functions are equivalent, they either return the inner value if the
/// `Result` is `Ok` or panic if the `Result` is `Err` printing the inner error
/// as the source. The only difference between them is that with `expect` you
/// provide a panic error message to be printed alongside the source, whereas
/// unwrap has a default message indicating only that you unwrapped an `Err`.
///
/// Of the two, `expect` is generally preferred since its `msg` field allows you
/// to convey your intent and assumptions which makes tracking down the source
/// of a panic easier. `unwrap` on the other hand can still be a good fit in
/// situations where you can trivially show that a piece of code will never
/// panic, such as `u32::try_from(404).unwrap()` or early prototyping.
///
/// ## Common Message Styles
/// There are two common styles for how people word expect messages. Using the
/// message to present information to users encountering a panic (“expect as
/// error message”) or using the message to present information to developers
/// debugging the panic (“expect as precondition”).
///
/// In the former case the expect message is used to describe the error that has
/// occurred which is considered a bug. Consider the following example with an
/// `auto read_env_var(std::string_view) -> Result<std::string, Box<DynError>>`
/// function:
///
/// ```
/// // Read environment variable, panic if it is not present
/// auto path = sus::env::var("IMPORTANT_PATH").unwrap();
/// ```
///
/// In the “expect as error message” style we would use expect to describe that
/// the environment variable was not set when it should have been:
///
/// ```
/// auto path = sus::env::var("IMPORTANT_PATH")
///     .expect("env variable `IMPORTANT_PATH` is not set");
/// In the “expect as precondition” style, we would instead describe the reason
/// we expect the Result should be Ok. With this style we would prefer to write:
/// ```
/// ```
/// auto path = sus::env::var("IMPORTANT_PATH")
///     .expect("env variable `IMPORTANT_PATH` should be set by "
///             "`wrapper_script.sh`");
/// ```
/// The “expect as error message” style does not work as well with the default
/// output of the std panic hooks, and often ends up repeating information that
/// is already communicated by the source error being unwrapped:
///
/// ```
/// PANIC! at 'env variable `IMPORTANT_PATH` is not set: NotFound',
/// sus/result/result.h:792:11
/// ```
///
/// In this example we end up mentioning that an
/// env variable is not set, followed by our source message that says the env is
/// not present, the only additional information we’re communicating is the name
/// of the environment variable being checked.
///
/// The “expect as precondition” style instead focuses on source code
/// readability, making it easier to understand what must have gone wrong in
/// situations where panics are being used to represent bugs exclusively. Also,
/// by framing our expect in terms of what “SHOULD” have happened to prevent the
/// source error, we end up introducing new information that is independent from
/// our source error.
///
/// ```
/// PANIC! at 'env variable `IMPORTANT_PATH` should be set by
/// `wrapper_script.sh`: NotPresent', sus/result/result.h:792:11
/// ```
///
/// In this example we are communicating not only the name of the environment
/// variable that should have been set, but also an explanation for why it
/// should have been set, and we let the source error display as a clear
/// contradiction to our expectation.
///
/// *Hint*: If you’re having trouble remembering how to phrase
/// expect-as-precondition style error messages remember to focus on the word
/// “should” as in “env variable should be set by blah” or “the given binary
/// should be available and executable by the current user”.
namespace error {}
}  // namespace sus

namespace sus::error {
struct DynError;

template <class T>
struct ErrorImpl {};

/// [`Error`]($sus::error::Error) is a concept representing the basic
/// expectations for error values, i.e., values of type `E` in
/// [`Result<T, E>`]($sus::result::Result).
///
/// Errors must describe themselves through a
/// [`display`]($sus::error::DynError::display) method. Error messages are
/// typically concise lowercase sentences without trailing punctuation:
///
/// ```
/// auto err = u32::try_from(-1).unwrap_err();
/// sus::check(fmt::to_string(err) == "out of bounds");
/// ```
///
/// # Implementing Error
/// To make an [`Error`]($sus::error::Error) type, specialize `ErrorImpl<T>`
/// for the error type `T` and implement the required methods:
///
/// * `static auto display(const T&) -> std::string`: An error message
///   describing the error. fmtlib support is provided for all error types
///   through a blanket implementation that uses `display`. Error messages are
///   typically concise lowercase sentences without trailing punctuation:
///   ```
///   auto err = u32::try_from(-1).unwrap_err();
///   sus::check(fmt::to_string(err) == "out of bounds");
///   ```
///
/// The following methods may optionally also be provided:
/// * `static auto source(const T&) -> sus::Option<sus::error::DynError>`:
///   Optional information about the cause of the error. A simple
///   implementation would just return [`sus::none()`]($sus::option::none).
///
///   `source` is generally used when errors cross "abstraction boundaries".
///   If one module must report an error that is caused by an error from a
///   lower-level module, it can allow accessing that error via
///   `source`. This makes it possible for the high-level module to provide
///   its own errors while also revealing some of the implementation for
///   debugging.
///
///   The [`Error`]($sus::error::Error) object returned by `source` must be
///   type-erased as an [`DynError`]($sus::error::DynError). This is typically
///   done by putting the error in [`Box<DynError>`]($sus::boxed::Box) via
///   the [`from`]($sus::boxed::Box::from#from.error) constructor method
///   satisfying [`From<Error>`]($sus::construct::From) for
///   [`Box`]($sus::boxed::Box).
///
/// # Using Error
/// To use an [`Error`]($sus::error::Error) type, use:
/// * [`error_display`]($sus::error::error_display) to get the string
///   description of the error.
/// * [`error_source`]($sus::error::error_source) to get the next level deeper
///   error for debugging.
///
/// Avoid instantiating `ErrorImpl<T>` yourself to call the static methods.
///
/// # Type erasure
/// The [`Error`]($sus::error::Error) concept supports type-erasure so that it
/// is possible to work with a type satisfying [`Error`]($sus::error::Error)
/// without requiring a template that knows the precise type. This allows
/// [`Error`]($sus::error::Error) types to be used in virtual method
/// signatures or dynamic library ABIs. The
/// [`DynError`]($sus::error::DynError) represents a type-erased error, and it
/// is typically created by moving the error object into a
/// [`Box<DynError>`]($sus::boxed::Box) via
/// [`sus::into(error)`]($sus::construct::into) or
/// [`Box::from`]($sus::boxed::Box::from#from.error).
///
/// Note that both [`DynError`]($sus::error::DynError) and
/// [`Box<DynError>`]($sus::boxed::Box) satisfy the
/// [`Error`]($sus::error::Error) concept.
///
/// Internally, [`Box`]($sus::boxed::Box) puts the error object into an
/// [`DynErrorTyped`]($sus::error::DynErrorTyped) on the heap and then casts
/// the pointer to the [`DynError`]($sus::error::DynError) base class to
/// perform the type erasure.
///
/// This is similar to `&dyn Error` when working with the Rust
/// [`Error`](https://doc.rust-lang.org/stable/std/error/trait.Error.html)
/// trait.
///
/// # Examples
/// An [enum](https://en.cppreference.com/w/cpp/language/enum) error type:
/// ```
/// enum class ErrorReason {
///   SomeReason
/// };
///
/// template <>
/// struct sus::error::ErrorImpl<ErrorReason>  {
///   constexpr static std::string display(const ErrorReason& self) noexcept {
///     switch (self) {
///       case ErrorReason::SomeReason: return "we saw SomeReason happen";
///     }
///     sus::unreachable();
///   }
/// };
///
/// static_assert(sus::error::error_display(ErrorReason::SomeReason) ==
///               "we saw SomeReason happen");
/// ```
///
/// An struct error type, which is backed by a string:
/// ```
/// struct ErrorString final {
///   std::string reason;
/// };
///
/// template <>
/// struct sus::error::ErrorImpl<ErrorString>  {
///   constexpr static std::string display(const ErrorString& self) noexcept {
///     return sus::clone(self.reason);
///   }
/// };
///
/// static_assert(sus::error::error_display(ErrorString("oops")) == "oops");
/// ```
///
/// An example function that returns a
/// [`Result<void, Box<DynError>>`]($sus::result::Result), allowing
/// it to return any error type:
/// ```
/// auto f = [](i32 i) -> sus::result::Result<void,
/// sus::Box<sus::error::DynError>> {
///   if (i > 10) return sus::err(sus::into(ErrorReason::SomeReason));
///   if (i < -10) return sus::err(sus::into(ErrorString("too low")));
///   return sus::ok();
/// };
///
/// sus::check(fmt::format("{}", f(20)) == "Err(we saw SomeReason happen)");
/// sus::check(fmt::format("{}", f(-20)) == "Err(too low)");
/// sus::check(fmt::format("{}", f(0)) == "Ok(<void>)");
/// ```
///
/// An example error that reports a `source`:
/// ```
/// struct SuperErrorSideKick {};
///
/// struct SuperError {
///   sus::Box<sus::error::DynError> source;
/// };
///
/// template <>
/// struct sus::error::ErrorImpl<SuperError> {
///   constexpr static std::string display(const SuperError&) noexcept {
///     return "SuperError is here!";
///   }
///   constexpr static sus::Option<const DynError&> source(
///       const SuperError& self) noexcept {
///     return sus::some(*self.source);
///   }
/// };
/// static_assert(sus::error::Error<SuperError>);
///
/// template <>
/// struct sus::error::ErrorImpl<SuperErrorSideKick> {
///   constexpr static std::string display(const SuperErrorSideKick&) noexcept
///   {
///     return "SuperErrorSideKick is here!";
///   }
/// };
/// static_assert(sus::error::Error<SuperErrorSideKick>);
///
/// auto get_super_error = []() -> sus::result::Result<void, SuperError> {
///   return sus::err(SuperError{.source = sus::into(SuperErrorSideKick())});
/// };
///
/// if (auto r = get_super_error(); r.is_err()) {
///   auto& e = r.as_err();
///   sus::check(fmt::format("Error: {}", e) == "Error: SuperError is here!");
///   sus::check(
///       fmt::format("Caused by: {}", sus::error::error_source(e).unwrap())
///       == "Caused by: SuperErrorSideKick is here!");
/// }
/// ```
template <class T>
concept Error = requires(const T& t) {
  // Required methods.
  { ErrorImpl<T>::display(t) } -> std::same_as<std::string>;
  // Optional methods.
  requires requires {
    { ErrorImpl<T>::source(t) } -> std::same_as<sus::Option<const DynError&>>;
  } || !requires {
    { ErrorImpl<T>::source(t) };
  };
};

/// A type-erased [`Error`]($sus::error::Error) object.
///
/// Using this allows the error type to be placed in heap-allocated smart
/// pointers without templates, and thus without knowing the concrete type.
/// For example a `void foo(Box<DynError>)` function can work with any
/// [`Error`]($sus::error::Error) type but does not need to be templated. This
/// allows the function to be virtual, to reduce complilation time/binary
/// size, or to provide dynamically linked library ABI.
///
/// Since it is type erased, it must only be referred to by reference/pointer,
/// and it can not be moved similar to
/// [`Pin<T>`](https://doc.rust-lang.org/std/pin/struct.Pin.html) types in
/// Rust.
struct DynError {
  /// Forwards to the [`Error`]($sus::error::Error) implementation of `E`.
  virtual std::string display() const noexcept = 0;
  /// Forwards to the [`Error`]($sus::error::Error) implementation of `E`.
  virtual sus::Option<const DynError&> source() const noexcept = 0;

  constexpr DynError() = default;
  constexpr virtual ~DynError() = default;
  DynError(DynError&&) = delete;
  DynError&& operator=(DynError&&) = delete;
};

/// Gets a string describing the `error` from an [`Error`]($sus::error::Error)
/// object.
template <Error E>
constexpr inline std::string error_display(const E& error) noexcept {
  return ErrorImpl<E>::display(error);
}

/// Gets a the source [`Error`]($sus::error::Error), type-erased as
/// [`DynError`]($sus::error::DynError), which caused the `error` to occur.
template <Error E>
  requires requires(const E& error) {
    {
      ErrorImpl<E>::source(error)
    } -> std::same_as<sus::Option<const DynError&>>;
  }
constexpr inline sus::Option<const DynError&> error_source(
    const E& error sus_lifetimebound) noexcept {
  return ErrorImpl<E>::source(error);
}

template <Error E>
  requires(!requires(const E& e) {
    { ErrorImpl<E>::source(e) };
  })
constexpr inline sus::Option<const DynError&> error_source(
    const E& sus_lifetimebound) noexcept {
  return sus::Option<const DynError&>();
}

/// The wrapper around an [`Error`]($sus::error::Error) object that allows it
/// to be type-erased as [`DynError`]($sus::error::DynError).
template <class E>
struct DynErrorTyped : public DynError {
  std::string display() const noexcept override {
    return error_display(error_);
  }
  sus::Option<const DynError&> source() const noexcept override {
    return error_source(error_);
  }

  constexpr DynErrorTyped(E&& error) : error_(::sus::move(error)) {}
  constexpr ~DynErrorTyped() override = default;

  /// Unwraps and returns the inner error type `E`, discarding the
  /// `DynErrorTyped`.
  constexpr E into_inner() && noexcept { return ::sus::move(error_); }

 private:
  E error_;
};

}  // namespace sus::error

// Type-erased [`DynError`]($sus::error::DynError) satisfies
// [`Error`]($sus::error::Error).
template <>
struct sus::error::ErrorImpl<::sus::error::DynError> {
  constexpr static std::string display(
      const ::sus::error::DynError& b) noexcept {
    return b.display();
  }
  constexpr static sus::Option<const DynError&> source(
      const ::sus::error::DynError& b sus_lifetimebound) noexcept {
    return b.source();
  }
};

// A blanket implementation of fmt::formatter for all types that satisfy
// [`Error`]($sus::error::Error).
template <::sus::error::Error E, class Char>
struct fmt::formatter<E, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(const E& e, FormatContext& ctx) const {
    auto out = ctx.out();
    return fmt::format_to(out, "{}", ::sus::error::error_display(e));
  }
};
