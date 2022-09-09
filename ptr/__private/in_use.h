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

#include "macros/always_inline.h"
#include "mem/relocate.h"
#include "ptr/ptr_concepts.h"

namespace sus::ptr {
template <class T>
class [[sus_trivial_abi]] Own;
}

namespace sus::ptr::__private {

template <class T>
class InUse {
 public:
  constexpr sus_always_inline ~InUse() noexcept { own.t_ = &t; }

  constexpr sus_always_inline const auto* operator->() const&& noexcept
    requires(HasArrow<T>)
  {
    return t.operator->();
  }
  constexpr sus_always_inline auto* operator->() && noexcept
    requires(HasArrowMut<T>)
  {
    return t.operator->();
  }

  constexpr sus_always_inline const auto* operator->() const&& noexcept
    requires(!HasArrowMut<T>)
  {
    return &t;
  }
  constexpr sus_always_inline auto* operator->() && noexcept
    requires(!HasArrowMut<T>)
  {
    return &t;
  }

  T& t;
  Own<T>& own;
};

}  // namespace sus::ptr::__private
