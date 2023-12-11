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

TEST_F(SubDocTest, ConceptInToplevel) {
  auto result = run_code(R"(
    /// Comment headline
    template <class T>
    concept S = true;
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_concept_comment(db, "2:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, ConceptInNamespace) {
  auto result = run_code(R"(
    namespace a {
    /// Comment headline
    template <class T>
    concept S = true;
    }
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_concept_comment(db, "3:5", "<p>Comment headline</p>"));
}

}  // namespace
