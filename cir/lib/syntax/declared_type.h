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
#include "cir/llvm.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/prelude.h"
#include "subspace/union/union.h"

namespace cir::syntax {

enum class DeclaredTypeDetailTag {
  Enum,
  Class,
  Union,
};

// clang-format off
using DeclaredTypeDetail = sus::Union<sus_value_types(
    (DeclaredTypeDetailTag::Enum, clang::TagDecl&),
    (DeclaredTypeDetailTag::Class, clang::CXXRecordDecl&),
    (DeclaredTypeDetailTag::Union, clang::CXXRecordDecl&)
)>;
// clang-format on

struct DeclaredType {
  static DeclaredType with_qual_type(clang::QualType q,
                                     SourceSpan span) noexcept {
    if (q->isEnumeralType()) {
      return DeclaredType{
          .detail = DeclaredTypeDetail::with<DeclaredTypeDetail::Tag::Enum>(
              *q->getAsTagDecl()),
          .span = span,
      };
    }
    if (q->isClassType()) {
      return DeclaredType{
          .detail = DeclaredTypeDetail::with<DeclaredTypeDetail::Tag::Class>(
              *q->getAsCXXRecordDecl()),
          .span = span,
      };
    }
    if (q->isUnionType()) {
      return DeclaredType{
          .detail = DeclaredTypeDetail::with<DeclaredTypeDetail::Tag::Union>(
              *q->getAsCXXRecordDecl()),
          .span = span,
      };
    }
    sus::unreachable();
  }

  DeclaredTypeDetail detail;
  SourceSpan span;
};

}  // namespace cir::syntax
