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

#include <unordered_map>

#include "subdoc/lib/path.h"
#include "subdoc/lib/unique_symbol.h"
#include "subdoc/llvm.h"
#include "subspace/choice/choice.h"
#include "subspace/prelude.h"

namespace subdoc {

struct Comment {
#if defined(__clang__) && !__has_feature(__cpp_aggregate_paren_init)
  Comment(std::string raw_text, clang::SourceRange source_range)
      : raw_text(raw_text), source_range(source_range) {}
#endif

  std::string raw_text;
  std::string begin_loc;
};

struct CommentElement {
  explicit CommentElement(sus::Vec<Namespace> namespace_path, Comment comment,
                          std::string name)
      : namespace_path(sus::move(namespace_path)),
        comment(sus::move(comment)),
        name(sus::move(name)) {}

  sus::Vec<Namespace> namespace_path;
  Comment comment;
  std::string name;
};

struct ClassElement : public CommentElement {
  explicit ClassElement(sus::Vec<Namespace> namespaces, Comment comment,
                        std::string name)
      : CommentElement(sus::move(namespaces), sus::move(comment),
                       sus::move(name)) {}

  // TODO: Template parameters and requires clause.
};
struct UnionElement : public CommentElement {
  explicit UnionElement(sus::Vec<Namespace> namespaces, Comment comment,
                        std::string name)
      : CommentElement(sus::move(namespaces), sus::move(comment),
                       sus::move(name)) {}

  // TODO: Template parameters and requires clause.
};
struct FunctionElement : public CommentElement {
  explicit FunctionElement(sus::Vec<Namespace> namespaces, Comment comment,
                           std::string name, std::string signature)
      : CommentElement(sus::move(namespaces), sus::move(comment),
                       sus::move(name)),
        signature(sus::move(signature)) {}

  std::string signature;
};
struct FieldElement : public CommentElement {
  enum StaticType {
    Static,
    NonStatic,
  };

  explicit FieldElement(sus::Vec<Namespace> namespaces, Comment comment,
                        std::string name, sus::Vec<std::string> record_path,
                        StaticType is_static)
      : CommentElement(sus::move(namespaces), sus::move(comment),
                       sus::move(name)),
        record_path(sus::move(record_path)),
        is_static(is_static) {}

  sus::Vec<std::string> record_path;
  // TODO: clang::Qualifiers.
  StaticType is_static;
};

struct Database {
  std::unordered_map<UniqueSymbol, ClassElement> classes;
  std::unordered_map<UniqueSymbol, UnionElement> unions;
  std::unordered_map<UniqueSymbol, FunctionElement> functions;
  std::unordered_map<UniqueSymbol, FunctionElement> deductions;
  std::unordered_map<UniqueSymbol, FunctionElement> ctors;
  std::unordered_map<UniqueSymbol, FunctionElement> dtors;
  std::unordered_map<UniqueSymbol, FunctionElement> conversions;
  std::unordered_map<UniqueSymbol, FunctionElement> methods;
  std::unordered_map<UniqueSymbol, FieldElement> fields;

  bool is_empty() const noexcept {
    return classes.empty() && unions.empty() && functions.empty() &&
           deductions.empty() && ctors.empty() && dtors.empty() &&
           conversions.empty() && methods.empty() && fields.empty();
  }
};

static_assert(sus::mem::Move<Database>);

}  // namespace subdoc
