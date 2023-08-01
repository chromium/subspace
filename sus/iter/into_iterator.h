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

#include "sus/iter/iterator_concept.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"

namespace sus::iter {

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
concept IntoIterator = requires(std::remove_cvref_t<T> t) {
  { ::sus::move(t).into_iter() } -> Iterator<Item>;
};

/// Conversion into an `Iterator` over any type of values.
///
/// Like `IntoIterator` but is only satisfied for an iterator over any type of
/// item, without needing to know the `Item` apriori.
///
/// This is useful to work around the limits of type deduction in templates,
/// along with `IntoIteratorOutputType` to extract the `Item` being iterated
/// over in the resulting iterator.
template <class T>
concept IntoIteratorAny = requires(std::remove_cvref_t<T> t) {
  {
    ::sus::move(t).into_iter()
  } -> Iterator<
      typename std::decay_t<decltype(::sus::move(t).into_iter())>::Item>;
};

/// Returns the type of iterator that will be produced from `T` where `T`
/// satisifies `IntoIteratorAny<T>`.
///
/// The returned type is decayed to a value, such that if into_iter() returns a
/// reference to itself, this resolves to the iterator type without the
/// reference.
template <class T>
  requires(IntoIteratorAny<T>)
using IntoIteratorOutputType = std::decay_t<
    decltype(std::declval<std::remove_cvref_t<T>&&>().into_iter())>;

}  // namespace sus::iter
