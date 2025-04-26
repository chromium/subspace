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

#include <stddef.h>
#include <stdint.h>

#include "sus/collections/array.h"
#include "sus/num/unsigned_integer.h"
#include "sus/option/option.h"
#include "sus/ptr/copy.h"
#include "sus/result/result.h"
#include "sus/tuple/tuple.h"

#define _self u32
#define _pointer 0
#define _primitive uint32_t
#define _signed i32
#include "sus/num/__private/unsigned_integer_methods_impl.inc"

#define _self u8
#define _pointer 0
#define _primitive uint8_t
#define _signed i8
#include "sus/num/__private/unsigned_integer_methods_impl.inc"

#define _self u16
#define _pointer 0
#define _primitive uint16_t
#define _signed i16
#include "sus/num/__private/unsigned_integer_methods_impl.inc"

#define _self u64
#define _pointer 0
#define _primitive uint64_t
#define _signed i64
#include "sus/num/__private/unsigned_integer_methods_impl.inc"

#define _self usize
#define _pointer 0
#define _primitive ::sus::num::__private::ptr_type<::sus::mem::size_of<size_t>()>::unsigned_type
#define _signed isize
#include "sus/num/__private/unsigned_integer_methods_impl.inc"

#define _self uptr
#define _pointer 1
#define _primitive ::sus::num::__private::ptr_type<::sus::mem::size_of<uintptr_t>()>::unsigned_type
#include "sus/num/__private/unsigned_integer_methods_impl.inc"

#include "sus/num/unsigned_integer_consts.h"
