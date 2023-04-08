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
#include "subspace/num/signed_integer.h"
#include "subspace/ptr/copy.h"

namespace sus::num {

_sus__signed_out_of_line_impl(i8, /*PrimitiveT=*/int8_t, /*UnsignedT=*/u8);

_sus__signed_out_of_line_impl(i16, /*PrimitiveT=*/int16_t, /*UnsignedT=*/u16);

_sus__signed_out_of_line_impl(i32, /*PrimitiveT=*/int32_t, /*UnsignedT=*/u32);

_sus__signed_out_of_line_impl(i64, /*PrimitiveT=*/int64_t,
                              /*UnsignedT=*/u64);

_sus__signed_out_of_line_impl(
    isize,
    /*PrimitiveT=*/::sus::num::__private::ptr_type<>::signed_type,
    /*UnsignedT=*/usize);

}  // namespace sus::num
