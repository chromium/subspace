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

#include "sus/iter/__private/iterator_end.h"

namespace sus::iter::__private {

template <class IteratorRange, class Item>
class RangeBegin {
 public:
  explicit RangeBegin(IteratorRange* r) : range_(r) {}

  // This violates const correctness but it's required to by
  // `std::indirectly_readable`, which we need to satisfy to also satisfy
  // `std::input_iterator`, so that `sus::iter::IteratorRange` can satisfy
  // `std::input_range`.
  Item& operator*() const& noexcept { return *range_->item_; }

  RangeBegin& operator++() & noexcept {
    range_->item_ = range_->it_.next();
    return *this;
  }
  void operator++(int) & noexcept { range_->item_ = range_->it_.next(); }

  bool operator==(::sus::iter::__private::IteratorEnd) const noexcept {
    return range_->item_.is_none();
  }

 private:
  IteratorRange* range_;
};

}  // namespace sus::iter::__private
