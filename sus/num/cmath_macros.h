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

// IWYU pragma: private, include "sus/num/types.h"
// IWYU pragma: friend "sus/.*"
#pragma once

// By including math.h (and cmath) and then #undef-ing NAN and INFINITY, we
// prevent these macros from polluting the symbol namespace, which would
// conflict with constants in f32 and f64. We provide alternative `SUS_NAN` and
// `SUS_INFINITY` macros that provide NaN and Infinity float values.
//
// A codebase in migration to subspace would use `SUS_NAN` in place of NAN and
// `SUS_INFINITY` in place of INFINITY, or could move to using the constants in
// std::numeric_limits<float>.
#include <math.h>

#include <cmath>

#undef NAN
#undef INFINITY

#include "sus/num/__private/intrinsics.h"

/// A macro that replaces the `NAN` macro from the `<cmath>` header. Consider
/// using f32::NAN instead.
#define SUS_NAN ::sus::num::__private::nan<float>()
/// A macro that replaces the `INFINITY` macro from the `<cmath>` header.
/// Consider using f32::INFINITY instead.
#define SUS_INFINITY ::sus::num::__private::infinity<float>()
