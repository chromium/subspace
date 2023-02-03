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
#include "subspace/assertions/unreachable.h"
#include "subspace/prelude.h"

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

inline std::string friendly_type_name(const clang::QualType& type) noexcept {
  clang::QualType unqualified = type.getUnqualifiedType();
  // Clang writes booleans as "_Bool".
  if (unqualified->isBooleanType()) return "bool";
  std::string full = unqualified.getAsString();

  // Type path component other than the last show up like `struct S` instead of
  // just `S`. So we're dropping those from each component.
  sus::Vec<std::string> split = [&]() {
    sus::Vec<std::string> v;
    // TODO: Add a String type to subspace, give it String::split().
    while (true) {
      size_t pos = full.find_first_of("::");
      if (pos == std::string::npos) {
        v.push(sus::move(full));
        break;
      }
      v.push(full.substr(0, pos));
      full = full.substr(pos + strlen("::"));
    }
    for (std::string& s : v.iter_mut()) {
      if (s.starts_with("struct "))
        s = s.substr(strlen("struct "));
      else if (s.starts_with("class "))
        s = s.substr(strlen("class "));
      else if (s.starts_with("union "))
        s = s.substr(strlen("union "));
      else if (s.starts_with("enum "))
        s = s.substr(strlen("enum "));
    }
    return v;
  }();

  std::ostringstream s;
  if (type->isEnumeralType()) {
    s << "enum ";
  } else if (type->isStructureType()) {
    s << "struct ";
  } else if (type->isClassType()) {
    s << "class ";
  } else if (type->isUnionType()) {
    s << "union ";
  } else if (type->isTypedefNameType()) {
    s << "typedef ";
  }
  bool add_colons = false;
  for (std::string&& component : sus::move(split).into_iter()) {
    if (add_colons) s << "::";
    s << sus::move(component);
    add_colons = true;
  }
  return sus::move(s).str();
}

inline std::string friendly_short_type_name(
    const clang::QualType& type) noexcept {
  clang::QualType unqualified = type.getUnqualifiedType();
  // Clang writes booleans as "_Bool".
  if (unqualified->isBooleanType()) return "bool";
  std::string full = unqualified.getAsString();
  // TODO: This does the wrong things with templates! But it tries.
  // `S::T<int>::V<Z::X, Y::A>` should come out as `V<Z::X, Y::A>`.
  //
  // TODO: Drop the namespaces if the type and function it's related to (not
  // given as an input here yet) have the same top-level namespace.
  auto pos = std::string::npos;
  while (true) {
    if (auto close_pos = full.rfind(">", pos); close_pos != std::string::npos) {
      pos = close_pos - 1u;
      continue;
    }
    if (auto open_pos = full.rfind("<", pos); open_pos != std::string::npos) {
      pos = open_pos - 1u;
      continue;
    }
    break;
  }
  pos = full.rfind("::", pos);
  if (pos != std::string::npos) {
    pos += strlen("::");
    full = full.substr(pos);
  }
  return full;
}

inline std::string friendly_record_type_name(RecordType t,
                                             bool capitalize) noexcept {
  switch (t) {
    case RecordType::Class: return capitalize ? "Class" : "class";
    case RecordType::Struct: return capitalize ? "Struct" : "struct";
    case RecordType::Union: return capitalize ? "Union" : "union";
  }
  sus::unreachable_unchecked(unsafe_fn);
}

}  // namespace subdoc
