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
#include <type_traits>

#include "concepts/callable.h"
#include "mem/forward.h"

namespace sus::fn {

namespace __private {

struct FnStorageBase;

enum StorageConstructionFnOnceType { StorageConstructionFnOnce };
enum StorageConstructionFnMutType { StorageConstructionFnMut };
enum StorageConstructionFnType { StorageConstructionFn };

enum FnType {
  MovedFrom,
  FnPointer,
  Storage,
};

template <class F>
struct SusBind {
  F lambda;
};

}  // namespace __private

template <class R, class... Args>
class FnOnce;
template <class R, class... Args>
class FnMut;
template <class R, class... Args>
class Fn;

/// A closure that holds a `Callable` and may only be called a single time.
///
/// Closures optionally store arguments that will be passed to the `Callable`
/// when it is run. This closure owns any arguments passed to it for storage,
/// copying or moving into its storage as needed. A `FnOnce` will move those
/// stored arguments to the `Callable`, allowing it to receive them by value and
/// construct them from a move operation. Alternatively the `Callable` can
/// receive the stored arguments as an rvalue (`&&`) or mutable reference (`&`)
/// and is allowed to mutate the arguments stored within the `FnOnce`.
///
/// Fn and FnMut are convertible to FnOnce.
///
/// # Why can a "const" Fn convert to a mutable FnOnce?
///
/// A FnOnce is _allowed_ to mutate but for a Fn converted to a FnOnce, it would
/// become a FnOnce which chooses not to mutate its storage.
template <class R, class... CallArgs>
class FnOnce<R(CallArgs...)> {
 public:
  /// Makes a FnOnce closure that holds a function pointer inline. No heap
  /// allocations are performed.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  constexpr static FnOnce with(F fn) noexcept {
    return FnOnce(static_cast<R (*)(CallArgs...)>(fn));
  }

  /// Makes a FnOnce closure that holds a function pointer inline. No heap
  /// allocations are performed.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  constexpr static FnOnce with(
      __private::SusBind<F>&& holder) noexcept {
    return FnOnce(static_cast<R (*)(CallArgs...)>(holder.lambda));
  }

  /// Makes a FnOnce closure that holds its functor and any stored arguments in
  /// a heap allocation.
  ///
  /// Requires that `F` can receive a reference to, or a moved value, of each
  /// stored argument.
  // TODO: Check return type.
  template <::sus::concepts::callable::CallableObjectOnce<CallArgs...> F>
  constexpr static FnOnce with(
      __private::SusBind<F>&& holder) noexcept {
    return FnOnce(__private::StorageConstructionFnOnce,
                  static_cast<F&&>(holder.lambda));
  }

  ~FnOnce() noexcept;

  FnOnce(FnOnce&& o) noexcept;
  FnOnce& operator=(FnOnce&& o) noexcept;

  FnOnce(const FnOnce&) noexcept = delete;
  FnOnce& operator=(const FnOnce&) noexcept = delete;

  // Runs and consumes the closure.
  inline R operator()(CallArgs&&... args) && noexcept {
    return static_cast<FnOnce&&>(*this).call_once(forward<CallArgs>(args)...);
  }

  // Runs and consumes the closure.
  R call_once(CallArgs&&...) && noexcept;

 protected:
  // Function pointer constructor.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  FnOnce(F fn) noexcept;

  template <class ConstructionType, class F>
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

  // TODO: Small size optimization? When can we inline the storage beyond
  // a fn pointer with no bound args?
  __private::FnType type_;

  union {
    struct {
      R (*fn_)(CallArgs...);
    } fn_ptr_;

    __private::FnStorageBase* storage_;
  };
};

/// A closure that holds a `Callable`, may be called multiple times, and is able
/// to mutate the stored arguments within the `FnMut`.
///
/// Closures optionally store arguments that will be passed to the `Callable`
/// when it is run. This closure owns any arguments passed to it for storage,
/// copying or moving into its storage as needed. A `FnMut` will pass a mutable
/// reference (`&`) to those stored arguments to the `Callable`, allowing it to
/// receive them by const or mutable reference, or by value if it wants to copy.
/// The `Callable` is able to mutate the stored arguments within the `FnMut`.
///
/// Fn is convertible to FnMut, and FnMut is convertible to FnOnce.
///
/// # Why can a "const" Fn convert to a mutable FnMut?
///
/// A FnMut is _allowed_ to mutate but for a Fn converted to a FnMut, it would
/// become a FnMut which chooses not to mutate its storage.
template <class R, class... CallArgs>
class FnMut<R(CallArgs...)> : public FnOnce<R(CallArgs...)> {
 public:
  /// Makes a FnMut closure that holds a function pointer inline. No heap
  /// allocations are performed.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  constexpr static FnMut with(F fn) noexcept {
    return FnMut(static_cast<R (*)(CallArgs...)>(fn));
  }

  /// Makes a FnMut closure that holds a function pointer inline. No heap
  /// allocations are performed.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  constexpr static FnMut with(__private::SusBind<F>&& holder) noexcept {
    return FnMut(static_cast<R (*)(CallArgs...)>(holder.lambda));
  }

  /// Makes a FnOnce closure that holds its functor and any stored arguments in
  /// a heap allocation.
  ///
  /// Requires that `F` can receive a (const or mutable) reference to each
  /// stored argument.
  template <::sus::concepts::callable::CallableObjectMut<CallArgs...> F>
  constexpr static FnMut with(__private::SusBind<F>&& holder) noexcept {
    return FnMut(__private::StorageConstructionFnMut,
                 static_cast<F&&>(holder.lambda));
  }

  ~FnMut() noexcept = default;

  FnMut(FnMut&&) noexcept = default;
  FnMut& operator=(FnMut&&) noexcept = default;

  FnMut(const FnMut&) noexcept = delete;
  FnMut& operator=(const FnMut&) noexcept = delete;

  // Runs the closure.
  inline R operator()(CallArgs&&... args) & noexcept {
    return call_mut(forward<CallArgs>(args)...);
  }
  inline R operator()(CallArgs&&... args) && noexcept {
    return static_cast<FnOnce<R(CallArgs...)>&&>(*this).call_once(
        forward<CallArgs>(args)...);
  }

  // Runs the closure.
  R call_mut(CallArgs&&...) & noexcept;

 protected:
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  FnMut(F fn) noexcept;

  template <class ConstructionType,
            ::sus::concepts::callable::CallableObjectMut<CallArgs...> F>
  FnMut(ConstructionType, F&& fn) noexcept;
};

/// A closure that holds a `Callable`, may be called multiple times, and is will
/// never mutate any arguments held in its storage.
///
/// Closures optionally store arguments that will be passed to the `Callable`
/// when it is run. This closure owns any arguments passed to it for storage,
/// copying or moving into its storage as needed. A `Fn` will pass a const
/// reference (`const&`) to those stored arguments to the `Callable`, allowing
/// it to receive them by const reference, or by value if it wants to copy. The
/// `Callable` may not mutate the stored arguments within the `Fn`.
///
/// Fn is convertible to FnMut and to FnOnce.
///
/// # Why can a "const" Fn convert to a mutable FnMut?
///
/// A FnMut is _allowed_ to mutate but for a Fn converted to a FnMut, it would
/// become a FnMut which chooses not to mutate its storage.
template <class R, class... CallArgs>
class Fn<R(CallArgs...)> : public FnMut<R(CallArgs...)> {
 public:
  /// Makes a Fn closure that holds a function pointer inline. No heap
  /// allocations are performed.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  constexpr static Fn with(F fn) noexcept {
    return Fn(static_cast<R (*)(CallArgs...)>(fn));
  }

  /// Makes a FnMut closure that holds a function pointer inline. No heap
  /// allocations are performed.
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  constexpr static Fn with(__private::SusBind<F>&& holder) noexcept {
    return Fn(static_cast<R (*)(CallArgs...)>(holder.lambda));
  }

  /// Makes a Fn closure that holds its functor and any stored arguments in a
  /// heap allocation.
  ///
  /// Requires that `F` can receive a const reference to each stored argument.
  template <::sus::concepts::callable::CallableObjectConst<CallArgs...> F>
  constexpr static Fn with(__private::SusBind<F>&& holder) noexcept {
    return Fn(__private::StorageConstructionFn,
              static_cast<F&&>(holder.lambda));
  }

  ~Fn() noexcept = default;

  Fn(Fn&&) noexcept = default;
  Fn& operator=(Fn&&) noexcept = default;

  Fn(const Fn&) noexcept = delete;
  Fn& operator=(const Fn&) noexcept = delete;

  // Runs the closure.
  inline R operator()(CallArgs&&... args) const& noexcept {
    return call(forward<CallArgs>(args)...);
  }

  // Runs the closure.
  R call(CallArgs&&...) const& noexcept;

 protected:
  template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
  Fn(F fn) noexcept;

  template <::sus::concepts::callable::CallableObjectConst<CallArgs...> F>
  Fn(__private::StorageConstructionFnType, F&& fn) noexcept;

  // This class may only have trivially-destructible storage and must not
  // do anything in its destructor, as `FnOnce` moves from itself, and it
  // would slice that off.
};

}  // namespace sus::fn
