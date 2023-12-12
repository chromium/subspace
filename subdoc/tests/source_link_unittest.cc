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
#include "sus/prelude.h"

namespace {

TEST_F(SubDocTest, SourceLinkFilePath) {
#ifdef _MSC_VER
  const char PATH[] = "C:\\path\\to\\test.cc";
#else
  const char PATH[] = "/path/to/test.cc";
#endif
  auto result = run_code_with_options(
      subdoc::RunOptions().set_show_progress(false), PATH, R"(
    /// Comment headline 1
    int i;
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  ASSERT_TRUE(has_variable_comment(db, "2:5", "<p>Comment headline 1</p>"));
  auto& e = db.find_variable_comment("2:5").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
#ifdef _MSC_VER
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "C:/path/to/test.cc");
#else
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "/path/to/test.cc");
#endif
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "3");
}

TEST_F(SubDocTest, SourceLinkPrefixes) {
#ifdef _MSC_VER
  const char PATH[] = "C:\\path\\to\\test.cc";
  const char REMOVE[] = "C:\\path";
#else
  const char PATH[] = "/path/to/test.cc";
  const char REMOVE[] = "/path";
#endif

  auto options = subdoc::RunOptions()
                     .set_show_progress(false)
                     .set_remove_path_prefix(sus::some(REMOVE))
                     .set_add_path_prefix(sus::some("/things"))
                     .set_source_line_prefix(sus::some("L"));
  auto result = run_code_with_options(options, PATH, R"(
    /// Comment headline 1
    int i;
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  ASSERT_TRUE(has_variable_comment(db, "2:5", "<p>Comment headline 1</p>"));
  auto& e = db.find_variable_comment("2:5").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "/things/to/test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "L3");
}

}  // namespace
