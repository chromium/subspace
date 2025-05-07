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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/iter/__private/iterator_end.h"
#include "sus/macros/lifetimebound.h"
#include "sus/option/option.h"

namespace sus::iter::__private {

/// An adaptor for range-based for loops.
template <class Iter>
class [[nodiscard]] IteratorLoop final {
  using Item = typename std::remove_reference_t<Iter>::Item;

 public:
  constexpr IteratorLoop(Iter&& iter) noexcept
      : iter_(::sus::forward<Iter>(iter)), item_(iter_.next()) {}

  constexpr friend bool operator==(const IteratorLoop& x,
                                   __private::IteratorEnd) noexcept {
    return x.item_.is_none();
  }
  constexpr inline void operator++() & noexcept { item_ = iter_.next(); }
  constexpr inline Item operator*() & noexcept {
    // UB occurs if operator*() is called after IteratorLoop == IteratorEnd.
    // This can not occur in a ranged-for loop, and IteratorLoop should never be
    // held in other contexts.
    //
    // TODO: Write a clang-tidy for this.
    return item_.take().unwrap_unchecked(::sus::marker::unsafe_fn);
  }

 private:
  Iter iter_;
  Option<Item> item_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(iter_), decltype(item_));
};

}  // namespace sus::iter::__private

namespace sus::iter {

/// ADL helper to call `T::iter()` in a range-based for loop, which will call
/// `begin(T)`.
///
/// In the same namespace as a type that has iter(), place:
/// ```
/// using ::sus::iter::begin;
/// using ::sus::iter::end;
/// ```
template <class T>
  requires requires(const T& t) {
    { t.iter() };
  }
constexpr auto begin(const T& t) noexcept {
  return __private::IteratorLoop(t.iter());
}

/// ADL helper to call `T::iter()` in a range-based for loop, which will call
/// `end(T)`.
///
/// In the same namespace as a type that has iter(), place:
/// ```
/// using ::sus::iter:::begin;
/// using ::sus::iter::end;
/// ```
template <class T>
  requires requires(const T& t) {
    { t.iter() };
  }
constexpr auto end(const T&) noexcept {
  return __private::IteratorEnd();
}

}  // namespace sus::iter
