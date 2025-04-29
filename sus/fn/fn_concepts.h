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

// IWYU pragma: private, include "sus/fn/fn.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <functional>
#include <type_traits>

#include "sus/fn/__private/signature.h"
#include "sus/macros/inline.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"

namespace sus::fn {

/// When used as the return type of the function signature in
/// [`Fn`]($sus::fn::Fn), [`FnMut`]($sus::fn::FnMut) and
/// [`FnOnce`]($sus::fn::FnOnce), the concepts will match against any return
/// type from a functor except `void`.
///
/// Use [`Anything`]($sus::fn::Anything) to include `void` as an accepted return
/// type.
struct NonVoid {
  template <class T>
  constexpr NonVoid(T&&) noexcept {}
};

/// When used as the return type of the function signature in
/// [`Fn`]($sus::fn::Fn), [`FnMut`]($sus::fn::FnMut) and
/// [`FnOnce`]($sus::fn::FnOnce), the concepts will match against any return
/// type from a functor including `void`.
///
/// Use [`NonVoid`]($sus::fn::NonVoid) to exclude `void` as an accepted return
/// type.
struct Anything {
  template <class T>
  constexpr Anything(T&&) noexcept {}
};

/// The version of a callable object that may be called only once.
///
/// A `FnOnce` is typically the best fit for any callable that will
/// only be called at most once.
///
/// A type that satisfies `FnOnce` will return a type that can be converted to
/// `R` when called with the arguments `Args...`. `FnOnce` is satisfied by
/// being callable as an rvalue (which is done by providing an operator() that
/// is not `&`-qualified). Mutable and const lambdas will satisfy `FnOnce`.
///
/// The second argument of `FnOnce<F, S>` is a function signature with the
/// format `ReturnType(Args...)`, where `Args...` are the arguments that will
/// be passed to the `FnOnce` and `ReturnType` is what is expected to be
/// received back. It would appear as a matching concept as:
/// ```
/// void function(FnOnce<ReturnType(Args...)> auto f) { ... }
/// ```
///
/// # Use of `FnOnce`
/// `FnOnce` should be received by value typically. If received as a rvalue
/// (universal) reference, it should be constrained by
/// [`IsMoveRef<decltype(f)>`]($sus::mem::IsMoveRef) to avoid moving out of
/// an lvalue in the caller.
///
/// A `FnOnce` should be called by moving it with `sus::move()` when passing it
/// to [`call_once`]($sus::fn::call_once) along with any arguments.
/// This ensures the
/// correct overload is called on the object and that method pointers are
/// called correctly. It is moved-from after calling, and it should only be
/// called once.
///
/// Calling a `FnOnce` multiple times may [`panic`]($sus::panic)
/// or cause Undefined Behaviour.
/// Not moving the `FnOnce` when calling it may fail to compile,
/// [`panic`]($sus::panic), or cause Undefined Behaviour depending on the type
/// that is being used to satisfy `FnOnce`.
///
/// # Type erasure
///
/// Using a concept like `FnOnce` in a function parameter requires the function
/// to be a template. Template functions can not be virtual, they must appear
/// in the header, and they can have a negative impact on binary size. So it can
/// be desirable to work with a `FnOnce` without templates.
///
/// To do so, `FnOnce` supports being type-erased, on the heap or the stack,
/// into a [`DynFnOnce`]($sus::fn::DynFnOnce) type.
/// To receive ownership of a type-erased `FnOnce`, receive a
/// [`Box`]($sus::boxed::Box)`<`[`DynFnOnce<R(Args...)>`]($sus::fn::DynFnOnce)`>`
/// instead.
/// To receive a reference to a type-erased `FnOnce`, receive a
/// [`DynFnOnce<R(Args...)>&&`]($sus::fn::DynFnOnce) instead.
///
/// See [`DynConcept`]($sus::boxed::DynConcept) for more on type erasure of
/// concept-satisfying types.
///
/// # Compatibility
/// Any callable type that satisfies `Fn` or `FnMut` will also satisfy `FnOnce`.
/// While a `FnOnce` should only be called once, this is compatible with the
/// requirements of `FnMut` and `Fn` which can be called only a single time. As
/// well, `FnOnce` is allowed to mutate internal state, but it does not have to,
/// which is compatible with the const nature of `Fn`.
///
/// # Examples
/// A function that receives a `FnOnce` matching type and calls it:
/// ```
/// // Accepts any type that can be called once with (Option<i32>) and returns
/// // i32.
/// i32 do_stuff_once(sus::fn::FnOnce<i32(sus::Option<i32>)> auto f) {
///   return sus::fn::call_once(sus::move(f), sus::some(400));
/// }
///
/// i32 x = do_stuff_once([](sus::Option<i32> o) -> i32 {
///   return sus::move(o).unwrap_or_default() + 4;
/// });
///  sus_check(x == 400 + 4);
/// ```
///
/// A `FnOnce` whose first parameter is a class can be matched with a method
/// from that same class if the remaining parameters match the method's
/// signature:
/// ```
/// struct Class {
///   Class(i32 value) : value_(value) {}
///   i32 value() const { return value_; }
///
///  private:
///   i32 value_;
/// };
///
/// i32 map_class_once(const Class& c,
///                    sus::fn::FnOnce<i32(const Class&)> auto f) {
///   return sus::fn::call_once(sus::move(f), c);
/// }
///
/// // Map the class C to its value().
/// auto c = Class(42);
/// sus_check(map_class_once(c, &Class::value) == 42);
/// ```
///
/// Using a method pointer as the parameter for `Option::map()` will call that
/// method on the object inside the Option:
/// ```
/// struct Class {
///   Class(i32 value) : value_(value) {}
///   i32 value() const { return value_; }
///
///  private:
///   i32 value_;
/// };
///
/// auto o = sus::Option<Class>(Class(42));
/// sus_check(o.map(&Class::value) == sus::some(42));
/// ```
template <class F, class... S>
concept FnOnce = requires {
  // Receives and passes along the signature as a pack instead of a single
  // argument in order to consistently provide a static_assert() in `Sig` when
  // `S` is not a function signature.
  {
    __private::InvokedFnOnce<F, typename __private::Sig<S...>::Args>::returns()
  } -> __private::ValidReturnType<typename __private::Sig<S...>::Return>;
};

/// The version of a callable object that is allowed to mutate internal state
/// and may be called multiple times.
///
/// A `FnMut` is typically the best fit for any callable that may be called one
/// or more times.
/// Because a `FnMut` is able to mutate internal state, it may return different
/// values each time it is called with the same inputs.
///
/// A type that satisfies `FnMut` will return a type that can be converted to
/// `R` when called with the arguments `Args...`. `FnMut` is satisfied by
/// being callable as an lvalue (which is done by providing an operator() that
/// is not `&&`-qualified). Mutable and const lambdas will satisfy
/// `FnMut`.
///
/// The second argument of `FnMut<F, S>` is a function signature with the
/// format `ReturnType(Args...)`, where `Args...` are the arguments that will
/// be passed to the `FnMut` and `ReturnType` is what is expected to be
/// received back. It would appear as a matching concept as:
/// ```
/// void function(FnMut<ReturnType(Args...)> auto f) { ... }
/// ```
///
/// # Use of `FnMut`
/// `FnMut` should be received by value typically, though it can be received by
/// reference if mutations should be visible to the caller.
///
/// A `FnMut` should be called by passing it to [`call_mut`]($sus::fn::call_mut)
/// along with any arguments.
/// This ensures the correct overload is called on the object and
/// that method pointers are called correctly.
/// A `FnMut` may be called any number of times, unlike `FnOnce`,
/// and should not be moved when called.
///
/// # Type erasure
///
/// Using a concept like `FnMut` in a function parameter requires the function
/// to be a template. Template functions can not be virtual, they must appear
/// in the header, and they can have a negative impact on binary size. So it can
/// be desirable to work with a `FnMut` without templates.
///
/// To do so, `FnMut` supports being type-erased, on the heap or the stack,
/// into a [`DynFnMut`]($sus::fn::DynFnMut) type.
/// To receive ownership of a type-erased `FnMut`, receive a
/// [`Box`]($sus::boxed::Box)`<`[`DynFnMut<R(Args...)>`]($sus::fn::DynFnMut)`>`
/// instead.
/// To receive a reference to a type-erased `FnMut`, receive a
/// [`DynFnMut<R(Args...)>&&`]($sus::fn::DynFnMut) instead.
///
/// See [`DynConcept`]($sus::boxed::DynConcept) for more on type erasure of
/// concept-satisfying types.
///
/// # Compatibility
/// Any callable type that satisfies `Fn` or `FnMut` will also satisfy `FnOnce`.
/// While a `FnOnce` should only be called once, this is compatible with the
/// requirements of `FnMut` and `Fn` which can be called only a single time. As
/// well, `FnOnce` is allowed to mutate internal state, but it does not have to,
/// which is compatible with the const nature of `Fn`.
///
/// # Examples
/// A function that receives a `FnMut` matching type and calls it:
/// ```
/// // Accepts any type that can be called once with (Option<i32>) and returns
/// // i32.
/// static i32 do_stuff_mut(sus::fn::FnMut<i32(sus::Option<i32>)> auto f) {
///   return sus::fn::call_mut(f, sus::some(400)) +
///          sus::fn::call_mut(f, sus::some(100));
/// }
///
/// i32 x = do_stuff_mut([i = 0_i32](sus::Option<i32> o) mutable -> i32 {
///   i += 1;
///   return sus::move(o).unwrap_or_default() + i;
/// });
/// sus_check(x == 401 + 102);
/// ```
///
/// A `FnMut` whose first parameter is a class can be matched with a method from
/// that same class if the remaining parameters match the method's signature:
/// ```
/// struct Class {
///   Class(i32 value) : value_(value) {}
///   i32 value() const { return value_; }
///
///  private:
///   i32 value_;
/// };
///
/// i32 map_class_mut(const Class& c,
///                   sus::fn::FnMut<i32(const Class&)> auto f) {
///   return sus::fn::call_mut(f, c);
/// }
///
/// // Map the class C to its value().
/// auto c = Class(42);
/// sus_check(map_class_mut(c, &Class::value) == 42);
/// ```
///
/// Using a method pointer as the parameter for `Option::map()` will call that
/// method on the object inside the Option:
/// ```
/// struct Class {
///   Class(i32 value) : value_(value) {}
///   i32 value() const { return value_; }
///
///  private:
///   i32 value_;
/// };
///
/// auto o = sus::Option<Class>(Class(42));
/// sus_check(o.map(&Class::value) == sus::some(42));
/// ```
template <class F, class... S>
concept FnMut = requires {
  // Receives and passes along the signature as a pack instead of a single
  // argument in order to consistently provide a static_assert() in `Sig` when
  // `S` is not a function signature.
  {
    __private::InvokedFnMut<F, typename __private::Sig<S...>::Args>::returns()
  } -> __private::ValidReturnType<typename __private::Sig<S...>::Return>;

  requires FnOnce<F, S...>;
};

/// The version of a callable object that may be called multiple times without
/// mutating internal state.
///
/// A `Fn` is useful for a callable that is expected to be called one or more
/// times and whose results do not change between calls. This is of course
/// possible to violate with `mutable` or global state, but it is discouraged
/// as it violates the `Fn` protocol expectations of the caller.
/// [`FnMut`]($sus::fn::FnMut) should be used when the function will
/// mutate anything and can return different values as a result.
///
/// A type that satisfies `Fn` will return a type that can be converted to
/// `R` when called with the arguments `Args...`. `Fn` is satisfied by
/// being callable as a const lvalue (which is done by providing an operator()
/// that is `const`- or `const&`-qualified). Const lambdas will satisfy
/// `Fn` but mutable ones will not.
///
/// The second argument of `Fn<F, S>` is a function signature with the format
/// `ReturnType(Args...)`, where `Args...` are the arguments that will be passed
/// to the `Fn` and `ReturnType` is what is expected to be received back. It
/// would appear as a matching concept as:
/// ```
/// void function(Fn<ReturnType(Args...)> auto f) { ... }
/// ```
///
/// # Use of `Fn`
/// `Fn` should be received by value typically, but can also be received as a
/// const reference.
///
/// A `Fn` should be called by passing it to [`call`]($sus::fn::call) along
/// with any arguments.
/// This ensures the correct overload is called on the object and
/// that method pointers are called correctly.
/// A `Fn` may be called any number of times, unlike `FnOnce`, and should not
/// be moved when called.
///
/// # Type erasure
///
/// Using a concept like `Fn` in a function parameter requires the function
/// to be a template. Template functions can not be virtual, they must appear
/// in the header, and they can have a negative impact on binary size. So it can
/// be desirable to work with a `Fn` without templates.
///
/// To do so, `Fn` supports being type-erased, on the heap or the stack,
/// into a [`DynFn`]($sus::fn::DynFn) type.
/// To receive ownership of a type-erased `Fn`, receive a
/// [`Box`]($sus::boxed::Box)`<`[`DynFn<R(Args...)>`]($sus::fn::DynFn)`>`
/// instead.
/// To receive a reference to a type-erased `Fn`, receive a
/// [`DynFn<R(Args...)>&&`]($sus::fn::DynFn) instead.
///
/// See [`DynConcept`]($sus::boxed::DynConcept) for more on type erasure of
/// concept-satisfying types.
///
/// # Compatibility
/// Any callable type that satisfies `Fn` will also satisfy `FnMut` and
/// `FnOnce`. A `Fn` may be called multiple times, or a single time, which is
/// compatible with both `FnMut` and `FnOnce`. And while `FnMut` and `FnOnce`
/// are able to mutate state when run, they are not required to and a constant
/// `Fn` satisfies them.
///
/// # Examples
/// A function that receives a `Fn` matching type and calls it:
/// ```
/// // Accepts any type that can be called once with (Option<i32>) and returns
/// // i32.
/// static i32 do_stuff(sus::fn::Fn<i32(sus::Option<i32>)> auto f) {
///   return sus::fn::call(f, sus::some(400)) +
///          sus::fn::call(f, sus::some(100));
/// }
///
/// i32 x = do_stuff([i = 1_i32](sus::Option<i32> o) -> i32 {
///   return sus::move(o).unwrap_or_default() + i;
/// });
/// sus_check(x == 401 + 101);
/// ```
///
/// A `Fn` whose first parameter is a class can be matched with a method from
/// that same class if the remaining parameters match the method's signature:
/// ```
/// struct Class {
///   Class(i32 value) : value_(value) {}
///   i32 value() const { return value_; }
///
///  private:
///   i32 value_;
/// };
///
/// i32 map_class(const Class& c,
///               sus::fn::Fn<i32(const Class&)> auto const& f) {
///   return sus::fn::call(f, c);
/// }
///
/// // Map the class C to its value().
/// auto c = Class(42);
/// sus_check(map_class(c, &Class::value) == 42);
/// ```
///
/// Using a method pointer as the parameter for `Option::map()` will call that
/// method on the object inside the Option:
/// ```
/// struct Class {
///   Class(i32 value) : value_(value) {}
///   i32 value() const { return value_; }
///
///  private:
///   i32 value_;
/// };
///
/// auto o = sus::Option<Class>(Class(42));
/// sus_check(o.map(&Class::value) == sus::some(42));
/// ```
template <class F, class... S>
concept Fn = requires {
  // Receives and passes along the signature as a pack instead of a single
  // argument in order to consistently provide a static_assert() in `Sig` when
  // `S` is not a function signature.
  {
    __private::InvokedFn<F, typename __private::Sig<S...>::Args>::returns()
  } -> __private::ValidReturnType<typename __private::Sig<S...>::Return>;

  requires FnMut<F, S...>;
  requires FnOnce<F, S...>;
};

/// Invokes the [`FnOnce`]($sus::fn::FnOnce), passing any given arguments along,
/// and returning the result.
///
/// This function is like
/// [`std::invoke`](https://en.cppreference.com/w/cpp/utility/functional/invoke)
/// but it provides the following additional guiderails:
/// * Verifies that the thing being invoked is being moved from so that the
///   correct overload will be invoked.
template <class F, class... Args>
_sus_always_inline constexpr decltype(auto) call_once(F&& f, Args&&... args)
  requires(::sus::mem::IsMoveRef<decltype(f)>)
{
  return std::invoke(sus::move(f), sus::forward<Args>(args)...);
}

/// Invokes the [`FnMut`]($sus::fn::FnMut), passing any given arguments along,
/// and returning the result.
///
/// This function is like
/// [`std::invoke`](https://en.cppreference.com/w/cpp/utility/functional/invoke)
/// but it provides the following additional guiderails:
/// * Verifies that the thing being invoked is called as a mutable lvalue so
///   that the correct overload will be invoked.
template <class F, class... Args>
  requires(!std::is_const_v<std::remove_reference_t<F>>)
_sus_always_inline constexpr decltype(auto) call_mut(F&& f, Args&&... args) {
  return std::invoke(static_cast<std::remove_reference_t<F>&>(f),
                     sus::forward<Args>(args)...);
}

/// Invokes the [`Fn`]($sus::fn::Fn), passing any given arguments along,
/// and returning the result.
///
/// This function is like
/// [`std::invoke`](https://en.cppreference.com/w/cpp/utility/functional/invoke)
/// but it provides the following additional guiderails:
/// * Verifies that the thing being invoked is called as a const lvalue so
///   that the correct overload will be invoked.
template <class F, class... Args>
_sus_always_inline constexpr decltype(auto) call(F&& f, Args&&... args) {
  return std::invoke(static_cast<const std::remove_reference_t<F>&>(f),
                     sus::forward<Args>(args)...);
}

/// Resolves to the return type of a [`FnOnce`]($sus::fn::FnOnce) object when
/// passed `Args...`.
template <class F, class... Args>
  requires(FnOnce<F, Anything(Args...)>)
using ReturnOnce =
    decltype(call_once(std::declval<F&&>(), std::declval<Args>()...));
/// Resolves to the return type of a [`FnMut`]($sus::fn::FnMut) object when
/// passed `Args...`.
template <class F, class... Args>
  requires(FnMut<F, Anything(Args...)>)
using ReturnMut =
    decltype(call_mut(std::declval<F&&>(), std::declval<Args>()...));
/// Resolves to the return type of a [`Fn`]($sus::fn::Fn) object when
/// passed `Args...`.
template <class F, class... Args>
  requires(Fn<F, Anything(Args...)>)
using Return = decltype(call(std::declval<F&&>(), std::declval<Args>()...));

}  // namespace sus::fn
