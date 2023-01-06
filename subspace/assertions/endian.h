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

#include "subspace/macros/always_inline.h"

namespace sus::assertions {

constexpr sus_always_inline bool is_big_endian() {
#if _MSC_VER

#if _M_PPC
  return true;
#else
  return false;
#endif

#elif defined(__BYTE_ORDER__)

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return true;
#else
  return false;
#endif

#else

#error "Compiler doesn't specify __BYTE_ORDER__."

#endif
}

constexpr sus_always_inline bool is_little_endian() noexcept {
  return !is_big_endian();
}

}  // namespace sus::assertions
