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

#include "subspace/option/option.h"

namespace sus::fn::__private {

struct BoxFnStorageVtableBase {};

struct BoxFnStorageBase {
  // Should be to a static lifetime pointee.
  Option<const BoxFnStorageVtableBase&> vtable = Option<const BoxFnStorageVtableBase&>::none();
};

template <class R, class... CallArgs>
struct BoxFnStorageVtable final : public BoxFnStorageVtableBase {
  R (*call_once)(__private::BoxFnStorageBase&&, CallArgs...);
  R (*call_mut)(__private::BoxFnStorageBase&, CallArgs...);
  R (*call)(const __private::BoxFnStorageBase&, CallArgs...);
};

template <class F>
class BoxFnStorage final : public BoxFnStorageBase {
 public:
  constexpr BoxFnStorage(F&& callable) : callable_(::sus::move(callable)) {}

  template <class R, class... CallArgs>
  static R call(const BoxFnStorageBase& self_base, CallArgs... callargs) {
    const auto& self = static_cast<const BoxFnStorage&>(self_base);
    return self.callable_(forward<CallArgs>(callargs)...);
  }

  template <class R, class... CallArgs>
  static R call_mut(BoxFnStorageBase& self_base, CallArgs... callargs) {
    auto& self = static_cast<BoxFnStorage&>(self_base);
    return self.callable_(forward<CallArgs>(callargs)...);
  }

  template <class R, class... CallArgs>
  static R call_once(BoxFnStorageBase&& self_base, CallArgs... callargs) {
    auto&& self = static_cast<BoxFnStorage&&>(self_base);
    return ::sus::move(self.callable_)(forward<CallArgs>(callargs)...);
  }

  F callable_;
};

}  // namespace sus::fn::__private
