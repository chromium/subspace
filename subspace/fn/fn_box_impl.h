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

#include "subspace/assertions/check.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/fn/__private/fn_box_storage.h"
#include "subspace/fn/fn_box_defn.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/replace.h"

namespace sus::fn {

template <class R, class... CallArgs>
template <::sus::fn::callable::FunctionPointerMatches<R, CallArgs...> F>
FnOnceBox<R(CallArgs...)>::FnOnceBox(F ptr) noexcept
    : fn_ptr_(ptr), type_(__private::FnBoxPointer) {
  ::sus::check(ptr != nullptr);
}

template <class R, class... CallArgs>
template <class ConstructionType,
          ::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
FnOnceBox<R(CallArgs...)>::FnOnceBox(ConstructionType construction,
                               F&& lambda) noexcept
    : type_(__private::Storage) {
  using FnBoxStorage = __private::FnBoxStorage<F>;
  // TODO: Allow overriding the global allocator? Use the allocator in place of
  // `new` and `delete` directly?
  auto* s = new FnBoxStorage(::sus::move(lambda));
  make_vtable(*s, construction);
  storage_ = s;
}

template <class R, class... CallArgs>
template <class FnBoxStorage>
void FnOnceBox<R(CallArgs...)>::make_vtable(
    FnBoxStorage& storage, __private::StorageConstructionFnOnceBoxType) noexcept {
  static __private::FnBoxStorageVtable<R, CallArgs...> vtable = {
      .call_once = &FnBoxStorage::template call_once<R, CallArgs...>,
      .call_mut = nullptr,
      .call = nullptr,
  };
  storage.vtable.insert(vtable);
}

template <class R, class... CallArgs>
template <class FnBoxStorage>
void FnOnceBox<R(CallArgs...)>::make_vtable(
    FnBoxStorage& storage, __private::StorageConstructionFnMutBoxType) noexcept {
  static __private::FnBoxStorageVtable<R, CallArgs...> vtable = {
      .call_once = &FnBoxStorage::template call_once<R, CallArgs...>,
      .call_mut = &FnBoxStorage::template call_mut<R, CallArgs...>,
      .call = nullptr,
  };
  storage.vtable.insert(vtable);
}

template <class R, class... CallArgs>
template <class FnBoxStorage>
void FnOnceBox<R(CallArgs...)>::make_vtable(
    FnBoxStorage& storage, __private::StorageConstructionFnBoxType) noexcept {
  static __private::FnBoxStorageVtable<R, CallArgs...> vtable = {
      .call_once = &FnBoxStorage::template call_once<R, CallArgs...>,
      .call_mut = &FnBoxStorage::template call_mut<R, CallArgs...>,
      .call = &FnBoxStorage::template call<R, CallArgs...>,
  };
  storage.vtable.insert(vtable);
}

template <class R, class... CallArgs>
FnOnceBox<R(CallArgs...)>::~FnOnceBox() noexcept {
  switch (type_) {
    // Note that the FnBoxPointer state is set when destroying from the never value
    // state.
    case __private::FnBoxPointer: break;
    case __private::Storage: {
      if (auto* s = ::sus::mem::replace_ptr(mref(storage_), nullptr); s)
        delete s;
      break;
    }
  }
}

template <class R, class... CallArgs>
FnOnceBox<R(CallArgs...)>::FnOnceBox(FnOnceBox&& o) noexcept : type_(o.type_) {
  switch (type_) {
    case __private::FnBoxPointer:
      ::sus::check(o.fn_ptr_);  // Catch use-after-move.
      fn_ptr_ = ::sus::mem::replace_ptr(mref(o.fn_ptr_), nullptr);
      break;
    case __private::Storage:
      ::sus::check(o.storage_);  // Catch use-after-move.
      storage_ = ::sus::mem::replace_ptr(mref(o.storage_), nullptr);
      break;
  }
}

template <class R, class... CallArgs>
FnOnceBox<R(CallArgs...)>& FnOnceBox<R(CallArgs...)>::operator=(FnOnceBox&& o) noexcept {
  switch (type_) {
    case __private::FnBoxPointer: break;
    case __private::Storage:
      if (auto* s = ::sus::mem::replace_ptr(mref(storage_), nullptr); s)
        delete s;
  }
  switch (type_ = o.type_) {
    case __private::FnBoxPointer:
      ::sus::check(o.fn_ptr_);  // Catch use-after-move.
      fn_ptr_ = ::sus::mem::replace_ptr(mref(o.fn_ptr_), nullptr);
      break;
    case __private::Storage:
      ::sus::check(o.storage_);  // Catch use-after-move.
      storage_ = ::sus::mem::replace_ptr(mref(o.storage_), nullptr);
      break;
  }
  return *this;
}

template <class R, class... CallArgs>
R FnOnceBox<R(CallArgs...)>::operator()(CallArgs... args) && noexcept {
  switch (type_) {
    case __private::FnBoxPointer: {
      ::sus::check(fn_ptr_);  // Catch use-after-move.
      auto* fn = ::sus::mem::replace_ptr(mref(fn_ptr_), nullptr);
      return fn(static_cast<CallArgs&&>(args)...);
    }
    case __private::Storage: {
      ::sus::check(storage_);  // Catch use-after-move.
      auto* storage = ::sus::mem::replace_ptr(mref(storage_), nullptr);
      auto& vtable =
          static_cast<const __private::FnBoxStorageVtable<R, CallArgs...>&>(
              storage->vtable.as_mut().unwrap());

      // Delete storage, after the call_once() is complete.
      //
      // TODO: `storage` and `storage_` should be owning smart pointers.
      struct DeleteStorage final {
        sus_clang_bug_54040(
            constexpr inline DeleteStorage(__private::FnBoxStorageBase* storage)
            : storage(storage){});
        ~DeleteStorage() { delete storage; }
        __private::FnBoxStorageBase* storage;
      } deleter(storage);

      return vtable.call_once(static_cast<__private::FnBoxStorageBase&&>(*storage),
                              forward<CallArgs>(args)...);
    }
  }
  ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
}

template <class R, class... CallArgs>
R FnMutBox<R(CallArgs...)>::operator()(CallArgs... args) & noexcept {
  using Super = FnOnceBox<R(CallArgs...)>;
  switch (Super::type_) {
    case __private::FnBoxPointer:
      ::sus::check(Super::fn_ptr_);  // Catch use-after-move.
      return Super::fn_ptr_(static_cast<CallArgs&&>(args)...);
    case __private::Storage: {
      ::sus::check(Super::storage_);  // Catch use-after-move.
      auto& vtable =
          static_cast<const __private::FnBoxStorageVtable<R, CallArgs...>&>(
              Super::storage_->vtable.as_mut().unwrap());
      return vtable.call_mut(
          static_cast<__private::FnBoxStorageBase&>(*Super::storage_),
          ::sus::forward<CallArgs>(args)...);
    }
  }
  ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
}

template <class R, class... CallArgs>
R FnBox<R(CallArgs...)>::operator()(CallArgs... args) const& noexcept {
  using Super = FnOnceBox<R(CallArgs...)>;
  switch (Super::type_) {
    case __private::FnBoxPointer:
      ::sus::check(Super::fn_ptr_);  // Catch use-after-move.
      return Super::fn_ptr_(static_cast<CallArgs&&>(args)...);
    case __private::Storage: {
      ::sus::check(Super::storage_);  // Catch use-after-move.
      auto& vtable =
          static_cast<const __private::FnBoxStorageVtable<R, CallArgs...>&>(
              Super::storage_->vtable.as_mut().unwrap());
      return vtable.call(
          static_cast<const __private::FnBoxStorageBase&>(*Super::storage_),
          ::sus::forward<CallArgs>(args)...);
    }
  }
  ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
}

}  // namespace sus::fn
