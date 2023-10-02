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

TEST_F(SubDocTest, AliasUsingConcept) {
  auto result = run_code(R"(
    namespace a {
    /// Comment headline
    template <class T> concept S = true;
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

TEST_F(SubDocTest, AliasUsingEnumInNamespace) {
  auto result = run_code(R"(
    namespace a { enum class E { First, Second }; }
    /// Alias comment headline
    using a::E;
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_alias_comment(db, "3:5", "<p>Alias comment headline</p>"));
}

TEST_F(SubDocTest, DISABLED_AliasUsingEnum) {
  auto result = run_code(R"(
    namespace a { enum class E { First, Second }; }
    namespace b {
    struct S {
      using enum a::E;
    };
    struct S2 {
      /// Alias comment headline
      using a::E::First;
    };
    }
    using enum a::E;
    /// Global comment headline
    using a::E::First;
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_alias_comment(db, "8:7", "<p>Alias comment headline</p>"));
  EXPECT_TRUE(has_alias_comment(db, "13:5", "<p>Alias comment headline</p>"));
}

TEST_F(SubDocTest, DISABLED_AliasCommentOnUsingEnum) {
  auto result = run_code(R"(
    namespace a { enum class E { First, Second }; }
    /// Invalid comment
    using enum a::E;
  )");
  ASSERT_TRUE(result.is_err());
  auto diags = sus::move(result).unwrap_err();
  ASSERT_EQ(diags.locations.len(), 1u);
  // A comment on `using enum` has nowhere to display, so it's an error.
  EXPECT_EQ(diags.locations[0u], "test.cc:3:5");
}

TEST_F(SubDocTest, AliasNamedTypeInNamespace) {
  auto result = run_code(R"(
    namespace a {
    /// Comment headline
    struct S {};
    }
    namespace b {
    /// Alias comment headline
    using T = a::S;
    }
    /// Global comment headline
    using T2 = a::S;
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_alias_comment(db, "7:5", "<p>Alias comment headline</p>"));
  EXPECT_TRUE(has_alias_comment(db, "10:5", "<p>Global comment headline</p>"));
}

TEST_F(SubDocTest, AliasNamedTypeInRecord) {
  auto result = run_code(R"(
    namespace a {
    /// Comment headline
    struct S {};
    }
    struct T {
      /// Alias comment headline
      using A = a::S;
    };
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_alias_comment(db, "7:7", "<p>Alias comment headline</p>"));
}

TEST_F(SubDocTest, AliasUsingVariable) {
  auto result = run_code(R"(
    namespace a {
    /// Comment headline
    int i;
    }
    /// Using comment
    using a::i;
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_alias_comment(db, "6:5", "<p>Using comment</p>"));
}
