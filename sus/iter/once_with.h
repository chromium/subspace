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
class OnceWith;

/// Creates an iterator that lazily generates a value exactly once by invoking
/// the provided closure.
///
/// This is commonly used to adapt a single value generator into a `chain()` of
/// other kinds of iteration. Maybe you have an iterator that covers almost
/// everything, but you need an extra special case. Maybe you have a function
/// which works on iterators, but you only need to process one value.
///
/// Unlike `once()`, this function will lazily generate the value on request.
///
/// # Example
/// ```
/// auto ow = sus::iter::once_with<u16>([]() { return 3_u16; });
/// sus::check(ow.next().unwrap() == 3_u16);
/// ```
template <class Item>
inline OnceWith<Item> once_with(::sus::fn::FnMutBox<Item()> gen) noexcept {
  return OnceWith<Item>(::sus::move(gen));
}

/// An Iterator that walks over at most a single Item.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] OnceWith final
    : public IteratorBase<OnceWith<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    return gen_.take().map([](auto&& gen) { return ::sus::fn::call_mut(gen); });
  }
  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    ::sus::num::usize rem = gen_.is_some() ? 1u : 0u;
    return SizeHint(rem, ::sus::Option<::sus::num::usize>::with(rem));
  }
  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    return gen_.take().map([](auto&& gen) { return ::sus::fn::call_mut(gen); });
  }
  // sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept { return gen_.is_some() ? 1u : 0u; }

 private:
  friend OnceWith<Item> sus::iter::once_with<Item>(
      ::sus::fn::FnMutBox<Item()> gen) noexcept;

  OnceWith(::sus::fn::FnMutBox<Item()> gen)
      : gen_(sus::Option<::sus::fn::FnMutBox<Item()>>::with(::sus::move(gen))) {
  }

  Option<::sus::fn::FnMutBox<Item()>> gen_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(gen_));
};

}  // namespace sus::iter
