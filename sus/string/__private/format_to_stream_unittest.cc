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

#include "sus/string/__private/format_to_stream.h"

#include <sstream>
#include <string>

#include "fmt/core.h"
#include "googletest/include/gtest/gtest.h"
#include "sus/choice/choice.h"
#include "sus/collections/array.h"
#include "sus/prelude.h"
#include "sus/tuple/tuple.h"

namespace {
struct Streamable {
  _sus_format_to_stream(Streamable);
};
}  // namespace

template <>
struct fmt::formatter<Streamable, char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(const Streamable&, FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "hello");
  }
};

namespace {

TEST(FormatToStream, ToStringStream) {
  std::stringstream s;
  s << Streamable();
  EXPECT_EQ(s.str(), "hello");
}

struct StreamWithMethod {
  StreamWithMethod& operator<<(std::string) & {
    called = true;
    return *this;
  }
  bool called = false;
};

TEST(FormatToStream, ToStreamWithMethod) {
  StreamWithMethod s;
  EXPECT_EQ(s.called, false);
  s << Streamable();
  EXPECT_EQ(s.called, true);
}

struct StreamWithADL {
  friend StreamWithADL& operator<<(StreamWithADL& self, std::string) {
    self.called = true;
    return self;
  }
  bool called = false;
};

TEST(FormatToStream, ToStreamWithADL) {
  StreamWithADL s;
  EXPECT_EQ(s.called, false);
  s << Streamable();
  EXPECT_EQ(s.called, true);
}

// Array has a manually written stream impl, instead of using the macro.
TEST(FormatToStream, Array) {
  StreamWithADL s;
  EXPECT_EQ(s.called, false);
  s << sus::Array<i32, 3>(1, 2, 3);
  EXPECT_EQ(s.called, true);
}

// Choice has a manually written stream impl, instead of using the macro.
TEST(FormatToStream, Choice) {
  StreamWithADL s;
  EXPECT_EQ(s.called, false);
  s << sus::Choice<sus_choice_types((1_i32, i32))>::with<1>(1);
  EXPECT_EQ(s.called, true);
}

// Tuple has a manually written stream impl, instead of using the macro.
TEST(FormatToStream, Tuple) {
  StreamWithADL s;
  EXPECT_EQ(s.called, false);
  s << sus::Tuple<i32>(1);
  EXPECT_EQ(s.called, true);
}

}  // namespace
