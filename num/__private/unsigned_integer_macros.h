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

#include <stdint.h>

#include <concepts>
#include <type_traits>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace sus::num::__private {

// TODO: Move to a (constexpr) compiler intrinsics library?
template <class T>
  requires(std::same_as<T, uint32_t>)
constexpr uint32_t count_ones(T value) noexcept {
#if defined(_MSC_VER)
  if (std::is_constant_evaluated()) {
    // The low 2 bits are either:
    // - 0b00, (value + 1) / 2 is number of bits = 0.
    // - 0b01, (value + 1) / 2 is number of bits = 1.
    // - 0b11, value is 3. (value + 1) / 2 is the number of bits = 2.
    // - 0b10, value is 2. (value + 1) / 2 is the number of bits = 2.
    // So we can count the ones 2 at a time.
    uint32_t count = 0;
    for (uint32_t i = 0; i < sizeof(value); ++i) {
      count += ((value & 0b11) + 1) / 2;
      count >>= 2;
      count += ((value & 0b11) + 1) / 2;
      count >>= 2;
      count += ((value & 0b11) + 1) / 2;
      count >>= 2;
      count += ((value & 0b11) + 1) / 2;
      count >>= 2;
    }
    return count;
  } else {
    static_assert(sizeof(value) == 4);  // Use __popcnt16 and __popcnt64.
    return __popcnt(value);
  }
#else
  static_assert(sizeof(value) ==
                sizeof(unsigned int));  // Use popcountl and popcountll
  return __builtin_popcount(value);
#endif
}

}  // namespace sus::num::__private
