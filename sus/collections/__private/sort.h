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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/fn/fn_concepts.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/swap.h"
#include "sus/num/integer_concepts.h"
#include "sus/tuple/tuple.h"
#include "sus/lib/__private/forward_decl.h"

namespace sus::collections::__private {

// Helper function for indexing our vector by the smallest possible type, to
// reduce allocation.
template <class U, class Key, class T, ::sus::fn::FnMut<Key(const T&)> KeyFn>
void sort_slice_by_cached_key(const ::sus::collections::SliceMut<T>& slice,
                              KeyFn& f) noexcept {
  auto indices =
      slice.iter()
          .map(f)
          .enumerate()
          .map([](::sus::Tuple<usize, Key>&& t) {
            auto&& [i, k] = ::sus::move(t);
            return ::sus::Tuple<Key, U>(::sus::forward<Key>(k), i);
          })
          .collect_vec();
  // The elements of `indices` are unique, as they are indexed, so any sort
  // will be stable with respect to the original slice. We use `sort_unstable`
  // here because it requires less memory allocation.
  indices.sort_unstable();
  const usize length = slice.len();
  for (usize i; i < length; i += 1u) {
    auto index = indices[i].template at<1>();
    while (index < i) {
      index = indices[index].template at<1>();
    }
    indices[i].template at_mut<1>() = index;
    slice.swap_unchecked(::sus::marker::unsafe_fn, i, index);
  }
};

}  // namespace sus::collections::__private
