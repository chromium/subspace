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

#include <stddef.h>
// TODO: Replace std::index_sequence to remove this heavy include.
#include <utility>

#include "fn/__private/flex_ref.h"
#include "fn/__private/run_type.h"
#include "option/option.h"

namespace sus::fn::__private {

struct StoredArgsStorageBase {};

template <size_t N, class... T>
struct StoredArgsStorage;

template <>
struct StoredArgsStorage<0> {};

template <class T>
struct StoredArgsStorage<1, T> {
  template <size_t I>
  using Type = T;
  T t;
};

template <size_t N, class T, class... MoreT>
struct StoredArgsStorage<N, T, MoreT...> : StoredArgsStorage<N - 1, MoreT...> {
  using Super = StoredArgsStorage<N - 1, MoreT...>;
  template <size_t I>
  using Type =
      std::conditional_t<I == 0, T, typename Super::template Type<I - 1>>;

  template <class U, class... MoreU>
  StoredArgsStorage(U&& t, MoreU&&... more)
      : Super(forward<MoreU>(more)...), t(forward<U>(t)) {}
  T t;
};

template <class Storage, size_t I>
struct StorageAccess {
  static constexpr const auto& get(const Storage& storage) {
    return StorageAccess<typename Storage::Super, I - 1>::get(storage);
  }

  static constexpr auto& get_mut(Storage& storage) {
    return StorageAccess<typename Storage::Super, I - 1>::get_mut(storage);
  }

  static constexpr auto&& get_once(Storage&& storage) {
    return StorageAccess<typename Storage::Super, I - 1>::get_once(
        static_cast<Storage&&>(storage));
  }
};

template <class Storage>
struct StorageAccess<Storage, 0> {
  static constexpr const auto& get(const Storage& storage) { return storage.t; }

  static constexpr auto& get_mut(Storage& storage) { return storage.t; }

  static constexpr auto&& get_once(Storage&& storage) {
    return static_cast<decltype(storage.t)&&>(storage.t);
  }
};

struct FnStorageVtableBase {};

struct FnStorageBase {
  // Should be to a static lifetime pointee.
  Option<FnStorageVtableBase&> vtable = Option<FnStorageVtableBase&>::none();
};

template <class R, class... CallArgs>
struct FnStorageVtable : public FnStorageVtableBase {
  R (*call_once)(__private::FnStorageBase&&, CallArgs...);
  R (*call_mut)(__private::FnStorageBase&, CallArgs...);
  R (*call)(const __private::FnStorageBase&, CallArgs...);
};

template <sus::concepts::callable::Callable F, class... StoredArgs>
class FnStorage : public FnStorageBase {
  using Storage = StoredArgsStorage<sizeof...(StoredArgs), StoredArgs...>;

 public:
  template <class... PassedStoredArgs>
  constexpr FnStorage(F&& callable, PassedStoredArgs&&... args)
      : callable_(static_cast<F&&>(callable)),
        stored_(forward<PassedStoredArgs>(args)...) {}

  template <class R, class... CallArgs>
  static R call(const FnStorageBase& self_base, CallArgs... callargs) {
    const auto& self = static_cast<const FnStorage&>(self_base);
    return CallImpl<CallArgs...>::template call_impl<R>(
        self.callable_, self.stored_, std::index_sequence_for<StoredArgs...>(),
        forward<CallArgs>(callargs)...);
  }

  template <class R, class... CallArgs>
  static R call_mut(FnStorageBase& self_base, CallArgs... callargs) {
    auto& self = static_cast<FnStorage&>(self_base);
    return CallImpl<CallArgs...>::template call_mut_impl<R>(
        self.callable_, self.stored_, std::index_sequence_for<StoredArgs...>(),
        forward<CallArgs>(callargs)...);
  }

  template <class R, class... CallArgs>
  static R call_once(FnStorageBase&& self_base, CallArgs... callargs) {
    auto&& self = static_cast<FnStorage&&>(self_base);
    return CallImpl<CallArgs...>::template call_once_impl<R>(
        static_cast<F&&>(self.callable_), static_cast<Storage&&>(self.stored_),
        std::index_sequence_for<StoredArgs...>(),
        runtype_args_for<F, sizeof...(StoredArgs)>(),
        forward<CallArgs>(callargs)...);
  }

 private:
  // We can only have one variadic argument set per template, so we have to
  // split the `StoredArgs` and `CallArgs` onto separate types, and the
  // `Indices` onto the template function. Thus, we next these functions inside
  // the CallImpl type, giving us 3 levels of templates.
  template <class... CallArgs>
  struct CallImpl {
    template <class R, size_t... Indicies>
    static inline R call_impl(const F& callable_, const Storage& storage,
                              std::index_sequence<Indicies...>,
                              CallArgs&&... args) {
      return RunType<F>::call(callable_,
                              StorageAccess<Storage, Indicies>::get(storage)...,
                              forward<CallArgs>(args)...);
    }

    template <class R, size_t... Indicies>
    static inline R call_mut_impl(F& callable_, Storage& storage,
                                  std::index_sequence<Indicies...>,
                                  CallArgs&&... args) {
      return RunType<F>::call(
          callable_, StorageAccess<Storage, Indicies>::get_mut(storage)...,
          forward<CallArgs>(args)...);
    }

    template <class R, size_t... Indicies, class... ReceiverArgs>
    static inline R call_once_impl(F&& callable_, Storage&& storage,
                                   std::index_sequence<Indicies...>,
                                   Pack<ReceiverArgs...>, CallArgs&&... args) {
      return RunType<F>::call(
          static_cast<F&&>(callable_),
          FlexRef<ReceiverArgs>(StorageAccess<Storage, Indicies>::get_once(
              static_cast<Storage&&>(storage)))...,
          forward<CallArgs>(args)...);
    }
  };

  F callable_;
  [[no_unique_address]] Storage stored_;
};

}  // namespace sus::fn::__private
