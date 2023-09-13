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
  EXPECT_TRUE(has_record_comment(db, "2:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, TemplateStruct) {
  auto result = run_code(R"(
    /// Comment headline
    template <class T>
    struct S {};
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "2:5", "<p>Comment headline</p>"));
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
  ASSERT_TRUE(result.is_err());
  auto diags = sus::move(result).unwrap_err();
  ASSERT_EQ(diags.locations.len(), 1u);
  // The 2nd comment on the same structure causes an error as it is ambiguous.
  EXPECT_EQ(diags.locations[0u], "test.cc:5:5");
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
  EXPECT_TRUE(has_record_comment(db, "3:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, StructInPrivateNamespace) {
  auto result = run_code(R"(
    namespace __private {
    struct S {
      /// Comment headline
      int i;
    };
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(db.has_any_comments());
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
  EXPECT_TRUE(has_record_comment(db, "2:5", "<p>Comment headline 1</p>"));
  EXPECT_TRUE(has_record_comment(db, "4:7", "<p>Comment headline 2</p>"));
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

TEST_F(SubDocTest, ReplaceDocSelf) {
  auto result = run_code(R"(
    /// Comment headline @doc.self 1.
    struct S {
      /// Comment headline @doc.self 2.
      S() {}
      /// Comment headline @doc.self 3.
      ~S() {}
      /// Comment headline @doc.self 4.
      void m() {}
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "2:5", "<p>Comment headline S 1.</p>"));
  EXPECT_TRUE(has_ctor_comment(db, "4:7", "<p>Comment headline S 2.</p>"));
  EXPECT_TRUE(has_dtor_comment(db, "6:7", "<p>Comment headline S 3.</p>"));
  EXPECT_TRUE(has_method_comment(db, "8:7", "<p>Comment headline S 4.</p>"));
}
