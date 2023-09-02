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

#include "subdoc/lib/stmt_to_string.h"
#include "sus/assertions/check.h"
#include "sus/iter/iterator.h"

namespace subdoc {

namespace {
Qualifier qualifier_from_qualtype(clang::QualType q) noexcept {
  return Qualifier{
      .is_const = q.isLocalConstQualified(),
      .is_volatile = q.isLocalVolatileQualified(),
  };
}

std::string name_of_type(clang::QualType q) noexcept {
  // Clang writes booleans as "_Bool".
  if (q->isBooleanType()) return "bool";
  std::string name = q.getLocalUnqualifiedType().getAsString();
  // TODO: Is there a better way to drop the namespaces/records from the type?
  if (usize pos = name.rfind("::"); pos != std::string::npos)
    name = name.substr(pos + 2u);
  // TODO: Is there a better way to drop the template specialization from the
  // type?
  if (usize pos = name.find("<"); pos != std::string::npos)
    name = name.substr(0u, pos);
  return name;
}

TypeOrValue build_template_param(const clang::TemplateArgument& arg,
                                 const clang::SourceManager& sm,
                                 clang::Preprocessor& preprocessor) noexcept {
  switch (arg.getKind()) {
    case clang::TemplateArgument::ArgKind::Null: sus::unreachable();
    case clang::TemplateArgument::ArgKind::Type: {
      if (arg.getAsType()->isDependentType()) {
        // A template argument that is a template parameter (from the function,
        // the class, etc.)
        return TypeOrValue(
            TypeOrValueChoice::with<TypeOrValueChoice::Tag::DependentType>(
                arg.getAsType().getAsString()));
      }
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
          sus::tuple(build_local_type(arg.getIntegralType(), sm, preprocessor),
                     llvm_int_to_string(arg.getAsIntegral()))));
    }
    case clang::TemplateArgument::ArgKind::Template: sus::unreachable();
    case clang::TemplateArgument::ArgKind::TemplateExpansion:
      sus::unreachable();
    case clang::TemplateArgument::ArgKind::Expression: sus::unreachable();
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

  // Auto type that is deduced should be unpacked to the deduced type. Pointers
  // can be auto, so this needs to come after unpacking the pointers, but should
  // come before anything else as it may be deduced to anything else.
  if (auto* auto_type = clang::dyn_cast<clang::AutoType>(&*qualtype)) {
    if (auto_type->isDeduced()) qualtype = auto_type->getDeducedType();
  }

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
  }

  // Find the context from which to collect the namespace/record paths.
  clang::DeclContext* context = nullptr;
  if (clang::isa<clang::AutoType>(&*qualtype)) {
    // No context.
  } else if (clang::isa<clang::BuiltinType>(&*qualtype)) {
    // No context.
  } else if (auto* tag_type = clang::dyn_cast<clang::TagType>(&*qualtype)) {
    context = tag_type->getDecl()->getDeclContext();
  } else if (auto* spec_type =
                 clang::dyn_cast<clang::TemplateSpecializationType>(
                     &*qualtype)) {
    context =
        spec_type->getTemplateName().getAsTemplateDecl()->getDeclContext();
  } else if (clang::isa<clang::TemplateTypeParmType>(&*qualtype)) {
    // No context.
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

  std::string name = name_of_type(qualtype);

  return Type(sus::move(namespace_path), sus::move(record_path),
              sus::move(name), refs, sus::move(qualifier), sus::move(pointers),
              sus::move(array_dims), sus::move(template_params));
}

}  // namespace subdoc
