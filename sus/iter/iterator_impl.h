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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <type_traits>

#include "sus/iter/try_from_iterator.h"

namespace sus::iter {

// Implementation of IteratorBase::try_collect has to live here to avoid
// cyclical includes, as the impl of try_from_iter() needs to see IteratorBase
// in order to make something that can be consumed by FromIterator.
template <class Iter, class Item>
template <class C>
  requires(::sus::ops::Try<Item> &&  //
           FromIterator<C, ::sus::ops::TryOutputType<Item>> &&
           // Void can not be collected from.
           !std::is_void_v<::sus::ops::TryOutputType<Item>>)
constexpr auto IteratorBase<Iter, Item>::try_collect() noexcept {
  return try_from_iter<C>(static_cast<Iter&&>(*this));
}

}  // namespace sus::iter
