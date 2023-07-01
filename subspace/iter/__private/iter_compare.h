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
/// [`weak_cmp_by`](sus::iter::IteratorBase::weak_cmp_by).
template <class Ordering, class ItemA, class ItemB>
inline Ordering iter_compare(
    ::sus::iter::Iterator<ItemA> auto&& a,
    ::sus::iter::Iterator<ItemB> auto&& b,
    ::sus::fn::FnMut<Ordering(const std::remove_reference_t<ItemA>&,
                              const std::remove_reference_t<ItemB>&)> auto&&
        f) {
  Ordering value = Ordering::equivalent;
  while (true) {
    ::sus::Option<ItemA> item_a = a.next();
    ::sus::Option<ItemB> item_b = b.next();
    if (item_a.is_none() && item_b.is_none()) {
      return value;
    } else if (item_a.is_none()) {
      value = Ordering::less;
      return value;
    } else if (item_b.is_none()) {
      value = Ordering::greater;
      return value;
    } else {
      value = f(item_a.as_value(), item_b.as_value());
      if (!(value == 0)) return value;
      // Otherwise, try the next pair of elements.
    }
  }
}

/// Compares two iterators for equality element-wise using the given function.
template <class ItemA, class ItemB>
inline bool iter_compare_eq(
    ::sus::iter::Iterator<ItemA> auto&& a,
    ::sus::iter::Iterator<ItemB> auto&& b,
    ::sus::fn::FnMut<bool(const std::remove_reference_t<ItemA>&,
                          const std::remove_reference_t<ItemB>&)> auto&& f) {
  bool value = true;
  while (true) {
    ::sus::Option<ItemA> item_a = a.next();
    ::sus::Option<ItemB> item_b = b.next();
    if (item_a.is_none() && item_b.is_none()) {
      return value;
    } else if (item_a.is_none()) {
      value = false;
      return value;
    } else if (item_b.is_none()) {
      value = false;
      return value;
    } else {
      value = f(item_a.as_value(), item_b.as_value());
      if (!value) return value;
      // Otherwise, try the next pair of elements.
    }
  }
}
}  // namespace sus::iter::__private
