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

#include <type_traits>

#include "subspace/macros/pure.h"

namespace sus::mem {

template <class T>
sus_pure_const constexpr T* addressof(T& arg) noexcept {
  // __builtin_addressof also handles Obj-C ARC pointers.
  return __builtin_addressof(arg);
}

template <class T>
T* addressof(T&& arg) = delete;

}  // namespace sus::mem
