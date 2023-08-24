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

#include "sus/error/error.h"
#include "sus/macros/no_unique_address.h"
#include "sus/macros/pure.h"
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/mem/never_value.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/mem/size_of.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus::boxed {

// TODO: Box has an allocator parameter in Rust but std::unique_ptr does not.
// Which do we do?
template <class T>
class [[sus_trivial_abi]] Box final {
  static_assert(!std::is_reference_v<T>, "Box of a reference is not allowed.");
  static_assert(sus::mem::size_of<T>() != 0u);  // Ensure that `T` is defined.

 public:
  /// Constructs a Box which allocates space on the heap and moves `T` into it.s
  explicit constexpr Box(T t) noexcept
    requires(::sus::mem::Move<T>)
      : Box(FROM_OBJECT, ::sus::move(t)) {}

  constexpr ~Box() noexcept {
    if (t_) delete t_;
  }

  /// Returns a new box with a clone() of this box’s contents.
  ///
  /// Satisfies the [`Clone`]($sus::mem::Clone) concept if `T` satisfies
  /// [`Clone`]($sus::mem::Clone).
  constexpr Box clone() const noexcept
    requires(::sus::mem::Clone<T>)
  {
    ::sus::check_with_message(t_, *"Box used after move");
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
    ::sus::check_with_message(rhs.t_, *"Box used after move");
    ::sus::check_with_message(t_, *"Box used after move");
    ::sus::clone_into(*t_, *rhs.t_);
  }

  /// Satisifes the [`Move`]($sus::mem::Move) concept for
  /// [`Box`]($sus::boxed::Box).
  ///
  /// #[doc.overloads=move]
  constexpr Box(Box&& rhs) noexcept
      : Box(FROM_POINTER, ::sus::mem::replace(rhs.t_, nullptr)) {
    ::sus::check_with_message(t_, *"Box used after move");
  }
  /// Satisifes the [`Move`]($sus::mem::Move) concept for
  /// [`Box`]($sus::boxed::Box).
  ///
  /// #[doc.overloads=move]
  constexpr Box& operator=(Box&& rhs) noexcept {
    ::sus::check_with_message(rhs.t_, *"Box used after move");
    if (t_) delete t_;
    t_ = ::sus::mem::replace(rhs.t_, nullptr);
    return *this;
  }

  sus_pure constexpr const T& operator*() const {
    ::sus::check_with_message(t_, *"Box used after move");
    return *t_;
  }
  sus_pure constexpr T& operator*()
    requires(!std::is_const_v<T>)
  {
    ::sus::check_with_message(t_, *"Box used after move");
    return *t_;
  }

  sus_pure constexpr const T* operator->() const {
    ::sus::check_with_message(t_, *"Box used after move");
    return t_;
  }
  sus_pure constexpr T* operator->()
    requires(!std::is_const_v<T>)
  {
    ::sus::check_with_message(t_, *"Box used after move");
    return t_;
  }

  /// Converts `T` into a [`Box<T>`]($sus::boxed::Box).
  ///
  /// The conversion allocates on the heap and moves `t` into it.
  ///
  /// Satisfies the [`From<T>`]($sus::construct::From) concept for
  /// [`Box<T>`]($sus::boxed::Box).
  constexpr static Box from(T t) noexcept
    requires(::sus::mem::Move<T>)
  {
    return Box(FROM_OBJECT, ::sus::move(t));
  }

  /// Satisfies the [`From<E>`]($sus::construct::From) concept when `E`
  /// satisfies [`Error`]($sus::error::Error). This conversion moves and
  /// type-erases `E` into a heap-alloocated
  /// [`DynError`]($sus::error::DynError).
  ///
  /// #[doc.overloads=from.error]
  template <::sus::error::Error E>
    requires(sus::mem::Move<E>)
  constexpr static Box from(E e) noexcept
    requires(std::same_as<T, ::sus::error::DynError>)
  {
    using HeapError = ::sus::error::DynErrorTyped<E>;
    auto* heap_e = new HeapError(::sus::move(e));
    // This implicitly upcasts to the DynError.
    return Box(FROM_POINTER, heap_e);
  }

 private:
  enum FromObject { FROM_OBJECT };
  Box(FromObject, T&& t) : t_(new T(::sus::move(t))) {}
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
