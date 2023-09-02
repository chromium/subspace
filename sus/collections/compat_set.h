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

#include <set>

#include "sus/iter/compat_ranges.h"
#include "sus/iter/from_iterator.h"
#include "sus/iter/iterator.h"

template <class Key, class Compare, class Allocator>
struct sus::iter::FromIteratorImpl<std::set<Key, Compare, Allocator>> {
  static constexpr std::set<Key, Compare, Allocator> from_iter(
      ::sus::iter::IntoIterator<Key> auto&& into_iter) noexcept
    requires(sus::mem::IsMoveRef<decltype(into_iter)>)
  {
    auto&& iter = sus::move(into_iter).into_iter();
    auto s = std::set<Key, Compare, Allocator>();
    for (Key k : iter) s.insert(k);
    return s;
  }
};

template <class Key, class Compare, class Allocator>
struct sus::iter::FromIteratorImpl<std::multiset<Key, Compare, Allocator>> {
  static constexpr std::multiset<Key, Compare, Allocator> from_iter(
      ::sus::iter::IntoIterator<Key> auto&& into_iter) noexcept
    requires(sus::mem::IsMoveRef<decltype(into_iter)>)
  {
    auto&& iter = sus::move(into_iter).into_iter();
    auto s = std::multiset<Key, Compare, Allocator>();
    for (Key k : iter) s.insert(k);
    return s;
  }
};
