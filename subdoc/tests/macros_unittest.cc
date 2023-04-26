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

// Clang 17 has a bugfix that allows reading doc comments from inside a macro,
// and these tests fail before Clang 17. But Clang 17 is also broken due to
// https://github.com/llvm/llvm-project/issues/62272, so our CI needs to use
// prebuilt Clang 16 libraries.
#if CLANG_VERSION_MAJOR >= 17

TEST_F(SubDocTest, MacroFunction) {
  auto result = run_code(R"(
    #define M() \
      /** Comment headline */ \
      void f() {}
    
    M()
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "3:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, MacroClass) {
  auto result = run_code(R"(
    #define M() \
      /** Comment headline */ \
      struct S {};
    
    M()
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "3:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, MacroField) {
  auto result = run_code(R"(
    #define M() \
      struct S { \
        /** Comment headline */ \
        int field; \
      };
    
    M()
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "4:9", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, MacroNamesFunction) {
  auto result = run_code(R"(
    #define M(name) \
      /** Comment headline */ \
      void name() {}
    
    M(f)
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_function_comment(db, "3:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, MacroNamesClass) {
  auto result = run_code(R"(
    #define M(name) \
      /** Comment headline */ \
      struct name {};
    
    M(S)
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "3:7", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, MacroNamesField) {
  auto result = run_code(R"(
    #define M(name) \
      struct S { \
        /** Comment headline */ \
        int name; \
      };
    
    M(field)
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_field_comment(db, "4:9", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, MacroModName) {
  auto result = run_code(R"(
    #define MOD_NAME(name) MOD_NAME_##name
    
    /// Comment headline
    struct MOD_NAME(S) {};
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(has_record_comment(db, "4:5", "<p>Comment headline</p>"));
}

TEST_F(SubDocTest, MacroMultilineComment) {
  auto result = run_code(R"(
    #define M() \
      /** Comment headline \
       * Second line */ \
      void f() {}
    
    M()
  )");
  ASSERT_TRUE(result.is_ok());
  subdoc::Database db = sus::move(result).unwrap();
  EXPECT_TRUE(
      has_function_comment(db, "3:7", "<p>Comment headline Second line</p>"));
}

#endif
