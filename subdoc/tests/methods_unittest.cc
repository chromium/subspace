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
}
