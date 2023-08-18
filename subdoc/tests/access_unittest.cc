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

namespace {

TEST_F(SubDocTest, AccessPublicField) {
  auto result = run_code(R"(
    struct S {
      /// Comment headline
      int f = 1;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "3:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessProtectedField) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      int f = 1;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(has_field_comment(db, "4:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessPrivateField) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      int f = 1;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(has_field_comment(db, "4:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessPublicStaticDataMember) {
  auto result = run_code(R"(
    struct S {
      /// Comment headline
      static int f;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "3:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessProtectedStaticDataMember) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      static int f;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(has_field_comment(db, "4:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessPrivateStaticDataMember) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      static int f;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(has_field_comment(db, "4:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessPublicMethod) {
  auto result = run_code(R"(
    struct S {
      /// Comment headline
      int f();
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "3:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessProtectedMethod) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      int f();
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(has_method_comment(db, "4:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessPrivateMethod) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      int f();
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(has_method_comment(db, "4:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessPublicStaticMethod) {
  auto result = run_code(R"(
    struct S {
      /// Comment headline
      static int f();
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "3:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessProtectedStaticMethod) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      static int f();
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(has_method_comment(db, "4:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessPrivateStaticMethod) {
  auto result = run_code(R"(
    struct S {
     private:
      /// Comment headline
      static int f();
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(has_method_comment(db, "4:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, AccessInheritedPrivateMethod_CommentOnInherited) {
  auto result = run_code(R"(
    struct BaseWithoutComment {
     private:
      virtual int f();
    };
    struct S : public BaseWithoutComment {
     public:
      /// Comment headline 1
      int f() override;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "8:7", "<p>Comment headline 1</p>"));
}

TEST_F(SubDocTest, AccessInheritedConflictingCommentOnOverride) {
  auto result = run_code(R"(
    struct BaseWithComment {
     private:
      /// Base comment headline
      virtual int f();
    };
    struct S : public BaseWithComment {
     public:
      /// Comment headline 2
      int f() override;
    };
  )");
  ASSERT_TRUE(result.is_err());
  auto diags = sus::move(result).unwrap_err();
  ASSERT_EQ(diags.locations.len(), 1u);
  // The method has a comment on the final and override, which conflict as they
  // are ambiguous.
  EXPECT_EQ(diags.locations[0u], "test.cc:4:7");
}

TEST_F(SubDocTest, AccessInheritedPrivateMethod_CommentOnBase) {
  auto result = run_code(R"(
    struct BaseWithComment {
     private:
      /// Base comment headline
      virtual int f();
    };
    struct S : public BaseWithComment {
     public:
      int f() override;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "4:7", "<p>Base comment headline</p>"));
}

TEST_F(SubDocTest, AccessInheritedProtectedMethod_CommentOnBase) {
  auto result = run_code(R"(
    struct BaseWithComment {
     protected:
      /// Base comment headline
      virtual int f();
    };
    struct S : public BaseWithComment {
     public:
      int f() override;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "4:7", "<p>Base comment headline</p>"));
}

}  // namespace
