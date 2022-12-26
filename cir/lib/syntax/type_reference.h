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

#include "cir/lib/source_span.h"
#include "cir/lib/syntax/declared_type.h"
#include "cir/lib/syntax/type_reference_annotations.h"
#include "cir/llvm.h"
#include "subspace/prelude.h"
#include "subspace/union/union.h"

namespace cir::syntax {

enum class BuiltInTypes {
  Nullptr,
  Bool,
  Char,  // TODO: Figure out if char is signed and remove this option?
  UChar,
  SChar,
  Char16,
  Char32,
  Short,
  UShort,
  Int,
  Uint,
  Long,
  ULong,
  LongLong,
  ULongLong,
  Float,
  Double,
  LongDouble,
  Pointer,
  /// A pointer to a function or method.
  ///
  /// These include method pointers, which just include the class in their name.
  FnPointer,
};

enum class TypeRefKindTag {
  Builtin,
  Declared,
};

// clang-format off
using TypeRefKind = sus::Union<sus_value_types(
    (TypeRefKindTag::Builtin, BuiltInTypes),
    (TypeRefKindTag::Declared, DeclaredType&),
)>;
// clang-format on

struct TypeReference {
  TypeRefKind kind;
  TypeReferenceAnnotations annotations;
  SourceSpan span;
};

}  // namespace cir::syntax
