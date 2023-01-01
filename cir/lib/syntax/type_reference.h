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

#pragma once

#include <sstream>
#include <string>

#include "cir/lib/source_span.h"
#include "cir/lib/syntax/declared_type.h"
#include "cir/lib/syntax/function_id.h"
#include "cir/lib/syntax/object_annotations.h"
#include "cir/lib/syntax/pointer_annotations.h"
#include "cir/llvm.h"
#include "subspace/choice/choice.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

using sus::option::Option;

namespace cir::syntax {

enum class BuiltinType {
  Nullptr,
  Bool,
  Char,
  UChar,
  WideChar,
  UWideChar,
  Char8,
  Char16,
  Char32,
  Short,
  UShort,
  Int,
  UInt,
  Long,
  ULong,
  LongLong,
  ULongLong,
  Int128,
  UInt128,
  Float,
  Double,
  LongDouble,
  ObjCId,
};

inline Option<BuiltinType> builtin_type(const clang::QualType& q) noexcept {
  auto* b = clang::dyn_cast<clang::BuiltinType>(&*q.getCanonicalType());
  if (b == nullptr) return sus::none();
  switch (b->getKind()) {
    case clang::BuiltinType::NullPtr: return sus::some(BuiltinType::Nullptr);
    case clang::BuiltinType::Bool: return sus::some(BuiltinType::Bool);
    case clang::BuiltinType::Char_U: return sus::some(BuiltinType::UChar);
    case clang::BuiltinType::UChar: return sus::some(BuiltinType::UChar);
    case clang::BuiltinType::WChar_U: return sus::some(BuiltinType::UWideChar);
    case clang::BuiltinType::Char8: return sus::some(BuiltinType::Char8);
    case clang::BuiltinType::Char16: return sus::some(BuiltinType::Char16);
    case clang::BuiltinType::Char32: return sus::some(BuiltinType::Char32);
    case clang::BuiltinType::Char_S: return sus::some(BuiltinType::Char);
    case clang::BuiltinType::SChar: return sus::some(BuiltinType::Char);
    case clang::BuiltinType::WChar_S: return sus::some(BuiltinType::WideChar);
    case clang::BuiltinType::Short: return sus::some(BuiltinType::Short);
    case clang::BuiltinType::UShort: return sus::some(BuiltinType::UShort);
    case clang::BuiltinType::Int: return sus::some(BuiltinType::Int);
    case clang::BuiltinType::UInt: return sus::some(BuiltinType::UInt);
    case clang::BuiltinType::Long: return sus::some(BuiltinType::Long);
    case clang::BuiltinType::ULong: return sus::some(BuiltinType::ULong);
    case clang::BuiltinType::LongLong: return sus::some(BuiltinType::LongLong);
    case clang::BuiltinType::ULongLong:
      return sus::some(BuiltinType::ULongLong);
    case clang::BuiltinType::Int128: return sus::some(BuiltinType::Int128);
    case clang::BuiltinType::UInt128: return sus::some(BuiltinType::UInt128);
    case clang::BuiltinType::Float: return sus::some(BuiltinType::Float);
    case clang::BuiltinType::Double: return sus::some(BuiltinType::Double);
    case clang::BuiltinType::LongDouble:
      return sus::some(BuiltinType::LongDouble);
    case clang::BuiltinType::ObjCId: return sus::some(BuiltinType::ObjCId);
    default: return sus::none();
  }
}

inline std::string builtin_type_to_string(BuiltinType b) noexcept {
  switch (b) {
    case BuiltinType::Nullptr: return "nullptr_t";
    case BuiltinType::Bool: return "bool";
    case BuiltinType::Char: return "signed char";
    case BuiltinType::UChar: return "unsigned char";
    case BuiltinType::WideChar: return "signed wchar_t";
    case BuiltinType::UWideChar: return "unsigned wchar_t";
    case BuiltinType::Char8: return "char8_t";
    case BuiltinType::Char16: return "char16_t";
    case BuiltinType::Char32: return "char32_t";
    case BuiltinType::Short: return "signed short";
    case BuiltinType::UShort: return "unsigned short";
    case BuiltinType::Int: return "signed int";
    case BuiltinType::UInt: return "unsigned int";
    case BuiltinType::Long: return "signed long";
    case BuiltinType::ULong: return "unsigned long";
    case BuiltinType::LongLong: return "signed long long";
    case BuiltinType::ULongLong: return "unsigned long long";
    case BuiltinType::Int128: return "int128_t";
    case BuiltinType::UInt128: return "unsigned int128_t";
    case BuiltinType::Float: return "float";
    case BuiltinType::Double: return "double";
    case BuiltinType::LongDouble: return "long double";
    case BuiltinType::ObjCId: return "ObjcID";
  }
  sus::unreachable();
}

enum class TypeRefKindTag {
  Builtin,
  Declared,
  Pointer,
  /// A pointer to a function or method.
  ///
  /// These include method pointers, which just include the class in their name.
  FnPointer,
};

// clang-format off
using TypeRefKind = sus::Choice<sus_choice_types(
    (TypeRefKindTag::Builtin, BuiltinType, ObjectAnnotations),
    (TypeRefKindTag::Declared, DeclaredType, ObjectAnnotations),
    (TypeRefKindTag::Pointer, PointerAnnotations, ObjectAnnotations), // TODO: Store the pointee(s) types.
    (TypeRefKindTag::FnPointer, FunctionId, ObjectAnnotations),
)>;
// clang-format on

struct TypeReference {
  static TypeReference with_return_type(clang::QualType q, bool nullable,
                                        SourceSpan span) {
    TypeRefKind kind = [&]() {
      Option<BuiltinType> b = builtin_type(q);
      if (b.is_some()) {
        return TypeRefKind::with<TypeRefKind::Tag::Builtin>(
            sus::Tuple<BuiltinType, ObjectAnnotations>::with(
                sus::move(b).unwrap(), ObjectAnnotations{
                                           .is_const = q.isConstQualified(),
                                       }));
      }

      if (q->isPointerType() || q->isReferenceType() ||
          q->isMemberDataPointerType())
        return TypeRefKind::with<TypeRefKind::Tag::Pointer>(
            sus::Tuple<PointerAnnotations, ObjectAnnotations>::with(
                PointerAnnotations{
                    .is_const = q->getPointeeType().isConstQualified(),
                    .is_nullable = nullable && !q->isReferenceType(),
                    // TODO: lifetimes
                },
                ObjectAnnotations{
                    .is_const = q.isConstQualified(),
                }));

      if (q->isFunctionPointerType() || q->isFunctionReferenceType() ||
          q->isMemberFunctionPointerType() || q->isReferenceType())
        return TypeRefKind::with<TypeRefKind::Tag::FnPointer>(
            sus::Tuple<FunctionId, ObjectAnnotations>::with(
                // TODO: get the ID from the clang::FunctionDecl + cir::Output.
                FunctionId(0_u32), ObjectAnnotations{
                                       .is_const = q.isConstQualified(),
                                   }));

      return TypeRefKind::with<TypeRefKind::Tag::Declared>(
          sus::Tuple<DeclaredType, ObjectAnnotations>::with(
              DeclaredType::with_qual_type(q, span),
              ObjectAnnotations{
                  .is_const = q.isConstQualified(),
              }));
    }();

    return TypeReference{
        .kind = sus::move(kind),
        .span = sus::move(span),
    };
  }

  TypeRefKind kind;
  SourceSpan span;
};

}  // namespace cir::syntax

namespace cir {

struct Output;

std::string to_string(const syntax::TypeReference& typeref,
                      const Output& output) noexcept;

}  // namespace cir
