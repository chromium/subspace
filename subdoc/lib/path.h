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
#include "sus/choice/choice.h"
#include "sus/collections/vec.h"
#include "sus/iter/iterator.h"
#include "sus/prelude.h"

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
  explicit RecordIter(clang::DeclContext* decl) noexcept : next_decl_(nullptr) {
    while (decl) {
      if (clang::isa<clang::RecordDecl>(decl)) {
        next_decl_ = clang::cast<clang::RecordDecl>(decl);
        break;
      }
      decl = decl->getParent();
    }
  }

  Option<std::string_view> next() noexcept {
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
  sus::iter::SizeHint size_hint() const noexcept {
    return sus::iter::SizeHint(0u, sus::none());
  }

 private:
  clang::RecordDecl* next_decl_;

  sus_class_trivially_relocatable(unsafe_fn, decltype(next_decl_));
};

static_assert(::sus::iter::Iterator<RecordIter, std::string_view>);

}  // namespace __private

/// Returns an iterator over the record `decl` and any records it is nested
/// within, ordered from inside to outside.
///
/// The iterator returns std::string_view references to string in the clang
/// AST, which are valid as long as the RecordDecl's pointee is valid.
inline sus::iter::Iterator<std::string_view> auto iter_record_path(
    clang::DeclContext* decl) {
  return __private::RecordIter(decl);
}

namespace __private {

class NamespaceIter final
    : public sus::iter::IteratorBase<NamespaceIter, Namespace> {
 public:
  explicit NamespaceIter(clang::Decl* decl) noexcept
      : next_ndecl_(find_nearest_namespace(decl)) {}

  Option<Namespace> next() noexcept {
    if (next_ndecl_) {
      clang::NamespaceDecl* cur_ndecl = next_ndecl_;
      next_ndecl_ =
          clang::dyn_cast<clang::NamespaceDecl>(cur_ndecl->getParent());
      if (cur_ndecl->isAnonymousNamespace()) {
        return sus::some(Namespace::with<NamespaceType::Anonymous>());
      } else {
        return sus::some(Namespace::with<NamespaceType::Named>(
            cur_ndecl->getNameAsString()));
      }
    } else if (!done_) {
      done_ = true;
      return sus::some(Namespace::with<NamespaceType::Global>());
    } else {
      return sus::none();
    }
  }
  sus::iter::SizeHint size_hint() const noexcept {
    return sus::iter::SizeHint(0u, sus::none());
  }

 private:
  bool done_ = false;
  clang::NamespaceDecl* next_ndecl_;

  sus_class_trivially_relocatable(unsafe_fn, decltype(done_),
                                  decltype(next_ndecl_));
};

static_assert(::sus::iter::Iterator<NamespaceIter, Namespace>);

}  // namespace __private

/// Returns an iterator over the namespace that `decl` is in, ordered from the
/// nearest inner namespace out to the global namespace.
inline sus::iter::Iterator<Namespace> auto iter_namespace_path(
    clang::Decl* decl) {
  return __private::NamespaceIter(decl);
}

inline std::string namespace_path_to_string(
    sus::iter::Iterator<const Namespace&> auto it) noexcept {
  std::ostringstream s;
  u32 count;
  for (const Namespace& n : it) {
    if (count > 0u) s << "::";
    switch (n) {
      case Namespace::Tag::Global: break;
      case Namespace::Tag::Anonymous:
        s << "<anonymous>";
        count += 1u;
        break;
      case Namespace::Tag::Named:
        s << n.as<Namespace::Tag::Named>();
        count += 1u;
        break;
    }
  }
  return sus::move(s).str();
}

/// Whether the `decl` has `n` in its namespace path.
bool path_contains_namespace(clang::Decl* decl, Namespace n) noexcept;

/// Whether the `decl` is marked private anywhere along its namespace/record
/// path.
bool path_is_private(clang::NamedDecl* decl) noexcept;

}  // namespace subdoc
