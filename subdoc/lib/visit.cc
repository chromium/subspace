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

#include "subdoc/lib/database.h"
#include "subdoc/lib/path.h"
#include "subdoc/lib/unique_symbol.h"
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

static bool should_skip_decl(clang::Decl* decl) {
  auto* ndecl = clang::dyn_cast<clang::NamedDecl>(decl);
  if (!ndecl) return true;

  // TODO: These could be configurable. As well as user-defined namespaces to
  // skip.
  if (path_contains_namespace(ndecl,
                              sus::choice<Namespace::Tag::Anonymous>())) {
    return true;
  }
  if (path_contains_namespace(
          ndecl, sus::choice<Namespace::Tag::Named>("__private"))) {
    return true;
  }
  if (path_is_private(ndecl)) {
    return true;
  }
  return false;
}

static clang::RawComment* get_raw_comment(clang::Decl* decl) {
  return decl->getASTContext().getRawCommentForDeclNoCache(decl);
}

static Comment to_db_comment(clang::Decl* decl,
                             clang::RawComment& raw) noexcept {
  auto& ast_cx = decl->getASTContext();
  auto& src_manager = ast_cx.getSourceManager();
  return Comment(std::string(raw.getRawText(src_manager)),
                 raw.getBeginLoc().printToString(src_manager));
}

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
    if (should_skip_decl(decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);
    if (!raw_comment) return true;
    return true;
  }

  bool VisitRecordDecl(clang::RecordDecl* decl) noexcept {
    if (should_skip_decl(decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);
    if (!raw_comment) return true;
    if (decl->isUnion()) {
      auto ue = UnionElement(collect_namespace_path(decl),
                             to_db_comment(decl, *raw_comment),
                             decl->getQualifiedNameAsString());
      return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(ue),
                               mref(docs_db_.unions));
    } else {
      auto ce = ClassElement(collect_namespace_path(decl),
                             to_db_comment(decl, *raw_comment),
                             decl->getQualifiedNameAsString());
      return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(ce),
                               mref(docs_db_.classes));
    }
    return true;
  }

  bool VisitFieldDecl(clang::FieldDecl* decl) noexcept {
    if (should_skip_decl(decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);
    if (!raw_comment) return true;

    auto fe = FieldElement(collect_namespace_path(decl),
                           to_db_comment(decl, *raw_comment),
                           decl->getQualifiedNameAsString(),
                           collect_record_path(decl->getParent()),
                           // Static data members are found in VisitVarDecl.
                           FieldElement::NonStatic);
    return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(fe),
                             mref(docs_db_.fields));
  }

  bool VisitVarDecl(clang::VarDecl* decl) noexcept {
    // Static data members are represented as VarDecl, not FieldDecl. So we
    // visit VarDecls but only for them.
    if (!decl->isStaticDataMember()) return true;

    if (should_skip_decl(decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);
    if (!raw_comment) return true;

    auto* parent = clang::cast<clang::RecordDecl>(decl->getDeclContext());
    auto fe = FieldElement(
        collect_namespace_path(decl), to_db_comment(decl, *raw_comment),
        decl->getQualifiedNameAsString(), collect_record_path(parent),
        // NonStatic data members are found in VisitFieldDecl.
        FieldElement::Static);
    return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(fe),
                             mref(docs_db_.fields));
  }

  bool VisitEnumDecl(clang::EnumDecl* decl) noexcept {
    if (should_skip_decl(decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);
    if (!raw_comment) return true;
    return true;
    llvm::errs() << "EnumDecl " << raw_comment->getKind() << "\n";
  }

  bool VisitTypedefDecl(clang::TypedefDecl* decl) noexcept {
    if (should_skip_decl(decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);
    if (!raw_comment) return true;
    return true;
    llvm::errs() << "TypedefDecl " << raw_comment->getKind() << "\n";
  }

  bool VisitTypeAliasDecl(clang::TypeAliasDecl* decl) noexcept {
    if (should_skip_decl(decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);
    if (!raw_comment) return true;
    return true;
    llvm::errs() << "TypeAliasDecl " << raw_comment->getKind() << "\n";
  }

  bool VisitFunctionDecl(clang::FunctionDecl* decl) noexcept {
    if (should_skip_decl(decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);
    if (!raw_comment) return true;

    // TODO: The signature string should include the whole signature not just
    // the name.
    auto signature = decl->getQualifiedNameAsString();
    auto fe = FunctionElement(
        collect_namespace_path(decl), to_db_comment(decl, *raw_comment),
        decl->getQualifiedNameAsString(), sus::move(signature));

    if (clang::isa<clang::CXXConstructorDecl>(decl)) {
      return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(fe),
                               mref(docs_db_.ctors));
    } else if (clang::isa<clang::CXXDestructorDecl>(decl)) {
      return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(fe),

                               mref(docs_db_.dtors));
    } else if (clang::isa<clang::CXXConversionDecl>(decl)) {
      return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(fe),

                               mref(docs_db_.conversions));
    } else if (clang::isa<clang::CXXMethodDecl>(decl)) {
      return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(fe),

                               mref(docs_db_.methods));
    } else if (clang::isa<clang::CXXDeductionGuideDecl>(decl)) {
      return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(fe),

                               mref(docs_db_.deductions));
    } else {
      return add_comment_to_db(decl, raw_comment->getBeginLoc(), sus::move(fe),

                               mref(docs_db_.functions));
    }

    return true;
  }

 private:
  template <class ElementT, class MapT>
    requires ::sus::convert::SameOrSubclassOf<ElementT*, CommentElement*>
  bool add_comment_to_db(clang::Decl* decl, clang::SourceLocation loc,
                         ElementT comment_element, MapT& db_map) noexcept {
    auto& ast_cx = decl->getASTContext();

    auto [old, added] =
        db_map.emplace(unique_from_decl(decl), sus::move(comment_element));
    if (!added) {
      auto& diagnostics_engine = ast_cx.getDiagnostics();
      const ElementT& old_element = old->second;
      diagnostics_engine.Report(loc, diag_ids_.superceded_comment)
          .AddString(old_element.comment.begin_loc);
    }
    return true;
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
