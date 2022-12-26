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
#include "cir/lib/syntax/let.h"
#include "cir/lib/syntax/type_reference.h"
#include "subspace/assertions/unreachable.h"

namespace cir {

void visit_decl(VisitCtx& ctx, const clang::Decl& decl,
                Output& output) noexcept {
  if (auto* fn_decl = clang::dyn_cast<clang::FunctionDecl>(&decl); fn_decl) {
    // TODO: visit everything inside.
    auto f = syntax::Function{
        .id = ctx.make_function_id(),
        .name = fn_decl->getNameAsString(),
        .span = SourceSpan::from_decl(*fn_decl),
    };
    output.functions.push(sus::move(f));
    return;
  }
  if (auto* rec_decl = clang::dyn_cast<clang::CXXRecordDecl>(&decl); rec_decl) {
    return;
  }
  if (auto* enum_decl = clang::dyn_cast<clang::EnumDecl>(&decl); enum_decl) {
    return;
  }
  if (auto* class_tmpl_decl = clang::dyn_cast<clang::ClassTemplateDecl>(&decl);
      class_tmpl_decl) {
    return;
  }
  if (auto* fn_tmpl_decl = clang::dyn_cast<clang::FunctionTemplateDecl>(&decl);
      fn_tmpl_decl) {
    return;
  }

  llvm::errs() << "Unknown top level Decl:\n";
  decl.dumpColor();
  sus::unreachable();
}

}  // namespace cir
