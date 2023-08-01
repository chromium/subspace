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

#include "sus/mem/size_of.h"
#include "sus/num/unsigned_integer.h"

namespace sus::choice_type::__private {

/// Determines the number of bytes to use in the index.
///
/// The index must have room for `count` many values. And we should use a 32-bit
/// value unless there's a reason not to. So we use something smaller only if
/// it fits into the inner union's tail padding.
template <size_t count, size_t size_of_union, size_t data_size_of_union>
consteval size_t index_size() noexcept {
  constexpr size_t size_of_padding = size_of_union - data_size_of_union;
  if constexpr (count + 1 <= size_t{0xff} && size_of_padding >= 8u) return 8u;
  if constexpr (count + 1 <= size_t{0xffff} && size_of_padding >= 16u)
    return 16u;
  if constexpr (count + 1 <= size_t{0xffffffff}) return 32u;
  return 64u;
}

/// Defines the type of the index.
///
/// Indexes with size greater than `sizeof(size_t)` bytes are not allowed.
/// It would introduce UB because we use sizeof...() on the pack of members of
/// the union, and it can only handle size_t many things.
///
// clang-format off
template <size_t count, size_t size_of_union, size_t data_size_of_union>
using IndexType =
  std::conditional_t<
      index_size<count, size_of_union, data_size_of_union>() == 8, uint8_t,
  std::conditional_t<
      index_size<count, size_of_union, data_size_of_union>() == 16, uint16_t,
  std::conditional_t<
      index_size<count, size_of_union, data_size_of_union>() == 32, uint32_t,
  std::conditional_t<
      ::sus::mem::size_of<size_t>() == ::sus::mem::size_of<uint64_t>(), uint64_t,
  void>>>>;
// clang-format on

}  // namespace sus::choice_type::__private
