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

#include "subspace/iter/iterator_defn.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"

namespace sus::iter {

using ::sus::option::Option;

/// An Iterator that walks over at most a single Item.
template <class ItemT>
class [[nodiscard]] Once final : public IteratorImpl<Once<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  static Once with(Option<Item>&& o) noexcept { return Once(::sus::move(o)); }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept final { return single_.take(); }
  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept { return single_.take(); }

 private:
  Once(Option<Item>&& single) : single_(::sus::move(single)) {}

  Option<Item> single_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(single_));
};

}  // namespace sus::iter
