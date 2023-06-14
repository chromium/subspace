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

#include "subspace/option/option_compat.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/construct/into.h"
#include "subspace/prelude.h"

namespace {

struct Moved {
  constexpr Moved() = default;
  constexpr Moved(Moved&& rhs) { moved = rhs.moved + 1; }
  constexpr void operator=(Moved&& rhs) { moved = rhs.moved + 1; }
  i32 moved = 0;
};

TEST(OptionCompat, CtorOptionalCopy) {
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

TEST(OptionCompat, CtorOptionalMove) {
  // Explicit.
  {
    auto s = sus::Option<int>(std::optional<int>(2));
    EXPECT_EQ(s.as_value(), 2);
  }
  {
    auto s = sus::Option<int>(std::optional<int>());
    EXPECT_EQ(s.is_some(), false);
  }
  // Implicit.
  {
    sus::Option<int> s = std::optional<int>(2);
    EXPECT_EQ(s.as_value(), 2);
  }
  {
    sus::Option<int> s = std::optional<int>();
    EXPECT_EQ(s.is_some(), false);
  }

  {
    auto sm = sus::Option<Moved>(std::optional<Moved>(std::in_place));
    EXPECT_GE(sm.as_value().moved, 1);
  }
}

TEST(OptionCompat, FromOptionalCopy) {
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

TEST(OptionCompat, FromOptionalMove) {
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

TEST(OptionCompat, ToOptionalCopy) {
  // Explicit.
  {
    constexpr auto s = sus::Option<int>::with(2);
    constexpr auto o = std::optional<int>(s);
    EXPECT_EQ(o.value(), s.as_value());
  }
  {
    constexpr auto s = sus::Option<int>();
    constexpr auto o = std::optional<int>(s);
    EXPECT_EQ(o.has_value(), false);
  }
  // Implicit.
  {
    constexpr auto s = sus::Option<int>::with(2);
    constexpr std::optional<int> o = s;
    EXPECT_EQ(o.value(), s.as_value());
  }
  {
    constexpr auto s = sus::Option<int>();
    constexpr std::optional<int> o = s;
    EXPECT_EQ(o.has_value(), false);
  }

  // TODO: Support .into() back to std? Need a different (ADL? .into<T>()?)
  // extension point.
}

TEST(OptionCompat, ToOptionalMove) {
  // Explicit.
  {
    constexpr auto o = std::optional<int>(sus::Option<int>::with(2));
    EXPECT_EQ(o.value(), 2);
  }
  {
    constexpr auto o = std::optional<int>(sus::Option<int>());
    EXPECT_EQ(o.has_value(), false);
  }
  // Implicit.
  {
    constexpr std::optional<int> o = sus::Option<int>::with(2);
    EXPECT_EQ(o.value(), 2);
  }
  {
    constexpr std::optional<int> o = sus::Option<int>();
    EXPECT_EQ(o.has_value(), false);
  }

  // TODO: Support .into() back to std? Need a different (ADL? .into<T>()?)
  // extension point.

  {
    auto s = sus::Option<Moved>::with(Moved());
    EXPECT_EQ(s.as_value().moved, 1);
    auto o = std::optional<Moved>(sus::move(s));
    EXPECT_GT(o.value().moved, 1);
  }
}

TEST(OptionCompat, ToOptionalRef) {
  // Explicit.
  {
    int i = 2;
    auto s = sus::Option<int&>::with(i);
    auto o = std::optional<const int*>(s);
    EXPECT_EQ(o.value(), &i);
  }
  {
    int i = 2;
    auto s = sus::Option<int&>::with(i);
    auto o = std::optional<int*>(s);
    EXPECT_EQ(o.value(), &i);
  }
  {
    int i = 2;
    auto s = sus::Option<int&>::with(i);
    auto o = std::optional<const int*>(s);
    EXPECT_EQ(o.value(), &i);
  }
  {
    constexpr auto s = sus::Option<int&>();
    constexpr auto o = std::optional<const int*>(s);
    EXPECT_EQ(o.has_value(), false);
  }
  // Implicit.
  {
    int i = 2;
    auto s = sus::Option<int&>::with(i);
    std::optional<const int*> o = s;
    EXPECT_EQ(o.value(), &i);
  }
  {
    int i = 2;
    auto s = sus::Option<int&>::with(i);
    std::optional<int*> o = s;
    EXPECT_EQ(o.value(), &i);
  }
  {
    int i = 2;
    auto s = sus::Option<int&>::with(i);
    std::optional<const int*> o = s;
    EXPECT_EQ(o.value(), &i);
  }
  {
    constexpr auto s = sus::Option<int&>();
    constexpr std::optional<const int*> o = s;
    EXPECT_EQ(o.has_value(), false);
  }

  // TODO: Support .into() back to std? Need a different (ADL? .into<T>()?)
  // extension point.
}

TEST(OptionCompat, FromOptionalCopyWithConversion) {
  static_assert(sus::construct::Into<i64, i32>);

  // Move.
  sus::Option<i32> o = sus::into(std::optional<i64>(101));
  EXPECT_EQ(o.as_value(), 101_i32);

  // Copy.
  auto f = std::optional<i64>(101);
  sus::Option<i32> t = sus::into(f);
  EXPECT_EQ(t.as_value(), 101_i32);
}

}  // namespace
