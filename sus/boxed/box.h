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
#include <memory>
#include <type_traits>

#include "sus/boxed/__private/string_error.h"
#include "sus/boxed/boxed.h"  // namespace docs.
#include "sus/construct/default.h"
#include "sus/error/error.h"
#include "sus/fn/fn_concepts.h"
#include "sus/fn/fn_dyn.h"
#include "sus/iter/iterator_defn.h"
#include "sus/macros/no_unique_address.h"
#include "sus/macros/pure.h"
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/mem/never_value.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/mem/size_of.h"
#include "sus/cmp/eq.h"
#include "sus/ptr/subclass.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus::boxed {

namespace __private {

template <class Box, class T, bool = sus::iter::IteratorAny<T>>
struct [[_sus_trivial_abi]] BoxBase {};

template <class Box, class T>
struct [[_sus_trivial_abi]] BoxBase<Box, T, true>
    : public sus::iter::IteratorBase<Box,
                                     typename std::remove_cvref_t<T>::Item> {
  using Item = typename std::remove_cvref_t<T>::Item;
};

}  // namespace __private

// TODO: Box has an allocator parameter in Rust but std::unique_ptr does not.
// Which do we do?

/// A heap allocated object.
///
/// A `Box<T>` holds ownership of an object of type `T` on the heap. When `Box`
/// is destroyed, or an rvalue-qualified method is called, the inner heap
/// object is freed.
///
/// `Box` is similar to [`std::unique_ptr`](
/// https://en.cppreference.com/w/cpp/memory/unique_ptr) with some differences,
/// and should be preferred when those differences will benefit you:
/// * Const correctness. A `const Box` treats the inner object as `const` and
///   does not expose mutable access to it.
/// * Never null. A `Box` always holds a value until it is moved-from. It is
///   constructed from a value, not a pointer. Alternatively it can be
///   constructed from the constructor arguments by
///   [`with_args`]($sus::boxed::Box::with_args), like [`std::make_unique`](
///   https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique)
///   but built into the type.
///   A moved-from `Box` may not be used except to be assigned to or destroyed.
///   Using a moved-from `Box` will [`panic`]($sus_panic) and
///   terminate the program rather than operate on a null. This prevents
///   Undefined Behaviour and memory bugs caused by dereferencing null or using
///   null in unintended ways.
/// * Supports up-casting (TODO: and [`down-casting`](
///   https://github.com/chromium/subspace/issues/333)) without leaving
///   requiring a native pointer to be pulled out of the `Box`.
/// * No native arrays, `Box` can hold [`Array`]($sus::collections::Array) or
///   [`std::array`](https://en.cppreference.com/w/cpp/container/array) but
///   avoids API complexity for supporting pointers and native arrays (which
///   decay to pointers) by rejecting native arrays.
/// * Integration with [concept type-erasure]($sus::boxed::DynConcept) for
///   holding and constructing from type-erased objects which satisfy a given
///   concept in a type-safe way and without requiring inheritence. This
///   construction is done through [`From`]($sus::construct::From) and thus
///   can `Box<DynC>` can be constructed in a type-deduced way from any object
///   that satisfies the concept `C` via [`sus::into()`]($sus::construct::into).
///   This only requires that `DynC` is compatible with
///   [`DynConcept`]($sus::boxed::DynConcept) which is the case for all
///   type-erasable concepts in the Subspace library.
/// * Additional integration with Subspace library concepts like
///   [`Error`]($sus::error::Error) and [`Fn`]($sus::fn::Fn) such that `Box`
///   [will satisfy those concepts itself](
///   #box-implements-some-concepts-for-its-inner-type) when holding
///   a type-erased object that satisfies those concepts.
///
/// # Box implements some concepts for its inner type
///
/// The Subspace library provides a number of concepts which support
/// type-erasure through [`DynConcept`]($sus::boxed::DynConcept), and when
/// `Box` is holding these as its value, it may itself implement the concept,
/// forwarding use of the concept through to the inner type.
///
/// The canonical example of this is
/// [`Result`]($sus::result::Result)`<T, Box<`[`DynError`](
/// $sus::error::DynError)`>>`, which allows construction via
/// `sus::err(sus::into(e))` for any `e` that satisfies
/// [`Error`]($sus::error::Error). The error field, now being a `Box` is still
/// usable as an [`Error`]($sus::error::Error) allowing generic code that
/// expects the [`Result`]($sus::result::Result) to hold an
/// [`Error`]($sus::error::Error) to function even though the actual error has
/// been type-erased and no longer needs the functions to be templated on it.
///
/// The following concepts, when type-erased in `Box` will also be satisfied
/// by the `Box` itself, avoiding the need to unwrap the inner type and allowing
/// the `Box` to be used in templated code requiring that concept:
/// * [`Error`]($sus::error::Error)
/// * [`Fn`]($sus::fn::Fn)
/// * [`FnMut`]($sus::fn::FnMut)
/// * [`FnOnce`]($sus::fn::FnOnce)
/// * [`Iterator`]($sus::iter::Iterator)
/// * [`DoubleEndedIterator`]($sus::iter::DoubleEndedIterator)
/// * [`ExactSizeIterator`]($sus::iter::ExactSizeIterator)
template <class T>
class [[_sus_trivial_abi]] Box final : public __private::BoxBase<Box<T>, T> {
  static_assert(!std::is_reference_v<T>, "Box of a reference is not allowed.");
  static_assert(!std::is_array_v<T>,
                "Box<T[N]> is not allowed, use Box<Array<T, N>>");

 public:
  /// Constructs a Box which allocates space on the heap and moves `T` into it.
  /// #[doc.overloads=ctor.convert]
  template <std::convertible_to<T> U>
    requires(!::sus::ptr::SameOrSubclassOf<U*, T*>)
  explicit constexpr Box(U u) noexcept
    requires(::sus::mem::Move<U>)
      : Box(FROM_POINTER, new T(::sus::move(u))) {}

  /// Constructs a Box which allocates space on the heap and moves `T` into it.
  /// #[doc.overloads=ctor.inherit]
  template <class U>
    requires(::sus::ptr::SameOrSubclassOf<U*, T*>)
  explicit constexpr Box(U u) noexcept
    requires(::sus::mem::Move<U>)
      : Box(FROM_POINTER, new U(::sus::move(u))) {}

  /// Destroys the `Box` and frees the heap memory for the inner type `T`.
  ///
  /// Does nothing if the `Box` was moved-from and thus no longer owns a value.
  constexpr ~Box() noexcept {
    if (t_) delete t_;
  }

  /// Constructs `Box<T>` with the default value for the type `T`.
  ///
  /// `Box` can not satisfy [`Default`]($sus::construct::Default) because it
  /// prevents C++ from using `Box` to build recursive types where `T` holds
  /// an `Option<Box<T>>` as a member.
  static constexpr Box with_default() noexcept
    requires(::sus::construct::Default<T>)
  {
    return Box(FROM_POINTER, new T());
  }

  /// Constructs a Box by calling the constructor of `T` with `args`.
  ///
  /// This allows construction of `T` directly on the heap, which is required
  /// for types which do not satisfy [`Move`]($sus::mem::Move). This is a common
  /// case for virtual clases which require themselves to be heap allocated.
  ///
  /// For type-erasure of concept objects using [`DynConcept`](
  /// $sus::boxed::DynConcept), such as [`DynError`]($sus::error::DynError),
  /// the [`from`]($sus::boxed::Box::from!dync) constructor method
  /// can be used to type erase the concept object and move it to the heap.
  template <class... Args>
    requires(std::constructible_from<T, Args && ...>)
  constexpr static Box with_args(Args&&... args) noexcept {
    return Box(FROM_POINTER, new T(::sus::forward<Args>(args)...));
  }

  /// Converts `U` into a [`Box<T>`]($sus::boxed::Box).
  ///
  /// The conversion allocates on the heap and moves `u` into it.
  ///
  /// Satisfies the [`From<U>`]($sus::construct::From) concept for
  /// [`Box<T>`]($sus::boxed::Box) where `U` is convertible to `T` and is not a
  /// subclass of `T`.
  /// #[doc.overloads=convert]
  template <std::convertible_to<T> U>
    requires(!::sus::ptr::SameOrSubclassOf<U*, T*>)
  constexpr static Box from(U u) noexcept
    requires(::sus::mem::Move<U>)
  {
    return Box(FROM_POINTER, new T(::sus::move(u)));
  }
  /// Converts `U` into a [`Box<T>`]($sus::boxed::Box).
  ///
  /// The conversion allocates on the heap and moves `u` into it.
  ///
  /// Satisfies the [`From<U>`]($sus::construct::From) concept for
  /// [`Box<T>`]($sus::boxed::Box) where U is `T` or a subclass of `T`.
  /// #[doc.overloads=inherit]
  template <class U>
    requires(::sus::ptr::SameOrSubclassOf<U*, T*>)
  constexpr static Box from(U u) noexcept
    requires(::sus::mem::Move<U>)
  {
    return Box(FROM_POINTER, new U(::sus::move(u)));
  }

  /// Returns a new box with a clone() of this box’s contents.
  ///
  /// Satisfies the [`Clone`]($sus::mem::Clone) concept if `T` satisfies
  /// [`Clone`]($sus::mem::Clone).
  constexpr Box clone() const noexcept
    requires(::sus::mem::Clone<T>)
  {
    sus_check_with_message(t_, "Box used after move");
    return Box(::sus::clone(*t_));
  }

  /// Copies `source`'s contents into the contained `T` without creating a new
  /// allocation.
  ///
  /// An optimization to reuse the existing storage for
  /// [`clone_into`]($sus::mem::clone_into).
  constexpr void clone_from(const Box& rhs) noexcept
    requires(::sus::mem::Clone<T>)
  {
    sus_check_with_message(t_, "Box used after move");
    ::sus::clone_into(*t_, *rhs);
  }

  /// Satisifes the [`Move`]($sus::mem::Move) concept for
  /// [`Box`]($sus::boxed::Box).
  ///
  /// #[doc.overloads=move]
  constexpr Box(Box&& rhs) noexcept
      : Box(FROM_POINTER, ::sus::move(rhs).into_raw()) {
    sus_check_with_message(t_, "Box used after move");
  }
  template <class U>
    requires(::sus::ptr::SameOrSubclassOf<U*, T*>)
  constexpr Box(Box<U>&& rhs) noexcept
      : Box(FROM_POINTER, ::sus::move(rhs).into_raw()) {
    sus_check_with_message(t_, "Box used after move");
  }
  /// Satisifes the [`Move`]($sus::mem::Move) concept for
  /// [`Box`]($sus::boxed::Box).
  ///
  /// #[doc.overloads=move]
  constexpr Box& operator=(Box&& rhs) noexcept {
    if (t_) delete t_;
    t_ = ::sus::move(rhs).into_raw();
    return *this;
  }
  template <class U>
    requires(::sus::ptr::SameOrSubclassOf<U*, T*>)
  constexpr Box& operator=(Box<U>&& rhs) noexcept {
    if (t_) delete t_;
    t_ = ::sus::move(rhs).into_raw();
    return *this;
  }

  _sus_pure constexpr const T& operator*() const {
    sus_check_with_message(t_, "Box used after move");
    return *t_;
  }
  _sus_pure constexpr T& operator*()
    requires(!std::is_const_v<T>)
  {
    sus_check_with_message(t_, "Box used after move");
    return *t_;
  }

  _sus_pure constexpr const T* operator->() const {
    sus_check_with_message(t_, "Box used after move");
    return t_;
  }
  _sus_pure constexpr T* operator->()
    requires(!std::is_const_v<T>)
  {
    sus_check_with_message(t_, "Box used after move");
    return t_;
  }

  /// For a type-erased `DynC` of a concept `C`, `Box<DynC>` can be
  /// constructed from a type that satisfies `C`.
  ///
  /// This satisfies the [`From<DynC, C>`]($sus::construct::From) concept for
  /// constructing `Box<DynC>` from any type that satisfies the concept `C`. It
  /// allows returning a non-templated type satisfying a concept.
  ///
  /// See [`DynConcept`]($sus::boxed::DynConcept) for more on type erasure of
  /// concept-satisfying types.
  ///
  /// #[doc.overloads=dync]
  template <sus::mem::Move U>
  constexpr static Box from(U u) noexcept
    requires(std::same_as<U, std::remove_cvref_t<U>> &&  //
             DynConcept<T, U> &&                         //
             T::template SatisfiesConcept<U>)
  {
    using DynTyped = T::template DynTyped<U, U>;
    // This implicitly upcasts to the `DynConcept` type `T`.
    return Box(FROM_POINTER, new DynTyped(::sus::move(u)));
  }

  /// A `Box<DynError>` can be constructed from a string, which gets type-erased
  /// into a type that satisfies [`Error`]($sus::error::Error).

  /// Satisfies the [`From<std::string>`]($sus::construct::From) concept for
  /// [`Box`]($sus::boxed::Box)`<`[`DynError`]($sus::error::DynError)`>`. This
  /// conversion moves and type-erases the `std::string` into a heap-alloocated
  /// [`DynError`]($sus::error::DynError).
  ///
  /// #[doc.overloads=dynerror]
  constexpr static Box from(std::string s) noexcept
    requires(std::same_as<T, ::sus::error::DynError>)
  {
    using HeapError = ::sus::error::DynErrorTyped<__private::StringError,
                                                  __private::StringError>;
    auto* heap_e = new HeapError(__private::StringError(::sus::move(s)));
    // This implicitly upcasts to the DynError.
    return Box(FROM_POINTER, heap_e);
  }

  /// Constructs a box from a raw pointer.

  /// After calling this function, the raw pointer is owned by the resulting
  /// Box. Specifically, the Box destructor will call the destructor of T and
  /// free the allocated memory. For this to be safe, the memory must have been
  /// allocated on the heap in the same way as the `Box` type, using
  /// [`operator new`](
  /// https://en.cppreference.com/w/cpp/memory/new/operator_new) and must not be
  /// an array.
  static constexpr Box from_raw(::sus::marker::UnsafeFnMarker,
                                T* raw) noexcept {
    return Box(FROM_POINTER, raw);
  }

  /// Converts `Box` into a mutable reference of the inner type.
  constexpr T& as_mut() & noexcept {
    sus_check_with_message(t_, "Box used after move");
    return *t_;
  }

  /// Converts `Box` into a const reference of the inner type.
  constexpr const T& as_ref() const& noexcept {
    sus_check_with_message(t_, "Box used after move");
    return *t_;
  }
  constexpr const T& as_ref() && noexcept = delete;

  /// Consumes the `Box`, returning the wrapped value.
  ///
  /// This allows the caller to make use of the wrapped object as an rvalue
  /// without leaving a moved-from `T` inside the `Box`.
  constexpr T into_inner() && noexcept
    requires(::sus::mem::Move<T>)
  {
    sus_check_with_message(t_, "Box used after move");
    T ret = ::sus::move(*t_);
    delete ::sus::mem::replace(t_, nullptr);
    return ret;
  }

  /// Consumes the `Box`, returning a wrapped raw pointer.
  ///
  /// The pointer will be properly aligned and non-null.
  ///
  /// After calling this function, the caller is responsible for the memory
  /// previously managed by the `Box`. In particular, the caller should properly
  /// destroy `T` and
  /// [delete](https://en.cppreference.com/w/cpp/memory/new/operator_delete) the
  /// memory, taking into account the alignment if any that would be passed by
  /// `Box` to [`operator new`](
  /// https://en.cppreference.com/w/cpp/memory/new/operator_new).  The easiest
  /// way to do this is to convert the raw pointer back back into a `Box` with
  /// the [`Box::from_raw`]($sus::boxed::Box::from_raw) function, allowing the
  /// `Box` destructor to perform the cleanup.
  ///
  /// Note: this is a not a static function, unlike the matching Rust function
  /// [`Box::into_raw`](https://doc.rust-lang.org/stable/std/boxed/struct.Box.html#method.into_raw),
  /// since in C++ the `Box` can not expose the methods of the inner type
  /// directly.
  ///
  /// # Examples
  /// Converting the raw pointer back into a `Box` with
  /// [`Box::from_raw`]($sus::boxed::Box::from_raw) for automatic cleanup:
  ///
  /// ```
  /// auto x = Box<std::string>("Hello");
  /// auto* ptr = sus::move(x).into_raw();
  /// x = Box<std::string>::from_raw(unsafe_fn, ptr);
  /// ```
  /// Manual cleanup by explicitly running the destructor and deallocating the
  /// memory:
  /// ```
  /// auto x = Box<std::string>("Hello");
  /// auto* p = sus::move(x).into_raw();
  /// delete p;
  /// ```
  constexpr T* into_raw() && noexcept {
    sus_check_with_message(t_, "Box used after move");
    return ::sus::mem::replace(t_, nullptr);
  }

  /// Consumes and leaks the `Box`, returning a mutable reference, `T&`. Note
  /// that the type T must outlive the returned reference.
  ///
  /// This function is mainly useful for data that lives for the remainder of
  /// the program's life. Dropping the returned reference will cause a memory
  /// leak. If this is not acceptable, the reference should first be wrapped
  /// with the [`Box::from_raw`]($sus::boxed::Box::from_raw) method producing a
  /// `Box`. This `Box` can then be dropped which will properly destroy `T` and
  /// release the allocated memory.
  ///
  /// This method is not functionally different than [`into_raw`](
  /// $sus::boxed::Box::into_raw) but expresses a different intent, and returns
  /// a reference type indicating it can not ever return null.
  ///
  /// Note: unlike with the Rust [`Box::leak`](
  /// https://doc.rust-lang.org/stable/std/boxed/struct.Box.html#method.leak)
  /// this is not a static method, since `Box` can not expose the inner type's
  /// methods directly in C++.
  constexpr T& leak() && noexcept { return *::sus::move(*this).into_raw(); }

  /// Compares the inner value of two `Box` objects for equality. This does not
  /// perform pointer equality on the boxes themselves.
  ///
  /// Satisfies [`Eq`]($sus::cmp::Eq) for `Box<T>` if `T` also satisifes [`Eq`](
  /// $sus::cmp::Eq).
  ///
  /// #[doc.overloads=box.eq]
  friend constexpr bool operator==(const Box& lhs, const Box& rhs) noexcept
    requires(::sus::cmp::Eq<T>)
  {
    return lhs.as_ref() == rhs.as_ref();
  }
  /// #[doc.overloads=box.eq]
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const Box<T>& lhs,
                                   const Box<U>& rhs) noexcept {
    return lhs.as_ref() == rhs.as_ref();
  }
  /// #[doc.overloads=box.eq]
  template <class U>
    requires(!::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const Box<T>& lhs,
                                   const Box<U>& rhs) noexcept = delete;

  /// Compares the inner value of two `Box` objects for ordering. This compares
  /// the values pointed to from the `Box`, not the pointers themselves.
  ///
  /// Satisfies the strongest of [`StrongOrd`]($sus::cmp::StrongOrd),
  /// [`Ord`]($sus::cmp::Ord), or [`PartialOrd`]($sus::cmp::PartialOrd), for
  /// `Box<T>` for the strongest ordering that `T` satisifes.
  ///
  /// #[doc.overloads=box.ord]
  friend constexpr std::strong_ordering operator<=>(const Box& lhs,
                                                    const Box& rhs) noexcept
    requires(::sus::cmp::ExclusiveStrongOrd<T>)
  {
    return lhs.as_ref() <=> rhs.as_ref();
  }
  /// #[doc.overloads=box.ord]
  template <class U>
    requires(::sus::cmp::ExclusiveStrongOrd<T, U>)
  friend constexpr std::strong_ordering operator<=>(
      const Box<T>& lhs, const Box<U>& rhs) noexcept {
    return lhs.as_ref() <=> rhs.as_ref();
  }
  /// #[doc.overloads=box.ord]
  friend constexpr std::weak_ordering operator<=>(const Box& lhs,
                                                  const Box& rhs) noexcept
    requires(::sus::cmp::ExclusiveOrd<T>)
  {
    return lhs.as_ref() <=> rhs.as_ref();
  }
  /// #[doc.overloads=box.ord]
  template <class U>
    requires(::sus::cmp::ExclusiveOrd<T, U>)
  friend constexpr std::weak_ordering operator<=>(const Box<T>& lhs,
                                                  const Box<U>& rhs) noexcept {
    return lhs.as_ref() <=> rhs.as_ref();
  }
  /// #[doc.overloads=box.ord]
  friend constexpr std::partial_ordering operator<=>(const Box& lhs,
                                                     const Box& rhs) noexcept
    requires(::sus::cmp::ExclusivePartialOrd<T>)
  {
    return lhs.as_ref() <=> rhs.as_ref();
  }
  /// #[doc.overloads=box.ord]
  template <class U>
    requires(::sus::cmp::ExclusivePartialOrd<T, U>)
  friend constexpr std::partial_ordering operator<=>(
      const Box<T>& lhs, const Box<U>& rhs) noexcept {
    return lhs.as_ref() <=> rhs.as_ref();
  }
  // No ordering.
  /// #[doc.overloads=box.ord]
  template <class U>
    requires(!::sus::cmp::PartialOrd<T, U>)
  friend constexpr auto operator<=>(const Box<T>& lhs,
                                    const Box<U>& rhs) noexcept = delete;

  /// A `Box` holding a type-erased function type will satisfy the fn concepts
  /// and can be used as a function type. It will forward the call through
  /// to the inner type.
  ///
  /// * `Box<`[`DynFn`]($sus::fn::DynFn)`>` satisfies [`Fn`]($sus::fn::Fn).
  /// * `Box<`[`DynFnMut`]($sus::fn::DynFnMut)`>` satisfies
  ///   [`FnMut`]($sus::fn::FnMut).
  /// * `Box<`[`DynFnOnce`]($sus::fn::DynFnOnce)`>` satisfies
  ///   [`FnOnce`]($sus::fn::FnOnce).
  ///
  /// The usual compatibility rules apply, allowing [`DynFn`]($sus::fn::DynFn)
  /// to be treated like [`DynFnMut`]($sus::fn::DynFnMut)
  /// and both of them to be treated like [`DynFnOnce`]($sus::fn::DynFnOnce).
  ///
  /// A `Box<`[`DynFnOnce`]($sus::fn::DynFnOnce)`>` must be moved from
  /// when called, and will destroy the
  /// underlying function object after the call completes.
  ///
  /// # Examples
  ///
  /// A `Box<DynFn>` being used as a function object:
  /// ```
  /// const auto b = Box<sus::fn::DynFn<usize(std::string_view)>>::from(
  ///     &std::string_view::size);
  /// sus_check(b("hello world") == 11u);
  ///
  /// auto mut_b = Box<sus::fn::DynFn<usize(std::string_view)>>::from(
  ///     &std::string_view::size);
  /// sus_check(mut_b("hello world") == 11u);
  ///
  /// sus_check(sus::move(mut_b)("hello world") == 11u);
  /// // The object inside `mut_b` is now destroyed.
  /// ```
  ///
  /// A `Box<DynFnMut>` being used as a function object. It can not be called
  /// on a `const Box` as it requies the ability to mutate, and `const Box`
  /// treats its inner object as const:
  /// ```
  /// auto mut_b = Box<sus::fn::DynFnMut<usize(std::string_view)>>::from(
  ///     &std::string_view::size);
  /// sus_check(mut_b("hello world") == 11u);
  ///
  /// sus_check(sus::move(mut_b)("hello world") == 11u);
  /// // The object inside `mut_b` is now destroyed.
  /// ```
  ///
  /// A `Box<DynFnOnce>` being used as a function object. It must be an rvalue
  /// (either a return from a call or moved with [`move`]($sus::mem::move))
  /// to call through it:
  /// ```
  /// auto b = Box<sus::fn::DynFnOnce<usize(std::string_view)>>::from(
  ///     &std::string_view::size);
  /// sus_check(sus::move(b)("hello world") == 11u);
  ///
  /// auto x = [] {
  ///   return Box<sus::fn::DynFnOnce<usize(std::string_view)>>::from(
  ///       &std::string_view::size);
  /// };
  /// sus_check(x()("hello world") == 11u);
  /// ```
  // TODO: https://github.com/llvm/llvm-project/issues/65904
  // clang-format off
  template <class... Args>
  constexpr sus::fn::Return<T, Args...> operator()(Args&&... args) const&
    requires(T::IsDynFn &&
             ::sus::fn::Fn<T, sus::fn::Return<T, Args...>(Args...)>)
  {
    sus_check_with_message(t_, "Box used after move");
    return ::sus::fn::call(*t_, ::sus::forward<Args>(args)...);
  }

  template <class... Args>
  constexpr sus::fn::ReturnMut<T, Args...> operator()(Args&&... args) &
    requires(T::IsDynFn &&
             ::sus::fn::FnMut<T, sus::fn::ReturnMut<T, Args...>(Args...)>)
  {
    sus_check_with_message(t_, "Box used after move");
    return ::sus::fn::call_mut(*t_, ::sus::forward<Args>(args)...);
  }

  template <class... Args>
  constexpr sus::fn::ReturnOnce<T, Args...> operator()(Args&&... args) &&
    requires(T::IsDynFn &&  //
             ::sus::fn::FnOnce<T, sus::fn::ReturnOnce<T, Args...>(Args...)>)
  {
    sus_check_with_message(t_, "Box used after move");
    struct Cleanup {
      constexpr ~Cleanup() noexcept { delete t; }
      T* t;
    };
    // Note: This function does not require `Move<T>` because while it's calling
    // an rvalue `operator()` on `T` it is not constructing a `T` through a
    // move.
    return ::sus::fn::call_once(
        sus::move(*Cleanup(::sus::mem::replace(t_, nullptr)).t),
                           ::sus::forward<Args>(args)...);
  }
  ;
  // clang-format on

  /// Implements [`Iterator`]($sus::iter::Iterator) if `T` is an [`Iterator`](
  /// $sus::iter::Iterator), forwarding through to the inner `T` object.
  auto next() noexcept
    requires(::sus::iter::IteratorAny<T>)
  {
    return (**this).next();
  }
  /// Implements [`Iterator`]($sus::iter::Iterator) if `T` is an [`Iterator`](
  /// $sus::iter::Iterator), forwarding through to the inner `T` object.
  ::sus::iter::SizeHint size_hint() const noexcept
    requires(::sus::iter::IteratorAny<T>)
  {
    return (**this).size_hint();
  }
  /// Implements [`DoubleEndedIterator`]($sus::iter::DoubleEndedIterator) if
  /// `T` is a [`DoubleEndedIterator`]($sus::iter::DoubleEndedIterator),
  /// forwarding through to the inner `T` object.
  auto next_back() noexcept
    requires(::sus::iter::IteratorAny<T> &&  //
             ::sus::iter::DoubleEndedIterator<T, typename T::Item>)
  {
    return (**this).next_back();
  }
  /// Implements [`ExactSizeIterator`]($sus::iter::ExactSizeIterator) if
  /// `T` is an [`ExactSizeIterator`]($sus::iter::ExactSizeIterator),
  /// forwarding through to the inner `T` object.
  usize exact_size_hint() const noexcept
    requires(::sus::iter::IteratorAny<T> &&  //
             ::sus::iter::ExactSizeIterator<T, typename T::Item>)
  {
    return (**this).exact_size_hint();
  }
  /// sus::iter::TrustedLen trait.
  /// #[doc.hidden]
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept
    requires(::sus::iter::IteratorAny<T> &&  //
             ::sus::iter::TrustedLen<T>)
  {
    return {};
  }

 private:
  enum FromPointer { FROM_POINTER };
  constexpr explicit Box(FromPointer, T* t) noexcept : t_(t) {}

  T* t_;

  static T* never_value() {
    return reinterpret_cast<T*>(alignof(T));
  }

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(t_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, Box, t_, never_value(),
                              nullptr);
  explicit Box(::sus::mem::NeverValueConstructor) noexcept
      : t_(never_value()) {}
};

}  // namespace sus::boxed

// Satisfies
// [`sus::error::Error`]($sus::error::Error) for
// heap-allocated type-erased errors
// [`Box`]($sus::boxed::Box)`<`[`DynError`]($sus::error::DynError)`>`.
template <::sus::error::Error T>
struct sus::error::ErrorImpl<::sus::boxed::Box<T>> {
  constexpr static std::string display(const ::sus::boxed::Box<T>& b) noexcept {
    return ::sus::error::error_display(*b);
  }
  constexpr static sus::Option<const DynError&> source(
      const ::sus::boxed::Box<T>& b) noexcept {
    return ::sus::error::error_source(*b);
  }
};

// fmt support.
template <class T, class Char>
struct fmt::formatter<::sus::boxed::Box<T>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::boxed::Box<T>& t,
                        FormatContext& ctx) const {
    return underlying_.format(*t, ctx);
  }

 private:
  ::sus::string::__private::AnyFormatter<T, Char> underlying_;
};

// Stream support.
_sus_format_to_stream(sus::boxed, Box, T);

// Promote `Box` into the `sus` namespace.
namespace sus {
using ::sus::boxed::Box;
}
