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

#include "sus/lib/__private/forward_decl.h"
#include "sus/iter/into_iterator.h"

namespace sus::iter::__private {

template <class T>
struct IntoIteratorArchetype {
  template <class Item>
  struct Iter final : public IteratorBase<Iter<Item>, Item> {
    ::sus::option::Option<Item> next() noexcept;
    SizeHint size_hint() const noexcept;
  };
  Iter<T> into_iter() && { return Iter<T>(); }
};

}  // namespace sus::iter::__private
