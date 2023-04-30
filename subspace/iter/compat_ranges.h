// Copyright 2023 Google LLC
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

#include <iterator>

#include "subspace/iter/__private/iterator_end.h"
#include "subspace/iter/__private/range_begin.h"
#include "subspace/iter/iterator_concept.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/move.h"
#include "subspace/option/option.h"

namespace sus::iter {

/// Support for use of a sus::Iterator as a `std::ranges::input_range` in the
/// std::ranges library.
///
/// This type is returned from `Iterator::range()`.
template <class Iter>
class IteratorRange {
  using Item = typename Iter::Item;

 public:
  static constexpr auto with(Iter&& it) noexcept
    requires requires {
      typename Iter::Item;
      requires sus::iter::Iterator<Iter, typename Iter::Item>;
    }
  {
    return IteratorRange(::sus::move(it));
  }

  constexpr auto begin() noexcept {
    return __private::RangeBegin<IteratorRange, Item>(this);
  }
  constexpr auto end() noexcept { return __private::IteratorEnd(); }

 private:
  friend class __private::RangeBegin<IteratorRange, Item>;

  constexpr IteratorRange(Iter&& it) noexcept : it_(::sus::move(it)) {
    item_ = it_.next();
  }

  Iter it_;
  Option<Item> item_;
};

}  // namespace sus::iter

namespace std {

template <class IteratorRange, class Item>
struct iterator_traits<
    typename ::sus::iter::__private::RangeBegin<IteratorRange, Item>> {
  using difference_type = std::ptrdiff_t;
  using value_type = Item;
  using reference = Item&;
  using iterator_category = std::input_iterator_tag;
  using iterator_concept = std::input_iterator_tag;
};

}  // namespace std
