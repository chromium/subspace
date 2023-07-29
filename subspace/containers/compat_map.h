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

#include <map>

#include "subspace/containers/compat_pair_concept.h"
#include "subspace/iter/from_iterator.h"
#include "subspace/iter/iterator.h"
#include "subspace/mem/forward.h"

template <class Key, class T, class Compare, class Allocator>
struct sus::iter::FromIteratorImpl<std::map<Key, T, Compare, Allocator>> {
 private:
  using Self = std::map<Key, T, Compare, Allocator>;

 public:
  template <
      class IntoIter,
      class Item = typename sus::iter::IntoIteratorOutputType<IntoIter>::Item>
    requires(sus::containers::compat::Pair<Item, Key, T>)
  static constexpr Self from_iter(IntoIter&& into_iter) noexcept {
    auto&& iter = sus::move(into_iter).into_iter();
    auto s = Self();
    for (auto&& [key, t] : iter)
      s.emplace(sus::forward<Key>(key), sus::forward<T>(t));
    return s;
  }
};
