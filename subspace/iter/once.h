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

#include "subspace/iter/iterator_defn.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"

namespace sus::iter {

using ::sus::option::Option;

// clang-format off
sus_clang_bug_58859(
  namespace __private {

  template <class Item>
  inline Iterator<Once<Item>> once(Option<Item>&& single) noexcept {
    return Once<Item>::with_option(::sus::move(single));
  }

  }
)
// clang-format on

/// An IteratorBase implementation that walks over at most a single Item.
template <class Item>
class Once : public IteratorBase<Item> {
 public:
  Option<Item> next() noexcept final { return single_.take(); }

 protected:
  Once(Option<Item>&& single) : single_(::sus::move(single)) {}

 private:
  template <class U>
  friend inline Iterator<Once<U>> once(Option<U>&& single) noexcept
    requires(sus::mem::Move<U>);
  // clang-format off
  sus_clang_bug_58859(
    template <class U>
    friend inline Iterator<Once<U>> __private::once(Option<U>&& single) noexcept
  );
  // clang-format on

  static Iterator<Once> with_option(Option<Item>&& single) {
    return Iterator<Once>(::sus::move(single));
  }

  Option<Item> single_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(single_));
};

template <class Item>
inline Iterator<Once<Item>> once(Option<Item>&& single) noexcept
  requires(sus::mem::Move<Item>)
{
  sus_clang_bug_58859(return __private::once(::sus::move(single)));
  sus_clang_bug_58859_else(return Once<Item>::with_option(::sus::move(single)));
}

}  // namespace sus::iter
