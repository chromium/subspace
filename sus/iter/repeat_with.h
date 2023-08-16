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

template <class ItemT>
class RepeatWith;

/// Creates a new iterator that repeats elements of type `Item` endlessly by
/// applying the provided closure, the repeater, `FnMut<Item()>`.
///
/// The `repeat_with()` function calls the repeater over and over again.
///
/// Infinite iterators like `repeat_with()` are often used with adapters like
/// [`Iterator::take()`](sus-iter-IteratorBase.html#method.take), in order to make them
/// finite.
///
/// If the element type of the iterator you need implements `Clone`, and it is
/// OK to keep the source element in memory, you should instead use the
/// [`repeat()`](sus-iter-fn.repeat.html) function.
///
/// # Exampler
/// ```
/// auto r = sus::iter::repeat_with<u16>([] { return 3_u16; });
/// sus::check(r.next().unwrap() == 3_u16);
/// sus::check(r.next().unwrap() == 3_u16);
/// sus::check(r.next().unwrap() == 3_u16);
/// ```
template <class Item>
inline RepeatWith<Item> repeat_with(::sus::fn::FnMutBox<Item()> gen) noexcept {
  return RepeatWith<Item>(::sus::move(gen));
}

/// An Iterator that walks over at most a single Item.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] RepeatWith final
    : public IteratorBase<RepeatWith<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    return ::sus::some(::sus::fn::call_mut(gen_));
  }
  /// sus::iter::Iterator trait.
  SizeHint size_hint() const noexcept {
    return SizeHint(usize::MAX, ::sus::Option<::sus::num::usize>());
  }
  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    return ::sus::some(::sus::fn::call_mut(gen_));
  }

 private:
  friend RepeatWith<Item> sus::iter::repeat_with<Item>(
      ::sus::fn::FnMutBox<Item()> gen) noexcept;

  RepeatWith(::sus::fn::FnMutBox<Item()> gen) : gen_(::sus::move(gen)) {}

  ::sus::fn::FnMutBox<Item()> gen_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(gen_));
};

}  // namespace sus::iter
