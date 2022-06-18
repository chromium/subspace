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

#include "fn/fn_defn.h"
#include "iter/iterator_defn.h"
#include "iter/sized_iterator.h"
#include "mem/__private/relocatable_storage.h"
#include "mem/__private/relocate.h"

namespace sus::iter {

using ::sus::iter::IteratorBase;
using ::sus::mem::__private::RelocatableStorage;
using ::sus::mem::__private::relocate_one_by_memcpy_v;

template <class Item, size_t InnerIterSize, size_t InnerIterAlign>
class Filter : public IteratorBase<Item> {
  using Pred = ::sus::fn::FnMut<bool(
      // TODO: write a sus::const_ref<T>?
      const std::remove_reference_t<const std::remove_reference_t<Item>&>&)>;
  using InnerSizedIter = SizedIterator<Item, InnerIterSize, InnerIterAlign>;

  struct Data {
    Pred pred_;
    InnerSizedIter next_iter_;

    sus_class_maybe_trivial_relocatable_types(unsafe_fn, decltype(pred_),
                                              decltype(next_iter_));
  };

 public:
  Option<Item> next() noexcept final {
    IteratorBase<Item>& next_iter =
        data_.storage_mut().next_iter_.iterator_mut();
    Pred& pred = data_.storage_mut().pred_;

    // TODO: Just call find(pred) on itself?
    Option<Item> item = next_iter.next();
    while (item.is_some() && !pred.call_mut(item.as_ref().unwrap_unchecked(unsafe_fn))) {
      item = next_iter.next();
    }
    return item;
  }

 protected:
  Filter(Pred&& pred, InnerSizedIter&& next_iter)
      : data_(Option<Data>::some(Data{
            .pred_ = static_cast<decltype(pred)&&>(pred),
            .next_iter_ = static_cast<decltype(next_iter)&&>(next_iter)})) {}

 private:
  RelocatableStorage<Data> data_;

  sus_class_maybe_trivial_relocatable_types(unsafe_fn, decltype(data_));
};

}  // namespace sus::iter
