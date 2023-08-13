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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include <stddef.h>

#include "sus/lib/__private/forward_decl.h"
#include "sus/mem/size_of.h"

namespace sus::num::__private {

template <unsigned int bytes = ::sus::mem::size_of<void*>()>
struct ptr_type;

template <>
struct ptr_type<4> {
  using unsigned_type = uint32_t;
  using signed_type = int32_t;
  using pointer_sized_type = u32;
};

template <>
struct ptr_type<8> {
  using unsigned_type = uint64_t;
  using signed_type = int64_t;
  using pointer_sized_type = u64;
};

template <unsigned int bytes = ::sus::mem::size_of<size_t>()>
struct addr_type;

template <>
struct addr_type<4> {
  using unsigned_type = uint32_t;
  using signed_type = int32_t;
};

template <>
struct addr_type<8> {
  using unsigned_type = uint64_t;
  using signed_type = int64_t;
};

}  // namespace sus::num::__private
