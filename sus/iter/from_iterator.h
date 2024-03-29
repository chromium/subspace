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

#include "sus/iter/__private/into_iterator_archetype.h"
#include "sus/iter/into_iterator.h"
#include "sus/mem/move.h"

namespace sus::iter {

template <class ToType>
struct FromIteratorImpl;

/// A concept that indicates `ToType` can be constructed from an `Iterator`, via
/// `sus::iter::from_iter<ToType>(Iterator<IterType>)`.
///
/// Any type that matches this concept can be constructed from
/// `Iterator::collect()`.
///
/// The `from_iter()` is less often called, as the `collect()` method provides
/// the preferred way to construct from an iterator. But in generic template
/// code especially, the `from_iter()` can be more clear.
template <class ToType, class ItemType>
concept FromIterator = requires(
    __private::IntoIteratorArchetype<ItemType>&& from) {
  {
    FromIteratorImpl<std::remove_const_t<ToType>>::from_iter(::sus::move(from))
  } -> std::same_as<std::remove_const_t<ToType>>;
};

/// Constructs `ToType` from a type that can be turned into an `Iterator` over
/// elements of type `ItemType`.
///
/// This is the other end of
/// [`Iterator::collect()`](::sus::iter::IteratorBase::collect), and is
/// typically called through calling `collect()` on an iterator. However this
/// function can be preferrable for some readers, especially in generic template
/// code.
template <class ToType, IntoIteratorAny IntoIter>
  requires(
      FromIterator<ToType, typename IntoIteratorOutputType<IntoIter>::Item>)
constexpr inline ToType from_iter(IntoIter&& into_iter) noexcept
  requires(::sus::mem::IsMoveRef<decltype(into_iter)>)
{
  return FromIteratorImpl<ToType>::from_iter(
      ::sus::move(into_iter).into_iter());
}

}  // namespace sus::iter
