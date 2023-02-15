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

#include <type_traits>

#include "subspace/convert/subclass.h"
#include "subspace/mem/forward.h"

namespace sus::iter {

template <class Iter, class Item>
class IteratorImpl;

/// A concept for all implementations of iterators.
///
/// Types that satisfy this concept can be used in for loops and provide
/// all the methods of an iterator type, which are found in
/// `sus::iter::IteratorImpl`.
template <class T, class Item>
concept Iterator = requires {
  // Has a T::Item typename.
  requires(!std::is_void_v<typename std::decay_t<T>::Item>);
  // The T::Item matches the concept input.
  requires(std::same_as<typename std::decay_t<T>::Item, Item>);
  // Subclasses from IteratorImpl<T, T::Item>.
  requires ::sus::convert::SameOrSubclassOf<
      std::decay_t<T>*, IteratorImpl<std::decay_t<T>, Item>*>;
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
