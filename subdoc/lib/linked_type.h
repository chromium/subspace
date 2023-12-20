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

#include "subdoc/lib/path.h"
#include "subdoc/lib/type.h"
#include "sus/choice/choice.h"
#include "sus/collections/vec.h"
#include "sus/option/option.h"

namespace subdoc {

struct ConceptElement;
struct Database;
struct FieldElement;
struct FunctionElement;
struct RecordElement;

enum class TypeRefTag {
  Concept,
  Record,
};
/// A reference to a type in the `Database`.
using TypeRef =
    sus::Choice<sus_choice_types((TypeRefTag::Concept, const ConceptElement&),
                                 (TypeRefTag::Record, const RecordElement&))>;

/// A fully described and printable type, with all its sub-types linked to the
/// database when they exist there and are not marked hidden.
struct LinkedType {
  static LinkedType with_type(Type t, const Database& db) noexcept;

  Type type;
  /// References into the database for every type that makes up `type`.
  Vec<Option<TypeRef>> type_element_refs;

 private:
  enum Construct { CONSTRUCT };
  LinkedType(Construct, Type t, Vec<Option<TypeRef>> v)
      : type(sus::move(t)), type_element_refs(sus::move(v)) {}
};

enum class ConceptRefOrNameTag {
  Ref,
  Name,
};
/// A reference to a concept in the `Database`, or name.
using ConceptRefOrName = sus::Choice<sus_choice_types(
    (ConceptRefOrNameTag::Ref, const ConceptElement&),
    (ConceptRefOrNameTag::Name, std::string))>;

struct LinkedConcept {
  static LinkedConcept with_concept(
      sus::Slice<Namespace> namespace_path, std::string name,
      const Database& db) noexcept;

  ConceptRefOrName ref_or_name;
};

enum class FunctionRefOrNameTag {
  Ref,
  Name,
};
/// A reference to a concept in the `Database`, or name.
using FunctionRefOrName = sus::Choice<sus_choice_types(
    (FunctionRefOrNameTag::Ref, const FunctionElement&),
    (FunctionRefOrNameTag::Name, std::string))>;

struct LinkedFunction {
  static LinkedFunction with_function(
      sus::Slice<Namespace> namespace_path, std::string name,
      const Database& db) noexcept;

  FunctionRefOrName ref_or_name;
};

enum class VariableRefOrNameTag {
  Ref,
  Name,
};
/// A reference to a concept in the `Database`, or name.
using VariableRefOrName = sus::Choice<sus_choice_types(
    (VariableRefOrNameTag::Ref, const FieldElement&),
    (VariableRefOrNameTag::Name, std::string))>;

struct LinkedVariable {
  static LinkedVariable with_variable(
      sus::Slice<Namespace> namespace_path, std::string name,
      const Database& db) noexcept;

  VariableRefOrName ref_or_name;
};

}  // namespace subdoc
