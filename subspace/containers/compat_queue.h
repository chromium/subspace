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

#include <queue>

#include "subspace/iter/from_iterator.h"
#include "subspace/iter/iterator.h"

template <class T, class Container>
struct sus::iter::FromIteratorImpl<std::queue<T, Container>> {
  static constexpr std::queue<T, Container> from_iter(
      ::sus::iter::IntoIterator<T> auto&& into_iter) noexcept {
    auto&& iter = sus::move(into_iter).into_iter();
    auto v = std::queue<T, Container>();
    for (T t : iter) v.push(::sus::move(t));
    return v;
  }
};

template <class T, class Container, class Compare>
struct sus::iter::FromIteratorImpl<std::priority_queue<T, Container, Compare>> {
  static constexpr std::priority_queue<T, Container, Compare> from_iter(
      ::sus::iter::IntoIterator<T> auto&& into_iter) noexcept {
    auto&& iter = sus::move(into_iter).into_iter();
    auto v = std::priority_queue<T, Container, Compare>();
    for (T t : iter) v.push(::sus::move(t));
    return v;
  }
};
