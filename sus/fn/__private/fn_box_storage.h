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

#include <functional>

#include "sus/option/option.h"

namespace sus::fn::__private {

struct FnBoxStorageVtableBase {};

struct FnBoxStorageBase {
  virtual ~FnBoxStorageBase() = default;

  // Should be to a static lifetime pointee.
  Option<const FnBoxStorageVtableBase&> vtable;
};

template <class R, class... CallArgs>
struct FnBoxStorageVtable final : public FnBoxStorageVtableBase {
  R (*call_once)(__private::FnBoxStorageBase&&, CallArgs...);
  R (*call_mut)(__private::FnBoxStorageBase&, CallArgs...);
  R (*call)(const __private::FnBoxStorageBase&, CallArgs...);
};

template <class F>
class FnBoxStorage final : public FnBoxStorageBase {
 public:
  ~FnBoxStorage() override = default;

  constexpr FnBoxStorage(F&& callable)
    requires(!(std::is_member_function_pointer_v<std::remove_reference_t<F>> ||
               std::is_member_object_pointer_v<std::remove_reference_t<F>>))
      : callable_(::sus::move(callable)) {}
  constexpr FnBoxStorage(F ptr)
    requires(std::is_member_function_pointer_v<F> ||
             std::is_member_object_pointer_v<F>)
      : callable_(ptr) {}

  template <class R, class... CallArgs>
  static R call(const FnBoxStorageBase& self_base, CallArgs... callargs) {
    const auto& self = static_cast<const FnBoxStorage&>(self_base);
    return std::invoke(self.callable_, forward<CallArgs>(callargs)...);
  }

  template <class R, class... CallArgs>
  static R call_mut(FnBoxStorageBase& self_base, CallArgs... callargs) {
    auto& self = static_cast<FnBoxStorage&>(self_base);
    return std::invoke(self.callable_, forward<CallArgs>(callargs)...);
  }

  template <class R, class... CallArgs>
  static R call_once(FnBoxStorageBase&& self_base, CallArgs... callargs) {
    auto&& self = static_cast<FnBoxStorage&&>(self_base);
    return std::invoke(::sus::move(self.callable_),
                       forward<CallArgs>(callargs)...);
  }

  F callable_;
};

}  // namespace sus::fn::__private
