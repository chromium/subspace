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
#include "subspace/iter/iterator.h"
#include "subspace/prelude.h"

namespace subdoc {

enum class NamespaceType {
  Global,
  Anonymous,
  Named,
};

// clang-format off
using Namespace = sus::Choice<sus_choice_types(
    (NamespaceType::Global, void),
    (NamespaceType::Anonymous, void),
    (NamespaceType::Named, std::string)
)>;
// clang-format on

inline std::string namespace_with_path_to_string(
    sus::Slice<const Namespace> path, const Namespace& tail) noexcept {
  std::ostringstream s;
  bool add_colons = false;

  for (const Namespace& n : path.iter().rev()) {
    if (add_colons) s << "::";
    switch (n) {
      case Namespace::Tag::Global: break;
      case Namespace::Tag::Anonymous:
        s << "(anonymous)";
        add_colons = true;
        break;
      case Namespace::Tag::Named:
        s << n.as<Namespace::Tag::Named>();
        add_colons = true;
        break;
    }
  }

  if (add_colons) s << "::";
  switch (tail) {
    case Namespace::Tag::Global: s << "Global namespace"; break;
    case Namespace::Tag::Anonymous: s << "(anonymous)"; break;
    case Namespace::Tag::Named: s << tail.as<Namespace::Tag::Named>(); break;
  }

  return s.str();
}

inline clang::NamespaceDecl* find_nearest_namespace(
    clang::Decl* decl) noexcept {
  if (auto* ndecl = clang::dyn_cast<clang::NamespaceDecl>(decl)) return ndecl;

  clang::DeclContext* cx = decl->getDeclContext();
  while (cx) {
    if (auto* ndecl = clang::dyn_cast<clang::NamespaceDecl>(cx)) return ndecl;
    cx = cx->getParent();
  }
  return nullptr;
}

namespace __private {

class RecordIter final
    : public sus::iter::IteratorBase<RecordIter, std::string_view> {
 public:
  static RecordIter with(clang::RecordDecl* decl) noexcept {
    return RecordIter(decl);
  }

  sus::Option<std::string_view> next() noexcept {
    if (next_decl_) {
      clang::RecordDecl* cur_decl = next_decl_;

      clang::DeclContext* cx = cur_decl->getDeclContext();
      if (clang::isa<clang::RecordDecl>(cx)) {
        next_decl_ = clang::dyn_cast<clang::RecordDecl>(cx);
      } else {
        next_decl_ = nullptr;
      }
      return sus::some(std::string_view(cur_decl->getName()));
    } else {
      return sus::none();
    }
  }

 protected:
  RecordIter(clang::RecordDecl* decl) noexcept : next_decl_(decl) {}

 private:
  clang::RecordDecl* next_decl_;

  sus_class_trivially_relocatable(unsafe_fn, decltype(next_decl_));
};

}  // namespace __private

/// Returns an iterator over the record `decl` and any records it is nested
/// within, ordered from inside to outside.
///
/// The iterator returns std::string_view references to string in the clang
/// AST, which are valid as long as the RecordDecl's pointee is valid.
inline auto iter_record_path(clang::RecordDecl* decl) {
  return __private::RecordIter::with(decl);
}

namespace __private {

class NamespaceIter final
    : public sus::iter::IteratorBase<NamespaceIter, Namespace> {
 public:
  static NamespaceIter with(clang::Decl* decl) noexcept {
    return NamespaceIter(decl);
  }

  sus::Option<Namespace> next() noexcept {
    if (next_ndecl_) {
      clang::NamespaceDecl* cur_ndecl = next_ndecl_;
      next_ndecl_ =
          clang::dyn_cast<clang::NamespaceDecl>(cur_ndecl->getParent());
      if (cur_ndecl->isAnonymousNamespace()) {
        return sus::some(sus::choice<NamespaceType::Anonymous>());
      } else {
        return sus::some(
            sus::choice<NamespaceType::Named>(cur_ndecl->getNameAsString()));
      }
    } else if (!done_) {
      done_ = true;
      return sus::some(sus::choice<NamespaceType::Global>());
    } else {
      return sus::none();
    }
  }

 protected:
  NamespaceIter(clang::Decl* decl) noexcept
      : next_ndecl_(find_nearest_namespace(decl)) {}

 private:
  bool done_ = false;
  clang::NamespaceDecl* next_ndecl_;

  sus_class_trivially_relocatable(unsafe_fn, decltype(done_),
                                  decltype(next_ndecl_));
};

}  // namespace __private

/// Returns an iterator over the namespace that `decl` is in, ordered from the
/// nearest inner namespace out to the global namespace.
inline auto iter_namespace_path(clang::Decl* decl) {
  return __private::NamespaceIter::with(decl);
}

inline bool path_contains_namespace(clang::Decl* decl, Namespace n) noexcept {
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

inline bool path_is_private(clang::NamedDecl* decl) noexcept {
  clang::Linkage linkage = decl->getLinkageInternal();
  if (linkage != clang::Linkage::ModuleLinkage &&
      linkage != clang::Linkage::ExternalLinkage) {
    return true;
  }

  // Checks if the declaration itself is private.
  if (decl->getAccess() == clang::AccessSpecifier::AS_private) {
    return true;
  }

  // Look at parent scopes for private access as well.
  clang::DeclContext* cx = decl->getDeclContext();
  while (cx) {
    if (auto* tdecl = clang::dyn_cast<clang::TagDecl>(cx)) {
      // TODO: getAccess() can assert if it's not determined yet due to
      // template instatiation being incomplete..? clang-doc uses
      // getAccessUnsafe() which can give the wrong answer.
      if (tdecl->getAccess() == clang::AccessSpecifier::AS_private) {
        return true;
      }
    }

    cx = cx->getParent();
  }
  return false;
}

}  // namespace subdoc
