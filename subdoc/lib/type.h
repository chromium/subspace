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

enum class Nullness {
  Allowed,
  Disallowed,
  Unknown,
};

struct Qualifier {
  /// Creates `Qualifier` with neither const nor volatile set, and with
  /// nullness set to `Nullness::Unknown`.
  constexpr Qualifier() = default;
  /// Creates `Qualifier` with const set.
  constexpr static Qualifier with_const() noexcept {
    return Qualifier().set_const(true);
  }
  /// Creates `Qualifier` with volatile set.
  constexpr static Qualifier with_volatile() noexcept {
    return Qualifier().set_volatile(true);
  }
  /// Creates `Qualifier` with both const and volatile set.
  constexpr static Qualifier with_cv() noexcept {
    return Qualifier().set_const(true).set_volatile(true);
  }

  /// Creates a new `Qualifier` from this with const set to `c`.
  constexpr Qualifier set_const(bool c) noexcept {
    Qualifier q = *this;
    q.is_const = c;
    return q;
  }
  /// Creates a new `Qualifier` from this with volatile set to `v`.
  constexpr Qualifier set_volatile(bool v) noexcept {
    Qualifier q = *this;
    q.is_volatile = v;
    return q;
  }
  /// Creates a new `Qualifier` from this with nullness set to `n`.
  constexpr Qualifier set_nullness(Nullness n) noexcept {
    Qualifier q = *this;
    q.nullness = n;
    return q;
  }

  bool is_const = false;
  bool is_volatile = false;
  Nullness nullness = Nullness::Unknown;

  friend bool operator==(const Qualifier&, const Qualifier&) = default;
};
static_assert(sus::ops::Eq<Qualifier>);
}  // namespace subdoc

template <>
struct fmt::formatter<subdoc::Nullness> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(const subdoc::Nullness& t, FormatContext& ctx) const {
    using enum subdoc::Nullness;
    auto out = ctx.out();
    switch (t) {
      case Allowed: out = fmt::format_to(out, "Allowed"); break;
      case Disallowed: out = fmt::format_to(out, "Disallowed"); break;
      case Unknown: out = fmt::format_to(out, "Unknown"); break;
    }
    ctx.advance_to(out);
    return out;
  }
};

template <>
struct fmt::formatter<subdoc::Qualifier> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(const subdoc::Qualifier& t, FormatContext& ctx) const {
    auto out = ctx.out();
    out = fmt::format_to(out, "Qualifier(");
    ctx.advance_to(out);
    u32 count;
    if (t.is_const) {
      out = fmt::format_to(out, "c");
      ctx.advance_to(out);
      count += 1u;
    }
    if (t.is_volatile) {
      if (count > 0u) {
        out = fmt::format_to(out, ", ");
        ctx.advance_to(out);
      }
      out = fmt::format_to(out, "v");
      ctx.advance_to(out);
      count += 1u;
    }
    if (t.nullness != subdoc::Nullness::Unknown) {
      if (count > 0u) {
        out = fmt::format_to(out, ", ");
        ctx.advance_to(out);
      }
      if (t.nullness == subdoc::Nullness::Allowed) {
        out = fmt::format_to(out, "nullable");
        ctx.advance_to(out);
      } else {
        out = fmt::format_to(out, "nonnull");
        ctx.advance_to(out);
      }
    }
    out = fmt::format_to(out, ")");
    ctx.advance_to(out);
    return out;
  }
};

namespace subdoc {
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
  /// References can only be applied to the outermost type. While most of the
  /// `Type` structure refers to the innermost type (the deepest pointee, a
  /// non-pointer), this refers to the outermost type (the first pointer in
  /// `int***`).
  Refs refs;
  /// Const-volatile qualifiers for the outermost type.
  Qualifier qualifier;
  /// The qualifiers of each level of pointer indirection. Empty if the type is
  /// not a pointer. The order is reversed from the order that they are applied,
  /// to optimize for display. The qualifiers for the inner most type are stored
  /// on the `Type`.
  ///
  /// `const T *const<1st *const<2nd *const<3rd`.
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
