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

#include "subspace/iter/iterator_concept.h"
#include "subspace/mem/forward.h"

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
concept IntoIterator = requires(T&& t) {
  { ::sus::forward<T>(t).into_iter() } -> Iterator<Item>;
};

}  // namespace sus::iter
