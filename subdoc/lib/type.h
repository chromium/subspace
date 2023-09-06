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

#include "subdoc/llvm.h"
#include "sus/choice/choice.h"
#include "sus/collections/vec.h"
#include "sus/fn/fn_ref.h"
#include "sus/prelude.h"

namespace subdoc {

struct TypeOrValue;

struct Qualifier {
  bool is_const;
  bool is_volatile;

  friend bool operator==(const Qualifier&, const Qualifier&) = default;
};
static_assert(sus::ops::Eq<Qualifier>);

enum class Refs {
  LValueRef,
  None,
  RValueRef,
};

enum class TypeCategory {
  /// A concrete type or template specialization.
  Type,
  /// A concept.
  Concept,
  /// A reference to a template variable.
  TemplateVariable,
};

struct Type {
  TypeCategory category;
  /// Namespaces the type is nested in, ordered from closest to furthest. An
  /// empty string indicates an anonymous namespace. The global namespace is not
  /// represented.
  sus::Vec<std::string> namespace_path;
  /// Records the type is nested in, ordered from closest to furthest.
  sus::Vec<std::string> record_path;
  /// The name of the type. For `category == TemplateVariable` this will be the
  /// the name of the variable.
  std::string name;
  /// For types of the form `A::B::C` the `nested_name` would hold `B` and `C`.
  sus::Vec<TypeOrValue> nested_names;
  /// Refs can only appear on the outermost type.
  Refs refs;
  /// Const-volatile qualifiers for the outermost type.
  Qualifier qualifier;
  /// The qualifiers of each level of pointer indirection. Empty if the type is
  /// not a pointer. The order is reversed from the order that they are applied,
  /// to optimize for display.
  ///
  /// `T *const<1st *const<2nd *const<3rd`.
  ///
  sus::Vec<Qualifier> pointers;
  /// The dimension of each level of an array, if any. An empty string
  /// represents an unsized dimension (like `int a[]`). They are ordered left
  /// to right.
  sus::Vec<std::string> array_dims;
  /// Recursive structure, each template param is another type, or value.
  sus::Vec<TypeOrValue> template_params;
  /// When true, the type is a parameter pack, and should append `...`.
  bool is_pack;
};

/// A function proto is a template parameter that names a function signature
/// like `int(char*)` in `T<i32(char*)>`.
struct FunctionProtoType {
  Type return_type;
  sus::Vec<Type> param_types;
};

enum class TypeOrValueTag {
  Type,
  FunctionProto,
  Value,
};

// clang-format off
using TypeOrValueChoice = sus::Choice<sus_choice_types(
    (TypeOrValueTag::Type, Type),
    (TypeOrValueTag::FunctionProto, FunctionProtoType),
    (TypeOrValueTag::Value, std::string /* The value as text*/))>;
// clang-format on

struct TypeOrValue {
  /// We can't forward declare a Choice type, as it needs to know the size of the
  /// things it can hold, when we declare it, so we forward declare the
  /// `TypeOrValue` struct and put a Choice inside it.
  TypeOrValueChoice choice;
};

/// Builds a Type structure from `qualtype` without looking through type
/// aliases.
Type build_local_type(clang::QualType qualtype, const clang::SourceManager& sm,
                      clang::Preprocessor& preprocessor) noexcept;

struct TypeToStringQuery {
  sus::Slice<std::string> namespace_path;
  sus::Slice<std::string> record_path;
  std::string_view name;
};

/// Produces a text representation of the type, allowing a callback to be
/// executed for each type encountered. Text in between types is emitted to the
/// `text_fn`, and the types are emitted to `type_fn`. The `type_fn` callback
/// can use `ToStringQuery::name` to just forward the name along as text.
///
/// The `var_name_fn` is called at the place where the variable name (if any)
/// would appear.
void type_to_string(
    const Type& type, sus::fn::FnMutRef<void(std::string_view)> text_fn,
    sus::fn::FnMutRef<void(TypeToStringQuery)> type_fn,
    sus::fn::FnMutRef<void()> const_qualifier_fn,
    sus::fn::FnMutRef<void()> volatile_qualifier_fn,
    sus::Option<sus::fn::FnOnceRef<void()>> var_name_fn) noexcept;

/// Like `type_to_string` but just walks through the types and does not produce
/// any output.
void type_walk_types(
    const Type& type,
    sus::fn::FnMutRef<void(TypeToStringQuery)> type_fn) noexcept;

}  // namespace subdoc
