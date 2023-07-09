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

#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator with a `peek()` that returns an optional reference to the next
/// element.
///
/// This type is returned from `Iterator::peekable()`.
template <class InnerSizedIter>
class [[nodiscard]] Peekable final
    : public IteratorBase<Peekable<InnerSizedIter>,
                          typename InnerSizedIter::Item> {
 public:
  using Item = InnerSizedIter::Item;

  Peekable clone() const noexcept
    requires(InnerSizedIter::Clone &&  //
             ::sus::mem::CloneOrRef<Item>)
  {
    return Peekable(::sus::clone(peeked_), ::sus::clone(next_iter_));
  }

  /// Returns a const reference to the `next()` value without advancing the
  /// iterator.
  ///
  /// Like `next()`, if there is a value, it is wrapped in a `Some(T)`.
  /// But if the iteration is over, `None` is returned.
  Option<const std::remove_reference_t<Item>&> peek() noexcept {
    return peeked_.get_or_insert_with([this] { return next_iter_.next(); })
        .as_ref();
  }

  /// Returns a mutable reference to the `next()` value without advancing the
  /// iterator.
  ///
  /// Like `next()`, if there is a value, it is wrapped in a `Some(T)`.
  /// But if the iteration is over, `None` is returned.
  Option<Item&> peek_mut() noexcept {
    return peeked_.get_or_insert_with([this] { return next_iter_.next(); })
        .as_mut();
  }

  /// Consume and return the next value of this iterator if a condition is true.
  ///
  /// If `func` returns `true` for the next value of this iterator, consume and
  /// return it. Otherwise, return `None`.
  Option<Item> next_if(
      ::sus::fn::FnOnceRef<bool(const std::remove_reference_t<Item>&)>
          pred) noexcept {
    Option<Item> o = next();
    if (o.is_some() && ::sus::move(pred)(o.as_value())) {
      return o;
    } else {
      // Since we called `next()`, we consumed `peeked_`, so we can insert
      // without clobbering anything.
      peeked_.insert(o.take());
      return o;  // The above makes `o` into None.
    }
  }

  /// Consume and return the next item if it is equal to `expected`.
  Option<Item> next_if_eq(
      const std::remove_reference_t<Item>& expected) noexcept
    requires(::sus::ops::Eq<Item>)
  {
    return next_if([&](const auto& i) { return i == expected; });
  }

  /// sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    return peeked_.take().unwrap_or_else(
        [this]() { return next_iter_.next(); });
  }
  /// sus::iter::Iterator trait.
  SizeHint size_hint() const noexcept {
    usize peek_len;
    if (peeked_.is_some()) {
      if (peeked_.as_value().is_some()) {
        // There's a peeked value waiting.
        peek_len = 1u;
      } else {
        // peek() found the iterator to be empty.
        return SizeHint(0u, sus::some(0u));
      }
    }

    auto [lo, hi] = next_iter_.size_hint();
    return SizeHint(
        lo.saturating_add(peek_len),  //
        hi.and_then([&](usize i) { return i.checked_add(peek_len); }));
  }

  /// sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(InnerSizedIter::DoubleEnded)
  {
    if (peeked_.is_some()) {
      if (peeked_.as_value().is_some()) {
        return next_iter_.next_back().or_else([this] {
          // This leaves peeked_ with an empty Option inside it, indicating the
          // iterator is empty.
          return peeked_.as_value_mut().take();
        });
      } else {
        // Already saw empty.
        return Option<Item>();
      }
    } else {
      return next_iter_.next_back();
    }
  }

  /// sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept
    requires(InnerSizedIter::ExactSize)
  {
    if (peeked_.is_some()) {
      if (peeked_.as_value().is_some()) {
        // Won't panic for a well behaved ExactSizeIterator. It's length could
        // not exceed usize::MAX and peeked_ having a value means it is already
        // less than max length.
        return 1u + next_iter_.exact_size_hint();
      } else {
        // peek() found the iterator to be empty.
        return 0u;
      }
    }
    return next_iter_.exact_size_hint();
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static Peekable with(InnerSizedIter&& next_iter) noexcept {
    return Peekable(::sus::move(next_iter));
  }

  Peekable(InnerSizedIter&& next_iter) : next_iter_(::sus::move(next_iter)) {}
  Peekable(::sus::Option<::sus::Option<Item>>&& peeked,
           InnerSizedIter&& next_iter)
      : peeked_(::sus::move(peeked)), next_iter_(::sus::move(next_iter)) {}

  ::sus::Option<::sus::Option<Item>> peeked_;
  InnerSizedIter next_iter_;

  // The InnerSizedIter is trivially relocatable, but the Item may not be.
  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(peeked_),
                                           decltype(next_iter_));
};

}  // namespace sus::iter
