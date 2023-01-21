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

#include <source_location>

#include "subspace/assertions/panic.h"
#include "subspace/macros/always_inline.h"
#include "subspace/macros/builtin.h"
#include "subspace/marker/unsafe.h"

namespace sus::assertions {

[[noreturn]] sus_always_inline void unreachable(
    const std::source_location location = std::source_location::current()) {
  panic(location);
}

[[noreturn]] sus_always_inline void unreachable_unchecked(
    ::sus::marker::UnsafeFnMarker) {
#if __has_builtin(__builtin_unreachable)
  __builtin_unreachable();
#else
  __assume(false);
#endif
}

}  // namespace sus::assertions

namespace sus {
using sus::assertions::unreachable;
using sus::assertions::unreachable_unchecked;
}  // namespace sus
