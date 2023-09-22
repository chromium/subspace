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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <concepts>
#include <type_traits>

#include "sus/lib/__private/forward_decl.h"
#include "sus/ptr/subclass.h"

namespace sus::iter {

/// A concept for all implementations of iterators.
/// See [`IteratorBase`]($sus::iter::IteratorBase) for the methods on an
/// `Iterator`.
///
/// Types that satisfy this concept can be used in for loops and provide
/// all the methods of an iterator type, which are found in
/// [`IteratorBase`]($sus::iter::IteratorBase).
///
/// Any `Iterator`'s full definition includes a number of other methods as well,
/// built on top of next, and so you get them for free.
///
/// Iterators are also composable, and it's possible to chain them together to
/// do more complex forms of processing.
///
/// # Required methods.
///
/// An iterator has two required methods.
///
/// * `auto next() -> `[`Option`]($sus::option::Option)`<Item>`
/// 
///   Returns an [`Option`]($sus::option::Option)
///   containing the next `Item` as long as there are elements,
///   and once they've all been exhausted, will return `None` to indicate that
///   iteration is finished. Individual iterators may choose to resume
///   iteration, and so calling next again may or may not eventually start
///   returning an `Item` again at some point.
///
/// * `auto size_hint() const -> `[`SizeHint`]($sus::iter::SizeHint)
/// 
///   Returns a [`SizeHint`]($sus::iter::SizeHint) containing a lower bound
///   and optional upper bound on the number of elements left to be yielded by
///   the iterator. An upper bound of `None` indicates either an unknown upper
///   bound or a bound that is larger than `usize`. Returning `lower = 0` and
///   `upper = None` is correct for any iterator, but providing a more accurate
///   bound can benefit performance optiomizations. Returning an incorrect bound
///   is technically possible but is a violation of the `Iterator` protocol.
template <class T, class Item>
concept Iterator =
    requires(std::remove_cvref_t<T>& t, const std::remove_cvref_t<T>& c) {
      // Has a T::Item typename.
      requires(!std::is_void_v<typename std::remove_cvref_t<T>::Item>);
      // The T::Item matches the concept input.
      requires(std::same_as<typename std::remove_cvref_t<T>::Item, Item>);
      // Subclasses from IteratorBase<T, T::Item>.
      requires ::sus::ptr::SameOrSubclassOf<
          std::remove_cvref_t<T>*, IteratorBase<std::remove_cvref_t<T>, Item>*>;
      // Required methods.
      { t.next() } noexcept -> std::same_as<::sus::option::Option<Item>>;
      { c.size_hint() } noexcept -> std::same_as<SizeHint>;
    };

/// A concept for testing if a type `T` is an [`Iterator`]($sus::iter::Iterator)
/// without testing its `Item` type.
template <class T>
concept IteratorAny = requires {
  typename std::remove_cvref_t<T>::Item;
  requires Iterator<T, typename std::remove_cvref_t<T>::Item>;
};

/// An [`Iterator`]($sus::iter::Iterator) able to yield elements from both ends.
///
/// Something that implements `DoubleEndedIterator` has one extra capability
/// over something that implements `Iterator`: the ability to also take Items
/// from the back, as well as the front.
///
/// It is important to note that both back and forth work on the same range, and
/// do not cross: iteration is over when they meet in the middle.
///
/// In a similar fashion to the [`Iterator`]($sus::iter::Iterator) protocol,
/// once a
/// `DoubleEndedIterator` returns `None` from a `next_back()`, calling it again
/// may or may not ever return `Some` again. `next()` and `next_back()` are
/// interchangeable for this purpose.
///
/// # Required methods.
///
/// A `DoubleEndedIterator` has one required methods, in addition to those
/// required by [`Iterator`]($sus::iter::Iterator).
///
/// * `auto next_back() -> Option<Item>` returns an `Option` containing the next
/// `Item` from the back of the iterator as long as there are elements,
/// and once they've all been exhausted, will return `None` to indicate that
/// iteration is finished.
template <class T, class Item = typename std::remove_cvref_t<T>::Item>
concept DoubleEndedIterator =
    Iterator<T, Item> && requires(std::remove_cvref_t<T>& t) {
      { t.next_back() } noexcept -> std::same_as<::sus::option::Option<Item>>;
    };

/// An [`Iterator`]($sus::iter::Iterator) that knows its exact length.
///
/// Many [`Iterator`]($sus::iter::Iterator)s don’t know how many times they will
/// iterate, but some do. If
/// an iterator knows how many times it can iterate, providing access to that
/// information can be useful. For example, if you want to iterate backwards, a
/// good start is to know where the end is.
///
/// # Required methods.
///
/// An `ExactSizeIterator` has one required methods, in addition to those
/// required by [`Iterator`]($sus::iter::Iterator).
///
/// * `auto exact_size_hint() -> usize` returns the exact size of the iterator.
/// The implementation of `Iterator::size_hint()` must also return the exact
/// size of the iterator (usually by calling `exact_size_hint()`).
///
/// TODO: Rename `exact_size_hint` to `len`?
template <class T, class Item = typename std::remove_cvref_t<T>::Item>
concept ExactSizeIterator =
    Iterator<T, Item> && requires(const std::remove_cvref_t<T>& t) {
      { t.exact_size_hint() } noexcept -> std::same_as<::sus::num::usize>;
    };

namespace __private {
struct TrustedLenMarker {};
}  // namespace __private

/// An iterator that reports an accurate length.
///
/// The iterator reports a size hint where it is either exact (lower bound is
/// equal to upper bound), or the upper bound is None. The upper bound must
/// only be None if the actual iterator length is larger than
/// [`usize::MAX`]($sus::num::usize::MAX). In that case, the lower bound must be
/// [`usize::MAX`]($sus::num::usize::MAX), resulting in an
/// `Iterator::size_hint()` of `(usize::MAX, None)`.
///
/// The iterator must produce exactly the number of elements it reported.
///
/// # Implementing TrustedLen
/// To opt into implementing `TrustedLen` a `trusted_len() const` method should
/// return the `TrustedLenMarker` type.
///
/// # When shouldn't an adapter be TrustedLen?
/// If an adapter makes an iterator shorter by a given amount, then it's usually
/// incorrect for that adapter to implement `TrustedLen`. The inner iterator
/// might return more than [`usize::MAX`]($sus::num::usize::MAX) items, but
/// there's no way to know what `k` elements less than that will be, since the
/// `size_hint` from the inner iterator has already saturated and lost that
/// information.
///
/// This is why [`Skip<I>`]($sus::iter::Skip) isn't `TrustedLen`, even
/// when `I` implements `TrustedLen`.
///
/// # Safety
/// This trait must only be implemented when the contract is upheld. Consumers
/// of this trait must inspect `Iterator::size_hint()`'s upper bound.
// TODO: Satisfy this concept for our iterator adaptors if the underlying
// iterator is trusted.
template <class T>
concept TrustedLen = requires(const std::remove_cvref_t<T>& t) {
  { t.trusted_len() } -> std::same_as<__private::TrustedLenMarker>;
};

}  // namespace sus::iter
