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

#include "subdoc/lib/type.h"
#include "subdoc/tests/subdoc_test.h"

namespace {

sus::Option<clang::FunctionDecl&> find_function(
    clang::ASTContext& cx, std::string_view name) noexcept {
  clang::TranslationUnitDecl* tdecl = cx.getTranslationUnitDecl();
  for (clang::Decl* decl : tdecl->decls()) {
    if (auto* fdecl = clang::dyn_cast<clang::FunctionDecl>(decl)) {
      if (std::string_view(fdecl->getName()) == name) return sus::some(*fdecl);
    }
  }
  return sus::none();
}

TEST_F(SubDocTest, TypePrimitive) {
  const char code[] = R"(
    int f();
  )";

  auto test = [](clang::ASTContext& cx) {
    sus::Option<clang::FunctionDecl&> fdecl = find_function(cx, "f");
    ASSERT_TRUE(fdecl.is_some());
    subdoc::Type t =
        subdoc::build_local_type(cx.getSourceManager(), fdecl->getReturnType());

    EXPECT_EQ(t.name, "int");
    EXPECT_EQ(t.refs, subdoc::Refs::None);
    ASSERT_EQ(t.quals.len(), 1u);
    EXPECT_EQ(t.quals[0u], subdoc::Qualifier::None);
    EXPECT_TRUE(t.array_dims.is_empty());
    EXPECT_TRUE(t.template_params.is_empty());
  };

  auto result = run_code_with_options(
      subdoc::RunOptions().set_on_tu_complete(test), code);
  ASSERT_TRUE(result.is_ok());
}

}  // namespace
