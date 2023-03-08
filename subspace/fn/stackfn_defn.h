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

#include <stdint.h>

#include "subspace/fn/callable.h"
#include "subspace/mem/addressof.h"
#include "subspace/mem/forward.h"
#include "subspace/mem/never_value.h"
#include "subspace/mem/relocate.h"

namespace sus::fn {

template <class R, class... Args>
class FnOnce;
template <class R, class... Args>
class FnMut;
template <class R, class... Args>
class Fn;

// TODO: Consider generic lambdas, it should be possible to bind them into
// FnOnce/FnMut/Fn?
// Example:
// ```
//    auto even = [](const auto& i) { return i % 2 == 0; };
//    auto r0 = sus::Array<int, 11>::with_values(0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
//                                               10);
//    auto result = r0.iter().filter(even);
// ```

namespace __private {

template <class F>
struct Invoker {
  template <class R, class... Args>
  static R call_mut(uintptr_t p, Args... args) {
    F& f = *reinterpret_cast<F*>(p);
    return f(::sus::forward<Args>(args)...);
  }

  template <class R, class... Args>
  static R call_const(uintptr_t p, Args... args) {
    const F& f = *reinterpret_cast<const F*>(p);
    return f(::sus::forward<Args>(args)...);
  }
};

}  // namespace __private

/// A closure that erases the type of the internal callable object (lambda). A
/// FnMut may be called multiple times, and holds a const callable object, so
/// it will return the same value each call with the same inputs.
///
/// Fn can be used as a FnMut, which can be used as a FnOnce. Lambdas can be
/// converted into a FnOnce, FnMut, or Fn directly.
///
/// FnOnce, FnMut and Fn are only safe to appear as lvalues when they are a
/// function parameter, and a clang-tidy check is provided to enforce this. They
/// only hold a reference to the underlying lambda so they must not outlive the
/// lambda.
///
/// # Why can a "const" Fn convert to a mutable FnMut or FnOnce?
///
/// A FnMut or FnOnce is _allowed_ to mutate its storage, but a "const" Fn
/// closure would just choose not to do so.
///
/// However, a `const Fn` requires that the storage is not mutated, so it is
/// not useful if converted to a `const FnMut` or `const FnOnce` which are
/// only callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a FnOnce from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] Fn<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  Fn(F ptr) noexcept {
    ::sus::check(+ptr != nullptr);
    callable_ = reinterpret_cast<uintptr_t>(+ptr);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_const<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  Fn(F&& object) noexcept {
    callable_ = reinterpret_cast<uintptr_t>(::sus::mem::addressof(object));
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_const<R, CallArgs...>;
  }

  ~Fn() noexcept = default;

  Fn(Fn&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
        // never-value.
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }
  Fn& operator=(Fn&& o) noexcept {
    callable_ = ::sus::mem::replace(o.callable_, uintptr_t{0});
    // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
    // never-value.
    invoke_ = o.invoke_;
    ::sus::check(callable_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  Fn(const Fn&) noexcept = delete;
  Fn& operator=(const Fn&) noexcept = delete;

  /// sus::mem::Clone trait.
  Fn clone() const {
    ::sus::check(callable_);  // Catch use-after-move.
    return Fn(callable_, invoke_);
  }

  /// Runs the closure.
  inline R operator()(CallArgs... args) const& {
    return (*invoke_)(callable_, ::sus::forward<CallArgs>(args)...);
  }

  /// Runs and consumes the closure.
  inline R operator()(CallArgs... args) && {
    ::sus::check(callable_);  // Catch use-after-move.
    return (*invoke_)(::sus::mem::replace(callable_, uintptr_t{0}),
                      ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  constexpr static auto from(F fn) noexcept {
    return Fn(fn);
  }
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  constexpr static auto from(F&& object) noexcept {
    return Fn(::sus::forward<F>(object));
  }

  // operator to avoid extra indirections being inserted when converting, since
  // otherwise an extra Invoker call would be introduced.
  operator FnOnce<R, CallArgs...>() && {
    return FnOnce(::sus::mem::replace(callable_, uintptr_t{0}), invoke_);
  }
  // operator to avoid extra indirections being inserted when converting, since
  // otherwise an extra Invoker call would be introduced.
  operator FnMut<R, CallArgs...>() && {
    return FnMut(::sus::mem::replace(callable_, uintptr_t{0}), invoke_);
  }

 private:
  template <class RR, class... AArgs>
  friend class FnOnce;
  template <class RR, class... AArgs>
  friend class FnMut;

  Fn(uintptr_t callable, R (*invoke)(uintptr_t p, CallArgs... args))
      : callable_(callable), invoke_(invoke) {}

  uintptr_t callable_;
  R (*invoke_)(uintptr_t p, CallArgs... args);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(callable_),
                                  decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, Fn, invoke_, nullptr,
                              nullptr);

 protected:
  constexpr Fn() = default;  // For the NeverValueField.
};

/// A closure that erases the type of the internal callable object (lambda) that
/// may mutate internal state. A FnMut may be called multiple times, and may
/// return a different value on each call with the same inputs.
///
/// Fn can be used as a FnMut, which can be used as a FnOnce. Lambdas can be
/// converted into a FnOnce, FnMut, or Fn directly.
///
/// FnOnce, FnMut and Fn are only safe to appear as lvalues when they are a
/// function parameter, and a clang-tidy check is provided to enforce this. They
/// only hold a reference to the underlying lambda so they must not outlive the
/// lambda.
///
/// # Why can a "const" Fn convert to a mutable FnMut or FnOnce?
///
/// A FnMut or FnOnce is _allowed_ to mutate its storage, but a "const" Fn
/// closure would just choose not to do so.
///
/// However, a `const Fn` requires that the storage is not mutated, so it is
/// not useful if converted to a `const FnMut` or `const FnOnce` which are
/// only callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a FnOnce from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] FnMut<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  FnMut(F ptr) noexcept {
    ::sus::check(+ptr != nullptr);
    callable_ = reinterpret_cast<uintptr_t>(+ptr);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_mut<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  FnMut(F&& object) noexcept {
    callable_ = reinterpret_cast<uintptr_t>(::sus::mem::addressof(object));
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_mut<R, CallArgs...>;
  }

  /// Construction from Fn.
  ///
  /// Since Fn is callable, FnMut is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  FnMut(Fn<R(CallArgs...)>&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }

  ~FnMut() noexcept = default;

  FnMut(FnMut&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
        // never-value.
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }
  FnMut& operator=(FnMut&& o) noexcept {
    callable_ = ::sus::mem::replace(o.callable_, uintptr_t{0});
    // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
    // never-value.
    invoke_ = o.invoke_;
    ::sus::check(callable_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  FnMut(const FnMut&) noexcept = delete;
  FnMut& operator=(const FnMut&) noexcept = delete;

  /// sus::mem::Clone trait.
  FnMut clone() const {
    ::sus::check(callable_);  // Catch use-after-move.
    return FnMut(callable_, invoke_);
  }

  /// Runs the closure.
  inline R operator()(CallArgs... args) & {
    return (*invoke_)(callable_, ::sus::forward<CallArgs>(args)...);
  }

  /// Runs and consumes the closure.
  inline R operator()(CallArgs... args) && {
    ::sus::check(callable_);  // Catch use-after-move.
    return (*invoke_)(::sus::mem::replace(callable_, uintptr_t{0}),
                      ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  constexpr static auto from(F fn) noexcept {
    return FnMut(fn);
  }
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  constexpr static auto from(F&& object) noexcept {
    return FnMut(::sus::forward<F>(object));
  }

 private:
  template <class RR, class... AArgs>
  friend class FnOnce;

  FnMut(uintptr_t callable, R (*invoke)(uintptr_t p, CallArgs... args))
      : callable_(callable), invoke_(invoke) {}

  uintptr_t callable_;
  R (*invoke_)(uintptr_t p, CallArgs... args);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(callable_),
                                  decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, FnMut, invoke_,
                              nullptr, nullptr);

 protected:
  constexpr FnMut() = default;  // For the NeverValueField.
};

/// A closure that erases the type of the internal callable object (lambda). A
/// FnOnce may only be called a single time.
///
/// Fn can be used as a FnMut, which can be used as a FnOnce. Lambdas can be
/// converted into a FnOnce, FnMut, or Fn directly.
///
/// FnOnce, FnMut and Fn are only safe to appear as lvalues when they are a
/// function parameter, and a clang-tidy check is provided to enforce this. They
/// only hold a reference to the underlying lambda so they must not outlive the
/// lambda.
///
/// # Why can a "const" Fn convert to a mutable FnMut or FnOnce?
///
/// A FnMut or FnOnce is _allowed_ to mutate its storage, but a "const" Fn
/// closure would just choose not to do so.
///
/// However, a `const Fn` requires that the storage is not mutated, so it is
/// not useful if converted to a `const FnMut` or `const FnOnce` which are
/// only callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a FnOnce from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] FnOnce<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  FnOnce(F ptr) noexcept {
    ::sus::check(+ptr != nullptr);
    callable_ = reinterpret_cast<uintptr_t>(+ptr);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_mut<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  FnOnce(F&& object) noexcept {
    callable_ = reinterpret_cast<uintptr_t>(::sus::mem::addressof(object));
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_mut<R, CallArgs...>;
  }

  /// Construction from FnMut.
  ///
  /// Since FnMut is callable, FnOnce is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  FnOnce(FnMut<R(CallArgs...)>&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }

  /// Construction from Fn.
  ///
  /// Since Fn is callable, FnOnce is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  FnOnce(Fn<R(CallArgs...)>&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }

  ~FnOnce() noexcept = default;

  FnOnce(FnOnce&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
        // never-value.
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }
  FnOnce& operator=(FnOnce&& o) noexcept {
    callable_ = ::sus::mem::replace(o.callable_, uintptr_t{0});
    // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
    // never-value.
    invoke_ = o.invoke_;
    ::sus::check(callable_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  FnOnce(const FnOnce&) noexcept = delete;
  FnOnce& operator=(const FnOnce&) noexcept = delete;

  /// Runs and consumes the closure.
  inline R operator()(CallArgs... args) && {
    ::sus::check(callable_);  // Catch use-after-move.
    return (*invoke_)(::sus::mem::replace(callable_, uintptr_t{0}),
                      ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  constexpr static auto from(F fn) noexcept {
    return FnOnce(fn);
  }
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  constexpr static auto from(F&& object) noexcept {
    return FnOnce(::sus::forward<F>(object));
  }

 private:
  friend FnMut<R, CallArgs...>;
  friend Fn<R, CallArgs...>;

  FnOnce(uintptr_t callable, R (*invoke)(uintptr_t p, CallArgs... args))
      : callable_(callable), invoke_(invoke) {}

  uintptr_t callable_;
  R (*invoke_)(uintptr_t p, CallArgs... args);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(callable_),
                                  decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, FnOnce, invoke_,
                              nullptr, nullptr);

 protected:
  constexpr FnOnce() = default;  // For the NeverValueField.
};

}  // namespace sus::fn
