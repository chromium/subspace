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

#include "concepts/callable.h"
#include "mem/__private/relocate.h"
#include "mem/forward.h"

namespace sus::fn {

namespace __private {

/// The type-erased type (dropping the type of the internal lambda) of the
/// closure's heap-allocated storage.
struct FnStorageBase;

/// Helper type returned by sus_bind() and used to construct a closure.
template <class F>
struct SusBind;

/// Helper to determine which functions need to be instatiated for the closure,
/// to be called from FnOnce, FnMut, and/or Fn.
///
/// This type indicates the closure can only be called from FnOnce.
enum StorageConstructionFnOnceType { StorageConstructionFnOnce };
/// Helper to determine which functions need to be instatiated for the closure,
/// to be called from FnOnce, FnMut, and/or Fn.
///
/// This type indicates the closure can be called from FnMut or FnOnce.
enum StorageConstructionFnMutType { StorageConstructionFnMut };
/// Helper to determine which functions need to be instatiated for the closure,
/// to be called from FnOnce, FnMut, and/or Fn.
///
/// This type indicates the closure can be called from Fn, FnMut or FnOnce.
enum StorageConstructionFnType { StorageConstructionFn };

/// Used to indicate if the closure is holding a function pointer,
/// heap-allocated storage, or nothing due to being moved-from.
enum FnType {
  /// Is moved-from and should not be used in this state unless reinitialized.
  MovedFrom,
  /// Holds a function pointer or captureless lambda.
  FnPointer,
  /// Holds the type-erased output of sus_bind() in a heap allocation.
  Storage,
};

}  // namespace __private

template <class R, class... Args>
class FnOnce;
template <class R, class... Args>
class FnMut;
template <class R, class... Args>
class Fn;

/// A closure that erases the type of the internal callable object (lambda). A
/// FnOnce may only be called a single time.
///
/// Fn can be used as a FnMut, which can be used as a FnOnce.
///
/// Lambdas without captures can be converted into a FnOnce, FnMut, or Fn
/// directly. If the lambda has captured, it must be given to one of:
///
/// - `sus_bind(sus_store(..captures..), lambda)` to bind a const lambda which
/// captures variables from local state. Variables to be captured in the lambda
/// must also be named in sus_store(). `sus_bind()` only allows those named
/// variables to be captured, and ensures they are stored by value instead of by
/// reference.
///
/// - `sus_bind0(lambda)` to bind a const lambda which has bound variables that
/// don't capture state from outside the lambda, such as `[i = 2]() { return i;
/// }`.
///
/// - sus_bind_mut(sus_store(...captures...), lambda)` to bind a mutable lambda
/// which captures variables from local state.
///
/// - `sus_bind0_mut(lambda)` to bind a mutable lambda which has bound variables
/// that don't capture state from outside the lambda, such as `[i = 2]() {
/// return ++i; }`.
///
/// Within sus_store(), a varaible name can be wrapped with a helper to capture
/// in different ways:
///
/// - `sus_take(x)` will move `x` into the closure instead of copying it.
///
/// - `sus_unsafe_pointer(x)` will allow capturing a pointer. Otherwise it be
/// disallowed, and it is strongly discouraged as it requires careful present
/// and future understanding of the pointee's lifetime.
///
/// # Example
///
/// Moves `a` into the closure's storage, and copies b. The lambda then refers
/// to the closure's stored values by reference.
///
/// ```
/// int a = 1;
/// int b = 2;
/// FnOnce<void()> f = sus_bind_mut(
///   sus_store(sus_take(a), b), [&a, &b]() mutable { a += b; }
/// );
/// ```
///
/// # Why can a "const" Fn convert to a mutable FnMut or FnOnce?
///
/// A FnMut or FnOnce is _allowed_ to mutate its storage, but a "const" Fn
/// closure would just choose not to do so.
///
/// However, a `const Fn` requires that the storage is not mutated, so it is not
/// useful if converted to a `const FnMut` or `const FnOnce` which are only
/// callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a FnOnce from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] FnOnce<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  FnOnce(F ptr) noexcept;

  /// Construction from the output of `sus_bind()`.
  template <::sus::concepts::callable::LambdaReturns<R, CallArgs...> F>
  FnOnce(__private::SusBind<F>&& holder) noexcept
      : FnOnce(__private::StorageConstructionFnOnce,
               static_cast<F&&>(holder.lambda)) {}

  ~FnOnce() noexcept;

  FnOnce(FnOnce&& o) noexcept;
  FnOnce& operator=(FnOnce&& o) noexcept;

  FnOnce(const FnOnce&) noexcept = delete;
  FnOnce& operator=(const FnOnce&) noexcept = delete;

  /// Runs and consumes the closure.
  inline R operator()(CallArgs&&... args) && noexcept;

  /// `sus::concepts::from::From` trait implementation for function pointers or
  /// lambdas without captures.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  constexpr static auto from(F fn) noexcept {
    return FnOnce(static_cast<R (*)(CallArgs...)>(fn));
  }
  /// `sus::concepts::from::From` trait implementation for the output of
  /// `sus_bind()`.
  template <::sus::concepts::callable::LambdaReturns<R, CallArgs...> F>
  constexpr static auto from(__private::SusBind<F>&& holder) noexcept {
    return FnOnce(static_cast<__private::SusBind<F>&&>(holder));
  }

 protected:
  template <class ConstructionType,
            ::sus::concepts::callable::LambdaReturns<R, CallArgs...> F>
  FnOnce(ConstructionType, F&& lambda) noexcept;

  // Functions to construct and return a pointer to a static vtable object for
  // the `__private::FnStorage` being stored in `storage_`.
  //
  // A FnOnce needs to store only a single pointer, for call_once(). But a Fn
  // needs to store three, for call(), call_mut() and call_once() since it can
  // be converted to a FnMut or FnOnce. For that reason we have 3 overloads
  // where each one instantiates only the functions it requires - to avoid
  // trying to compile functions that aren't accessible and thus don't need to
  // be able to compile.
  template <class FnStorage>
  static void make_vtable(FnStorage&,
                          __private::StorageConstructionFnOnceType) noexcept;
  template <class FnStorage>
  static void make_vtable(FnStorage&,
                          __private::StorageConstructionFnMutType) noexcept;
  template <class FnStorage>
  static void make_vtable(FnStorage&,
                          __private::StorageConstructionFnType) noexcept;

  __private::FnType type_;
  union {
    // Used when the closure is a function pointer (or a captureless lambda,
    // which is converted to a function pointer).
    struct {
      R (*fn_)(CallArgs...);
    } fn_ptr_;

    // Used when the closure is a lambda with storage, generated by
    // `sus_bind()`. This is a type-erased pointer to the heap storage.
    __private::FnStorageBase* storage_;
  };

 private:
  sus_class_trivial_relocatable(unsafe_fn);
};

/// A closure that erases the type of the internal callable object (lambda). A
/// FnMut may be called a multiple times, and may mutate its storage.
///
/// Fn can be used as a FnMut, which can be used as a FnOnce.
///
/// Lambdas without captures can be converted into a FnOnce, FnMut, or Fn
/// directly. If the lambda has captured, it must be given to one of:
///
/// - `sus_bind(sus_store(..captures..), lambda)` to bind a const lambda which
/// captures variables from local state. Variables to be captured in the lambda
/// must also be named in sus_store(). `sus_bind()` only allows those named
/// variables to be captured, and ensures they are stored by value instead of by
/// reference.
///
/// - `sus_bind0(lambda)` to bind a const lambda which has bound variables that
/// don't capture state from outside the lambda, such as `[i = 2]() { return i;
/// }`.
///
/// - sus_bind_mut(sus_store(...captures...), lambda)` to bind a mutable lambda
/// which captures variables from local state.
///
/// - `sus_bind0_mut(lambda)` to bind a mutable lambda which has bound variables
/// that don't capture state from outside the lambda, such as `[i = 2]() {
/// return ++i; }`.
///
/// Within sus_store(), a varaible name can be wrapped with a helper to capture
/// in different ways:
///
/// - `sus_take(x)` will move `x` into the closure instead of copying it.
///
/// - `sus_unsafe_pointer(x)` will allow capturing a pointer. Otherwise it be
/// disallowed, and it is strongly discouraged as it requires careful present
/// and future understanding of the pointee's lifetime.
///
/// # Example
///
/// Moves `a` into the closure's storage, and copies b. The lambda then refers
/// to the closure's stored values by reference.
///
/// ```
/// int a = 1;
/// int b = 2;
/// FnMut<void()> f = sus_bind_mut(
///   sus_store(sus_take(a), b), [&a, &b]() mutable { a += b; }
/// );
/// ```
///
/// # Why can a "const" Fn convert to a mutable FnMut or FnOnce?
///
/// A FnMut or FnOnce is _allowed_ to mutate its storage, but a "const" Fn
/// closure would just choose not to do so.
///
/// However, a `const Fn` requires that the storage is not mutated, so it is not
/// useful if converted to a `const FnMut` or `const FnOnce` which are only
/// callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a FnMut from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] FnMut<R(CallArgs...)>
    : public FnOnce<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  FnMut(F ptr) noexcept : FnOnce<R(CallArgs...)>(static_cast<F&&>(ptr)) {}

  /// Construction from the output of `sus_bind()`.
  template <::sus::concepts::callable::LambdaReturns<R, CallArgs...> F>
  FnMut(__private::SusBind<F>&& holder) noexcept
      : FnOnce<R(CallArgs...)>(__private::StorageConstructionFnMut,
                               static_cast<F&&>(holder.lambda)) {}

  ~FnMut() noexcept = default;

  FnMut(FnMut&&) noexcept = default;
  FnMut& operator=(FnMut&&) noexcept = default;

  FnMut(const FnMut&) noexcept = delete;
  FnMut& operator=(const FnMut&) noexcept = delete;

  // Runs the closure.
  inline R operator()(CallArgs&&... args) & noexcept;
  // Runs and consumes the closure.
  inline R operator()(CallArgs&&... args) && noexcept {
    return static_cast<FnOnce<R(CallArgs...)>&&>(*this)(
        forward<CallArgs>(args)...);
  }

  /// `sus::concepts::from::From` trait implementation for function pointers or
  /// lambdas without captures.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  constexpr static auto from(F fn) noexcept {
    return FnMut(static_cast<R (*)(CallArgs...)>(fn));
  }
  /// `sus::concepts::from::From` trait implementation for the output of
  /// `sus_bind()`.
  template <::sus::concepts::callable::LambdaReturns<R, CallArgs...> F>
  constexpr static auto from(__private::SusBind<F>&& holder) noexcept {
    return FnMut(static_cast<__private::SusBind<F>&&>(holder));
  }

 protected:
  // This class may only have trivially-destructible storage and must not
  // do anything in its destructor, as `FnOnce` moves from itself, and it
  // would slice that off.

  template <class ConstructionType,
            ::sus::concepts::callable::LambdaReturns<R, CallArgs...> F>
  FnMut(ConstructionType c, F&& lambda) noexcept
      : FnOnce<R(CallArgs...)>(c, static_cast<F&&>(lambda)) {}

 private:
  sus_class_trivial_relocatable(unsafe_fn);
};

/// A closure that erases the type of the internal callable object (lambda). A
/// Fn may be called a multiple times, and will not mutate its storage.
///
/// Fn can be used as a FnMut, which can be used as a FnOnce.
///
/// Lambdas without captures can be converted into a FnOnce, FnMut, or Fn
/// directly. If the lambda has captured, it must be given to one of:
///
/// - `sus_bind(sus_store(..captures..), lambda)` to bind a const lambda which
/// captures variables from local state. Variables to be captured in the lambda
/// must also be named in sus_store(). `sus_bind()` only allows those named
/// variables to be captured, and ensures they are stored by value instead of by
/// reference.
///
/// - `sus_bind0(lambda)` to bind a const lambda which has bound variables that
/// don't capture state from outside the lambda, such as `[i = 2]() { return i;
/// }`.
///
/// - sus_bind_mut(sus_store(...captures...), lambda)` to bind a mutable lambda
/// which captures variables from local state.
///
/// - `sus_bind0_mut(lambda)` to bind a mutable lambda which has bound variables
/// that don't capture state from outside the lambda, such as `[i = 2]() {
/// return ++i; }`.
///
/// Within sus_store(), a varaible name can be wrapped with a helper to capture
/// in different ways:
///
/// - `sus_take(x)` will move `x` into the closure instead of copying it.
///
/// - `sus_unsafe_pointer(x)` will allow capturing a pointer. Otherwise it be
/// disallowed, and it is strongly discouraged as it requires careful present
/// and future understanding of the pointee's lifetime.
///
/// # Example
///
/// Moves `a` into the closure's storage, and copies b. The lambda then refers
/// to the closure's stored values by reference.
///
/// ```
/// int a = 1;
/// int b = 2;
/// Fn<int()> f = sus_bind(
///   sus_store(sus_take(a), b), [&a, &b]() { return a + b; }
/// );
/// ```
///
/// # Why can a "const" Fn convert to a mutable FnMut or FnOnce?
///
/// A FnMut or FnOnce is _allowed_ to mutate its storage, but a "const" Fn
/// closure would just choose not to do so.
///
/// However, a `const Fn` requires that the storage is not mutated, so it is not
/// useful if converted to a `const FnMut` or `const FnOnce` which are only
/// callable as mutable objects.
///
/// # Null pointers
///
/// A null function pointer is not allowed, constructing a Fn from a null
/// pointer will panic.
template <class R, class... CallArgs>
class [[sus_trivial_abi]] Fn<R(CallArgs...)> : public FnMut<R(CallArgs...)> {
 public:
  /// Construction from a function pointer or captureless lambda.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  Fn(F ptr) noexcept : FnMut<R(CallArgs...)>(static_cast<F&&>(ptr)) {}

  /// Construction from the output of `sus_bind()`.
  template <::sus::concepts::callable::LambdaReturnsConst<R, CallArgs...> F>
  Fn(__private::SusBind<F>&& holder) noexcept
      : FnMut<R(CallArgs...)>(__private::StorageConstructionFn,
                              static_cast<F&&>(holder.lambda)) {}

  ~Fn() noexcept = default;

  Fn(Fn&&) noexcept = default;
  Fn& operator=(Fn&&) noexcept = default;

  Fn(const Fn&) noexcept = delete;
  Fn& operator=(const Fn&) noexcept = delete;

  // Runs the closure.
  inline R operator()(CallArgs&&... args) const& noexcept;

  // Runs and consumes the closure.
  inline R operator()(CallArgs&&... args) && noexcept {
    return static_cast<FnOnce<R(CallArgs...)>&&>(*this)(
        forward<CallArgs>(args)...);
  }

  /// `sus::concepts::from::From` trait implementation for function pointers or
  /// lambdas without captures.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  constexpr static auto from(F fn) noexcept {
    return Fn(static_cast<R (*)(CallArgs...)>(fn));
  }
  /// `sus::concepts::from::From` trait implementation for the output of
  /// `sus_bind()`.
  template <::sus::concepts::callable::LambdaReturnsConst<R, CallArgs...> F>
  constexpr static auto from(__private::SusBind<F>&& holder) noexcept {
    return Fn(static_cast<__private::SusBind<F>&&>(holder));
  }

 protected:
  // This class may only have trivially-destructible storage and must not
  // do anything in its destructor, as `FnOnce` moves from itself, and it
  // would slice that off.

  template <::sus::concepts::callable::LambdaReturnsConst<R, CallArgs...> F>
  Fn(__private::StorageConstructionFnType, F&& fn) noexcept;

 private:
  sus_class_trivial_relocatable(unsafe_fn);
};

}  // namespace sus::fn
