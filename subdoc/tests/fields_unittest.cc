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
