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

TEST_F(SubDocTest, MarkdownParagraph) {
  auto result = run_code(R"(
    /// Comment headline
    ///
    /// Next Paragraph
    /// Next Line
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(
      db, "2:5", "<p>Comment headline</p><p>Next Paragraph Next Line</p>"));
}

TEST_F(SubDocTest, MarkdownCodeBlock) {
  auto result = run_code(R"(
    /// Comment headline
    ///
    /// Before code
    /// ```
    /// Code 1
    /// Code 2
    /// ```
    /// After code
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "2:5",
                                   "<p>Comment headline</p>"
                                   "<p>Before code</p>"
                                   "<pre><code>Code 1\nCode 2\n</code></pre>"
                                   "<p>After code</p>"));
}

TEST_F(SubDocTest, MarkdownCodeSnippet) {
  auto result = run_code(R"(
    /// Comment headline `has snippet`
    ///
    /// This `snippet goes
    /// across lines` but works out.
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(
      db, "2:5",
      "<p>Comment headline <code>has snippet</code></p>"
      "<p>This <code>snippet goes across lines</code> but works out.</p>"));
}

TEST_F(SubDocTest, MarkdownUnmatchedCodeBlock) {
  auto result = run_code(R"(
    /// Comment headline `has snippet`
    ///
    /// ```
    /// This block never ends
    void f() {}
  )");
  ASSERT_TRUE(result.is_err());
  auto diags = sus::move(result).unwrap_err();
  ASSERT_EQ(diags.locations.len(), 1u);
  // The code snippet didn't end so it makes an error.
  EXPECT_EQ(diags.locations[0u], "test.cc:2:5");
}

TEST_F(SubDocTest, MarkdownUnmatchedCodeSnippet) {
  auto result = run_code(R"(
    /// Comment headline `has snippet`
    ///
    /// This `snippet` never `ends
    void f() {}
  )");
  ASSERT_TRUE(result.is_err());
  auto diags = sus::move(result).unwrap_err();
  ASSERT_EQ(diags.locations.len(), 1u);
  // The code snippet didn't end so it makes an error.
  EXPECT_EQ(diags.locations[0u], "test.cc:2:5");
}

TEST_F(SubDocTest, MarkdownMultiLineCodeSnippet) {
  auto result = run_code(R"(
    /// Comment headline
    ///
    /// This `snippet` will `end
    /// on the next line`.
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "2:5",
                                   "<p>Comment headline</p>"
                                   "<p>This <code>snippet</code> will "
                                   "<code>end on the next line</code>.</p>"));
}

TEST_F(SubDocTest, MarkdownHeaderMarkerInCodeBlock) {
  auto result = run_code(R"(
    /// Comment headline
    ///
    /// ```
    /// This is not a
    /// ## header.
    /// ```
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(
      db, "2:5",
      "<p>Comment headline</p>"
      "<p></p>"
      "<pre><code>This is not a\n## header.\n</code></pre>"));
}

TEST_F(SubDocTest, MarkdownHeaderMarkerInCodeSnippet) {
  auto result = run_code(R"(
    /// Comment headline
    ///
    /// This `is not a
    /// ## header`.
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(
      has_function_comment(db, "2:5",
                           "<p>Comment headline</p>"
                           "<p>This <code>is not a ## header</code>.</p>"));
}
