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
class SFnOnce;
template <class R, class... Args>
class SFnMut;
template <class R, class... Args>
class SFn;

// TODO: Consider generic lambdas, it should be possible to bind them into
// SFnOnce/SFnMut/SFn?
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
/// SFnMut may be called multiple times, and holds a const callable object, so
/// it will return the same value each call with the same inputs.
///
/// SFn can be used as a SFnMut, which can be used as a SFnOnce. Lambdas can be
/// converted into a SFnOnce, SFnMut, or SFn directly.
///
/// SFnOnce, SFnMut and SFn are only safe to appear as lvalues when they are a
/// function parameter, and a clang-tidy check is provided to enforce this. They
/// only hold a reference to the underlying lambda so they must not outlive the
/// lambda.
///
/// # Why can a "const" SFn convert to a mutable SFnMut or SFnOnce?
///
/// A SFnMut or SFnOnce is _allowed_ to mutate its storage, but a "const" SFn
/// closure would just choose not to do so.
///
/// However, a `const SFn` requires that the storage is not mutated, so it is
/// not useful if converted to a `const SFnMut` or `const SFnOnce` which are
/// only callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a SFnOnce from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] SFn<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  SFn(F ptr) noexcept {
    ::sus::check(+ptr != nullptr);
    callable_ = reinterpret_cast<uintptr_t>(+ptr);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_const<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  SFn(F&& object) noexcept {
    callable_ = reinterpret_cast<uintptr_t>(::sus::mem::addressof(object));
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_const<R, CallArgs...>;
  }

  ~SFn() noexcept = default;

  SFn(SFn&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
        // never-value.
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }
  SFn& operator=(SFn&& o) noexcept {
    callable_ = ::sus::mem::replace(o.callable_, uintptr_t{0});
    // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
    // never-value.
    invoke_ = o.invoke_;
    ::sus::check(callable_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  SFn(const SFn&) noexcept = delete;
  SFn& operator=(const SFn&) noexcept = delete;

  /// sus::mem::Clone trait.
  SFn clone() const {
    ::sus::check(callable_);  // Catch use-after-move.
    return SFn(callable_, invoke_);
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
    return SFn(fn);
  }
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  constexpr static auto from(F&& object) noexcept {
    return SFn(::sus::forward<F>(object));
  }

  // operator to avoid extra indirections being inserted when converting, since
  // otherwise an extra Invoker call would be introduced.
  operator SFnOnce<R, CallArgs...>() && {
    return SFnOnce(::sus::mem::replace(callable_, uintptr_t{0}), invoke_);
  }
  // operator to avoid extra indirections being inserted when converting, since
  // otherwise an extra Invoker call would be introduced.
  operator SFnMut<R, CallArgs...>() && {
    return SFnMut(::sus::mem::replace(callable_, uintptr_t{0}), invoke_);
  }

 private:
  template <class RR, class... AArgs>
  friend class SFnOnce;
  template <class RR, class... AArgs>
  friend class SFnMut;

  SFn(uintptr_t callable, R (*invoke)(uintptr_t p, CallArgs... args))
      : callable_(callable), invoke_(invoke) {}

  uintptr_t callable_;
  R (*invoke_)(uintptr_t p, CallArgs... args);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(callable_),
                                  decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, SFn, invoke_, nullptr,
                              nullptr);

 protected:
  constexpr SFn() = default;  // For the NeverValueField.
};

/// A closure that erases the type of the internal callable object (lambda) that
/// may mutate internal state. A SFnMut may be called multiple times, and may
/// return a different value on each call with the same inputs.
///
/// SFn can be used as a SFnMut, which can be used as a SFnOnce. Lambdas can be
/// converted into a SFnOnce, SFnMut, or SFn directly.
///
/// SFnOnce, SFnMut and SFn are only safe to appear as lvalues when they are a
/// function parameter, and a clang-tidy check is provided to enforce this. They
/// only hold a reference to the underlying lambda so they must not outlive the
/// lambda.
///
/// # Why can a "const" SFn convert to a mutable SFnMut or SFnOnce?
///
/// A SFnMut or SFnOnce is _allowed_ to mutate its storage, but a "const" SFn
/// closure would just choose not to do so.
///
/// However, a `const SFn` requires that the storage is not mutated, so it is
/// not useful if converted to a `const SFnMut` or `const SFnOnce` which are
/// only callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a SFnOnce from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] SFnMut<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  SFnMut(F ptr) noexcept {
    ::sus::check(+ptr != nullptr);
    callable_ = reinterpret_cast<uintptr_t>(+ptr);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_mut<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  SFnMut(F&& object) noexcept {
    callable_ = reinterpret_cast<uintptr_t>(::sus::mem::addressof(object));
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_mut<R, CallArgs...>;
  }

  /// Construction from SFn.
  ///
  /// Since SFn is callable, SFnMut is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  SFnMut(SFn<R(CallArgs...)>&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }

  ~SFnMut() noexcept = default;

  SFnMut(SFnMut&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
        // never-value.
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }
  SFnMut& operator=(SFnMut&& o) noexcept {
    callable_ = ::sus::mem::replace(o.callable_, uintptr_t{0});
    // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
    // never-value.
    invoke_ = o.invoke_;
    ::sus::check(callable_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  SFnMut(const SFnMut&) noexcept = delete;
  SFnMut& operator=(const SFnMut&) noexcept = delete;

  /// sus::mem::Clone trait.
  SFnMut clone() const {
    ::sus::check(callable_);  // Catch use-after-move.
    return SFnMut(callable_, invoke_);
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
    return SFnMut(fn);
  }
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  constexpr static auto from(F&& object) noexcept {
    return SFnMut(::sus::forward<F>(object));
  }

 private:
  template <class RR, class... AArgs>
  friend class SFnOnce;

  SFnMut(uintptr_t callable, R (*invoke)(uintptr_t p, CallArgs... args))
      : callable_(callable), invoke_(invoke) {}

  uintptr_t callable_;
  R (*invoke_)(uintptr_t p, CallArgs... args);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(callable_),
                                  decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, SFnMut, invoke_,
                              nullptr, nullptr);

 protected:
  constexpr SFnMut() = default;  // For the NeverValueField.
};

/// A closure that erases the type of the internal callable object (lambda). A
/// SFnOnce may only be called a single time.
///
/// SFn can be used as a SFnMut, which can be used as a SFnOnce. Lambdas can be
/// converted into a SFnOnce, SFnMut, or SFn directly.
///
/// SFnOnce, SFnMut and SFn are only safe to appear as lvalues when they are a
/// function parameter, and a clang-tidy check is provided to enforce this. They
/// only hold a reference to the underlying lambda so they must not outlive the
/// lambda.
///
/// # Why can a "const" SFn convert to a mutable SFnMut or SFnOnce?
///
/// A SFnMut or SFnOnce is _allowed_ to mutate its storage, but a "const" SFn
/// closure would just choose not to do so.
///
/// However, a `const SFn` requires that the storage is not mutated, so it is
/// not useful if converted to a `const SFnMut` or `const SFnOnce` which are
/// only callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a SFnOnce from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] SFnOnce<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  SFnOnce(F ptr) noexcept {
    ::sus::check(+ptr != nullptr);
    callable_ = reinterpret_cast<uintptr_t>(+ptr);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_mut<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  SFnOnce(F&& object) noexcept {
    callable_ = reinterpret_cast<uintptr_t>(::sus::mem::addressof(object));
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template call_mut<R, CallArgs...>;
  }

  /// Construction from SFnMut.
  ///
  /// Since SFnMut is callable, SFnOnce is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  SFnOnce(SFnMut<R(CallArgs...)>&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }

  /// Construction from SFn.
  ///
  /// Since SFn is callable, SFnOnce is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  SFnOnce(SFn<R(CallArgs...)>&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }

  ~SFnOnce() noexcept = default;

  SFnOnce(SFnOnce&& o) noexcept
      : callable_(::sus::mem::replace(o.callable_, uintptr_t{0})),
        // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
        // never-value.
        invoke_(o.invoke_) {
    ::sus::check(callable_);  // Catch use-after-move.
  }
  SFnOnce& operator=(SFnOnce&& o) noexcept {
    callable_ = ::sus::mem::replace(o.callable_, uintptr_t{0});
    // Not setting `o.invoke_` to nullptr, as invoke_ as nullptr is its
    // never-value.
    invoke_ = o.invoke_;
    ::sus::check(callable_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  SFnOnce(const SFnOnce&) noexcept = delete;
  SFnOnce& operator=(const SFnOnce&) noexcept = delete;

  /// Runs and consumes the closure.
  inline R operator()(CallArgs... args) && {
    ::sus::check(callable_);  // Catch use-after-move.
    return (*invoke_)(::sus::mem::replace(callable_, uintptr_t{0}),
                      ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
  constexpr static auto from(F fn) noexcept {
    return SFnOnce(fn);
  }
  template <::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
  constexpr static auto from(F&& object) noexcept {
    return SFnOnce(::sus::forward<F>(object));
  }

 private:
  friend SFnMut<R, CallArgs...>;
  friend SFn<R, CallArgs...>;

  SFnOnce(uintptr_t callable, R (*invoke)(uintptr_t p, CallArgs... args))
      : callable_(callable), invoke_(invoke) {}

  uintptr_t callable_;
  R (*invoke_)(uintptr_t p, CallArgs... args);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(callable_),
                                  decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, SFnOnce, invoke_,
                              nullptr, nullptr);

 protected:
  constexpr SFnOnce() = default;  // For the NeverValueField.
};

}  // namespace sus::fn
