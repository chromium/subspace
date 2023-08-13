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

#include "subdoc/lib/requires.h"

#include "sus/iter/compat_ranges.h"
#include "sus/iter/iterator.h"

namespace subdoc {

std::string template_arg_to_string(
    const clang::TemplateArgumentLoc& loc) noexcept {
  const clang::TemplateArgument& arg = loc.getArgument();
  switch (arg.getKind()) {
    case clang::TemplateArgument::ArgKind::Null:
      // How can this happen in a concept instantiation?
      sus::unreachable();
    case clang::TemplateArgument::ArgKind::Type: {
      if (arg.getAsType()->isDependentType()) {
        // A template argument that is a template parameter (from the function,
        // the class, etc.)
        return arg.getAsType().getAsString();
      }
      // TODO: Can be a link to a TypeElement in the Database.
      return arg.getAsType().getCanonicalType().getAsString();
    }
    case clang::TemplateArgument::ArgKind::Declaration:
      // How can this happen in a concept instantiation?
      sus::unreachable();
    case clang::TemplateArgument::ArgKind::NullPtr: return "nullptr";
    case clang::TemplateArgument::ArgKind::Integral: {
      llvm::APSInt i = arg.getAsIntegral();
      llvm::SmallString<20> str;
      i.toString(str, 10u, i.isSigned());
      return std::string(str.str());
    }
    case clang::TemplateArgument::ArgKind::Template:
      // How can this happen in a concept instantiation?
      sus::unreachable();
    case clang::TemplateArgument::ArgKind::TemplateExpansion:
      // How can this happen in a concept instantiation?
      sus::unreachable();
    case clang::TemplateArgument::ArgKind::Expression:
      // How can this happen in a concept instantiation?
      sus::unreachable();
    case clang::TemplateArgument::ArgKind::Pack:
      return std::string("TODO: pack");
  }
  sus::unreachable();
};

void requires_constraints_add_expr(RequiresConstraints& constaints,
                                   const clang::ASTContext& context,
                                   const clang::Expr* e) noexcept {
  e = e->IgnoreParens();

  if (auto* bin = clang::dyn_cast<clang::BinaryOperator>(e);
      bin != nullptr && bin->getOpcode() == clang::BO_LAnd) {
    requires_constraints_add_expr(constaints, context, bin->getLHS());
    requires_constraints_add_expr(constaints, context, bin->getRHS());
  } else if (auto* c = clang::dyn_cast<clang::ConceptSpecializationExpr>(e);
             c != nullptr && c->getNamedConcept()) {
    sus::Vec<std::string> args =
        sus::iter::from_range(c->getTemplateArgsAsWritten()->arguments())
            .map(template_arg_to_string)
            .collect<sus::Vec<std::string>>();
    for (std::string_view s : args) {
      // `Concept auto foo` appears in the function signature, no need to add a
      // constrait for it outside, and there's no type name to refer to in it
      // either.
      if (s.ends_with(" auto")) return;
    }
    constaints.list.push(
        RequiresConstraint::with<RequiresConstraint::Tag::Concept>(
            RequiresConceptConstraint{
                // TODO: Split this up into namespaces and link them to
                // NamespaceElements and a ConceptElement in the Database.
                .concept_name =
                    c->getNamedConcept()->getQualifiedNameAsString(),
                .args = sus::move(args),
            }));
  } else {
    const clang::SourceManager& sm = context.getSourceManager();
    const char* start = sm.getCharacterData(e->getBeginLoc());
    const char* end = sm.getCharacterData(e->getEndLoc()) + 1;

    // TODO: There can be types in here that need to be resolved, such as the
    // macro name `_primitive` in:
    // * `::sus::mem::size_of<S>() <= ::sus::mem::size_of<_primitive>()`
    constaints.list.push(
        RequiresConstraint::with<RequiresConstraint::Tag::Text>(
            std::string(std::string_view(start, end - start))));
  }
}

}  // namespace subdoc
