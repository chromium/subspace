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

TEST_F(SubDocTest, Class) {
  auto db = run_code(R"(
    /// Comment headline
    struct S {};
    )");
  EXPECT_TRUE(db.is_some());
}

TEST_F(SubDocTest, TemplateClass) {
  auto db = run_code(R"(
    /// Comment headline
    template <class T>
    struct S {};
    )");
  EXPECT_TRUE(db.is_some());
}

TEST_F(SubDocTest, TemplateClassSpecialization) {
  auto db = run_code(R"(
    /// Comment headline 1
    template <class T>
    struct S {};
    /// Comment headline 2
    template <>
    struct S<void> {};
    )");
  EXPECT_TRUE(db.is_some());
}
