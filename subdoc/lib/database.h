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

#include "subdoc/lib/doc_attributes.h"
#include "subdoc/lib/friendly_names.h"
#include "subdoc/lib/method_qualifier.h"
#include "subdoc/lib/parse_comment.h"
#include "subdoc/lib/path.h"
#include "subdoc/lib/record_type.h"
#include "subdoc/lib/requires.h"
#include "subdoc/lib/type.h"
#include "subdoc/lib/linked_type.h"
#include "subdoc/lib/unique_symbol.h"
#include "subdoc/llvm.h"
#include "sus/assertions/check.h"
#include "sus/choice/choice.h"
#include "sus/fn/fn.h"
#include "sus/iter/compat_ranges.h"
#include "sus/option/option.h"
#include "sus/prelude.h"

namespace subdoc {

struct NamespaceElement;
struct FunctionElement;
struct TypeElement;
struct ConceptElement;
struct FieldElement;
struct RecordElement;

enum class FoundNameTag {
  Namespace,
  Function,
  Type,  // Includes Records, Typedefs, Enums.
  Concept,
  Field,
};
using FoundName = sus::Choice<sus_choice_types(
    (FoundNameTag::Namespace, const NamespaceElement&),
    (FoundNameTag::Function, const FunctionElement&),
    (FoundNameTag::Type, const TypeElement&),
    (FoundNameTag::Concept, const ConceptElement&),
    (FoundNameTag::Field, const FieldElement&))>;

struct Comment {
  Comment() = default;
  Comment(std::string text, std::string begin_loc, DocAttributes attrs)
      : text(sus::move(text)),
        begin_loc(sus::move(begin_loc)),
        attrs(sus::move(attrs)) {}

  std::string text;
  std::string begin_loc;
  DocAttributes attrs;

  void inherit_from(const Comment& source) {
    text = sus::clone(source.text);
    attrs = sus::clone(source.attrs);
    // location is not modified.
  }
};

struct CommentElement {
  explicit CommentElement(sus::Vec<Namespace> namespace_path, Comment comment,
                          std::string name, u32 sort_key)
      : namespace_path(sus::move(namespace_path)),
        comment(sus::move(comment)),
        name(sus::move(name)),
        sort_key(sort_key) {
    // All elements have the Global namespace in their path.
    sus::check(this->namespace_path.len() > 0u);
  }

  sus::Vec<Namespace> namespace_path;
  Comment comment;
  std::string name;
  u32 sort_key;

  /// Used during visit to determine if a comment has already been found and
  /// applied to the element.
  bool has_found_comment() const {
    return !comment.text.empty() || comment.attrs.inherit.is_some() ||
           comment.attrs.hidden;
  }
  // Used during generation to get the comment for an element, if any.
  sus::Option<const Comment&> get_comment() const noexcept sus_lifetimebound {
    if (!comment.text.empty())
      return sus::some(comment);
    else
      return sus::none();
  }
  bool hidden() const { return comment.attrs.hidden; }
};

struct TypeElement : public CommentElement {
  TypeElement(sus::Vec<Namespace> containing_namespaces, Comment comment,
              std::string name, sus::Vec<std::string> record_path, u32 sort_key)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name), sort_key),
        record_path(sus::move(record_path)) {}

  /// The records in which this type is nested, not including the type
  /// itself, if it is a record.
  ///
  /// In this example, the record_path would be {S, R}.
  /// ```
  ///   struct R { struct S { struct T{}; }; };
  /// ```
  sus::Vec<std::string> record_path;
};

struct MethodSpecific {
  bool is_static;
  bool is_volatile;
  bool is_virtual;
  bool is_ctor;
  bool is_dtor;
  bool is_conversion;

  // TODO: Find the Database element of the root ancestor virtual method,
  // when this one is virtual, and link to it.

  MethodQualifier qualifier;
};

struct Qualifiers {
  bool is_const;
  bool is_volatile;
};

struct FunctionParameter {
  sus::Option<const TypeElement&> type_element;
  sus::Option<std::string> default_value;
  std::string type_name;
  std::string short_type_name;
  std::string parameter_name;
  Qualifiers qualifiers;
  bool is_lvalue_reference;
  bool is_rvalue_reference;
  // Counts how many `*` are part of the type and their qualifiers.
  sus::Vec<Qualifiers> pointers;
};

struct FunctionOverload {
  sus::Vec<FunctionParameter> parameters;
  sus::Option<MethodSpecific> method;
  // The return type is in the overload info because operator overloads can each
  // have different return types, e.g. operator+(int, int) vs
  // operator+(char, char).
  LinkedType return_type;
  sus::Option<RequiresConstraints> constraints;
  sus::Vec<std::string> template_params;
  bool is_deleted = false;
  // Used to look for uniqueness to avoid adding each forward decl and get
  // multiple overloads of the same function.
  std::string signature;

  // TODO: `noexcept` stuff from FunctionDecl::getExceptionSpecType().
};

struct FunctionElement : public CommentElement {
  explicit FunctionElement(sus::Vec<Namespace> containing_namespaces,
                           Comment comment, std::string name,
                           std::string signature, bool is_operator,
                           LinkedType return_type,
                           sus::Option<RequiresConstraints> constraints,
                           sus::Vec<std::string> template_params,
                           bool is_deleted,
                           sus::Vec<FunctionParameter> parameters,
                           sus::Option<std::string> overload_set,
                           sus::Vec<std::string> record_path, u32 sort_key)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name), sort_key),
        is_operator(is_operator),
        overload_set(sus::move(overload_set)),
        record_path(sus::move(record_path)) {
    overloads.push(FunctionOverload{
        .parameters = sus::move(parameters),
        .method = sus::none(),
        .return_type = sus::move(return_type),
        .constraints = sus::move(constraints),
        .template_params = sus::move(template_params),
        .is_deleted = is_deleted,
        .signature = sus::move(signature),
    });
  }

  bool is_operator;
  sus::Vec<FunctionOverload> overloads;
  sus::Option<std::string> overload_set;
  // If the function is a method on a record, this holds the record and any
  // outer records it's nested within.
  sus::Vec<std::string> record_path;

  bool has_any_comments() const noexcept { return has_found_comment(); }

  sus::Option<const CommentElement&> find_comment(
      std::string_view comment_loc) const noexcept {
    if (comment.begin_loc.ends_with(comment_loc))
      return sus::some(*this);
    else
      return sus::none();
  }

  sus::Option<FoundName> find_name(
      const sus::Slice<std::string_view>& splits) const noexcept {
    if (sus::Option<const std::string_view&> first = splits.first();
        first.is_some()) {
      std::string_view matcher = sus::move(first).unwrap();
      auto overload_string = sus::Option<std::string_view>();
      // What's after ! matches with the #[doc.overloads=_] string.
      if (usize pos = matcher.find('!');
          pos != std::string_view::npos && pos + 1u < matcher.size()) {
        overload_string = sus::some(matcher.substr(pos + 1u));
        matcher = matcher.substr(0u, pos);
      }
      if (matcher == name && overload_string == overload_set) {
        if (splits.len() == 1u) {
          return sus::some(FoundName::with<FoundName::Tag::Function>(*this));
        }
      }
    }
    return sus::none();
  }

  void for_each_comment(sus::fn::FnMutRef<void(Comment&)> fn) { fn(comment); }
};

struct ConceptElement : public CommentElement {
  explicit ConceptElement(sus::Vec<Namespace> containing_namespaces,
                          Comment comment, std::string name,
                          sus::Vec<std::string> template_params,
                          RequiresConstraints constraints, u32 sort_key)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name), sort_key),
        template_params(sus::move(template_params)),
        constraints(sus::move(constraints)) {}

  sus::Vec<std::string> template_params;
  RequiresConstraints constraints;

  bool has_any_comments() const noexcept { return has_found_comment(); }

  sus::Option<const CommentElement&> find_comment(
      std::string_view comment_loc) const noexcept {
    if (comment.begin_loc.ends_with(comment_loc))
      return sus::some(*this);
    else
      return sus::none();
  }

  sus::Option<FoundName> find_name(
      const sus::Slice<std::string_view>& splits) const noexcept {
    if (!splits.is_empty() && splits[0u] == name) {
      if (splits.len() == 1u) {
        return sus::some(FoundName::with<FoundName::Tag::Concept>(*this));
      }
    }
    return sus::none();
  }

  void for_each_comment(sus::fn::FnMutRef<void(Comment&)> fn) { fn(comment); }
};

struct FieldElement : public CommentElement {
  enum StaticType {
    Static,
    NonStatic,
  };

  explicit FieldElement(sus::Vec<Namespace> containing_namespaces,
                        Comment comment, std::string name,
                        LinkedType linked_type,
                        sus::Vec<std::string> record_path, StaticType is_static,
                        sus::Vec<std::string> template_params, u32 sort_key)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name), sort_key),
        record_path(sus::move(record_path)),
        type(sus::move(linked_type)),
        is_static(is_static),
        template_params(sus::move(template_params)) {}

  sus::Vec<std::string> record_path;
  /// The complete type of the field, including any inner types in template
  /// params etc.
  LinkedType type;
  StaticType is_static;
  sus::Vec<std::string> template_params;

  bool has_any_comments() const noexcept { return has_found_comment(); }

  sus::Option<const CommentElement&> find_comment(
      std::string_view comment_loc) const noexcept {
    if (comment.begin_loc.ends_with(comment_loc))
      return sus::some(*this);
    else
      return sus::none();
  }

  sus::Option<FoundName> find_name(
      const sus::Slice<std::string_view>& splits) const noexcept {
    if (!splits.is_empty() && splits[0u] == name) {
      if (splits.len() == 1u) {
        return sus::some(FoundName::with<FoundName::Tag::Field>(*this));
      }
    }
    return sus::none();
  }

  void for_each_comment(sus::fn::FnMutRef<void(Comment&)> fn) { fn(comment); }
};

struct ConceptId {
  explicit ConceptId(std::string name) : name(sus::move(name)) {}

  std::string name;

  bool operator==(const ConceptId&) const = default;

  struct Hash {
    std::size_t operator()(const ConceptId& k) const {
      return std::hash<std::string>()(k.name);
    }
  };
};

struct NamespaceId {
  explicit NamespaceId(std::string name) : name(sus::move(name)) {}

  std::string name;

  bool operator==(const NamespaceId&) const = default;

  struct Hash {
    std::size_t operator()(const NamespaceId& k) const {
      return std::hash<std::string>()(k.name);
    }
  };
};

struct RecordId {
  explicit RecordId(std::string name) : name(sus::move(name)) {}
  explicit RecordId(std::string_view name) : name(name) {}
  explicit RecordId(const clang::RecordDecl& decl)
      : name(decl.getNameAsString()) {}

  std::string name;

  bool operator==(const RecordId&) const = default;

  struct Hash {
    std::size_t operator()(const RecordId& k) const {
      return std::hash<std::string>()(k.name);
    }
  };
};

struct FunctionId {
  explicit FunctionId(std::string name, bool is_static,
                      std::string overload_set)
      : name(sus::move(name)),
        is_static(is_static),
        overload_set(sus::move(overload_set)) {}

  std::string name;
  bool is_static;
  std::string overload_set;

  bool operator==(const FunctionId&) const = default;

  struct Hash {
    std::size_t operator()(const FunctionId& k) const {
      return (std::hash<std::string>()(k.name) << size_t{k.is_static}) +
             std::hash<std::string>()(k.overload_set);
    }
  };
};

struct RecordElement : public TypeElement {
  explicit RecordElement(sus::Vec<Namespace> containing_namespaces,
                         Comment comment, std::string name,
                         sus::Vec<std::string> record_path,
                         RecordType record_type,
                         sus::Option<RequiresConstraints> constraints,
                         sus::Vec<std::string> template_params, bool final,
                         u32 sort_key)
      : TypeElement(sus::move(containing_namespaces), sus::move(comment),
                    sus::move(name), sus::move(record_path), sort_key),
        record_type(record_type),
        constraints(sus::move(constraints)),
        template_params(sus::move(template_params)),
        final(final) {}

  // TODO: Template parameters and requires clause.

  // TODO: Link to all base classes.

  RecordType record_type;
  sus::Option<RequiresConstraints> constraints;
  sus::Vec<std::string> template_params;
  bool final;

  std::unordered_map<RecordId, RecordElement, RecordId::Hash> records;
  std::unordered_map<UniqueSymbol, FieldElement> fields;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> deductions;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> ctors;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> dtors;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> conversions;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> methods;

  bool has_any_comments() const noexcept {
    if (has_found_comment()) return true;
    for (const auto& [u, e] : records) {
      if (e.has_any_comments()) return true;
    }
    for (const auto& [u, e] : fields) {
      if (e.has_any_comments()) return true;
    }
    for (const auto& [u, e] : deductions) {
      if (e.has_any_comments()) return true;
    }
    for (const auto& [u, e] : ctors) {
      if (e.has_any_comments()) return true;
    }
    for (const auto& [u, e] : dtors) {
      if (e.has_any_comments()) return true;
    }
    for (const auto& [u, e] : conversions) {
      if (e.has_any_comments()) return true;
    }
    for (const auto& [u, e] : methods) {
      if (e.has_any_comments()) return true;
    }
    return false;
  }

  /// Finds a TypeElement in the record (not looking recursively) by its
  /// name. It looks for records, enums, etc.
  sus::Option<TypeRef> get_local_type_element_ref_by_name(
      std::string_view find_name) const noexcept {
    if (auto rec_it = records.find(RecordId(std::string(find_name)));
        rec_it != records.end() && !rec_it->second.hidden()) {
      return sus::some(TypeRef::with<TypeRef::Tag::Record>(rec_it->second));
    } else {
      return sus::none();
    }
  }

  sus::Option<const CommentElement&> find_record_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    if (comment.begin_loc.ends_with(comment_loc)) {
      out.insert(*this);
      return out;
    }
    for (const auto& [u, e] : records) {
      out = e.find_record_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_ctor_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : ctors) {
      out = e.find_comment(comment_loc);
      if (out.is_some()) return out;
    }
    for (const auto& [u, e] : records) {
      out = e.find_ctor_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_dtor_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : dtors) {
      out = e.find_comment(comment_loc);
      if (out.is_some()) return out;
    }
    for (const auto& [u, e] : records) {
      out = e.find_dtor_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_method_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : methods) {
      out = e.find_comment(comment_loc);
      if (out.is_some()) return out;
    }
    for (const auto& [u, e] : records) {
      out = e.find_method_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_field_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : fields) {
      out = e.find_comment(comment_loc);
      if (out.is_some()) return out;
    }
    for (const auto& [u, e] : records) {
      out = e.find_field_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<FoundName> find_name(
      const sus::Slice<std::string_view>& splits) const noexcept {
    sus::Option<FoundName> out;
    if (!splits.is_empty() && splits[0u] == name) {
      if (splits.len() == 1u) {
        out = sus::some(FoundName::with<FoundName::Tag::Type>(*this));
      } else {
        for (auto& [k, e] : records) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
        for (auto& [k, e] : fields) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
        for (auto& [k, e] : deductions) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
        for (auto& [k, e] : ctors) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
        for (auto& [k, e] : dtors) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
        for (auto& [k, e] : conversions) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
        for (auto& [k, e] : methods) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
      }
    }
    return out;
  }

  void for_each_comment(sus::fn::FnMutRef<void(Comment&)> fn) {
    fn(comment);
    for (auto& [k, e] : records) e.for_each_comment(fn);
    for (auto& [k, e] : fields) e.for_each_comment(fn);
    for (auto& [k, e] : deductions) e.for_each_comment(fn);
    for (auto& [k, e] : ctors) e.for_each_comment(fn);
    for (auto& [k, e] : dtors) e.for_each_comment(fn);
    for (auto& [k, e] : conversions) e.for_each_comment(fn);
    for (auto& [k, e] : methods) e.for_each_comment(fn);
  }
};

struct NamespaceElement : public CommentElement {
  explicit NamespaceElement(sus::Vec<Namespace> containing_namespaces,
                            Comment comment, std::string name, u32 sort_key)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name), sort_key),
        // The front of `namespace_path` will be this NamespaceElement's
        // identity.
        namespace_name(namespace_path[0u]) {}

  Namespace namespace_name;
  std::unordered_map<ConceptId, ConceptElement, ConceptId::Hash> concepts;
  std::unordered_map<NamespaceId, NamespaceElement, NamespaceId::Hash>
      namespaces;
  std::unordered_map<RecordId, RecordElement, RecordId::Hash> records;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> functions;

  bool is_empty() const noexcept {
    return namespaces.empty() && records.empty() && functions.empty();
  }

  bool has_any_comments() const noexcept {
    if (has_found_comment()) return true;
    for (const auto& [u, e] : concepts) {
      if (e.has_any_comments()) return true;
    }
    for (const auto& [u, e] : namespaces) {
      if (e.has_any_comments()) return true;
    }
    for (const auto& [u, e] : records) {
      if (e.has_any_comments()) return true;
    }
    for (const auto& [u, e] : functions) {
      if (e.has_any_comments()) return true;
    }
    return false;
  }

  /// Finds a TypeElement in the namespace (not looking recursively) by its
  /// name. It looks for records, concepts, etc.
  sus::Option<TypeRef> get_local_type_element_ref_by_name(
      std::string_view find_name) const noexcept {
    if (auto rec_it = records.find(RecordId(std::string(find_name)));
        rec_it != records.end() && !rec_it->second.hidden()) {
      return sus::some(TypeRef::with<TypeRef::Tag::Record>(rec_it->second));
    } else if (auto con_it = concepts.find(ConceptId(std::string(find_name)));
               con_it != concepts.end() && !con_it->second.hidden()) {
      return sus::some(TypeRef::with<TypeRef::Tag::Concept>(con_it->second));
    } else {
      return sus::none();
    }
  }

  sus::Option<const CommentElement&> find_concept_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : concepts) {
      out = e.find_comment(comment_loc);
      if (out.is_some()) return out;
    }
    for (const auto& [u, e] : namespaces) {
      out = e.find_concept_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_record_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : records) {
      out = e.find_record_comment(comment_loc);
      if (out.is_some()) return out;
    }
    for (const auto& [u, e] : namespaces) {
      out = e.find_record_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_namespace_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    if (comment.begin_loc.ends_with(comment_loc)) {
      out.insert(*this);
      return out;
    }
    for (const auto& [u, e] : namespaces) {
      out = e.find_namespace_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_function_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : namespaces) {
      out = e.find_function_comment(comment_loc);
      if (out.is_some()) return out;
    }
    for (const auto& [u, e] : functions) {
      out = e.find_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_ctor_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : records) {
      out = e.find_ctor_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_dtor_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : records) {
      out = e.find_dtor_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_method_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : records) {
      out = e.find_method_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<const CommentElement&> find_field_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : records) {
      out = e.find_field_comment(comment_loc);
      if (out.is_some()) return out;
    }
    return out;
  }

  sus::Option<FoundName> find_name(
      const sus::Slice<std::string_view>& splits) const noexcept {
    sus::Option<FoundName> out;
    if (!splits.is_empty() && splits[0u] == name) {
      if (splits.len() == 1u) {
        out = sus::some(FoundName::with<FoundName::Tag::Namespace>(*this));
      } else {
        for (auto& [k, e] : concepts) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
        for (auto& [k, e] : namespaces) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
        for (auto& [k, e] : records) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
        for (auto& [k, e] : functions) {
          if (out = e.find_name(splits["1.."_r]); out.is_some()) return out;
        }
      }
    }
    return out;
  }

  void for_each_comment(sus::fn::FnMutRef<void(Comment&)> fn) {
    fn(comment);
    for (auto& [k, e] : concepts) e.for_each_comment(fn);
    for (auto& [k, e] : namespaces) e.for_each_comment(fn);
    for (auto& [k, e] : records) e.for_each_comment(fn);
    for (auto& [k, e] : functions) e.for_each_comment(fn);
  }
};

inline NamespaceId key_for_namespace(clang::NamespaceDecl* decl) noexcept {
  return NamespaceId(decl->getNameAsString());
}

inline ConceptId key_for_concept(clang::ConceptDecl* decl) noexcept {
  return ConceptId(decl->getNameAsString());
}

inline FunctionId key_for_function(
    clang::FunctionDecl* decl, sus::Option<std::string> overload_set) noexcept {
  bool is_static = [&]() {
    if (auto* mdecl = clang::dyn_cast<clang::CXXMethodDecl>(decl))
      return mdecl->isStatic();
    return false;
  }();
  return FunctionId(decl->getNameAsString(), is_static,
                    sus::move(overload_set).unwrap_or_default());
}

struct Database {
  explicit Database(Comment overview_comment)
      : global(sus::vec(Namespace::with<Namespace::Tag::Global>()),
               sus::move(overview_comment), std::string(), 0u) {}

  NamespaceElement global;

  bool has_any_comments() const noexcept { return global.has_any_comments(); }

  sus::Result<void, std::string> resolve_inherited_comments() {
    sus::Vec<Comment*> to_resolve;
    {
      sus::Vec<Comment*>* to_resolve_ptr = &to_resolve;
      sus::fn::FnMutBox<void(Comment&)> fn = sus_bind_mut(
          sus_store(sus_unsafe_pointer(to_resolve_ptr)), [&](Comment& c) {
            if (c.attrs.inherit.is_some()) {
              to_resolve_ptr->push(&c);
            }
          });
      global.for_each_comment(fn);
    }

    while (!to_resolve.is_empty()) {
      auto remaining = sus::Vec<Comment*>::with_capacity(to_resolve.len());

      for (Comment* c : sus::move(to_resolve).into_iter()) {
        enum class Target {
          Namespace,
          Record,
          Function,
        };
        sus::Choice<sus_choice_types(
            (Target::Namespace, const NamespaceElement&),
            (Target::Record, const RecordElement&),
            (Target::Function, const FunctionElement&))>
            target = sus::choice<Target::Namespace>(global);
        for (const InheritPathElement& e : *(c->attrs.inherit)) {
          switch (e) {
            case InheritPathNamespace: {
              const std::string& name = e.as<InheritPathNamespace>();
              auto id = NamespaceId(name);
              if (target.which() != Target::Namespace) {
                std::ostringstream s;
                s << "Inherited comment at " << c->begin_loc
                  << " has invalid path, with a namespace inside a "
                     "non-namespace.";
                return sus::err(sus::move(s).str());
              }
              if (!target.as_mut<Target::Namespace>().namespaces.contains(id)) {
                std::ostringstream s;
                s << "Inherited comment at " << c->begin_loc
                  << " can't find namespace " << e.as<InheritPathNamespace>();
                return sus::err(sus::move(s).str());
              }
              target = sus::choice<Target::Namespace>(
                  target.as_mut<Target::Namespace>().namespaces.at(id));
              break;
            }
            case InheritPathRecord: {
              // TODO: Make Record maps keyed on a RecordId we can construct
              // here.
              const std::string& name = e.as<InheritPathRecord>();
              auto find_record = [&](const auto& record_map)
                  -> sus::Option<const RecordElement&> {
                for (const auto& [k, e] : record_map) {
                  if (e.name == name) return sus::some(e);
                }
                return sus::none();
              };
              sus::Option<const RecordElement&> record;
              switch (target) {
                case Target::Namespace:
                  record =
                      find_record(target.as_mut<Target::Namespace>().records);
                  break;
                case Target::Record:
                  record = find_record(target.as_mut<Target::Record>().records);
                  break;
                case Target::Function: {
                  std::ostringstream s;
                  s << "Inherited comment at " << c->begin_loc
                    << " has invalid path, with a record inside a function.";
                  return sus::err(sus::move(s).str());
                }
              }
              if (record.is_none()) {
                std::ostringstream s;
                s << "Inherited comment at " << c->begin_loc
                  << " can't find record " << name;
                return sus::err(sus::move(s).str());
              }
              target = sus::choice<Target::Record>(sus::move(record).unwrap());
              break;
            }
            case InheritPathFunction: {
              const std::string& name = e.as<InheritPathFunction>();
              auto find_function = [&](const auto& function_map)
                  -> sus::Option<const FunctionElement&> {
                for (const auto& [k, e] : function_map) {
                  if (e.name == name) return sus::some(e);
                }
                return sus::none();
              };
              sus::Option<const FunctionElement&> function;
              switch (target) {
                case Target::Namespace:
                  function = find_function(
                      target.as_mut<Target::Namespace>().functions);
                  break;
                case Target::Record:
                  function =
                      find_function(target.as_mut<Target::Record>().methods);
                  break;
                case Target::Function: {
                  std::ostringstream s;
                  s << "Inherited comment at " << c->begin_loc
                    << " has invalid path, with a function inside a function.";
                  return sus::err(sus::move(s).str());
                }
              }
              if (function.is_none()) {
                std::ostringstream s;
                s << "Inherited comment at " << c->begin_loc
                  << " can't find function " << name;
                return sus::err(sus::move(s).str());
              }
              target =
                  sus::choice<Target::Function>(sus::move(function).unwrap());
              break;
            }
          }
        }

        sus::Option<const Comment&> source;
        switch (target) {
          case Target::Namespace: {
            const NamespaceElement& e = target.as<Target::Namespace>();
            if (e.comment.attrs.inherit.is_none()) source.insert(e.comment);
            break;
          }
          case Target::Record: {
            const RecordElement& e = target.as<Target::Record>();
            if (e.comment.attrs.inherit.is_none()) source.insert(e.comment);
            break;
          }
          case Target::Function: {
            const FunctionElement& e = target.as<Target::Function>();
            if (e.comment.attrs.inherit.is_none()) source.insert(e.comment);
            break;
          }
        }
        if (source.is_some()) {
          c->inherit_from(sus::move(source).unwrap());
        } else {
          remaining.push(c);  // The target still needs to be resolved.
        }
      }

      to_resolve = sus::move(remaining);
    }

    return sus::ok();
  }

  sus::Option<const TypeElement&> find_type(clang::QualType qual) {
    clang::QualType unqualified = qual.getUnqualifiedType();
    const clang::Type* base_type = unqualified.getTypePtr();

    // TODO: The type can be something like std::remove_reference<T> in which
    // case we should really be finding the `T`?

    // TODO: But what about Option<Stuff>? We can't return both Option and
    // Stuff, so how do we track multiple types in a type?

    if (base_type->isArrayType()) {
      base_type = base_type->getArrayElementTypeNoTypeQual();
    }
    if (base_type->isPointerType()) {
      return find_type(base_type->getPointeeType());
    }
    if (base_type->isReferenceType()) {
      return find_type(base_type->getPointeeType());
    }

    // TODO: Templates are not TagDecl so this does not find incomplete
    // templates in our database. But see above, we need the full set of types
    // that make up the overall type, templates can have inner types.
    auto* decl = [&]() -> clang::Decl* {
      if (clang::TagDecl* tdecl = base_type->getAsTagDecl()) return tdecl;
      if (const clang::TypedefType* type =
              base_type->getAs<clang::TypedefType>())
        return type->getDecl();
      return nullptr;
    }();

    if (!decl) return sus::none();

    auto ns_cursor = [&]() -> sus::Option<const NamespaceElement&> {
      const NamespaceElement* cursor = &global;

      // Namespace path goes from the outside in to the global, we want the
      // inverse, and then to skip the global namespace.
      auto it = iter_namespace_path(decl).collect_vec().into_iter().rev();
      it.next();  // TODO: Use Iterator::skip().
      for (const Namespace& n : it) {
        switch (n) {
          case Namespace::Tag::Global: {
            // We skipped the 1st namespace in the path which is the global
            // one.
            sus::unreachable();
          }
          case Namespace::Tag::Anonymous: {
            return sus::none();
          }
          case Namespace::Tag::Named: {
            const std::string& name = n.as<Namespace::Tag::Named>();
            auto ns_it = cursor->namespaces.find(NamespaceId(name));
            if (ns_it == cursor->namespaces.end()) {
              return sus::none();
            }
            cursor = &ns_it->second;
          }
        }
      }
      return sus::some(*cursor);
    }();
    if (ns_cursor.is_none()) {
      // llvm::errs() << "WARNING: Unable to find namespace for type '"
      //              << qual.getAsString() << "'\n";
      return sus::none();
    }

    auto record_cursor = [&]() -> sus::Option<const RecordElement&> {
      if (auto* containing_record_decl =
              clang::dyn_cast<clang::RecordDecl>(decl->getDeclContext())) {
        const RecordElement* cursor = nullptr;

        bool first = true;  // TODO: Use Iterator::enumerate().
        for (std::string_view name : iter_record_path(containing_record_decl)
                                         .collect_vec()
                                         .into_iter()
                                         .rev()) {
          if (first) {
            auto r_it = ns_cursor->records.find(RecordId(name));
            if (r_it == ns_cursor->records.end()) {
              return sus::none();
            }
            cursor = &r_it->second;
          } else {
            auto r_it = cursor->records.find(RecordId(name));
            if (r_it == cursor->records.end()) {
              return sus::none();
            }
            cursor = &r_it->second;
          }
          first = false;
        }
        return sus::some(*cursor);
      } else {
        return sus::none();
      }
    }();

    if (auto* record_decl = clang::dyn_cast<clang::RecordDecl>(decl)) {
      if (record_cursor.is_some()) {  // The TagDecl is located in a record.
        auto it = record_cursor->records.find(RecordId(*record_decl));
        if (it == record_cursor->records.end()) return sus::none();
        return sus::some(it->second);
      } else {  // The TagDecl is located in a namespace.
        auto it = ns_cursor->records.find(RecordId(*record_decl));
        if (it == ns_cursor->records.end()) return sus::none();
        return sus::some(it->second);
      }
    } else if (auto* enum_decl = clang::dyn_cast<clang::EnumDecl>(decl)) {
      // TODO: Support enums!  They are not stored in the database.
    } else if (auto* tdef_decl =
                   clang::dyn_cast<clang::TypedefNameDecl>(decl)) {
      // TODO: Support typedefs!  They are not stored in the database.
    }
    return sus::none();
  }

  sus::Vec<sus::Option<TypeRef>> collect_type_element_refs(
      const Type& type) const noexcept {
    sus::Vec<sus::Option<TypeRef>> vec;
    type_walk_types(type, [&](TypeToStringQuery q) {
      const NamespaceElement* ns_cursor = &global;
      for (const std::string& name : q.namespace_path) {
        auto it = ns_cursor->namespaces.find(NamespaceId(name));
        if (it == ns_cursor->namespaces.end()) {
          vec.push(sus::none());
          return;
        }
        ns_cursor = &it->second;
      }
      if (q.record_path.is_empty()) {
        vec.push(ns_cursor->get_local_type_element_ref_by_name(q.name));
        return;
      }

      const RecordElement* rec_cursor = nullptr;
      for (const auto& [i, name] : q.record_path.iter().enumerate()) {
        if (i == 0u) {
          auto it = ns_cursor->records.find(RecordId(name));
          if (it == ns_cursor->records.end()) {
            vec.push(sus::none());
            return;
          }
          rec_cursor = &it->second;
        } else {
          auto it = rec_cursor->records.find(RecordId(name));
          if (it == rec_cursor->records.end()) {
            vec.push(sus::none());
            return;
          }
          rec_cursor = &it->second;
        }
      }
      vec.push(rec_cursor->get_local_type_element_ref_by_name(q.name));
    });
    return vec;
  }

  sus::Option<NamespaceElement&> find_namespace_mut(
      clang::NamespaceDecl* ndecl) & noexcept {
    if (!ndecl) return sus::some(global);

    sus::Option<NamespaceElement&> parent_element = find_namespace_mut(
        clang::dyn_cast<clang::NamespaceDecl>(ndecl->getParent()));
    if (parent_element.is_none()) return sus::none();
    if (auto it = parent_element->namespaces.find(key_for_namespace(ndecl));
        it != parent_element->namespaces.end()) {
      return sus::some(it->second);
    } else {
      return sus::none();
    }
  }
  sus::Option<RecordElement&> find_record_mut(
      clang::RecordDecl* rdecl) & noexcept {
    sus::Option<NamespaceElement&> ne =
        find_namespace_mut(find_nearest_namespace(rdecl));
    return sus::move(ne).and_then(
        [&](NamespaceElement& e) { return find_record_mut_impl(rdecl, e); });
  }

  /// Finds a comment whose location ends with the `comment_loc` suffix.
  ///
  /// The suffix can be used to look for the line:column and ignore the
  /// filename in the comment location format `filename:line:col`.
  sus::Option<const CommentElement&> find_concept_comment(
      std::string_view comment_loc) const& noexcept {
    return global.find_concept_comment(comment_loc);
  }
  sus::Option<const CommentElement&> find_concept_comment(
      std::string_view comment_loc) && = delete;

  /// Finds a comment whose location ends with the `comment_loc` suffix.
  ///
  /// The suffix can be used to look for the line:column and ignore the
  /// filename in the comment location format `filename:line:col`.
  sus::Option<const CommentElement&> find_record_comment(
      std::string_view comment_loc) const& noexcept {
    return global.find_record_comment(comment_loc);
  }
  sus::Option<const CommentElement&> find_record_comment(
      std::string_view comment_loc) && = delete;

  /// Finds a comment whose location ends with the `comment_loc` suffix.
  ///
  /// The suffix can be used to look for the line:column and ignore the
  /// filename in the comment location format `filename:line:col`.
  sus::Option<const CommentElement&> find_namespace_comment(
      std::string_view comment_loc) const& noexcept {
    return global.find_namespace_comment(comment_loc);
  }
  sus::Option<const CommentElement&> find_namespace_comment(
      std::string_view comment_loc) && = delete;

  /// Finds a comment whose location ends with the `comment_loc` suffix.
  ///
  /// The suffix can be used to look for the line:column and ignore the
  /// filename in the comment location format `filename:line:col`.
  sus::Option<const CommentElement&> find_function_comment(
      std::string_view comment_loc) const& noexcept {
    return global.find_function_comment(comment_loc);
  }
  sus::Option<const CommentElement&> find_function_comment(
      std::string_view comment_loc) && = delete;

  /// Finds a comment whose location ends with the `comment_loc` suffix.
  ///
  /// The suffix can be used to look for the line:column and ignore the
  /// filename in the comment location format `filename:line:col`.
  sus::Option<const CommentElement&> find_ctor_comment(
      std::string_view comment_loc) const noexcept {
    return global.find_ctor_comment(comment_loc);
  }

  /// Finds a comment whose location ends with the `comment_loc` suffix.
  ///
  /// The suffix can be used to look for the line:column and ignore the
  /// filename in the comment location format `filename:line:col`.
  sus::Option<const CommentElement&> find_dtor_comment(
      std::string_view comment_loc) const noexcept {
    return global.find_dtor_comment(comment_loc);
  }

  /// Finds a comment whose location ends with the `comment_loc` suffix.
  ///
  /// The suffix can be used to look for the line:column and ignore the
  /// filename in the comment location format `filename:line:col`.
  sus::Option<const CommentElement&> find_method_comment(
      std::string_view comment_loc) const noexcept {
    return global.find_method_comment(comment_loc);
  }

  /// Finds a comment whose location ends with the `comment_loc` suffix.
  ///
  /// The suffix can be used to look for the line:column and ignore the
  /// filename in the comment location format `filename:line:col`.
  sus::Option<const CommentElement&> find_field_comment(
      std::string_view comment_loc) const& noexcept {
    return global.find_field_comment(comment_loc);
  }
  sus::Option<const CommentElement&> find_field_comment(
      std::string_view comment_loc) && = delete;

  /// Finds an element in the database by its fully qualified C++ name, e.g.
  /// "sus::ops::Try".
  ///
  /// If there's a #, what comes after it is used as the overload set
  /// matcher for functions, which will match with what was specified in
  /// `#[doc.overloads=_]`.
  sus::Option<FoundName> find_name(std::string_view full_name) const noexcept {
    llvm::SmallVector<llvm::StringRef> splits;
    llvm::StringRef(full_name).split(splits, "::");

    auto v = sus::Vec<std::string_view>::with_capacity(splits.size() + 1u);
    // If the symbol starts with `::` then the first element is empty and
    // matches the global namespace; otherwise, we insert the global namespace
    // matcher.
    if (splits[0u] != "") v.push(std::string_view(global.name));
    v.extend(sus::iter::from_range(splits).map(
        [](llvm::StringRef r) { return std::string_view(r); }));
    return global.find_name(v.as_slice());
  }

 private:
  sus::Option<RecordElement&> find_record_mut_impl(clang::RecordDecl* rdecl,
                                                   NamespaceElement& ne) & {
    if (auto* parent = clang::dyn_cast<clang::RecordDecl>(rdecl->getParent())) {
      if (sus::Option<RecordElement&> parent_element =
              find_record_mut_impl(parent, ne);
          parent_element.is_some()) {
        return sus::some(parent_element->records.at(RecordId(*rdecl)));
      } else {
        return sus::none();
      }
    } else {
      if (auto it = ne.records.find(RecordId(*rdecl)); it != ne.records.end()) {
        return sus::some(it->second);
      } else {
        return sus::none();
      }
    }
  }
};

static_assert(sus::mem::Move<Database>);

}  // namespace subdoc
