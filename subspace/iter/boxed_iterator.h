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

#include "subspace/convert/subclass.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"
// Doesn't include iterator_defn.h because it's included from there.

namespace sus::iter {

template <class Item, size_t SubclassSize, size_t SubclassAlign>
struct [[sus_trivial_abi]] BoxedIterator : public IteratorBase<Item> {
  BoxedIterator(IteratorBase<Item>& iter,
                void (*destroy)(IteratorBase<Item>& boxed))
      : iter_(&iter), destroy_(destroy) {}

  BoxedIterator(BoxedIterator&& o) noexcept
      : iter_(::sus::mem::replace_ptr(mref(o.iter_), nullptr)),
        destroy_(::sus::mem::replace_ptr(mref(o.destroy_), nullptr)) {}
  BoxedIterator& operator=(BoxedIterator&& o) noexcept {
    if (destroy_) destroy_(*iter_);
    iter_ = ::sus::mem::replace_ptr(mref(o.iter_), nullptr);
    destroy_ = ::sus::mem::replace_ptr(mref(o.destroy_), nullptr);
  }

  ~BoxedIterator() {
    if (destroy_) destroy_(*iter_);
  }

  Option<Item> next() noexcept final { return iter_->next(); }

 private:
  IteratorBase<Item>* iter_;
  void (*destroy_)(IteratorBase<Item>& boxed);

  sus_class_assert_trivial_relocatable_types(::sus::marker::unsafe_fn,
                                             decltype(iter_),
                                             decltype(destroy_));
};

/// Make a BoxedIterator.
///
/// The BoxedIterator's internals will be on the heap, making the BoxedIterator
/// itself be trivially relocatable, as it's just some pointers to the heap.
template <::sus::mem::Move IteratorSubclass, int&...,
          class SubclassItem = typename IteratorSubclass::Item,
          class BoxedIteratorType = BoxedIterator<
              SubclassItem, ::sus::mem::size_of<IteratorSubclass>(),
              alignof(IteratorSubclass)>>
inline BoxedIteratorType make_boxed_iterator(IteratorSubclass&& subclass)
  requires(::sus::convert::SameOrSubclassOf<IteratorSubclass*,
                                            IteratorBase<SubclassItem>*> &&
           !::sus::mem::relocate_by_memcpy<IteratorSubclass>)
{
  return BoxedIteratorType(*new IteratorSubclass(::sus::move(subclass)),
                           [](IteratorBase<SubclassItem>& iter) {
                             delete reinterpret_cast<IteratorSubclass*>(&iter);
                           });
}

}  // namespace sus::iter
