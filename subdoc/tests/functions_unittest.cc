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

TEST_F(SubDocTest, Function) {
  auto result = run_code(R"(
    /// Comment headline
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "2:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, FunctionOverloads) {
  auto result = run_code(R"(
    /// Comment headline 1
    void f(char) {}
    void f(int) {}

    void g(char) {}
    /// Comment headline 2
    void g(int) {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "2:5", "<p>Comment headline 1</p>"));
  EXPECT_TRUE(has_function_comment(db, "7:5", "<p>Comment headline 2</p>"));
}

TEST_F(SubDocTest, FunctionOverloadsNoMerge) {
  auto result = run_code(R"(
    /// Comment headline 1
    /// #[doc.overloads=1]
    ///
    /// Body 1
    void f(char) {}
    /// Comment headline 2
    /// #[doc.overloads=2]
    ///
    /// Body 2
    void f(int) {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "2:5",
                                   "<p>Comment headline 1</p>\n<p>Body 1</p>"));
  EXPECT_TRUE(has_function_comment(db, "7:5",
                                   "<p>Comment headline 2</p>\n<p>Body 2</p>"));
}

TEST_F(SubDocTest, FunctionOverloadsMerge) {
  auto result = run_code(R"(
    /// Comment headline 1
    /// #[doc.overloads=1]
    void f(char) {}
    /// #[doc.overloads=1]
    void f(float) {}
    /// Comment headline 2
    /// #[doc.overloads=2]
    void f(int) {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "2:5", "<p>Comment headline 1</p>"));
  EXPECT_TRUE(has_function_comment(db, "7:5", "<p>Comment headline 2</p>"));
}

TEST_F(SubDocTest, FunctionOverloadsDuplicate) {
  auto result = run_code(R"(
    /// Comment headline 1
    void f(char) {}
    /// Comment headline 2
    void f(int) {}
  )");
  ASSERT_TRUE(result.is_err());
  auto diags = sus::move(result).unwrap_err();
  ASSERT_EQ(diags.locations.len(), 1u);
  // The 2nd comment on the same function causes an error we group overloads
  // under a single comment and having two is ambiguous.
  EXPECT_EQ(diags.locations[0u], "test.cc:4:5");
}

TEST_F(SubDocTest, FunctionOverloadsRequires) {
  auto result = run_code(R"(
    template <class A, class B>
    concept C = true;

    /// Comment headline 1
    template <class T>
    void f(T) requires(C<T, char>) {}
    template <class T>
    void f(T) requires(C<T, int>) {}

    template <class T>
    void g(T) requires(C<T, char>) {}
    /// Comment headline 1
    template <class T>
    void g(T) requires(C<T, int>) {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "5:5", "<p>Comment headline 1</p>"));
  EXPECT_TRUE(has_function_comment(db, "13:5", "<p>Comment headline 1</p>"));
}

TEST_F(SubDocTest, FunctionOverloadsRequiresDuplicate) {
  auto result = run_code(R"(
    template <class A, class B>
    concept C = true;

    /// Comment headline 1
    template <class T>
    void f(T) requires(C<T, char>) {}
    /// Comment headline 2
    template <class T>
    void f(T) requires(C<T, int>) {}
  )");
  ASSERT_TRUE(result.is_err());
  auto diags = sus::move(result).unwrap_err();
  // The 2nd comment on the same function causes an error we group overloads
  // under a single comment and having two is ambiguous.
  EXPECT_EQ(diags.locations[0u], "test.cc:8:5");
}

TEST_F(SubDocTest, ForwardDeclDuplicate) {
  auto result = run_code(R"(
    /// Comment headline 1
    void f();  // Forward decl.
    /// Comment headline 2
    void f() {}  // Defn.
  )");
  ASSERT_TRUE(result.is_err());
  auto diags = sus::move(result).unwrap_err();
  ASSERT_EQ(diags.locations.len(), 1u);
  // The 2nd comment on the same function causes an error as the comments become
  // ambiguous.
  EXPECT_EQ(diags.locations[0u], "test.cc:4:5");
}

TEST_F(SubDocTest, ForwardDeclDocumented) {
  auto result = run_code(R"(
    /// Comment headline
    void f();
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "2:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, ForwardDeclUndocumented) {
  auto result = run_code(R"(
    void f();
    /// Comment headline
    void f() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "3:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, FunctionInNamedNamespace) {
  auto result = run_code(R"(
    namespace n {
    /// Comment headline
    void f() {}
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "3:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, FunctionInAnonymousNamespace) {
  auto result = run_code(R"(
    namespace {
    /// Comment headline
    void f() {}
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(db.has_any_comments());
}

TEST_F(SubDocTest, FunctionInAnonymousAndNamedNamespace) {
  auto result = run_code(R"(
    namespace {
    namespace n {
    /// Comment headline
    void f() {}
    }
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_FALSE(db.has_any_comments());
}
