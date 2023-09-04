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

#include <sstream>

#include "subdoc/lib/stmt_to_string.h"
#include "sus/assertions/check.h"
#include "sus/iter/compat_ranges.h"
#include "sus/iter/iterator.h"
#include "sus/ptr/as_ref.h"

namespace subdoc {

namespace {
Qualifier qualifier_from_qualtype(clang::QualType q) noexcept {
  return Qualifier{
      .is_const = q.isLocalConstQualified(),
      .is_volatile = q.isLocalVolatileQualified(),
  };
}

std::string name_of_type(clang::QualType q) noexcept {
  clang::LangOptions lang;
  lang.LangStd = clang::LangStandard::Kind::lang_cxx20;  // TODO: Configurable?
  clang::PrintingPolicy p(lang);
  p.Bool = true;
  p.SuppressScope = true;
  p.SuppressUnwrittenScope = true;
  p.SuppressTagKeyword = true;
  p.SplitTemplateClosers = false;
  std::string name = q.getLocalUnqualifiedType().getAsString(p);
  // TODO: Is there a better way to drop the namespaces/records from the type?
  //if (usize pos = name.rfind("::"); pos != std::string::npos)
  //  name = name.substr(pos + 2u);
  // TODO: Is there a better way to drop the template specialization from the
  // type?
  if (usize pos = name.find("<"); pos != std::string::npos)
    name = name.substr(0u, pos);
  return name;
}

std::string template_to_string(clang::TemplateName template_name) noexcept {
  clang::TemplateDecl* decl = template_name.getAsTemplateDecl();
  sus::check_with_message(decl != nullptr, "TemplateName without Decl?");

  sus::Vec<clang::NamedDecl*> contexts;
  clang::DeclContext* context = decl->getDeclContext();
  while (context) {
    if (auto* n = clang::dyn_cast<clang::NamedDecl>(context)) {
      if (clang::isa<clang::NamespaceDecl>(n) ||
          clang::isa<clang::RecordDecl>(n))
        contexts.push(n);
    }
    context = context->getParent();
  }

  std::ostringstream str;
  for (clang::NamedDecl* n : sus::move(contexts).into_iter().rev()) {
    str << n->getNameAsString();
    str << "::";
  }
  str << decl->getNameAsString();
  return sus::move(str).str();
}

/// Returns whether the parameter is of the form `Concept auto` which
/// specializes and references a concept as an anonymous template type for the
/// parameter.
bool template_parameter_is_concept(
    const clang::TemplateTypeParmType& parm) noexcept {
  return (parm.getDecl()->hasTypeConstraint() && parm.getDecl()->isImplicit());
}

TypeOrValue build_template_param(const clang::TemplateArgument& arg,
                                 const clang::SourceManager& sm,
                                 clang::Preprocessor& preprocessor) noexcept {
  switch (arg.getKind()) {
    case clang::TemplateArgument::ArgKind::Null: sus::unreachable();
    case clang::TemplateArgument::ArgKind::Type: {
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Type>(
          build_local_type(arg.getAsType(), sm, preprocessor)));
    }
    case clang::TemplateArgument::ArgKind::Declaration:
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Type>(
          build_local_type(arg.getAsDecl()->getType(), sm, preprocessor)));
    case clang::TemplateArgument::ArgKind::NullPtr:
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Type>(
          build_local_type(arg.getNullPtrType(), sm, preprocessor)));
    case clang::TemplateArgument::ArgKind::Integral: {
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Value>(
          llvm_int_to_string(arg.getAsIntegral())));
    }
    case clang::TemplateArgument::ArgKind::Template:
      // Getting here means the template parameter is itself a template
      // (without its own parameters specified), rather than a specialization of
      // a template.
      // ```
      // template <class T> struct S {};
      // void f(Concept<S>);  // Does land in here.
      // void f(Concept<S<int>>);  // Does not land in here.
      // ```
      // Since it's not a complete type, we can't parse a `clang::QualType`. So
      // we save the string as a Value.
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Value>(
          template_to_string(arg.getAsTemplate())));
    case clang::TemplateArgument::ArgKind::TemplateExpansion:
      sus::unreachable();
    case clang::TemplateArgument::ArgKind::Expression:
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Value>(
          stmt_to_string(*arg.getAsExpr(), sm, preprocessor)));
    case clang::TemplateArgument::ArgKind::Pack: sus::unreachable();
  }
  sus::unreachable();
};

}  // namespace

Type build_local_type(clang::QualType qualtype, const clang::SourceManager& sm,
                      clang::Preprocessor& preprocessor) noexcept {
  Refs refs =
      qualtype->isLValueReferenceType()
          ? Refs::LValueRef
          : (qualtype->isRValueReferenceType() ? Refs::RValueRef : Refs::None);
  qualtype = qualtype.getNonReferenceType();

  // Arrays may already be "DecayedType", but we can get the original type from
  // it.
  if (auto* dtype = clang::dyn_cast<clang::DecayedType>(&*qualtype)) {
    qualtype = dtype->getOriginalType();
  }

  // Grab the qualifiers on the outer type, if it's a pointer this is what we
  // want. But for an array we will need to replace these.
  Qualifier qualifier = qualifier_from_qualtype(qualtype);

  sus::Vec<std::string> array_dims;
  while (qualtype->isArrayType()) {
    const clang::Type* const type = &*qualtype;
    if (auto* constarr = clang::dyn_cast<clang::ConstantArrayType>(type)) {
      array_dims.push(
          llvm_int_without_sign_to_string(constarr->getSize(), false));
    }
    if (auto* deparr = clang::dyn_cast<clang::DependentSizedArrayType>(type)) {
      array_dims.push(stmt_to_string(*deparr->getSizeExpr(), sm, preprocessor));
    }
    if (auto* incarr = clang::dyn_cast<clang::IncompleteArrayType>(type)) {
      array_dims.push("");
    }
    if (auto* vararr = clang::dyn_cast<clang::VariableArrayType>(type)) {
      sus::unreachable();  // This is a C thing, not C++.
    }

    qualtype = clang::cast<clang::ArrayType>(&*qualtype)->getElementType();
    // For arrays the root qualifiers come from the element type.
    qualifier = qualifier_from_qualtype(qualtype);
  }

  // The array can be an array of pointers, so we look for pointers after
  // unwrapping the array.
  sus::Vec<Qualifier> pointers;
  while (qualtype->isPointerType()) {
    qualtype = qualtype->getPointeeType();
    pointers.push(qualifier_from_qualtype(qualtype));
  }
  // TODO: Vec::reverse() when it exists.
  pointers = sus::move(pointers).into_iter().rev().collect_vec();

  // A `using A = B` is an elaborated type that names a typedef A, so unpack
  // the ElaboratedType. Template specializations can be inside an
  // ElaboratedType, so this comes first.
  while (auto* elab = clang::dyn_cast<clang::ElaboratedType>(&*qualtype))
    qualtype = elab->getNamedType();

  // Arrays and pointers aren't templated, but the inner type can be, so we
  // look for this after stripping off references, arrays, and pointers.
  sus::Vec<TypeOrValue> template_params;
  if (auto* ttype =
          clang::dyn_cast<clang::TemplateSpecializationType>(&*qualtype)) {
    for (const clang::TemplateArgument& arg : ttype->template_arguments()) {
      template_params.push(build_template_param(arg, sm, preprocessor));
    }
  } else if (auto* ptype =
                 clang::dyn_cast<clang::TemplateTypeParmType>(&*qualtype)) {
    if (template_parameter_is_concept(*ptype)) {
      // This is a `Concept<...> auto` parameter, which may or may not have
      // template arguments on the Concept.
      auto it =
          sus::ptr::as_ref(
              ptype->getDecl()->getTypeConstraint()->getTemplateArgsAsWritten())
              .into_iter()
              .flat_map(
                  [](const clang::ASTTemplateArgumentListInfo& as_written) {
                    return sus::iter::from_range(as_written.arguments());
                  })
              .map(&clang::TemplateArgumentLoc::getArgument);
      for (const clang::TemplateArgument& arg : it) {
        template_params.push(build_template_param(arg, sm, preprocessor));
      }
    }
  } else if (auto* auto_type = clang::dyn_cast<clang::AutoType>(&*qualtype)) {
    // This may be a `Concept auto` in a location other than a function
    // parameter. Arguments would be part of that Concept specialization.
    for (const clang::TemplateArgument& arg :
         auto_type->getTypeConstraintArguments()) {
      template_params.push(build_template_param(arg, sm, preprocessor));
    }
  }

  // Find the context from which to collect the namespace/record paths.
  clang::DeclContext* context = nullptr;
  if (auto* auto_type = clang::dyn_cast<clang::AutoType>(&*qualtype)) {
    if (clang::ConceptDecl* condecl = auto_type->getTypeConstraintConcept()) {
      context = condecl->getDeclContext();
    }
  } else if (clang::isa<clang::BuiltinType>(&*qualtype)) {
    // No context.
  } else if (clang::isa<clang::DecltypeType>(&*qualtype)) {
    // No context.
  } else if (clang::isa<clang::PackExpansionType>(&*qualtype)) {
    // No context.
  } else if (auto* tag_type = clang::dyn_cast<clang::TagType>(&*qualtype)) {
    context = tag_type->getDecl()->getDeclContext();
  } else if (auto* spec_type =
                 clang::dyn_cast<clang::TemplateSpecializationType>(
                     &*qualtype)) {
    context =
        spec_type->getTemplateName().getAsTemplateDecl()->getDeclContext();
  } else if (auto* tparm_type =
                 clang::dyn_cast<clang::TemplateTypeParmType>(&*qualtype)) {
    if (template_parameter_is_concept(*tparm_type)) {
      // This is a `Concept auto` parameter, get the context for the Concept.
      context = tparm_type->getDecl()
                    ->getTypeConstraint()
                    ->getNamedConcept()
                    ->getDeclContext();
    }
  } else if (auto* typedef_type =
                 clang::dyn_cast<clang::TypedefType>(&*qualtype)) {
    context = typedef_type->getDecl()->getDeclContext();
  } else if (auto* using_type =
                 clang::dyn_cast<clang::UnresolvedUsingType>(&*qualtype)) {
    context = using_type->getDecl()->getDeclContext();
  } else {
    qualtype->dump();
    sus::unreachable();  // Find the context.
  }

  sus::Vec<std::string> namespace_path;
  sus::Vec<std::string> record_path;
  while (context) {
    if (auto* record = clang::dyn_cast<clang::RecordDecl>(context)) {
      record_path.push(record->getNameAsString());
    }
    if (auto* ns = clang::dyn_cast<clang::NamespaceDecl>(context)) {
      namespace_path.push(ns->getNameAsString());
    }
    context = context->getParent();
  }
  // TODO: Use Vec::reverse().
  namespace_path = sus::move(namespace_path).into_iter().rev().collect_vec();
  record_path = sus::move(record_path).into_iter().rev().collect_vec();

  auto [name, category] = [&]() -> sus::Tuple<std::string, TypeCategory> {
    if (auto* c = clang::dyn_cast<clang::TemplateTypeParmType>(&*qualtype)) {
      if (template_parameter_is_concept(*c)) {
        // `Concept auto` returns true for `isImplicit`, while the use of a
        // template variable name constrained by a concept does not.
        return sus::tuple(c->getDecl()
                              ->getTypeConstraint()
                              ->getNamedConcept()
                              ->getNameAsString(),
                          TypeCategory::Concept);
      } else {
        return sus::tuple(name_of_type(qualtype),
                          TypeCategory::TemplateVariable);
      }
    } else if (auto* auto_type = clang::dyn_cast<clang::AutoType>(&*qualtype)) {
      if (clang::ConceptDecl* condecl = auto_type->getTypeConstraintConcept()) {
        // This is a `Concept auto` in a location other than a function
        // parameter.
        return sus::tuple(condecl->getNameAsString(), TypeCategory::Concept);
      } else {
        sus::check_with_message(!auto_type->isConstrained(),
                                "constrained auto without a concept?");
        if (auto_type->isDecltypeAuto()) {
          return sus::tuple("decltype(auto)", TypeCategory::TemplateVariable);
        } else {
          return sus::tuple("auto", TypeCategory::TemplateVariable);
        }
      }
    } else if (clang::isa<clang::DecltypeType>(&*qualtype)) {
      // A decltype is an expression, it should not link to a type itself, so we
      // call it a TemplateVariable. If we want to introspect inside the
      // decltype and get types from the expression, and we could but don't yet,
      // then we would need a different TypeCategory with data fields to hold
      // the expression parts, similar to but different from `template_params`.
      return sus::tuple(name_of_type(qualtype), TypeCategory::TemplateVariable);
    } else if (clang::isa<clang::PackExpansionType>(&*qualtype)) {
      return sus::tuple(name_of_type(qualtype), TypeCategory::TemplateVariable);
    } else {
      return sus::tuple(name_of_type(qualtype), TypeCategory::Type);
    }
  }();

  return Type(category, sus::move(namespace_path), sus::move(record_path),
              sus::move(name), refs, sus::move(qualifier), sus::move(pointers),
              sus::move(array_dims), sus::move(template_params));
}

namespace {

void type_to_string_internal(
    const Type& type, sus::fn::FnMutRef<void(std::string_view)>& text_fn,
    sus::fn::FnMutRef<void(TypeToStringQuery)>& type_fn,
    sus::fn::FnMutRef<void()>& const_qualifier_fn,
    sus::fn::FnMutRef<void()>& volatile_qualifier_fn,
    sus::Option<sus::fn::FnOnceRef<void()>> var_name_fn) noexcept {
  switch (type.category) {
    case TypeCategory::Concept: [[fallthrough]];
    case TypeCategory::Type:
      type_fn(TypeToStringQuery{
          .namespace_path = type.namespace_path.as_slice(),
          .record_path = type.record_path.as_slice(),
          .name = std::string_view(type.name),
      });
      break;
    case TypeCategory::TemplateVariable:
      // For template variables, do not call the callback. They may have
      // name collisions with actual types, but they are not those types.
      text_fn(type.name);
      break;
  }

  if (!type.template_params.is_empty()) {
    text_fn("<");
    for (const auto& [i, tv] : type.template_params.iter().enumerate()) {
      if (i > 0u) text_fn(", ");
      switch (tv.choice) {
        case TypeOrValueTag::Type: {
          const Type& template_type = tv.choice.as<TypeOrValueTag::Type>();
          type_to_string_internal(template_type, text_fn, type_fn,
                                  const_qualifier_fn, volatile_qualifier_fn,
                                  sus::none());
          break;
        }
        case TypeOrValueTag::Value: {
          // The type of the value isn't used here, we just write the value.
          text_fn(tv.choice.as<TypeOrValueTag::Value>());
          break;
        }
      }
    }
    text_fn(">");
  }

  if (type.category == TypeCategory::Concept) text_fn(" auto");

  for (Qualifier q : type.pointers) {
    if (q.is_const) {
      text_fn(" ");
      const_qualifier_fn();
    }
    if (q.is_volatile) {
      text_fn(" ");
      volatile_qualifier_fn();
    }
    text_fn("*");
  }

  // TODO: Option to put the const/volatile qualifiers before the type name?
  if (type.qualifier.is_const) {
    text_fn(" ");
    const_qualifier_fn();
  }
  if (type.qualifier.is_volatile) {
    text_fn(" ");
    volatile_qualifier_fn();
  }

  if (type.array_dims.is_empty()) {
    switch (type.refs) {
      case Refs::None: break;
      case Refs::LValueRef: text_fn("&"); break;
      case Refs::RValueRef: text_fn("&&"); break;
    }
    if (var_name_fn.is_some()) {
      text_fn(" ");
      var_name_fn.take().unwrap()();
    }
  } else {
    if (type.refs != Refs::None) {
      text_fn(" (");
      switch (type.refs) {
        case Refs::None: break;
        case Refs::LValueRef: text_fn("&"); break;
        case Refs::RValueRef: text_fn("&&"); break;
      }
      if (var_name_fn.is_some()) var_name_fn.take().unwrap()();

      text_fn(")");
    } else {
      if (var_name_fn.is_some()) {
        text_fn(" ");
        var_name_fn.take().unwrap()();
      }
    }
    if (!type.array_dims.is_empty()) {
      for (const std::string& dim : type.array_dims) {
        text_fn("[");
        text_fn(dim);
        text_fn("]");
      }
    }
  }
}

void type_walk_types_internal(
    const Type& type,
    sus::fn::FnMutRef<void(TypeToStringQuery)>& type_fn) noexcept {
  switch (type.category) {
    case TypeCategory::Concept: [[fallthrough]];
    case TypeCategory::Type:
      type_fn(TypeToStringQuery{
          .namespace_path = type.namespace_path.as_slice(),
          .record_path = type.record_path.as_slice(),
          .name = std::string_view(type.name),
      });
      break;
    case TypeCategory::TemplateVariable: break;
  }

  for (const TypeOrValue& tv : type.template_params) {
    switch (tv.choice) {
      case TypeOrValueTag::Type: {
        const Type& template_type = tv.choice.as<TypeOrValueTag::Type>();
        type_walk_types_internal(template_type, type_fn);
        break;
      }
      case TypeOrValueTag::Value: {
        break;
      }
    }
  }
}

}  // namespace

void type_to_string(
    const Type& type, sus::fn::FnMutRef<void(std::string_view)> text_fn,
    sus::fn::FnMutRef<void(TypeToStringQuery)> type_fn,
    sus::fn::FnMutRef<void()> const_qualifier_fn,
    sus::fn::FnMutRef<void()> volatile_qualifier_fn,
    sus::Option<sus::fn::FnOnceRef<void()>> var_name_fn) noexcept {
  return type_to_string_internal(type, text_fn, type_fn, const_qualifier_fn,
                                 volatile_qualifier_fn, sus::move(var_name_fn));
}

void type_walk_types(
    const Type& type,
    sus::fn::FnMutRef<void(TypeToStringQuery)> type_fn) noexcept {
  return type_walk_types_internal(type, type_fn);
}

}  // namespace subdoc
