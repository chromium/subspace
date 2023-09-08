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

TEST_F(SubDocTest, AliasUsingStruct) {
  auto result = run_code(R"(
    namespace a {
    /// Comment headline
    struct S {};
    }
    namespace b {
    /// Alias comment headline
    using a::S;
    }
    /// Global comment headline
    using a::S;
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_alias_comment(db, "7:5", "<p>Alias comment headline</p>"));
  EXPECT_TRUE(has_alias_comment(db, "10:5", "<p>Global comment headline</p>"));
}

TEST_F(SubDocTest, AliasUsingMethod) {
  auto result = run_code(R"(
    namespace a {
    struct S {
      /// Comment headline
      void M();
    };
    }
    namespace b {
    struct S2 : private a::S {
      /// Alias comment headline
      using S::M;
    };
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_alias_comment(db, "10:7", "<p>Alias comment headline</p>"));
}
