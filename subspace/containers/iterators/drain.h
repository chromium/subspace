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

#include "subspace/containers/iterators/slice_iter.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/mem/move.h"
#include "subspace/mem/nonnull.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/range.h"
#include "subspace/ptr/copy.h"

namespace sus::containers {
template <class T>
class Vec;
}  // namespace sus::containers

namespace sus::containers {

/// A draining iterator for Vec<T>.
///
/// This struct is created by Vec::drain. See its documentation for more.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] Drain final
    : public ::sus::iter::IteratorBase<Drain<ItemT>, ItemT> {
 public:
  using Item = ItemT;

 public:
  Drain(Drain&&) = default;
  Drain& operator=(Drain&&) = default;

  ~Drain() noexcept {
    // The `iter_` is None if keep_rest() was run, in which cast the Vec is
    // already restored. Or if Drain was moved from, in which case it has
    // nothing to do.
    if (iter_.is_some()) {
      restore_vec(0u);
    }
  }

  /// Keep unyielded elements in the source `Vec`.
  void keep_rest() && noexcept {
    const usize unyielded_len = iter_->exact_size_hint();
    Item* const unyielded_ptr =
        iter_.take().unwrap().as_mut_slice().as_mut_ptr();

    const usize start = vec_.len();
    Item* const start_ptr = vec_.as_mut_ptr() + start;

    // Move back unyielded elements.
    if (unyielded_ptr != start_ptr) {
      Item* const src = unyielded_ptr;
      Item* const dst = start_ptr;

      if constexpr (::sus::mem::relocate_by_memcpy<Item>) {
        // Since the Drain'd elements have been moved, and they are trivially
        // relocatable, move+destroy is a no-op, so we can skip the destructors
        // here and just memmove `src` into them moved-from `dst` objects.
        if (unyielded_len > 0u) {
          ::sus::ptr::copy(::sus::marker::unsafe_fn, src, dst, unyielded_len);
        }
      } else {
        for (usize i; i < unyielded_len; i += 1u) {
          *(dst + i) = ::sus::move(*(src + i));
        }
      }
    }

    restore_vec(unyielded_len);
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    // Moves from each element as it is drained. The moved-from element will
    // be destroyed when Drain is destroyed.
    return iter_->next().map([](Item& i) { return sus::move(i); });
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    // Moves from each element as it is drained. The moved-from element will
    // be destroyed when Drain is destroyed.
    return iter_->next_back().map([](Item& i) { return sus::move(i); });
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    return iter_->size_hint();
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    return iter_->exact_size_hint();
  }

 private:
  // Constructed by Vec.
  friend class Vec<Item>;

  void restore_vec(usize kept) {
    const usize start = vec_.len() + kept;
    const usize tail = tail_start_;
    if (start != tail) {
      // Drain range was not empty.

      const usize drop_len = tail - start;
      Item* const src = vec_.as_mut_ptr() + tail;
      Item* const dst = vec_.as_mut_ptr() + start;

      if constexpr (::sus::mem::relocate_by_memcpy<Item>) {
        // Since the Drain'd elements have been moved, and they are trivially
        // relocatable, move+destroy is a no-op, so we can skip the destructors
        // here and just memmove `src` into them moved-from `dst` objects.
        if (tail_len_ > 0u) {
          ::sus::ptr::copy(::sus::marker::unsafe_fn, src, dst, tail_len_);
        }
      } else {
        usize i;
        for (; i < tail_len_; i += 1u) {
          *(dst + i) = ::sus::move(*(src + i));
        }
        for (; i < tail_len_ + drop_len; i += 1u) {
          (dst + i)->~Item();
        }
      }
    }
    vec_.set_len(::sus::marker::unsafe_fn, start + tail_len_);
    original_vec_.as_mut() = ::sus::move(vec_);
  }

  static constexpr auto with(Vec<Item>&& vec sus_lifetimebound,
                             ::sus::ops::Range<usize> range) noexcept {
    return Drain(::sus::move(vec), range);
  }

  constexpr Drain(Vec<Item>&& vec sus_lifetimebound,
                  ::sus::ops::Range<usize> range) noexcept
      : tail_start_(range.finish),
        tail_len_(vec.len() - range.finish),
        iter_(::sus::some(SliceIterMut<Item&>::with(
            vec.as_mut_ptr() + range.start, range.finish - range.start))),
        vec_(::sus::move(vec)),
        original_vec_(sus::mem::nonnull(vec)) {
    vec_.set_len(::sus::marker::unsafe_fn, range.start);
  }

  /// Index of tail to preserve.
  usize tail_start_;
  /// Length of tail.
  usize tail_len_;
  /// Current remaining range to remove.
  Option<SliceIterMut<Item&>> iter_;
  /// The elements from the original_vec_, held locally for safe keeping so that
  /// mutation of the original Vec during drain will be flagged as
  /// use-after-move.
  Vec<Item> vec_;
  /// The original moved-from Vec which is restored when the iterator is
  /// destroyed.
  sus::mem::NonNull<Vec<Item>> original_vec_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(tail_start_), decltype(tail_len_),
                                  decltype(vec_), decltype(tail_start_),
                                  decltype(original_vec_));
};
}  // namespace sus::containers
