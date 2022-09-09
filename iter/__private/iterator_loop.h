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

#include "option/option.h"

namespace sus::iter::__private {

/// An adaptor for range-based for loops.
template <class Iterator>
class IteratorLoop final {
  using Item = typename std::remove_reference_t<Iterator>::Item;

 public:
  IteratorLoop(Iterator iter)
      : iter_(static_cast<Iterator&&>(iter)), item_(iter_.next()) {}

  inline bool operator==(const __private::IteratorEnd&) const {
    return item_.is_nome();
  }
  inline bool operator!=(const __private::IteratorEnd&) const {
    return item_.is_some();
  }
  inline void operator++() & { item_ = iter_.next(); }
  inline Item operator*() & { return item_.take().unwrap(); }

 private:
  /* TODO: NonNull<IteratorBase<Item>> */ Iterator iter_;
  Option<Item> item_;
};

// ADL helpers to call T::iter() in a range-based for loop, which will call
// `begin(T)`.

template <class T>
constexpr auto begin(const T& t) noexcept {
  return IteratorLoop(t.iter());
}

template <class T>
constexpr auto end(const T&) noexcept {
  return ::sus::iter::__private::IteratorEnd();
}

}  // namespace sus::iter::__private
