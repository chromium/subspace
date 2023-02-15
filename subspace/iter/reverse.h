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

#include "subspace/fn/fn_defn.h"
#include "subspace/iter/iterator_concept.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator that iterates over another iterator but in reverse.
///
/// The iterator wrapped by Reverse must be a DoubleEndedIterator.
template <class ItemT, class InnerIter>
class Reverse final : public IteratorImpl<Reverse<ItemT, InnerIter>, ItemT> {
  using InnerSizedIter = SizedIterator<ItemT, ::sus::mem::size_of<InnerIter>(),
                                       alignof(InnerIter)>;

 public:
  using Item = ItemT;

  static Reverse with(InnerSizedIter&& next_iter) noexcept
    requires(sus::iter::DoubleEndedIterator<InnerIter, Item>)
  {
    return Reverse(::sus::move(next_iter));
  }

  Option<Item> next() noexcept final {
    // This class must know the full type of the inner iterator in order to
    // access its next_back() method.
    //
    // TODO: We could consider storing an object pointer and method pointer
    // instead to erase the type, but needs to be a properly casted pointer
    // into the SizedIterator's storage to deal with the case where the Iterator
    // implementation has another virtual base class which comes before Iterator
    // and thus casting moves the pointer.
    //
    // TODO: If SizedIterator stored the method pointer for next_back and a
    // `static constexpr` tag to say if `next_back()` is callable, then all
    // composible iterators could preserve DoubleEndedIterator the same way
    // Reverse does.
    return static_cast<InnerIter&>(next_iter_.iterator_mut()).next_back();
  }

  // Reverse is reversible, and implements DoubleEndedIterator.
  Option<Item> next_back() noexcept {
    return static_cast<InnerIter&>(next_iter_.iterator_mut()).next();
  }

 private:
  Reverse(InnerSizedIter&& next_iter) : next_iter_(::sus::move(next_iter)) {}

  InnerSizedIter next_iter_;

  // The InnerSizedIter is trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(next_iter_));
};

}  // namespace sus::iter
