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

#include "sus/iter/iterator_concept.h"
#include "sus/iter/iterator_defn.h"
#include "sus/macros/lifetimebound.h"
#include "sus/macros/nonnull.h"
#include "sus/mem/addressof.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator that holds a reference to another iterator and proxies calls
/// through to it. Used to create multiple iterators that share underlying
/// state. The ByRef class must outlive the iterator it refers to.
///
/// This type is returned from `Iterator::by_ref()`.
template <class RefIterator>
class [[nodiscard]] [[sus_trivial_abi]] ByRef final
    : public IteratorBase<ByRef<RefIterator>, typename RefIterator::Item> {
 public:
  using Item = RefIterator::Item;

  /// sus::iter::Iterator trait.
  Option<Item> next() noexcept { return next_iter_->next(); }

  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return next_iter_->size_hint();
  }

  /// sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(::sus::iter::DoubleEndedIterator<RefIterator, Item>)
  {
    return next_iter_->next_back();
  }

  /// sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept
    requires(::sus::iter::ExactSizeIterator<RefIterator, Item>)
  {
    return next_iter_->exact_size_hint();
  }

  ~ByRef() noexcept {
    // TODO: Drop a refcount on the thing `next_iter_` is iterating on.
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static ByRef with(RefIterator& next_iter sus_lifetimebound) noexcept {
    return ByRef(next_iter);
  }

  ByRef(RefIterator& next_iter sus_lifetimebound) noexcept
      : next_iter_(::sus::mem::addressof(next_iter)) {
    // TODO: Add a refcount on the thing `next_iter_` is iterating on.
  }

  RefIterator* sus_nonnull_var next_iter_;

  // The RefIterator pointer is trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(next_iter_));
};

}  // namespace sus::iter
