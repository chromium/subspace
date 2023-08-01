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

#include "sus/iter/__private/into_iterator_archetype.h"
#include "sus/mem/move.h"

namespace sus::iter {

/// Extend a collection with the contents of an iterator.
///
/// Iterators produce a series of values, and collections can also be thought of
/// as a series of values. The `Extend` concept bridges this gap, allowing you to
/// extend a collection by including the contents of that iterator. When
/// extending a collection with an already existing key, that entry is updated
/// or, in the case of collections that permit multiple entries with equal keys,
/// that entry is inserted.
template <class Collection, class ItemType>
concept Extend =
    requires(Collection c, __private::IntoIteratorArchetype<ItemType>&& iter) {
      { c.extend(::sus::move(iter)) } -> std::same_as<void>;
    };

}  // namespace sus::iter
