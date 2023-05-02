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

#include "subspace/fmt/formatter.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/containers/vec.h"
#include "subspace/prelude.h"

namespace {
using namespace sus::fmt;

struct Writer final : public Write {
  ::sus::fmt::Result write_str(::sus::Slice<unsigned char> data) final {
    out.extend(sus::move(data));
    return sus::ok(Void{});
  }

  sus::Vec<unsigned char> out;
};

TEST(Formatter, WriteSigned) {
  Writer writer;
  Formatter formatter(writer);

  {
    Result r = formatter.write_fmt(
        Arguments{.args = sus::Slice<Argument>::from({new_signed(0)})});
    auto ex = sus::Vec<char>::with_values('0');
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(writer.out, ex);
    writer.out.clear();
  }
  {
    Result r = formatter.write_fmt(
        Arguments{.args = sus::Slice<Argument>::from({new_signed(1234567)})});
    auto ex = sus::Vec<char>::with_values('1', '2', '3', '4', '5', '6', '7');
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(writer.out, ex);
    writer.out.clear();
  }
  {
    Result r = formatter.write_fmt(
        Arguments{.args = sus::Slice<Argument>::from({new_signed(-1264598)})});
    auto ex =
        sus::Vec<char>::with_values('-', '1', '2', '6', '4', '5', '9', '8');
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(writer.out, ex);
    writer.out.clear();
  }
}

TEST(Formatter, WriteUnsigned) {
  Writer writer;
  Formatter formatter(writer);

  {
    Result r = formatter.write_fmt(
        Arguments{.args = sus::Slice<Argument>::from({new_unsigned(1234567u)})});
    auto ex = sus::Vec<char>::with_values('1', '2', '3', '4', '5', '6', '7');
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(writer.out, ex);
    writer.out.clear();
  }
  {
    Result r = formatter.write_fmt(
        Arguments{.args = sus::Slice<Argument>::from({new_unsigned(71264598u)})});
    auto ex =
        sus::Vec<char>::with_values('7', '1', '2', '6', '4', '5', '9', '8');
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(writer.out, ex);
    writer.out.clear();
  }
}

}  // namespace
