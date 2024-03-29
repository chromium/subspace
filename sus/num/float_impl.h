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

#include "sus/collections/array.h"
#include "sus/num/float.h"

#define _self f32
#define _primitive float
#define _unsigned u32
#include "sus/num/__private/float_methods_impl.inc"

#define _self f64
#define _primitive double
#define _unsigned u64
#include "sus/num/__private/float_methods_impl.inc"
