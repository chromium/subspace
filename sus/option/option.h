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

#include <optional>  // TODO: Make this.. optional?
#include <type_traits>

#include "fmt/format.h"
#include "sus/assertions/check.h"
#include "sus/assertions/unreachable.h"
#include "sus/cmp/eq.h"
#include "sus/cmp/ord.h"
#include "sus/construct/default.h"
#include "sus/construct/into.h"
#include "sus/fn/fn_concepts.h"
#include "sus/iter/from_iterator.h"
#include "sus/iter/into_iterator.h"
#include "sus/iter/iterator_concept.h"
#include "sus/iter/product.h"
#include "sus/iter/sum.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/inline.h"
#include "sus/macros/lifetimebound.h"
#include "sus/macros/nonnull.h"
#include "sus/macros/pure.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/__private/ref_concepts.h"
#include "sus/mem/clone.h"
#include "sus/mem/copy.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/mem/take.h"
#include "sus/ops/try.h"
#include "sus/option/__private/is_option_type.h"
#include "sus/option/__private/is_tuple_type.h"
#include "sus/option/__private/marker.h"
#include "sus/option/__private/storage.h"
#include "sus/option/state.h"
#include "sus/result/__private/is_result_type.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"

// Have to forward declare iterator stuff here because Iterator includes Option,
// so Option can't include Iterator back.
namespace sus::iter {
template <class T>
  requires requires(const T& t) {
    { t.iter() };
  }
constexpr auto begin(const T& t) noexcept;
template <class T>
  requires requires(const T& t) {
    { t.iter() };
  }
constexpr auto end(const T& t) noexcept;
}  // namespace sus::iter

namespace sus {
// clang-format off
/// The [`Option`]($sus::option::Option) type, and the
/// [`some`]($sus::option::some) and [`none`]($sus::option::none)
/// type-deduction constructor functions.
///
/// The [`Option`]($sus::option::Option) type represents an optional value:
/// every [`Option`]($sus::option::Option) is either Some and contains a
/// value, or None, and does not. It is similar to
/// [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) but
/// with some differences:
/// * Extensive vocabulary for combining [`Option`]($sus::option::Option)s
///   together.
/// * Safe defined behaviour (a panic) when unwrapping an empty
///   [`Option`]($sus::option::Option), with an
///   explicit unsafe backdoor
///   ([`unwrap_unchecked`]($sus::option::Option::unwrap_unchecked))
///   for when it is needed.
/// * Avoid accidental expensive copies. Supports [`Copy`]($sus::mem::Copy) if
///   the inner type is
///   [`Copy`]($sus::mem::Copy) and [`Clone`]($sus::mem::Clone) if the inner
///   type is [`Clone`]($sus::mem::Clone).
/// * Provides [`take()`]($sus::option::Option::take) to move a value
///   out of an lvalue [`Option`]($sus::option::Option), which mark the
///   lvalue as empty instead of leaving a moved-from value behind as with
///   `std::move(optional).value()`.
/// * A custom message can be printed when trying to unwrap an empty
///   [`Option`]($sus::option::Option).
/// * Subspace [Iterator]($sus::iter) integration.
///   [`Option`]($sus::option::Option) can be iterated
///   over, acting like a single-element [collection]($sus::collections),
///   which allows it to be chained together with other iterators, filtered,
///   etc.
///
/// [`Option`]($sus::option::Option) types are very common, as they have a
/// number of uses:
///
/// * Initial values
/// * Return values for functions that are not defined over their entire input
///   range (partial functions)
/// * Return value for otherwise reporting simple errors, where `None` is
///   returned   on error.
/// * Optional struct fields Struct fields that can be loaned or "taken"
/// * Optional function arguments
/// * Returning an optional reference to a member
///
/// # Quick start
/// When the type is known to the compiler, you can construct an
/// [`Option`]($sus::option::Option) from a value without writing the full
/// type again, by using
/// [`sus::some(x)`]($sus::option::some) to make an
/// [`Option`]($sus::option::Option) holding `x` or
/// [`sus::none()`]($sus::option::none) to make an empty
/// [`Option`]($sus::option::Option). If returning an
/// [`Option`]($sus::option::Option) from a lambda, be sure to specify the
/// return type on the lambda to allow successful type deduction.
/// ```
/// // Returns Some("power!") if the input is over 9000, or None otherwise.
/// auto is_power = [](i32 i) -> sus::Option<std::string> {
///   if (i > 9000) return sus::some("power!");
///   return sus::none();
/// };
/// ```
///
/// Use [`is_some`]($sus::option::Option::is_some) and
/// [`is_none`]($sus::option::Option::is_none) to see if the
/// [`Option`]($sus::option::Option) is holding a value.
///
/// To immediately pull the inner value out of an
/// [`Option`]($sus::option::Option) an an rvalue, use
/// [`unwrap`]($sus::option::Option::unwrap). If the
/// [`Option`]($sus::option::Option) is an lvalue, use
/// [`as_value`]($sus::option::Option::as_value) and
/// [`as_value_mut`]($sus::option::Option::as_value_mut) to access the
/// inner value. Like
/// [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional),
/// [`operator*`]($sus::option::Option::operator*) and
/// [`operator->`]($sus::option::Option::operator->) are also
/// available if preferred. However if doing this many times, consider doing
/// [`unwrap`]($sus::option::Option::unwrap) a single time up front.
/// ```
/// sus_check(is_power(9001).unwrap() == "power!");
///
/// if (Option<std::string> lvalue = is_power(9001); lvalue.is_some())
///   sus_check(lvalue.as_value() == "power!");
///
/// sus_check(is_power(9000).unwrap_or("unlucky") == "unlucky");
/// ```
///
/// [`Option<const T>`]($sus::option::Option) for non-reference-type `T`
///  is disallowed, as the [`Option`]($sus::option::Option)
/// owns the `T` in that case and it ensures the
/// [`Option`]($sus::option::Option) and the `T` are both
/// accessed with the same constness.
///
/// # Representation
///
/// If a type `T` is a reference or satisties
/// [`NeverValueField`]($sus::mem::NeverValueField), then
/// [`Option<T>`]($sus::option::Option) will have the same size as T and
/// will be internally represented as just a `T` (or `T*` in the case of
/// a reference `T&`).
///
/// The following types `T`, when stored in an
/// [`Option<T>`]($sus::option::Option), will have the same
/// size as the original type `T`:
///
/// * `const T&` or `T&` (have the same size as `const T*` or `T*`)
/// * [`ptr::NonNull<U>`]($sus::ptr::NonNull)
/// * [`Box<T>`]($sus::boxed::Box)
/// * [`Choice`]($sus::choice_type::Choice)
///
/// This is called the "NeverValueField optimization", but is also called the
/// ["null pointer optimization" or NPO in Rust](
/// https://doc.rust-lang.org/stable/std/option/index.html#representation).
///
/// # Reference parameters
///
/// As mentioned above [`Option`]($sus::option::Option) type can hold a
/// reference, which allows code to use `Option<const T&>` or `Option<T&>`
/// as a function parameter in place of `const Option<T>&` or `Option<T>&`.
/// This can have a positive impact on compiler optimizations (and codegen
/// size) as the function is receiving the [`Option`]($sus::option::Option)
/// by value and thus the compiler can reason locally about the
/// [`Option`]($sus::option::Option)'s state. Otherwise
/// it needs to assume any function call can change the `const Option<T>&` to
/// become empty/non-empty. This is a common optimization pitfall with
/// [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional).
///
/// As an example, this code is [optimized poorly](
/// https://chromium-review.googlesource.com/c/chromium/src/+/4860473),
/// keeping a runtime check on
/// the [`Option`]($sus::option::Option). Global analysis could perhaps show it
/// not required, but it is beyond the view of the compiler.
/// ```
/// void Foo(const Option<T>& t) {
///   if (t.is_some()) {
///      A();  // Compiler assumes `t` may be changed.
///      B(t->thingy);  // Compiler has to keep the check if `t` is Some.
///   }
/// }
/// ```
/// Whereas here the compiler can elide runtime checks, and the parameter's size
/// is still the same as a pointer.
/// Use the [`as_ref`]($sus::option::Option::as_ref) method to convert to
/// `Option<const T&>` in the caller.
/// ```
/// void Foo(Option<const T&> t) {
///   if (t.is_some()) {
///      A();  // Compiler knows `t` is not changed.
///      B(t->thingy);  // Compiler drops the redundant check if `t` is Some.
///   }
/// }
/// ```
///
/// # Querying the variant
/// The [`is_some`]($sus::option::Option::is_some) and
/// [`is_none`]($sus::option::Option::is_none) methods return
/// `true` if the [`Option`]($sus::option::Option) is
/// holding a value or not, respectively.
///
/// # Adapters for working with lvalues
/// The following methods allow you to create an
/// [`Option`]($sus::option::Option) that refers to the value
/// held in an lvalue, without copying or moving from the lvalue:
/// * [`as_ref`]($sus::option::Option::as_ref) converts from a const
///   lvalue [`Option<T>`]($sus::option::Option) to an rvalue
///   [`Option<const T&>`]($sus::option::Option)`.
/// * [`as_mut`]($sus::option::Option::as_mut) converts from a mutable
///   lvalue [`Option<T>`]($sus::option::Option) to an rvalue
///   [`Option<T&>`]($sus::option::Option).
/// * [`take`]($sus::option::Option::take) moves the element out of
///   the lvalue [`Option<T>`]($sus::option::Option) into an rvalue
///   [`Option<T>`]($sus::option::Option), leaving the lvalue empty.
///
/// # Extracting the contained value
/// These methods extract the contained value in an [`Option<T>`]($sus::option::Option) when it is
/// holding a value.
///
/// For working with the option as an lvalue:
/// * [`as_value`]($sus::option::Option::as_value) returns const
///   reference access to the inner value. It will panic with a generic
///   message when empty.
/// * [`as_value_mut`]($sus::option::Option::as_value_mut) returns
///   mutable reference access to the inner value. It will panic with a
///   generic message when empty.
/// * [`operator*`]($sus::option::Option::operator*) returns mutable
///   reference access to the inner value. It will panic with a generic message
///   when empty.
/// * [`operator->`]($sus::option::Option::operator->) returns mutable
///   pointer access to the inner value. It will panic with a generic message
///   when empty.
///
/// For working with the option as an rvalue (when it returned from a function
/// call):
/// * [`expect`]($sus::option::Option::expect) moves and returns the
///   inner value. It will panic with a provided custom message when empty.
/// * [`unwrap`]($sus::option::Option::unwrap) moves and returns the
///   inner value. It will panic with a generic message with empty.
/// * [`unwrap_or`]($sus::option::Option::unwrap_or) moves and returns
///   the inner value. It will returns the provided default value instead when
///   empty.
/// * [`unwrap_or_default`]($sus::option::Option::unwrap_or_default)
///   moves and returns the inner value. It will return the default value of
///   the type `T` (which must satisfy [`Default`]($sus::construct::Default))
///   when empty.
/// * [`unwrap_or_else`]($sus::option::Option::unwrap_or_else) moves
///   and returns the inner value. It will return the result of evaluating
///   the provided function when empty.
///
/// # Copying
/// Most methods of Option act on an rvalue and consume the Option to transform
/// it into a new Option with a new value. This ensures that the value inside an
/// Option is moved while transforming it.
///
/// However, if [`Option`]($sus::option::Option) is
/// [`Copy`]($sus::mem::Copy), then the majority of methods offer an
/// overload to be called as an lvalue, in which case the
/// [`Option`]($sus::option::Option) will copy
/// itself, and its contained value, and perform the intended method on the copy
/// instead. This can have performance implications!
///
/// The unwrapping methods are excluded from this, and are only available on an
/// rvalue [`Option`]($sus::option::Option) to avoid copying just to access
/// the inner value. To do that, access the inner value as a reference through
/// [`as_value`]($sus::option::Option::as_value) and
/// [`as_value_mut`]($sus::option::Option::as_value_mut) or through
/// [`operator*`]($sus::option::Option::operator*) and
/// [`operator->`]($sus::option::Option::operator->).
///
/// # Transforming contained values
/// These methods transform [`Option`]($sus::option::Option) to
/// [`Result`]($sus::result::Result):
///
/// * [`ok_or`]($sus::option::Option::ok_or) transforms `Some(v)`
///   to `Ok(v)`, and `None` to `Err(err)` using the provided default err value.
/// * [`ok_or_else`]($sus::option::Option::ok_or_else) transforms
///   `Some(v)` to `Ok(v)`, and `None` to a value of `Err` using the
///   provided function.
/// * [`transpose`]($sus::option::Option::transpose) transposes an
///   [`Option`]($sus::option::Option) of a [`Result`]($sus::result::Result)
///   into a [`Result`]($sus::result::Result) of an
///   [`Option`]($sus::option::Option).
///
/// These methods transform an option holding a value:
///
/// * [`filter`]($sus::option::Option::filter) calls the provided
///   predicate function on the contained value `t` if the
///   [`Option`]($sus::option::Option) is `Some(t)`, and returns `Some(t)`
///   if the function returns `true`; otherwise, returns `None`.
/// * [`flatten`]($sus::option::Option::flatten) removes one level
///   of nesting from an `Option<Option<T>>`.
/// * [`map`]($sus::option::Option::map) transforms
///   [`Option<T>`]($sus::option::Option) to
///   [`Option<U>`]($sus::option::Option) by applying the provided
///   function to the contained value of `Some` and leaving `None` values
///   unchanged.
///
/// These methods transform [`Option<T>`]($sus::option::Option) to a value
/// of a possibly different type `U`:
///
/// * [`map_or`]($sus::option::Option::map_or) applies the provided
///   function to the contained value of `Some`, or returns the provided
///   default value if the
///   [`Option`]($sus::option::Option) is `None`.
/// * [`map_or_else`]($sus::option::Option::map_or_else) applies the
///   provided function to the contained value of `Some`, or returns the result
///   of evaluating the provided fallback function if the
///   [`Option`]($sus::option::Option) is `None`.
///
/// These methods combine the Some variants of two Option values:
///
/// * [`zip`]($sus::option::Option::zip) returns `Some(Tuple<S, O>(s, o)))` if the
///   [`Option`]($sus::option::Option) is `Some(s)`
///   and the method is called with an [`Option`]($sus::option::Option) value
///   of `Some(o)`; otherwise, returns `None`
/// * TODO: [`zip_with`](https://github.com/chromium/subspace/issues/341) calls
///   the provided
///   function `f` and returns `Some(f(s, o))` if the
///   [`Option`]($sus::option::Option) is `Some(s)`
///   and the method is called with an [`Option`]($sus::option::Option)
///   value of `Some(o)`; otherwise, returns `None`.
///
/// # Boolean operators
/// These methods treat the [`Option`]($sus::option::Option) as a boolean value,
/// where `Some` acts like `true` and `None` acts like `false`. There are two
/// categories of these methods: ones that take an
/// [`Option`]($sus::option::Option) as input, and ones
/// that take a function as input (to be lazily evaluated).
///
/// The [`and_that`]($sus::option::Option::and_that),
/// [`or_that`]($sus::option::Option::or_that),
/// and [`xor_that`]($sus::option::Option::xor_that) methods take
/// another [`Option`]($sus::option::Option) as input, and produce an
/// [`Option`]($sus::option::Option) as output.
/// Only the [`and_that`]($sus::option::Option::and_that)
/// method can produce an [`Option<U>`]($sus::option::Option) value having a 
/// different inner type `U` than [`Option<T>`]($sus::option::Option).
///
/// | method                                               | self    | input     | output  |
/// | ---------------------------------------------------- | ------- | --------- | ------- |
/// | [`and_that`]($sus::option::Option::and_that) | None    | (ignored) | None    |
/// | [`and_that`]($sus::option::Option::and_that) | Some(x) | None      | None    |
/// | [`and_that`]($sus::option::Option::and_that) | Some(x) | Some(y)   | Some(y) |
/// | [`or_that`]($sus::option::Option::or_that)   | None    | None      | None    |
/// | [`or_that`]($sus::option::Option::or_that)   | None    | Some(y)   | Some(y) |
/// | [`or_that`]($sus::option::Option::or_that)   | Some(x) | (ignored) | Some(x) |
/// | [`xor_that`]($sus::option::Option::xor_that) | None    | None      | None    |
/// | [`xor_that`]($sus::option::Option::xor_that) | None    | Some(y)   | Some(y) |
/// | [`xor_that`]($sus::option::Option::xor_that) | Some(x) | None      | Some(x) |
/// | [`xor_that`]($sus::option::Option::xor_that) | Some(x) | Some(y)   | None    |
///
/// The [`and_then`]($sus::option::Option::and_then) and
/// [`or_else`]($sus::option::Option::or_else) methods take a function
/// as input, and only evaluate the function when they need to produce a new
/// value. Only the [`and_then`]($sus::option::Option::and_then) method
/// can produce an [`Option<U>`]($sus::option::Option) value having a
/// different inner type `U` than [`Option<T>`]($sus::option::Option).
///
/// | method                                               | self    | function input | function result | output  |
/// | ---------------------------------------------------- | ------- | -------------- | --------------- | ------- |
/// | [`and_then`]($sus::option::Option::and_then) | None	   | (not provided) |	(not evaluated) | None    |
/// | [`and_then`]($sus::option::Option::and_then) | Some(x) | x              | None            | None    |
/// | [`and_then`]($sus::option::Option::and_then) | Some(x) | x              | Some(y)         | Some(y) |
/// | [`or_else`]($sus::option::Option::or_else)   | None    | (not provided) | None            | None    |
/// | [`or_else`]($sus::option::Option::or_else)   | None    | (not provided) | Some(y)         | Some(y) |
/// | [`or_else`]($sus::option::Option::or_else)   | Some(x) | (not provided) | (not evaluated) | Some(x) |
///
/// This is an example of using methods like
/// [`and_then`]($sus::option::Option::and_then) and
/// [`or_that`]($sus::option::Option::or_that) in a pipeline of
/// method calls. Early stages of the pipeline pass failure values (`None`)
/// through unchanged, and continue processing on success values (`Some`).
/// Toward the end, or substitutes an error message if it receives `None`.
///
/// ```
/// auto to_string = [](u8 u) -> sus::Option<std::string> {
///   switch (uint8_t{u}) {  // switch requires a primitive.
///     case 20u: return sus::some("foo");
///     case 42u: return sus::some("bar");
///     default: return sus::none();
///   }
/// };
/// auto res =
///     sus::Vec<u8>(0_u8, 1_u8, 11_u8, 200_u8, 22_u8)
///         .into_iter()
///         .map([&](auto x) {
///           // `checked_sub()` returns `None` on error.
///           return x.checked_sub(1_u8)
///               // same with `checked_mul()`.
///               .and_then([](u8 x) { return x.checked_mul(2_u8); })
///               // `to_string` returns `None` on error.
///               .and_then([&](u8 x) { return to_string(x); })
///               // Substitute an error message if we have `None` so far.
///               .or_that(sus::some(std::string("error!")))
///               // Won't panic because we unconditionally used `Some` above.
///               .unwrap();
///         })
///         .collect<Vec<std::string>>();
/// sus_check(res == sus::vec("error!", "error!", "foo", "error!", "bar"));
/// ```
///
/// # Restrictions on returning references
///
/// Methods that return references are only callable on an rvalue
/// [`Option`]($sus::option::Option) if the
/// [`Option`]($sus::option::Option) is holding a reference. If the
/// [`Option`]($sus::option::Option) is holding a non-reference
/// type, returning a reference from an rvalue
/// [`Option`]($sus::option::Option) would be giving a reference to a
/// short-lived object which is a bugprone pattern in C++ leading to
/// memory-safety bugs.
namespace option {}
// clang-format on
}  // namespace sus

namespace sus::option {
using namespace sus::option::__private;

template <class Item>
class OptionIter;

namespace __private {
template <class T, ::sus::iter::Iterator<Option<T>> Iter>
  requires ::sus::iter::Product<T>
constexpr Option<T> from_product_impl(Iter&& it) noexcept;
template <class T, ::sus::iter::Iterator<Option<T>> Iter>
  requires ::sus::iter::Sum<T>
constexpr Option<T> from_sum_impl(Iter&& it) noexcept;
}  // namespace __private

/// The [`Option`]($sus::option::Option) type.
///
/// See the [namespace level documentation]($sus::option) for more.
template <class T>
class Option final {
  // Note that `const T&` is allowed (so we don't `std::remove_reference_t<T>`)
  // as it would require dropping the const qualification to copy that into the
  // Option as a `T&`.
  static_assert(!std::is_const_v<T>,
                "`Option<const T>` should be written `const Option<T>`, as "
                "const applies transitively.");
  static_assert(
      !std::is_rvalue_reference_v<T>,
      "`Option<T&&> is not supported, use Option<T&> or Option<const T&.");

 public:
  /// Default-construct an option that is holding no value.
  ///
  /// This satisfies [`Default`]($sus::construct::Default) for
  /// [`Option`]($sus::option::Option).
  /// #[doc.overloads=ctor.none]
  inline constexpr Option() noexcept = default;

  /// Construct an option that is holding the given value.
  ///
  /// # Const References
  ///
  /// For [`Option<const T&>`]($sus::option::Option) it is possible to bind to
  /// a temporary which would create a memory safety bug. The
  /// `[[clang::lifetimebound]]` attribute is used to prevent this via Clang.
  /// But additionally, the incoming type is required to match with
  /// [`SafelyConstructibleFromReference`]($sus::construct::SafelyConstructibleFromReference)
  /// to prevent conversions that would construct a temporary.
  ///
  /// To force accepting a const reference anyway in cases where a type can
  /// convert to a reference without constructing a temporary, use an unsafe
  /// `static_cast<const T&>()` at the callsite and document why a temporary is
  /// not constructed.
  /// #[doc.overloads=ctor.some]
  explicit constexpr Option(const T& t) noexcept
    requires(!std::is_reference_v<T> &&  //
             ::sus::mem::Copy<T>)
      : Option(WITH_SOME, t) {}

  /// #[doc.overloads=ctor.some]
  template <std::convertible_to<T> U>
  explicit constexpr Option(U&& u) noexcept
    requires(!std::is_reference_v<T> &&  //
             ::sus::mem::Move<T> &&      //
             ::sus::mem::IsMoveRef<decltype(u)>)
      : Option(WITH_SOME, move_to_storage(u)) {}

  /// #[doc.overloads=ctor.some]
  template <std::convertible_to<T> U>
  explicit constexpr Option(U&& t sus_lifetimebound) noexcept
    requires(std::is_reference_v<T> &&  //
             sus::construct::SafelyConstructibleFromReference<T, U &&>)
      : Option(WITH_SOME, move_to_storage(t)) {}

  /// Converts from `Option<X>` to `Option<Y>` if `X` is convertible to `Y`.
  /// #[doc.overloads=ctor.convert]
  template <class U>
    requires(std::convertible_to<U, T> &&  //
             std::is_reference_v<U> == std::is_reference_v<T>)
  constexpr Option(const Option<U>& other) : t_(other.t_) {}
  /// #[doc.overloads=ctor.convert]
  template <class U>
    requires(std::convertible_to<U &&, T> &&  //
             std::is_reference_v<U> == std::is_reference_v<T>)
  constexpr Option(Option<U>&& other) : t_(::sus::move(other.t_)) {}
  // Can not convert an Option<U> to Option<T&>. Use as_ref() to convert from
  // Option<T> to Option<T&> or map() to perform a manual conversion to a
  // reference.
  /// #[doc.overloads=ctor.convert]
  template <class U>
    requires(!std::is_reference_v<U> && std::is_reference_v<T>)
  constexpr Option(Option<U>&& other) = delete;

  /// Moves or copies `val` into a new option holding `Some(val)`.
  ///
  /// Implements [`From<Option<T>, T>`]($sus::construct::From).
  ///
  /// #[doc.overloads=from.t]
  template <class U>
    requires(std::constructible_from<Option, U &&>)
  static constexpr Option from(U&& val) noexcept
    requires(!std::is_reference_v<T>)
  {
    return Option(sus::forward<U>(val));
  }
  /// #[doc.overloads=from.t]
  template <class U>
    requires(std::constructible_from<Option, U &&>)
  static constexpr Option from(U&& val sus_lifetimebound) noexcept
    requires(std::is_reference_v<T>)
  {
    return Option(::sus::forward<U>(val));
  }

  /// Computes the product of an iterator over
  /// [`Option<T>`]($sus::option::Option) as long as there is no `None` found.
  /// If a `None` is found, the function returns `None`.
  ///
  /// Prefer to call `product()` on the iterator rather than calling
  /// `from_product()` directly.
  ///
  /// Implements [`sus::iter::Product<Option<T>>`]($sus::iter::Product).
  ///
  /// The product is computed using the implementation of the inner type `T`
  /// which also satisfies [`sus::iter::Product<T>`]($sus::iter::Product).
  template <::sus::iter::Iterator<Option<T>> Iter>
    requires ::sus::iter::Product<T>
  static constexpr Option from_product(Iter&& it) noexcept {
    return __private::from_product_impl<T>(::sus::move(it));
  }

  /// Computes the sum of an iterator over [`Option<T>`]($sus::option::Option)
  /// as long as there is no `None` found. If a `None` is found, the function
  /// returns `None`.
  ///
  /// Prefer to call `sum()` on the iterator rather than calling `from_sum()`
  /// directly.
  ///
  /// Implements [`sus::iter::Sum<Option<T>>`]($sus::iter::Sum).
  ///
  /// The sum is computed using the implementation of the inner type `T`
  /// which also satisfies [`sus::iter::Sum<T>`]($sus::iter::Sum).
  template <::sus::iter::Iterator<Option<T>> Iter>
    requires ::sus::iter::Sum<T>
  static constexpr Option from_sum(Iter&& it) noexcept {
    return __private::from_sum_impl<T>(::sus::move(it));
  }

  /// Destructor for the option.
  ///
  /// Destroys the value contained within the option, if there is one.
  ///
  /// If `T` can be trivially destroyed, we don't need to explicitly destroy it,
  /// so we can use the default destructor, which allows
  /// [`Option<T>`]($sus::option::Option) to also be trivially destroyed.
  constexpr ~Option() noexcept
    requires(::sus::mem::__private::IsTrivialDtorOrRef<T>)
  = default;

  constexpr inline ~Option() noexcept
    requires(!::sus::mem::__private::IsTrivialDtorOrRef<T>)
  {
    if (t_.state() == Some) t_.destroy();
  }

  /// Copy constructor for [`Option<T>`]($sus::option::Option) which will
  /// satisfy [`Copy<Option<T>>`]($sus::mem::Copy) if
  /// [`Copy<T>`]($sus::mem::Copy) is satisfied.
  ///
  /// If `T` can be trivially copy-constructed, then `Option<T>` can also be
  /// trivially copy-constructed.
  ///
  /// #[doc.overloads=copy]
  constexpr Option(const Option& o)
    requires(::sus::mem::CopyOrRef<T>)
  = default;
  /// #[doc.overloads=copy]
  constexpr Option(const Option& o)
    requires(!::sus::mem::CopyOrRef<T>)
  = delete;

  /// Move constructor for [`Option<T>`]($sus::option::Option) which will
  /// satisfy [`Move<Option<T>>`]($sus::mem::Move) if
  /// [`Move<T>`]($sus::mem::Move) is satisfied.
  ///
  /// If `T` can be trivially move-constructed, then `Option<T>` can also be
  /// trivially move-constructed. When trivially-moved, the option is copied on
  /// move, and the moved-from Option is unchanged but should still not be used
  /// thereafter without reinitializing it. Use `take()` instead to move the
  /// value out of the option when the option may be used again afterward.
  ///
  /// #[doc.overloads=move]
  constexpr Option(Option&& o)
    requires(::sus::mem::MoveOrRef<T>)
  = default;
  /// #[doc.overloads=move]
  constexpr Option(Option&& o)
    requires(!::sus::mem::MoveOrRef<T>)
  = delete;

  /// Copy assignment for [`Option`]($sus::option::Option) which will
  /// satisfy [`Copy`]($sus::mem::Copy) for `Option<T>` if
  /// `T` satisifes [`Copy`]($sus::mem::Copy).
  ///
  /// If `T` can be trivially copy-assigned, then
  /// `Option<T>` can also be trivially copy-assigned.
  ///
  /// #[doc.overloads=copy]
  constexpr Option& operator=(const Option& o)
    requires(::sus::mem::CopyOrRef<T>)
  = default;
  /// #[doc.overloads=copy]
  constexpr Option& operator=(const Option& o)
    requires(!::sus::mem::CopyOrRef<T>)
  = delete;

  /// Move assignment for [`Option`]($sus::option::Option) which will
  /// satisfy [`Move`]($sus::mem::Move) for `<Option<T>` if `T`
  /// satisifies [`Move`]($sus::mem::Move).
  ///
  /// If `T` can be trivially move-assigned, then
  /// `Option<T>` can also be trivially move-assigned.
  /// When trivially-moved, the `Option` is copied on move, and the moved-from
  /// `Option` is unchanged but should still not be used thereafter without
  /// reinitializing it. Use [`take`]($sus::option::Option::take) to
  /// move the value out of the `Option` when the `Option` may be used again
  /// afterward, such as with class fields.
  ///
  /// #[doc.overloads=move]
  constexpr Option& operator=(Option&& o)
    requires(::sus::mem::MoveOrRef<T>)
  = default;

  /// #[doc.overloads=move]
  constexpr Option& operator=(Option&& o)
    requires(!::sus::mem::MoveOrRef<T>)
  = delete;

  /// Satisifies the [`Clone`]($sus::mem::Clone) concept when `Option` is not
  /// [`Copy`]($sus::mem::Copy).
  constexpr Option clone() const& noexcept
    requires(::sus::mem::Clone<T> && !::sus::mem::CopyOrRef<T>)
  {
    if (t_.state() == Some)
      return Option(::sus::clone(t_.val()));
    else
      return Option();
  }

  /// Satisifies the [`CloneFrom`]($sus::mem::CloneFrom) concept.
  constexpr void clone_from(const Option& source) & noexcept
    requires(::sus::mem::Clone<T> && !::sus::mem::CopyOrRef<T>)
  {
    if (&source == this) [[unlikely]]
      return;
    if (t_.state() == Some && source.t_.state() == Some) {
      ::sus::clone_into(t_.val_mut(), source.t_.val());
    } else {
      *this = source.clone();
    }
  }

  /// Returns whether the option currently contains a value.
  ///
  /// If there is a value present, it can be extracted with
  /// [`unwrap`]($sus::option::Option::unwrap) or
  /// [`expect`]($sus::option::Option::expect). For lvalues, it can be
  /// accessed as a reference through
  /// [`as_value`]($sus::option::Option::as_value) and
  /// [`as_value_mut`]($sus::option::Option::as_value_mut) for explicit
  /// const/mutable access, or through
  /// [`operator*`]($sus::option::Option::operator*)
  /// and [`operator->`]($sus::option::Option::operator->).
  _sus_pure constexpr bool is_some() const noexcept {
    return t_.state() == Some;
  }
  /// Returns whether the option is currently empty, containing no value.
  _sus_pure constexpr bool is_none() const noexcept {
    return t_.state() == None;
  }

  /// An operator which returns the state of the option, either `Some` or
  /// `None`.
  ///
  /// This supports the use of an option in a switch, allowing it to act as
  /// a tagged union between "some value" and "no value".
  ///
  /// # Example
  ///
  /// ```cpp
  /// auto x = Option<int>(2);
  /// switch (x) {
  ///  case Some:
  ///   return sus::move(x).unwrap_unchecked(unsafe_fn);
  ///  case None:
  ///   return -1;
  /// }
  /// ```
  _sus_pure constexpr operator State() const& { return t_.state(); }

  /// Returns the contained value inside the option.
  ///
  /// The function will panic with the given `message` if the option's state is
  /// currently `None`.
  constexpr _sus_nonnull_fn T expect(
      /* TODO: string view type */ _sus_nonnull_arg const char* _sus_nonnull_var
          message) && noexcept {
    sus_check_with_message(t_.state() == Some, message);
    return ::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn);
  }
  constexpr _sus_nonnull_fn T expect(
      /* TODO: string view type */ _sus_nonnull_arg const char* _sus_nonnull_var
          message) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).expect(message);
  }

  /// Returns the contained value inside the option.
  ///
  /// The function will panic without a message if the option's state is
  /// currently `None`.
  constexpr T unwrap() && noexcept {
    sus_check(t_.state() == Some);
    return ::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn);
  }
  constexpr T unwrap() const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).unwrap();
  }

  /// Returns the contained value inside the option, if there is one. Otherwise,
  /// returns `default_result`.
  ///
  /// Note that if it is non-trivial to construct a `default_result`, that
  /// [`unwrap_or_else`]($sus::option::Option::unwrap_or_else) should be
  /// used instead, as it will only construct the default value if required.
  constexpr T unwrap_or(T default_result) && noexcept {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      return default_result;
    }
  }
  constexpr T unwrap_or(T default_result) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).unwrap_or(::sus::move(default_result));
  }

  /// Returns the contained value inside the Option, if there is one.
  /// Otherwise, returns the result of the given function.
  constexpr T unwrap_or_else(::sus::fn::FnOnce<T()> auto f) && noexcept {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      return ::sus::fn::call_once(::sus::move(f));
    }
  }
  constexpr T unwrap_or_else(::sus::fn::FnOnce<T()> auto f) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).unwrap_or_else(::sus::move(f));
  }

  /// Returns the contained value inside the option, if there is one.
  /// Otherwise, constructs a default value for the type and returns that.
  ///
  /// The option's contained type `T` must be
  /// [`Default`]($sus::construct::Default) in order to be constructed with a
  /// default value.
  constexpr T unwrap_or_default() && noexcept
    requires(!std::is_reference_v<T> &&  //
             ::sus::construct::Default<T>)
  {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      return T();
    }
  }
  constexpr T unwrap_or_default() const& noexcept
    requires(!std::is_reference_v<T> &&       //
             ::sus::construct::Default<T> &&  //
             ::sus::mem::Copy<T>)
  {
    return ::sus::clone(*this).unwrap_or_default();
  }

  /// Returns the contained value inside the option.
  ///
  /// # Safety
  ///
  /// It is Undefined Behaviour to call this function when the option's state is
  /// `None`. The caller is responsible for ensuring the option contains a value
  /// beforehand, and the safer [`unwrap`]($sus::option::Option::unwrap)
  /// or [`expect`]($sus::option::Option::expect) should almost always
  /// be preferred. The compiler will typically elide the checks if they program
  /// verified the value appropriately before use in order to not panic.
  constexpr inline T unwrap_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      // Result::unwrap_unchecked benefits from telling the compiler explicitly
      // that the other states are never set. Match that here until shown it's
      // actually not useful.
      sus_unreachable_unchecked(::sus::marker::unsafe_fn);
    }
  }
  constexpr inline T unwrap_unchecked(
      ::sus::marker::UnsafeFnMarker) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).t_.take_and_set_none();
  }

  /// Returns a const reference to the contained value inside the option.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap`]($sus::option::Option::unwrap) on
  /// an rvalue, and [`take`]($sus::option::Option::take) to move the
  /// contents of an lvalue option to an rvalue.
  ///
  /// # Panics
  ///
  /// The function will panic without a message if the option's state is
  /// currently `None`.
  ///
  /// # Implementation Notes
  ///
  /// Implementation note: We only allow calling this on an rvalue Option if the
  /// contained value is a reference, otherwise we are returning a reference to
  /// a short-lived object which leads to common C++ memory bugs.
  _sus_pure constexpr const std::remove_reference_t<T>& as_value()
      const& noexcept {
    sus_check(t_.state() == Some);
    return t_.val();
  }
  _sus_pure constexpr const std::remove_reference_t<T>& as_value() && noexcept
    requires(std::is_reference_v<T>)
  {
    sus_check(t_.state() == Some);
    return t_.val();
  }
  /// Returns a mutable reference to the contained value inside the option.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap`]($sus::option::Option::unwrap) on
  /// an rvalue, and [`take`]($sus::option::Option::take) to move the
  /// contents of an lvalue option to an rvalue.
  ///
  /// # Panics
  ///
  /// The function will panic without a message if the option's state is
  /// currently `None`.
  ///
  /// # Implementation Notes
  ///
  /// Implementation note: We only allow calling this on an rvalue Option if the
  /// contained value is a reference, otherwise we are returning a reference to
  /// a short-lived object which leads to common C++ memory bugs.
  _sus_pure constexpr std::remove_reference_t<T>& as_value_mut() & noexcept {
    sus_check(t_.state() == Some);
    return t_.val_mut();
  }
  _sus_pure constexpr std::remove_reference_t<T>& as_value_mut() && noexcept
    requires(std::is_reference_v<T>)
  {
    sus_check(t_.state() == Some);
    return t_.val_mut();
  }
  _sus_pure constexpr std::remove_reference_t<T>& as_value_mut() const& noexcept
    requires(std::is_reference_v<T>)
  {
    return ::sus::clone(*this).as_value_mut();
  }

  /// Returns a const reference to the contained value inside the option.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap_unchecked`]($sus::option::Option::unwrap_unchecked) on
  /// an rvalue, and [`take`]($sus::option::Option::take) to move the
  /// contents of an lvalue option to an rvalue.
  ///
  /// # Safety
  /// The option's state must be `Some` or Undefined Behaviour results.
  ///
  /// # Implementation Notes
  ///
  /// Implementation note: We only allow calling this on an rvalue Option if the
  /// contained value is a reference, otherwise we are returning a reference to
  /// a short-lived object which leads to common C++ memory bugs.
  _sus_pure constexpr const std::remove_reference_t<T>& as_value_unchecked(
      ::sus::marker::UnsafeFnMarker) const& noexcept {
    return t_.val();
  }
  _sus_pure constexpr const std::remove_reference_t<T>& as_value_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept
    requires(std::is_reference_v<T>)
  {
    return t_.val();
  }
  /// Returns a mutable reference to the contained value inside the option.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap_unchecked`]($sus::option::Option::unwrap_unchecked) on
  /// an rvalue, and [`take`]($sus::option::Option::take) to move the
  /// contents of an lvalue option to an rvalue.
  ///
  /// # Safety
  /// The option's state must be `Some` or Undefined Behaviour results.
  ///
  /// # Implementation Notes
  ///
  /// Implementation note: We only allow calling this on an rvalue Option if the
  /// contained value is a reference, otherwise we are returning a reference to
  /// a short-lived object which leads to common C++ memory bugs.
  _sus_pure constexpr std::remove_reference_t<T>& as_value_unchecked_mut(
      ::sus::marker::UnsafeFnMarker) & noexcept {
    return t_.val_mut();
  }
  _sus_pure constexpr std::remove_reference_t<T>& as_value_unchecked_mut(
      ::sus::marker::UnsafeFnMarker) && noexcept
    requires(std::is_reference_v<T>)
  {
    return t_.val_mut();
  }
  _sus_pure constexpr std::remove_reference_t<T>& as_value_unchecked_mut(
      ::sus::marker::UnsafeFnMarker) const& noexcept
    requires(std::is_reference_v<T>)
  {
    return ::sus::clone(*this).as_value_unchecked_mut(::sus::marker::unsafe_fn);
  }

  /// Returns a reference to the contained value inside the option.
  ///
  /// The reference is const if the option is const, and is mutable otherwise.
  /// This method allows calling methods directly on the type inside the option
  /// without unwrapping.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap`]($sus::option::Option::unwrap) on
  /// an rvalue, and [`take`]($sus::option::Option::take) to move the
  /// contents of an lvalue option to an rvalue.
  ///
  /// # Panics
  ///
  /// The function will panic without a message if the option's state is
  /// currently `None`.
  ///
  /// # Implementation Notes
  ///
  /// Implementation note: We only allow calling this on an rvalue option if the
  /// contained value is a reference, otherwise we are returning a reference to
  /// a short-lived object which leads to common C++ memory bugs.
  ///
  /// Implementation note: This method is added in addition to the Rust option
  /// API because:
  /// * C++ moving is verbose, making
  ///   [`unwrap`]($sus::option::Option::unwrap) on lvalues loud.
  /// * Unwrapping requires a new lvalue name since C++ doesn't allow name
  ///   reuse, making variable names bad.
  /// * We also provide [`as_value`]($sus::option::Option::as_value) and
  ///   [`as_value_mut`]($sus::option::Option::as_value_mut) for
  ///   explicit const/mutable lvalue access but...
  /// * It's expected in C++ ecosystems, due to
  ///   [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional)
  ///   and other pre-existing collection-of-one things to provide access
  ///   through [`operator*`]($sus::option::Option::operator*) and
  ///   [`operator->`]($sus::option::Option::operator->).
  _sus_pure constexpr const std::remove_reference_t<T>& operator*()
      const& noexcept {
    sus_check(t_.state() == Some);
    return t_.val();
  }
  _sus_pure constexpr const std::remove_reference_t<T>& operator*() && noexcept
    requires(std::is_reference_v<T>)
  {
    sus_check(t_.state() == Some);
    return t_.val();
  }
  _sus_pure constexpr std::remove_reference_t<T>& operator*() & noexcept {
    sus_check(t_.state() == Some);
    return t_.val_mut();
  }

  /// Returns a pointer to the contained value inside the option.
  ///
  /// The pointer is const if the option is const, and is mutable otherwise.
  /// This method allows calling methods directly on the type inside the option
  /// without unwrapping.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap`]($sus::option::Option::unwrap) on
  /// an rvalue, and [`take`]($sus::option::Option::take) to move the
  /// contents of an lvalue option to an rvalue.
  ///
  /// # Panics
  ///
  /// The function will panic without a message if the option's state is
  /// currently `None`.
  ///
  /// # Implementation Notes
  ///
  /// Implementation note: We only allow calling this on an rvalue option if the
  /// contained value is a reference, otherwise we are returning a reference to
  /// a short-lived object which leads to common C++ memory bugs.
  ///
  /// Implementation note: This method is added in addition to the Rust Option
  /// API because:
  /// * C++ moving is verbose, making
  ///   [`unwrap`]($sus::option::Option::unwrap) on lvalues loud.
  /// * Unwrapping requires a new lvalue name since C++ doesn't allow name
  ///   reuse, making variable names bad.
  /// * We also provide [`as_value`]($sus::option::Option::as_value) and
  /// [`as_value_mut`]($sus::option::Option::as_value_mut) for explicit
  ///   const/mutable lvalue access but...
  /// * It's expected in C++ ecosystems, due to
  ///   [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional)
  ///   and other pre-existing collection-of-one things to provide access
  ///   through [`operator*`]($sus::option::Option::operator*) and
  ///   [`operator->`]($sus::option::Option::operator->).
  _sus_pure constexpr const std::remove_reference_t<T>* operator->()
      const& noexcept {
    sus_check(t_.state() == Some);
    return ::sus::mem::addressof(
        static_cast<const std::remove_reference_t<T>&>(t_.val()));
  }
  _sus_pure constexpr const std::remove_reference_t<T>* operator->() && noexcept
    requires(std::is_reference_v<T>)
  {
    sus_check(t_.state() == Some);
    return ::sus::mem::addressof(
        static_cast<const std::remove_reference_t<T>&>(t_.val()));
  }
  _sus_pure constexpr std::remove_reference_t<T>* operator->() & noexcept {
    sus_check(t_.state() == Some);
    return ::sus::mem::addressof(
        static_cast<std::remove_reference_t<T>&>(t_.val_mut()));
  }

  /// Inserts `value` into the option, then returns a mutable reference to it.
  ///
  /// If the option already contains a value, the old value is dropped.
  ///
  /// See also
  /// [`Option::get_or_insert`]($sus::option::Option::get_or_insert),
  /// which doesn’t update the value if the option already contains `Some`.
  constexpr T& insert(T value) & noexcept
    requires(!std::is_reference_v<T> &&  //
             sus::mem::Move<T>)
  {
    t_.set_some(move_to_storage(value));
    return t_.val_mut();
  }
  template <std::convertible_to<T> U>
  constexpr T& insert(U&& value) & noexcept
    requires(std::is_reference_v<T> &&  //
             sus::construct::SafelyConstructibleFromReference<T, U &&>)
  {
    t_.set_some(move_to_storage(value));
    return t_.val_mut();
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// stores `value` inside the Option and returns a mutable reference to it.
  ///
  /// If it is non-trivial to construct `T`, the
  /// [`Option::get_or_insert_with`]($sus::option::Option::get_or_insert_with)
  /// method would be preferable, as it only constructs a `T` if needed.
  constexpr T& get_or_insert(T value) & noexcept sus_lifetimebound
    requires(!std::is_reference_v<T> &&  //
             sus::mem::Move<T>)
  {
    if (t_.state() == None) {
      t_.construct_from_none(move_to_storage(value));
    }
    return t_.val_mut();
  }
  template <std::convertible_to<T> U>
  constexpr T& get_or_insert(U&& value) & noexcept sus_lifetimebound
    requires(std::is_reference_v<T> &&  //
             sus::construct::SafelyConstructibleFromReference<T, U &&>)
  {
    if (t_.state() == None) {
      t_.construct_from_none(move_to_storage(value));
    }
    return t_.val_mut();
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// constructs a default value `T`, stores it inside the option and returns a
  /// mutable reference to the new value.
  ///
  /// This method differs from
  /// [`unwrap_or_default`]($sus::option::Option::unwrap_or_default)
  /// in that it does not consume the option, and instead it can not be called
  /// on rvalues.
  ///
  /// This is a shorthand for
  /// `Option<T>::get_or_insert_with([] { return T(); })`.
  ///
  /// The option's contained type `T` must satisfy
  /// [`Default`]($sus::construct::Default) so it can be constructed with its
  /// default value.
  constexpr T& get_or_insert_default() & noexcept sus_lifetimebound
    requires(::sus::construct::Default<T>)
  {
    if (t_.state() == None) t_.construct_from_none(T());
    return t_.val_mut();
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// constructs a `T` by calling `f`, stores it inside the option and returns a
  /// mutable reference to the new value.
  ///
  /// This method differs from
  /// [`unwrap_or_else`]($sus::option::Option::unwrap_or_else) in that
  /// it does not consume the option, and instead it can not be called on
  /// rvalues.
  constexpr T& get_or_insert_with(::sus::fn::FnOnce<T()> auto f) & noexcept {
    if (t_.state() == None) {
      t_.construct_from_none(
          move_to_storage(::sus::fn::call_once(::sus::move(f))));
    }
    return t_.val_mut();
  }

  /// Returns a new option containing whatever was inside the current option.
  ///
  /// If this option contains `None` then it is left unchanged and returns an
  /// option containing `None`. If this option contains `Some` with a value, the
  /// value is moved into the returned option and this option will contain
  /// `None` afterward.
  constexpr Option take() & noexcept {
    if (t_.state() == Some)
      return Option(WITH_SOME, t_.take_and_set_none());
    else
      return Option();
  }

  /// Maps the option's value through a function.
  ///
  /// When called on an rvalue, it consumes the option, passing the value
  /// through the map function, and returning an
  /// [`Option<R>`]($sus::option::Option) where `R` is the
  /// return type of the map function.
  ///
  /// Returns an [`Option<R>`]($sus::option::Option) in state `None`
  /// if the current option is in state `None`.
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn>
  constexpr Option<sus::fn::ReturnOnce<MapFn, T&&>> map(MapFn m) && noexcept {
    if (t_.state() == Some) {
      return Option<sus::fn::ReturnOnce<MapFn, T&&>>(::sus::fn::call_once(
          ::sus::move(m), ::sus::forward<T>(t_.take_and_set_none())));
    } else {
      return Option<sus::fn::ReturnOnce<MapFn, T&&>>();
    }
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn>
  constexpr Option<sus::fn::ReturnOnce<MapFn, T&&>> map(MapFn m) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).map(::sus::move(m));
  }

  /// Returns the provided default result (if none), or applies a function to
  /// the contained value (if any).
  ///
  /// Arguments passed to `map_or` are eagerly evaluated; if you are passing the
  /// result of a function call, it is recommended to use
  /// [`map_or_else`]($sus::option::Option::map_or_else), which is
  /// lazily evaluated.
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn>
  constexpr sus::fn::ReturnOnce<MapFn, T&&> map_or(
      sus::fn::ReturnOnce<MapFn, T&&> default_result, MapFn m) && noexcept {
    if (t_.state() == Some) {
      return ::sus::fn::call_once(::sus::move(m),
                                  ::sus::forward<T>(t_.take_and_set_none()));
    } else {
      return default_result;
    }
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn>
  constexpr sus::fn::ReturnOnce<MapFn, T&&> map_or(
      sus::fn::ReturnOnce<MapFn, T&&> default_result, MapFn m) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).map_or(
        ::sus::forward<decltype(default_result)>(default_result),
        ::sus::move(m));
  }

  /// Computes a default function result (if none), or applies a different
  /// function to the contained value (if any).
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> DefaultFn,
            ::sus::fn::FnOnce<::sus::fn::ReturnOnce<DefaultFn>(T&&)> MapFn>
  constexpr sus::fn::ReturnOnce<DefaultFn> map_or_else(DefaultFn default_fn,
                                                       MapFn m) && noexcept {
    if (t_.state() == Some) {
      return ::sus::fn::call_once(::sus::move(m),
                                  ::sus::forward<T>(t_.take_and_set_none()));
    } else {
      return ::sus::fn::call_once(::sus::move(default_fn));
    }
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> DefaultFn,
            ::sus::fn::FnOnce<::sus::fn::ReturnOnce<DefaultFn>(T&&)> MapFn>
  constexpr sus::fn::ReturnOnce<DefaultFn> map_or_else(DefaultFn default_fn,
                                                       MapFn m) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).map_or_else(::sus::move(default_fn),
                                           ::sus::move(m));
  }

  /// Consumes the option and applies a predicate function to the value
  /// contained in the option. Returns a new option with the same value if the
  /// predicate returns true, otherwise returns an Option with its state set to
  /// `None`.
  ///
  /// The predicate function must be able to receive `const T&` and return a
  /// value that converts to`bool`.
  constexpr Option<T> filter(
      ::sus::fn::FnOnce<bool(const std::remove_reference_t<T>&)> auto
          p) && noexcept {
    if (t_.state() == Some) {
      if (::sus::fn::call_once(
              ::sus::move(p),
              static_cast<const std::remove_reference_t<T>&>(t_.val()))) {
        return Option(WITH_SOME, t_.take_and_set_none());
      } else {
        // The state has to become None, and we must destroy the inner T.
        t_.set_none();
        return Option();
      }
    } else {
      return Option();
    }
  }
  constexpr Option<T> filter(
      ::sus::fn::FnOnce<bool(const std::remove_reference_t<T>&)> auto p)
      const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).filter(::sus::move(p));
  }

  /// Consumes this option and returns an option with `None` if this option
  /// holds `None`, otherwise returns `that` option.
  template <class U>
  constexpr Option<U> and_that(Option<U> that) && noexcept {
    if (t_.state() == Some) {
      t_.set_none();
    } else {
      that = Option<U>();
    }
    return that;
  }
  template <class U>
  constexpr Option<U> and_that(Option<U> that) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).and_that(::sus::move(that));
  }

  /// Consumes this option and returns an option with `None` if this option
  /// holds `None`, otherwise calls `f` with the contained value and returns the
  /// result.
  ///
  /// The function `f` receives the option's inner `T` and can return any
  /// [`Option<U>`]($sus::option::Option).
  ///
  /// Some languages call this operation flatmap.
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> AndFn>
    requires(__private::IsOptionType<sus::fn::ReturnOnce<AndFn, T &&>>::value)
  constexpr sus::fn::ReturnOnce<AndFn, T&&> and_then(AndFn f) && noexcept {
    if (t_.state() == Some) {
      return ::sus::fn::call_once(::sus::move(f),
                                  ::sus::forward<T>(t_.take_and_set_none()));
    } else {
      return sus::fn::ReturnOnce<AndFn, T&&>();
    }
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> AndFn>
    requires(__private::IsOptionType<sus::fn::ReturnOnce<AndFn, T &&>>::value)
  constexpr sus::fn::ReturnOnce<AndFn, T&&> and_then(AndFn f) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).and_then(::sus::move(f));
  }

  /// Consumes and returns an option with the same value if this option contains
  /// a value, otherwise returns `that` option.
  constexpr Option<T> or_that(Option<T> that) && noexcept {
    if (t_.state() == Some)
      return Option(WITH_SOME, t_.take_and_set_none());
    else
      return that;
  }
  constexpr Option<T> or_that(Option<T> that) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).or_that(::sus::move(that));
  }

  /// Consumes and returns an option with the same value if this option contains
  /// a value, otherwise returns the option returned by `f`.
  constexpr Option<T> or_else(
      ::sus::fn::FnOnce<Option<T>()> auto f) && noexcept {
    if (t_.state() == Some)
      return Option(WITH_SOME, t_.take_and_set_none());
    else
      return ::sus::fn::call_once(::sus::move(f));
  }
  constexpr Option<T> or_else(
      ::sus::fn::FnOnce<Option<T>()> auto f) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).or_else(::sus::move(f));
  }

  /// Consumes this option and returns an option holding the value from either
  /// this option or `that` option if exactly one of them holds a value,
  /// otherwise returns an Option that holds `None`.
  constexpr Option<T> xor_that(Option<T> that) && noexcept {
    if (t_.state() == Some) {
      // If `this` holds Some, we change `this` to hold None. If `that` is None,
      // we return what this was holding, otherwise we return None.
      if (that.t_.state() == None) {
        return Option(WITH_SOME, t_.take_and_set_none());
      } else {
        t_.set_none();
        return Option();
      }
    } else {
      // If `this` holds None, we need to do nothing to `this`. If `that` is
      // Some we would return its value, and if `that` is None we should return
      // None.
      return that;
    }
  }
  constexpr Option<T> xor_that(Option<T> that) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).xor_that(::sus::move(that));
  }

  /// Transforms the [`Option<T>`]($sus::option::Option) into a
  /// [`Result<T, E>`]($sus::result::Result), mapping `Some(v)` to `Ok(v)` and
  /// `None` to `Err(e)`.
  ///
  /// Arguments passed to `ok_or` are eagerly evaluated; if you are passing the
  /// result of a function call, it is recommended to use
  /// [`ok_or_else`]($sus::option::Option::ok_or_else), which is
  /// lazily evaluated.
  //
  // TODO: No refs in Result: https://github.com/chromium/subspace/issues/133
  template <class E, int&..., class Result = ::sus::result::Result<T, E>>
  constexpr Result ok_or(E e) && noexcept
    requires(!std::is_reference_v<T> && !std::is_reference_v<E>)
  {
    if (t_.state() == Some)
      return Result(t_.take_and_set_none());
    else
      return Result::with_err(::sus::move(e));
  }
  template <class E, int&..., class Result = ::sus::result::Result<T, E>>
  constexpr Result ok_or(E e) const& noexcept
    requires(!std::is_reference_v<T> && !std::is_reference_v<E> &&
             ::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).ok_or(::sus::move(e));
  }

  /// Transforms the [`Option<T>`]($sus::option::Option) into a
  /// [`Result<T, E>`]($sus::result::Result), mapping `Some(v)` to `Ok(v)` and
  /// `None` to `Err(f())`.
  //
  // TODO: No refs in Result: https://github.com/chromium/subspace/issues/133
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> ElseFn>
  constexpr sus::result::Result<T, sus::fn::ReturnOnce<ElseFn>> ok_or_else(
      ElseFn f) && noexcept
    requires(!std::is_reference_v<T> &&  //
             !std::is_reference_v<sus::fn::ReturnOnce<ElseFn>>)
  {
    using R = sus::result::Result<T, sus::fn::ReturnOnce<ElseFn>>;
    if (t_.state() == Some)
      return R(t_.take_and_set_none());
    else
      return R::with_err(::sus::fn::call_once(::sus::move(f)));
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> ElseFn>
  constexpr sus::result::Result<T, sus::fn::ReturnOnce<ElseFn>> ok_or_else(
      ElseFn f) const& noexcept
    requires(!std::is_reference_v<T> &&                            //
             !std::is_reference_v<sus::fn::ReturnOnce<ElseFn>> &&  //
             ::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).ok_or_else(::sus::move(f));
  }

  /// Transposes an [`Option`]($sus::option::Option) of a
  /// [`Result`]($sus::result::Result) into a
  /// [`Result`]($sus::result::Result) of an
  /// [`Option`]($sus::option::Option).
  ///
  /// `None` will be mapped to `Ok(None)`. `Some(Ok(_))` and `Some(Err(_))` will
  /// be mapped to `Ok(Some(_))` and `Err(_)`.
  template <int&...,
            class OkType =
                typename ::sus::result::__private::IsResultType<T>::ok_type,
            class ErrType =
                typename ::sus::result::__private::IsResultType<T>::err_type,
            class Result = ::sus::result::Result<Option<OkType>, ErrType>>
    requires(::sus::result::__private::IsResultType<T>::value)
  constexpr Result transpose() && noexcept {
    if (t_.state() == None) {
      return Result(Option<OkType>());
    } else {
      if (t_.val().is_ok()) {
        return Result(Option<OkType>(
            Option<OkType>::WITH_SOME,
            t_.take_and_set_none().unwrap_unchecked(::sus::marker::unsafe_fn)));
      } else {
        return Result::with_err(t_.take_and_set_none().unwrap_err_unchecked(
            ::sus::marker::unsafe_fn));
      }
    }
  }
  template <int&...,
            class OkType =
                typename ::sus::result::__private::IsResultType<T>::ok_type,
            class ErrType =
                typename ::sus::result::__private::IsResultType<T>::err_type,
            class Result = ::sus::result::Result<Option<OkType>, ErrType>>
    requires(::sus::result::__private::IsResultType<T>::value)
  constexpr Result transpose() const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).transpose();
  }

  /// Zips self with another option.
  ///
  /// If self is `Some(s)` and other is `Some(o)`, this method returns
  /// `Some(Tuple(s, o))`. Otherwise, `None` is returned.
  template <class U, int&..., class Tuple = ::sus::tuple_type::Tuple<T, U>>
  constexpr Option<Tuple> zip(Option<U> o) && noexcept {
    if (o.t_.state() == None) {
      if (t_.state() == Some) t_.set_none();
      return Option<Tuple>();
    } else if (t_.state() == None) {
      return Option<Tuple>();
    } else {
      // SAFETY: We have verified `*this` and `o` contain Some above.
      return Option<Tuple>(
          Option<Tuple>::WITH_SOME,
          Tuple(::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn),
                ::sus::move(o).unwrap_unchecked(::sus::marker::unsafe_fn)));
    }
  }
  template <class U, int&..., class Tuple = ::sus::tuple_type::Tuple<T, U>>
  constexpr Option<Tuple> zip(Option<U> o) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).zip(::sus::move(o));
  }

  /// Unzips an option holding a [`Tuple`]($sus::tuple_type::Tuple) of two
  /// values into a [`Tuple`]($sus::tuple_type::Tuple) of two
  /// [`Option`]($sus::option::Option)s.
  ///
  /// [`Option<Tuple<i32, u32>>`]($sus::option::Option) is unzipped to
  /// [`Tuple<Option<i32>, Option<u32>>`]($sus::tuple_type::Tuple).
  ///
  /// If self is `Some`, the result is a tuple with both options holding the
  /// values from self. Otherwise, the result is a tuple of two options set to
  /// None.
  constexpr auto unzip() && noexcept
    requires(!std::is_reference_v<T> && __private::IsTupleOfSizeTwo<T>::value)
  {
    using U = __private::IsTupleOfSizeTwo<T>::first_type;
    using V = __private::IsTupleOfSizeTwo<T>::second_type;
    if (is_some()) {
      auto&& [u, v] = t_.take_and_set_none();
      return ::sus::tuple_type::Tuple<Option<U>, Option<V>>(
          Option<U>(::sus::forward<U>(u)), Option<V>(::sus::forward<V>(v)));
    } else {
      return ::sus::tuple_type::Tuple<Option<U>, Option<V>>(Option<U>(),
                                                            Option<V>());
    }
  }
  constexpr auto unzip() const& noexcept
    requires(!std::is_reference_v<T> && __private::IsTupleOfSizeTwo<T>::value &&
             ::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).unzip();
  }

  /// Replaces whatever the Option is currently holding with `value` and returns
  /// an Option holding what was there previously, which may be empty.
  constexpr Option replace(T value) & noexcept
    requires(!std::is_reference_v<T> &&  //
             sus::mem::Move<T>)
  {
    if (t_.state() == None) {
      t_.construct_from_none(move_to_storage(value));
      return Option();
    } else {
      return Option(WITH_SOME, t_.replace_some(move_to_storage(value)));
    }
  }
  template <std::convertible_to<T> U>
  constexpr Option replace(U&& value) & noexcept
    requires(std::is_reference_v<T> &&  //
             sus::construct::SafelyConstructibleFromReference<T, U &&>)
  {
    if (t_.state() == None) {
      t_.construct_from_none(move_to_storage(value));
      return Option();
    } else {
      return Option(WITH_SOME, t_.replace_some(move_to_storage(value)));
    }
  }

  /// Maps an [`Option<T&>`]($sus::option::Option) to an
  /// [`Option<T>`]($sus::option::Option) by copying the referenced `T`.
  constexpr Option<std::remove_const_t<std::remove_reference_t<T>>> copied()
      const& noexcept
    requires(std::is_reference_v<T> && ::sus::mem::Copy<T>)
  {
    if (t_.state() == None) {
      return Option<std::remove_const_t<std::remove_reference_t<T>>>();
    } else {
      return Option<std::remove_const_t<std::remove_reference_t<T>>>(
          Option<std::remove_const_t<std::remove_reference_t<T>>>::WITH_SOME,
          t_.val());
    }
  }

  /// Maps an [`Option<T&>`]($sus::option::Option) to an
  /// [`Option<T>`]($sus::option::Option) by cloning the referenced `T`.
  constexpr Option<std::remove_const_t<std::remove_reference_t<T>>> cloned()
      const& noexcept
    requires(std::is_reference_v<T> && ::sus::mem::Clone<T>)
  {
    if (t_.state() == None) {
      return Option<std::remove_const_t<std::remove_reference_t<T>>>();
    } else {
      // Specify the type `T` for clone() as `t_.val()` may be a
      // `StoragePointer<T>` when the Option is holding a reference, and we want
      // to clone the `T` object, not the `StoragePointer<T>`. The latter
      // converts to a `const T&`.
      return Option<std::remove_const_t<std::remove_reference_t<T>>>(
          Option<std::remove_const_t<std::remove_reference_t<T>>>::WITH_SOME,
          ::sus::clone(
              static_cast<const std::remove_reference_t<T>&>(t_.val())));
    }
  }

  /// Maps an [`Option<Option<T>>`]($sus::option::Option) to an
  /// [`Option<T>`]($sus::option::Option).
  constexpr T flatten() && noexcept
    requires(::sus::option::__private::IsOptionType<T>::value)
  {
    if (t_.state() == Some)
      return ::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn);
    else
      return T();
  }
  constexpr T flatten() const& noexcept
    requires(::sus::option::__private::IsOptionType<T>::value &&
             ::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).flatten();
  }

  /// Returns an [`Option<const T&>`]($sus::option::Option) from this
  /// [`Option<T>`]($sus::option::Option), that either holds `None` or a
  /// reference to the value in this option.
  ///
  /// # Implementation Notes
  ///
  /// Implementation note: We only allow calling this on an rvalue option if the
  /// contained value is a reference, otherwise we are returning a reference to
  /// a short-lived object which leads to common C++ memory bugs.
  _sus_pure constexpr Option<const std::remove_reference_t<T>&> as_ref()
      const& noexcept {
    if (t_.state() == None)
      return Option<const std::remove_reference_t<T>&>();
    else
      return Option<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>::WITH_SOME, t_.val());
  }
  _sus_pure constexpr Option<const std::remove_reference_t<T>&>
  as_ref() && noexcept
    requires(std::is_reference_v<T>)
  {
    if (t_.state() == None)
      return Option<const std::remove_reference_t<T>&>();
    else
      return Option<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>::WITH_SOME,
          t_.take_and_set_none());
  }

  /// Returns an [`Option<T&>`]($sus::option::Option) from this
  /// [`Option<T>`]($sus::option::Option), that either holds `None` or a
  /// reference to the value in this option.
  _sus_pure constexpr Option<T&> as_mut() & noexcept {
    if (t_.state() == None)
      return Option<T&>();
    else
      return Option<T&>(Option<T&>::WITH_SOME, t_.val_mut());
  }
  // Calling as_mut() on an rvalue is not returning a reference to the inner
  // value if the inner value is already a reference, so we allow calling it on
  // an rvalue Option in that case.
  _sus_pure constexpr Option<T&> as_mut() && noexcept
    requires(std::is_reference_v<T>)
  {
    if (t_.state() == None)
      return Option<T&>();
    else
      return Option<T&>(Option<T&>::WITH_SOME, t_.take_and_set_none());
  }
  _sus_pure constexpr Option<T&> as_mut() const& noexcept
    requires(std::is_reference_v<T>)
  {
    return ::sus::clone(*this).as_mut();
  }

  /// Produces an [`Iterator`]($sus::iter::Iterator) over the single item in the
  /// `Option`, or an empty iterator. The iterator will return a const
  /// reference.
  _sus_pure constexpr OptionIter<const std::remove_reference_t<T>&> iter()
      const& noexcept;
  constexpr OptionIter<const std::remove_reference_t<T>&> iter() && noexcept
    requires(std::is_reference_v<T>);
  constexpr OptionIter<const std::remove_reference_t<T>&> iter() const& noexcept
    requires(std::is_reference_v<T>);

  /// Produces an [`Iterator`]($sus::iter::Iterator) over the single item in the
  /// `Option`, or an empty iterator. If the `Option` holds a value, the
  /// iterator will return a mutable reference to it. If the `Option` holds a
  /// reference, it will return that reference.
  _sus_pure constexpr OptionIter<T&> iter_mut() & noexcept;
  constexpr OptionIter<T&> iter_mut() && noexcept
    requires(std::is_reference_v<T>);
  constexpr OptionIter<T&> iter_mut() const& noexcept
    requires(std::is_reference_v<T>);

  /// Produces an [`Iterator`]($sus::iter::Iterator) over the single item in the
  /// `Option`, or an empty iterator. If the Option holds a value, the iterator
  /// will return ownership of the value. If the `Option` holds a reference, it
  /// will return that reference.
  constexpr OptionIter<T> into_iter() && noexcept;
  constexpr OptionIter<T> into_iter() const& noexcept
    requires(::sus::mem::CopyOrRef<T>);

  /// Satisfies the [`Eq<Option<U>>`]($sus::cmp::Eq) concept.
  ///
  /// The non-template overload allows some/none marker types to convert to
  /// Option for comparison.
  friend constexpr inline bool operator==(const Option& l,
                                          const Option& r) noexcept
    requires(::sus::cmp::Eq<T>)
  {
    switch (l) {
      case Some:
        return r.is_some() && (l.as_value_unchecked(::sus::marker::unsafe_fn) ==
                               r.as_value_unchecked(::sus::marker::unsafe_fn));
      case None: return r.is_none();
    }
    sus_unreachable_unchecked(::sus::marker::unsafe_fn);
  }
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
  friend constexpr inline bool operator==(const Option<T>& l,
                                          const Option<U>& r) noexcept {
    switch (l) {
      case Some:
        return r.is_some() && (l.as_value_unchecked(::sus::marker::unsafe_fn) ==
                               r.as_value_unchecked(::sus::marker::unsafe_fn));
      case None: return r.is_none();
    }
    sus_unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  template <class U>
    requires(!::sus::cmp::Eq<T, U>)
  friend constexpr inline bool operator==(const Option<T>& l,
                                          const Option<U>& r) = delete;

  /// Compares two options. This function requires that `T` is ordered.
  /// An empty Option always compares less than a non-empty Option.
  ///
  /// * Satisfies [`StrongOrd<Option<T>>`]($sus::cmp::StrongOrd) if
  ///   [`StrongOrd<T>`]($sus::cmp::StrongOrd).
  /// * Satisfies [`Ord<Option<T>>`]($sus::cmp::Ord) if
  ///   [`Ord<T>`]($sus::cmp::Ord).
  /// * Satisfies [`PartialOrd<Option<T>>`]($sus::cmp::PartialOrd) if
  ///   [`PartialOrd<T>`]($sus::cmp::PartialOrd).
  ///
  /// The non-template overloads allow some/none marker types to convert to
  /// an option for comparison.
  //
  // sus::cmp::StrongOrd<Option<U>> trait.
  friend constexpr inline std::strong_ordering operator<=>(
      const Option& l, const Option& r) noexcept
    requires(::sus::cmp::ExclusiveStrongOrd<T>)
  {
    switch (l) {
      case Some:
        if (r.is_some()) {
          return l.as_value_unchecked(::sus::marker::unsafe_fn) <=>
                 r.as_value_unchecked(::sus::marker::unsafe_fn);
        } else {
          return std::strong_ordering::greater;
        }
      case None:
        if (r.is_some())
          return std::strong_ordering::less;
        else
          return std::strong_ordering::equivalent;
    }
    sus_unreachable_unchecked(::sus::marker::unsafe_fn);
  }
  template <class U>
    requires(::sus::cmp::ExclusiveStrongOrd<T, U>)
  friend constexpr inline std::strong_ordering operator<=>(
      const Option<T>& l, const Option<U>& r) noexcept {
    switch (l) {
      case Some:
        if (r.is_some()) {
          return l.as_value_unchecked(::sus::marker::unsafe_fn) <=>
                 r.as_value_unchecked(::sus::marker::unsafe_fn);
        } else {
          return std::strong_ordering::greater;
        }
      case None:
        if (r.is_some())
          return std::strong_ordering::less;
        else
          return std::strong_ordering::equivalent;
    }
    sus_unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  // sus::cmp::Ord<Option<U>> trait.
  friend constexpr inline std::weak_ordering operator<=>(
      const Option& l, const Option& r) noexcept
    requires(::sus::cmp::ExclusiveOrd<T>)
  {
    switch (l) {
      case Some:
        if (r.is_some()) {
          return l.as_value_unchecked(::sus::marker::unsafe_fn) <=>
                 r.as_value_unchecked(::sus::marker::unsafe_fn);
        } else {
          return std::weak_ordering::greater;
        }
      case None:
        if (r.is_some())
          return std::weak_ordering::less;
        else
          return std::weak_ordering::equivalent;
    }
    sus_unreachable_unchecked(::sus::marker::unsafe_fn);
  }
  template <class U>
    requires(::sus::cmp::ExclusiveOrd<T, U>)
  friend constexpr inline std::weak_ordering operator<=>(
      const Option<T>& l, const Option<U>& r) noexcept {
    switch (l) {
      case Some:
        if (r.is_some()) {
          return l.as_value_unchecked(::sus::marker::unsafe_fn) <=>
                 r.as_value_unchecked(::sus::marker::unsafe_fn);
        } else {
          return std::weak_ordering::greater;
        }
      case None:
        if (r.is_some())
          return std::weak_ordering::less;
        else
          return std::weak_ordering::equivalent;
    }
    sus_unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  // sus::cmp::PartialOrd<Option<U>> trait.
  template <class U>
    requires(::sus::cmp::ExclusivePartialOrd<T, U>)
  friend constexpr inline std::partial_ordering operator<=>(
      const Option<T>& l, const Option<U>& r) noexcept {
    switch (l) {
      case Some:
        if (r.is_some()) {
          return l.as_value_unchecked(::sus::marker::unsafe_fn) <=>
                 r.as_value_unchecked(::sus::marker::unsafe_fn);
        } else {
          return std::partial_ordering::greater;
        }
      case None:
        if (r.is_some())
          return std::partial_ordering::less;
        else
          return std::partial_ordering::equivalent;
    }
    sus_unreachable_unchecked(::sus::marker::unsafe_fn);
  }
  friend constexpr inline std::partial_ordering operator<=>(
      const Option& l, const Option& r) noexcept
    requires(::sus::cmp::ExclusivePartialOrd<T>)
  {
    switch (l) {
      case Some:
        if (r.is_some()) {
          return l.as_value_unchecked(::sus::marker::unsafe_fn) <=>
                 r.as_value_unchecked(::sus::marker::unsafe_fn);
        } else {
          return std::partial_ordering::greater;
        }
      case None:
        if (r.is_some())
          return std::partial_ordering::less;
        else
          return std::partial_ordering::equivalent;
    }
    sus_unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  template <class U>
    requires(!::sus::cmp::PartialOrd<T, U>)
  friend constexpr inline auto operator<=>(
      const Option<T>& l, const Option<U>& r) noexcept = delete;

  /// Implicit conversion from [`std::optional`](
  /// https://en.cppreference.com/w/cpp/utility/optional).
  ///
  /// May convert from `U` in `optional<U>` to `T` in `Option<T>`.
  ///
  /// #[doc.overloads=ctor.optional]
  template <class U>
    requires(std::convertible_to<U &&, T>)
  constexpr Option(std::optional<U>&& s) noexcept
    requires(!std::is_reference_v<T>)
      : Option() {
    if (s.has_value()) t_.set_some(move_to_storage(::sus::move(s).value()));
  }
  /// #[doc.overloads=ctor.optional]
  template <class U>
    requires(std::convertible_to<const U&, T>)
  constexpr Option(const std::optional<U>& s) noexcept
    requires(!std::is_reference_v<T>)
      : Option() {
    if (s.has_value()) t_.set_some(copy_to_storage(s.value()));
  }
  /// Implicit conversion to [`std::optional`](
  /// https://en.cppreference.com/w/cpp/utility/optional).
  ///
  /// May convert from `T` in `Option<T>` to `U` in `optional<U>`.
  ///
  /// #[doc.overloads=convert.optional]
  template <class U>
    requires(std::convertible_to<const std::remove_reference_t<T>&, U>)
  constexpr operator std::optional<U>() const& noexcept {
    if (is_some()) {
      return std::optional<U>(std::in_place,
                              as_value_unchecked(::sus::marker::unsafe_fn));
    } else {
      return std::nullopt;
    }
  }
  /// #[doc.overloads=convert.optional]
  template <class U>
    requires(std::convertible_to<T, U>)
  constexpr operator std::optional<U>() && noexcept {
    if (is_some()) {
      return std::optional<U>(
          std::in_place,
          ::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn));
    } else {
      return std::nullopt;
    }
  }
  /// #[doc.overloads=convert.optional]
  constexpr operator std::optional<const std::remove_reference_t<T>*>()
      const& noexcept
    requires(std::is_reference_v<T>)
  {
    if (is_some()) {
      return std::optional<const std::remove_reference_t<T>*>(
          ::sus::mem::addressof(as_value()));
    } else {
      return std::nullopt;
    }
  }
  // Skip implementing `& noexcept` `&& noexcept` and `const& noexcept` and just
  // implement `const& noexcept` which covers them all.
  /// #[doc.overloads=convert.optional]
  constexpr operator std::optional<std::remove_reference_t<T>*>()
      const& noexcept
    requires(!std::is_const_v<std::remove_reference_t<T>> &&
             std::is_reference_v<T>)
  {
    if (is_some()) {
      return std::optional<std::remove_reference_t<T>*>(
          ::sus::mem::addressof(as_value_mut()));
    } else {
      return std::nullopt;
    }
  }

 private:
  template <class U>
  friend class Option;

  // Since `T` may be a reference or a value type, this constructs the correct
  // storage from a `T` object or a `T&&` (which is received as `T&`).
  template <class U>
  static constexpr inline decltype(auto) copy_to_storage(const U& t) {
    if constexpr (std::is_reference_v<T>)
      return StoragePointer<T>(t);
    else
      return t;
  }
  // Since `T` may be a reference or a value type, this constructs the correct
  // storage from a `T` object or a `T&&` (which is received as `T&`).
  template <class U>
  static constexpr inline decltype(auto) move_to_storage(U&& t) {
    if constexpr (std::is_reference_v<T>) {
      return StoragePointer<T>(::sus::forward<U>(t));
    } else {
      return ::sus::move(t);
    }
  }

  /// Constructors for `Some`.
  enum WithSome { WITH_SOME };
  constexpr explicit Option(WithSome, T t)
    requires(std::is_reference_v<T>)
      : t_(StoragePointer<T>(t)) {}
  constexpr explicit Option(WithSome, const T& t)
    requires(!std::is_reference_v<T>)
      : t_(t) {}
  constexpr explicit Option(WithSome, T&& t)
    requires(!std::is_reference_v<T> && ::sus::mem::Move<T>)
      : t_(::sus::move(t)) {}

  template <class U>
  using StorageType =
      std::conditional_t<std::is_reference_v<U>, Storage<StoragePointer<U>>,
                         Storage<U>>;
  // TODO: We can make this no_unique_address, however... if StorageType<T> has
  // tail padding and Option was marked with no_unique_address, then
  // constructing T may clobber stuff OUTSIDE the Option. So this can only be
  // no_unique_address when StorageType<T> has no tail padding.
  StorageType<T> t_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           StorageType<T>);
};

template <class T>
Option(T) -> Option<std::remove_cvref_t<T>>;

/// Implicit for-ranged loop iteration via [`Option::iter`](
/// $sus::option::Option::iter).
using ::sus::iter::begin;
/// Implicit for-ranged loop iteration via [`Option::iter`](
/// $sus::option::Option::iter).
using ::sus::iter::end;

/// Used to construct an option with a Some(t) value.
///
/// Calling some() produces a hint to make an option but does not actually
/// construct option. This is to allow constructing an
/// [`Option<T>`]($sus::option::Option) or
/// [`Option<T&>`]($sus::option::Option) correctly.
template <class T>
[[nodiscard]] inline constexpr __private::SomeMarker<T&&> some(
    T&& t sus_lifetimebound) noexcept {
  return __private::SomeMarker<T&&>(::sus::forward<T>(t));
}

/// Used to construct an option with a None value.
///
/// Calling none() produces a hint to make an option but does not actually
/// construct the option. This is because the type `T` is not known until the
/// construction is explicitly requested.
__sus_pure_const inline constexpr auto none() noexcept {
  return __private::NoneMarker();
}

}  // namespace sus::option

// Implements [Try](sus-ops-Try.html) for [Option]($sus::option::Option).
template <class T>
struct sus::ops::TryImpl<::sus::option::Option<T>> {
  using Output = T;
  template <class U>
  using RemapOutput = ::sus::option::Option<U>;
  constexpr static bool is_success(const ::sus::option::Option<T>& t) noexcept {
    return t.is_some();
  }
  constexpr static Output into_output(::sus::option::Option<T> t) noexcept {
    // SAFETY: The Option is verified to be holding Some(T) by
    // `sus::ops::try_into_output()` before it calls here.
    return ::sus::move(t).unwrap_unchecked(::sus::marker::unsafe_fn);
  }
  constexpr static ::sus::option::Option<T> from_output(Output t) noexcept {
    return ::sus::option::Option<T>(::sus::move(t));
  }
  template <class U>
  constexpr static ::sus::option::Option<T> preserve_error(
      ::sus::option::Option<U>) noexcept {
    // The incoming Option is known to be empty (the error state) and this is
    // checked by try_preserve_error() before coming here. So we can just return
    // another empty Option.
    return ::sus::option::Option<T>();
  }

  // Implements [`TryDefault`](sus-ops-TryDefault.html) for
  // [`Option<T>`]($sus::option::Option) if `T` satisfies
  // [`Default`]($sus::construct::Default).
  constexpr static ::sus::option::Option<T> from_default() noexcept
    requires(sus::construct::Default<T>)
  {
    return ::sus::option::Option<T>(T());
  }
};

// std hash support.
template <class T>
struct std::hash<::sus::option::Option<T>> {
  auto operator()(const ::sus::option::Option<T>& u) const noexcept {
    if (u.is_some())
      return std::hash<T>()(*u);
    else
      return 0;
  }
};
template <class T>
  requires(::sus::cmp::Eq<T>)
struct std::equal_to<::sus::option::Option<T>> {
  constexpr auto operator()(const ::sus::option::Option<T>& l,
                            const ::sus::option::Option<T>& r) const noexcept {
    return l == r;
  }
};

// fmt support.
template <class T, class Char>
struct fmt::formatter<::sus::option::Option<T>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::option::Option<T>& t,
                        FormatContext& ctx) const {
    if (t.is_none()) {
      return fmt::format_to(ctx.out(), "None");
    } else {
      auto out = ctx.out();
      out = fmt::format_to(out, "Some(");
      ctx.advance_to(out);
      out = underlying_.format(t.as_value(), ctx);
      return fmt::format_to(out, ")");
    }
  }

 private:
  ::sus::string::__private::AnyFormatter<T, Char> underlying_;
};

// Stream support.
_sus_format_to_stream(sus::option, Option, T);

// Promote Option and its enum values into the `sus` namespace.
namespace sus {
using ::sus::option::none;
using ::sus::option::None;
using ::sus::option::Option;
using ::sus::option::some;
using ::sus::option::Some;
}  // namespace sus

//////

// This header contains a type that needs to use Option, and is used by an
// Option method implementation below. So we have to include it after defining
// Option but before implementing the following methods.
#include "sus/iter/size_hint.h"

namespace sus::option {

namespace __private {

// This is a separate function instead of an out-of-line definition to work
// around bug https://github.com/llvm/llvm-project/issues/63769 in Clang 16.
template <class T, ::sus::iter::Iterator<Option<T>> Iter>
  requires ::sus::iter::Product<T>
constexpr Option<T> from_product_impl(Iter&& it) noexcept {
  class IterUntilNone final
      : public ::sus::iter::IteratorBase<IterUntilNone, T> {
   public:
    constexpr IterUntilNone(Iter& iter, bool& found_none)
        : iter_(iter), found_none_(found_none) {}

    constexpr Option<T> next() noexcept {
      Option<Option<T>> next = iter_.next();
      Option<T> out;
      if (next.is_some()) {
        out = ::sus::move(next).unwrap_unchecked(::sus::marker::unsafe_fn);
        if (out.is_none()) found_none_ = true;
      }
      return out;
    }
    constexpr ::sus::iter::SizeHint size_hint() const noexcept {
      return ::sus::iter::SizeHint(0u, iter_.size_hint().upper);
    }

   private:
    Iter& iter_;
    bool& found_none_;
  };
  static_assert(::sus::iter::Iterator<IterUntilNone, T>);

  bool found_none = false;
  auto out = Option<T>(IterUntilNone(it, found_none).product());
  if (found_none) out = Option<T>();
  return out;
}

// This is a separate function instead of an out-of-line definition to work
// around bug https://github.com/llvm/llvm-project/issues/63769 in Clang 16.
template <class T, ::sus::iter::Iterator<Option<T>> Iter>
  requires ::sus::iter::Sum<T>
constexpr Option<T> from_sum_impl(Iter&& it) noexcept {
  class IterUntilNone final
      : public ::sus::iter::IteratorBase<IterUntilNone, T> {
   public:
    constexpr IterUntilNone(Iter& iter, bool& found_none)
        : iter_(iter), found_none_(found_none) {}

    constexpr Option<T> next() noexcept {
      Option<Option<T>> next = iter_.next();
      Option<T> out;
      if (next.is_some()) {
        out = ::sus::move(next).unwrap_unchecked(::sus::marker::unsafe_fn);
        if (out.is_none()) found_none_ = true;
      }
      return out;
    }
    constexpr ::sus::iter::SizeHint size_hint() const noexcept {
      return ::sus::iter::SizeHint(0u, iter_.size_hint().upper);
    }

   private:
    Iter& iter_;
    bool& found_none_;
  };
  static_assert(::sus::iter::Iterator<IterUntilNone, T>);

  bool found_none = false;
  auto out = Option<T>(IterUntilNone(it, found_none).sum());
  if (found_none) out = Option<T>();
  return out;
}

}  // namespace __private

}  // namespace sus::option

//////

// This header contains a type that needs to use Option, and is used by the
// Option method implementations below. So we have to include it after defining
// Option but before implementing the following methods.
#include "sus/option/option_iter.h"

namespace sus::option {

template <class T>
_sus_pure constexpr OptionIter<const std::remove_reference_t<T>&>
Option<T>::iter() const& noexcept {
  return OptionIter<const std::remove_reference_t<T>&>(as_ref());
}
template <class T>
constexpr OptionIter<const std::remove_reference_t<T>&>
Option<T>::iter() && noexcept
  requires(std::is_reference_v<T>)
{
  return OptionIter<const std::remove_reference_t<T>&>(
      ::sus::move(*this).as_ref());
}
template <class T>
constexpr OptionIter<const std::remove_reference_t<T>&> Option<T>::iter()
    const& noexcept
  requires(std::is_reference_v<T>)
{
  return ::sus::clone(*this).iter();
}

template <class T>
_sus_pure constexpr OptionIter<T&> Option<T>::iter_mut() & noexcept {
  return OptionIter<T&>(as_mut());
}
template <class T>
constexpr OptionIter<T&> Option<T>::iter_mut() && noexcept
  requires(std::is_reference_v<T>)
{
  return OptionIter<T&>(::sus::move(*this).as_mut());
}
template <class T>
constexpr OptionIter<T&> Option<T>::iter_mut() const& noexcept
  requires(std::is_reference_v<T>)
{
  return ::sus::clone(*this).iter_mut();
}

template <class T>
constexpr OptionIter<T> Option<T>::into_iter() && noexcept {
  return OptionIter<T>(take());
}
template <class T>
constexpr OptionIter<T> Option<T>::into_iter() const& noexcept
  requires(::sus::mem::CopyOrRef<T>)
{
  return ::sus::clone(*this).into_iter();
}

}  // namespace sus::option

// sus::iter::FromIterator trait for Option.
template <class T>
struct sus::iter::FromIteratorImpl<::sus::option::Option<T>> {
  // TODO: Subdoc doesn't split apart template instantiations so comments
  // collide. This should be able to appear in the docs.
  //
  // Takes each item in the Iterator: if it is None, no further elements are
  // taken, and the None is returned. Should no None occur, a collection of type
  // T containing the values of type U from each Option<U> is returned.
  template <class IntoIter, int&...,
            class Iter =
                std::decay_t<decltype(std::declval<IntoIter&&>().into_iter())>,
            class O = typename Iter::Item,
            class U = ::sus::option::__private::IsOptionType<O>::inner_type>
    requires(::sus::option::__private::IsOptionType<O>::value &&
             ::sus::iter::IntoIterator<IntoIter, ::sus::option::Option<U>>)
  static constexpr ::sus::option::Option<T> from_iter(
      IntoIter&& option_iter) noexcept
    requires(!std::is_reference_v<T> && ::sus::iter::FromIterator<T, U>)
  {
    // An iterator over `option_iter`'s iterator that returns each element in it
    // until it reaches a `None` or the end.
    struct UntilNoneIter final
        : public ::sus::iter::IteratorBase<UntilNoneIter, U> {
      constexpr UntilNoneIter(Iter&& iter, bool& found_none)
          : iter(iter), found_none(found_none) {}

      constexpr ::sus::option::Option<U> next() noexcept {
        ::sus::option::Option<::sus::option::Option<U>> item = iter.next();
        if (found_none || item.is_none()) return ::sus::option::Option<U>();
        found_none = item->is_none();
        return ::sus::move(item).flatten();
      }
      constexpr ::sus::iter::SizeHint size_hint() const noexcept {
        return ::sus::iter::SizeHint(0u, iter.size_hint().upper);
      }

      Iter& iter;
      bool& found_none;
    };

    bool found_none = false;
    auto iter = UntilNoneIter(::sus::move(option_iter).into_iter(), found_none);
    auto collected = Option<T>(sus::iter::from_iter<T>(::sus::move(iter)));
    if (found_none) collected = ::sus::option::Option<T>();
    return collected;
  }
};
