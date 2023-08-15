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

#include <regex>

#include "subdoc/lib/database.h"
#include "subdoc/lib/doc_attributes.h"
#include "subdoc/lib/parse_comment.h"
#include "subdoc/lib/path.h"
#include "subdoc/lib/record_type.h"
#include "subdoc/lib/requires.h"
#include "subdoc/lib/unique_symbol.h"
#include "sus/assertions/check.h"
#include "sus/assertions/unreachable.h"
#include "sus/mem/swap.h"
#include "sus/ops/ord.h"
#include "sus/prelude.h"

namespace subdoc {

struct DiagnosticIds {
  static DiagnosticIds with_context(clang::ASTContext& ast_cx) noexcept {
    return DiagnosticIds{
        .superceded_comment = ast_cx.getDiagnostics().getCustomDiagID(
            clang::DiagnosticsEngine::Error,
            "ignored API comment, superceded by comment at %0"),
        .malformed_comment = ast_cx.getDiagnostics().getCustomDiagID(
            clang::DiagnosticsEngine::Error, "malformed API comment: %0"),
    };
  }

  const unsigned int superceded_comment;
  const unsigned int malformed_comment;
};

static bool should_skip_decl(VisitCx& cx, clang::Decl* decl) {
  auto* ndecl = clang::dyn_cast<clang::NamedDecl>(decl);
  if (!ndecl) return true;

  // TODO: These could be configurable. As well as user-defined namespaces to
  // skip.
  if (path_contains_namespace(ndecl,
                              sus::choice<Namespace::Tag::Anonymous>())) {
    return true;
  }
  // TODO: Make this configurable on the command line.
  if (path_contains_namespace(
          ndecl, sus::choice<Namespace::Tag::Named>("__private"))) {
    return true;
  }
  // TODO: Make this configurable on the command line.
  if (path_contains_namespace(ndecl,
                              sus::choice<Namespace::Tag::Named>("test"))) {
    return true;
  }
  if (path_is_private(ndecl)) {
    return true;
  }
  if (!cx.should_include_decl_based_on_file(decl)) {
    return true;
  }
  return false;
}

static clang::RawComment* get_raw_comment(clang::Decl* decl) {
  return decl->getASTContext().getRawCommentForDeclNoCache(decl);
}

class Visitor : public clang::RecursiveASTVisitor<Visitor> {
 public:
  Visitor(VisitCx& cx, Database& docs_db, DiagnosticIds ids)
      : cx_(cx), docs_db_(docs_db), diag_ids_(sus::move(ids)) {}
  bool shouldVisitLambdaBody() const { return false; }

  bool VisitStaticAssertDecl(clang::StaticAssertDecl*) noexcept {
    // llvm::errs() << "StaticAssertDecl\n";
    return true;
  }

  bool VisitNamespaceDecl(clang::NamespaceDecl* decl) noexcept {
    if (should_skip_decl(cx_, decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);

    Comment comment = make_db_comment(decl, raw_comment, "");
    auto ne =
        NamespaceElement(iter_namespace_path(decl).collect_vec(),
                         sus::move(comment), decl->getNameAsString(),
                         decl->getASTContext().getSourceManager().getFileOffset(
                             decl->getLocation()));
    NamespaceElement& parent = [&]() -> NamespaceElement& {
      clang::DeclContext* context = decl->getDeclContext();
      // TODO: Save the linkage spec (`extern "C"`) so we can show it.
      while (clang::isa<clang::LinkageSpecDecl>(context))
        context = context->getParent();

      if (clang::isa<clang::TranslationUnitDecl>(context)) {
        return docs_db_.find_namespace_mut(nullptr).unwrap();
      } else {
        return docs_db_
            .find_namespace_mut(clang::cast<clang::NamespaceDecl>(context))
            .unwrap();
      }
    }();
    add_namespace_to_db(decl, sus::move(ne), parent.namespaces);
    return true;
  }

  bool VisitRecordDecl(clang::RecordDecl* decl) noexcept {
    if (should_skip_decl(cx_, decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);

    if (auto* cxxdecl = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
      // It's a C++ class. So it may have a template decl which has the template
      // parameters. And it may be a specialization.
      // llvm::errs() << "Visiting CXX class with template, specialization
      // kind:"
      //              << cxxdecl->getTemplateSpecializationKind() << "\n";
    }

    RecordType type = [&]() {
      if (decl->isStruct()) return RecordType::Struct;
      if (decl->isUnion()) return RecordType::Union;
      return RecordType::Class;
    }();

    clang::RecordDecl* parent_record_decl =
        clang::dyn_cast<clang::RecordDecl>(decl->getDeclContext());

    Comment comment = make_db_comment(decl, raw_comment, decl->getName());
    auto re = RecordElement(
        iter_namespace_path(decl).collect_vec(), sus::move(comment),
        decl->getNameAsString(),
        iter_record_path(parent_record_decl)
            .map([](std::string_view&& v) { return std::string(v); })
            .collect_vec(),
        type,
        decl->getASTContext().getSourceManager().getFileOffset(
            decl->getLocation()));

    clang::DeclContext* context = decl->getDeclContext();
    // TODO: Save the linkage spec (`extern "C"`) so we can show it.
    while (clang::isa<clang::LinkageSpecDecl>(context))
      context = context->getParent();

    if (clang::isa<clang::TranslationUnitDecl>(context)) {
      NamespaceElement& parent = docs_db_.find_namespace_mut(nullptr).unwrap();
      add_record_to_db(decl, sus::move(re), parent.records);
    } else if (clang::isa<clang::NamespaceDecl>(context)) {
      auto* namespace_decl = clang::cast<clang::NamespaceDecl>(context);
      // Template specializations can be for classes that are part of a
      // namespace we never recorded as the files were excluded. e.g.
      // ```
      // template <>
      // struct fmt::formatter<MyType, char> {};
      // ```
      if (should_skip_decl(cx_, namespace_decl)) {
        // TODO: Should we generate docs for such things?
        return true;
      }
      NamespaceElement& parent =
          docs_db_.find_namespace_mut(namespace_decl)
              .expect(
                  "No parent namespace found in db for NamespaceDecl context");
      add_record_to_db(decl, sus::move(re), parent.records);
    } else {
      sus::check(clang::isa<clang::RecordDecl>(context));
      if (sus::Option<RecordElement&> parent =
              docs_db_.find_record_mut(clang::cast<clang::RecordDecl>(context));
          parent.is_some()) {
        add_record_to_db(decl, sus::move(re), parent->records);
      }
    }
    return true;
  }

  bool VisitFieldDecl(clang::FieldDecl* decl) noexcept {
    if (should_skip_decl(cx_, decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);

    clang::RecordDecl* record_decl =
        clang::cast<clang::RecordDecl>(decl->getDeclContext());

    Comment comment =
        make_db_comment(decl, raw_comment, record_decl->getName());
    auto fe = FieldElement(
        iter_namespace_path(decl).collect_vec(), sus::move(comment),
        std::string(decl->getName()), decl->getType(),
        iter_record_path(record_decl)
            .map([](std::string_view&& v) { return std::string(v); })
            .collect_vec(),
        // Static data members are found in VisitVarDecl.
        FieldElement::NonStatic,
        decl->getASTContext().getSourceManager().getFileOffset(
            decl->getLocation()));
    fe.type_element = docs_db_.find_type(decl->getType());

    if (sus::Option<RecordElement&> parent =
            docs_db_.find_record_mut(record_decl);
        parent.is_some()) {
      add_comment_to_db(decl, sus::move(fe), parent->fields);
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl* decl) noexcept {
    // Static data members are represented as VarDecl, not FieldDecl. So we
    // visit VarDecls but only for them.
    if (!decl->isStaticDataMember()) return true;

    if (should_skip_decl(cx_, decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);

    Comment comment = make_db_comment(decl, raw_comment, "");
    auto* record_decl = clang::cast<clang::RecordDecl>(decl->getDeclContext());
    auto fe = FieldElement(
        iter_namespace_path(decl).collect_vec(), sus::move(comment),
        std::string(decl->getName()), decl->getType(),
        iter_record_path(record_decl)
            .map([](std::string_view&& v) { return std::string(v); })
            .collect_vec(),
        // NonStatic data members are found in VisitFieldDecl.
        FieldElement::Static,
        decl->getASTContext().getSourceManager().getFileOffset(
            decl->getLocation()));

    if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
            clang::cast<clang::RecordDecl>(decl->getDeclContext()));
        parent.is_some()) {
      add_comment_to_db(decl, sus::move(fe), parent->fields);
    }
    return true;
  }

  bool VisitEnumDecl(clang::EnumDecl* decl) noexcept {
    if (should_skip_decl(cx_, decl)) return true;
    // clang::RawComment* raw_comment = get_raw_comment(decl);
    // if (raw_comment)
    //   llvm::errs() << "EnumDecl " << raw_comment->getKind() << "\n";
    return true;
  }

  bool VisitTypedefDecl(clang::TypedefDecl* decl) noexcept {
    if (should_skip_decl(cx_, decl)) return true;
    // clang::RawComment* raw_comment = get_raw_comment(decl);
    // if (raw_comment)
    //   llvm::errs() << "TypedefDecl " << raw_comment->getKind() << "\n";
    return true;
  }

  bool VisitTypeAliasDecl(clang::TypeAliasDecl* decl) noexcept {
    if (should_skip_decl(cx_, decl)) return true;
    // clang::RawComment* raw_comment = get_raw_comment(decl);
    // if (raw_comment)
    //   llvm::errs() << "TypeAliasDecl " << raw_comment->getKind() << "\n";
    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl* decl) noexcept {
    if (decl->isTemplateInstantiation()) return true;
    if (should_skip_decl(cx_, decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);

    // TODO: Save the linkage spec (`extern "C"`) so we can show it.
    clang::DeclContext* context = decl->getDeclContext();
    while (clang::isa<clang::LinkageSpecDecl>(context))
      context = context->getParent();

    auto map_and_self_name = [&]()
        -> sus::Option<sus::Tuple<
            std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash>&,
            std::string_view>> {
      if (clang::isa<clang::CXXConstructorDecl>(decl)) {
        sus::check(clang::isa<clang::RecordDecl>(context));
        if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
                clang::cast<clang::RecordDecl>(context));
            parent.is_some()) {
          return sus::some(
              sus::tuple(parent.as_value_mut().ctors, parent.as_value().name));
        }
      } else if (clang::isa<clang::CXXDestructorDecl>(decl)) {
        sus::check(clang::isa<clang::RecordDecl>(context));
        if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
                clang::cast<clang::RecordDecl>(context));
            parent.is_some()) {
          return sus::some(
              sus::tuple(parent.as_value_mut().dtors, parent.as_value().name));
        }
      } else if (clang::isa<clang::CXXConversionDecl>(decl)) {
        sus::check(clang::isa<clang::RecordDecl>(context));
        if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
                clang::cast<clang::RecordDecl>(context));
            parent.is_some()) {
          return sus::some(sus::tuple(parent.as_value_mut().conversions,
                                      parent.as_value().name));
        }
      } else if (clang::isa<clang::CXXMethodDecl>(decl)) {
        sus::check(clang::isa<clang::RecordDecl>(context));
        if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
                clang::cast<clang::RecordDecl>(context));
            parent.is_some()) {
          return sus::some(sus::tuple(parent.as_value_mut().methods,
                                      parent.as_value().name));
        }
      } else if (clang::isa<clang::CXXDeductionGuideDecl>(decl)) {
        sus::check(clang::isa<clang::NamespaceDecl>(context));
        // TODO: How do we get from here to the class that the deduction guide
        // is for reliably? getCorrespondingConstructor() would work if it's
        // generated only. Will the DeclContext find it?
        // return sus::some(parent->deductions);
      } else {
        if (sus::Option<NamespaceElement&> parent =
                docs_db_.find_namespace_mut(find_nearest_namespace(decl));
            parent.is_some()) {
          return sus::some(sus::tuple(parent.as_value_mut().functions, ""));
        }
      }
      return sus::none();
    }();

    if (map_and_self_name.is_some()) {
      auto&& [map, self_name] = sus::move(map_and_self_name).unwrap();

      Comment comment = make_db_comment(decl, raw_comment, self_name);

      auto params =
          sus::Vec<FunctionParameter>::with_capacity(decl->parameters().size());
      for (const clang::ParmVarDecl* v : decl->parameters()) {
        params.emplace(docs_db_.find_type(v->getOriginalType()),
                       sus::none(),  // TODO: `v->getDefaultArg()`
                       friendly_type_name(v->getOriginalType()),
                       friendly_short_type_name(v->getOriginalType()),
                       v->getNameAsString());
      }

      sus::Option<RequiresConstraints> constraints;
      if (clang::FunctionTemplateDecl* tmpl =
              decl->getDescribedFunctionTemplate()) {
        llvm::SmallVector<const clang::Expr*> assoc;
        tmpl->getAssociatedConstraints(assoc);
        for (const clang::Expr* e : assoc) {
          if (constraints.is_none()) constraints.insert(RequiresConstraints());
          requires_constraints_add_expr(constraints.as_value_mut(),
                                        decl->getASTContext(), e);
        }
      } else {
        llvm::SmallVector<const clang::Expr*> assoc;
        decl->getAssociatedConstraints(assoc);
        for (const clang::Expr* e : assoc) {
          if (constraints.is_none()) constraints.insert(RequiresConstraints());
          requires_constraints_add_expr(constraints.as_value_mut(),
                                        decl->getASTContext(), e);
        }
      }

      auto fe = FunctionElement(
          iter_namespace_path(decl).collect_vec(), sus::move(comment),
          decl->getNameAsString(), decl->isOverloadedOperator(),
          decl->getReturnType(), sus::move(constraints), decl->isDeleted(),
          sus::move(params),
          decl->getASTContext().getSourceManager().getFileOffset(
              decl->getLocation()));
      fe.overloads[0u].return_type_element =
          docs_db_.find_type(decl->getReturnType());

      if (clang::isa<clang::CXXMethodDecl>(decl)) {
        sus::check(clang::isa<clang::RecordDecl>(context));

        // TODO: It's possible to overload a method in a base class. What should
        // we show then?

        if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
                clang::cast<clang::RecordDecl>(context));
            parent.is_some()) {
          auto* mdecl = clang::cast<clang::CXXMethodDecl>(decl);
          fe.overloads[0u].method = sus::some(MethodSpecific{
              .is_static = mdecl->isStatic(),
              .is_volatile = mdecl->isVolatile(),
              .is_virtual = mdecl->isVirtual(),
              .is_ctor = clang::isa<clang::CXXConstructorDecl>(decl),
              .is_dtor = clang::isa<clang::CXXDestructorDecl>(decl),
              .is_conversion = clang::isa<clang::CXXConversionDecl>(decl),
              .qualifier =
                  [mdecl]() {
                    switch (mdecl->getRefQualifier()) {
                      case clang::RQ_None:
                        if (mdecl->isConst())
                          return MethodQualifier::Const;
                        else
                          return MethodQualifier::Mutable;
                      case clang::RQ_LValue:
                        if (mdecl->isConst())
                          return MethodQualifier::ConstLValue;
                        else
                          return MethodQualifier::MutableLValue;
                      case clang::RQ_RValue:
                        if (mdecl->isConst())
                          return MethodQualifier::ConstRValue;
                        else
                          return MethodQualifier::MutableRValue;
                    }
                    sus::unreachable();
                  }(),
          });
        }
      }
      add_function_overload_to_db(decl, sus::move(fe), map);
    }
    return true;
  }

 private:
  template <class MapT>
  void add_function_overload_to_db(clang::FunctionDecl* decl,
                                   FunctionElement db_element,
                                   MapT& db_map) noexcept {
    FunctionId key =
        key_for_function(decl, db_element.comment.attrs.overload_set);
    bool add_overload = true;
    auto it = db_map.find(key);
    if (it == db_map.end()) {
      db_map.emplace(key, std::move(db_element));
      add_overload = false;
    } else if (!it->second.has_comment() && db_element.has_comment()) {
      // Steal the comment.
      sus::mem::swap(db_map.at(key).comment, db_element.comment);
    } else if (!db_element.has_comment() & it->second.has_comment()) {
      // Leave the existing comment in place.
    } else if (db_element.comment.begin_loc == it->second.comment.begin_loc) {
      // We already visited this thing, from another translation unit.
      add_overload = false;
    } else {
      auto& ast_cx = decl->getASTContext();
      const FunctionElement& old_element = it->second;
      ast_cx.getDiagnostics()
          .Report(db_element.comment.attrs.location,
                  diag_ids_.superceded_comment)
          .AddString(old_element.comment.begin_loc);
    }

    if (add_overload) {
      ::sus::check_with_message(
          db_element.overloads.len() == 1u,
          *"Expected to add FunctionElement with 1 overload");
      db_map.at(key).overloads.push(sus::move(db_element.overloads[0u]));
    }
  }

  template <class MapT>
  void add_namespace_to_db(clang::NamespaceDecl* decl,
                           NamespaceElement db_element, MapT& db_map) noexcept {
    auto key = key_for_namespace(decl);
    auto it = db_map.find(key);
    if (it == db_map.end()) {
      db_map.emplace(key, std::move(db_element));
    } else if (!it->second.has_comment()) {
      // Steal the comment.
      sus::mem::swap(db_map.at(key).comment, db_element.comment);
    } else if (!db_element.has_comment()) {
      // Leave the existing comment in place, do nothing.
    } else if (db_element.comment.begin_loc == it->second.comment.begin_loc) {
      // We already visited this thing, from another translation unit.
    } else {
      auto& ast_cx = decl->getASTContext();
      const NamespaceElement& old_element = it->second;
      ast_cx.getDiagnostics()
          .Report(db_element.comment.attrs.location,
                  diag_ids_.superceded_comment)
          .AddString(old_element.comment.begin_loc);
    }
  }

  template <class ElementT, class MapT>
    requires ::sus::ptr::SameOrSubclassOf<ElementT*, CommentElement*>
  void add_record_to_db(clang::RecordDecl* decl, ElementT db_element,
                        MapT& db_map) noexcept {
    auto key = RecordId(*decl);
    auto it = db_map.find(key);
    if (it == db_map.end()) {
      db_map.emplace(key, std::move(db_element));
    } else if (!it->second.has_comment()) {
      // Steal the comment.
      sus::mem::swap(db_map.at(key).comment, db_element.comment);
    } else if (!db_element.has_comment()) {
      // Leave the existing comment in place, do nothing.
    } else if (db_element.comment.begin_loc == it->second.comment.begin_loc) {
      // We already visited this thing, from another translation unit.
    } else {
      auto& ast_cx = decl->getASTContext();
      const ElementT& old_element = it->second;
      ast_cx.getDiagnostics()
          .Report(db_element.comment.attrs.location,
                  diag_ids_.superceded_comment)
          .AddString(old_element.comment.begin_loc);
    }
  }
  template <class ElementT, class MapT>
    requires ::sus::ptr::SameOrSubclassOf<ElementT*, CommentElement*>
  void add_comment_to_db(clang::Decl* decl, ElementT db_element,
                         MapT& db_map) noexcept {
    UniqueSymbol uniq = unique_from_decl(decl);
    auto it = db_map.find(uniq);
    if (it == db_map.end()) {
      db_map.emplace(uniq, std::move(db_element));
    } else if (!it->second.has_comment()) {
      // Steal the comment.
      sus::mem::swap(db_map.at(uniq).comment, db_element.comment);
    } else if (!db_element.has_comment()) {
      // Leave the existing comment in place, do nothing.
    } else if (db_element.comment.begin_loc == it->second.comment.begin_loc) {
      // We already visited this thing, from another translation unit.
    } else {
      auto& ast_cx = decl->getASTContext();
      const ElementT& old_element = it->second;
      ast_cx.getDiagnostics()
          .Report(db_element.comment.attrs.location,
                  diag_ids_.superceded_comment)
          .AddString(old_element.comment.begin_loc);
    }
  }

  Comment make_db_comment(clang::Decl* decl, const clang::RawComment* raw,
                          std::string_view self_name) noexcept {
    auto& ast_cx = decl->getASTContext();
    auto& src_manager = ast_cx.getSourceManager();
    if (raw) {
      sus::Result<ParsedComment, ParseCommentError> comment_result =
          parse_comment(ast_cx, *raw, self_name);
      if (comment_result.is_ok()) {
        auto&& [attrs, full_html, summary_html] =
            sus::move(comment_result).unwrap();
        return Comment(full_html, summary_html,
                       raw->getBeginLoc().printToString(src_manager),
                       sus::move(attrs));
      }
      ast_cx.getDiagnostics()
          .Report(raw->getBeginLoc(), diag_ids_.malformed_comment)
          .AddString(sus::move(comment_result).unwrap_err().message);
    }
    return Comment();
  }

  VisitCx& cx_;
  Database& docs_db_;
  DiagnosticIds diag_ids_;
};

class AstConsumer : public clang::ASTConsumer {
 public:
  AstConsumer(VisitCx& cx, Database& docs_db) : cx_(cx), docs_db_(docs_db) {}

  bool HandleTopLevelDecl(clang::DeclGroupRef group_ref) noexcept final {
    for (clang::Decl* decl : group_ref) {
      clang::SourceManager& sm = decl->getASTContext().getSourceManager();

      if (!decl->getLocation().isMacroID()) {
        // Don't visit the same file repeatedly.
        auto v = VisitedLocation(decl->getLocation().printToString(sm));
        if (cx_.visited_locations.contains(v)) {
          continue;
        }
        cx_.visited_locations.emplace(sus::move(v));
      }

      if (!cx_.should_include_decl_based_on_file(decl)) {
        continue;
      }

      if (!Visitor(cx_, docs_db_, DiagnosticIds::with_context(decl->getASTContext()))
               .TraverseDecl(decl))
        return false;
    }
    return true;
  }

  void HandleTranslationUnit(clang::ASTContext& ast_cx) noexcept final {
    if (cx_.options.on_tu_complete.is_some()) {
      ::sus::fn::call(*cx_.options.on_tu_complete, ast_cx);
    }
  }

 private:
  VisitCx& cx_;
  Database& docs_db_;
};

std::unique_ptr<clang::FrontendAction> VisitorFactory::create() noexcept {
  return std::make_unique<VisitorAction>(cx, docs_db, line_stats);
}

std::unique_ptr<clang::ASTConsumer> VisitorAction::CreateASTConsumer(
    clang::CompilerInstance&, llvm::StringRef file) noexcept {
  if (cx.options.show_progress) {
    if (std::string_view(file) != line_stats.cur_file_name) {
      llvm::outs() << "[" << line_stats.cur_file << "/" << line_stats.num_files
                   << "] " << file << "\n";
      line_stats.cur_file += 1u;
      line_stats.cur_file_name = std::string(file);
    }
  }
  return std::make_unique<AstConsumer>(cx, docs_db);
}

bool VisitCx::should_include_decl_based_on_file(clang::Decl* decl) noexcept {
  clang::SourceManager& sm = decl->getASTContext().getSourceManager();

  clang::SourceLocation loc = decl->getLocation();
  const clang::FileEntry* entry = sm.getFileEntryForID(sm.getFileID(loc));
  // For a macro, find the place of the macro expansion, which is in an actual
  // file.
  while (loc.isMacroID()) {
    loc = sm.getExpansionLoc(loc);
    entry = sm.getFileEntryForID(sm.getFileID(loc));
    sus::check(entry != nullptr);
  }

  // No FileEntry (and not a macro, since we've found the macro expansion
  // above already) means a builtin, including a lot of `std::`, or maybe some
  // other things. We don't want to chase builtins.
  if (!entry) {
    return false;
  }

  // And if there's no path then we also default to include it.
  llvm::StringRef path = entry->tryGetRealPathName();
  if (path.empty()) {
    return true;
  }
  // Canonicalize the path to use `/` instead of `\`.
  auto canonical_path = std::string(path);
  std::replace(canonical_path.begin(), canonical_path.end(), '\\', '/');

  // Compare the path to the user-specified include/exclude-patterns.
  enum { CheckPath, ExcludePath, IncludePath } what_to_do;
  // We cache the regex decision for each visited path name.
  if (auto it = visited_paths_.find(canonical_path);
      it != visited_paths_.end()) {
    what_to_do = it->second.included ? IncludePath : ExcludePath;
  } else {
    what_to_do = CheckPath;
  }

  switch (what_to_do) {
    case IncludePath: return true;
    case ExcludePath: return false;
    case CheckPath: {
      auto [it, inserted] =
          visited_paths_.emplace(sus::move(canonical_path), VisitedPath(true));
      sus::check(inserted);
      auto& [path_str, visited_path] = *it;
      if (!std::regex_search(path_str, options.include_path_patterns)) {
        visited_path.included = false;
        return false;
      }
      if (std::regex_search(path_str, options.exclude_path_patterns)) {
        visited_path.included = false;
        return false;
      }
      return true;
    }
  }
  sus::unreachable();
}

}  // namespace subdoc
