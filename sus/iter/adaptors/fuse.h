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
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator that returns the inner iterator's values until it sees `None`,
/// and then only returns `None`.
///
/// This type is returned from `Iterator::fuse()`.
template <class InnerIter>
class [[nodiscard]] Fuse final
    : public IteratorBase<Fuse<InnerIter>, typename InnerIter::Item> {
 public:
  using Item = InnerIter::Item;

  // Type is Move and (can be) Clone.
  Fuse(Fuse&&) = default;
  Fuse& operator=(Fuse&&) = default;

  // sus::mem::Clone trait.
  constexpr Fuse clone() const noexcept
    requires(::sus::mem::Clone<InnerIter>)
  {
    return Fuse(CLONE, ::sus::clone(iter_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
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
  constexpr SizeHint size_hint() const noexcept {
    if (iter_.is_some())
      return iter_.as_value().size_hint();
    else
      return SizeHint(0u, Option<usize>(0u));
  }

  /// sus::iter::ExactSizeIterator trait.
  constexpr usize exact_size_hint() const noexcept
    requires(ExactSizeIterator<InnerIter>)
  {
    if (iter_.is_some())
      return iter_.as_value().exact_size_hint();
    else
      return 0u;
  }

  /// sus::iter::TrustedLen trait.
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept
    requires(TrustedLen<InnerIter>)
  {
    return {};
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept
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

  // Regular ctor.
  explicit constexpr Fuse(InnerIter&& iter) noexcept
      : iter_(::sus::Option<InnerIter>(::sus::move(iter))) {}
  // Clone ctor.
  enum Clone { CLONE };
  explicit constexpr Fuse(Clone, ::sus::Option<InnerIter>&& iter) noexcept
      : iter_(::sus::move(iter)) {}

  ::sus::Option<InnerIter> iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(iter_));
};

}  // namespace sus::iter
