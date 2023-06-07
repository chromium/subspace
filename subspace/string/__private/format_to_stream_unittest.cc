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

#include "subspace/string/__private/format_to_stream.h"

#include <sstream>
#include <string>

#include "fmt/core.h"
#include "googletest/include/gtest/gtest.h"
#include "subspace/prelude.h"

namespace {
struct Streamable {};
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

sus__format_to_stream(, Streamable);

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

}  // namespace
