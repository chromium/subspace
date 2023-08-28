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

#include <deque>

#include "sus/iter/from_iterator.h"
#include "sus/iter/iterator.h"

template <class T, class Allocator>
struct sus::iter::FromIteratorImpl<std::deque<T, Allocator>> {
  static constexpr std::deque<T, Allocator> from_iter(
      ::sus::iter::IntoIterator<T> auto&& into_iter) noexcept {
    auto&& iter = sus::move(into_iter).into_iter();
    auto [lower, upper] = iter.size_hint();
    auto v = std::deque<T, Allocator>();
    for (T&& t : iter) v.push_back(::sus::move(t));
    return v;
  }
};
