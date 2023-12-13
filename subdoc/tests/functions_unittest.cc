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

  auto& e = db.find_function_comment("2:5").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "3");
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
  {
    auto& e = db.find_function_comment("2:5").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "3");
  }
  {
    auto& e = db.find_function_comment("7:5").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "8");
  }
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
  {
    auto& e = db.find_function_comment("2:5").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "6");
  }
  {
    auto& e = db.find_function_comment("7:5").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "11");
  }
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
  {
    auto& e = db.find_function_comment("2:5").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "4");
  }
  {
    auto& e = db.find_function_comment("7:5").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "9");
  }
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

  // For source links: The decl is preferred over the comment on a forward decl.
  auto& e = db.find_function_comment("2:5").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "4");
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

  // For source links: The commented decl is preferred above all.
  auto& e = db.find_function_comment("3:5").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "4");
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

  auto& e = db.find_function_comment("3:5").unwrap();
  ASSERT_TRUE(e.source_link.is_some());
  EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
  EXPECT_EQ(e.source_link.as_ref().unwrap().line, "4");
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

TEST_F(SubDocTest, FunctionFriend) {
  auto result = run_code(R"(
    struct S {
      /// Comment a headline
      friend void a() {}

      friend void b();
      /// Comment c headline
      friend void c();
    };
    
    void c() {}

    /// Comment b headline
    void b() {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "3:7", "<p>Comment a headline</p>"));
  EXPECT_TRUE(has_function_comment(db, "13:5", "<p>Comment b headline</p>"));
  // Friend decls are not visited if they aren't a definition. This prevents
  // them from showing up separately in the overload set.
  EXPECT_FALSE(has_function_comment(db, "7:7", "<p>Comment c headline</p>"));
  // Links go to the definitions.
  {
    auto& e = db.find_function_comment("3:7").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "4");
  }
  {
    auto& e = db.find_function_comment("13:5").unwrap();
    ASSERT_TRUE(e.source_link.is_some());
    EXPECT_EQ(e.source_link.as_ref().unwrap().file_path, "test.cc");
    EXPECT_EQ(e.source_link.as_ref().unwrap().line, "14");
  }
}

TEST_F(SubDocTest, FunctionRequiresOverload) {
  auto result = run_code(R"(
    template <class A, class B> concept C = true;

    /// Comment headline one
    /// #[doc.overloads=yes]
    template <class A, class B>
    void f() requires(C<A, B>) {}
    
    /// Comment headline two
    /// #[doc.overloads=no]
    template <class A, class B>
    void f() requires(!C<A, B>) {}
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "4:5", "<p>Comment headline one</p>"));
  // The second function has a different signature due to a different requires
  // clause, so does not collide with the first function (when doc.overloads
  // is specified).
  EXPECT_TRUE(has_function_comment(db, "9:5", "<p>Comment headline two</p>"));
}
