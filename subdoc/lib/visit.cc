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

#include <stdio.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

#include <regex>

#include "subdoc/lib/database.h"
#include "subdoc/lib/doc_attributes.h"
#include "subdoc/lib/parse_comment.h"
#include "subdoc/lib/path.h"
#include "subdoc/lib/record_type.h"
#include "subdoc/lib/unique_symbol.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/macros/compiler.h"
#include "subspace/mem/swap.h"
#include "subspace/ops/ord.h"
#include "subspace/prelude.h"

namespace subdoc {

struct DiagnosticIds {
  static DiagnosticIds with(clang::ASTContext& ast_cx) noexcept {
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
  if (path_contains_namespace(
          ndecl, sus::choice<Namespace::Tag::Named>("__private"))) {
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

    Comment comment = make_db_comment(decl, raw_comment);
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
    add_namespace_to_db(decl, sus::move(ne), mref(parent.namespaces));
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

    Comment comment = make_db_comment(decl, raw_comment);
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
      add_record_to_db(decl, sus::move(re), mref(parent.records));
    } else if (clang::isa<clang::NamespaceDecl>(context)) {
      NamespaceElement& parent =
          docs_db_
              .find_namespace_mut(clang::cast<clang::NamespaceDecl>(context))
              .unwrap();
      add_record_to_db(decl, sus::move(re), mref(parent.records));
    } else {
      assert(clang::isa<clang::RecordDecl>(context));
      if (sus::Option<RecordElement&> parent =
              docs_db_.find_record_mut(clang::cast<clang::RecordDecl>(context));
          parent.is_some()) {
        add_record_to_db(decl, sus::move(re), mref(parent->records));
      }
    }
    return true;
  }

  bool VisitFieldDecl(clang::FieldDecl* decl) noexcept {
    if (should_skip_decl(cx_, decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);

    clang::RecordDecl* record_decl =
        clang::cast<clang::RecordDecl>(decl->getDeclContext());

    Comment comment = make_db_comment(decl, raw_comment);
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
      add_comment_to_db(decl, sus::move(fe), mref(parent->fields));
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl* decl) noexcept {
    // Static data members are represented as VarDecl, not FieldDecl. So we
    // visit VarDecls but only for them.
    if (!decl->isStaticDataMember()) return true;

    if (should_skip_decl(cx_, decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);

    Comment comment = make_db_comment(decl, raw_comment);
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
      add_comment_to_db(decl, sus::move(fe), mref(parent->fields));
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
    if (should_skip_decl(cx_, decl)) return true;
    clang::RawComment* raw_comment = get_raw_comment(decl);

    Comment comment = make_db_comment(decl, raw_comment);

    auto params =
        sus::Vec<FunctionParameter>::with_capacity(decl->parameters().size());
    for (const clang::ParmVarDecl* v : decl->parameters()) {
      params.emplace(docs_db_.find_type(v->getOriginalType()),
                     sus::none(),  // TODO: `v->getDefaultArg()`
                     friendly_type_name(v->getOriginalType()),
                     friendly_short_type_name(v->getOriginalType()));
    }

    auto fe = FunctionElement(
        iter_namespace_path(decl).collect_vec(), sus::move(comment),
        decl->getNameAsString(), decl->getReturnType(), sus::move(params),
        decl->getASTContext().getSourceManager().getFileOffset(
            decl->getLocation()));
    fe.return_type_element = docs_db_.find_type(decl->getReturnType());

    // TODO: It's possible to overload a method in a base class. What should we
    // show then?

    if (clang::isa<clang::CXXConstructorDecl>(decl)) {
      assert(clang::isa<clang::RecordDecl>(decl->getDeclContext()));
      if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
              clang::cast<clang::RecordDecl>(decl->getDeclContext()));
          parent.is_some()) {
        add_function_overload_to_db(decl, sus::move(fe), mref(parent->ctors));
      }
    } else if (clang::isa<clang::CXXDestructorDecl>(decl)) {
      assert(clang::isa<clang::RecordDecl>(decl->getDeclContext()));
      if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
              clang::cast<clang::RecordDecl>(decl->getDeclContext()));
          parent.is_some()) {
        add_function_overload_to_db(decl, sus::move(fe), mref(parent->dtors));
      }
    } else if (clang::isa<clang::CXXConversionDecl>(decl)) {
      assert(clang::isa<clang::RecordDecl>(decl->getDeclContext()));
      if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
              clang::cast<clang::RecordDecl>(decl->getDeclContext()));
          parent.is_some()) {
        add_function_overload_to_db(decl, sus::move(fe),
                                    mref(parent->conversions));
      }
    } else if (clang::isa<clang::CXXMethodDecl>(decl)) {
      assert(clang::isa<clang::RecordDecl>(decl->getDeclContext()));
      if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
              clang::cast<clang::RecordDecl>(decl->getDeclContext()));
          parent.is_some()) {
        auto* mdecl = clang::cast<clang::CXXMethodDecl>(decl);
        fe.overloads[0u].method = sus::some(MethodSpecific{
            .is_static = mdecl->isStatic(),
            .is_volatile = mdecl->isVolatile(),
            .is_virtual = mdecl->isVirtual(),
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
        add_function_overload_to_db(decl, sus::move(fe), mref(parent->methods));
      }
    } else if (clang::isa<clang::CXXDeductionGuideDecl>(decl)) {
      clang::DeclContext* context = decl->getDeclContext();
      // TODO: Save the linkage spec (`extern "C"`) so we can show it.
      while (clang::isa<clang::LinkageSpecDecl>(context))
        context = context->getParent();

      // TODO: How do we get from here to the class that the deduction guide is
      // for reliably? getCorrespondingConstructor() would work if it's
      // generated only. Will the getDeclContext find it?
      assert(clang::isa<clang::NamespaceDecl>(decl->getDeclContext()));
      /* TODO:
      if (sus::Option<RecordElement&> parent = docs_db_.find_record_mut(
              clang::cast<clang::RecordDecl>(decl->getDeclContext()));
          parent.is_some()) {
        add_function_overload_to_db(decl,
                                    sus::move(fe), mref(parent->deductions));
      }
      */
    } else {
      if (sus::Option<NamespaceElement&> parent =
              docs_db_.find_namespace_mut(find_nearest_namespace(decl));
          parent.is_some()) {
        add_function_overload_to_db(decl, sus::move(fe),
                                    mref(parent->functions));
      }
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
    } else if (!it->second.has_comment()) {
      // Steal the comment.
      sus::mem::swap(db_map.at(key).comment, db_element.comment);
    } else if (!db_element.has_comment()) {
      // Leave the existing comment in place.
    } else if (db_element.comment.begin_loc == it->second.comment.begin_loc) {
      // We already visited this thing, from another translation unit.
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
    requires ::sus::convert::SameOrSubclassOf<ElementT*, CommentElement*>
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
    requires ::sus::convert::SameOrSubclassOf<ElementT*, CommentElement*>
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

  Comment make_db_comment(clang::Decl* decl,
                          const clang::RawComment* raw) noexcept {
    auto& ast_cx = decl->getASTContext();
    auto& src_manager = ast_cx.getSourceManager();
    if (raw) {
      sus::result::Result<ParsedComment, ParseCommentError> comment_result =
          parse_comment(ast_cx, *raw);
      if (comment_result.is_ok()) {
        auto&& [attrs, string] = sus::move(comment_result).unwrap();
        return Comment(string, raw->getBeginLoc().printToString(src_manager),
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

      // Don't visit the same file repeatedly.
      auto v = VisitedLocation(decl->getLocation().printToString(sm));
      if (cx_.visited_locations.contains(v)) continue;
      cx_.visited_locations.emplace(sus::move(v));

      if (!cx_.should_include_decl_based_on_file(decl)) continue;

      if (!Visitor(cx_, docs_db_, DiagnosticIds::with(decl->getASTContext()))
               .TraverseDecl(decl))
        return false;
    }
    return true;
  }

  void HandleTranslationUnit(clang::ASTContext& ast_cx) noexcept final {
    if (cx_.options.on_tu_complete.is_some()) {
      (*cx_.options.on_tu_complete)(ast_cx);
    }
  }

 private:
  VisitCx& cx_;
  Database& docs_db_;
};

std::unique_ptr<clang::FrontendAction> VisitorFactory::create() noexcept {
  return std::make_unique<VisitorAction>(cx, docs_db, line_stats);
}

bool VisitorAction::PrepareToExecuteAction(
    clang::CompilerInstance& inst) noexcept {
  // Speed things up by skipping things we're not looking at.
  inst.getFrontendOpts().SkipFunctionBodies = true;
  return true;
}

std::unique_ptr<clang::ASTConsumer> VisitorAction::CreateASTConsumer(
    clang::CompilerInstance&, llvm::StringRef file) noexcept {
  if (cx.options.show_progress) {
    if (std::string_view(file) != line_stats.cur_file_name) {
      std::string to_print = [&]() {
        std::ostringstream s;
        s << "[" << size_t{line_stats.cur_file} << "/"
          << size_t{line_stats.num_files} << "]";
        s << " " << std::string_view(file);
        return sus::move(s).str();
      }();
      auto isatty = &sus_if_msvc_else(_isatty, isatty);
      auto fileno = &sus_if_msvc_else(_fileno, fileno);
      if (isatty(fileno(stdin))) {
        llvm::outs() << "\r" << to_print;
        for (usize i;
             i < line_stats.last_line_len.saturating_sub(to_print.size());
             i += 1u) {
          llvm::errs() << " ";
        }
      } else {
        llvm::outs() << to_print << "\n";
      }
      line_stats.last_line_len = to_print.size();
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
  // No FileEntry means a builtin, including a lot of `std::`, or inside
  // a macro instantiation, or maybe some other things. We want to chase
  // decls inside macros, but not builtins.
  if (!entry && !loc.isMacroID()) return false;

  // Unable to find a file path without a FileEntry, so default to include it.
  if (!entry) return true;
  // And if there's no path then we also default to include it.
  llvm::StringRef path = entry->tryGetRealPathName();
  if (path.empty()) return true;

  // Compare the path to the user-specified include/exclude-patterns.
  enum { CheckPath, ExcludePath, IncludePath } what_to_do;
  // We cache the regex decision for each visited path name.
  if (auto it = visited_paths_.find(std::string_view(path));
      it != visited_paths_.end()) {
    what_to_do = it->second.included ? IncludePath : ExcludePath;
  } else {
    what_to_do = CheckPath;
  }

  switch (what_to_do) {
    case IncludePath: return true;
    case ExcludePath: return false;
    case CheckPath: {
      // std::regex requires a string, so store it up front.
      auto [it, inserted] =
          visited_paths_.emplace(std::string(path), VisitedPath(true));
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
  // SAFETY: All enum values are covered in the switch, and they all return.
  sus::unreachable_unchecked(unsafe_fn);
}

}  // namespace subdoc
