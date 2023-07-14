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

#include "subspace/iter/__private/iterator_archetype.h"
#include "subspace/iter/iterator_concept.h"
#include "subspace/mem/move.h"

namespace sus::iter {

/// Trait to represent types that can be created by adding elements of an
/// iterator.
///
/// Types which implement this trait can be generated by using the sum() method
/// on an iterator. Like with `FromIterator`, the `from_sum()` method should
/// rarely be called directly.
///
/// When given an empty iterator, the result of `from_sum()` should be the
/// "zero" value of the type.
template <class T, class Item = T>
concept Sum = requires(__private::IteratorArchetype<Item>&& iter) {
  // Use cloned() or copied() to convert an Iterator of references to values.
  requires(!std::is_reference_v<T>);
  { T::from_sum(::sus::move(iter)) } -> std::same_as<T>;
};

}  // namespace sus::iter
