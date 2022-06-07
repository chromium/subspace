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

#include "traits/iter/iterator_defn.h"

namespace sus::traits::iter {

using ::sus::option::Option;
using ::sus::traits::iter::Iterator;

/// An Iterator implementation that walks over at most a single Item.
template <class Item>
class SingleIter final : public Iterator<Item> {
 public:
  SingleIter(Option<Item>&& single)
      : single_(static_cast<decltype(single)&&>(single)) {}

  Option<Item> next() noexcept final { return single_.take(); }

 private:
  Option<Item> single_;
};

}  // namespace sus::traits::iter
