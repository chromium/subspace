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
  auto db = run_code(R"(
    namespace n {
    /// Comment headline
    void f() {}
    }
    )");
  ASSERT_TRUE(db.is_ok());
  for (const auto& [k, v] : sus::move(db).unwrap().functions) {
    llvm::errs() << k << "\n";
    llvm::errs() << v.raw_text << "\n";
    llvm::errs() << v.begin_loc << "\n";
  }
}

TEST_F(SubDocTest, FunctionDefnDecl) {
  auto db = run_code(R"(
    namespace n {
    /// Comment headline 1
    void f();  // Forward decl.
    /// Comment headline 2
    void f(int) {}  // Defn.
    }
    )");
  ASSERT_TRUE(db.is_err());
  auto results = sus::move(db).unwrap_err();
  ASSERT_EQ(results.locations.len(), 1u);
  // The 2nd comment on the same function causes an error as the comments become
  // ambiguous.
  EXPECT_EQ(results.locations[0u], "test.cc:5:5");
}

TEST_F(SubDocTest, FunctionOverloads) {
  auto db = run_code(R"(
    namespace n {
    /// Comment headline 1
    void f(char) {}
    /// Comment headline 2
    void f(int) {}
    }
    )");
  ASSERT_TRUE(db.is_ok());
}

TEST_F(SubDocTest, FunctionOverloadsRequires) {
  auto db = run_code(R"(
    namespace n {
    /// Comment headline 1
    template <class T>
    void f(T) requires(std::same_as<T, char>) {}
    /// Comment headline 2
    template <class T>
    void f(T) requires(std::same_as<T, int>) {}
    }
    )");
  ASSERT_TRUE(db.is_ok());
}
