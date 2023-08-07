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

#include "sus/iter/from_iterator.h"
#include "sus/iter/into_iterator.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/ops/try.h"
#include "sus/option/option.h"

namespace sus::iter {

namespace __private {

template <class SourceIter>
struct TryFromIteratorUnwrapper final
    : public IteratorBase<
          TryFromIteratorUnwrapper<SourceIter>,
          ::sus::ops::TryOutputType<typename SourceIter::Item>> {
  using FromItem = typename SourceIter::Item;
  using Item = ::sus::ops::TryOutputType<FromItem>;

  constexpr TryFromIteratorUnwrapper(SourceIter& iter,
                                     ::sus::Option<FromItem>& failure)
      : iter(iter), failure(failure) {}

  constexpr ::sus::Option<Item> next() noexcept {
    ::sus::option::Option<FromItem> input = iter.next();
    if (input.is_some()) {
      if (::sus::ops::try_is_success(input.as_value())) {
        return ::sus::move(input).map(&::sus::ops::try_into_output<FromItem>);
      } else {
        failure = ::sus::move(input);
      }
    }
    return ::sus::Option<Item>();
  }

  constexpr SizeHint size_hint() const noexcept {
    return SizeHint(0u, iter.size_hint().upper);
  }

  SourceIter& iter;
  ::sus::Option<FromItem>& failure;
};

}  // namespace __private

/// Constructs `ToType` from a type that can be turned into an `Iterator` over
/// elements of type `ItemType`.
///
/// If a failure value is seen in the iterator, then the failure value will be
/// returned. Otherwise, the `ToType` success type (`TryOutputType<ToType>`) is
/// constructed from the success values in the Iterator, and a
/// success-representing `ToType` is returned containing that success type.
///
/// This is the other end of
/// [`Iterator::try_collect()`](::sus::iter::IteratorBase::try_collect), and is
/// typically called through calling `try_collect()` on an iterator. However
/// this function can be preferrable for some readers, especially in generic
/// template code.
template <class C, IntoIteratorAny IntoIter, int&...,
          class FromType = typename IntoIteratorOutputType<IntoIter>::Item,
          class ToType = ::sus::ops::TryRemapOutputType<FromType, C>>
  requires(::sus::mem::IsMoveRef<IntoIter &&> &&  //
           ::sus::ops::Try<FromType> &&           //
           FromIterator<C, ::sus::ops::TryOutputType<FromType>> &&
           // Void can not be collected from.
           !std::is_void_v<::sus::ops::TryOutputType<FromType>>)
constexpr inline ToType try_from_iter(IntoIter&& into_iter) noexcept {
  // This is supposed to be guaranteed by TryRemapOutputType.
  static_assert(::sus::ops::TryErrorConvertibleTo<FromType, ToType>);

  auto&& iter = ::sus::move(into_iter).into_iter();
  using SourceIter = std::remove_reference_t<decltype(iter)>;

  ::sus::Option<typename SourceIter::Item> failure;
  auto out = ::sus::ops::try_from_output<ToType>(from_iter<C>(
      __private::TryFromIteratorUnwrapper<SourceIter>(iter, failure)));
  if (failure.is_some())
    out = ::sus::ops::try_preserve_error<ToType>(::sus::move(failure).unwrap());
  return out;
}

}  // namespace sus::iter
