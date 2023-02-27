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

#include "subspace/iter/iterator_concept.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"
#include "subspace/option/option.h"
// Doesn't include iterator_defn.h because it's included from there.

namespace sus::iter {

template <class ItemT, size_t SubclassSize, size_t SubclassAlign,
          bool DoubleEndedB>
struct [[sus_trivial_abi]] SizedIterator final {
  using Item = ItemT;
  static constexpr bool DoubleEnded = DoubleEndedB;

  constexpr SizedIterator(void (*destroy)(char& sized),
                          Option<Item> (*next)(char& sized),
                          Option<Item> (*next_back)(char& sized))
      : destroy_(destroy), next_(next), next_back_(next_back) {}

  SizedIterator(SizedIterator&& o) noexcept
      : destroy_(::sus::mem::replace_ptr(mref(o.destroy_), nullptr)),
        next_(::sus::mem::replace_ptr(mref(o.next_), nullptr)),
        next_back_(::sus::mem::replace_ptr(mref(o.next_back_), nullptr)) {
    memcpy(sized_, o.sized_, SubclassSize);
  }
  SizedIterator& operator=(SizedIterator&& o) = delete;

  ~SizedIterator() noexcept {
    if (destroy_) destroy_(*sized_);
  }

  Option<Item> next() noexcept { return next_(*sized_); }
  Option<Item> next_back() noexcept
    requires(DoubleEnded)
  {
    return next_back_(*sized_);
  }

  char* as_ptr_mut() noexcept { return sized_; }

 private:
  alignas(SubclassAlign) char sized_[SubclassSize];
  void (*destroy_)(char& sized);
  Option<Item> (*next_)(char& sized);
  // TODO: We could remove this field with a nested struct + template
  // specialization when DoubleEnded is false.
  Option<Item> (*next_back_)(char& sized);

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(sized_),
                                  decltype(destroy_), decltype(next_back_));
};

template <class Iter>
struct SizedIteratorType {
  using type = SizedIterator<
      typename Iter::Item, ::sus::mem::size_of<Iter>(), alignof(Iter),
      ::sus::iter::DoubleEndedIterator<Iter, typename Iter::Item>>;
};

/// Make a SizedIterator which wraps a trivially relocatable iterator and erases
/// its type.
///
/// This type may only be used when the IteratorSubclass can be trivially
/// relocated. It stores the SubclassIterator directly into the SizedIterator,
/// erasing its type but remaining trivially relocatable.
template <::sus::mem::Move Iter, int&..., class Item = typename Iter::Item>
inline SizedIteratorType<Iter>::type make_sized_iterator(Iter&& iter)
  requires(::sus::iter::Iterator<Iter, Item> && ::sus::mem::relocate_by_memcpy<Iter>)
{
  // IteratorImpl also checks this. It's needed for correctness of the casts
  // here.
  static_assert(std::is_final_v<Iter>);

  void (*destroy)(char& sized) = [](char& sized) {
    reinterpret_cast<Iter&>(sized).~Iter();
  };
  Option<Item> (*next)(char& sized) = [](char& sized) {
    return reinterpret_cast<Iter&>(sized).next();
  };
  Option<Item> (*next_back)(char& sized);
  if constexpr (SizedIteratorType<Iter>::type::DoubleEnded) {
    next_back = [](char& sized) {
      return reinterpret_cast<Iter&>(sized).next_back();
    };
  } else {
    next_back = nullptr;
  }

  auto it = typename SizedIteratorType<Iter>::type(destroy, next, next_back);
  new (it.as_ptr_mut()) Iter(::sus::move(iter));
  return it;
}
}  // namespace sus::iter
