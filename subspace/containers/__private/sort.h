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

#include "subspace/fn/fn_concepts.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/mem/swap.h"
#include "subspace/num/integer_concepts.h"
#include "subspace/tuple/tuple.h"

namespace sus::containers {
template <class T>
class SliceMut;
}  // namespace sus::containers

namespace sus::containers::__private {

// Helper function for indexing our vector by the smallest possible type, to
// reduce allocation.
template <class U, class Key, class T, int&...,
          class KeyFn = ::sus::fn::FnMut<Key(const T&)>>
void sort_slice_by_cached_key(KeyFn& f,
                              ::sus::containers::SliceMut<T>& slice) noexcept {
  auto indices =
      slice.iter()
          .map(f)
          .enumerate()
          .map([](::sus::Tuple<usize, Key>&& t) {
            auto&& [i, k] = ::sus::move(t);
            return ::sus::Tuple<Key, U>::with(::sus::forward<Key>(k), i);
          })
          .collect_vec();
  // The elements of `indices` are unique, as they are indexed, so any sort
  // will be stable with respect to the original slice. We use `sort_unstable`
  // here because it requires less memory allocation.
  indices.sort_unstable();
  const usize length = slice.len();
  for (usize i; i < length; i += 1u) {
    auto index = indices[i].at<1>();
    while (index < i) {
      index = indices[index].at<1>();
    }
    indices[i].at_mut<1>() = index;
    slice.swap_unchecked(::sus::marker::unsafe_fn, i, index);
  }
};

}  // namespace sus::containers::__private
