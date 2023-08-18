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

TEST_F(SubDocTest, SelfNameReplaceClass) {
  auto result = run_code(R"(
    /// Replace @doc.self all @doc.self here
    template <class T>
    struct TheName {};
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "2:5", "<p>Replace TheName all TheName here</p>"));
}

TEST_F(SubDocTest, SelfNameReplaceMethod) {
  auto result = run_code(R"(
    template <class T>
    struct TheName {
      /// Replace @doc.self all @doc.self here
      void f();
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "4:7", "<p>Replace TheName all TheName here</p>"));
}

TEST_F(SubDocTest, SelfNameReplaceStaticMethod) {
  auto result = run_code(R"(
    template <class T>
    struct TheName {
      /// Replace @doc.self all @doc.self here
      static void f();
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_method_comment(db, "4:7", "<p>Replace TheName all TheName here</p>"));
}

TEST_F(SubDocTest, SelfNameReplaceField) {
  auto result = run_code(R"(
    template <class T>
    struct TheName {
      /// Replace @doc.self all @doc.self here
      int f;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "4:7", "<p>Replace TheName all TheName here</p>"));
}

TEST_F(SubDocTest, SelfNameReplaceStaticDataMember) {
  auto result = run_code(R"(
    template <class T>
    struct TheName {
      /// Replace @doc.self all @doc.self here
      static int f;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "4:7", "<p>Replace TheName all TheName here</p>"));
}
