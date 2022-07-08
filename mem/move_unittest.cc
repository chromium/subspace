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

#include "mem/move.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::move;

template <class T>
concept can_move = requires(T &&t) {
                     { move(t) };
                   };

static_assert(can_move<int>);
static_assert(can_move<int &>);
static_assert(can_move<int &&>);
static_assert(!can_move<const int &>);
static_assert(!can_move<const int &&>);

void bind_rvalue(int&&) {}
void bind_value(int) {}

TEST(Move, Binds) {
  int i;
  bind_rvalue(move(i));
  bind_rvalue(move(1));
  bind_value(move(i));
  bind_value(move(1));
}

struct MoveOnly {
  MoveOnly() = default;
  MoveOnly(MoveOnly&&) = default;
};

void bind_rvalue(MoveOnly&&) {}
void bind_value(MoveOnly) {}
void bind_const(const MoveOnly&) {}

TEST(Move, MoveOnly) {
  MoveOnly m;
  bind_rvalue(move(m));
  bind_rvalue(move(MoveOnly()));
  bind_value(move(m));
  bind_value(move(MoveOnly()));
}

}  // namespace
