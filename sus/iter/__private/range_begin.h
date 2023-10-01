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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include <iterator>

#include "sus/iter/__private/iterator_end.h"
#include "sus/mem/addressof.h"

namespace sus::iter::__private {

/// The std iterator type that works with IteratorRange, which is a std range.
template <class IteratorRange, class Item>
class RangeBegin {
 public:
  constexpr explicit RangeBegin(IteratorRange* r) : range_(r) {}

  // This violates const correctness but it's required to by
  // `std::indirectly_readable`, which we need to satisfy to also satisfy
  // `std::input_iterator`, so that `sus::iter::IteratorRange` can satisfy
  // `std::input_range`.
  constexpr Item& operator*() const& noexcept { return *range_->item_; }

  constexpr RangeBegin& operator++() & noexcept {
    range_->step();
    return *this;
  }
  constexpr RangeBegin& operator++(int) & noexcept {
    range_->step();
    return *this;
  }

  constexpr bool operator==(
      ::sus::iter::__private::IteratorEnd) const noexcept {
    return range_->item_.is_none();
  }

 private:
  IteratorRange* range_;
};

}  // namespace sus::iter::__private

namespace std {

template <class IteratorRange, class Item>
struct iterator_traits<
    typename ::sus::iter::__private::RangeBegin<IteratorRange, Item&>> {
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_cv_t<Item>;
  using reference = Item&;
  using iterator_concept = std::input_iterator_tag;
};

template <class IteratorRange, class Item>
struct iterator_traits<
    typename ::sus::iter::__private::RangeBegin<IteratorRange, Item>> {
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_cv_t<Item>;
  using reference = Item&;
  using iterator_concept = std::input_iterator_tag;
};

}  // namespace std
