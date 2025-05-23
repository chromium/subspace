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
#include "sus/boxed/macros.h"
#include "sus/macros/lifetimebound.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"

namespace sus::boxed {

/// A concept for generalized type erasure of concepts, allowing use of a
/// concept-satisfying type `T` without knowing the concrete type `T`.
///
/// By providing a virtual type-erasure class that satisifies
/// [`DynConcept`]($sus::boxed::DynConcept) along
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
/// that satisfies this [`DynConcept`]($sus::boxed::DynConcept) concept
/// for all concrete types `ConcreteT`
/// which satisfy the concept `C` being type-erased.
///
/// # Performing the type erasure
///
/// To type-erase a concept-satisfying object (a "concept object")
/// into the heap, use [`Box`]($sus::boxed::Box).
/// For example `Box<DynC>` would hold a type-erased
/// heap-allocated object that is known to satisfy the concept `C`. A
/// [`Box`]($sus::boxed::Box) should
/// always be used when storing the function object beyond the current
/// stack frame, such as in a class data member. It can also be done
/// for ease of working with type-erased concepts.
///
/// ```
/// // This function receives and uses a type-erased concept object.
/// void use_fn(sus::Box<sus::fn::DynFn<void(i32)>> b) { b(2); }
/// ```
///
/// A [`Box`]($sus::boxed::Box) holding a type-erased concept can be
/// constructed with the
/// [`from`]($sus::boxed::Box::from!dync) constructor method. It receives a
/// concept object as an input, and moves it to the heap.
/// Since this satisfies the [`From`]($sus::construct::From) concept, the
/// `Box<DynC>` can also be constructed with type deduction through
/// [`sus::into()`]($sus::construct::into).
///
/// ```
/// auto f = [](i32 i) { fmt::println("{}", i); };
/// // Converts the lambda, which satisfies the `Fn<void(i32)>` concept
/// // into a `Box<DynFn<void(i32)>>` for the function argument.
/// use_fn(sus::into(f));
/// ```
///
/// In performance-sensitive code, it can be necessary to avoid heap
/// allocations while working with type-erased concept objects, or to work with
/// a concept object without taking ownership of it. It is possible
/// to receive a type-erased concept object by reference instead of through a
/// [`Box`]($sus::boxed::Box).
///
/// ```
/// // This function receives and uses a type-erased concept object.
/// void use_fn_ref(const sus::fn::DynFn<void(i32)>& b) { b(2); }
/// ```
///
/// To get a type-erased reference from a concept object, pass it to
/// [`sus::dyn()`]($sus::boxed::dyn). The [`sus::dyn()`]($sus::boxed::dyn)
/// function constructs a type-erasure on the stack and automatically converts
/// to a reference to it.
///
/// ```
/// auto f = [](i32 i) { fmt::println("{}", i); };
/// // Erases the type of the lambda, constructing a type-erased reference to a
/// // `DynFn` to pass as the function argument.
/// use_fn_ref(sus::dyn<sus::fn::DynFn<void(i32)>>(f));
/// ```
///
/// # Type erasure of concepts in the Subspace library
///
/// Some concepts in the Subspace library come with a virtual type-erasure class
/// that satisfies [`DynConcept`]($sus::boxed::DynConcept) and can be
/// type-erased into `Box<DynC>` for the concept `C`:
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
/// ## Implementing concept type-erasure
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
///
/// ## Holding dyn() in a stack variable
///
/// When a function receives a type-erased `DynC&` by reference, it allows the
/// caller to avoid heap allocations should they wish. In the easy case, the
/// caller will simply call `sus::dyn()` directly in the function arguments to
/// construct the `DynC&` reference, which ensures it outlives the function
/// call.
///
/// In a more complicated scenario, the caller may wish to conditionally decide
/// to pass an Option<DynC&> with or without a reference, or to choose between
/// different references. It is not possible to return the result of
/// `sus::dyn()` without creating a dangling stack reference, which will be
/// caught by clang in most cases. This means in particular that lambdas such
/// as those passed to functions like [`Option::map`]($sus::option::Option::map)
/// can not be used to construct the `DynC&` reference.
///
/// In order to ensure the target of the `DynC&` reference outlives the function
/// it can be constructed as a stack variable before calling the function.
/// ```
/// std::srand(sus::cast<unsigned>(std::time(nullptr)));
///
/// auto x = [](sus::Option<sus::fn::DynFn<std::string()>&> fn) {
///   if (fn.is_some())
///     return sus::move(fn).unwrap()();
///   else
///     return std::string("tails");
/// };
///
/// auto heads = [] { return std::string("heads"); };
/// // Type-erased `Fn<std::string()>` that represents `heads`. Placed on the
/// // stack to outlive its use in the `Option` and the call to `x(cb)`.
/// auto dyn_heads = sus::dyn<sus::fn::DynFn<std::string()>>(heads);
/// // Conditionally holds a type-erased reference to `heads`. This requires a
/// // type-erasure that outlives the `cb` variable.
/// auto cb = [&]() -> sus::Option<sus::fn::DynFn<std::string()>&> {
///   if (std::rand() % 2) return sus::some(dyn_heads);
///   return sus::none();
/// }();
///
/// std::string s = x(cb);
///
/// fmt::println("{}", s);  // Prints one of "heads" or "tails.
/// ```
/// It can greatly simplify correctness of code to use owned type-erased
/// concept objects through [`Box`]($sus::boxed::Box), such as
/// `Box<DynFn<std::string()>>` in the above example. Though references can be
/// useful, especially in simple or perf-critical code paths.
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
