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

#include "subdoc/lib/names.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/prelude.h"

namespace subdoc {

struct DiagnosticIds {
  static DiagnosticIds with(clang::ASTContext& ast_cx) noexcept {
    return DiagnosticIds{
        .superceded_comment = ast_cx.getDiagnostics().getCustomDiagID(
            clang::DiagnosticsEngine::Error,
            "ignored API comment, superceded by comment at %0"),
    };
  }

  const unsigned int superceded_comment;
};

class Visitor : public clang::RecursiveASTVisitor<Visitor> {
 public:
  Visitor(VisitCx& cx, Database& docs_db, DiagnosticIds ids)
      : cx_(cx), docs_db_(docs_db), diag_ids_(sus::move(ids)) {}
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

    auto& ast_cx = decl->getASTContext();
    auto& diagnostics_engine = ast_cx.getDiagnostics();
    auto& src_manager = ast_cx.getSourceManager();
    auto loc_str = raw_comment->getBeginLoc().printToString(src_manager);
    auto comment_str = std::string(raw_comment->getRawText(src_manager));
    auto name = unique_name_for_function(*decl);
    auto comment = Comment(comment_str, loc_str);

    llvm::errs() << name << "\n";

    if (clang::isa<clang::CXXConstructorDecl>(decl)) {
      docs_db_.ctors.emplace("ctor", sus::move(comment));
    } else if (clang::isa<clang::CXXDestructorDecl>(decl)) {
      docs_db_.dtors.emplace("dtor", sus::move(comment));
    } else if (clang::isa<clang::CXXConversionDecl>(decl)) {
      docs_db_.conversions.emplace("cver", sus::move(comment));
    } else if (clang::isa<clang::CXXMethodDecl>(decl)) {
      docs_db_.methods.emplace("method", sus::move(comment));
    } else if (clang::isa<clang::CXXDeductionGuideDecl>(decl)) {
      docs_db_.deductions.emplace("deduction", sus::move(comment));
    } else {
      auto [old, added] = docs_db_.functions.emplace(name, sus::move(comment));
      if (!added) {
        diagnostics_engine
            .Report(raw_comment->getBeginLoc(), diag_ids_.superceded_comment)
            .AddString(old->second.begin_loc);
      }
    }

    return true;
  }

 private:
  clang::RawComment* get_raw_comment(clang::Decl& decl) const {
    return decl.getASTContext().getRawCommentForDeclNoCache(&decl);
  }

  VisitCx& cx_;
  Database& docs_db_;
  DiagnosticIds diag_ids_;
};

class AstConsumer : public clang::ASTConsumer {
 public:
  AstConsumer(VisitCx& cx, Database& docs_db) : cx_(cx), docs_db_(docs_db) {}

  bool HandleTopLevelDecl(clang::DeclGroupRef group_ref) noexcept override {
    for (clang::Decl* decl : group_ref) {
      if (!Visitor(cx_, docs_db_, DiagnosticIds::with(decl->getASTContext()))
               .TraverseDecl(decl))
        return false;
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
