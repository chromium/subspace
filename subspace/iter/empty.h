// Copyright 2023 Google LLC
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

#include "subspace/iter/iterator_defn.h"
#include "subspace/option/option.h"

namespace sus::iter {

/// An IteratorBase implementation that returns nothing.
template <class Item>
class [[sus_trivial_abi]] Empty : public IteratorBase<Item> {
 public:
  constexpr Option<Item> next() noexcept final {
    return sus::Option<Item>::none();
  }

 protected:
  constexpr Empty() = default;

 private:
  template <class U>
  friend constexpr inline Iterator<Empty<U>> empty() noexcept;

  constexpr static auto with() { return Iterator<Empty<Item>>(); }

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn);
};

/// Creates an empty iterator that never returns an `Item`.
template <class Item>
constexpr inline Iterator<Empty<Item>> empty() noexcept {
  return Empty<Item>::with();
}

}  // namespace sus::iter
