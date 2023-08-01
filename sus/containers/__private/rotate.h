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

#include "sus/mem/relocate.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ptr/swap.h"

namespace sus::container::__private {

template <class T>
inline void ptr_rotate(::sus::num::usize left, T* mid,
                       ::sus::num::usize right) noexcept {
  if constexpr (!::sus::mem::relocate_by_memcpy<T>) {
  } else {
  }
}

}  // namespace sus::container::__private
