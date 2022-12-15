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

#include "iter/iterator_defn.h"
#include "mem/relocate.h"

namespace sus::iter {

template <class Item, size_t SubclassSize, size_t SubclassAlign,
          bool InlineStorage = true>
struct [[sus_trivial_abi]] SizedIterator final {
  SizedIterator(void (*destroy)(char& sized)) : destroy(destroy) {}

  SizedIterator(SizedIterator&& o)
      : destroy(::sus::mem::replace_ptr(mref(o.destroy), nullptr)) {
    memcpy(sized, o.sized, SubclassSize);
  }
  SizedIterator& operator=(SizedIterator&& o) = delete;

  ~SizedIterator() {
    if (destroy) destroy(*sized);
  }

  IteratorBase<Item>& iterator_mut() {
    // The `sized` array points to the stored Iterator subclass, which we cast
    // to the base class.
    return *reinterpret_cast<IteratorBase<Item>*>(sized);
  }

  alignas(SubclassAlign) char sized[SubclassSize];
  void (*destroy)(char& sized);

 private:
  sus_class_assert_trivial_relocatable_types(::sus::marker::unsafe_fn, decltype(sized[0]),
                                             decltype(destroy));
};

template <class Item, size_t SubclassSize, size_t SubclassAlign>
struct [[sus_trivial_abi]] SizedIterator<Item, SubclassSize, SubclassAlign,
                                         /*InlineStorage=*/false>
    final {
  SizedIterator(IteratorBase<Item>& iter,
                void (*destroy)(IteratorBase<Item>& sized))
      : iter(&iter), destroy(destroy) {}

  SizedIterator(SizedIterator&& o)
      : iter(::sus::mem::replace_ptr(mref(o.iter), nullptr)),
        destroy(::sus::mem::replace_ptr(mref(o.destroy), nullptr)) {}
  SizedIterator& operator=(SizedIterator&& o) = delete;

  ~SizedIterator() {
    if (destroy) destroy(*iter);
  }

  IteratorBase<Item>& iterator_mut() { return *iter; }

  IteratorBase<Item>* iter;
  void (*destroy)(IteratorBase<Item>& sized);

 private:
  sus_class_assert_trivial_relocatable_types(::sus::marker::unsafe_fn, decltype(iter),
                                             decltype(destroy));
};

/// Make a SizedIterator.
///
/// This overload is used when the IteratorSubclass can be trivially relocated.
/// It stores the SubclassIterator directly into the SizedIterator, avoiding a
/// heap allocation, since the SizedIterator can then be trivially relocated.
template <::sus::mem::Move IteratorSubclass, int&...,
          class SubclassItem = typename IteratorSubclass::Item,
          class SizedIteratorType =
              SizedIterator<SubclassItem, sizeof(IteratorSubclass),
                            alignof(IteratorSubclass)>>
inline SizedIteratorType make_sized_iterator(IteratorSubclass&& subclass)
  requires(
      std::is_convertible_v<IteratorSubclass&, IteratorBase<SubclassItem>&> &&
      ::sus::mem::relocate_one_by_memcpy<IteratorSubclass>)
{
  auto it = SizedIteratorType([](char& sized) {
    reinterpret_cast<IteratorSubclass&>(sized).~IteratorSubclass();
  });
  new (it.sized) IteratorSubclass(::sus::move(subclass));
  return it;
}

/// Make a SizedIterator.
///
/// This overload is used when the IteratorSubclass can not be trivially
/// relocated. Therefore it heap allocates and moves the IteratorSubclass onto
/// the heap. That way the IteratorSubclass does not need to be moved again, but
/// the pointer and the SizedIterator can be trivially relocated.
template <::sus::mem::Move IteratorSubclass, int&...,
          class SubclassItem = typename IteratorSubclass::Item,
          class SizedIteratorType =
              SizedIterator<SubclassItem, sizeof(IteratorSubclass),
                            alignof(IteratorSubclass), /*InlineStorage=*/false>>
inline SizedIteratorType make_sized_iterator(IteratorSubclass&& subclass)
  requires(
      std::is_convertible_v<IteratorSubclass&, IteratorBase<SubclassItem>&> &&
      !::sus::mem::relocate_one_by_memcpy<IteratorSubclass>)
{
  return SizedIteratorType(*new IteratorSubclass(::sus::move(subclass)),
                           [](IteratorBase<SubclassItem>& iter) {
                             delete reinterpret_cast<IteratorSubclass*>(&iter);
                           });
}

}  // namespace sus::iter
