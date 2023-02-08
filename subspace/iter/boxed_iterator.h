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
#include "subspace/iter/iterator_concept.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"
// Doesn't include iterator_defn.h because it's included from there.

namespace sus::iter {

template <class Item>
class IteratorBase;
template <class Iter, class Item>
class IteratorImpl;

/// A BoxedIterator wraps another Iterator but pushes it onto the heap.
///
/// This makes the BoxedIterator itself be trivially relocatable, as it's just
/// some pointers to the heap.
///
/// BoxedIterator is only constructible from an iterator that is not trivially
/// relocatable.
template <class ItemT, size_t SubclassSize, size_t SubclassAlign>
class [[sus_trivial_abi]] BoxedIterator final
    : public IteratorImpl<BoxedIterator<ItemT, SubclassSize, SubclassAlign>,
                          ItemT> {
 public:
  using Item = ItemT;

  template <::sus::mem::Move IteratorSubclass>
  static BoxedIterator with(IteratorSubclass&& subclass) noexcept
    requires(::sus::convert::SameOrSubclassOf<
                 IteratorSubclass*, IteratorImpl<IteratorSubclass, ItemT>*> &&
             !::sus::mem::relocate_by_memcpy<IteratorSubclass>)
  {
    return BoxedIterator(
        *new IteratorSubclass(::sus::move(subclass)),  // Move it to the heap.
        [](IteratorBase<Item>& iter) {
          delete static_cast<IteratorSubclass*>(&iter);
        });
  }

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
  BoxedIterator(IteratorBase<Item>& iter,
                void (*destroy)(IteratorBase<Item>& boxed))
      : iter_(&iter), destroy_(destroy) {}

  IteratorBase<Item>* iter_;
  void (*destroy_)(IteratorBase<Item>& boxed);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(iter_),
                                  decltype(destroy_));
};

}  // namespace sus::iter
