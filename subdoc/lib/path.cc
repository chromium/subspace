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

#include "subdoc/lib/path.h"

namespace subdoc {

bool path_contains_namespace(clang::Decl* decl, Namespace n) noexcept {
  clang::NamespaceDecl* ndecl = find_nearest_namespace(decl);

  while (ndecl) {
    switch (n) {
      case Namespace::Tag::Global: return true;
      case Namespace::Tag::Anonymous:
        if (ndecl->isAnonymousNamespace()) return true;
        break;
      case Namespace::Tag::Named:
        const auto& name = n.as<Namespace::Tag::Named>();
        if (ndecl->getNameAsString() == name) return true;
        break;
    }
    ndecl = clang::dyn_cast<clang::NamespaceDecl>(ndecl->getParent());
  }
  return false;
}

bool path_is_private(clang::NamedDecl* decl) noexcept {
  clang::Linkage linkage = decl->getLinkageInternal();
  if (linkage != clang::Linkage::ModuleLinkage &&
      linkage != clang::Linkage::ExternalLinkage) {
    // NoLinkage describes itself as "can only be referred to from within its
    // scope".
    //
    // However `namespace a { using b::S; }` brings S into `a` in a way that is
    // usable publicly from other scopes. So we accept `NoLinkage` for
    // `UsingDecl` and `UsingEnumDecl` (aka `BaseUsingDecl`). Similar for
    // type aliases.
    if (!(linkage == clang::Linkage::NoLinkage &&
          (clang::isa<clang::BaseUsingDecl>(decl) ||
           clang::isa<clang::TypedefNameDecl>(decl)))) {
      return true;
    }
  }

  // Private members are not shown, protected members either. If they become
  // public in a subclass they would be shown there.
  if (decl->getAccess() == clang::AccessSpecifier::AS_private ||
      decl->getAccess() == clang::AccessSpecifier::AS_protected) {
    return true;
  }

  // Look at parent scopes for private access as well.
  clang::DeclContext* cx = decl->getDeclContext();
  while (cx) {
    if (auto* tdecl = clang::dyn_cast<clang::TagDecl>(cx)) {
      // TODO: getAccess() can assert if it's not determined yet due to
      // template instantiation being incomplete..? clang-doc uses
      // getAccessUnsafe() which can give the wrong answer.
      if (tdecl->getAccess() == clang::AccessSpecifier::AS_private ||
          tdecl->getAccess() == clang::AccessSpecifier::AS_protected) {
        return true;
      }
    }

    cx = cx->getParent();
  }
  return false;
}

}  // namespace subdoc
