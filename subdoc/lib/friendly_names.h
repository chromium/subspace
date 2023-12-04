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
#include <string>

#include "subdoc/lib/method_qualifier.h"
#include "subdoc/lib/record_type.h"
#include "subdoc/llvm.h"
#include "sus/assertions/unreachable.h"
#include "sus/prelude.h"

namespace subdoc {

inline std::string friendly_function_name(clang::FunctionDecl& decl) noexcept {
  std::ostringstream s;
  if (auto* mdecl = clang::dyn_cast<clang::CXXMethodDecl>(&decl)) {
    s << mdecl->getThisType().getCanonicalType().getAsString();
    s << "::";
  }
  s << decl.getQualifiedNameAsString();
  s << "(";
  bool comma = false;
  for (auto* p : decl.parameters()) {
    if (comma) s << ", ";
    s << p->getOriginalType().getCanonicalType().getAsString();
    comma = true;
  }
  s << ") > ";
  s << decl.getReturnType().getCanonicalType().getAsString();
  clang::Expr* req = decl.getTrailingRequiresClause();
  if (req) {
    s << " requires ";
    // TODO: dump it.
  }

  if (auto* mdecl = clang::dyn_cast<clang::CXXMethodDecl>(&decl)) {
    if (mdecl->getThisType().isConstQualified()) {
      s << " const";
    }
    if (mdecl->getThisType()->isRValueReferenceType()) {
      s << " &&";
    } else if (mdecl->getThisType()->isLValueReferenceType()) {
      s << " &";
    }
  }
  return s.str();
}

inline std::string friendly_record_type_name(RecordType t,
                                             bool capitalize) noexcept {
  switch (t) {
    case RecordType::Class: return capitalize ? "Class" : "class";
    case RecordType::Struct: return capitalize ? "Struct" : "struct";
    case RecordType::Union: return capitalize ? "Union" : "union";
  }
  sus_unreachable_unchecked(unsafe_fn);
}

}  // namespace subdoc
