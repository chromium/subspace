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

#include "sus/fn/fn_concepts.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::option::Option;

template <class ItemT, class RepeatFn>
class RepeatWith;

/// Creates a new iterator that repeats elements of type `Item` endlessly by
/// applying the provided closure, the repeater,
/// [`FnMut<Item()>`]($sus::fn::FnMut).
///
/// The `repeat_with` function calls the repeater over and over again.
/// Infinite iterators like `repeat_with` are often used with adapters like
/// [`Iterator::take()`]($sus::iter::IteratorBase::take), in order to make them
/// finite.
///
/// If the element type of the iterator you need implements [`Clone`](
/// $sus::mem::Clone), and it is OK to keep the source element in memory, you
/// should instead use the [`repeat`]($sus::iter::repeat) function.
///
/// # Exampler
/// ```
/// auto r = sus::iter::repeat_with<u16>([] { return 3_u16; });
/// sus_check(r.next().unwrap() == 3_u16);
/// sus_check(r.next().unwrap() == 3_u16);
/// sus_check(r.next().unwrap() == 3_u16);
/// ```
template <class Item, ::sus::fn::FnMut<Item()> RepeatFn>
constexpr inline RepeatWith<Item, RepeatFn> repeat_with(
    RepeatFn repeater) noexcept {
  return RepeatWith<Item, RepeatFn>(::sus::move(repeater));
}

/// An Iterator that walks over at most a single Item.
template <class ItemT, class RepeatFn>
class [[nodiscard]] RepeatWith final
    : public IteratorBase<RepeatWith<ItemT, RepeatFn>, ItemT> {
 public:
  using Item = ItemT;

  // Type is Move and (can be) Clone.
  RepeatWith(RepeatWith&&) = default;
  RepeatWith& operator=(RepeatWith&&) = default;

  // sus::mem::Clone trait.
  constexpr RepeatWith clone() const noexcept
    requires(::sus::mem::Clone<RepeatFn>)
  {
    return RepeatWith(sus::clone(repeater_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    return ::sus::some(::sus::fn::call_mut(repeater_));
  }
  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    return SizeHint(usize::MAX, ::sus::Option<::sus::num::usize>());
  }
  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
    return ::sus::some(::sus::fn::call_mut(repeater_));
  }

 private:
  friend constexpr RepeatWith<Item, RepeatFn> sus::iter::repeat_with<Item>(
      RepeatFn repeater) noexcept;

  constexpr RepeatWith(RepeatFn repeater) : repeater_(::sus::move(repeater)) {}

  RepeatFn repeater_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(repeater_));
};

}  // namespace sus::iter
