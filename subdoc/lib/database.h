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
        name(sus::move(name)) {
    // All elements have the Global namespace in their path.
    assert(this->namespace_path.len() > 0u);
  }

  sus::Vec<Namespace> namespace_path;
  Comment comment;
  std::string name;
};

struct NamespaceElement : public CommentElement {
  explicit NamespaceElement(sus::Vec<Namespace> containing_namespaces,
                            Comment comment, std::string name)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name)),
        // The front of `namespace_path` will be this NamespaceElement's
        // identity.
        namespace_name(namespace_path[0u]) {}

  Namespace namespace_name;
};
struct RecordElement : public CommentElement {
  enum RecordType { Class, Struct, Union };

  explicit RecordElement(sus::Vec<Namespace> containing_namespaces,
                         Comment comment, std::string name,
                         RecordType record_type)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name)),
        record_type(record_type) {}

  // TODO: Template parameters and requires clause.

  /// The classes in which this class is nested.
  ///
  /// In this example, the class_path would be {S, R}.
  /// ```
  ///   struct R { struct S { struct T{}; }; };
  /// ```
  sus::Vec<std::string> class_path;

  RecordType record_type;
};
struct FunctionElement : public CommentElement {
  explicit FunctionElement(sus::Vec<Namespace> containing_namespaces,
                           Comment comment, std::string name,
                           std::string signature)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name)),
        signature(sus::move(signature)) {}

  std::string signature;
};
struct FieldElement : public CommentElement {
  enum StaticType {
    Static,
    NonStatic,
  };

  explicit FieldElement(sus::Vec<Namespace> containing_namespaces,
                        Comment comment, std::string name,
                        sus::Vec<std::string> record_path, StaticType is_static)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name)),
        record_path(sus::move(record_path)),
        is_static(is_static) {}

  sus::Vec<std::string> record_path;
  // TODO: clang::Qualifiers.
  StaticType is_static;
};

struct Database {
  std::unordered_map<UniqueSymbol, NamespaceElement> namespaces;
  std::unordered_map<UniqueSymbol, RecordElement> records;
  std::unordered_map<UniqueSymbol, FunctionElement> functions;
  std::unordered_map<UniqueSymbol, FunctionElement> deductions;
  std::unordered_map<UniqueSymbol, FunctionElement> ctors;
  std::unordered_map<UniqueSymbol, FunctionElement> dtors;
  std::unordered_map<UniqueSymbol, FunctionElement> conversions;
  std::unordered_map<UniqueSymbol, FunctionElement> methods;
  std::unordered_map<UniqueSymbol, FieldElement> fields;

  bool is_empty() const noexcept {
    return records.empty() && functions.empty() && deductions.empty() &&
           ctors.empty() && dtors.empty() && conversions.empty() &&
           methods.empty() && fields.empty();
  }
};

static_assert(sus::mem::Move<Database>);

}  // namespace subdoc
