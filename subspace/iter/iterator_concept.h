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

#include "subspace/convert/subclass.h"

namespace sus::iter {

template <class Iter, class Item>
class IteratorImpl;

/// A concept for dealing with iterators.
///
/// Types that satisfy this concept can be used in for loops and provide
/// all the methods of an iterator type, which are found in
/// `sus::iter::IteratorImpl`.
template <class T, class Item>
concept Iterator = requires {
  // Has a T::Item typename.
  requires(!std::is_void_v<typename T::Item>);
  // The T::Item matches the concept input.
  requires(std::same_as<typename T::Item, Item>);
  // Subclasses from IteratorImpl<T, T::Item>.
  requires ::sus::convert::SameOrSubclassOf<T*, IteratorImpl<T, Item>*>;
};

}  // namespace sus::iter
