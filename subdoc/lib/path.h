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
#include "subspace/choice/choice.h"
#include "subspace/containers/vec.h"
#include "subspace/prelude.h"

namespace subdoc {

enum class NamespaceType {
  Anonymous,
  Named,
};

// clang-format off
using Namespace = sus::Choice<sus_choice_types(
    (NamespaceType::Anonymous, void),
    (NamespaceType::Named, std::string)
)>;
// clang-format on

inline clang::NamespaceDecl* find_nearest_namespace(
    clang::Decl* decl) noexcept {
  clang::DeclContext* cx = decl->getDeclContext();
  while (cx) {
    if (auto* ndecl = clang::dyn_cast<clang::NamespaceDecl>(cx)) return ndecl;
    cx = cx->getParent();
  }
  return nullptr;
}

inline sus::Vec<Namespace> collect_namespace_path(clang::Decl* decl) noexcept {
  clang::NamespaceDecl* ndecl = find_nearest_namespace(decl);

  auto v = sus::Vec<Namespace>();
  while (ndecl) {
    if (ndecl->isAnonymousNamespace())
      v.push(sus::choice<NamespaceType::Anonymous>());
    else
      v.push(sus::choice<NamespaceType::Named>(ndecl->getNameAsString()));
    ndecl = clang::dyn_cast<clang::NamespaceDecl>(ndecl->getParent());
  }
  return v;
}

inline bool path_contains_namespace(clang::Decl* decl, Namespace n) noexcept {
  clang::NamespaceDecl* ndecl = find_nearest_namespace(decl);

  while (ndecl) {
    switch (n) {
      case Namespace::Tag::Anonymous:
        if (ndecl->isAnonymousNamespace()) return true;
        break;
      case Namespace::Tag::Named:
        const auto& name = n.get_ref<Namespace::Tag::Named>();
        if (ndecl->getNameAsString() == name) return true;
        break;
    }
    ndecl = clang::dyn_cast<clang::NamespaceDecl>(ndecl->getParent());
  }
  return false;
}

inline bool path_is_private(clang::NamedDecl* decl) noexcept {
  clang::Linkage linkage = decl->getLinkageInternal();
  if (linkage != clang::Linkage::ModuleLinkage &&
      linkage != clang::Linkage::ExternalLinkage) {
    return true;
  }

  clang::DeclContext* cx = decl->getDeclContext();
  while (cx) {
    if (auto* tdecl = clang::dyn_cast<clang::TagDecl>(cx)) {
      // TODO: getAccess() can assert if it's not determined yet due to template
      // instatiation being incomplete..? clang-doc uses getAccessUnsafe() which
      // can give the wrong answer.
      clang::AccessSpecifier access = tdecl->getAccess();
      if (access == clang::AccessSpecifier::AS_private) {
        return true;
      }
    }

    cx = cx->getParent();
  }
  return false;
}

}  // namespace subdoc
