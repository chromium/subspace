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

#pragma once

#include "subspace/fn/fn_concepts.h"
#include "subspace/iter/iterator_concept.h"

namespace sus::iter::__private {

/// Compares two iterators element-wise using the given function.
///
/// Isolates the logic shared by [`cmp_by`](sus::iter::IteratorBase::cmp_by),
/// [`partial_cmp_by`](sus::iter::IteratorBase::partial_cmp_by), and
/// [`eq_by`](sus::iter::IteratorBase::eq_by).
template <class Ordering, class Item>
inline Ordering iter_compare(
    ::sus::iter::Iterator<Item> auto&& a, ::sus::iter::Iterator<Item> auto&& b,
    ::sus::fn::FnMut<Ordering(const std::remove_reference_t<Item>&,
                              const std::remove_reference_t<Item>&)> auto&& f) {
  while (true) {
    ::sus::Option<Item> item_a = a.next();
    ::sus::Option<Item> item_b = b.next();
    if (item_a.is_none() && item_b.is_none()) {
      return Ordering::equivalent;
    } else if (item_a.is_none()) {
      return Ordering::less;
    } else if (item_b.is_none()) {
      return Ordering::greater;
    } else {
      Ordering ord = f(item_a.as_value(), item_b.as_value());
      if (ord < 0)
        return Ordering::less;
      else if (ord > 0)
        return Ordering::greater;
      // Otherwise, try the next pair of elements.
    }
  }
}

}  // namespace sus::iter::__private
