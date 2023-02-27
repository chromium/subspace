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
template <class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] Reverse final
    : public IteratorBase<Reverse<InnerSizedIter>,
                          typename InnerSizedIter::Item> {
  static_assert(InnerSizedIter::DoubleEnded);

 public:
  using Item = InnerSizedIter::Item;

  static Reverse with(InnerSizedIter&& next_iter) noexcept {
    return Reverse(::sus::move(next_iter));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return next_iter_.next_back(); }
  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept { return next_iter_.next(); }

 private:
  Reverse(InnerSizedIter&& next_iter) : next_iter_(::sus::move(next_iter)) {}

  InnerSizedIter next_iter_;

  // The InnerSizedIter is trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(next_iter_));
};

}  // namespace sus::iter
