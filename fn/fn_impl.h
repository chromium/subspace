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

#include "assertions/unreachable.h"
#include "fn/__private/fn_storage.h"
#include "fn/fn_defn.h"
#include "mem/replace.h"

namespace sus::fn {

template <class R, class... CallArgs>
template <::sus::concepts::callable::FunctionPointerReturns<R, CallArgs...> F>
FnOnce<R(CallArgs...)>::FnOnce(F ptr) noexcept
    : type_(__private::FnPointer), fn_ptr_(ptr) {}

template <class R, class... CallArgs>
template <class ConstructionType,
          ::sus::concepts::callable::LambdaReturns<R, CallArgs...> F>
FnOnce<R(CallArgs...)>::FnOnce(ConstructionType construction,
                               F&& lambda) noexcept
    : type_(__private::Storage) {
  using FnStorage = __private::FnStorage<F>;
  // TODO: Allow overriding the global allocator? Use the allocator in place of
  // `new` and `delete` directly?
  auto* s = new FnStorage(static_cast<F&&>(lambda));
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
    case __private::MovedFrom:
      break;
    case __private::FnPointer:
      break;
    case __private::Storage: {
      if (auto* p = ::sus::mem::replace_ptr(mref(storage_), nullptr); p)
        delete p;
      break;
    }
  }
}

template <class R, class... CallArgs>
FnOnce<R(CallArgs...)>::FnOnce(FnOnce&& o) noexcept
    : type_(::sus::mem::replace(mref(o.type_), __private::MovedFrom)) {
  switch (type_) {
    case __private::MovedFrom:
      ::sus::panic();
    case __private::FnPointer:
      fn_ptr_.fn_ = ::sus::mem::replace_ptr(mref(o.fn_ptr_.fn_), nullptr);
      break;
    case __private::Storage:
      storage_ = ::sus::mem::replace_ptr(mref(o.storage_), nullptr);
      break;
  }
}

template <class R, class... CallArgs>
FnOnce<R(CallArgs...)>& FnOnce<R(CallArgs...)>::operator=(FnOnce&& o) noexcept {
  switch (type_) {
    case __private::MovedFrom:
      break;
    case __private::FnPointer:
      break;
    case __private::Storage:
      if (auto* s = ::sus::mem::replace_ptr(mref(storage_), nullptr); s)
        delete s;
  }
  switch (type_ = ::sus::mem::replace(mref(o.type_), __private::MovedFrom)) {
    case __private::MovedFrom:
      ::sus::panic();
    case __private::FnPointer:
      fn_ptr_.fn_ = ::sus::mem::replace_ptr(mref(o.fn_ptr_.fn_), nullptr);
      break;
    case __private::Storage:
      storage_ = ::sus::mem::replace_ptr(mref(o.storage_), nullptr);
      break;
  }
  return *this;
}

template <class R, class... CallArgs>
R FnOnce<R(CallArgs...)>::operator()(CallArgs&&... args) && noexcept {
  switch (::sus::mem::replace(mref(type_), __private::MovedFrom)) {
    case __private::MovedFrom:
      ::sus::panic();
    case __private::FnPointer: {
      auto* fn = ::sus::mem::replace_ptr(mref(fn_ptr_.fn_), nullptr);
      return fn(static_cast<CallArgs&&>(args)...);
    }
    case __private::Storage: {
      auto* storage = ::sus::mem::replace_ptr(mref(storage_), nullptr);
      ::sus::check(storage);
      auto& vtable = static_cast<__private::FnStorageVtable<R, CallArgs...>&>(
          storage->vtable.unwrap_mut());

      // Delete storage, after the call_once() is complete.
      //
      // TODO: `storage` and `storage_` should be owning smart pointers.
      struct DeleteStorage {
        ~DeleteStorage() { delete storage; }
        __private::FnStorageBase* storage;
      } deleter(storage);

      return vtable.call_once(static_cast<__private::FnStorageBase&&>(*storage),
                              forward<CallArgs>(args)...);
    }
  }
  ::sus::unreachable_unchecked(unsafe_fn);
}

template <class R, class... CallArgs>
R FnMut<R(CallArgs...)>::operator()(CallArgs&&... args) & noexcept {
  using Super = FnOnce<R(CallArgs...)>;
  switch (Super::type_) {
    case __private::MovedFrom:
      ::sus::panic();
    case __private::FnPointer:
      return Super::fn_ptr_.fn_(static_cast<CallArgs&&>(args)...);
    case __private::Storage: {
      auto& vtable = static_cast<__private::FnStorageVtable<R, CallArgs...>&>(
          Super::storage_->vtable.unwrap_mut());
      return vtable.call_mut(
          static_cast<__private::FnStorageBase&>(*Super::storage_),
          forward<CallArgs>(args)...);
    }
  }
  ::sus::unreachable_unchecked(unsafe_fn);
}

template <class R, class... CallArgs>
R Fn<R(CallArgs...)>::operator()(CallArgs&&... args) const& noexcept {
  using Super = FnOnce<R(CallArgs...)>;
  switch (Super::type_) {
    case __private::MovedFrom:
      ::sus::panic();
    case __private::FnPointer:
      return Super::fn_ptr_.fn_(static_cast<CallArgs&&>(args)...);
    case __private::Storage: {
      auto& vtable = static_cast<__private::FnStorageVtable<R, CallArgs...>&>(
          Super::storage_->vtable.unwrap_mut());
      return vtable.call(
          static_cast<const __private::FnStorageBase&>(*Super::storage_),
          forward<CallArgs>(args)...);
    }
  }
  ::sus::unreachable_unchecked(unsafe_fn);
}

}  // namespace sus::fn
