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

#include <concepts>

#include "subspace/mem/forward.h"

namespace sus::fn {
struct Anything;
}

namespace sus::fn::__private {

/// The return type inferred for a functor when it is not able to be called with
/// a set of argument types, which indicates an error occured.
struct NoOverloadMatchesArguments {};

/// Always fails, used to generate a static_assert() when a type is meant to be
/// a function signature but it's ill-formed.
template <class T>
concept InvalidFunctionSignature = false;

/// Represents the argument types that will be passed to a functor.
template <class... Ts>
struct ArgsPack;

/// Unpacks a function signature `ReturnType(Args...)` into its components.
///
/// Generates a static_assert() failure if the `T` in `Sig<T>` is not a function
/// signature.
template <class R, class... A>
struct Sig {
  static_assert(
      InvalidFunctionSignature<R>,
      "expected a function signature of the form `ReturnType(Args...)`");
};

template <class R, class... A>
struct Sig<R(A...)> {
  using Return = R;
  using Args = ArgsPack<A...>;
};

/// Unpacks an `ArgsPack` of function argument types, and determines if a
/// functor `F` is once-callable with them, to be called through a `FnOnce`.
///
/// If `F` is callable with the argument types in the `ArgsPack`, the
/// `returns()` method will have the same return type as `F`. Otherwise, it
/// indicates failure by having a return type of `NoOverloadMatchesArguments`.
template <class F, class ArgsPack>
struct InvokedFnOnce {
  static NoOverloadMatchesArguments returns();
};

template <class F, class... Ts>
  requires requires(F&& f) {
    { ::sus::forward<F>(f)(std::declval<Ts>()...) };
  }
struct InvokedFnOnce<F, ArgsPack<Ts...>> {
  static decltype(std::declval<F&&>()(std::declval<Ts>()...)) returns();
};

/// Unpacks an `ArgsPack` of function argument types, and determines if a
/// functor `F` is mutably-callable with them, to be called through a `FnMut`.
///
/// If `F` is callable with the argument types in the `ArgsPack`, the
/// `returns()` method will have the same return type as `F`. Otherwise, it
/// indicates failure by having a return type of `NoOverloadMatchesArguments`.
template <class F, class ArgsPack>
struct InvokedFnMut {
  static NoOverloadMatchesArguments returns();
};

template <class F, class... Ts>
  requires requires(F f) {
    { f(std::declval<Ts>()...) };
  }
struct InvokedFnMut<F, ArgsPack<Ts...>> {
  static decltype(std::declval<F>()(std::declval<Ts>()...)) returns();
};

/// Unpacks an `ArgsPack` of function argument types, and determines if a
/// functor `F` is const-callable with them, to be called through a `Fn`.
///
/// If `F` is callable with the argument types in the `ArgsPack`, the
/// `returns()` method will have the same return type as `F`. Otherwise, it
/// indicates failure by having a return type of `NoOverloadMatchesArguments`.
template <class F, class ArgsPack>
struct InvokedFn {
  static NoOverloadMatchesArguments returns();
};

template <class F, class... Ts>
  requires requires(const F& f) {
    { f(std::declval<Ts>()...) };
  }
struct InvokedFn<F, ArgsPack<Ts...>> {
  static decltype(std::declval<const F&>()(std::declval<Ts>()...)) returns();
};

/// Whether the `ReturnType` of a functor is compatible with receiving it as
/// `T`.
///
/// If the receiver specifies `T` as `sus::fn::Anything` then all return types
/// are accepted, which can be useful in generic code. Simiarly, if the receiver
/// specifies `T` as `sus::fn::NonVoid` then all return types other than `void`
/// are accepted.
template <class ReturnType, class T>
concept ValidReturnType =
    !std::same_as<ReturnType, NoOverloadMatchesArguments> &&
    (std::same_as<::sus::fn::Anything, T> ||
     std::convertible_to<ReturnType, T>);

}  // namespace sus::fn::__private
