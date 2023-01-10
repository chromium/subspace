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

TEST_F(SubDocTest, Struct) {
  auto result = run_code(R"(
    /// Comment headline
    struct S {};
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "2:5", "/// Comment headline"));
}

TEST_F(SubDocTest, TemplateStruct) {
  auto result = run_code(R"(
    /// Comment headline
    template <class T>
    struct S {};
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "2:5", "/// Comment headline"));
}

TEST_F(SubDocTest, TemplateStructSpecialization) {
  auto result = run_code(R"(
    /// Comment headline 1
    template <class T>
    struct S {};
    /// Comment headline 2
    template <>
    struct S<void> {};
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "2:5", "/// Comment headline 1"));
  EXPECT_TRUE(has_record_comment(db, "5:5", "/// Comment headline 2"));
}

TEST_F(SubDocTest, StructInNamedNamespace) {
  auto result = run_code(R"(
    namespace n {
    /// Comment headline
    struct S {};
    }
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "3:5", "/// Comment headline"));
}

TEST_F(SubDocTest, StructInAnonymousNamespace) {
  auto result = run_code(R"(
    namespace {
    /// Comment headline
    struct S {};
    }
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(db.has_any_comments());
}

TEST_F(SubDocTest, StructInAnonymousAndNamedNamespace) {
  auto result = run_code(R"(
    namespace {
    namespace n {
    /// Comment headline
    struct S {};
    }
    }
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(db.has_any_comments());
}

TEST_F(SubDocTest, NestedStruct) {
  auto result = run_code(R"(
    /// Comment headline 1
    struct S {
      /// Comment headline 2
      struct R {};
    };
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "2:5", "/// Comment headline 1"));
  EXPECT_TRUE(has_record_comment(db, "4:7", "/// Comment headline 2"));
}

TEST_F(SubDocTest, PrivateStruct) {
  auto result = run_code(R"(
    struct S {
    private:
      /// Comment headline
      struct R {};
    };
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(db.has_any_comments());
}
