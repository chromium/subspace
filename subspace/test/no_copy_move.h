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

#include "mem/clone.h"
#include "mem/copy.h"
#include "mem/move.h"

namespace sus::test {

struct NoCopyMove {
  NoCopyMove() = default;
  NoCopyMove(NoCopyMove&&) = delete;
  NoCopyMove& operator=(NoCopyMove&&) = delete;

  friend constexpr bool operator==(const NoCopyMove& lhs,
                                   const NoCopyMove& rhs) noexcept {
    return &lhs == &rhs;
  }
  friend constexpr auto operator<=>(const NoCopyMove& lhs,
                                    const NoCopyMove& rhs) noexcept {
    return &lhs <=> &rhs;
  }
};

static_assert(!sus::mem::Copy<NoCopyMove>);
static_assert(!sus::mem::Clone<NoCopyMove>);
static_assert(!sus::mem::Move<NoCopyMove>);
static_assert(NoCopyMove() != NoCopyMove());
static_assert([]() {
  NoCopyMove n;
  return n == n;
});

}  // namespace sus::test
