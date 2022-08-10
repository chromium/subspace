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

#include "num/__private/float_macros.h"
#include "num/__private/literals.h"

namespace sus::num {

struct f32 final {
  _sus__float(f32, float)
};

struct f64 final {
  _sus__float(f64, double)
};

}  // namespace sus::num

_sus__float_literal(f32, ::sus::num::f32);
_sus__float_literal(f64, ::sus::num::f64);

using sus::num::f32;
using sus::num::f64;
