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

#ifdef TEST_MODULE
import sus;
#else
#include "sus/option/compat_option.h"

#include "sus/construct/into.h"
#include "sus/prelude.h"
#endif

#include "googletest/include/gtest/gtest.h"
#include "sus/macros/__private/compiler_bugs.h"

namespace {

struct Moved {
  constexpr Moved() = default;
  constexpr Moved(Moved&& rhs) { moved = rhs.moved + 1; }
  constexpr void operator=(Moved&& rhs) { moved = rhs.moved + 1; }
  i32 moved = 0;
};

TEST(CompatOption, CtorOptionalCopy) {
  // Explicit.
  {
    constexpr auto o = std::optional<int>(2);
    auto s = sus::Option<int>(o);
    EXPECT_EQ(o.value(), s.as_value());
  }
  {
    constexpr auto o = std::optional<int>();
    auto s = sus::Option<int>(o);
    EXPECT_EQ(s.is_some(), false);
  }
  // Implicit.
  {
    constexpr auto o = std::optional<int>(2);
    sus::Option<int> s = o;
    EXPECT_EQ(o.value(), s.as_value());
  }
  {
    constexpr auto o = std::optional<int>();
    sus::Option<int> s = o;
    EXPECT_EQ(s.is_some(), false);
  }
}

TEST(CompatOption, CtorOptionalMove) {
  // Explicit.
  {
    constexpr auto s = sus::Option<int>(std::optional<int>(2));
    EXPECT_EQ(s.as_value(), 2);
  }
  {
    constexpr auto s = sus::Option<int>(std::optional<int>());
    EXPECT_EQ(s.is_some(), false);
  }
  // Implicit.
  {
    constexpr sus::Option<int> s = std::optional<int>(2);
    EXPECT_EQ(s.as_value(), 2);
  }
  {
    constexpr sus::Option<int> s = std::optional<int>();
    EXPECT_EQ(s.is_some(), false);
  }

  {
    constexpr auto sm = sus::Option<Moved>(std::optional<Moved>(std::in_place));
    EXPECT_GE(sm.as_value().moved, 1);
  }
}

TEST(CompatOption, FromOptionalCopy) {
  {
    constexpr auto o = std::optional<int>(2);
    constexpr auto s = sus::Option<int>::from(o);
    EXPECT_EQ(o.value(), s.as_value());
  }
  {
    constexpr auto o = std::optional<int>();
    constexpr auto s = sus::Option<int>::from(o);
    EXPECT_EQ(s.is_some(), false);
  }

  {
    constexpr auto o = std::optional<int>(2);
    constexpr sus::Option<int> s = sus::into(o);
    EXPECT_EQ(o.value(), s.as_value());
  }
  {
    constexpr auto o = std::optional<int>();
    constexpr sus::Option<int> s = sus::into(o);
    EXPECT_EQ(s.is_some(), false);
  }
}

TEST(CompatOption, FromOptionalMove) {
  {
    constexpr auto s = sus::Option<int>::from(std::optional<int>(2));
    EXPECT_EQ(s.as_value(), 2);
  }
  {
    constexpr auto s = sus::Option<int>::from(std::optional<int>());
    EXPECT_EQ(s.is_none(), true);
  }

  {
    constexpr sus::Option<int> s = sus::into(std::optional<int>(2));
    EXPECT_EQ(s.as_value(), 2);
  }
  {
    constexpr sus::Option<int> s = sus::into(std::optional<int>());
    EXPECT_EQ(s.is_none(), true);
  }

  {
    constexpr sus::Option<Moved> sm =
        sus::into(std::optional<Moved>(std::in_place));
    EXPECT_EQ(sm.as_value().moved, 1);
  }
}

TEST(CompatOption, ToOptionalCopy) {
  // Explicit.
  {
    constexpr auto s = sus::Option<int>(2);
    constexpr auto o = std::optional<int>(s);
    EXPECT_EQ(o.value(), s.as_value());
  }
  {
    sus_gcc_bug_110245_else(constexpr) auto s = sus::Option<int>();
    sus_gcc_bug_110245_else(constexpr) auto o = std::optional<int>(s);
    EXPECT_EQ(o.has_value(), false);
  }
  // Implicit.
  {
    constexpr auto s = sus::Option<int>(2);
    constexpr std::optional<int> o = s;
    EXPECT_EQ(o.value(), s.as_value());
  }
  {
    sus_gcc_bug_110245_else(constexpr) auto s = sus::Option<int>();
    sus_gcc_bug_110245_else(constexpr) std::optional<int> o = s;
    EXPECT_EQ(o.has_value(), false);
  }

  // TODO: Support .into() back to std? Need a different (ADL? .into<T>()?)
  // extension point.
}

TEST(CompatOption, ToOptionalMove) {
  // Explicit.
  {
    constexpr auto o = std::optional<int>(sus::Option<int>(2));
    EXPECT_EQ(o.value(), 2);
  }
  {
    constexpr auto o = std::optional<int>(sus::Option<int>());
    EXPECT_EQ(o.has_value(), false);
  }
  // Implicit.
  {
    constexpr std::optional<int> o = sus::Option<int>(2);
    EXPECT_EQ(o.value(), 2);
  }
  {
    constexpr std::optional<int> o = sus::Option<int>();
    EXPECT_EQ(o.has_value(), false);
  }

  // TODO: Support .into() back to std? Need a different (ADL? .into<T>()?)
  // extension point.

  {
    auto s = sus::Option<Moved>(Moved());
    EXPECT_EQ(s.as_value().moved, 1);
    auto o = std::optional<Moved>(sus::move(s));
    EXPECT_GT(o.value().moved, 1);
  }
}

TEST(CompatOption, Try) {
  static_assert(sus::ops::Try<std::optional<i32>>);
  static_assert(sus::ops::TryDefault<std::optional<i32>>);
  static_assert(sus::ops::try_is_success(std::optional<i32>(std::in_place, 1)));
  static_assert(!sus::ops::try_is_success(std::optional<i32>()));
  static_assert(sus::ops::try_from_default<std::optional<i32>>() ==
                std::optional<i32>(0_i32));
}

}  // namespace
