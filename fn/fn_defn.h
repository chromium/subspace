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
#include "fn/__private/fn_concepts.h"
#include "fn/__private/run_type.h"
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
  // TODO: Method pointers, with and without storage.

  template <::sus::concepts::callable::FunctionPointer F>
    requires(std::is_same_v<
             decltype(std::declval<F>()(std::declval<CallArgs>()...)), R>)
  constexpr static FnOnce with_fn_pointer(F fn) {
    return FnOnce(static_cast<R (*)(CallArgs...)>(fn));
  }

  template <::sus::concepts::callable::Callable F, class... StoredArgs>
    requires(__private::CheckFnOnceCompatible<F, R>(
        __private::runtype_args_for<F, sizeof...(StoredArgs)>(),
        __private::Pack<std::decay_t<StoredArgs>&&...>(),
        __private::Pack<CallArgs...>()))
  constexpr static FnOnce with_storage(F fn, StoredArgs&&... stored) {
    return FnOnce(__private::StorageConstructionFnOnce,
                  static_cast<decltype(fn)&&>(fn),
                  (forward<StoredArgs>(stored))...);
  }

  ~FnOnce();

  FnOnce(FnOnce&& o);
  FnOnce& operator=(FnOnce&& o);

  inline R operator()(CallArgs&&... args) && {
    return static_cast<decltype(*this)&&>(*this).call_once(
        forward<CallArgs>(args)...);
  }

  R call_once(CallArgs&&...) &&;

 protected:
  // Function pointer constructor.
  template <::sus::concepts::callable::FunctionPointer F>
  FnOnce(F fn);

  template <class ConstructionType,
            ::sus::concepts::callable::FunctionPointer F, class... StoredArgs>
  FnOnce(ConstructionType, F fn, StoredArgs&&... stored);

  template <class ConstructionType, ::sus::concepts::callable::CallableObject F,
            class... StoredArgs>
  FnOnce(ConstructionType, F&& fn, StoredArgs&&... stored);

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
  void make_vtable(FnStorage&, __private::StorageConstructionFnOnceType);
  template <class FnStorage>
  void make_vtable(FnStorage&, __private::StorageConstructionFnMutType);
  template <class FnStorage>
  void make_vtable(FnStorage&, __private::StorageConstructionFnType);

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
  template <::sus::concepts::callable::FunctionPointer F>
    requires(std::is_same_v<
             decltype(std::declval<F>()(std::declval<CallArgs>()...)), R>)
  constexpr static FnMut with_fn_pointer(F fn) {
    return FnMut(static_cast<R (*)(CallArgs...)>(fn));
  }

  template <::sus::concepts::callable::Callable F, class... StoredArgs>
    requires(__private::FnCompatibleMut<
             F, R, __private::Pack<std::decay_t<StoredArgs>...>,
             __private::Pack<CallArgs...>>)
  constexpr static FnMut with_storage(F fn, StoredArgs&&... stored) {
    return FnMut(__private::StorageConstructionFnMut,
                 static_cast<decltype(fn)&&>(fn),
                 (forward<StoredArgs>(stored))...);
  }

  ~FnMut() = default;

  FnMut(FnMut&&) = default;
  FnMut& operator=(FnMut&&) = default;

  inline R operator()(CallArgs&&... args) & {
    return static_cast<decltype(*this)&&>(*this).call_mut(
        forward<CallArgs>(args)...);
  }

  R call_mut(CallArgs&&...) &;

 protected:
  template <::sus::concepts::callable::FunctionPointer F>
  FnMut(F fn);

  template <class ConstructionType, ::sus::concepts::callable::Callable F,
            class... StoredArgs>
  FnMut(ConstructionType, F&& fn, StoredArgs&&... args);
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
  template <::sus::concepts::callable::FunctionPointer F>
    requires(std::is_same_v<
             decltype(std::declval<F>()(std::declval<CallArgs>()...)), R>)
  constexpr static Fn with_fn_pointer(F fn) {
    return Fn(static_cast<R (*)(CallArgs...)>(fn));
  }

  template <::sus::concepts::callable::Callable F, class... StoredArgs>
    requires(__private::FnCompatibleConst<
             F, R, __private::Pack<std::decay_t<StoredArgs>...>,
             __private::Pack<CallArgs...>>)
  constexpr static Fn with_storage(F fn, StoredArgs&&... stored) {
    return Fn(__private::StorageConstructionFn, static_cast<decltype(fn)&&>(fn),
              (forward<StoredArgs>(stored))...);
  }

  ~Fn() = default;

  Fn(Fn&&) = default;
  Fn& operator=(Fn&&) = default;

  inline R operator()(CallArgs&&... args) const& {
    return static_cast<decltype(*this)&&>(*this).call(
        forward<CallArgs>(args)...);
  }
  R call(CallArgs&&...) const&;

 protected:
  template <::sus::concepts::callable::FunctionPointer F>
  Fn(F fn);

  template <::sus::concepts::callable::Callable F, class... StoredArgs>
  Fn(__private::StorageConstructionFnType, F&& fn, StoredArgs&&... args);

  // This class may only have trivially-destructible storage and must not
  // do anything in its destructor, as `FnOnce` moves from itself, and it
  // would slice that off.
};

}  // namespace sus::fn
