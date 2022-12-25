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

#include "assertions/check.h"
#include "assertions/unreachable.h"
#include "fn/__private/fn_storage.h"
#include "fn/fn_defn.h"
#include "macros/__private/compiler_bugs.h"
#include "mem/replace.h"

namespace sus::fn {

template <class R, class... CallArgs>
template <::sus::fn::callable::FunctionPointerReturns<R, CallArgs...> F>
FnOnce<R(CallArgs...)>::FnOnce(F ptr) noexcept
    : fn_ptr_(ptr), type_(__private::FnPointer) {
  ::sus::check(ptr != nullptr);
}

template <class R, class... CallArgs>
template <class ConstructionType,
          ::sus::fn::callable::CallableObjectReturns<R, CallArgs...> F>
FnOnce<R(CallArgs...)>::FnOnce(ConstructionType construction,
                               F&& lambda) noexcept
    : type_(__private::Storage) {
  using FnStorage = __private::FnStorage<F>;
  // TODO: Allow overriding the global allocator? Use the allocator in place of
  // `new` and `delete` directly?
  auto* s = new FnStorage(::sus::move(lambda));
  make_vtable(*s, construction);
  storage_ = s;
}

template <class R, class... CallArgs>
template <class FnStorage>
void FnOnce<R(CallArgs...)>::make_vtable(
    FnStorage& storage, __private::StorageConstructionFnOnceType) noexcept {
  static __private::FnStorageVtable<R, CallArgs...> vtable = {
      .call_once = &FnStorage::template call_once<R, CallArgs...>,
      .call_mut = nullptr,
      .call = nullptr,
  };
  storage.vtable.insert(vtable);
}

template <class R, class... CallArgs>
template <class FnStorage>
void FnOnce<R(CallArgs...)>::make_vtable(
    FnStorage& storage, __private::StorageConstructionFnMutType) noexcept {
  static __private::FnStorageVtable<R, CallArgs...> vtable = {
      .call_once = &FnStorage::template call_once<R, CallArgs...>,
      .call_mut = &FnStorage::template call_mut<R, CallArgs...>,
      .call = nullptr,
  };
  storage.vtable.insert(vtable);
}

template <class R, class... CallArgs>
template <class FnStorage>
void FnOnce<R(CallArgs...)>::make_vtable(
    FnStorage& storage, __private::StorageConstructionFnType) noexcept {
  static __private::FnStorageVtable<R, CallArgs...> vtable = {
      .call_once = &FnStorage::template call_once<R, CallArgs...>,
      .call_mut = &FnStorage::template call_mut<R, CallArgs...>,
      .call = &FnStorage::template call<R, CallArgs...>,
  };
  storage.vtable.insert(vtable);
}

template <class R, class... CallArgs>
FnOnce<R(CallArgs...)>::~FnOnce() noexcept {
  switch (type_) {
    case __private::FnPointer: break;
    case __private::Storage: {
      if (auto* s = ::sus::mem::replace_ptr(mref(storage_), nullptr); s)
        delete s;
      break;
    }
  }
}

template <class R, class... CallArgs>
FnOnce<R(CallArgs...)>::FnOnce(FnOnce&& o) noexcept : type_(o.type_) {
  switch (type_) {
    case __private::FnPointer:
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
FnOnce<R(CallArgs...)>& FnOnce<R(CallArgs...)>::operator=(FnOnce&& o) noexcept {
  switch (type_) {
    case __private::FnPointer: break;
    case __private::Storage:
      if (auto* s = ::sus::mem::replace_ptr(mref(storage_), nullptr); s)
        delete s;
  }
  switch (type_ = o.type_) {
    case __private::FnPointer:
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
R FnOnce<R(CallArgs...)>::operator()(CallArgs&&... args) && noexcept {
  switch (type_) {
    case __private::FnPointer: {
      ::sus::check(fn_ptr_);  // Catch use-after-move.
      auto* fn = ::sus::mem::replace_ptr(mref(fn_ptr_), nullptr);
      return fn(static_cast<CallArgs&&>(args)...);
    }
    case __private::Storage: {
      ::sus::check(storage_);  // Catch use-after-move.
      auto* storage = ::sus::mem::replace_ptr(mref(storage_), nullptr);
      auto& vtable = static_cast<const __private::FnStorageVtable<R, CallArgs...>&>(
          storage->vtable.unwrap_mut());

      // Delete storage, after the call_once() is complete.
      //
      // TODO: `storage` and `storage_` should be owning smart pointers.
      struct DeleteStorage final {
        sus_clang_bug_54040(
            constexpr inline DeleteStorage(__private::FnStorageBase* storage)
            : storage(storage){});
        ~DeleteStorage() { delete storage; }
        __private::FnStorageBase* storage;
      } deleter(storage);

      return vtable.call_once(static_cast<__private::FnStorageBase&&>(*storage),
                              forward<CallArgs>(args)...);
    }
  }
  ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
}

template <class R, class... CallArgs>
R FnMut<R(CallArgs...)>::operator()(CallArgs&&... args) & noexcept {
  using Super = FnOnce<R(CallArgs...)>;
  switch (Super::type_) {
    case __private::FnPointer:
      ::sus::check(Super::fn_ptr_);  // Catch use-after-move.
      return Super::fn_ptr_(static_cast<CallArgs&&>(args)...);
    case __private::Storage: {
      ::sus::check(Super::storage_);  // Catch use-after-move.
      auto& vtable = static_cast<const __private::FnStorageVtable<R, CallArgs...>&>(
          Super::storage_->vtable.unwrap_mut());
      return vtable.call_mut(
          static_cast<__private::FnStorageBase&>(*Super::storage_),
          ::sus::forward<CallArgs>(args)...);
    }
  }
  ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
}

template <class R, class... CallArgs>
R Fn<R(CallArgs...)>::operator()(CallArgs&&... args) const& noexcept {
  using Super = FnOnce<R(CallArgs...)>;
  switch (Super::type_) {
    case __private::FnPointer:
      ::sus::check(Super::fn_ptr_);  // Catch use-after-move.
      return Super::fn_ptr_(static_cast<CallArgs&&>(args)...);
    case __private::Storage: {
      ::sus::check(Super::storage_);  // Catch use-after-move.
      auto& vtable = static_cast<const __private::FnStorageVtable<R, CallArgs...>&>(
          Super::storage_->vtable.unwrap_mut());
      return vtable.call(
          static_cast<const __private::FnStorageBase&>(*Super::storage_),
          ::sus::forward<CallArgs>(args)...);
    }
  }
  ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
}

}  // namespace sus::fn
