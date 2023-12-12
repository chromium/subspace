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

TEST_F(SubDocTest, Field) {
  auto result = run_code(R"(
    struct S {
      /// Comment headline
      int f = 1;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "3:7", "<p>Comment headline</p>"));
  auto& e = db.find_field_comment("3:7").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "4");
}

TEST_F(SubDocTest, StaticField) {
  auto result = run_code(R"(
    struct S {
      /// Comment headline
      constexpr static int f = 1;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "3:7", "<p>Comment headline</p>"));
  auto& e = db.find_field_comment("3:7").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "4");
}

TEST_F(SubDocTest, StaticFieldSplit) {
  auto result = run_code(R"(
    struct S {
      /// Comment headline
      static int f;
    };

    int S::f = 1;
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "3:7", "<p>Comment headline</p>"));
  auto& e = db.find_field_comment("3:7").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "7");
}

TEST_F(SubDocTest, PrivateField) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      int f = 1;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(db.has_any_comments());
}

TEST_F(SubDocTest, PrivateStaticField) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      constexpr static int f = 1;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(db.has_any_comments());
}

TEST_F(SubDocTest, NestedField) {
  auto result = run_code(R"(
    struct Outer { struct S {
      /// Comment headline
      int f = 1;
    }; };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "3:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, FieldInNamespaces) {
  auto result = run_code(R"(
    namespace a::b::c {
    struct S {
      /// Comment headline
      int f = 1;
    };
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "4:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, Variables) {
  auto result = run_code(R"(
    /// Comment headline 1
    int i;
    namespace n {
      /// Comment headline 2
      int j;
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  ASSERT_TRUE(has_variable_comment(db, "2:5", "<p>Comment headline 1</p>"));
  ASSERT_TRUE(has_variable_comment(db, "5:7", "<p>Comment headline 2</p>"));
  {
    auto& e = db.find_variable_comment("2:5").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "3");
  }
  {
    auto& e = db.find_variable_comment("5:7").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "6");
  }
}
