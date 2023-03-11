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
#include "subspace/macros/lifetimebound.h"
#include "subspace/mem/addressof.h"
#include "subspace/mem/forward.h"
#include "subspace/mem/never_value.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/replace.h"

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

union Storage {
  void (*fnptr)();
  void* object;
};

template <class F>
struct Invoker {
  template <class R, class... Args>
  static R fnptr_call_mut(const union Storage& s, Args... args) {
    F f = reinterpret_cast<F>(s.fnptr);
    return (*f)(::sus::forward<Args>(args)...);
  }

  template <class R, class... Args>
  static R object_call_mut(const union Storage& s, Args... args) {
    F& f = *static_cast<F*>(s.object);
    return f(::sus::forward<Args>(args)...);
  }

  template <class R, class... Args>
  static R object_call_once(const union Storage& s, Args... args) {
    F& f = *static_cast<F*>(s.object);
    return ::sus::move(f)(::sus::forward<Args>(args)...);
  }

  template <class R, class... Args>
  static R fnptr_call_const(const union Storage& s, Args... args) {
    const F f = reinterpret_cast<const F>(s.fnptr);
    return (*f)(::sus::forward<Args>(args)...);
  }

  template <class R, class... Args>
  static R object_call_const(const union Storage& s, Args... args) {
    const F& f = *static_cast<const F*>(s.object);
    return f(::sus::forward<Args>(args)...);
  }
};

template <class R, class... CallArgs>
using InvokeFnPtr = R (*)(const union Storage& s, CallArgs... args);

template <class F, class R, class... Args>
concept FunctionPointer =
    std::is_pointer_v<std::decay_t<F>> && requires(F f, Args... args) {
      { (*f)(args...) } -> std::convertible_to<R>;
    };

template <class T>
concept IsFunctionPointer =
    std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>;

template <class F>
concept ConvertsToFunctionPointer = requires(F f) {
  { +f } -> IsFunctionPointer;
};

template <class F, class R, class... Args>
concept CallableOnceMut =
    !FunctionPointer<F, R, Args...> && requires(F && f, Args... args) {
      { ::sus::move(f)(args...) } -> std::convertible_to<R>;
    };

template <class F, class R, class... Args>
concept CallableMut =
    !FunctionPointer<F, R, Args...> && requires(F & f, Args... args) {
      { f(args...) } -> std::convertible_to<R>;
    };

template <class F, class R, class... Args>
concept CallableConst =
    !FunctionPointer<F, R, Args...> && requires(const F& f, Args... args) {
      { f(args...) } -> std::convertible_to<R>;
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
  /// Construction from a function pointer.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <__private::FunctionPointer<R, CallArgs...> F>
  Fn(F ptr) noexcept {
    ::sus::check(ptr != nullptr);
    storage_.fnptr = reinterpret_cast<void (*)()>(ptr);
    invoke_ = &__private::Invoker<F>::template fnptr_call_const<R, CallArgs...>;
  }

  /// Construction from a non-capturing lambda.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <__private::CallableConst<R, CallArgs...> F>
    requires(__private::ConvertsToFunctionPointer<F>)
  constexpr Fn(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_const<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.capturelambda]
  template <__private::CallableConst<R, CallArgs...> F>
    requires(!__private::ConvertsToFunctionPointer<F>)
  constexpr Fn(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_const<R, CallArgs...>;
  }

  ~Fn() noexcept = default;

  constexpr Fn(Fn&& o sus_lifetimebound) noexcept
      : storage_(o.storage_),
        invoke_(::sus::mem::replace_ptr(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }
  constexpr Fn& operator=(Fn&& o sus_lifetimebound) noexcept {
    storage_ = o.storage_;
    invoke_ = ::sus::mem::replace_ptr(o.invoke_, nullptr);
    ::sus::check(invoke_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  Fn(const Fn&) noexcept = delete;
  Fn& operator=(const Fn&) noexcept = delete;

  /// sus::mem::Clone trait.
  constexpr Fn clone() const {
    ::sus::check(invoke_);  // Catch use-after-move.
    return Fn(storage_, invoke_);
  }

  /// Runs the closure.
  inline R operator()(CallArgs... args) const& {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*invoke_)(storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// Runs and consumes the closure.
  inline R operator()(CallArgs... args) && {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*::sus::mem::replace_ptr(invoke_, nullptr))(
        storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  ///
  /// Fn satisfies `From<T>` for the same types that it is constructible from:
  /// function pointers that exactly match its own signature, and const-callable
  /// objects (lambdas) that are compatible with its signature.
  constexpr static auto from(
      __private::FunctionPointer<R, CallArgs...> auto ptr) noexcept {
    return Fn(ptr);
  }
  template <__private::CallableConst<R, CallArgs...> F>
  constexpr static auto from(F&& object sus_lifetimebound) noexcept {
    return Fn(::sus::forward<F>(object));
  }

 private:
  template <class RR, class... AArgs>
  friend class FnOnce;
  template <class RR, class... AArgs>
  friend class FnMut;

  constexpr Fn(union __private::Storage storage,
               __private::InvokeFnPtr<R, CallArgs...> invoke)
      : storage_(storage), invoke_(invoke) {}

  union __private::Storage storage_;
  __private::InvokeFnPtr<R, CallArgs...> invoke_;

  // A function pointer to use as a never-value for InvokeFnPointer.
  static R invoke_never_value(const union __private::Storage& s,
                              CallArgs... args) {}

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(storage_.fnptr),
                                  decltype(storage_.object), decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, Fn, invoke_,
                              &invoke_never_value, &invoke_never_value);

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
  template <__private::FunctionPointer<R, CallArgs...> F>
  FnMut(F ptr) noexcept {
    ::sus::check(ptr != nullptr);
    storage_.fnptr = reinterpret_cast<void (*)()>(ptr);
    invoke_ = &__private::Invoker<F>::template fnptr_call_mut<R, CallArgs...>;
  }

  /// Construction from a non-capturing lambda.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <__private::CallableMut<R, CallArgs...> F>
    requires(__private::ConvertsToFunctionPointer<F>)
  constexpr FnMut(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_mut<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.capturelambda]
  template <__private::CallableMut<R, CallArgs...> F>
    requires(!__private::ConvertsToFunctionPointer<F>)
  constexpr FnMut(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_mut<R, CallArgs...>;
  }

  /// Construction from Fn.
  ///
  /// Since Fn is callable, FnMut is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  constexpr FnMut(Fn<R(CallArgs...)>&& o sus_lifetimebound) noexcept
      : storage_(o.storage_),
        invoke_(::sus::mem::replace_ptr(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }

  ~FnMut() noexcept = default;

  constexpr FnMut(FnMut&& o sus_lifetimebound) noexcept
      : storage_(o.storage_),
        invoke_(::sus::mem::replace_ptr(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }
  constexpr FnMut& operator=(FnMut&& o sus_lifetimebound) noexcept {
    storage_ = o.storage_;
    invoke_ = ::sus::mem::replace_ptr(o.invoke_, nullptr);
    ::sus::check(invoke_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  FnMut(const FnMut&) noexcept = delete;
  FnMut& operator=(const FnMut&) noexcept = delete;

  /// sus::mem::Clone trait.
  constexpr FnMut clone() const {
    ::sus::check(invoke_);  // Catch use-after-move.
    return FnMut(storage_, invoke_);
  }

  /// Runs the closure.
  inline R operator()(CallArgs... args) & {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*invoke_)(storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// Runs and consumes the closure.
  inline R operator()(CallArgs... args) && {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*::sus::mem::replace_ptr(invoke_, nullptr))(
        storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  ///
  /// FnMut satisfies `From<T>` for the same types that it is constructible
  /// from: function pointers that exactly match its own signature, and callable
  /// objects (lambdas) that are compatible with its signature.
  constexpr static auto from(
      __private::FunctionPointer<R, CallArgs...> auto ptr) noexcept {
    return FnMut(ptr);
  }
  template <__private::CallableMut<R, CallArgs...> F>
  constexpr static auto from(F&& object sus_lifetimebound) noexcept {
    return FnMut(::sus::forward<F>(object));
  }

 private:
  template <class RR, class... AArgs>
  friend class FnOnce;

  constexpr FnMut(union __private::Storage storage,
                  __private::InvokeFnPtr<R, CallArgs...> invoke)
      : storage_(storage), invoke_(invoke) {}

  union __private::Storage storage_;
  __private::InvokeFnPtr<R, CallArgs...> invoke_;

  // A function pointer to use as a never-value for InvokeFnPointer.
  static R invoke_never_value(const union __private::Storage& s,
                              CallArgs... args) {}

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(storage_.fnptr),
                                  decltype(storage_.object), decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, FnMut, invoke_,
                              &invoke_never_value, &invoke_never_value);

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
  template <__private::FunctionPointer<R, CallArgs...> F>
  FnOnce(F ptr) noexcept {
    ::sus::check(ptr != nullptr);
    storage_.fnptr = reinterpret_cast<void (*)()>(ptr);
    invoke_ = &__private::Invoker<F>::template fnptr_call_mut<R, CallArgs...>;
  }

  /// Construction from a non-capturing lambda.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <__private::CallableOnceMut<R, CallArgs...> F>
    requires(__private::ConvertsToFunctionPointer<F>)
  constexpr FnOnce(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_once<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.capturelambda]
  template <__private::CallableOnceMut<R, CallArgs...> F>
    requires(!__private::ConvertsToFunctionPointer<F>)
  constexpr FnOnce(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_once<R, CallArgs...>;
  }

  /// Construction from FnMut.
  ///
  /// Since FnMut is callable, FnOnce is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  constexpr FnOnce(FnMut<R(CallArgs...)>&& o sus_lifetimebound) noexcept
      : storage_(o.storage_),
        invoke_(::sus::mem::replace_ptr(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }

  /// Construction from Fn.
  ///
  /// Since Fn is callable, FnOnce is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  constexpr FnOnce(Fn<R(CallArgs...)>&& o sus_lifetimebound) noexcept
      : storage_(o.storage_),
        invoke_(::sus::mem::replace_ptr(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }

  ~FnOnce() noexcept = default;

  constexpr FnOnce(FnOnce&& o sus_lifetimebound) noexcept
      : storage_(o.storage_),
        invoke_(::sus::mem::replace_ptr(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }
  constexpr FnOnce& operator=(FnOnce&& o sus_lifetimebound) noexcept {
    storage_ = o.storage_;
    invoke_ = ::sus::mem::replace_ptr(o.invoke_, nullptr);
    ::sus::check(invoke_);  // Catch use-after-move.
    return *this;
  }

  /// A split FnOnce object, which can be used to construct other FnOnce
  /// objects, but enforces that only one of them is called.
  ///
  /// The Split object must not outlive the FnOnce it's constructed from or
  /// Undefined Behaviour results.
  class Split {
   public:
    Split(FnOnce& fn sus_lifetimebound) : fn_(fn) {}

    // Not Copy or Move, only used to construct FnOnce objects, which are
    // constructible from this type.
    Split(Split&&) = delete;
    Split& operator=(Split&&) = delete;

    /// Runs the underlying `FnOnce`. The `FnOnce` may only be called a single
    /// time and will panic on the second call.
    R operator()(CallArgs... args) && {
      return ::sus::move(fn_)(::sus::forward<CallArgs>(args)...);
    }

   private:
    FnOnce& fn_;
  };

  // Not copyable.
  FnOnce(const FnOnce&) noexcept = delete;
  FnOnce& operator=(const FnOnce&) noexcept = delete;

  /// A `FnOnce` can be split into any number of `FnOnce` objects, while
  /// enforcing that the underlying function is only called a single time.
  ///
  /// This method returns a type that can convert into any number of `FnOnce`
  /// objects. If two of them are called, the second call will panic.
  ///
  /// The returned object must not outlive the `FnOnce` object it is constructed
  /// from, this is normally enforced by only using the `FnOnce` type in
  /// function parameters, which ensures it lives for the entire function body,
  /// and calling `split()` to construct temporary objects for passing to other
  /// functions that receive a `FnOnce`. The result of `split()` should never be
  /// stored as a member of an object.
  ///
  /// Only callable on an lvalue FnOnce (typically written as a function
  /// parameter) as an rvalue can be simply passed along without splitting.
  constexpr Split split() & noexcept sus_lifetimebound { return Split(*this); }

  /// Runs and consumes the closure.
  inline R operator()(CallArgs... args) && {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*::sus::mem::replace_ptr(invoke_, nullptr))(
        storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  ///
  /// FnOnce satisfies `From<T>` for the same types that it is constructible
  /// from: function pointers that exactly match its own signature, and callable
  /// objects (lambdas) that are compatible with its signature.
  constexpr static auto from(
      __private::FunctionPointer<R, CallArgs...> auto ptr) noexcept {
    return FnOnce(ptr);
  }
  template <__private::CallableOnceMut<R, CallArgs...> F>
  constexpr static auto from(F&& object sus_lifetimebound) noexcept {
    return FnOnce(::sus::forward<F>(object));
  }

 private:
  friend FnMut<R, CallArgs...>;
  friend Fn<R, CallArgs...>;

  constexpr FnOnce(union __private::Storage storage,
                   __private::InvokeFnPtr<R, CallArgs...> invoke)
      : storage_(storage), invoke_(invoke) {}

  union __private::Storage storage_;
  __private::InvokeFnPtr<R, CallArgs...> invoke_;

  // A function pointer to use as a never-value for InvokeFnPointer.
  static R invoke_never_value(const union __private::Storage& s,
                              CallArgs... args) {}

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(storage_.fnptr),
                                  decltype(storage_.object), decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, FnOnce, invoke_,
                              &invoke_never_value, &invoke_never_value);

 protected:
  constexpr FnOnce() = default;  // For the NeverValueField.
};

}  // namespace sus::fn
