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

#include <functional>  // TODO: Replace std::function with something.

#include "traits/iter/iterator_defn.h"

// Iterator provided functions are implemented in this file, so that they can be
// easily included by library users, but don't have to be included in every
// library header that returns an Iterator.

namespace sus::traits::iter {

template <class Item>
bool Iterator<Item>::all(std::function<bool(Item)> f) noexcept {
  while (item_.is_some()) {
    // Safety: `item_` was checked to hold Some already.
    Item i = item_.take().unwrap_unchecked(unsafe_fn);
    if (!f(static_cast<decltype(i)&&>(i))) return false;
    item_ = next();
  }
  return true;
}

template <class Item>
bool Iterator<Item>::any(std::function<bool(Item)> f) noexcept {
  while (item_.is_some()) {
    // Safety: `item_` was checked to hold Some already.
    Item i = item_.take().unwrap_unchecked(unsafe_fn);
    if (f(static_cast<decltype(i)&&>(i))) return true;
    item_ = next();
  }
  return false;
}

template <class Item>
size_t Iterator<Item>::count() noexcept {
  size_t c = 0;
  while (item_.is_some()) {
    c += 1;
    item_ = next();
  }
  return c;
}

}  // namespace sus::traits::iter
