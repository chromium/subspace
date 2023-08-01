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

#include "sus/ptr/subclass.h"
#include "sus/iter/iterator_concept.h"
#include "sus/iter/size_hint.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/mem/size_of.h"
#include "sus/option/option.h"
// Doesn't include iterator_defn.h because it's included from there.

namespace sus::iter {

/// A BoxedIterator wraps another Iterator but pushes it onto the heap.
///
/// This makes the BoxedIterator itself be trivially relocatable, as it's just
/// some pointers to the heap.
///
/// BoxedIterator is only constructible from an iterator that is not trivially
/// relocatable.
///
/// This type is returned from `Iterator::box()`.
template <class ItemT, size_t SubclassSize, size_t SubclassAlign, bool Clone,
          bool DoubleEnded, bool ExactSize>
class [[nodiscard]] [[sus_trivial_abi]] BoxedIterator final
    : public IteratorBase<BoxedIterator<ItemT, SubclassSize, SubclassAlign,
                                        Clone, DoubleEnded, ExactSize>,
                          ItemT> {
 public:
  using Item = ItemT;

  BoxedIterator(BoxedIterator&& o) noexcept
      : iter_(::sus::mem::replace(o.iter_, nullptr)),
        destroy_(::sus::mem::replace(o.destroy_, nullptr)),
        clone_(::sus::mem::replace(o.clone_, nullptr)),
        next_(::sus::mem::replace(o.next_, nullptr)),
        next_back_(::sus::mem::replace(o.next_back_, nullptr)),
        size_hint_(::sus::mem::replace(o.size_hint_, nullptr)),
        exact_size_hint_(
            ::sus::mem::replace(o.exact_size_hint_, nullptr)) {}
  BoxedIterator& operator=(BoxedIterator&& o) noexcept {
    if (destroy_) destroy_(iter_);
    iter_ = ::sus::mem::replace(o.iter_, nullptr);
    destroy_ = ::sus::mem::replace(o.destroy_, nullptr);
    clone_ = ::sus::mem::replace(o.clone_, nullptr);
    next_ = ::sus::mem::replace(o.next_, nullptr);
    next_back_ = ::sus::mem::replace(o.next_back_, nullptr);
    size_hint_ = ::sus::mem::replace(o.size_hint_, nullptr);
    exact_size_hint_ = ::sus::mem::replace(o.exact_size_hint_, nullptr);
  }

  ~BoxedIterator() {
    if (destroy_) destroy_(iter_);
  }

  // sus::mem::Clone trait.
  BoxedIterator clone() const noexcept
    requires(Clone)
  {
    return clone_(iter_);
  }
  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return next_(iter_); }
  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept { return size_hint_(iter_); }
  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(DoubleEnded)
  {
    return next_back_(iter_);
  }
  // sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept
    requires(ExactSize)
  {
    return exact_size_hint_(iter_);
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  template <::sus::mem::Move Iter>
  static BoxedIterator with(Iter&& iter) noexcept
    requires(
        ::sus::ptr::SameOrSubclassOf<Iter*, IteratorBase<Iter, Item>*> &&
        !::sus::mem::relocate_by_memcpy<Iter>)
  {
    // IteratorBase also checks this. It's needed for correctness of the move
    // onto the heap.
    static_assert(std::is_final_v<Iter>);

    usize (*exact_size_hint)(const void*) = nullptr;
    if constexpr (ExactSize) {
      exact_size_hint = [](const void* boxed_iter) {
        return static_cast<const Iter*>(boxed_iter)->exact_size_hint();
      };
    }

    BoxedIterator (*clone)(const void*) = nullptr;
    if constexpr (Clone) {
      clone = [](const void* boxed_iter) {
        return BoxedIterator::with(
            ::sus::clone(*static_cast<const Iter*>(boxed_iter)));
      };
    }

    if constexpr (DoubleEnded) {
      return BoxedIterator(
          new Iter(::sus::move(iter)),  // Move it to the heap.
          [](void* boxed_iter) { delete static_cast<Iter*>(boxed_iter); },
          clone,
          [](void* boxed_iter) {
            return static_cast<Iter*>(boxed_iter)->next();
          },
          [](void* boxed_iter) {
            return static_cast<Iter*>(boxed_iter)->next_back();
          },
          [](const void* boxed_iter) {
            return static_cast<const Iter*>(boxed_iter)->size_hint();
          },
          exact_size_hint);
    } else {
      return BoxedIterator(
          new Iter(::sus::move(iter)),  // Move it to the heap.
          [](void* boxed_iter) { delete static_cast<Iter*>(boxed_iter); },
          clone,
          [](void* boxed_iter) {
            return static_cast<Iter*>(boxed_iter)->next();
          },
          [](const void* boxed_iter) {
            return static_cast<const Iter*>(boxed_iter)->size_hint();
          },
          exact_size_hint);
    }
  }

  // DoubleEnded constructor.
  BoxedIterator(void* iter, void (*destroy)(void* boxed_iter),
                BoxedIterator (*clone)(const void* boxed_iter),
                Option<Item> (*next)(void* boxed_iter),
                Option<Item> (*next_back)(void* boxed_iter),
                SizeHint (*size_hint)(const void* boxed_iter),
                usize (*exact_size_hint)(const void* boxed_iter))
      : iter_(iter),
        destroy_(destroy),
        clone_(clone),
        next_(next),
        next_back_(next_back),
        size_hint_(size_hint),
        exact_size_hint_(exact_size_hint) {}
  // Not-DoubleEnded constructor.
  BoxedIterator(void* iter, void (*destroy)(void* boxed_iter),
                BoxedIterator (*clone)(const void* boxed_iter),
                Option<Item> (*next)(void* boxed_iter),
                SizeHint (*size_hint)(const void* boxed_iter),
                usize (*exact_size_hint)(const void* boxed_iter))
      : iter_(iter),
        destroy_(destroy),
        clone_(clone),
        next_(next),
        next_back_(nullptr),
        size_hint_(size_hint),
        exact_size_hint_(exact_size_hint) {}

  void* iter_;
  void (*destroy_)(void* boxed_iter);
  BoxedIterator (*clone_)(const void* boxed_iter);
  Option<Item> (*next_)(void* boxed_iter);
  // TODO: We could remove this field with a nested struct + template
  // specialization when DoubleEnded is false.
  Option<Item> (*next_back_)(void* boxed_iter);
  SizeHint (*size_hint_)(const void* boxed_iter);
  usize (*exact_size_hint_)(const void* boxed_iter);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(iter_),
                                  decltype(destroy_), decltype(clone_),
                                  decltype(next_), decltype(next_back_),
                                  decltype(size_hint_),
                                  decltype(exact_size_hint_));
};

}  // namespace sus::iter
