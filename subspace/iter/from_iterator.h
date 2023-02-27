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

#include <concepts>

#include "subspace/iter/__private/into_iterator_archetype.h"
#include "subspace/mem/move.h"

namespace sus::iter {

/// A concept that indicates ToType can be constructed from an Iterator, via
/// `ToType::from_iterator(Iterator<IterType>)`.
///
/// Any type that matches this concept can be constructed from
/// Iterator::collect().
template <class ToType, class ItemType>
concept FromIterator =
    requires(__private::IntoIteratorArchetype<ItemType>&& from) {
      { ToType::from_iter(::sus::move(from)) } -> std::same_as<ToType>;
    };

}  // namespace sus::iter
