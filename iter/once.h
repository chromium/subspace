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

#include "iter/iterator_defn.h"
#include "mem/__private/relocatable_storage.h"
#include "mem/move.h"

namespace sus::iter {

using ::sus::iter::IteratorBase;
using ::sus::mem::__private::RelocatableStorage;
using ::sus::option::Option;

/// An IteratorBase implementation that walks over at most a single Item.
template <class Item>
class [[sus_trivial_abi]] Once : public IteratorBase<Item> {
 public:
  Option<Item> next() noexcept final { return single_.take(); }

 protected:
  Once(Option<Item>&& single) : single_(::sus::move(single)) {}

 private:
  RelocatableStorage<Item> single_;

  sus_class_maybe_trivial_relocatable_types(unsafe_fn, decltype(single_));
};

template <class Item>
inline Once<Item> once(Option<Item>&& single)
  requires(std::is_move_constructible_v<Item>)
{
  return Once(::sus::move(single));
}

}  // namespace sus::iter
