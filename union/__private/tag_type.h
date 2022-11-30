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

namespace sus::union_type::__private {

// clang-format off
template <size_t count>
using TagType =
  std::conditional_t<count - 1u <= size_t{0xff}, uint8_t,
  std::conditional_t<count - 1u <= size_t{0xffff}, uint16_t,
  std::conditional_t<count - 1u <= size_t{0xffffffff}, uint32_t,
  uint64_t>>>;
// clang-format on

}  // namespace sus::union_type::__private
