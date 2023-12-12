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

TEST_F(SubDocTest, NamespaceTopLevel) {
  auto result = run_code(R"(
    /// Comment headline
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "2:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, NamespaceSingle) {
  auto result = run_code(R"(
    namespace single {
    /// Comment headline
    void f() {}
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "3:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, NamespaceNested) {
  auto result = run_code(R"(
    namespace single {
    namespace nested {
    /// Comment headline
    void f() {}
    }
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "4:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, NamespaceNestedThenDots) {
  auto result = run_code(R"(
    namespace single {
    namespace nested {
    }
    }
    namespace single::nested {
    /// Comment headline
    void f() {}
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "7:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, NamespaceDots) {
  auto result = run_code(R"(
    namespace single::nested {
    /// Comment headline
    void f() {}
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "3:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, NamespaceTwoDots) {
  auto result = run_code(R"(
    namespace single::nested::more {
    /// Comment headline
    void f() {}
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "3:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, NamespaceComment) {
  auto result = run_code(R"(
    /// Comment headline
    namespace single {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_namespace_comment(db, "2:5", "<p>Comment headline</p>"));

  auto& e = db.find_namespace_comment("2:5").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "3");
}

TEST_F(SubDocTest, NestedNamespaceComment) {
  auto result = run_code(R"(
    namespace single {
    /// Comment headline
    namespace nested {}
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_namespace_comment(db, "3:5", "<p>Comment headline</p>"));

  auto& e = db.find_namespace_comment("3:5").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "4");
}

TEST_F(SubDocTest, NamespaceDotsComment) {
  auto result = run_code(R"(
    /// Comment headline
    namespace single::nested {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  // Unfortunately, as of Clang 17, Clang applies this comment to both `single`
  // and `nested` so it's not a useful way to write comments in practice.
  EXPECT_TRUE(has_namespace_comment(db, "2:5", "<p>Comment headline</p>"));
}
