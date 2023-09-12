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
#include "sus/iter/generator.h"
#include "sus/iter/iterator.h"
#include "sus/iter/once.h"
#include "sus/ops/range.h"
#include "sus/ptr/as_ref.h"

namespace subdoc {

namespace {
Qualifier qualifier_from_qualtype(clang::QualType q) noexcept {
  Nullness null = Nullness::Unknown;
  if (auto* attr_type = clang::dyn_cast<clang::AttributedType>(&*q)) {
    if (auto try_null = attr_type->getImmediateNullability();
        try_null.has_value()) {
      switch (try_null.value()) {
        case clang::NullabilityKind::NonNull:
          null = Nullness::Disallowed;
          break;
        case clang::NullabilityKind::Nullable: null = Nullness::Allowed; break;
        case clang::NullabilityKind::NullableResult:
          null = Nullness::Allowed;
          break;
        case clang::NullabilityKind::Unspecified: break;
      }
    }

    // `AttributedType` does not have qualifiers, the type inside does.
    q = attr_type->getEquivalentType();
  }
  return Qualifier()
      .set_const(q.isLocalConstQualified())
      .set_volatile(q.isLocalVolatileQualified())
      .set_nullness(null);
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

std::string name_of_template_parm_type(
    clang::QualType q,
    llvm::ArrayRef<clang::NamedDecl*> template_params_from_context) noexcept {
  sus::check_with_message(
      clang::isa<clang::TemplateTypeParmType>(&*q),
      "not a TemplateTypeParmType, use name_of_type() instead");
  auto* parm = clang::cast<clang::TemplateTypeParmType>(&*q);
  if (parm->getIdentifier()) {
    return name_of_type(q);
  } else {
    sus::check(parm->getDepth() == 0u);
    sus::check(parm->getIndex() < template_params_from_context.size());
    return template_params_from_context[parm->getIndex()]->getNameAsString();
  }
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
  return parm.getDecl() && parm.getDecl()->hasTypeConstraint() &&
         parm.getDecl()->isImplicit();
}

Type build_local_type_internal(
    clang::QualType qualtype, llvm::ArrayRef<clang::NamedDecl*> template_params,
    const clang::SourceManager& sm, clang::Preprocessor& preprocessor,
    clang::SourceLocation loc) noexcept;

TypeOrValue build_template_param(
    const clang::TemplateArgument& arg,
    llvm::ArrayRef<clang::NamedDecl*> template_params,
    const clang::SourceManager& sm, clang::Preprocessor& preprocessor,
    clang::SourceLocation loc) noexcept {
  switch (arg.getKind()) {
    case clang::TemplateArgument::ArgKind::Null:
      arg.dump();
      fmt::println(stderr, "");
      loc.dump(sm);
      sus::unreachable();
    case clang::TemplateArgument::ArgKind::Type:
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Type>(
          build_local_type_internal(arg.getAsType(), template_params, sm,
                                    preprocessor, loc)));
    case clang::TemplateArgument::ArgKind::Declaration:
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Type>(
          build_local_type_internal(arg.getAsDecl()->getType(), template_params,
                                    sm, preprocessor, loc)));
    case clang::TemplateArgument::ArgKind::NullPtr:
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Type>(
          build_local_type_internal(arg.getNullPtrType(), template_params, sm,
                                    preprocessor, loc)));
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
      arg.dump();
      fmt::println(stderr, "");
      loc.dump(sm);
      sus::unreachable();
    case clang::TemplateArgument::ArgKind::Expression:
      return TypeOrValue(TypeOrValueChoice::with<TypeOrValueChoice::Tag::Value>(
          stmt_to_string(*arg.getAsExpr(), sm, preprocessor)));
    case clang::TemplateArgument::ArgKind::Pack:
      // Packs are handled at a higher level since they produce multiple types.
      loc.dump(sm);
      sus::unreachable();
  }
  sus::unreachable();
};

clang::QualType unwrap_skipped_types(clang::QualType q) noexcept {
  // Arrays may already be "DecayedType", but we can get the original type from
  // it.
  if (auto* dtype = clang::dyn_cast<clang::DecayedType>(&*q))
    return unwrap_skipped_types(dtype->getOriginalType());

  // Paren types disapper, we add them back if needed (such as for arrays or
  // function pointers) when constructing a string.
  if (auto* ptype = clang::dyn_cast<clang::ParenType>(&*q))
    return unwrap_skipped_types(ptype->getInnerType());

  // A `using A = B` is an elaborated type that names a typedef A, so unpack
  // the ElaboratedType. Template specializations can be inside an
  // ElaboratedType, so this comes first.
  if (auto* elab = clang::dyn_cast<clang::ElaboratedType>(&*q))
    return unwrap_skipped_types(elab->getNamedType());

  // `AttributedType` have an attribute applied, and should be unwrapped to get
  // to the underlying type.
  if (auto* attr = clang::dyn_cast<clang::AttributedType>(&*q))
    return unwrap_skipped_types(attr->getEquivalentType());

  return q;
}

Type build_local_type_internal(
    clang::QualType qualtype,
    llvm::ArrayRef<clang::NamedDecl*> template_params_from_context,
    const clang::SourceManager& sm, clang::Preprocessor& preprocessor,
    clang::SourceLocation loc) noexcept {
  // PackExpansionTypes wrap a QualType that has all the actual type data we
  // want on it. We just need to remember that it was a pack to add back the
  // `...`.
  bool is_pack = false;
  if (auto* pack_type = clang::dyn_cast<clang::PackExpansionType>(&*qualtype)) {
    qualtype = pack_type->getPattern();
    is_pack = true;
  }

  Refs refs =
      qualtype->isLValueReferenceType()
          ? Refs::LValueRef
          : (qualtype->isRValueReferenceType() ? Refs::RValueRef : Refs::None);
  // Grab the qualifiers on the outer type. Each time we unpack a nested type in
  // the tree, we replace these.
  Qualifier qualifier = qualifier_from_qualtype(qualtype.getNonReferenceType());
  qualtype = unwrap_skipped_types(qualtype.getNonReferenceType());

  sus::Vec<TypeOrValue> nested_names;
  if (auto* dep = clang::dyn_cast<clang::DependentNameType>(&*qualtype)) {
    clang::NestedNameSpecifier* spec = dep->getQualifier();
    while (spec) {
      clang::NestedNameSpecifier::SpecifierKind kind = spec->getKind();
      if (kind == clang::NestedNameSpecifier::Identifier) {
        nested_names.push(
            TypeOrValue(TypeOrValueChoice::with<TypeOrValueTag::Value>(
                std::string(spec->getAsIdentifier()->getName()))));
      } else {
        sus::check(kind == clang::NestedNameSpecifier::TypeSpec ||
                   kind == clang::NestedNameSpecifier::TypeSpecWithTemplate);
        nested_names.push(
            TypeOrValue(TypeOrValueChoice::with<TypeOrValueTag::Type>(
                build_local_type_internal(
                    clang::QualType(spec->getAsType(), 0u),
                    llvm::ArrayRef<clang::NamedDecl*>(), sm, preprocessor,
                    loc))));
      }
      spec = spec->getPrefix();
    }
  }

  sus::Vec<std::string> array_dims;
  sus::Vec<Qualifier> pointers;
  sus::Vec<Qualifier> pointers_to_array;

  // It's possible to have pointers to an array, in which case inside the
  // pointers we find an array. Then we will apply the array to the root type,
  // and come back to look for pointers again, this time they are pointers to
  // the type inside the array, and the original pointers are pointers to the
  // array.
  //
  // While we can have pointer-to-array-of-pointer, what we can't have is
  // array-of-pointer-to-array.
  for (usize i : "0..2"_r) {
    bool was_array = false;

    while (clang::isa<clang::ArrayType>(&*qualtype)) {
      // The type inside a pack can not be an array.
      if (is_pack) {
        qualtype->dump();
        loc.dump(sm);
        sus::unreachable();
      }

      // Arrays come with the var name wrapped in parens, which must be removed.
      qualtype = qualtype.IgnoreParens();

      const clang::ArrayType* const type =
          clang::cast<clang::ArrayType>(&*qualtype);
      if (auto* constarr = clang::dyn_cast<clang::ConstantArrayType>(type)) {
        array_dims.push(
            llvm_int_without_sign_to_string(constarr->getSize(), false));
      }
      if (auto* deparr =
              clang::dyn_cast<clang::DependentSizedArrayType>(type)) {
        array_dims.push(
            stmt_to_string(*deparr->getSizeExpr(), sm, preprocessor));
      }
      if (auto* incarr = clang::dyn_cast<clang::IncompleteArrayType>(type)) {
        array_dims.push("");
      }
      if (auto* vararr = clang::dyn_cast<clang::VariableArrayType>(type)) {
        qualtype->dump();
        loc.dump(sm);
        sus::unreachable();  // This is a C thing, not C++.
      }

      // For arrays the root qualifiers come from the element type.
      qualifier = qualifier_from_qualtype(type->getElementType());
      qualtype = unwrap_skipped_types(type->getElementType());
      was_array = true;
    }

    if (i == 1u && was_array) {
      // The first level of pointers are normally to the root type, unless
      // this is a pointer to an array. We find that out on the second pass,
      // when we see an array inside the pointers, which lands us here. Then
      // those pointers were to the array itself.
      using std::swap;
      swap(pointers_to_array, pointers);
    }

    // The array can be an array of pointers, so we look for pointers after
    // unwrapping the array.
    while (qualtype->isPointerType()) {
      pointers.push(qualifier);
      qualifier = qualifier_from_qualtype(qualtype->getPointeeType());
      qualtype = unwrap_skipped_types(qualtype->getPointeeType());
    }
  }

  sus::Option<sus::Box<Type>> member_pointer_type;
  if (auto* member = clang::dyn_cast<clang::MemberPointerType>(&*qualtype)) {
    member_pointer_type = sus::some(sus::Box<Type>(build_local_type_internal(
        clang::QualType(member->getClass(), 0u), template_params_from_context,
        sm, preprocessor, loc)));

    pointers.push(qualifier);
    qualifier = qualifier_from_qualtype(qualtype->getPointeeType());
    qualtype = unwrap_skipped_types(qualtype->getPointeeType());
  }

  // TODO: Drop the from_range() on the uses of iter_args:
  // https://github.com/chromium/subspace/issues/348
  auto iter_args =
      [](sus::iter::IntoIterator<const clang::TemplateArgument&> auto ii)
      -> sus::iter::Generator<const clang::TemplateArgument&> {
    {
      for (const clang::TemplateArgument& arg : sus::move(ii).into_iter())
        if (arg.getKind() == clang::TemplateArgument::ArgKind::Pack) {
          for (const clang::TemplateArgument& p : arg.pack_elements())
            co_yield p;
        } else {
          co_yield arg;
        }
    }
  };

  // Arrays and pointers aren't templated, but the inner type can be, so we
  // look for this after stripping off references, arrays, and pointers.
  sus::Vec<TypeOrValue> template_params;
  if (auto* ttype =
          clang::dyn_cast<clang::TemplateSpecializationType>(&*qualtype)) {
    for (const clang::TemplateArgument& arg :
         iter_args(sus::iter::from_range(ttype->template_arguments()))) {
      template_params.push(build_template_param(
          arg, template_params_from_context, sm, preprocessor, loc));
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
      for (const clang::TemplateArgument& arg :
           sus::move(it).generate(iter_args)) {
        template_params.push(build_template_param(
            arg, template_params_from_context, sm, preprocessor, loc));
      }
    }
  } else if (auto* auto_type = clang::dyn_cast<clang::AutoType>(&*qualtype)) {
    // This may be a `Concept auto` in a location other than a function
    // parameter. Arguments would be part of that Concept specialization.
    for (const clang::TemplateArgument& arg : iter_args(
             sus::iter::from_range(auto_type->getTypeConstraintArguments()))) {
      template_params.push(build_template_param(
          arg, template_params_from_context, sm, preprocessor, loc));
    }
  } else if (auto* rec_type = clang::dyn_cast<clang::RecordType>(&*qualtype)) {
    if (auto* partial =
            clang::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(
                rec_type->getDecl())) {
      // Partial specialization in another type?
      partial->dump();
      loc.dump(sm);
      sus::unreachable();
    } else if (auto* full =
                   clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(
                       rec_type->getDecl())) {
      // There are both `getTemplateArgs` and `getTemplateInstantiationArgs`,
      // and they both return the same thing in my tests, so what is the
      // difference?
      for (const clang::TemplateArgument& arg : iter_args(
               sus::iter::from_range(full->getTemplateArgs().asArray()))) {
        template_params.push(build_template_param(
            arg, template_params_from_context, sm, preprocessor, loc));
      }
    }
  } else if (auto* inj_type =
                 clang::dyn_cast<clang::InjectedClassNameType>(&*qualtype)) {
    llvm::ArrayRef<clang::NamedDecl*> template_params_from_context_here;
    if (auto* partial =
            clang::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(
                inj_type->getDecl())) {
      // In a partial specialization, any `TemplateTypeParmType` (template
      // arguments) that refer to a template parameter on the class do not have
      // the usual `getDecl` pointer or even `getIdentifier` pointer. They only
      // have a name like "type-parameter-0-0" which is the depth and index.
      //
      // To work backward and get the parameter from the class we need to pass
      // that in to `build_template_param` here.
      //
      // It feels like we should be pushing this array of NamedDecl onto a stack
      // in case there's a `TemplateTypeParmType` with a depth > 0, but it's
      // unclear how to get into that position, as you can't have multiple
      // levels of partial specializations nested.
      template_params_from_context_here =
          partial->getTemplateParameters()->asArray();
    }
    for (const clang::TemplateArgument& arg : iter_args(sus::iter::from_range(
             inj_type->getInjectedTST()->template_arguments()))) {
      template_params.push(build_template_param(
          arg, template_params_from_context_here, sm, preprocessor, loc));
    }
  } else {
    // No template parameters.
    // qualtype->dump();
    // loc.dump(sm);
    // sus::unreachable();
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
  } else if (clang::isa<clang::DependentNameType>(&*qualtype)) {
    // No context.
  } else if (clang::isa<clang::FunctionProtoType>(&*qualtype)) {
    // No context.
  } else if (clang::isa<clang::MemberPointerType>(&*qualtype)) {
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
  } else if (auto* un_using_type =
                 clang::dyn_cast<clang::UnresolvedUsingType>(&*qualtype)) {
    context = un_using_type->getDecl()->getDeclContext();
  } else if (auto* using_type = clang::dyn_cast<clang::UsingType>(&*qualtype)) {
    context = using_type->getFoundDecl()->getDeclContext();
  } else if (auto* injected_type =
                 clang::dyn_cast<clang::InjectedClassNameType>(&*qualtype)) {
    context = injected_type->getDecl()->getDeclContext();
  } else {
    qualtype->dump();
    loc.dump(sm);
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

  sus::Option<sus::Box<Type>> fn_return_type;
  sus::Vec<Type> fn_param_types;
  if (auto* proto = clang::dyn_cast<clang::FunctionProtoType>(&*qualtype)) {
    fn_return_type = sus::some(sus::Box<Type>(build_local_type_internal(
        proto->getReturnType(), template_params_from_context, sm, preprocessor,
        loc)));

    for (clang::QualType p : proto->param_types()) {
      fn_param_types.push(build_local_type_internal(
          p, template_params_from_context, sm, preprocessor, loc));
    }
  }

  auto [name, category] = [&]() -> sus::Tuple<std::string, TypeCategory> {
    if (auto* c = clang::dyn_cast<clang::TemplateTypeParmType>(&*qualtype)) {
      if (template_parameter_is_concept(*c)) {
        // This is a `Concept auto` in a function parameter position.
        return sus::tuple(c->getDecl()
                              ->getTypeConstraint()
                              ->getNamedConcept()
                              ->getNameAsString(),
                          TypeCategory::Concept);
      } else {
        return sus::tuple(
            name_of_template_parm_type(qualtype, template_params_from_context),
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
    } else if (auto* dep_type =
                   clang::dyn_cast<clang::DependentNameType>(&*qualtype)) {
      // The dependent name is not a resolved type, so we call it a
      // TemplateVariable so it's just displayed as text.
      return sus::tuple(std::string(dep_type->getIdentifier()->getName()),
                        TypeCategory::TemplateVariable);
    } else if (auto* proto =
                   clang::dyn_cast<clang::FunctionProtoType>(&*qualtype)) {
      // A function proto is actually a group of types, found in the
      // `fn_return_type` and `fn_param_types`. The root type here is the
      // pointer information if any (for function pointers).
      return sus::tuple("", TypeCategory::FunctionProto);
    } else {
      return sus::tuple(name_of_type(qualtype), TypeCategory::Type);
    }
  }();

  // TODO: Use Vec::reverse().
  namespace_path = sus::move(namespace_path).into_iter().rev().collect_vec();
  record_path = sus::move(record_path).into_iter().rev().collect_vec();
  nested_names = sus::move(nested_names).into_iter().rev().collect_vec();
  pointers = sus::move(pointers).into_iter().rev().collect_vec();

  return Type(category, sus::move(namespace_path), sus::move(record_path),
              sus::move(name), sus::move(nested_names), refs,
              sus::move(qualifier), sus::move(pointers),
              sus::move(pointers_to_array), sus::move(member_pointer_type),
              sus::move(array_dims), sus::move(template_params), is_pack,
              sus::move(fn_return_type), sus::move(fn_param_types));
}

}  // namespace

Type build_local_type(clang::QualType qualtype, const clang::SourceManager& sm,
                      clang::Preprocessor& preprocessor,
                      clang::SourceLocation loc) noexcept {
  return build_local_type_internal(
      qualtype, llvm::ArrayRef<clang::NamedDecl*>(), sm, preprocessor, loc);
}

/// Writes the pointers and returns if it ended with a qualifer.
bool write_pointers(bool punctuation_last,
                    sus::iter::Iterator<const Qualifier&> auto pointers,
                    sus::fn::DynFnMut<void(std::string_view)>& text_fn,
                    sus::fn::DynFnMut<void()>& const_qualifier_fn,
                    sus::fn::DynFnMut<void()>& volatile_qualifier_fn) noexcept {
  bool wrote_quals = false;
  for (const Qualifier& q : pointers) {
    // If there are quals on either side of the `*` put a space to the left
    // of the `*.
    //
    // wrote_quals gives: *const[space here]*
    // has_quals gives: *[space here]*const
    //
    // Except when we just wrote a puntuation, like (*const f) or S::*const,
    // then we don't need aspace to the left of the first pointer, as there's
    // no variable name there.
    bool has_quals = (q.is_const || q.is_volatile) && !punctuation_last;
    if (wrote_quals || has_quals) text_fn(" ");
    text_fn("*");
    wrote_quals = false;
    punctuation_last = false;

    if (q.is_const) {
      if (wrote_quals) text_fn(" ");
      wrote_quals = true;
      const_qualifier_fn();
    }
    if (q.is_volatile) {
      if (wrote_quals) text_fn(" ");
      wrote_quals = true;
      volatile_qualifier_fn();
    }
  }
  return wrote_quals;
}

namespace {

void type_to_string_internal(
    const Type& type, sus::fn::DynFnMut<void(std::string_view)>& text_fn,
    sus::fn::DynFnMut<void(TypeToStringQuery)>& type_fn,
    sus::fn::DynFnMut<void()>& const_qualifier_fn,
    sus::fn::DynFnMut<void()>& volatile_qualifier_fn,
    sus::Option<sus::fn::DynFnMut<void()>&> var_name_fn) noexcept {
  if (type.category == TypeCategory::FunctionProto) {
    // For function proto lead with the return type.
    type_to_string_internal(**type.fn_return_type, text_fn, type_fn,
                            const_qualifier_fn, volatile_qualifier_fn,
                            sus::none());
  }

  if (type.qualifier.is_const) {
    const_qualifier_fn();
    text_fn(" ");
  }
  if (type.qualifier.is_volatile) {
    volatile_qualifier_fn();
    text_fn(" ");
  }

  for (const TypeOrValue& tv : type.nested_names) {
    switch (tv.choice) {
      case TypeOrValueTag::Type: {
        type_to_string_internal(tv.choice.as<TypeOrValueTag::Type>(), text_fn,
                                type_fn, const_qualifier_fn,
                                volatile_qualifier_fn, sus::none());
        break;
      }
      case TypeOrValueTag::Value:
        text_fn(tv.choice.as<TypeOrValueTag::Value>());
        break;
    }
    text_fn("::");
  }

  switch (type.category) {
    case TypeCategory::Concept: [[fallthrough]];
    case TypeCategory::Type:
      type_fn(TypeToStringQuery{
          .namespace_path = type.namespace_path.as_slice(),
          .record_path = type.record_path.as_slice(),
          .name = std::string_view(type.name),
      });
      break;
    case TypeCategory::FunctionProto:
      // The FunctionProto types (return, params) are recursed on elsewhere.
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

  bool wrote_var_open_paren = false;
  bool punctuation_last = false;
  if (type.category == TypeCategory::FunctionProto) {
    if (!type.pointers.is_empty() || var_name_fn.is_some()) {
      text_fn(" (");  // Closed after the variable name.
      wrote_var_open_paren = true;
      punctuation_last = true;
    }
  }

  if (type.member_pointer_type.is_some()) {
    if (!punctuation_last) text_fn(" ");
    type_to_string_internal(**type.member_pointer_type, text_fn, type_fn,
                            const_qualifier_fn, volatile_qualifier_fn,
                            sus::none());
    text_fn("::");  // Closed after the variable name.
    punctuation_last = true;
  }

  bool ended_with_qual =
      write_pointers(punctuation_last, type.pointers.iter(), text_fn,
                     const_qualifier_fn, volatile_qualifier_fn);

  if (type.array_dims.is_empty()) {
    switch (type.refs) {
      case Refs::None: break;
      case Refs::LValueRef: text_fn("&"); break;
      case Refs::RValueRef: text_fn("&&"); break;
    }
    if (type.is_pack) text_fn("...");
    if (var_name_fn.is_some()) {
      // If we're in a function proto then we don't need a space
      // before the var name, there's a `()` around it. But if there
      // were quals then we do for separation.
      if (ended_with_qual || !wrote_var_open_paren) text_fn(" ");
      var_name_fn.take().unwrap()();
    }
  } else {
    sus::check(!type.is_pack);

    if (type.refs != Refs::None || !type.pointers_to_array.is_empty()) {
      text_fn(" (");

      write_pointers(/* punctuation_last=*/true, type.pointers_to_array.iter(),
                     text_fn, const_qualifier_fn, volatile_qualifier_fn);

      switch (type.refs) {
        case Refs::None: break;
        case Refs::LValueRef: text_fn("&"); break;
        case Refs::RValueRef: text_fn("&&"); break;
      }
      if (var_name_fn.is_some()) {
        var_name_fn.take().unwrap()();
      }

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

  if (wrote_var_open_paren) text_fn(")");

  if (type.category == TypeCategory::FunctionProto) {
    text_fn("(");
    for (const auto& [i, arg] : type.fn_param_types.iter().enumerate()) {
      if (i > 0u) text_fn(", ");
      type_to_string_internal(arg, text_fn, type_fn, const_qualifier_fn,
                              volatile_qualifier_fn, sus::none());
    }
    text_fn(")");
  }
}

/// This function walks the types in the same order as
/// `type_to_string_internal`.
///
/// Every call to `type_walk_types_internal` and `type_to_string_internal` must
/// happen in the same order.
void type_walk_types_internal(
    const Type& type,
    sus::fn::DynFnMut<void(TypeToStringQuery)>& type_fn) noexcept {
  if (type.category == TypeCategory::FunctionProto)
    type_walk_types_internal(**type.fn_return_type, type_fn);

  for (const TypeOrValue& tv : type.nested_names) {
    switch (tv.choice) {
      case TypeOrValueTag::Type: {
        type_walk_types_internal(tv.choice.as<TypeOrValueTag::Type>(), type_fn);
        break;
      }
      case TypeOrValueTag::Value: break;
    }
  }

  switch (type.category) {
    case TypeCategory::Concept: [[fallthrough]];
    case TypeCategory::Type:
      type_fn(TypeToStringQuery{
          .namespace_path = type.namespace_path.as_slice(),
          .record_path = type.record_path.as_slice(),
          .name = std::string_view(type.name),
      });
      break;
    case TypeCategory::FunctionProto: break;
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

  if (type.member_pointer_type.is_some()) {
    type_walk_types_internal(**type.member_pointer_type, type_fn);
  }

  if (type.category == TypeCategory::FunctionProto) {
    for (const Type& arg : type.fn_param_types)
      type_walk_types_internal(arg, type_fn);
  }
}

}  // namespace

void type_to_string(
    const Type& type, sus::fn::DynFnMut<void(std::string_view)>& text_fn,
    sus::fn::DynFnMut<void(TypeToStringQuery)>& type_fn,
    sus::fn::DynFnMut<void()>& const_qualifier_fn,
    sus::fn::DynFnMut<void()>& volatile_qualifier_fn,
    sus::Option<sus::fn::DynFnMut<void()>&> var_name_fn) noexcept {
  return type_to_string_internal(type, text_fn, type_fn, const_qualifier_fn,
                                 volatile_qualifier_fn, sus::move(var_name_fn));
}

void type_walk_types(
    const Type& type,
    sus::fn::DynFnMut<void(TypeToStringQuery)>& type_fn) noexcept {
  return type_walk_types_internal(type, type_fn);
}

}  // namespace subdoc
