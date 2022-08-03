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

#include "mem/relocate.h"
#include "mem/mref.h"
#include "option/option.h"

namespace sus::mem::__private {

template <class T>
struct [[sus_trivial_abi]] RelocatableStorage;

template <class T>
  requires(!::sus::mem::relocate_one_by_memcpy<T>)
struct [[sus_trivial_abi]] RelocatableStorage<T> final {
  RelocatableStorage(Option<T>&& t)
      : heap_(static_cast<decltype(t)&&>(t).and_then([](T&& t) {
          return Option<T&>::some(mref(*new T(static_cast<T&&>(t))));
        })) {}

  ~RelocatableStorage() {
    heap_.take().and_then(
        [](T& t) {  // TODO: Use inspect() instead if we add it?
          delete &t;
          return Option<T&>::none();
        });
  }

  RelocatableStorage(RelocatableStorage&& o) : heap_(o.heap_.take()) {}

  RelocatableStorage& operator=(RelocatableStorage&& o) {
    heap_.take().and_then(
        [](T& t) {  // TODO: Use inspect() instead if we add it?
          delete &t;
          return Option<T&>::none();
        });
    heap_ = o.heap_.take();
    return *this;
  }

  T& storage_mut() & { return heap_.as_mut().unwrap(); }

  Option<T> take() & {
    return heap_.take().map([](T& t) {
      T take = static_cast<T&&>(t);
      delete &t;
      return take;
    });
  }

  Option<T&> heap_;

  sus_class_assert_trivial_relocatable_types(unsafe_fn, decltype(heap_));
};

template <class T>
  requires(::sus::mem::relocate_one_by_memcpy<T>)
struct [[sus_trivial_abi]] RelocatableStorage<T> final {
  RelocatableStorage(Option<T>&& t) : stack_(static_cast<decltype(t)&&>(t)) {}

  // TODO: Do memcpy instead of take() when not in a constexpr context.
  RelocatableStorage(RelocatableStorage&& o) : stack_(o.stack_.take()) {}
  RelocatableStorage& operator=(RelocatableStorage&& o) {
    stack_ = o.stack_.take();
    return *this;
  }

  T& storage_mut() & { return stack_.as_mut().unwrap(); }

  Option<T> take() & { return stack_.take(); }

  // TODO: Remove the Option from here, put the Option at the callers where it's
  // needed.
  Option<T> stack_;

  sus_class_assert_trivial_relocatable_types(unsafe_fn, decltype(stack_));
};

}  // namespace sus::mem::__private
