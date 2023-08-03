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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/fn/fn_box_defn.h"
#include "sus/iter/iterator_defn.h"
#include "sus/iter/sized_iterator.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator that returns the inner iterator's values until it sees `None`,
/// and then only returns `None`.
///
/// This type is returned from `Iterator::fuse()`.
///
/// # Implementation Note
/// This class uses the actual inner iterator type instead of a SizedIterator,
/// which is unusual. This leads to more template instantiations, as every
/// iterator type `I` that is used with Fuse produces a `Fuse<I>` instantiation.
/// The reason for this choice is that Fuse is meant to be a no-op, other than
/// to stop returning anything past `None`, and thus we want to avoid any extra
/// codegen/overhead involved in making a SizedIterator for it.
template <class InnerIter>
class [[nodiscard]] Fuse final
    : public IteratorBase<Fuse<InnerIter>, typename InnerIter::Item> {
 public:
  using Item = InnerIter::Item;

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> o;
    if (iter_.is_some()) {
      o = iter_.as_value_mut().next();
      if (o.is_none()) iter_ = Option<InnerIter>();
    }
    return o;
    // TODO: Is this as efficient? Does it get NRVO?
    // return iter_.as_mut().map_or_else(  //
    //     [] { return Option<Item>(); },
    //     [this](InnerIter& it) {
    //       Option<Item> o = it.next();
    //       if (o.is_none()) iter_ = Option<InnerIter>();
    //       return o;
    //     });
  }
  /// sus::iter::Iterator trait.
  SizeHint size_hint() const noexcept {
    if (iter_.is_some())
      return iter_.as_value().size_hint();
    else
      return SizeHint(0u, Option<usize>::with(0u));
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerIter, Item>)
  {
    Option<Item> o;
    if (iter_.is_some()) {
      o = iter_.as_value_mut().next_back();
      if (o.is_none()) iter_ = Option<InnerIter>();
    }
    return o;
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static Fuse with(InnerIter&& iter) noexcept {
    return Fuse(::sus::move(iter));
  }

  Fuse(InnerIter&& iter) noexcept
      : iter_(::sus::Option<InnerIter>::with(::sus::move(iter))) {}

  ::sus::Option<InnerIter> iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(iter_));
};

}  // namespace sus::iter
