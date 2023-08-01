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

// IWYU pragma: private, include "sus/fn/fn.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/fn/__private/callable_types.h"
#include "sus/fn/__private/fn_ref_invoker.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/lifetimebound.h"
#include "sus/mem/addressof.h"
#include "sus/mem/forward.h"
#include "sus/mem/never_value.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"

namespace sus::fn {

/// A closure that erases the type of the internal callable object (lambda). A
/// FnMutRef may be called multiple times, and holds a const callable object, so
/// it will return the same value each call with the same inputs.
///
/// FnRef can be used as a FnMutRef, which can be used as a FnOnceRef. Lambdas
/// can be converted into a FnOnceRef, FnMutRef, or FnRef directly.
///
/// FnOnceRef, FnMutRef and FnRef are only safe to appear as lvalues when they
/// are a function parameter, and a clang-tidy check is provided to enforce
/// this. They only hold a reference to the underlying lambda so they must not
/// outlive the lambda.
///
/// # Why can a "const" FnRef convert to a mutable FnMutRef or FnOnceRef?
///
/// A FnMutRef or FnOnceRef is _allowed_ to mutate its storage, but a "const"
/// FnRef closure would just choose not to do so.
///
/// However, a `const FnRef` requires that the storage is not mutated, so it is
/// not useful if converted to a `const FnMutRef` or `const FnOnceRef` which are
/// only callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a FnOnceRef from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] FnRef<R(CallArgs...)> {
 public:
  /// Construction from a function pointer.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <__private::FunctionPointer<R, CallArgs...> F>
  FnRef(F ptr) noexcept {
    ::sus::check(ptr != nullptr);
    storage_.fnptr = reinterpret_cast<void (*)()>(ptr);
    invoke_ = &__private::Invoker<F>::template fnptr_call_const<R, CallArgs...>;
  }

  /// Construction from a non-capturing lambda.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <__private::CallableConst<R, CallArgs...> F>
    requires(__private::ConvertsToFunctionPointer<F>)
  constexpr FnRef(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_const<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.capturelambda]
  template <__private::CallableConst<R, CallArgs...> F>
    requires(!__private::ConvertsToFunctionPointer<F>)
  constexpr FnRef(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_const<R, CallArgs...>;
  }

  ~FnRef() noexcept = default;

  constexpr FnRef(FnRef&& o sus_lifetimebound) noexcept
      : storage_(o.storage_), invoke_(::sus::mem::replace(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }
  constexpr FnRef& operator=(FnRef&& o sus_lifetimebound) noexcept {
    storage_ = o.storage_;
    invoke_ = ::sus::mem::replace(o.invoke_, nullptr);
    ::sus::check(invoke_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  FnRef(const FnRef&) noexcept = delete;
  FnRef& operator=(const FnRef&) noexcept = delete;

  /// sus::mem::Clone trait.
  constexpr FnRef clone() const {
    ::sus::check(invoke_);  // Catch use-after-move.
    return FnRef(storage_, invoke_);
  }

  /// Runs the closure.
  ///
  /// #[doc.overloads=call.const]
  inline R operator()(CallArgs... args) const& {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*invoke_)(storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// Runs and consumes the closure.
  ///
  /// #[doc.overloads=call.rvalue]
  inline R operator()(CallArgs... args) && {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*::sus::mem::replace(invoke_, nullptr))(
        storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  ///
  /// FnRef satisfies `From<T>` for the same types that it is constructible
  /// from: function pointers that exactly match its own signature, and
  /// const-callable objects (lambdas) that are compatible with its signature.
  constexpr static auto from(
      __private::FunctionPointer<R, CallArgs...> auto ptr) noexcept {
    return FnRef(ptr);
  }
  template <__private::CallableConst<R, CallArgs...> F>
  constexpr static auto from(F&& object sus_lifetimebound) noexcept {
    return FnRef(::sus::forward<F>(object));
  }

 private:
  template <class RR, class... AArgs>
  friend class FnOnceRef;
  template <class RR, class... AArgs>
  friend class FnMutRef;

  constexpr FnRef(union __private::Storage storage,
                  __private::InvokeFnPtr<R, CallArgs...> invoke)
      : storage_(storage), invoke_(invoke) {}

  union __private::Storage storage_;
  /// The `invoke_` pointer is set to null to indicate the FnRef is moved-from.
  /// It uses another pointer value as its never-value.
  __private::InvokeFnPtr<R, CallArgs...> invoke_;

  // A function pointer to use as a never-value for InvokeFnPointer.
  static R invoke_never_value(const union __private::Storage& s,
                              CallArgs... args) {}

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(storage_.fnptr),
                                  decltype(storage_.object), decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, FnRef, invoke_,
                              &invoke_never_value, &invoke_never_value);
  // For the NeverValueField.
  explicit constexpr FnRef(::sus::mem::NeverValueConstructor) noexcept
      : invoke_(&invoke_never_value) {}
};

/// A closure that erases the type of the internal callable object (lambda) that
/// may mutate internal state. A FnMutRef may be called multiple times, and may
/// return a different value on each call with the same inputs.
///
/// FnRef can be used as a FnMutRef, which can be used as a FnOnceRef. Lambdas
/// can be converted into a FnOnceRef, FnMutRef, or FnRef directly.
///
/// FnOnceRef, FnMutRef and FnRef are only safe to appear as lvalues when they
/// are a function parameter, and a clang-tidy check is provided to enforce
/// this. They only hold a reference to the underlying lambda so they must not
/// outlive the lambda.
///
/// # Why can a "const" FnRef convert to a mutable FnMutRef or FnOnceRef?
///
/// A FnMutRef or FnOnceRef is _allowed_ to mutate its storage, but a "const"
/// FnRef closure would just choose not to do so.
///
/// However, a `const FnRef` requires that the storage is not mutated, so it is
/// not useful if converted to a `const FnMutRef` or `const FnOnceRef` which are
/// only callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a FnOnceRef from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] FnMutRef<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <__private::FunctionPointer<R, CallArgs...> F>
  FnMutRef(F ptr) noexcept {
    ::sus::check(ptr != nullptr);
    storage_.fnptr = reinterpret_cast<void (*)()>(ptr);
    invoke_ = &__private::Invoker<F>::template fnptr_call_mut<R, CallArgs...>;
  }

  /// Construction from a non-capturing lambda.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <__private::CallableMut<R, CallArgs...> F>
    requires(__private::ConvertsToFunctionPointer<F>)
  constexpr FnMutRef(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_mut<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.capturelambda]
  template <__private::CallableMut<R, CallArgs...> F>
    requires(!__private::ConvertsToFunctionPointer<F>)
  constexpr FnMutRef(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_mut<R, CallArgs...>;
  }

  /// Construction from FnRef.
  ///
  /// Since FnRef is callable, FnMutRef is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  ///
  /// #[doc.overloads=ctor.fnref]
  constexpr FnMutRef(FnRef<R(CallArgs...)>&& o sus_lifetimebound) noexcept
      : storage_(o.storage_), invoke_(::sus::mem::replace(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }

  ~FnMutRef() noexcept = default;

  constexpr FnMutRef(FnMutRef&& o sus_lifetimebound) noexcept
      : storage_(o.storage_), invoke_(::sus::mem::replace(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }
  constexpr FnMutRef& operator=(FnMutRef&& o sus_lifetimebound) noexcept {
    storage_ = o.storage_;
    invoke_ = ::sus::mem::replace(o.invoke_, nullptr);
    ::sus::check(invoke_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  FnMutRef(const FnMutRef&) noexcept = delete;
  FnMutRef& operator=(const FnMutRef&) noexcept = delete;

  /// sus::mem::Clone trait.
  constexpr FnMutRef clone() const {
    ::sus::check(invoke_);  // Catch use-after-move.
    return FnMutRef(storage_, invoke_);
  }

  /// Runs the closure.
  ///
  /// #[doc.overloads=call.mut]
  inline R operator()(CallArgs... args) & {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*invoke_)(storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// Runs and consumes the closure.
  ///
  /// #[doc.overloads=call.rvalue]
  inline R operator()(CallArgs... args) && {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*::sus::mem::replace(invoke_, nullptr))(
        storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  ///
  /// FnMutRef satisfies `From<T>` for the same types that it is constructible
  /// from: function pointers that exactly match its own signature, and callable
  /// objects (lambdas) that are compatible with its signature.
  constexpr static auto from(
      __private::FunctionPointer<R, CallArgs...> auto ptr) noexcept {
    return FnMutRef(ptr);
  }
  template <__private::CallableMut<R, CallArgs...> F>
  constexpr static auto from(F&& object sus_lifetimebound) noexcept {
    return FnMutRef(::sus::forward<F>(object));
  }

 private:
  template <class RR, class... AArgs>
  friend class FnOnceRef;

  constexpr FnMutRef(union __private::Storage storage,
                     __private::InvokeFnPtr<R, CallArgs...> invoke)
      : storage_(storage), invoke_(invoke) {}

  union __private::Storage storage_;
  /// The `invoke_` pointer is set to null to indicate the `FnMutRef` is
  /// moved-from. It uses another pointer value as its never-value.
  __private::InvokeFnPtr<R, CallArgs...> invoke_;

  // A function pointer to use as a never-value for InvokeFnPointer.
  static R invoke_never_value(const union __private::Storage& s,
                              CallArgs... args) {}

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(storage_.fnptr),
                                  decltype(storage_.object), decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, FnMutRef, invoke_,
                              &invoke_never_value, &invoke_never_value);
  // For the NeverValueField.
  explicit constexpr FnMutRef(::sus::mem::NeverValueConstructor) noexcept
      : invoke_(&invoke_never_value) {}
};

/// A closure that erases the type of the internal callable object (lambda). A
/// FnOnceRef may only be called a single time.
///
/// FnRef can be used as a FnMutRef, which can be used as a FnOnceRef. Lambdas
/// can be converted into a FnOnceRef, FnMutRef, or FnRef directly.
///
/// FnOnceRef, FnMutRef and FnRef are only safe to appear as lvalues when they
/// are a function parameter, and a clang-tidy check is provided to enforce
/// this. They only hold a reference to the underlying lambda so they must not
/// outlive the lambda.
///
/// # Why can a "const" FnRef convert to a mutable FnMutRef or FnOnceRef?
///
/// A FnMutRef or FnOnceRef is _allowed_ to mutate its storage, but a "const"
/// FnRef closure would just choose not to do so.
///
/// However, a `const FnRef` requires that the storage is not mutated, so it is
/// not useful if converted to a `const FnMutRef` or `const FnOnceRef` which are
/// only callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a FnOnceRef from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] FnOnceRef<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  ///
  /// #[doc.overloads=ctor.fnpointer]
  template <__private::FunctionPointer<R, CallArgs...> F>
  FnOnceRef(F ptr) noexcept {
    ::sus::check(ptr != nullptr);
    storage_.fnptr = reinterpret_cast<void (*)()>(ptr);
    invoke_ = &__private::Invoker<F>::template fnptr_call_mut<R, CallArgs...>;
  }

  /// Construction from a non-capturing lambda.
  ///
  /// #[doc.overloads=ctor.lambda]
  template <__private::CallableOnceMut<R, CallArgs...> F>
    requires(__private::ConvertsToFunctionPointer<F>)
  constexpr FnOnceRef(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_once<R, CallArgs...>;
  }

  /// Construction from a capturing lambda or other callable object.
  ///
  /// #[doc.overloads=ctor.capturelambda]
  template <__private::CallableOnceMut<R, CallArgs...> F>
    requires(!__private::ConvertsToFunctionPointer<F>)
  constexpr FnOnceRef(F&& object sus_lifetimebound) noexcept {
    storage_.object = ::sus::mem::addressof(object);
    invoke_ = &__private::Invoker<
        std::remove_reference_t<F>>::template object_call_once<R, CallArgs...>;
  }

  /// Construction from FnMutRef.
  ///
  /// Since FnMutRef is callable, FnOnceRef is already constructible from it,
  /// but this constructor avoids extra indirections being inserted when
  /// converting, since otherwise an extra invoker call would be introduced.
  ///
  /// #[doc.overloads=ctor.fnmutref]
  constexpr FnOnceRef(FnMutRef<R(CallArgs...)>&& o sus_lifetimebound) noexcept
      : storage_(o.storage_), invoke_(::sus::mem::replace(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }

  /// Construction from FnRef.
  ///
  /// Since FnRef is callable, FnOnceRef is already constructible from it, but
  /// this constructor avoids extra indirections being inserted when converting,
  /// since otherwise an extra invoker call would be introduced.
  ///
  /// #[doc.overloads=ctor.fnref]
  constexpr FnOnceRef(FnRef<R(CallArgs...)>&& o sus_lifetimebound) noexcept
      : storage_(o.storage_), invoke_(::sus::mem::replace(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }

  ~FnOnceRef() noexcept = default;

  constexpr FnOnceRef(FnOnceRef&& o sus_lifetimebound) noexcept
      : storage_(o.storage_), invoke_(::sus::mem::replace(o.invoke_, nullptr)) {
    ::sus::check(invoke_);  // Catch use-after-move.
  }
  constexpr FnOnceRef& operator=(FnOnceRef&& o sus_lifetimebound) noexcept {
    storage_ = o.storage_;
    invoke_ = ::sus::mem::replace(o.invoke_, nullptr);
    ::sus::check(invoke_);  // Catch use-after-move.
    return *this;
  }

  // Not copyable.
  FnOnceRef(const FnOnceRef&) noexcept = delete;
  FnOnceRef& operator=(const FnOnceRef&) noexcept = delete;

  /// A split FnOnceRef object, which can be used to construct other FnOnceRef
  /// objects, but enforces that only one of them is called.
  ///
  /// The Split object must not outlive the FnOnceRef it's constructed from or
  /// Undefined Behaviour results.
  class Split {
   public:
    Split(FnOnceRef& fn sus_lifetimebound) : fn_(fn) {}

    // Not Copy or Move, only used to construct FnOnceRef objects, which are
    // constructible from this type.
    Split(Split&&) = delete;
    Split& operator=(Split&&) = delete;

    /// Runs the underlying `FnOnceRef`. The `FnOnceRef` may only be called a
    /// single time and will panic on the second call.
    R operator()(CallArgs... args) && {
      return ::sus::move(fn_)(::sus::forward<CallArgs>(args)...);
    }

   private:
    FnOnceRef& fn_;
  };

  /// A `FnOnceRef` can be split into any number of `FnOnceRef` objects, while
  /// enforcing that the underlying function is only called a single time.
  ///
  /// This method returns a type that can convert into any number of `FnOnceRef`
  /// objects. If two of them are called, the second call will panic.
  ///
  /// The returned object must not outlive the `FnOnceRef` object it is
  /// constructed from, this is normally enforced by only using the `FnOnceRef`
  /// type in function parameters, which ensures it lives for the entire
  /// function body, and calling `split()` to construct temporary objects for
  /// passing to other functions that receive a `FnOnceRef`. The result of
  /// `split()` should never be stored as a member of an object.
  ///
  /// Only callable on an lvalue FnOnceRef (typically written as a function
  /// parameter) as an rvalue can be simply passed along without splitting.
  constexpr Split split() & noexcept sus_lifetimebound { return Split(*this); }

  /// Runs and consumes the closure.
  inline R operator()(CallArgs... args) && {
    ::sus::check(invoke_);  // Catch use-after-move.
    return (*::sus::mem::replace(invoke_, nullptr))(
        storage_, ::sus::forward<CallArgs>(args)...);
  }

  /// `sus::construct::From` trait implementation.
  ///
  /// FnOnceRef satisfies `From<T>` for the same types that it is constructible
  /// from: function pointers that exactly match its own signature, and callable
  /// objects (lambdas) that are compatible with its signature.
  constexpr static auto from(
      __private::FunctionPointer<R, CallArgs...> auto ptr) noexcept {
    return FnOnceRef(ptr);
  }
  template <__private::CallableOnceMut<R, CallArgs...> F>
  constexpr static auto from(F&& object sus_lifetimebound) noexcept {
    return FnOnceRef(::sus::forward<F>(object));
  }

 private:
  friend FnMutRef<R, CallArgs...>;
  friend FnRef<R, CallArgs...>;

  constexpr FnOnceRef(union __private::Storage storage,
                      __private::InvokeFnPtr<R, CallArgs...> invoke)
      : storage_(storage), invoke_(invoke) {}

  union __private::Storage storage_;
  /// The `invoke_` pointer is set to null to indicate the FnOnceRef is
  /// moved-from. It uses another pointer value as its never-value.
  __private::InvokeFnPtr<R, CallArgs...> invoke_;

  // A function pointer to use as a never-value for InvokeFnPointer.
  static R invoke_never_value(const union __private::Storage& s,
                              CallArgs... args) {}

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(storage_.fnptr),
                                  decltype(storage_.object), decltype(invoke_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, FnOnceRef, invoke_,
                              &invoke_never_value, &invoke_never_value);
  // For the NeverValueField.
  explicit constexpr FnOnceRef(::sus::mem::NeverValueConstructor) noexcept
      : invoke_(&invoke_never_value) {}
};

}  // namespace sus::fn
