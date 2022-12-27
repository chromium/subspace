// Copyright 2022 Google LLC
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

#include "cir/lib/visit.h"

#include "cir/lib/syntax/declared_type.h"
#include "cir/lib/syntax/function.h"
#include "cir/lib/syntax/statements/let.h"
#include "cir/lib/syntax/type_reference.h"
#include "subspace/assertions/unreachable.h"

using sus::Option;

namespace cir {

enum class FunctionType {
  Function,
  Method,
};

class Visitor : public clang::RecursiveASTVisitor<Visitor> {
 public:
  Visitor(VisitCtx& ctx, Output& output) : ctx_(ctx), output_(output) {}
  bool shouldVisitTemplateInstantiations() const { return true; }

  bool VisitFunctionDecl(clang::FunctionDecl* decl) {
    auto return_var = Option<syntax::Let>::none();
    if (!decl->getReturnType()->isVoidType()) {
      // TODO: If it's a pointer, the function may be nonnull-annotated.
      auto type_ref = syntax::TypeReference::with_return_type(
          decl->getReturnType(), /*nullable=*/true,
          SourceSpan::from_decl(*decl));

      return_var.insert(syntax::Let{
          .id = ctx_.make_local_var_id(),
          .type = sus::move(type_ref),
          .span = SourceSpan::from_decl(*decl),
          .clang_type =
              syntax::LetClangType::with<syntax::LetClangTypeTag::Return>(
                  decl->getReturnType()),
      });
    }

    auto this_param = Option<syntax::Let>::none();
    if (auto* method = clang::dyn_cast<clang::CXXMethodDecl>(decl)) {
      // TODO: `this` is the first parameter.
    }

    // TODO: The function's parameters and return type.

    auto id = ctx_.make_function_id();
    auto f = syntax::Function{
        .id = id,
        .name = decl->getNameAsString(),
        .span = SourceSpan::from_decl(*decl),
        .return_var = sus::move(return_var),
        .decl = *decl,
    };

    output_.functions.emplace(id, sus::move(f));

    // Recurse so we can tell when we leave the function, and store the
    // syntax::Function in Output when we're done.
    ctx_.in_functions.push(id);
    bool b = Visitor(ctx_, output_).TraverseStmt(decl->getBody());
    ctx_.in_functions.pop();
    return b;
  }

 private:
  VisitCtx& ctx_;
  Output& output_;
};

void visit_decl(VisitCtx& ctx, clang::Decl& decl, Output& output) noexcept {
  Visitor(ctx, output).TraverseDecl(&decl);
}

}  // namespace cir
