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

#include "sus/fn/fn_box_defn.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::option::Option;

template <class ItemT, class GenFn>
class RepeatWith;

/// Creates a new iterator that repeats elements of type `Item` endlessly by
/// applying the provided closure, the repeater, `FnMut<Item()>`.
///
/// The `repeat_with()` function calls the repeater over and over again.
///
/// Infinite iterators like `repeat_with()` are often used with adapters like
/// [`Iterator::take()`]($sus::iter::IteratorBase::take), in order to make them
/// finite.
///
/// If the element type of the iterator you need implements `Clone`, and it is
/// OK to keep the source element in memory, you should instead use the
/// [`repeat()`]($sus::iter::repeat) function.
///
/// # Exampler
/// ```
/// auto r = sus::iter::repeat_with<u16>([] { return 3_u16; });
/// sus::check(r.next().unwrap() == 3_u16);
/// sus::check(r.next().unwrap() == 3_u16);
/// sus::check(r.next().unwrap() == 3_u16);
/// ```
template <class Item, ::sus::fn::FnMut<Item()> GenFn>
constexpr inline RepeatWith<Item, GenFn> repeat_with(GenFn gen) noexcept {
  return RepeatWith<Item, GenFn>(::sus::move(gen));
}

/// An Iterator that walks over at most a single Item.
template <class ItemT, class GenFn>
class [[nodiscard]] RepeatWith final
    : public IteratorBase<RepeatWith<ItemT, GenFn>, ItemT> {
 public:
  using Item = ItemT;

  // Type is Move and (can be) Clone.
  RepeatWith(RepeatWith&&) = default;
  RepeatWith& operator=(RepeatWith&&) = default;

  // sus::mem::Clone trait.
  constexpr RepeatWith clone() const noexcept
    requires(::sus::mem::Clone<GenFn>)
  {
    return RepeatWith(sus::clone(gen_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    return ::sus::some(::sus::fn::call_mut(gen_));
  }
  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    return SizeHint(usize::MAX, ::sus::Option<::sus::num::usize>());
  }
  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
    return ::sus::some(::sus::fn::call_mut(gen_));
  }

 private:
  friend constexpr RepeatWith<Item, GenFn> sus::iter::repeat_with<Item>(
      GenFn gen) noexcept;

  constexpr RepeatWith(GenFn gen) : gen_(::sus::move(gen)) {}

  GenFn gen_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(gen_));
};

}  // namespace sus::iter
