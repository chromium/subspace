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

#pragma once

#include <sstream>

#include "subdoc/llvm.h"
#include "subspace/assertions/check.h"
#include "subspace/choice/choice.h"
#include "subspace/containers/vec.h"
#include "subspace/prelude.h"

namespace subdoc {

enum class Qualifier {
  None,
  Const,
  Volatile,
  ConstVolatile,
};

enum class Refs {
  None,
  LValueRef,
  RValueRef,
};

struct TypeOrValue;

enum class SizeOrStringTag {
  Size,
  String,
};

using SizeOrString = sus::Choice<sus_choice_types(
    (SizeOrStringTag::Size, usize), (SizeOrStringTag::String, std::string))>;

struct Type {
  std::string name;
  /// Refs can only appear on the outermost type.
  Refs refs;
  /// The first element is the innermost pointee.
  ///
  /// `const<0th T *const<1st *const<2nd *const<3rd`.
  ///
  /// There will be 1 element if the type is not a pointer.
  sus::Vec<Qualifier> quals;
  /// The dimension of each level of an array, if any. If the size is
  /// not a number, then we represent it as a string.
  sus::Vec<std::string> array_dims;
  /// Recursive structure, each template param is another type, or value.
  sus::Vec<TypeOrValue> template_params;
};

enum class TypeOrValueTag {
  Type,
  Value,
};

using TypeOrValueChoice = sus::Choice<sus_choice_types(
    (TypeOrValueTag::Type, Type),  //
    (TypeOrValueTag::Value, Type /* a primitive or enum */,
     std::string /* value as text*/))>;

struct TypeOrValue {
  TypeOrValueChoice choice;
};

namespace __private {

inline Qualifier get_qualifiers(clang::QualType qualtype) noexcept {
  return qualtype.isLocalConstQualified()
             ? (qualtype.isLocalVolatileQualified() ? Qualifier::ConstVolatile
                                                    : Qualifier::Const)
             : (qualtype.isLocalVolatileQualified() ? Qualifier::Volatile
                                                    : Qualifier::None);
}

inline const clang::QualType find_pointee_type(
    const clang::SourceManager& sm, clang::QualType qualtype,
    sus::Vec<Qualifier>& ptr_quals, sus::Vec<std::string>& dims) noexcept {
  if (qualtype->isPointerType()) {
    const clang::QualType pointee =
        find_pointee_type(sm, qualtype->getPointeeType(), ptr_quals, dims);
    ptr_quals.push(get_qualifiers(qualtype));
    return pointee;
  } else if (const auto* array_type =
                 clang::dyn_cast<clang::ArrayType>(qualtype.getTypePtr())) {
    auto dim = [&]() -> std::string {
      if (const auto* con =
              clang::dyn_cast<clang::ConstantArrayType>(array_type)) {
        std::ostringstream stream;
        stream << con->getSize().getLimitedValue();
        return sus::move(stream).str();
      }
      if (const auto* dep =
              clang::dyn_cast<clang::DependentSizedArrayType>(array_type)) {
        std::string s = dep->getBracketsRange().printToString(sm);
        sus::check(s.size() > 2u);  // The string includes the `[]`.
        return sus::move(s).substr(1u, s.size() - 2u);
      }
      if (const auto* inc =
              clang::dyn_cast<clang::IncompleteArrayType>(array_type)) {
        return std::string();
      }
      if (const auto* var =
              clang::dyn_cast<clang::VariableArrayType>(array_type)) {
        std::string s = var->getBracketsRange().printToString(sm);
        sus::check(s.size() > 2u);  // The string includes the `[]`.
        return sus::move(s).substr(1u, s.size() - 2u);
      }
      // This would imply Clang added a ne ArrayType subclass.
      sus::unreachable();
    }();
    dims.push(sus::move(dim));
    return find_pointee_type(sm, array_type->getElementType(), ptr_quals, dims);
  } else {
    ptr_quals.push(get_qualifiers(qualtype));
    return qualtype;
  }
}

inline std::string get_type_name(const clang::QualType& type) noexcept {
  // Clang writes booleans as "_Bool".
  if (type->isBooleanType()) return "bool";
  return type.getAsString();
}

}  // namespace __private

/// Builds a Type structure from `qual` without looking through type aliases.
inline Type build_local_type(const clang::SourceManager& sm,
                             clang::QualType qualtype) noexcept {
  auto refs =
      qualtype->isLValueReferenceType()
          ? Refs::LValueRef
          : (qualtype->isRValueReferenceType() ? Refs::RValueRef : Refs::None);
  sus::Vec<Qualifier> quals;
  sus::Vec<std::string> array_dims;
  const clang::QualType pointee =
      __private::find_pointee_type(sm, qualtype, mref(quals), mref(array_dims));
  sus::Vec<TypeOrValue> template_params;
  if (auto* template_type =
          clang::dyn_cast<clang::TemplateSpecializationType>(&*pointee)) {
    (void)template_type;
  }

  // TODO: We need to remove the template params and namespaces and store the
  // namespaces somehow.
  std::string name = __private::get_type_name(pointee);

  return Type(sus::move(name), refs, sus::move(quals), sus::move(array_dims),
              sus::move(template_params));
}

}  // namespace subdoc
