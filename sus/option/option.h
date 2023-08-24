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
#include "sus/ops/eq.h"
#include "sus/ops/ord.h"
#include "sus/ops/try.h"
#include "sus/option/__private/is_option_type.h"
#include "sus/option/__private/is_tuple_type.h"
#include "sus/option/__private/marker.h"
#include "sus/option/__private/storage.h"
#include "sus/option/state.h"
#include "sus/result/__private/is_result_type.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"

// Have to forward declare a bunch of iterator stuff here because Iterator
// includes Option, so it can't include iterator back.
namespace sus::iter {
template <class Item>
class Once;
template <class Item>
constexpr Once<Item> once(::sus::option::Option<Item> o) noexcept;
namespace __private {
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
}  // namespace __private
}  // namespace sus::iter

namespace sus {
// clang-format off
/// The [`Option`](sus-option-Option.html) type, and the
/// [`some`](sus-option-fn.some.html) and [`none`](sus-option-fn.none.html)
/// type-deduction constructor functions.
///
/// The [`Option`](sus-option-Option.html) type represents an optional value:
/// every [`Option`](sus-option-Option.html) is either Some and contains a
/// value, or None, and does not. It is similar to
/// [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) but
/// with some differences:
/// * Extensive vocabulary for combining [`Option`](sus-option-Option.html)s
///   together.
/// * Safe defined behaviour (a panic) when unwrapping an empty
///   [`Option`](sus-option-Option.html), with an
///   explicit unsafe backdoor
///   ([`unwrap_unchecked`](sus-option-Option.html#method.unwrap_unchecked))
///   for when it is needed.
/// * Avoid accidental expensive copies. Supports [`Copy`](sus-mem-Copy.html) if
///   the inner type is
///   [`Copy`](sus-mem-Copy.html) and [`Clone`](sus-mem-Clone.html) if the inner
///   type is [`Clone`](sus-mem-Clone.html).
/// * Provides [`take()`](sus-option-Option.html#method.take) to move a value
///   out of an lvalue [`Option`](sus-option-Option.html), which mark the
///   lvalue as empty instead of leaving a moved-from value behind as with
///   `std::move(optional).value()`.
/// * A custom message can be printed when trying to unwrap an empty
///   [`Option`](sus-option-Option.html).
/// * Subspace [Iterator](sus-iter.html) integration.
///   [`Option`](sus-option-Option.html) can be iterated
///   over, acting like a single-element [collection](sus-collections.html),
///   which allows it to be chained together with other iterators, filtered,
///   etc.
///
/// [`Option`](sus-option-Option.html) types are very common, as they have a
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
/// [`Option`](sus-option-Option.html) from a value without writing the full
/// type again, by using
/// [`sus::some(x)`](sus-option-fn.some.html) to make an
/// [`Option`](sus-option-Option.html) holding `x` or
/// [`sus::none()`](sus-option-fn.none.html) to make an empty
/// [`Option`](sus-option-Option.html). If returning an
/// [`Option`](sus-option-Option.html) from a lambda, be sure to specify the
/// return type on the lambda to allow successful type deduction.
/// ```
/// // Returns Some("power!") if the input is over 9000, or None otherwise.
/// auto is_power = [](i32 i) -> sus::Option<std::string> {
///   if (i > 9000) return sus::some("power!");
///   return sus::none();
/// };
/// ```
///
/// Use [`is_some`](sus-option-Option.html#method.is_some) and
/// [`is_none`](sus-option-Option.html#method.is_none) to see if the
/// [`Option`](sus-option-Option.html) is holding a value.
///
/// To immediately pull the inner value out of an
/// [`Option`](sus-option-Option.html) an an rvalue, use
/// [`unwrap`](sus-option-Option.html#method.unwrap). If the
/// [`Option`](sus-option-Option.html) is an lvalue, use
/// [`as_value`](sus-option-Option.html#method.as_value) and
/// [`as_value_mut`](sus-option-Option.html#method.as_value_mut) to access the
/// inner value. Like
/// [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional),
/// [`operator*`](sus-option-Option.html#method.operator*) and
/// [`operator->`](sus-option-Option.html#method.operator->) are also
/// available if preferred. However if doing this many times, consider doing
/// [`unwrap`](sus-option-Option.html#method.unwrap) a single time up front.
/// ```
/// sus::check(is_power(9001).unwrap() == "power!");
///
/// if (Option<std::string> lvalue = is_power(9001); lvalue.is_some())
///   sus::check(lvalue.as_value() == "power!");
///
/// sus::check(is_power(9000).unwrap_or("unlucky") == "unlucky");
/// ```
///
/// [`Option<const T>`](sus-option-Option.html) for non-reference-type `T`
///  is disallowed, as the [`Option`](sus-option-Option.html)
/// owns the `T` in that case and it ensures the
/// [`Option`](sus-option-Option.html) and the `T` are both
/// accessed with the same constness.
///
/// # Representation
///
/// If a type `T` is a reference or satisties
/// [`NeverValueField`](sus-mem-NeverValueField.html), then
/// [`Option<T>`](sus-option-Option.html) will have the same size as T and
/// will be internally represented as just a `T` (or `T*` in the case of
/// a reference `T&`).
///
/// The following types `T`, when stored in an
/// [`Option<T>`](sus-option-Option.html), will have the same
/// size as the original type `T`:
///
/// * `const T&` or `T&` (have the same size as `const T*` or `T*`)
/// * [`ptr::NonNull<U>`](sus-ptr-NonNull.html)
///
/// This is called the "NeverValueField optimization", but is also called the
/// ["null pointer optimization" or NPO in Rust](
/// https://doc.rust-lang.org/stable/std/option/index.html#representation).
///
/// # Querying the variant
/// The [`is_some`](sus-option-Option.html#method.is_some) and
/// [`is_none`](sus-option-Option.html#method.is_none) methods return
/// `true` if the [`Option`](sus-option-Option.html) is
/// holding a value or not, respectively.
///
/// # Adapters for working with lvalues
/// The following methods allow you to create an
/// [`Option`](sus-option-Option.html) that refers to the value
/// held in an lvalue, without copying or moving from the lvalue:
/// * [`as_ref`](sus-option-Option.html#method.as_ref) converts from a const
///   lvalue [`Option<T>`](sus-option-Option.html) to an rvalue
///   [`Option<const T&>`](sus-option-Option.html)`.
/// * [`as_mut`](sus-option-Option.html#method.as_mut) converts from a mutable
///   lvalue [`Option<T>`](sus-option-Option.html) to an rvalue
///   [`Option<T&>`](sus-option-Option.html).
/// * [`take`](sus-option-Option.html#method.take) moves the element out of
///   the lvalue [`Option<T>`](sus-option-Option.html) into an rvalue
///   [`Option<T>`](sus-option-Option.html), leaving the lvalue empty.
///
/// # Extracting the contained value
/// These methods extract the contained value in an [`Option<T>`](sus-option-Option.html) when it is
/// holding a value.
///
/// For working with the option as an lvalue:
/// * [`as_value`](sus-option-Option.html#method.as_value) returns const
///   reference access to the inner value. It will panic with a generic
///   message when empty.
/// * [`as_value_mut`](sus-option-Option.html#method.as_value_mut) returns
///   mutable reference access to the inner value. It will panic with a
///   generic message when empty.
/// * [`operator*`](sus-option-Option.html#method.operator*) returns mutable
///   reference access to the inner value. It will panic with a generic message
///   when empty.
/// * [`operator->`](sus-option-Option.html#method.operator->) returns mutable
///   pointer access to the inner value. It will panic with a generic message
///   when empty.
///
/// For working with the option as an rvalue (when it returned from a function
/// call):
/// * [`expect`](sus-option-Option.html#method.expect) moves and returns the
///   inner value. It will panic with a provided custom message when empty.
/// * [`unwrap`](sus-option-Option.html#method.unwrap) moves and returns the
///   inner value. It will panic with a generic message with empty.
/// * [`unwrap_or`](sus-option-Option.html#method.unwrap_or) moves and returns
///   the inner value. It will returns the provided default value instead when
///   empty.
/// * [`unwrap_or_default`](sus-option-Option.html#method.unwrap_or_default)
///   moves and returns the inner value. It will return the default value of
///   the type `T` (which must satisfy [`Default`](sus-construct-Default.html))
///   when empty.
/// * [`unwrap_or_else`](sus-option-Option.html#method.unwrap_or_else) moves
///   and returns the inner value. It will return the result of evaluating
///   the provided function when empty.
///
/// # Copying
/// Most methods of Option act on an rvalue and consume the Option to transform
/// it into a new Option with a new value. This ensures that the value inside an
/// Option is moved while transforming it.
///
/// However, if [`Option`](sus-option-Option.html) is
/// [`Copy`](sus-mem-Copy.html), then the majority of methods offer an
/// overload to be called as an lvalue, in which case the
/// [`Option`](sus-option-Option.html) will copy
/// itself, and its contained value, and perform the intended method on the copy
/// instead. This can have performance implications!
///
/// The unwrapping methods are excluded from this, and are only available on an
/// rvalue [`Option`](sus-option-Option.html) to avoid copying just to access
/// the inner value. To do that, access the inner value as a reference through
/// [`as_value`](sus-option-Option.html#method.as_value) and
/// [`as_value_mut`](sus-option-Option.html#method.as_value_mut) or through
/// [`operator*`](sus-option-Option.html#method.operator*) and
/// [`operator->`](sus-option-Option.html#method.operator->).
///
/// # Transforming contained values
/// These methods transform [`Option`](sus-option-Option.html) to
/// [`Result`](sus-result-Result.html):
///
/// * [`ok_or`](sus-option-Option.html#method.ok_or) transforms `Some(v)`
///   to `Ok(v)`, and `None` to `Err(err)` using the provided default err value.
/// * [`ok_or_else`](sus-option-Option.html#method.ok_or_else) transforms
///   `Some(v)` to `Ok(v)`, and `None` to a value of `Err` using the
///   provided function.
/// * [`transpose`](sus-option-Option.html#method.transpose) transposes an
///   [`Option`](sus-option-Option.html) of a [`Result`](sus-result-Result.html)
///   into a [`Result`](sus-result-Result.html) of an
///   [`Option`](sus-option-Option.html).
///
/// These methods transform an option holding a value:
///
/// * [`filter`](sus-option-Option.html#method.filter) calls the provided
///   predicate function on the contained value `t` if the
///   [`Option`](sus-option-Option.html) is `Some(t)`, and returns `Some(t)`
///   if the function returns `true`; otherwise, returns `None`.
/// * [`flatten`](sus-option-Option.html#method.flatten) removes one level
///   of nesting from an `Option<Option<T>>`.
/// * [`map`](sus-option-Option.html#method.map) transforms
///   [`Option<T>`](sus-option-Option.html) to
///   [`Option<U>`](sus-option-Option.html) by applying the provided
///   function to the contained value of `Some` and leaving `None` values
///   unchanged.
///
/// These methods transform [`Option<T>`](sus-option-Option.html) to a value
/// of a possibly different type `U`:
///
/// * [`map_or`](sus-option-Option.html#method.map_or) applies the provided
///   function to the contained value of `Some`, or returns the provided
///   default value if the
///   [`Option`](sus-option-Option.html) is `None`.
/// * [`map_or_else`](sus-option-Option.html#method.map_or_else) applies the
///   provided function to the contained value of `Some`, or returns the result
///   of evaluating the provided fallback function if the
///   [`Option`](sus-option-Option.html) is `None`.
///
/// These methods combine the Some variants of two Option values:
///
/// * [`zip`](sus-option-Option.html#method.zip) returns `Some(Tuple<S, O>(s, o)))` if the
///   [`Option`](sus-option-Option.html) is `Some(s)`
///   and the method is called with an [`Option`](sus-option-Option.html) value of `Some(o)`; otherwise,
///   returns `None`
/// * TODO: [`zip_with`](sus-option-Option.html#method.zip_with) calls the provided function `f` and returns
///   `Some(f(s, o))` if the [`Option`](sus-option-Option.html) is Some(s)
///   and the method is called with an [`Option`](sus-option-Option.html)
///   value of `Some(o)`; otherwise, returns `None`.
///
/// # Boolean operators
/// These methods treat the [`Option`](sus-option-Option.html) as a boolean value,
/// where `Some` acts like `true` and `None` acts like `false`. There are two
/// categories of these methods: ones that take an
/// [`Option`](sus-option-Option.html) as input, and ones
/// that take a function as input (to be lazily evaluated).
///
/// The [`and_that`](sus-option-Option.html#method.and_that),
/// [`or_that`](sus-option-Option.html#method.or_that),
/// and [`xor_that`](sus-option-Option.html#method.xor_that) methods take
/// another [`Option`](sus-option-Option.html) as input, and produce an
/// [`Option`](sus-option-Option.html) as output.
/// Only the [`and_that`](sus-option-Option.html#method.and_that)
/// method can produce an [`Option<U>`](sus-option-Option.html) value having a different inner type `U`
/// than [`Option<T>`](sus-option-Option.html).
///
/// | method                                               | self    | input     | output  |
/// | ---------------------------------------------------- | ------- | --------- | ------- |
/// | [`and_that`](sus-option-Option.html#method.and_that) | None    | (ignored) | None    |
/// | [`and_that`](sus-option-Option.html#method.and_that) | Some(x) | None      | None    |
/// | [`and_that`](sus-option-Option.html#method.and_that) | Some(x) | Some(y)   | Some(y) |
/// | [`or_that`](sus-option-Option.html#method.or_that)   | None    | None      | None    |
/// | [`or_that`](sus-option-Option.html#method.or_that)   | None    | Some(y)   | Some(y) |
/// | [`or_that`](sus-option-Option.html#method.or_that)   | Some(x) | (ignored) | Some(x) |
/// | [`xor_that`](sus-option-Option.html#method.xor_that) | None    | None      | None    |
/// | [`xor_that`](sus-option-Option.html#method.xor_that) | None    | Some(y)   | Some(y) |
/// | [`xor_that`](sus-option-Option.html#method.xor_that) | Some(x) | None      | Some(x) |
/// | [`xor_that`](sus-option-Option.html#method.xor_that) | Some(x) | Some(y)   | None    |
///
/// The [`and_then`](sus-option-Option.html#method.and_then) and
/// [`or_else`](sus-option-Option.html#method.or_else) methods take a function
/// as input, and only evaluate the function when they need to produce a new
/// value. Only the [`and_then`](sus-option-Option.html#method.and_then) method
/// can produce an [`Option<U>`](sus-option-Option.html) value having a
/// different inner type `U` than [`Option<T>`](sus-option-Option.html).
///
/// | method                                               | self    | function input | function result | output  |
/// | ---------------------------------------------------- | ------- | -------------- | --------------- | ------- |
/// | [`and_then`](sus-option-Option.html#method.and_then) | None	   | (not provided) |	(not evaluated) | None    |
/// | [`and_then`](sus-option-Option.html#method.and_then) | Some(x) | x              | None            | None    |
/// | [`and_then`](sus-option-Option.html#method.and_then) | Some(x) | x              | Some(y)         | Some(y) |
/// | [`or_else`](sus-option-Option.html#method.or_else)   | None    | (not provided) | None            | None    |
/// | [`or_else`](sus-option-Option.html#method.or_else)   | None    | (not provided) | Some(y)         | Some(y) |
/// | [`or_else`](sus-option-Option.html#method.or_else)   | Some(x) | (not provided) | (not evaluated) | Some(x) |
///
/// This is an example of using methods like
/// [`and_then`](sus-option-Option.html#method.and_then) and
/// [`or_that`](sus-option-Option.html#method.or_that) in a pipeline of
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
/// sus::check(res == sus::vec("error!", "error!", "foo", "error!", "bar"));
/// ```
///
/// # Restrictions on returning references
///
/// Methods that return references are only callable on an rvalue
/// [`Option`](sus-option-Option.html) if the
/// [`Option`](sus-option-Option.html) is holding a reference. If the
/// [`Option`](sus-option-Option.html) is holding a non-reference
/// type, returning a reference from an rvalue
/// [`Option`](sus-option-Option.html) would be giving a reference to a
/// short-lived object which is a bugprone pattern in C++ leading to
/// memory-safety bugs.
namespace option {}
// clang-format on
}  // namespace sus

namespace sus::option {

using State::None;
using State::Some;
using ::sus::iter::Once;
using ::sus::mem::__private::IsTrivialCopyAssignOrRef;
using ::sus::mem::__private::IsTrivialCopyCtorOrRef;
using ::sus::mem::__private::IsTrivialDtorOrRef;
using ::sus::mem::__private::IsTrivialMoveAssignOrRef;
using ::sus::mem::__private::IsTrivialMoveCtorOrRef;
using ::sus::option::__private::Storage;
using ::sus::option::__private::StoragePointer;

namespace __private {
template <class T, ::sus::iter::Iterator<Option<T>> Iter>
  requires ::sus::iter::Product<T>
constexpr Option<T> from_product_impl(Iter&& it) noexcept;
template <class T, ::sus::iter::Iterator<Option<T>> Iter>
  requires ::sus::iter::Sum<T>
constexpr Option<T> from_sum_impl(Iter&& it) noexcept;
}  // namespace __private

/// The [`Option`](sus-option-Option.html) type.
///
/// See the [namespace level documentation](sus-option.html) for more.
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
  /// This satisfies [`Default`](sus-construct-Default.html) for
  /// [`Option`](sus-option-Option.html).
  /// #[doc.overloads=ctor.none]
  inline constexpr Option() noexcept = default;

  /// Construct an option that is holding the given value.
  ///
  /// # Const References
  ///
  /// For [`Option<const T&>`](sus-option-Option.html) it is possible to bind to
  /// a temporary which would create a memory safety bug. The
  /// `[[clang::lifetimebound]]` attribute is used to prevent this via Clang.
  /// But additionally, the incoming type is required to match with
  /// [`SafelyConstructibleFromReference`](sus-construct-SafelyConstructibleFromReference.html)
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
  explicit constexpr Option(U&& t) noexcept
    requires(!std::is_reference_v<T> &&  //
             ::sus::mem::Move<T> &&      //
             ::sus::mem::IsMoveRef<U &&>)
      : Option(WITH_SOME, move_to_storage(t)) {}

  /// #[doc.overloads=ctor.some]
  template <std::convertible_to<T> U>
  explicit constexpr Option(U&& t sus_lifetimebound) noexcept
    requires(std::is_reference_v<T> &&  //
             sus::construct::SafelyConstructibleFromReference<T, U &&>)
      : Option(WITH_SOME, move_to_storage(t)) {}

  /// Moves `val` into a new option holding `Some(val)`.
  ///
  /// Implements [`From<Option<T>, T>`](sus-construct-From.html).
  ///
  /// #[doc.overloads=from.t]
  template <class U>
    requires(std::constructible_from<Option, U &&>)
  static constexpr Option from(U&& val) noexcept
    requires(!std::is_reference_v<T>)
  {
    return Option(val);
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
  /// [`Option<T>`](sus-option-Option.html) as long as there is no `None` found.
  /// If a `None` is found, the function returns `None`.
  ///
  /// Prefer to call `product()` on the iterator rather than calling
  /// `from_product()` directly.
  ///
  /// Implements [`sus::iter::Product<Option<T>>`](sus-iter-Product.html).
  ///
  /// The product is computed using the implementation of the inner type `T`
  /// which also satisfies [`sus::iter::Product<T>`](sus-iter-Product.html).
  template <::sus::iter::Iterator<Option<T>> Iter>
    requires ::sus::iter::Product<T>
  static constexpr Option from_product(Iter&& it) noexcept {
    return __private::from_product_impl<T>(::sus::move(it));
  }

  /// Computes the sum of an iterator over [`Option<T>`](sus-option-Option.html)
  /// as long as there is no `None` found. If a `None` is found, the function
  /// returns `None`.
  ///
  /// Prefer to call `sum()` on the iterator rather than calling `from_sum()`
  /// directly.
  ///
  /// Implements [`sus::iter::Sum<Option<T>>`](sus-iter-Sum.html).
  ///
  /// The sum is computed using the implementation of the inner type `T`
  /// which also satisfies [`sus::iter::Sum<T>`](sus-iter-Sum.html).
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
  /// [`Option<T>`](sus-option-Option.html) to also be trivially destroyed.
  constexpr ~Option() noexcept
    requires(IsTrivialDtorOrRef<T>)
  = default;

  constexpr inline ~Option() noexcept
    requires(!IsTrivialDtorOrRef<T>)
  {
    if (t_.state() == Some) t_.destroy();
  }

  /// Copy constructor for [`Option<T>`](sus-option-Option.html) which will
  /// satisfy [`Copy<Option<T>>`](sus-mem-Copy.html) if
  /// [`Copy<T>`](sus-mem-Copy.html) is satisfied.
  ///
  /// If `T` can be trivially copy-constructed, then `Option<T>` can also be
  /// trivially copy-constructed.
  ///
  /// #[doc.overloads=copy]
  constexpr Option(const Option& o)
    requires(::sus::mem::CopyOrRef<T> && IsTrivialCopyCtorOrRef<T>)
  = default;

  /// #[doc.overloads=copy]
  constexpr Option(const Option& o) noexcept
    requires(::sus::mem::CopyOrRef<T> && !IsTrivialCopyCtorOrRef<T>)
  {
    if (o.t_.state() == Some)
      t_.construct_from_none(copy_to_storage(o.t_.val()));
  }

  /// #[doc.overloads=copy]
  constexpr Option(const Option& o)
    requires(!::sus::mem::CopyOrRef<T>)
  = delete;

  /// Move constructor for [`Option<T>`](sus-option-Option.html) which will
  /// satisfy [`Move<Option<T>>`](sus-mem-Move.html) if
  /// [`Move<T>`](sus-mem-Move.html) is satisfied.
  ///
  /// If `T` can be trivially move-constructed, then `Option<T>` can also be
  /// trivially move-constructed. When trivially-moved, the option is copied on
  /// move, and the moved-from Option is unchanged but should still not be used
  /// thereafter without reinitializing it. Use `take()` instead to move the
  /// value out of the option when the option may be used again afterward.
  ///
  /// #[doc.overloads=move]
  constexpr Option(Option&& o)
    requires(::sus::mem::MoveOrRef<T> && IsTrivialMoveCtorOrRef<T>)
  = default;

  /// #[doc.overloads=move]
  constexpr Option(Option&& o) noexcept
    requires(::sus::mem::MoveOrRef<T> && !IsTrivialMoveCtorOrRef<T>)
  {
    if (o.t_.state() == Some) t_.construct_from_none(o.t_.take_and_set_none());
  }

  /// #[doc.overloads=move]
  constexpr Option(Option&& o)
    requires(!::sus::mem::MoveOrRef<T>)
  = delete;

  /// Copy assignment for [`Option<T>`](sus-option-Option.html) which will
  /// satisfy [`Copy<Option<T>>`](sus-mem-Copy.html) if
  /// [`Copy<T>`](sus-mem-Copy.html) is satisfied.
  ///
  /// If `T` can be trivially copy-assigned, then
  /// [`Option<T>`](sus-option-Option.html) can also be
  /// trivially copy-assigned.
  ///
  /// #[doc.overloads=copy]
  constexpr Option& operator=(const Option& o)
    requires(::sus::mem::CopyOrRef<T> && IsTrivialCopyAssignOrRef<T>)
  = default;

  /// #[doc.overloads=copy]
  constexpr Option& operator=(const Option& o) noexcept
    requires(::sus::mem::CopyOrRef<T> && !IsTrivialCopyAssignOrRef<T>)
  {
    if (o.t_.state() == Some)
      t_.set_some(copy_to_storage(o.t_.val()));
    else if (t_.state() == Some)
      t_.set_none();
    return *this;
  }

  /// #[doc.overloads=copy]
  constexpr Option& operator=(const Option& o)
    requires(!::sus::mem::CopyOrRef<T>)
  = delete;

  /// Move assignment for [`Option<T>`](sus-option-Option.html) which will
  /// satisfy [`Move<Option<T>>`](sus-mem-Move.html) if
  /// [`Move<T>`](sus-mem-Move.html) is satisfied.
  ///
  /// If `T` can be trivially move-assigned, then
  /// [`Option<T>`](sus-option-Option.html) can also be trivially move-assigned.
  /// When trivially-moved, the option is copied on move, and the moved-from
  /// option is unchanged but should still not be used thereafter without
  /// reinitializing it. Use `take()` instead to move the value out of the
  /// option when the option may be used again afterward.
  ///
  /// #[doc.overloads=move]
  constexpr Option& operator=(Option&& o)
    requires(::sus::mem::MoveOrRef<T> && IsTrivialMoveAssignOrRef<T>)
  = default;

  /// #[doc.overloads=move]
  constexpr Option& operator=(Option&& o) noexcept
    requires(::sus::mem::MoveOrRef<T> && !IsTrivialMoveAssignOrRef<T>)
  {
    if (o.t_.state() == Some)
      t_.set_some(o.t_.take_and_set_none());
    else if (t_.state() == Some)
      t_.set_none();
    return *this;
  }

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
  /// [`unwrap`](sus-option-Option.html#method.unwrap) or
  /// [`expect`](sus-option-Option.html#method.expect). For lvalues, it can be
  /// accessed as a reference through
  /// [`as_value`](sus-option-Option.html#method.as_value) and
  /// [`as_value_mut`](sus-option-Option.html#method.as_value_mut) for explicit
  /// const/mutable access, or through
  /// [`operator*`](sus-option-Option.html#method.operator*)
  /// and [`operator->`](sus-option-Option.html#method.operator->).
  sus_pure constexpr bool is_some() const noexcept {
    return t_.state() == Some;
  }
  /// Returns whether the option is currently empty, containing no value.
  sus_pure constexpr bool is_none() const noexcept {
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
  sus_pure constexpr operator State() const& { return t_.state(); }

  /// Returns the contained value inside the option.
  ///
  /// The function will panic with the given `message` if the option's state is
  /// currently `None`.
  constexpr sus_nonnull_fn T expect(
      /* TODO: string view type */ sus_nonnull_arg const char* sus_nonnull_var
          message) && noexcept {
    ::sus::check_with_message(t_.state() == Some, *message);
    return ::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn);
  }

  /// Returns the contained value inside the option.
  ///
  /// The function will panic without a message if the option's state is
  /// currently `None`.
  constexpr T unwrap() && noexcept {
    ::sus::check(t_.state() == Some);
    return ::sus::move(*this).unwrap_unchecked(::sus::marker::unsafe_fn);
  }

  /// Returns the contained value inside the option, if there is one. Otherwise,
  /// returns `default_result`.
  ///
  /// Note that if it is non-trivial to construct a `default_result`, that
  /// [`unwrap_or_else`](sus-option-Option.html#method.unwrap_or_else) should be
  /// used instead, as it will only construct the default value if required.
  constexpr T unwrap_or(T default_result) && noexcept {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      return default_result;
    }
  }

  /// Returns the contained value inside the Option, if there is one.
  /// Otherwise, returns the result of the given function.
  constexpr T unwrap_or_else(::sus::fn::FnOnce<T()> auto&& f) && noexcept {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      return ::sus::fn::call_once(::sus::move(f));
    }
  }

  /// Returns the contained value inside the option, if there is one.
  /// Otherwise, constructs a default value for the type and returns that.
  ///
  /// The option's contained type `T` must be
  /// [`Default`](sus-construct-Default.html) in order to be constructed with a
  /// default value.
  constexpr T unwrap_or_default() && noexcept
    requires(!std::is_reference_v<T> && ::sus::construct::Default<T>)
  {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      return T();
    }
  }

  /// Returns the contained value inside the option.
  ///
  /// # Safety
  ///
  /// It is Undefined Behaviour to call this function when the option's state is
  /// `None`. The caller is responsible for ensuring the option contains a value
  /// beforehand, and the safer [`unwrap`](sus-option-Option.html#method.unwrap)
  /// or [`expect`](sus-option-Option.html#method.expect) should almost always
  /// be preferred. The compiler will typically elide the checks if they program
  /// verified the value appropriately before use in order to not panic.
  constexpr inline T unwrap_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    return t_.take_and_set_none();
  }

  /// Returns a const reference to the contained value inside the option.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap`](sus-option-Option.html#method.unwrap) on
  /// an rvalue, and [`take`](sus-option-Option.html#method.take) to move the
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
  sus_pure constexpr const std::remove_reference_t<T>& as_value()
      const& noexcept {
    ::sus::check(t_.state() == Some);
    return t_.val();
  }
  sus_pure constexpr const std::remove_reference_t<T>& as_value() && noexcept
    requires(std::is_reference_v<T>)
  {
    ::sus::check(t_.state() == Some);
    return t_.val();
  }
  /// Returns a mutable reference to the contained value inside the option.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap`](sus-option-Option.html#method.unwrap) on
  /// an rvalue, and [`take`](sus-option-Option.html#method.take) to move the
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
  sus_pure constexpr std::remove_reference_t<T>& as_value_mut() & noexcept {
    ::sus::check(t_.state() == Some);
    return t_.val_mut();
  }
  sus_pure constexpr std::remove_reference_t<T>& as_value_mut() && noexcept
    requires(std::is_reference_v<T>)
  {
    ::sus::check(t_.state() == Some);
    return t_.val_mut();
  }
  sus_pure constexpr std::remove_reference_t<T>& as_value_mut() const& noexcept
    requires(std::is_reference_v<T>)
  {
    return ::sus::clone(*this).as_value_mut();
  }

  /// Returns a const reference to the contained value inside the option.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap_unchecked`](sus-option-Option.html#method.unwrap_unchecked) on
  /// an rvalue, and [`take`](sus-option-Option.html#method.take) to move the
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
  sus_pure constexpr const std::remove_reference_t<T>& as_value_unchecked(
      ::sus::marker::UnsafeFnMarker) const& noexcept {
    return t_.val();
  }
  sus_pure constexpr const std::remove_reference_t<T>& as_value_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept
    requires(std::is_reference_v<T>)
  {
    return t_.val();
  }
  /// Returns a mutable reference to the contained value inside the option.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap_unchecked`](sus-option-Option.html#method.unwrap_unchecked) on
  /// an rvalue, and [`take`](sus-option-Option.html#method.take) to move the
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
  sus_pure constexpr std::remove_reference_t<T>& as_value_unchecked_mut(
      ::sus::marker::UnsafeFnMarker) & noexcept {
    return t_.val_mut();
  }
  sus_pure constexpr std::remove_reference_t<T>& as_value_unchecked_mut(
      ::sus::marker::UnsafeFnMarker) && noexcept
    requires(std::is_reference_v<T>)
  {
    return t_.val_mut();
  }
  sus_pure constexpr std::remove_reference_t<T>& as_value_unchecked_mut(
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
  /// [`unwrap`](sus-option-Option.html#method.unwrap) on
  /// an rvalue, and [`take`](sus-option-Option.html#method.take) to move the
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
  ///   [`unwrap`](sus-option-Option.html#method.unwrap) on lvalues loud.
  /// * Unwrapping requires a new lvalue name since C++ doesn't allow name
  ///   reuse, making variable names bad.
  /// * We also provide [`as_value`](sus-option-Option.html#method.as_value) and
  ///   [`as_value_mut`](sus-option-Option.html#method.as_value_mut) for
  ///   explicit const/mutable lvalue access but...
  /// * It's expected in C++ ecosystems, due to
  ///   [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional)
  ///   and other pre-existing collection-of-one things to provide access
  ///   through [`operator*`](sus-option-Option.html#method.operator*) and
  ///   [`operator->`](sus-option-Option.html#method.operator->).
  sus_pure constexpr const std::remove_reference_t<T>& operator*()
      const& noexcept {
    ::sus::check(t_.state() == Some);
    return t_.val();
  }
  sus_pure constexpr const std::remove_reference_t<T>& operator*() && noexcept
    requires(std::is_reference_v<T>)
  {
    ::sus::check(t_.state() == Some);
    return t_.val();
  }
  sus_pure constexpr std::remove_reference_t<T>& operator*() & noexcept {
    ::sus::check(t_.state() == Some);
    return t_.val_mut();
  }

  /// Returns a pointer to the contained value inside the option.
  ///
  /// The pointer is const if the option is const, and is mutable otherwise.
  /// This method allows calling methods directly on the type inside the option
  /// without unwrapping.
  ///
  /// To extract the value inside an option, use
  /// [`unwrap`](sus-option-Option.html#method.unwrap) on
  /// an rvalue, and [`take`](sus-option-Option.html#method.take) to move the
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
  ///   [`unwrap`](sus-option-Option.html#method.unwrap) on lvalues loud.
  /// * Unwrapping requires a new lvalue name since C++ doesn't allow name
  ///   reuse, making variable names bad.
  /// * We also provide [`as_value`](sus-option-Option.html#method.as_value) and
  /// [`as_value_mut`](sus-option-Option.html#method.as_value_mut) for explicit
  ///   const/mutable lvalue access but...
  /// * It's expected in C++ ecosystems, due to
  ///   [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional)
  ///   and other pre-existing collection-of-one things to provide access
  ///   through [`operator*`](sus-option-Option.html#method.operator*) and
  ///   [`operator->`](sus-option-Option.html#method.operator->).
  sus_pure constexpr const std::remove_reference_t<T>* operator->()
      const& noexcept {
    ::sus::check(t_.state() == Some);
    return ::sus::mem::addressof(
        static_cast<const std::remove_reference_t<T>&>(t_.val()));
  }
  sus_pure constexpr const std::remove_reference_t<T>* operator->() && noexcept
    requires(std::is_reference_v<T>)
  {
    ::sus::check(t_.state() == Some);
    return ::sus::mem::addressof(
        static_cast<const std::remove_reference_t<T>&>(t_.val()));
  }
  sus_pure constexpr std::remove_reference_t<T>* operator->() & noexcept {
    ::sus::check(t_.state() == Some);
    return ::sus::mem::addressof(
        static_cast<std::remove_reference_t<T>&>(t_.val_mut()));
  }

  /// Inserts `value` into the option, then returns a mutable reference to it.
  ///
  /// If the option already contains a value, the old value is dropped.
  ///
  /// See also
  /// [`Option::get_or_insert`](sus-option-Option.html#method.get_or_insert),
  /// which doesn’t update the value if the option already contains `Some`.
  constexpr T& insert(T value) & noexcept
    requires(sus::mem::MoveOrRef<T>)
  {
    t_.set_some(move_to_storage(value));
    return t_.val_mut();
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// stores `value` inside the Option and returns a mutable reference to it.
  ///
  /// If it is non-trivial to construct `T`, the
  /// [`Option::get_or_insert_with`](sus-option-Option.html#method.get_or_insert_with)
  /// method would be preferable, as it only constructs a `T` if needed.
  constexpr T& get_or_insert(T value) & noexcept sus_lifetimebound
    requires(sus::mem::MoveOrRef<T>)
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
  /// [`unwrap_or_default`](sus-option-Option.html#method.unwrap_or_default)
  /// in that it does not consume the option, and instead it can not be called
  /// on rvalues.
  ///
  /// This is a shorthand for
  /// `Option<T>::get_or_insert_with([] { return T(); })`.
  ///
  /// The option's contained type `T` must satisfy
  /// [`Default`](sus-construct-Default.html) so it can be constructed with its
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
  /// [`unwrap_or_else`](sus-option-Option.html#method.unwrap_or_else) in that
  /// it does not consume the option, and instead it can not be called on
  /// rvalues.
  constexpr T& get_or_insert_with(::sus::fn::FnOnce<T()> auto&& f) & noexcept {
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
  /// [`Option<R>`](sus-option-Option.html) where `R` is the
  /// return type of the map function.
  ///
  /// Returns an [`Option<R>`](sus-option-Option.html) in state `None`
  /// if the current option is in state `None`.
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn, int&...,
            class R = std::invoke_result_t<MapFn&&, T&&>>
  constexpr Option<R> map(MapFn&& m) && noexcept {
    if (t_.state() == Some) {
      return Option<R>(
          Option<R>::WITH_SOME,
          ::sus::fn::call_once(::sus::move(m),
                               static_cast<T&&>(t_.take_and_set_none())));
    } else {
      return Option<R>();
    }
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn, int&...,
            class R = std::invoke_result_t<MapFn&&, T&&>>
  constexpr Option<R> map(MapFn&& m) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).map(::sus::move(m));
  }

  /// Returns the provided default result (if none), or applies a function to
  /// the contained value (if any).
  ///
  /// Arguments passed to `map_or` are eagerly evaluated; if you are passing the
  /// result of a function call, it is recommended to use
  /// [`map_or_else`](sus-option-Option.html#method.map_or_else), which is
  /// lazily evaluated.
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn, int&...,
            class R = std::invoke_result_t<MapFn&&, T&&>>
  constexpr R map_or(R default_result, MapFn&& m) && noexcept {
    if (t_.state() == Some) {
      return ::sus::fn::call_once(::sus::move(m),
                                  static_cast<T&&>(t_.take_and_set_none()));
    } else {
      return default_result;
    }
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn, int&...,
            class R = std::invoke_result_t<MapFn&&, T&&>>
  constexpr R map_or(R default_result, MapFn&& m) const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).map_or(::sus::move(default_result),
                                      ::sus::move(m));
  }

  /// Computes a default function result (if none), or applies a different
  /// function to the contained value (if any).
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> DefaultFn,
            ::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn, int&...,
            class D = std::invoke_result_t<DefaultFn>,
            class R = std::invoke_result_t<MapFn, T&&>>
    requires(std::is_same_v<D, R>)
  constexpr R map_or_else(DefaultFn&& default_fn, MapFn&& m) && noexcept {
    if (t_.state() == Some) {
      return ::sus::fn::call_once(::sus::move(m),
                                  static_cast<T&&>(t_.take_and_set_none()));
    } else {
      return ::sus::move(default_fn)();
    }
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> DefaultFn,
            ::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> MapFn, int&...,
            class D = std::invoke_result_t<DefaultFn>,
            class R = std::invoke_result_t<MapFn, T&&>>
    requires(std::is_same_v<D, R>)
  constexpr R map_or_else(DefaultFn&& default_fn, MapFn&& m) const& noexcept
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
      ::sus::fn::FnOnce<bool(const std::remove_reference_t<T>&)> auto&&
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
      ::sus::fn::FnOnce<bool(const std::remove_reference_t<T>&)> auto&& p)
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
  /// [`Option<U>`](sus-option-Option.html).
  ///
  /// Some languages call this operation flatmap.
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> AndFn, int&...,
            class R = std::invoke_result_t<AndFn, T&&>,
            class U = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  constexpr Option<U> and_then(AndFn&& f) && noexcept {
    if (t_.state() == Some)
      return ::sus::fn::call_once(::sus::move(f),
                                  static_cast<T&&>(t_.take_and_set_none()));
    else
      return Option<U>();
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid(T&&)> AndFn, int&...,
            class R = std::invoke_result_t<AndFn, T&&>,
            class U = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  constexpr Option<U> and_then(AndFn&& f) const& noexcept
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
      ::sus::fn::FnOnce<Option<T>()> auto&& f) && noexcept {
    if (t_.state() == Some)
      return Option(WITH_SOME, t_.take_and_set_none());
    else
      return ::sus::fn::call_once(::sus::move(f));
  }
  constexpr Option<T> or_else(
      ::sus::fn::FnOnce<Option<T>()> auto&& f) const& noexcept
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

  /// Transforms the [`Option<T>`](sus-option-Option.html) into a
  /// [`Result<T, E>`](sus-result-Result.html), mapping `Some(v)` to `Ok(v)` and
  /// `None` to `Err(e)`.
  ///
  /// Arguments passed to `ok_or` are eagerly evaluated; if you are passing the
  /// result of a function call, it is recommended to use
  /// [`ok_or_else`](sus-option-Option.html#method.ok_or_else), which is
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

  /// Transforms the [`Option<T>`](sus-option-Option.html) into a
  /// [`Result<T, E>`](sus-result-Result.html), mapping `Some(v)` to `Ok(v)` and
  /// `None` to `Err(f())`.
  //
  // TODO: No refs in Result: https://github.com/chromium/subspace/issues/133
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> ElseFn, int&...,
            class E = std::invoke_result_t<ElseFn>,
            class Result = ::sus::result::Result<T, E>>
  constexpr Result ok_or_else(ElseFn&& f) && noexcept
    requires(!std::is_reference_v<T> && !std::is_reference_v<E>)
  {
    if (t_.state() == Some)
      return Result(t_.take_and_set_none());
    else
      return Result::with_err(::sus::fn::call_once(::sus::move(f)));
  }
  template <::sus::fn::FnOnce<::sus::fn::NonVoid()> ElseFn, int&...,
            class E = std::invoke_result_t<ElseFn>,
            class Result = ::sus::result::Result<T, E>>
  constexpr Result ok_or_else(ElseFn&& f) const& noexcept
    requires(!std::is_reference_v<T> && !std::is_reference_v<E> &&
             ::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).ok_or_else(::sus::move(f));
  }

  /// Transposes an [`Option`](sus-option-Option.html) of a
  /// [`Result`](sus-result-Result.html) into a
  /// [`Result`](sus-result-Result.html) of an
  /// [`Option`](sus-option-Option.html).
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

  /// Unzips an option holding a [`Tuple`](sus-tuple_type-Tuple.html) of two
  /// values into a [`Tuple`](sus-tuple_type-Tuple.html) of two
  /// [`Option`](sus-option-Option.html)s.
  ///
  /// [`Option<Tuple<i32, u32>>`](sus-option-Option.html) is unzipped to
  /// [`Tuple<Option<i32>, Option<u32>>`](sus-tuple_type-Tuple.html).
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

  /// Replaces whatever the Option is currently holding with `Some` value `t`
  /// and returns an Option holding what was there previously.
  constexpr Option replace(T t) & noexcept
    requires(sus::mem::MoveOrRef<T>)
  {
    if (t_.state() == None) {
      t_.construct_from_none(move_to_storage(t));
      return Option();
    } else {
      return Option(WITH_SOME, t_.replace_some(move_to_storage(t)));
    }
  }

  /// Maps an [`Option<T&>`](sus-option-Option.html) to an
  /// [`Option<T>`](sus-option-Option.html) by copying the referenced `T`.
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

  /// Maps an [`Option<T&>`](sus-option-Option.html) to an
  /// [`Option<T>`](sus-option-Option.html) by cloning the referenced `T`.
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

  /// Maps an [`Option<Option<T>>`](sus-option-Option.html) to an
  /// [`Option<T>`](sus-option-Option.html).
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

  /// Returns an [`Option<const T&>`](sus-option-Option.html) from this
  /// [`Option<T>`](sus-option-Option.html), that either holds `None` or a
  /// reference to the value in this option.
  ///
  /// # Implementation Notes
  ///
  /// Implementation note: We only allow calling this on an rvalue option if the
  /// contained value is a reference, otherwise we are returning a reference to
  /// a short-lived object which leads to common C++ memory bugs.
  sus_pure constexpr Option<const std::remove_reference_t<T>&> as_ref()
      const& noexcept {
    if (t_.state() == None)
      return Option<const std::remove_reference_t<T>&>();
    else
      return Option<const std::remove_reference_t<T>&>(
          Option<const std::remove_reference_t<T>&>::WITH_SOME, t_.val());
  }
  sus_pure constexpr Option<const std::remove_reference_t<T>&>
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

  /// Returns an [`Option<T&>`](sus-option-Option.html) from this
  /// [`Option<T>`](sus-option-Option.html), that either holds `None` or a
  /// reference to the value in this option.
  sus_pure constexpr Option<T&> as_mut() & noexcept {
    if (t_.state() == None)
      return Option<T&>();
    else
      return Option<T&>(Option<T&>::WITH_SOME, t_.val_mut());
  }
  // Calling as_mut() on an rvalue is not returning a reference to the inner
  // value if the inner value is already a reference, so we allow calling it on
  // an rvalue Option in that case.
  sus_pure constexpr Option<T&> as_mut() && noexcept
    requires(std::is_reference_v<T>)
  {
    if (t_.state() == None)
      return Option<T&>();
    else
      return Option<T&>(Option<T&>::WITH_SOME, t_.take_and_set_none());
  }
  sus_pure constexpr Option<T&> as_mut() const& noexcept
    requires(std::is_reference_v<T>)
  {
    return ::sus::clone(*this).as_mut();
  }

  sus_pure constexpr Once<const std::remove_reference_t<T>&> iter()
      const& noexcept {
    return ::sus::iter::once<const std::remove_reference_t<T>&>(as_ref());
  }
  constexpr Once<const std::remove_reference_t<T>&> iter() && noexcept
    requires(std::is_reference_v<T>)
  {
    return ::sus::iter::once<const std::remove_reference_t<T>&>(
        ::sus::move(*this).as_ref());
  }
  constexpr Once<const std::remove_reference_t<T>&> iter() const& noexcept
    requires(std::is_reference_v<T>)
  {
    return ::sus::clone(*this).iter();
  }

  sus_pure constexpr Once<T&> iter_mut() & noexcept {
    return ::sus::iter::once<T&>(as_mut());
  }
  constexpr Once<T&> iter_mut() && noexcept
    requires(std::is_reference_v<T>)
  {
    return ::sus::iter::once<T&>(::sus::move(*this).as_mut());
  }
  constexpr Once<T&> iter_mut() const& noexcept
    requires(std::is_reference_v<T>)
  {
    return ::sus::clone(*this).iter_mut();
  }

  constexpr Once<T> into_iter() && noexcept {
    return ::sus::iter::once<T>(take());
  }
  constexpr Once<T> into_iter() const& noexcept
    requires(::sus::mem::CopyOrRef<T>)
  {
    return ::sus::clone(*this).into_iter();
  }

  /// Satisfies the [`Eq<Option<U>>`](sus-ops-Eq.html) concept.
  ///
  /// The non-template overload allows some/none marker types to convert to
  /// Option for comparison.
  friend constexpr inline bool operator==(const Option& l,
                                          const Option& r) noexcept
    requires(::sus::ops::Eq<T>)
  {
    switch (l) {
      case Some:
        return r.is_some() && (l.as_value_unchecked(::sus::marker::unsafe_fn) ==
                               r.as_value_unchecked(::sus::marker::unsafe_fn));
      case None: return r.is_none();
    }
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }
  template <class U>
    requires(::sus::ops::Eq<T, U>)
  friend constexpr inline bool operator==(const Option<T>& l,
                                          const Option<U>& r) noexcept {
    switch (l) {
      case Some:
        return r.is_some() && (l.as_value_unchecked(::sus::marker::unsafe_fn) ==
                               r.as_value_unchecked(::sus::marker::unsafe_fn));
      case None: return r.is_none();
    }
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  template <class U>
    requires(!::sus::ops::Eq<T, U>)
  friend constexpr inline bool operator==(const Option<T>& l,
                                          const Option<U>& r) = delete;

  /// Compares two options.
  ///
  /// * Satisfies [`StrongOrd<Option<T>>`](sus-ops-StrongOrd.html) if
  ///   [`StrongOrd<T>`](sus-ops-StrongOrd.html).
  /// * Satisfies [`Ord<Option<T>>`](sus-ops-Ord.html) if
  ///   [`Ord<T>`](sus-ops-Ord.html).
  /// * Satisfies [`PartialOrd<Option<T>>`](sus-ops-PartialOrd.html) if
  ///   [`PartialOrd<T>`](sus-ops-PartialOrd.html).
  ///
  /// The non-template overloads allow some/none marker types to convert to
  /// an option for comparison.
  //
  // sus::ops::StrongOrd<Option<U>> trait.
  friend constexpr inline std::strong_ordering operator<=>(
      const Option& l, const Option& r) noexcept
    requires(::sus::ops::ExclusiveStrongOrd<T>)
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
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }
  template <class U>
    requires(::sus::ops::ExclusiveStrongOrd<T, U>)
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
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  // sus::ops::Ord<Option<U>> trait.
  friend constexpr inline std::weak_ordering operator<=>(
      const Option& l, const Option& r) noexcept
    requires(::sus::ops::ExclusiveOrd<T>)
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
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }
  template <class U>
    requires(::sus::ops::ExclusiveOrd<T, U>)
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
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  // sus::ops::PartialOrd<Option<U>> trait.
  template <class U>
    requires(::sus::ops::ExclusivePartialOrd<T, U>)
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
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }
  friend constexpr inline std::partial_ordering operator<=>(
      const Option& l, const Option& r) noexcept
    requires(::sus::ops::ExclusivePartialOrd<T>)
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
    ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

  template <class U>
    requires(!::sus::ops::PartialOrd<T, U>)
  friend constexpr inline auto operator<=>(
      const Option<T>& l, const Option<U>& r) noexcept = delete;

  /// Implements [`From<std::optional>`](sus-construct-From.html).
  ///
  /// This also allows `sus::into()` to convert with type deduction from
  /// [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional)
  /// to [`sus::Option`](sus-option-Option.html).
  ///
  /// #[doc.overloads=from.optional]
  constexpr static Option from(
      const std::optional<std::remove_reference_t<T>>& s) noexcept
    requires(::sus::mem::Copy<T> && !std::is_reference_v<T>)
  {
    if (s.has_value())
      return Option(s.value());
    else
      return Option();
  }
  /// #[doc.overloads=from.optional]
  constexpr static Option from(
      std::optional<std::remove_reference_t<T>>&& s) noexcept
    requires(::sus::mem::Move<T> && !std::is_reference_v<T>)
  {
    if (s.has_value())
      return Option(::sus::move(s).value());
    else
      return Option();
  }
  /// Implements [`From<std::optional<U>>`](sus-construct-From.html) when `U`
  /// can be converted to `T`, i.e. [`Into<U, T>`](sus-construct-Into.html).
  ///
  /// #[doc.overloads=from.optional.u]
  template <::sus::construct::Into<T> U>
  static inline constexpr Option from(const std::optional<U>& s) noexcept
    requires(!std::is_reference_v<T>)
  {
    if (s.has_value())
      return Option(::sus::into(s.value()));
    else
      return Option();
  }
  /// #[doc.overloads=from.optional.u]
  template <::sus::construct::Into<T> U>
  static inline constexpr Option from(std::optional<U>&& s) noexcept
    requires(!std::is_reference_v<T>)
  {
    if (s.has_value())
      return Option(::sus::into(::sus::move(s).value()));
    else
      return Option();
  }
  /// Implicit conversion from [`std::optional`](
  /// https://en.cppreference.com/w/cpp/utility/optional).
  ///
  /// Prevents conversions from `U` to `optional<T>` when constructing
  /// `Option<optional<T>>`.
  ///
  /// #[doc.overloads=ctor.optional]
  constexpr Option(
      std::same_as<std::optional<std::remove_reference_t<T>>> auto s) noexcept
    requires(!std::is_reference_v<T> &&  //
             ::sus::mem::Move<T>)
      : Option() {
    if (s.has_value()) insert(::sus::move(s).value());
  }
  /// Implicit conversion to [`std::optional`](
  /// https://en.cppreference.com/w/cpp/utility/optional).
  ///
  /// #[doc.overloads=convert.optional]
  constexpr operator std::optional<
      std::remove_const_t<std::remove_reference_t<T>>>() const& noexcept
    requires(::sus::mem::Copy<T> && !std::is_reference_v<T>)
  {
    if (is_some()) {
      return std::optional<std::remove_reference_t<T>>(std::in_place,
                                                       as_value());
    } else {
      return std::nullopt;
    }
  }
  /// #[doc.overloads=convert.optional]
  constexpr operator std::optional<std::remove_reference_t<T>>() && noexcept
    requires(::sus::mem::Move<T> && !std::is_reference_v<T>)
  {
    if (is_some()) {
      return std::optional<std::remove_reference_t<T>>(
          std::in_place, ::sus::move(*this).unwrap());
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
    if constexpr (std::is_reference_v<T>)
      return StoragePointer<T>(t);
    else
      return ::sus::move(t);
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
  StorageType<T> t_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           StorageType<T>);
};

/// Implicit for-ranged loop iteration via
/// [`Option::iter`](sus-option-Option.html#method.iter).
using ::sus::iter::__private::begin;
/// Implicit for-ranged loop iteration via
/// [`Option::iter`](sus-option-Option.html#method.iter).
using ::sus::iter::__private::end;

/// Used to construct an option with a Some(t) value.
///
/// Calling some() produces a hint to make an option but does not actually
/// construct option. This is to allow constructing an
/// [`Option<T>`](sus-option-Option.html) or
/// [`Option<T&>`](sus-option-Option.html) correctly.
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
sus_pure_const inline constexpr auto none() noexcept {
  return __private::NoneMarker();
}

}  // namespace sus::option

// Implements [Try](sus-ops-Try.html) for [Option](sus-option-Option.html).
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
  // [`Option<T>`](sus-option-Option.html) if `T` satisfies
  // [`Default`](sus-construct-Default.html).
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
  requires(::sus::ops::Eq<T>)
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
sus__format_to_stream(sus::option, Option, T);

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
