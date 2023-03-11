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
#include "subdoc/lib/path.h"
#include "subdoc/lib/record_type.h"
#include "subdoc/lib/type.h"
#include "subdoc/lib/unique_symbol.h"
#include "subdoc/llvm.h"
#include "subspace/choice/choice.h"
#include "subspace/fn/fn.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

namespace subdoc {

struct Comment {
  Comment() = default;
  Comment(std::string raw_text, std::string begin_loc, DocAttributes attrs)
      : raw_text(raw_text),
        begin_loc(sus::move(begin_loc)),
        attrs(sus::move(attrs)) {}

  std::string raw_text;
  std::string begin_loc;
  DocAttributes attrs;

  void inherit_from(const Comment& source) {
    raw_text = source.raw_text;
    attrs = sus::clone(source.attrs);
    // location is not modified.
  }

  std::string_view summary() const& { return raw_text; }
  std::string_view summary() && = delete;
};

struct CommentElement {
  explicit CommentElement(sus::Vec<Namespace> namespace_path, Comment comment,
                          std::string name, u32 sort_key)
      : namespace_path(sus::move(namespace_path)),
        comment(sus::move(comment)),
        name(sus::move(name)),
        sort_key(sort_key) {
    // All elements have the Global namespace in their path.
    assert(this->namespace_path.len() > 0u);
  }

  sus::Vec<Namespace> namespace_path;
  Comment comment;
  std::string name;
  u32 sort_key;

  bool has_comment() const {
    return !comment.raw_text.empty() || comment.attrs.inherit.is_some();
  }
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

  // TODO: Find the Database element of the root ancestor virtual method,
  // when this one is virtual, and link to it.

  MethodQualifier qualifier;
};

struct FunctionParameter {
  sus::Option<const TypeElement&> type_element;
  sus::Option<std::string> default_value;
  std::string type_name;
  std::string short_type_name;
};

struct FunctionOverload {
  sus::Vec<FunctionParameter> parameters;
  sus::Option<MethodSpecific> method;

  // TODO: `noexcept` stuff from FunctionDecl::getExceptionSpecType().
};

struct FunctionElement : public CommentElement {
  explicit FunctionElement(sus::Vec<Namespace> containing_namespaces,
                           Comment comment, std::string name,
                           clang::QualType return_qual_type,
                           sus::Vec<FunctionParameter> parameters, u32 sort_key)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name), sort_key),
        return_type_name(friendly_type_name(return_qual_type)),
        return_short_type_name(friendly_short_type_name(return_qual_type)) {
    overloads.push(FunctionOverload{
        .parameters = sus::move(parameters),
        .method = sus::none(),
    });
  }

  sus::Option<const TypeElement&> return_type_element;
  std::string return_type_name;
  std::string return_short_type_name;

  sus::Vec<FunctionOverload> overloads;

  bool has_any_comments() const noexcept { return has_comment(); }

  sus::Option<const CommentElement&> find_comment(
      std::string_view comment_loc) const noexcept {
    if (comment.begin_loc.ends_with(comment_loc))
      return sus::some(*this);
    else
      return sus::none();
  }

  void for_each_comment(sus::fn::FnMut<void(Comment&)> fn) { fn(comment); }
};

struct FieldElement : public CommentElement {
  enum StaticType {
    Static,
    NonStatic,
  };

  explicit FieldElement(sus::Vec<Namespace> containing_namespaces,
                        Comment comment, std::string name,
                        clang::QualType qual_type,
                        sus::Vec<std::string> record_path, StaticType is_static,
                        u32 sort_key)
      : CommentElement(sus::move(containing_namespaces), sus::move(comment),
                       sus::move(name), sort_key),
        record_path(sus::move(record_path)),
        type_name(friendly_type_name(qual_type)),
        short_type_name(friendly_short_type_name(qual_type)),
        is_const(qual_type.getQualifiers().hasConst()),
        is_volatile(qual_type.getQualifiers().hasVolatile()),
        is_static(is_static) {}

  sus::Vec<std::string> record_path;
  sus::Option<const TypeElement&> type_element;
  std::string type_name;
  std::string short_type_name;
  bool is_const;
  bool is_volatile;
  StaticType is_static;

  bool has_any_comments() const noexcept { return has_comment(); }

  sus::Option<const CommentElement&> find_comment(
      std::string_view comment_loc) const noexcept {
    if (comment.begin_loc.ends_with(comment_loc))
      return sus::some(*this);
    else
      return sus::none();
  }

  void for_each_comment(sus::fn::FnMut<void(Comment&)> fn) { fn(comment); }
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
  explicit FunctionId(std::string name, bool is_static, u64 overload_set)
      : name(sus::move(name)),
        is_static(is_static),
        overload_set(overload_set) {}

  std::string name;
  bool is_static;
  u64 overload_set;

  bool operator==(const FunctionId&) const = default;

  struct Hash {
    std::size_t operator()(const FunctionId& k) const {
      return (std::hash<std::string>()(k.name) << size_t{k.is_static}) +
             std::hash<u64>()(k.overload_set);
    }
  };
};

struct RecordElement : public TypeElement {
  explicit RecordElement(sus::Vec<Namespace> containing_namespaces,
                         Comment comment, std::string name,
                         sus::Vec<std::string> record_path,
                         RecordType record_type, u32 sort_key)
      : TypeElement(sus::move(containing_namespaces), sus::move(comment),
                    sus::move(name), sus::move(record_path), sort_key),
        record_type(record_type) {}

  // TODO: Template parameters and requires clause.

  // TODO: Link to all base classes.

  RecordType record_type;

  std::unordered_map<RecordId, RecordElement, RecordId::Hash> records;
  std::unordered_map<UniqueSymbol, FieldElement> fields;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> deductions;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> ctors;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> dtors;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> conversions;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> methods;

  bool has_any_comments() const noexcept {
    if (has_comment()) return true;
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

  sus::Option<const CommentElement&> find_method_comment(
      std::string_view comment_loc) const noexcept {
    sus::Option<const CommentElement&> out;
    for (const auto& [u, e] : methods) {
      out = e.find_comment(comment_loc);
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
    return out;
  }

  void for_each_comment(sus::fn::FnMut<void(Comment&)> fn) {
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
  std::unordered_map<NamespaceId, NamespaceElement, NamespaceId::Hash>
      namespaces;
  std::unordered_map<RecordId, RecordElement, RecordId::Hash> records;
  std::unordered_map<FunctionId, FunctionElement, FunctionId::Hash> functions;

  bool has_any_comments() const noexcept {
    if (has_comment()) return true;
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

  void for_each_comment(sus::fn::FnMut<void(Comment&)> fn) {
    fn(comment);
    for (auto& [k, e] : namespaces) e.for_each_comment(fn);
    for (auto& [k, e] : records) e.for_each_comment(fn);
    for (auto& [k, e] : functions) e.for_each_comment(fn);
  }
};

inline NamespaceId key_for_namespace(clang::NamespaceDecl* decl) noexcept {
  return NamespaceId(decl->getNameAsString());
}

inline FunctionId key_for_function(clang::FunctionDecl* decl,
                                   sus::Option<u64> overload_set) noexcept {
  bool is_static = [&]() {
    if (auto* mdecl = clang::dyn_cast<clang::CXXMethodDecl>(decl))
      return mdecl->isStatic();
    return false;
  }();
  return FunctionId(decl->getNameAsString(), is_static,
                    sus::move(overload_set).unwrap_or_default());
}

struct Database {
  NamespaceElement global =
      NamespaceElement(sus::vec(Namespace::with<Namespace::Tag::Global>()),
                       Comment(), std::string(), 0u);

  bool has_any_comments() const noexcept { return global.has_any_comments(); }

  sus::result::Result</* TODO: void */ int, std::string>
  resolve_inherited_comments() {
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
                return sus::result::err(sus::move(s).str());
              }
              if (!target.as_mut<Target::Namespace>().namespaces.contains(id)) {
                std::ostringstream s;
                s << "Inherited comment at " << c->begin_loc
                  << " can't find namespace " << e.as<InheritPathNamespace>();
                return sus::result::err(sus::move(s).str());
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
                  if (e.name == name) return sus::some(mref(e));
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
                  return sus::result::err(sus::move(s).str());
                }
              }
              if (record.is_none()) {
                std::ostringstream s;
                s << "Inherited comment at " << c->begin_loc
                  << " can't find record " << name;
                return sus::result::err(sus::move(s).str());
              }
              target = sus::choice<Target::Record>(sus::move(record).unwrap());
              break;
            }
            case InheritPathFunction: {
              const std::string& name = e.as<InheritPathFunction>();
              auto find_function = [&](const auto& function_map)
                  -> sus::Option<const FunctionElement&> {
                for (const auto& [k, e] : function_map) {
                  if (e.name == name) return sus::some(mref(e));
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
                  return sus::result::err(sus::move(s).str());
                }
              }
              if (function.is_none()) {
                std::ostringstream s;
                s << "Inherited comment at " << c->begin_loc
                  << " can't find function " << name;
                return sus::result::err(sus::move(s).str());
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

    return sus::result::ok(0);
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
      llvm::errs() << "WARNING: Unable to find namespace for type '"
                   << qual.getAsString() << "'\n";
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
