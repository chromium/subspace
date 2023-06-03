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

#include "subspace/containers/array.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/option/option.h"
#include "subspace/ptr/copy.h"
#include "subspace/result/result.h"

#define _self u32
#define _primitive uint32_t
#define _signed i32
#include "subspace/num/__private/unsigned_integer_methods_impl.inc"

#define _self u8
#define _primitive uint8_t
#define _signed i8
#include "subspace/num/__private/unsigned_integer_methods_impl.inc"

#define _self u16
#define _primitive uint16_t
#define _signed i16
#include "subspace/num/__private/unsigned_integer_methods_impl.inc"

#define _self u64
#define _primitive uint64_t
#define _signed i64
#include "subspace/num/__private/unsigned_integer_methods_impl.inc"

#define _self usize
#define _primitive ::sus::num::__private::ptr_type<>::unsigned_type
#define _signed isize
#include "subspace/num/__private/unsigned_integer_methods_impl.inc"
