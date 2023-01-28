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

#include "subdoc/tests/subdoc_test.h"

TEST_F(SubDocTest, IncludeRegexMissesTest) {
  const auto opts = subdoc::RunOptions()           //
                        .set_show_progress(false)  //
                        .set_include_path_patterns(std::regex("not_test.cc"));
  auto result = run_code_with_options(opts, R"(
    /// Comment headline
    struct S {};
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(!db.has_any_comments());
}

TEST_F(SubDocTest, ExcludeRegexHitsTest) {
  const auto opts = subdoc::RunOptions()           //
                        .set_show_progress(false)  //
                        .set_exclude_path_patterns(std::regex("test.cc"));
  auto result = run_code_with_options(opts, R"(
    /// Comment headline
    struct S {};
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(!db.has_any_comments());
}

TEST_F(SubDocTest, ExcludeRegexMissesTest) {
  const auto opts = subdoc::RunOptions()           //
                        .set_show_progress(false)  //
                        .set_exclude_path_patterns(std::regex("teOOPSst.cc"));
  auto result = run_code_with_options(opts, R"(
    /// Comment headline
    struct S {};
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "2:5", "<p>Comment headline</p>"));
}
