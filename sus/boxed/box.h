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
#include "sus/error/error.h"
#include "sus/fn/fn_concepts.h"
#include "sus/fn/fn_dyn.h"
#include "sus/macros/no_unique_address.h"
#include "sus/macros/pure.h"
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/mem/never_value.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/mem/size_of.h"
#include "sus/ptr/subclass.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus::boxed {

// TODO: Box has an allocator parameter in Rust but std::unique_ptr does not.
// Which do we do?

/// A heap allocated object.
///
/// # Box implements some concepts for its inner type
/// For some concepts in the Subspace library, if `T` satisfies the concept,
/// then `Box<T>` will as well, forwarding through to the inner heap-allocated
/// object. Those concepts are:
/// * [`Error`]($sus::error::Error).
template <class T>
class [[sus_trivial_abi]] Box final {
  static_assert(!std::is_reference_v<T>, "Box of a reference is not allowed.");
  static_assert(sus::mem::size_of<T>() != 0u);  // Ensure that `T` is defined.

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

  constexpr ~Box() noexcept {
    if (t_) delete t_;
  }

  /// Converts `U` into a [`Box<T>`]($sus::boxed::Box).
  ///
  /// The conversion allocates on the heap and moves `u` into it.
  ///
  /// Satisfies the [`From<U>`]($sus::construct::From) concept for
  /// [`Box<T>`]($sus::boxed::Box) where `U` is convertible to `T` and is not a
  /// subclass of `T`.
  /// #[doc.overloads=from.convert]
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
  /// #[doc.overloads=from.inherit]
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
    ::sus::check_with_message(t_, "Box used after move");
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
    ::sus::check_with_message(t_, "Box used after move");
    ::sus::clone_into(*t_, *rhs);
  }

  /// Satisifes the [`Move`]($sus::mem::Move) concept for
  /// [`Box`]($sus::boxed::Box).
  ///
  /// #[doc.overloads=move]
  constexpr Box(Box&& rhs) noexcept
      : Box(FROM_POINTER, ::sus::move(rhs).into_raw()) {
    ::sus::check_with_message(t_, "Box used after move");
  }
  template <class U>
    requires(::sus::ptr::SameOrSubclassOf<U*, T*>)
  constexpr Box(Box<U>&& rhs) noexcept
      : Box(FROM_POINTER, ::sus::move(rhs).into_raw()) {
    ::sus::check_with_message(t_, "Box used after move");
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

  sus_pure constexpr const T& operator*() const {
    ::sus::check_with_message(t_, "Box used after move");
    return *t_;
  }
  sus_pure constexpr T& operator*()
    requires(!std::is_const_v<T>)
  {
    ::sus::check_with_message(t_, "Box used after move");
    return *t_;
  }

  sus_pure constexpr const T* operator->() const {
    ::sus::check_with_message(t_, "Box used after move");
    return t_;
  }
  sus_pure constexpr T* operator->()
    requires(!std::is_const_v<T>)
  {
    ::sus::check_with_message(t_, "Box used after move");
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
  /// #[doc.overloads=dynerror.from.string]
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
    ::sus::check_with_message(t_, "Box used after move");
    return ::sus::mem::replace(t_, nullptr);
  }

  /// Consumes the `Box`, calling `f` with the wrapped value before destroying
  /// it.
  ///
  /// This allows the caller to make use of the wrapped object as an rvalue
  /// without moving out of the wrapped object in a way that leaves the Box with
  /// a moved-from object within.
  template <::sus::fn::FnOnce<sus::fn::Anything(T&&)> F>
  constexpr ::sus::fn::ReturnOnce<F, T&&> consume(F f) && noexcept {
    ::sus::check_with_message(t_, "Box used after move");
    ::sus::fn::ReturnOnce<F, T&&> ret =
        ::sus::fn::call_once(::sus::move(f), sus::move(*t_));
    delete ::sus::mem::replace(t_, nullptr);
    return ret;
  }

  template <class... Args>
  constexpr sus::fn::Return<T, Args...> operator()(
      Args&&... args) const noexcept
    requires(
        std::same_as<T, ::sus::fn::DynFn<sus::fn::Return<T, Args...>(Args...)>>)
  {
    ::sus::check_with_message(t_, "Box used after move");
    struct Cleanup {
      constexpr ~Cleanup() noexcept { delete t_; }
      T* t;
    };
    auto cleanup = Cleanup(::sus::mem::replace(t_, nullptr));
    return ::sus::fn::call(*cleanup.t, ::sus::forward<Args>(args)...);
  }

 private:
  enum FromPointer { FROM_POINTER };
  Box(FromPointer, T* t) : t_(t) {}

  T* t_;

  static constexpr T* never_value() {
    return static_cast<T*>(nullptr) + alignof(T);
  }

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(t_));
  sus_class_never_value_field(::sus::marker::unsafe_fn, Box, t_, never_value(),
                              nullptr);
  Box(::sus::mem::NeverValueConstructor) : t_(never_value()) {}
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
sus__format_to_stream(sus::boxed, Box, T);

// Promote `Box` into the `sus` namespace.
namespace sus {
using ::sus::boxed::Box;
}
