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
#include "subspace/mem/relocate.h"

namespace sus::iter {

/// An Iterator that never returns an `Item`.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] Empty final
    : public IteratorBase<Empty<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  /// Constructs an `Empty` iterator, which is an empty iterator that returns
  /// nothing.
  constexpr Empty() = default;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept { return sus::Option<Item>(); }
  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return SizeHint(0u, ::sus::Option<::sus::num::usize>::with(0u));
  }
  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept { return sus::Option<Item>(); }
  // sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept { return 0u; }

 private:
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn);
};

}  // namespace sus::iter
