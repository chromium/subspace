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

#include "subdoc/lib/visit.h"

#include "subspace/assertions/unreachable.h"
#include "subspace/prelude.h"

namespace subdoc {

class Visitor : public clang::RecursiveASTVisitor<Visitor> {
 public:
  Visitor(VisitCx& cx, Database& docs_db) : cx_(cx), docs_db_(docs_db) {}
  bool shouldVisitLambdaBody() const { return false; }

  bool VisitStaticAssertDecl(clang::StaticAssertDecl*) noexcept {
    llvm::errs() << "StaticAssertDecl\n";
    return true;
  }
  bool VisitNamespaceDecl(clang::NamespaceDecl* decl) noexcept {
    clang::RawComment* raw_comment = get_raw_comment(*decl);
    if (!raw_comment) return true;
    return true;
  }
  bool VisitRecordDecl(clang::RecordDecl* decl) noexcept {
    clang::RawComment* raw_comment = get_raw_comment(*decl);
    if (!raw_comment) return true;
    llvm::errs() << "RecordDecl " << raw_comment->getKind() << "\n";
    return true;
  }
  bool VisitEnumDecl(clang::EnumDecl* decl) noexcept {
    clang::RawComment* raw_comment = get_raw_comment(*decl);
    if (!raw_comment) return true;
    return true;
    llvm::errs() << "EnumDecl " << raw_comment->getKind() << "\n";
  }
  bool VisitTypedefDecl(clang::TypedefDecl* decl) noexcept {
    clang::RawComment* raw_comment = get_raw_comment(*decl);
    if (!raw_comment) return true;
    return true;
    llvm::errs() << "TypedefDecl " << raw_comment->getKind() << "\n";
  }
  bool VisitTypeAliasDecl(clang::TypeAliasDecl* decl) noexcept {
    clang::RawComment* raw_comment = get_raw_comment(*decl);
    if (!raw_comment) return true;
    return true;
    llvm::errs() << "TypeAliasDecl " << raw_comment->getKind() << "\n";
  }
  bool VisitFunctionDecl(clang::FunctionDecl* decl) noexcept {
    clang::RawComment* raw_comment = get_raw_comment(*decl);
    if (!raw_comment) return true;
    llvm::errs() << "FunctionDecl " << raw_comment->getKind() << "\n";
    return true;
  }

 private:
  clang::RawComment* get_raw_comment(clang::Decl& decl) const {
    return decl.getASTContext().getRawCommentForDeclNoCache(&decl);
  }

  VisitCx& cx_;
  Database& docs_db_;
};

class AstConsumer : public clang::ASTConsumer {
 public:
  AstConsumer(VisitCx& cx, Database& docs_db) : cx_(cx), docs_db_(docs_db) {}

  bool HandleTopLevelDecl(clang::DeclGroupRef group_ref) noexcept override {
    for (clang::Decl* decl : group_ref) {
      if (!Visitor(cx_, docs_db_).TraverseDecl(decl)) return false;
    }
    return true;
  }

 private:
  VisitCx& cx_;
  Database& docs_db_;
};

std::unique_ptr<clang::FrontendAction> VisitorFactory::create() noexcept {
  return std::make_unique<VisitorAction>(cx, docs_db);
}

std::unique_ptr<clang::ASTConsumer> VisitorAction::CreateASTConsumer(
    clang::CompilerInstance&, llvm::StringRef) noexcept {
  // set preprocessor options?
  return std::make_unique<AstConsumer>(cx, docs_db);
}

}  // namespace subdoc
