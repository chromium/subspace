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

#include <stddef.h>  // Replace with usize?

#include <type_traits>

#include "assertions/check.h"
#include "macros/__private/compiler_bugs.h"
#include "marker/unsafe.h"
#include "mem/move.h"
#include "mem/relocate.h"

namespace sus::mem {

/// Annotate an lvalue usage, for static analysis.
template <class T>
  requires(!std::is_reference_v<T>)
inline constexpr T& mref(T& t) noexcept {
  return t;
}

}  // namespace sus::mem

// Promote mref() into the `sus` namespace.
namespace sus {
using ::sus::mem::mref;
}  // namespace sus
