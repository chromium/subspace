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

#include <concepts>
#include <type_traits>

#include "sus/boxed/boxed.h"  // namespace docs.
#include "sus/macros/lifetimebound.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"

namespace sus::boxed {

/// A concept for generalized type erasure of concepts, allowing use of a
/// concept-satisfying type `T` without knowing the concrete type `T`.
///
/// By providing a virtual type-erasure class that satisifies `DynConcept` along
/// with a concept `C`, it allows the use of generic concept-satisfying objects
/// without the use of templates.
/// This means a function accepting a concept-satisying object as
/// a parameter can:
/// * Be written outside of the header.
/// * Be a virtual method.
/// * Be part of a non-header-only library API.
///
/// We can not specify a concept as a type parameter so there is a level of
/// indirection involved and the concept itself is not named.
///
/// To type-erase a concept `C`, there must be a virtual class `DynC` declared
/// that satisfies this `DynConcept` concept for all concrete types `ConcreteT`
/// which satisfy the concept `C` being type-erased.
///
/// # Performing the type erasure
///
/// To type-erase a concept-satisfying object into the heap, use
/// [`Box`]($sus::boxed::Box), such as `Box<DynC>` to hold a type-erased
/// heap-allocated object that is known to satisfy the concept `C`.
///
/// Then, `Box<DynC>` will come with a method
/// [`from`]($sus::boxed::Box::from!dync) that lifts any object satisfying the
/// concept `C` into the heap and type-erases it to `DynC`. This implies that
/// [`Box<DynC>`]($sus::boxed::Box) will satisfy the
/// [`From`]($sus::construct::From) concept, and can be constructed with type
/// deduction through [`sus::into()`]($sus::construct::into) or
/// explicitly with `Box<DynC>::from(sus::move(satisfies_c))`.
///
/// To type-erase a concept-satisfying object into a `DynC` reference without
/// heap allocation, use [`dyn`]($sus::boxed::dyn), such as `dyn<DynC>(x)` to
/// construct a type-erased reference to an object `x` that is known to satisfy
/// the concept `C`.
///
/// The [`dyn`]($sus::boxed::dyn) function, and its returned type should only
/// appear in function call arguments. The returned type
/// [`Dyn`]($sus::boxed::Dyn) can not be moved, and it only converts a
/// reference to a type `T&` into a reference `DynC&` that also satisfies `C`
/// but without templates.
/// This can be used to call functions that accept a type-erased concept
/// by reference, such as with a `const DynC&` parameter.
///
/// # Type erasure of concepts in the Subspace library
///
/// Some concepts in the Subspace library come with a virtual type-erasure class
/// that satisfies `DynConcept` and can be type-erased into `Box<DynC>` for the
/// concept `C`:
/// * [`Error`]($sus::error::Error)
/// * [`Fn`]($sus::fn::Fn)
/// * [`FnMut`]($sus::fn::FnMut)
/// * [`FnOnce`]($sus::fn::FnOnce)
///
/// For some concepts in the Subspace library, `Box<DynC>` will also satisfy the
/// concept `C` itself, without having use the inner type. See
/// [Box implements some concepts for its inner type](
/// $sus::boxed::Box#box-implements-some-concepts-for-its-inner-type).
///
/// Since the actual type is erased, it can not be moved or copied. While it can
/// be constructed on the stack or the heap, any access to it other than its
/// initial declaration must be through a pointer or reference, similar to
/// [`Pin<T>`](https://doc.rust-lang.org/std/pin/struct.Pin.html) types in
/// Rust.
///
/// # Examples
///
/// Providing the mechanism to type erase objects that satisfy a concept named
/// `MyConcept` through a `DynMyConcept` class:
/// ```
/// // A concept which requires a single const-access method named `concept_fn`.
/// template <class T>
/// concept MyConcept = requires(const T& t) {
///   { t.concept_fn() } -> std::same_as<void>;
/// };
///
/// template <class T, class Store>
/// class DynMyConceptTyped;
///
/// class DynMyConcept {
///   sus_dyn_concept(MyConcept, DynMyConcept, DynMyConceptTyped);
///
///  public:
///   // Pure virtual concept API.
///   virtual void concept_fn() const = 0;
/// };
/// // Verifies that DynMyConcept also satisfies MyConcept, which is required.
/// static_assert(MyConcept<DynMyConcept>);
///
/// template <class T, class Store>
/// class DynMyConceptTyped final : public DynMyConcept {
///   sus_dyn_concept_typed(MyConcept, DynMyConcept, DynMyConceptTyped, v);
///
///   // Virtual concept API implementation.
///   void concept_fn() const override { return v.concept_fn(); };
/// };
///
/// // A type which satiesfies `MyConcept`.
/// struct MyConceptType {
///   void concept_fn() const {}
/// };
///
/// int main() {
///   // Verifies that DynMyConcept is functioning correctly, testing it against
///   // a type that satisfies MyConcept.
///   static_assert(sus::boxed::DynConcept<DynMyConcept, MyConceptType>);
///
///   auto b = [](Box<DynMyConcept> c) { c->concept_fn(); };
///   // `Box<DynMyConcept>` constructs from `MyConceptType`.
///   b(sus::into(MyConceptType()));
///
///   auto d = [](const DynMyConcept& c) { c.concept_fn(); };
///   // `MyConceptType` converts to `const MyConcept&` with `sus::dyn()`.
///   d(sus::dyn<const DynMyConcept>(MyConceptType()));
/// }
/// ```
///
/// An identical example to above, with a `DynMyConcept` class providing type
/// erasure for the `MyConcept` concept, however without the use of the helper
/// macros, showing all the required machinery:
/// ```
/// // A concept which requires a single const-access method named `concept_fn`.
/// template <class T>
/// concept MyConcept = requires(const T& t) {
///   { t.concept_fn() } -> std::same_as<void>;
/// };
///
/// template <class T, class Store>
/// class DynMyConceptTyped;
///
/// class DynMyConcept {
///  public:
///   // Pure virtual concept API.
///   virtual void concept_fn() const = 0;
///   template <class T>
///
///   static constexpr bool SatisfiesConcept = MyConcept<T>;
///   template <class T, class Store>
///   using DynTyped = DynMyConceptTyped<T, Store>;
///
///   DynMyConcept() = default;
///   virtual ~DynMyConcept() = default;
///   DynMyConcept(DynC&&) = delete;
///   DynMyConcept& operator=(DynMyConcept&&) = delete;
/// };
/// // Verifies that DynMyConcept also satisfies MyConcept, which is required.
/// static_assert(MyConcept<DynMyConcept>);
///
/// template <class T, class Store>
/// class DynMyConceptTyped final : public DynMyConcept {
///  public:
///   // Virtual concept API implementation.
///   void concept_fn() const override { return c_.concept_fn(); };
///
///   constexpr DynMyConceptTyped(Store&& c) : c_(::sus::forward<Store>(c)) {}
///
///  private:
///   Store c_;
/// };
///
/// // A type which satiesfies `MyConcept`.
/// struct MyConceptType {
///   void concept_fn() const {}
/// };
///
/// int main() {
///   // Verifies that DynMyConcept is functioning correctly, testing it against
///   // a type that satisfies MyConcept.
///   static_assert(sus::boxed::DynConcept<DynMyConcept, MyConceptType>);
///
///   auto b = [](Box<DynMyConcept> c) { c->concept_fn(); };
///   // `Box<DynMyConcept>` constructs from `MyConceptType`.
///   b(sus::into(MyConceptType()));
///
///   auto d = [](const DynMyConcept& c) { c.concept_fn(); };
///   // `MyConceptType` converts to `const MyConcept&` with `sus::dyn()`.
///   d(sus::dyn<const DynMyConcept>(MyConceptType()));
/// }
/// ```
template <class DynC, class ConcreteT>
concept DynConcept = requires {
  // The types are not qualified or references.
  requires std::same_as<DynC, std::remove_cvref_t<DynC>>;
  requires std::same_as<ConcreteT, std::remove_cvref_t<ConcreteT>>;

  // The SatisfiesConcept bool tests against the concept.
  { DynC::template SatisfiesConcept<ConcreteT> } -> std::same_as<const bool&>;
  // The `DynTyped` type alias names the typed subclass. The `DynTyped` class
  // has two template parameters, the concrete type and the storage type (value
  // or reference).
  typename DynC::template DynTyped<ConcreteT, ConcreteT>;

  // The type-erased `DynC` must also satisfy the concept, so it can be used
  // in templated code still as well.
  requires DynC::template SatisfiesConcept<DynC>;

  // The typed class is a subclass of the type-erased `DynC` base class, and is
  // final.
  requires std::is_base_of_v<
      DynC, typename DynC::template DynTyped<ConcreteT, ConcreteT>>;
  requires std::is_final_v<
      typename DynC::template DynTyped<ConcreteT, ConcreteT>>;

  // The type-erased `DynC` can not be moved (which would slice the typed
  // subclass off).
  requires !std::is_move_constructible_v<DynC>;
  requires !std::is_move_assignable_v<DynC>;
};

/// A type erasure of a type satisfying a concept, which can be used as a
/// reference without heap allocation or templates.
/// Returned from [`dyn`]($sus::boxed::dyn).
///
/// This type is similar to `Box<DynC>` for purposes of type erasure but does
/// not require heap allocation,
/// and it converts directly to a reference to the erased type.
///
/// Use [`dyn`]($sus::boxed::dyn) to convert to a `DynC` reference instead of
/// constructing this type directly.
///
/// See [`DynConcept`]($sus::boxed::DynConcept) for more on type erasure of
/// concept-satisfying types.
template <class DynC, class ConcreteT>
class [[nodiscard]] Dyn {
 public:
  static_assert(std::same_as<const DynC, const std::remove_cvref_t<DynC>>,
                "DynC can be const-qualified but not a reference");
  static_assert(std::same_as<ConcreteT, std::remove_cvref_t<ConcreteT>>,
                "ConcreteT can not be qualified or a reference");

  static_assert(
      DynC::template SatisfiesConcept<ConcreteT>,
      "ConcreteT must satisfy the concept that DynC type-erases for.");

  /// Construct a `Dyn<DynC, T>` from a mutable reference to `T`, which will
  /// vend a mutable reference `DynC&`.
  ///
  /// #[doc.overloads=mut]
  Dyn(ConcreteT& concrete sus_lifetimebound)
    requires(!std::is_const_v<DynC>)
      : dyn_(concrete) {}
  /// #[doc.overloads=mut]
  Dyn(ConcreteT&& concrete sus_lifetimebound)
    requires(!std::is_const_v<DynC>)
      : dyn_(concrete) {}
  /// Construct a `Dyn<const DynC, T>` from a reference to `T`, which will
  /// vend a const reference `const DynC&`.
  ///
  /// #[doc.overloads=const]
  Dyn(const ConcreteT& concrete sus_lifetimebound)
    requires(std::is_const_v<DynC>)
      // This drops the const qualifier on `concrete` however we have a const
      // qualifier on `DynC` (checked by the requires clause on this
      // constructor) which prevents the `concrete` from being accessed in a
      // non-const way through the `operator DynC&` overloads.
      : dyn_(const_cast<ConcreteT&>(concrete)) {}

  /// Converts the reference to `ConcreteT` into a `DynC` reference.
  operator const DynC&() const& { return dyn_; }
  operator DynC&() &
    requires(!std::is_const_v<DynC>)
  {
    return dyn_;
  }
  operator DynC&() &&
    requires(!std::is_const_v<DynC>)
  {
    return dyn_;
  }
  operator DynC&&() &&
    requires(!std::is_const_v<DynC>)
  {
    return ::sus::move(dyn_);
  }

  /// `Dyn` can not be moved.
  ///
  /// `Dyn` only exists as a temporary to convert a
  /// concrete reference to a concept type to into a type-erased reference.
  Dyn(Dyn&&) = delete;
  /// `Dyn` can not be moved.
  ///
  /// `Dyn` only exists as a temporary to convert a
  /// concrete reference to a concept type to into a type-erased reference.
  Dyn& operator=(Dyn&&) = delete;

 private:
  /// The typed subclass of `DynC` which holds the reference to `ConcreteT`.
  typename DynC::template DynTyped<ConcreteT, ConcreteT&> dyn_;
};

/// Type erases a reference to a type `T&` which satisfies a concept `C`,
/// into a reference `DynC&` that also satisfies `C` but without templates.
///
/// Use `dyn<DynC>(x)` to convert a mutable reference to `x` into `DynC&` and
/// `dyn<const DynC>(x)` to convert a const or mutable reference to `x` into
/// `const Dyn&`.
///
/// Type erasure into `DynC` allows calling a method that receives a `DynC`
/// reference, such as `const DynC&`, without requiring a heap allocation into
/// a `Box<DynC>`.
///
/// See [`DynConcept`]($sus::boxed::DynConcept) for more on type erasure of
/// concept-satisfying types.
template <class DynC, class ConcreteT>
  requires(std::same_as<DynC, std::remove_cvref_t<DynC>> &&  //
           std::same_as<ConcreteT, std::remove_cvref_t<ConcreteT>>)
constexpr Dyn<DynC, ConcreteT> dyn(ConcreteT& t sus_lifetimebound) noexcept {
  return Dyn<DynC, ConcreteT>(t);
}
template <class DynC, class ConcreteT>
  requires(std::same_as<DynC, std::remove_cvref_t<DynC>> &&            //
           std::same_as<ConcreteT, std::remove_cvref_t<ConcreteT>> &&  //
           std::is_rvalue_reference_v<ConcreteT &&>)
constexpr Dyn<DynC, ConcreteT> dyn(ConcreteT&& t sus_lifetimebound) noexcept {
  return Dyn<DynC, ConcreteT>(t);
}
template <class DynC, class ConcreteT>
  requires(std::same_as<const DynC, const std::remove_cvref_t<DynC>> &&
           std::same_as<ConcreteT, std::remove_cvref_t<ConcreteT>> &&
           std::is_const_v<DynC>)
constexpr Dyn<DynC, ConcreteT> dyn(
    const ConcreteT& t sus_lifetimebound) noexcept {
  return Dyn<DynC, ConcreteT>(t);
}

}  // namespace sus::boxed

// Promote `dyn` to the top `sus` namespace.
namespace sus {
using sus::boxed::dyn;
}

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
