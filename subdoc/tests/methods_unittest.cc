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

TEST_F(SubDocTest, Method) {
  auto result = run_code(R"(
    struct S {
      /// Comment headline
      void f() {}
    };
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "3:7", "Comment headline"));
}

TEST_F(SubDocTest, MethodOverload) {
  auto result = run_code(R"(
    struct S {
      /// Comment headline 1
      void f(char) {}
      /// Comment headline 2
      void f(int) {}
    };
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "3:7", "Comment headline 1"));
  EXPECT_TRUE(has_method_comment(db, "5:7", "Comment headline 2"));
}

TEST_F(SubDocTest, MethodOverloadRequires) {
  auto result = run_code(R"(
    template <class A>
    concept C = true;

    template <class T>
    struct S {
      /// Comment headline 1
      void f() requires(C<T>) {}
      /// Comment headline 2
      void f() requires(!C<T>) {}
    };
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "7:7", "Comment headline 1"));
  EXPECT_TRUE(has_method_comment(db, "9:7", "Comment headline 2"));
}

TEST_F(SubDocTest, MethodTemplateOverloadRequires) {
  auto result = run_code(R"(
    template <class A>
    concept C = true;

    struct S {
      /// Comment headline 1
      template <class D>
      void f() requires(C<D>) {}
      /// Comment headline 2
      template <class D>
      void f() requires(!C<D>) {}
    };
    )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "6:7", "Comment headline 1"));
  EXPECT_TRUE(has_method_comment(db, "9:7", "Comment headline 2"));
}
