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

#include <concepts>
#include <type_traits>

#include "sus/ptr/subclass.h"
#include "sus/lib/__private/forward_decl.h"

namespace sus::iter {

/// A concept for all implementations of iterators.
///
/// Types that satisfy this concept can be used in for loops and provide
/// all the methods of an iterator type, which are found in
/// `sus::iter::IteratorBase`.
///
/// Any Iterator's full definition includes a number of other methods as well,
/// built on top of next, and so you get them for free.
///
/// Iterators are also composable, and it's possible to chain them together to
/// do more complex forms of processing.
///
/// # Required methods.
///
/// An iterator has two required methods.
///
/// `next()`, which when called, returns an `Option<Item>`. Calling next will
/// return an `Option` containing the next `Item` as long as there are elements,
/// and once they've all been exhausted, will return `None` to indicate that
/// iteration is finished. Individual iterators may choose to resume iteration,
/// and so calling next again may or may not eventually start returning an
/// `Item` again at some point.
///
/// `size_hint()` that returns a `sus::iter::SizeHint` containing a lower bound
/// and optional upper bound on the number of elements left to be yielded by the
/// iterator. An upper bound of `None` indicates either an unknown upper bound
/// or a bound that is larger than `usize`. Rerturning `lower = 0` and `upper =
/// None` is correct for any iterator, but providing a more accurate bound can
/// benefit performance optiomizations. Returning an incorrect bound is
/// technically possible but is a violation of the Iterator protocol.
template <class T, class Item>
concept Iterator = requires(T& t, const T& c) {
  // Has a T::Item typename.
  requires(!std::is_void_v<typename std::decay_t<T>::Item>);
  // The T::Item matches the concept input.
  requires(std::same_as<typename std::decay_t<T>::Item, Item>);
  // Subclasses from IteratorBase<T, T::Item>.
  requires ::sus::ptr::SameOrSubclassOf<
      std::decay_t<T>*, IteratorBase<std::decay_t<T>, Item>*>;
  // Required methods.
  { t.next() } noexcept -> std::same_as<::sus::option::Option<Item>>;
  { c.size_hint() } noexcept -> std::same_as<::sus::iter::SizeHint>;
};

/// An `Iterator` able to yield elements from both ends.
///
/// Something that implements `DoubleEndedIterator` has one extra capability
/// over something that implements `Iterator`: the ability to also take Items
/// from the back, as well as the front.
///
/// It is important to note that both back and forth work on the same range, and
/// do not cross: iteration is over when they meet in the middle.
///
/// In a similar fashion to the `Iterator` protocol, once a
/// `DoubleEndedIterator` returns `None` from a `next_back()`, calling it again
/// may or may not ever return `Some` again. `next()` and `next_back()` are
/// interchangeable for this purpose.
template <class T, class Item>
concept DoubleEndedIterator = Iterator<T, Item> && requires(T& t) {
  { t.next_back() } noexcept -> std::same_as<::sus::option::Option<Item>>;
};

/// An iterator that knows its exact length.
///
/// Many Iterators don’t know how many times they will iterate, but some do. If
/// an iterator knows how many times it can iterate, providing access to that
/// information can be useful. For example, if you want to iterate backwards, a
/// good start is to know where the end is.
///
/// When implementing an `ExactSizeIterator`, you must also implement
/// `Iterator`.
///
/// Then, to implement `ExactSizeIterator`, provide the `exact_size_hint()`
/// method that returns the exact size of the iterator. The implementation of
/// `Iterator::size_hint()` must also return the exact size of the iterator
/// (usually by calling `exact_size_hint()`).
//
// TODO: Rename exact_size_hint() to len()?
template <class T, class Item>
concept ExactSizeIterator = Iterator<T, Item> && requires(const T& t) {
  { t.exact_size_hint() } noexcept -> std::same_as<::sus::num::usize>;
};

}  // namespace sus::iter
