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

#ifdef TEST_MODULE
import sus;
#else
#include "sus/mem/clone.h"

#include "sus/num/types.h"
#include "sus/prelude.h"
#endif

#include "googletest/include/gtest/gtest.h"
#include "sus/macros/__private/compiler_bugs.h"

using sus::mem::Clone;
using sus::mem::clone;
using sus::mem::clone_into;
using sus::mem::Copy;

namespace {

struct Cloneable {
  Cloneable() = default;

  Cloneable clone() const noexcept { return Cloneable(); }

  Cloneable(Cloneable&&) = default;
  Cloneable& operator=(Cloneable&&) = default;
};

struct Copyable {
  Copyable() = default;
  Copyable(const Copyable&) = default;
  Copyable& operator=(const Copyable&) = default;
};

struct NoCloneable {
  NoCloneable(const NoCloneable&) = delete;
  NoCloneable& operator=(NoCloneable&&) = default;
};

struct CopyCloneConflict {
  CopyCloneConflict() = default;

  CopyCloneConflict clone() const noexcept { return CopyCloneConflict(); }

  CopyCloneConflict(const CopyCloneConflict&) = default;
  CopyCloneConflict& operator=(const CopyCloneConflict&) = default;
};

static_assert(Clone<Cloneable>);
static_assert(!Copy<Cloneable>);

static_assert(Clone<Copyable>);
static_assert(Copy<Copyable>);

static_assert(!Clone<NoCloneable>);
static_assert(!Copy<NoCloneable>);

static_assert(!Clone<CopyCloneConflict>);
static_assert(Copy<CopyCloneConflict>);

TEST(Clone, Copy) {
  struct S {
    sus_clang_bug_54040(S(i32 i) : i(i){});

    i32 i;
  };

  auto s1 = S(2_i32);
  auto s2 = clone(s1);
  EXPECT_EQ(s1.i, s2.i);

  s1 = S(5_i32);
  EXPECT_EQ(s1.i, 5_i32);
  clone_into(s1, s2);
  EXPECT_EQ(s1.i, 2_i32);
}

TEST(Clone, Clone) {
  struct S {
    S(i32 i) : i(i) {}
    S(S&&) = default;
    S& operator=(S&&) = default;

    S clone() const { return S(i); }

    i32 i;
  };

  auto s1 = S(2_i32);
  auto s2 = clone(s1);
  EXPECT_EQ(s1.i, s2.i);

  s1 = S(5_i32);
  EXPECT_EQ(s1.i, 5_i32);
  clone_into(s1, s2);
  EXPECT_EQ(s1.i, 2_i32);
}

}  // namespace
