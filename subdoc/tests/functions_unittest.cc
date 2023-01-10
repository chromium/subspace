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
  EXPECT_TRUE(has_fn_comment(db, "2:5", "/// Comment headline"));
}

TEST_F(SubDocTest, FunctionOverloads) {
  auto result = run_code(R"(
    /// Comment headline 1
    void f(char) {}
    /// Comment headline 2
    void f(int) {}
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_fn_comment(db, "2:5", "/// Comment headline 1"));
  EXPECT_TRUE(has_fn_comment(db, "4:5", "/// Comment headline 2"));
}

TEST_F(SubDocTest, FunctionOverloadsRequires) {
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
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_fn_comment(db, "5:5", "/// Comment headline 1"));
  EXPECT_TRUE(has_fn_comment(db, "8:5", "/// Comment headline 2"));
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
  EXPECT_TRUE(has_fn_comment(db, "2:5", "/// Comment headline"));
}

TEST_F(SubDocTest, ForwardDeclUndocumented) {
  auto result = run_code(R"(
    void f();
    /// Comment headline
    void f() {}
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_fn_comment(db, "3:5", "/// Comment headline"));
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
  EXPECT_TRUE(has_fn_comment(db, "3:5", "/// Comment headline"));
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
