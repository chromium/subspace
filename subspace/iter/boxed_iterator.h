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

template <class Iter, class Item>
class IteratorBase;

/// A BoxedIterator wraps another Iterator but pushes it onto the heap.
///
/// This makes the BoxedIterator itself be trivially relocatable, as it's just
/// some pointers to the heap.
///
/// BoxedIterator is only constructible from an iterator that is not trivially
/// relocatable.
template <class ItemT, size_t SubclassSize, size_t SubclassAlign,
          bool DoubleEnded>
class [[nodiscard]] [[sus_trivial_abi]] BoxedIterator final
    : public IteratorBase<
          BoxedIterator<ItemT, SubclassSize, SubclassAlign, DoubleEnded>,
          ItemT> {
 public:
  using Item = ItemT;

  template <::sus::mem::Move Iter>
  static BoxedIterator with(Iter&& iter) noexcept
    requires(
        ::sus::convert::SameOrSubclassOf<Iter*, IteratorBase<Iter, Item>*> &&
        !::sus::mem::relocate_by_memcpy<Iter>)
  {
    // IteratorBase also checks this. It's needed for correctness of the move
    // onto the heap.
    static_assert(std::is_final_v<Iter>);

    if constexpr (DoubleEnded) {
      return BoxedIterator(
          new Iter(::sus::move(iter)),  // Move it to the heap.
          [](void* boxed_iter) { delete static_cast<Iter*>(boxed_iter); },
          [](void* boxed_iter) {
            return static_cast<Iter*>(boxed_iter)->next();
          },
          [](void* boxed_iter) {
            return static_cast<Iter*>(boxed_iter)->next_back();
          });
    } else {
      return BoxedIterator(
          new Iter(::sus::move(iter)),  // Move it to the heap.
          [](void* boxed_iter) { delete static_cast<Iter*>(boxed_iter); },
          [](void* boxed_iter) {
            return static_cast<Iter*>(boxed_iter)->next();
          });
    }
  }

  BoxedIterator(BoxedIterator&& o) noexcept
      : iter_(::sus::mem::replace(mref(o.iter_), nullptr)),
        destroy_(::sus::mem::replace(mref(o.destroy_), nullptr)),
        next_(::sus::mem::replace(mref(o.next_), nullptr)),
        next_back_(::sus::mem::replace(mref(o.next_back_), nullptr)) {}
  BoxedIterator& operator=(BoxedIterator&& o) noexcept {
    if (destroy_) destroy_(iter_);
    iter_ = ::sus::mem::replace(mref(o.iter_), nullptr);
    destroy_ = ::sus::mem::replace(mref(o.destroy_), nullptr);
    next_ = ::sus::mem::replace(mref(o.next_), nullptr);
    next_back_ = ::sus::mem::replace(mref(o.next_back_), nullptr);
  }

  ~BoxedIterator() {
    if (destroy_) destroy_(iter_);
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return next_(iter_); }
  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(DoubleEnded)
  {
    return next_back_(iter_);
  }

 private:
  // DoubleEnded constructor.
  BoxedIterator(void* iter, void (*destroy)(void* boxed_iter),
                Option<Item> (*next)(void* boxed_iter),
                Option<Item> (*next_back)(void* boxed_iter))
      : iter_(iter), destroy_(destroy), next_(next), next_back_(next_back) {}
  // Not-DoubleEnded constructor.
  BoxedIterator(void* iter, void (*destroy)(void* boxed_iter),
                Option<Item> (*next)(void* boxed_iter))
      : iter_(iter), destroy_(destroy), next_(next), next_back_(nullptr) {}

  void* iter_;
  void (*destroy_)(void* boxed_iter);
  Option<Item> (*next_)(void* boxed_iter);
  // TODO: We could remove this field with a nested struct + template
  // specialization when DoubleEnded is false.
  Option<Item> (*next_back_)(void* boxed_iter);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(iter_),
                                  decltype(destroy_), decltype(next_),
                                  decltype(next_back_));
};

}  // namespace sus::iter
