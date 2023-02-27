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

#include "subspace/iter/into_iterator.h"

namespace sus::option {
template <class T>
class Option;
}

namespace sus::iter {
template <class SubclassT, class ItemT>
class IteratorBase;
}  // namespace sus::iter

namespace sus::iter::__private {

template <class T>
struct IntoIteratorArchetype {
  template <class Item>
  struct Iter final : public IteratorBase<Iter<Item>, Item> {
    ::sus::option::Option<Item> next() noexcept {
      return ::sus::option::Option<Item>::none();
    }
  };
  Iter<T> into_iter() && { return Iter<T>(); }
  static_assert(::sus::iter::IntoIterator<Iter<T>, T>);
};

}  // namespace sus::iter::__private
