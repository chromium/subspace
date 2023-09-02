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

#include "sus/iter/from_iterator.h"
#include "sus/iter/iterator.h"

template <class T, class Collection>
struct sus::iter::FromIteratorImpl<std::queue<T, Collection>> {
  static constexpr std::queue<T, Collection> from_iter(
      ::sus::iter::IntoIterator<T> auto&& into_iter) noexcept
    requires(sus::mem::IsMoveRef<decltype(into_iter)>)
  {
    auto&& iter = sus::move(into_iter).into_iter();
    auto v = std::queue<T, Collection>();
    for (T&& t : iter) v.push(::sus::move(t));
    return v;
  }
};

template <class T, class Collection, class Compare>
struct sus::iter::FromIteratorImpl<
    std::priority_queue<T, Collection, Compare>> {
  static constexpr std::priority_queue<T, Collection, Compare> from_iter(
      ::sus::iter::IntoIterator<T> auto&& into_iter) noexcept
    requires(sus::mem::IsMoveRef<decltype(into_iter)>)
  {
    auto&& iter = sus::move(into_iter).into_iter();
    auto v = std::priority_queue<T, Collection, Compare>();
    for (T&& t : iter) v.push(::sus::move(t));
    return v;
  }
};
