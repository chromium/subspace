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

#include <concepts>

#include "subspace/mem/forward.h"
#include "subspace/mem/move.h"

namespace sus::fn {

struct NonVoid {
  template <class T>
  constexpr NonVoid(T&&) noexcept {}
};

struct Anything {
  template <class T>
  constexpr Anything(T&&) noexcept {}
};

namespace __private {
template <class... Ts>
struct Pack;

template <class R, class... Args>
struct Sig;

template <class R, class... A>
struct Sig<R(A...)> {
  using Return = R;
  using Args = Pack<A...>;
};

struct NoOverloadMatchesArguments {};

template <class F, class ArgsPack>
struct InvokedFnOnce {
  constexpr static NoOverloadMatchesArguments return_type();
};

template <class F, class... Ts>
  requires requires(F&& f) {
    { ::sus::move(f)(std::declval<Ts>()...) };
  }
struct InvokedFnOnce<F, Pack<Ts...>> {
  constexpr static decltype(std::declval<F&&>()(std::declval<Ts>()...))
  return_type();
};

template <class F, class ArgsPack>
struct InvokedFnMut {
  constexpr static NoOverloadMatchesArguments return_type();
};

template <class F, class... Ts>
  requires requires(F& f) {
    { f(std::declval<Ts>()...) };
  }
struct InvokedFnMut<F, Pack<Ts...>> {
  constexpr static decltype(std::declval<F&>()(std::declval<Ts>()...))
  return_type();
};

template <class F, class ArgsPack>
struct InvokedFn {
  constexpr static NoOverloadMatchesArguments return_type();
};

template <class F, class... Ts>
  requires requires(const F& f) {
    { f(std::declval<Ts>()...) };
  }
struct InvokedFn<F, Pack<Ts...>> {
  constexpr static decltype(std::declval<const F&>()(std::declval<Ts>()...))
  return_type();
};

template <class ReturnType, class T>
concept ValidReturnType =
    !std::same_as<ReturnType, NoOverloadMatchesArguments> &&
    (std::same_as<::sus::fn::Anything, T> ||
     std::convertible_to<ReturnType, T>);
}  // namespace __private

/// The version of a callable object that is called on an rvalue (moved-from)
/// receiver. A `FnOnce` is typically the best fit for any callable that will
/// only be called at most once. However when a template (or constexpr) is not
/// required, receiving a `FnOnceRef` instead of `FnOnce auto&&` will typically
/// produce less code by using type erasure to avoid template instantiation.
///
/// A type that satisfies `FnOnce` will return a type that can be converted to
/// `R` when called with the arguments `Args...`. `FnOnce` is satisfied by
/// being callable as an rvalue (which is done by providing an operator() that
/// is not `&`-qualified). Mutable and const lambdas will satisfy
/// `FnOnce`.
///
/// The second argument of `FnOnce<F, S>` is a function signature with the
/// format `ReturnType(Args...)`, where `Args...` are the arguments that will
/// be passed to the `FnOnce` and `ReturnType` is what is expected to be
/// received back. It would appear as a matching concept as:
/// ```
/// void function(FnOnce<ReturnType(Args...)> auto&& f) { ... }
/// ```
///
/// # Use of `FnOnce`
/// A `FnOnce` should only be called once, and should be moved with
/// `sus::move()` when calling it.  It is typically received as an rvalue
/// reference to avoid an unnecessary copy or move operation.
///
/// Calling it multiple times may panic or cause Undefined Behaviour. Not moving
/// the `FnOnce` when calling it may fail to compile, panic, or cause Undefined
/// Behaviour depending on the type that is being used to satisfy `FnOnce`.
///
/// # Compatibility
/// Any callable type that satisfies `Fn` or `FnMut` will also satisfy `FnOnce`.
/// While a `FnOnce` should only be called once, this is compatible with the
/// requirements of `FnMut` and `Fn` which can be called only a single time. As
/// well, `FnOnce` is allowed to mutate internal state, but it does not have to,
/// which is compatible with the const nature of `Fn`.
///
/// # Subspace types that satisfy `FnOnce`
/// The `FnOnceRef` and `FnOnceBox` types in the Subspace library satisfy
/// `FnOnce`. They must be moved from when called, and they panic if called more
/// than once, as that implies a use-after-move.
///
/// Like the concepts, `FnRef` is convertible to `FnMutRef` is convertible to
/// `FnOnceRef`. And `FnBox` is convertible to `FnMutBox` is convertible to
/// `FnOnceBox`.
///
/// # Example
/// ```
/// // Accepts any type that can be called once with (Option<i32>) and returns
/// // i32.
/// i32 call_once(sus::fn::FnOnce<i32(sus::Option<i32>)> auto&& f) {
///   return sus::move(f)(sus::some(400));  // Returns an i32.
/// }
///
/// i32 x = call_once([](sus::Option<i32> o) -> i32 {
///   return sus::move(o).unwrap_or_default() + 4;
/// });
///  sus::check(x == 400 + 4);
/// ```
template <class F, class S>
concept FnOnce = requires(F&& f) {
  {
    __private::InvokedFnOnce<F, typename __private::Sig<S>::Args>::return_type()
  } -> __private::ValidReturnType<typename __private::Sig<S>::Return>;
};

/// The version of a callable object that is allowed to mutate internal state
/// and may be called multiple times. A `FnMut` is typically the best fit for
/// any callable that may be called one or more times. However when a template
/// (or constexpr) is not required, receiving a `FnMutRef` instead of `FnMut
/// auto` will typically produce less code by using type erasure to avoid
/// template instantiation.
///
/// Because a `FnMut` is able to mutate internal state, it may return different
/// values each time it is called with the same inputs.
///
/// A type that satisfies `FnMut` will return a type that can be converted to
/// `R` when called with the arguments `Args...`. `FnMut` is satisfied by
/// being callable as an lvalue (which is done by providing an operator() that
/// is not `&&`-qualified). Mutable and const lambdas will satisfy
/// `FnMut`.
///
/// The second argument of `FnMut<F, S>` is a function signature with the
/// format `ReturnType(Args...)`, where `Args...` are the arguments that will
/// be passed to the `FnMut` and `ReturnType` is what is expected to be
/// received back. It would appear as a matching concept as:
/// ```
/// void function(FnMut<ReturnType(Args...)> auto f) { ... }
/// ```
///
/// # Use of `FnMut`
/// A `FnMut` may be called any number of times, unlike `FnOnce`, and need not
/// be moved when called. It is typically received as a function parameter by
/// value, which isolates any internal mutation to the current function.
///
/// # Compatibility
/// Any callable type that satisfies `Fn` or `FnMut` will also satisfy `FnOnce`.
/// While a `FnOnce` should only be called once, this is compatible with the
/// requirements of `FnMut` and `Fn` which can be called only a single time. As
/// well, `FnOnce` is allowed to mutate internal state, but it does not have to,
/// which is compatible with the const nature of `Fn`.
///
/// # Subspace types that satisfy `FnMut`
/// The `FnMutRef` and `FnMutBox` types in the Subspace library satisfy `FnMut`
/// (and thus `FnOnce` as well). They may be called any number of times and are
/// able to mutate internal state.
///
/// Like the concepts, `FnRef` is convertible to `FnMutRef` is convertible to
/// `FnOnceRef`. And `FnBox` is convertible to `FnMutBox` is convertible to
/// `FnOnceBox`.
///
/// # Example
/// ```
/// // Accepts any type that can be called once with (Option<i32>) and returns
/// // i32.
/// static i32 call_mut(sus::fn::FnMut<i32(sus::Option<i32>)> auto f) {
///   return f(sus::some(400)) + f(sus::some(100));  // Returns an i32.
/// }
///
/// i32 x = call_mut([i = 0_i32](sus::Option<i32> o) mutable -> i32 {
///   i += 1;
///   return sus::move(o).unwrap_or_default() + i;
/// });
/// sus::check(x == 401 + 102);
/// ```
template <class F, class S>
concept FnMut = requires(F& f) {
  {
    __private::InvokedFnMut<F, typename __private::Sig<S>::Args>::return_type()
  } -> __private::ValidReturnType<typename __private::Sig<S>::Return>;

  requires FnOnce<F, S>;
};

/// The version of a callable object that may be called multiple times without
/// mutating internal state. A `Fn` is useful for a callable that is received as
/// a const reference to indicate it and may be called one or more times and
/// does not change between call. However when a template (or constexpr) is not
/// required, receiving a `FnRef` instead of `const Fn auto&` will typically
/// produce less code by using type erasure to avoid template instantiation.
///
/// Because a `FnMut` is able to mutate internal state, it may return different
/// values each time it is called with the same inputs.
///
/// A type that satisfies `FnMut` will return a type that can be converted to
/// `R` when called with the arguments `Args...`. `FnMut` is satisfied by
/// being callable as an lvalue (which is done by providing an operator() that
/// is not `&&`-qualified). Mutable and const lambdas will satisfy
/// `FnMut`.
///
/// The second argument of `Fn<F, S>` is a function signature with the format
/// `ReturnType(Args...)`, where `Args...` are the arguments that will be passed
/// to the `Fn` and `ReturnType` is what is expected to be received back. It
/// would appear as a matching concept as:
/// ```
/// void function(const Fn<ReturnType(Args...)> auto& f) { ... }
/// ```
///
/// # Use of `Fn`
/// A `Fn` may be called any number of times, unlike `FnOnce`, and need not
/// be moved when called. It is typically received as a function parameter as a
/// const reference, which ensures a non-mutating call operator will be used.
///
/// # Compatibility
/// Any callable type that satisfies `Fn` will also satisfy `FnMut` and
/// `FnOnce`. A `Fn` may be called multiple times, or a single time, which is
/// compatible with both `FnMut` and `FnOnce`. And while `FnMut` and `FnOnce`
/// are able to mutate state when run, they are not required to and a constant
/// `Fn` satisfies them.
///
/// # Subspace types that satisfy `FnMut`
/// The `FnRef` and `FnBox` types in the Subspace library satisfy `Fn` (and thus
/// `FnMut` and `FnOnce` as well). They may be called any number of times and
/// are callable as a const object.
///
/// Like the concepts, `FnRef` is convertible to `FnMutRef` is convertible to
/// `FnOnceRef`. And `FnBox` is convertible to `FnMutBox` is convertible to
/// `FnOnceBox`.
///
/// # Example
/// ```
/// // Accepts any type that can be called once with (Option<i32>) and returns
/// // i32.
/// static i32 call_fn(const sus::fn::Fn<i32(sus::Option<i32>)> auto& f) {
///   return f(sus::some(400)) + f(sus::some(100));  // Returns an i32.
/// }
///
/// i32 x = call_fn([i = 1_i32](sus::Option<i32> o) -> i32 {
///   return sus::move(o).unwrap_or_default() + i;
/// });
/// sus::check(x == 401 + 101);
/// ```
template <class F, class S>
concept Fn = requires(const F& f) {
  {
    __private::InvokedFn<F, typename __private::Sig<S>::Args>::return_type()
  } -> __private::ValidReturnType<typename __private::Sig<S>::Return>;

  requires FnMut<F, S>;
  requires FnOnce<F, S>;
};

}  // namespace sus::fn
