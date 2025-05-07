// Copyright 2025 Google LLC
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

/// Macro to help implement `DynC` for a concept `C`. The macro is placed in the
/// body of the `DynC` class.
///
/// Here `DynC` is used as a placeholder name to refer to the virtual class
/// that type-erases for the concept `C`. The type erasure class is typically
/// named to match the concept, with a "Dyn" prefix. The type-aware subclass
/// of the type erasure class is typically named to match the concept with a
/// "Dyn" prefix and a "Typed" suffix.
///
/// The `Concept` parameter is the concept `C` for which types are being
/// type-erased.
///
/// The `DynConcept` parameter is the name of the type-erasure
/// class `DynC` which the macro is written within, and which has a pure virtual
/// interface matching the concept's requirements.
///
/// The `DynConceptTyped`
/// parameter is the type-aware subclass of `DynC` which contains the
/// `sus_dyn_concept_typed` macro in its body, and the
/// implementation of the virtual interface that forwards calls through to the
/// concrete type.
///
/// See [`DynConcept`]($sus::boxed::DynConcept) for more on type erasure of
/// concept-satisfying types, and [DynConcept examples](
/// $sus::boxed::DynConcept#examples) for examples of using the macro.
#define sus_dyn_concept(Concept, DynConcept, DynConceptTyped)  \
 public:                                                       \
  template <class ConcreteT>                                   \
  static constexpr bool SatisfiesConcept = Concept<ConcreteT>; \
  template <class ConcreteT, class Store>                      \
  using DynTyped = DynConceptTyped<ConcreteT, Store>;          \
                                                               \
  DynConcept() = default;                                      \
  virtual ~DynConcept() = default;                             \
  DynConcept(DynConcept&&) = delete;                           \
  DynConcept& operator=(DynConcept&&) = delete

/// Macro to help implement `DynCTyped` for a concept `C`. The macro is placed
/// in the body of the `DynCTyped` class.
///
/// See the TODO: link [`sus_dyn_concept`] macro for more, and
/// [DynConcept examples]($sus::boxed::DynConcept#examples) for examples
/// of using the macro.
#define sus_dyn_concept_typed(Concept, DynConcept, DynConceptTyped, VarName)  \
 public:                                                                      \
  static_assert(Concept<T>);                                                  \
  constexpr DynConceptTyped(Store&& c) : VarName(::sus::forward<Store>(c)) {} \
                                                                              \
 private:                                                                     \
  Store VarName;
