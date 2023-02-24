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

#include "subspace/convert/subclass.h"
#include "subspace/mem/forward.h"

namespace sus::num {
struct usize;
}

namespace sus::option {
template <class T>
class Option;
}

namespace sus::iter {

template <class Iter, class Item>
class IteratorImpl;

/// A concept for all implementations of iterators.
///
/// An iterator has one required method, `next()`, which when called, returns an
/// `Option<Item>`. Calling next will return an `Option` containing the next
/// `Item` as long as there are elements, and once they've all been exhausted,
/// will return `None` to indicate that iteration is finished. Individual
/// iterators may choose to resume iteration, and so calling next again may or
/// may not eventually start returning an `Item` again at some point.
///
/// Types that satisfy this concept can be used in for loops and provide
/// all the methods of an iterator type, which are found in
/// `sus::iter::IteratorImpl`.
///
/// Any Iterator's full definition includes a number of other methods as well,
/// built on top of next, and so you get them for free.
///
/// Iterators are also composable, and it's possible to chain them together to
/// do more complex forms of processing.
template <class T, class Item>
concept Iterator = requires(T& t) {
  // Has a T::Item typename.
  requires(!std::is_void_v<typename std::decay_t<T>::Item>);
  // The T::Item matches the concept input.
  requires(std::same_as<typename std::decay_t<T>::Item, Item>);
  // Subclasses from IteratorImpl<T, T::Item>.
  requires ::sus::convert::SameOrSubclassOf<
      std::decay_t<T>*, IteratorImpl<std::decay_t<T>, Item>*>;
  // Required methods.
  { t.next() } -> std::same_as<::sus::option::Option<Item>>;
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

template <class T, class Item>
concept ExactSizeIterator = Iterator<T, Item> && requires(const T& t) {
  { t.exact_size_hint() } noexcept -> std::same_as<::sus::num::usize>;
};

/// Conversion into an `Iterator`.
///
/// A more general trait than `Iterator` which will accept anything that can be
/// iterated, including an `Iterator` (since all `Iterator`s also satisfy
/// `IntoIterator`). This can be particularly useful when receiving an iterator
/// over a set of non-reference values, allowing the caller to pass a container
/// directly in place of an iterator.
///
/// Note that an `IntoIterator` type is not directly iterable in for loops, and
/// requires calling `into_iter()` on it to convert it into an `Iterator`
/// which is iterable in for loops.
template <class T, class Item>
concept IntoIterator = requires(T&& t) {
  { ::sus::forward<T>(t).into_iter() } -> Iterator<Item>;
};

}  // namespace sus::iter
