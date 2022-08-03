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
#include "mem/relocate.h"

namespace sus::iter {

template <class Item, size_t SubclassSize, size_t SubclassAlign>
struct SizedIterator final {
  SizedIterator(void (*destroy)(char& sized)) : destroy(destroy) {}

  SizedIterator(SizedIterator&& o) : destroy(o.destroy) {
    o.destroy = nullptr;
    memcpy(sized, o.sized, SubclassSize);
  }
  SizedIterator& operator=(SizedIterator&& o) = delete;

  ~SizedIterator() {
    if (destroy) destroy(*sized);
  }

  IteratorBase<Item>& iterator_mut() {
    return *reinterpret_cast<IteratorBase<Item>*>(sized);
  }

  alignas(SubclassAlign) char sized[SubclassSize];
  void (*destroy)(char& sized);
};

template <::sus::mem::Moveable IteratorSubclass, int&...,
          class SubclassItem = typename IteratorSubclass::Item,
          class SizedIteratorType =
              SizedIterator<SubclassItem, sizeof(IteratorSubclass),
                            alignof(IteratorSubclass)>>
inline SizedIteratorType make_sized_iterator(IteratorSubclass&& subclass)
    // TODO: write a sus::is_subclass?
  requires(
      std::is_convertible_v<IteratorSubclass&, IteratorBase<SubclassItem>&>)
{
  static_assert(::sus::mem::relocate_one_by_memcpy<IteratorSubclass>);
  auto it = SizedIteratorType([](char& sized) {
    reinterpret_cast<IteratorSubclass&>(sized).~IteratorSubclass();
  });
  new (it.sized) IteratorSubclass(::sus::move(subclass));
  return it;
}

}  // namespace sus::iter
