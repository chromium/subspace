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

// TODO: Overload all && functions with const& version if T is copyable? The
// latter copies T instead of moving it. This could lead to a lot of unintended
// copies if expensive types have copy constructors, which is common in
// preexisting C++ code since there's no concept of Clone there (which will TBD
// in this library). So it's not clear if this is the right thing to do
// actually, needs thought.

// TODO: Clone integration: cloned().

#pragma once

#include <type_traits>

#include "assertions/check.h"
#include "assertions/unreachable.h"
#include "construct/make_default.h"
#include "macros/always_inline.h"
#include "macros/nonnull.h"
#include "marker/unsafe.h"
#include "mem/mref.h"
#include "mem/replace.h"
#include "mem/take.h"
#include "num/num_concepts.h"
#include "ops/eq.h"
#include "ops/ord.h"
#include "option/__private/is_option_type.h"
#include "option/__private/storage.h"
#include "option/state.h"
#include "result/__private/is_result_type.h"

namespace sus::iter {
template <class Item>
class Once;
template <class I>
class Iterator;
namespace __private {
template <class T>
constexpr auto begin(const T& t) noexcept;
template <class T>
constexpr auto end(const T& t) noexcept;
}  // namespace __private
}  // namespace sus::iter

namespace sus::result {
template <class T, class E>
class Result;
}

namespace sus::tuple {
template <class T, class... Ts>
class Tuple;
}

namespace sus::option {

using State::None;
using State::Some;
using sus::iter::Iterator;
using sus::iter::Once;
using sus::option::__private::Storage;
using sus::option::__private::StoragePointer;

/// A type which either holds #Some value of type `T`, or #None.
///
/// `Option<const T>` for non-reference-type `T` is disallowed, as the Option
/// owns the `T` in that case and it ensures the `Option` and the `T` are both
/// accessed with the same const-ness.
///
/// If a type provides a never-value field (see mem/never_value.h), then
/// Option<T> will have the same size as T.
///
/// However the never-value field places some limitations on what can be
/// constexpr in the Option type. Because it is not possible to query the state
/// of the Option in a constant evaluation context, state-querying methods can
/// not be constexpr, nor any method that branches based on the current state,
/// such as `unwrap_or()`.
///
template <class T>
class Option;

/// Implementation of Option for a value type (non-reference).
template <class T>
class Option final {
  static_assert(!std::is_const_v<T>);

 public:
  /// Construct an Option that is holding the given value.
  static inline constexpr Option some(const T& t) noexcept
    requires(std::is_copy_constructible_v<T>)
  {
    return Option(t);
  }

  /// Construct an Option that is holding the given value.
  template <class U>
  static inline constexpr Option some(Mref<U> t) noexcept
    requires(std::is_copy_constructible_v<T>)
  {
    return Option(t);
  }

  /// Construct an Option that is holding the given value.
  static inline constexpr Option some(T&& t) noexcept
    requires(std::is_move_constructible_v<T>)
  {
    return Option(static_cast<T&&>(t));
  }
  /// Construct an Option that is holding no value.
  static inline constexpr Option none() noexcept { return Option(); }

  /// Construct an Option with the default value for the type it contains.
  ///
  /// The Option's contained type `T` must be #MakeDefault, and will be
  /// constructed through that trait.
  static inline constexpr Option<T> with_default() noexcept
    requires(::sus::construct::MakeDefault<T>)
  {
    return Option<T>(::sus::construct::make_default<T>());
  }

  /// Destructor for the Option.
  ///
  /// If T can be trivially destroyed, we don't need to explicitly destroy it,
  /// so we can use the default destructor, which allows Option<T> to also be
  /// trivially destroyed.
  constexpr ~Option() noexcept
    requires(std::is_trivially_destructible_v<T>)
  = default;

  /// Destructor for the Option.
  ///
  /// Destroys the value contained within the option, if there is one.
  inline ~Option() noexcept
    requires(!std::is_trivially_destructible_v<T>)
  {
    if (t_.state() == Some) t_.val_.~T();
  }

  /// If T can be trivially copy-constructed, Option<T> can also be trivially
  /// copy-constructed.
  constexpr Option(const Option& o)
    requires(std::is_trivially_copy_constructible_v<T>)
  = default;

  Option(const Option& o) noexcept
    requires(!std::is_trivially_copy_constructible_v<T> &&
             std::is_copy_constructible_v<T>)
  {
    if (o.t_.state() == Some) t_.set_some(o.t_.val_);
  }

  constexpr Option(const Option& o)
    requires(!std::is_copy_constructible_v<T>)
  = delete;

  /// If T can be trivially copy-constructed, Option<T> can also be trivially
  /// move-constructed.
  constexpr Option(Option&& o)
    requires(std::is_trivially_move_constructible_v<T>)
  = default;

  // TODO: If this could be done in a `constexpr` way, methods that receive an
  // Option could also be constexpr.
  Option(Option&& o) noexcept
    requires(!std::is_trivially_move_constructible_v<T> &&
             std::is_move_constructible_v<T>)
  {
    if (o.t_.state() == Some) t_.set_some(o.t_.take_and_set_none());
  }

  constexpr Option(Option&& o)
    requires(!std::is_move_constructible_v<T>)
  = delete;

  /// If T can be trivially copy-assigned, Option<T> can also be trivially
  /// copy-assigned.
  constexpr Option& operator=(const Option& o)
    requires(std::is_trivially_copy_assignable_v<T>)
  = default;

  Option& operator=(const Option& o) noexcept
    requires(!std::is_trivially_copy_assignable_v<T> &&
             std::is_copy_assignable_v<T>)
  {
    if (o.t_.state() == Some)
      t_.set_some(o.t_.val_);
    else if (t_.state() == Some)
      t_.set_none();
    return *this;
  }

  constexpr Option& operator=(const Option& o)
    requires(!std::is_copy_assignable_v<T>)
  = delete;

  /// If T can be trivially move-assigned, we don't need to explicitly construct
  /// it, so we can use the default destructor, which allows Option<T> to also
  /// be trivially move-assigned.
  constexpr Option& operator=(Option&& o)
    requires(std::is_trivially_move_assignable_v<T>)
  = default;

  Option& operator=(Option&& o) noexcept
    requires(!std::is_trivially_move_assignable_v<T> &&
             std::is_move_assignable_v<T>)
  {
    if (o.t_.state() == Some)
      t_.set_some(o.t_.take_and_set_none());
    else if (t_.state() == Some)
      t_.set_none();
    return *this;
  }

  constexpr Option& operator=(Option&& o)
    requires(!std::is_move_assignable_v<T>)
  = delete;

  /// Drop the current value from the Option, if there is one.
  ///
  /// Afterward the option will unconditionally be #None.
  void clear() & noexcept {
    if (t_.state() == Some) t_.set_none();
  }

  /// Returns whether the Option currently contains a value.
  ///
  /// If there is a value present, it can be extracted with <unwrap>() or
  /// <expect>().
  bool is_some() const noexcept { return t_.state() == Some; }
  /// Returns whether the Option is currently empty, containing no value.
  bool is_none() const noexcept { return t_.state() == None; }

  /// An operator which returns the state of the Option, either #Some or #None.
  ///
  /// This supports the use of an Option in a `switch()`, allowing it to act as
  /// a tagged union between "some value" and "no value".
  ///
  /// # Example
  ///
  /// ```cpp
  /// auto x = Option<int>::some(2);
  /// switch (x) {
  ///  case Some:
  ///   return sus::move(x).unwrap_unchecked(unsafe_fn);
  ///  case None:
  ///   return -1;
  /// }
  /// ```
  operator State() const& { return t_.state(); }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr sus_nonnull_fn T expect(
      /* TODO: string view type */ sus_nonnull_arg const char*
          msg) && noexcept {
    if (!std::is_constant_evaluated())
      ::sus::check_with_message(t_.state() == Some, *msg);
    return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr sus_nonnull_fn const T& expect_ref(
      /* TODO: string view type */ sus_nonnull_arg const char* msg)
      const& noexcept {
    if (!std::is_constant_evaluated())
      ::sus::check_with_message(t_.state() == Some, *msg);
    return t_.val_;
  }
  const T& expect_ref() && noexcept = delete;

  /// Returns a mutable reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_mut().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr sus_nonnull_fn T& expect_mut(
      /* TODO: string view type */ sus_nonnull_arg const char*
          msg) & noexcept {
    if (!std::is_constant_evaluated())
      ::sus::check_with_message(t_.state() == Some, *msg);
    return t_.val_;
  }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr T unwrap() && noexcept {
    if (!std::is_constant_evaluated()) ::sus::check(t_.state() == Some);
    return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
  }

  /// Returns the contained value inside the Option.
  ///
  /// # Safety
  ///
  /// It is Undefined Behaviour to call this function when the Option's state is
  /// `None`. The caller is responsible for ensuring the Option contains a value
  /// beforehand, and the safer <unwrap>() or <expect>() should almost always be
  /// preferred.
  constexpr inline T unwrap_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    return t_.take_and_set_none();
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().unwrap()`.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr const T& unwrap_ref() const& noexcept {
    if (!std::is_constant_evaluated()) ::sus::check(t_.state() == Some);
    return t_.val_;
  }
  const T& unwrap_ref() && noexcept = delete;

  /// Returns a mutable reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_mut().unwrap()`.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr T& unwrap_mut() & noexcept {
    if (!std::is_constant_evaluated()) ::sus::check(t_.state() == Some);
    return t_.val_;
  }

  /// Returns the contained value inside the Option, if there is one. Otherwise,
  /// returns `default_result`.
  ///
  /// Note that if it is non-trivial to construct a `default_result`, that
  /// <unwrap_or_else>() should be used instead, as it will only construct the
  /// default value if required.
  T unwrap_or(T default_result) && noexcept {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      return default_result;
    }
  }

  /// Returns the contained value inside the Option, if there is one.
  /// Otherwise, returns the result of the given function.
  template <class Functor>
    requires(std::is_same_v<std::invoke_result_t<Functor>, T>)
  T unwrap_or_else(Functor f) && noexcept {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      return f();
    }
  }

  /// Returns the contained value inside the Option, if there is one.
  /// Otherwise, constructs a default value for the type and returns that.
  ///
  /// The Option's contained type `T` must be #MakeDefault, and will be
  /// constructed through that trait.
  T unwrap_or_default() && noexcept
    requires(::sus::construct::MakeDefault<T>)
  {
    if (t_.state() == Some) {
      return t_.take_and_set_none();
    } else {
      return ::sus::construct::make_default<T>();
    }
  }

  /// Stores the value `t` inside this Option, replacing any previous value, and
  /// returns a mutable reference to the new value.
  constexpr T& insert(T t) & noexcept {
    t_.set_some(static_cast<T&&>(t));
    return t_.val_;
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// stores `t` inside the Option and returns a mutable reference to the new
  /// value.
  ///
  /// If it is non-trivial to construct `T`, the <get_or_insert_with>() method
  /// would be preferable, as it only constructs a `T` if needed.
  T& get_or_insert(T t) & noexcept {
    if (t_.state() == None) t_.construct_from_none(static_cast<T&&>(t));
    return t_.val_;
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// constructs a default value `T`, stores it inside the Option and returns a
  /// mutable reference to the new value.
  ///
  /// This method differs from <unwrap_or_default>() in that it does not consume
  /// the Option, and instead it can not be called on rvalues.
  ///
  /// This is a shorthand for
  /// `Option<T>::get_or_insert_default(MakeDefault<T>::make_default)`.
  ///
  /// The Option's contained type `T` must be #MakeDefault, and will be
  /// constructed through that trait.
  T& get_or_insert_default() & noexcept
    requires(::sus::construct::MakeDefault<T>)
  {
    if (t_.state() == None)
      t_.construct_from_none(::sus::construct::make_default<T>());
    return t_.val_;
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// constructs a `T` by calling `f`, stores it inside the Option and returns a
  /// mutable reference to the new value.
  ///
  /// This method differs from <unwrap_or_else>() in that it does not consume
  /// the Option, and instead it can not be called on rvalues.
  template <class WithFn>
    requires(std::is_same_v<std::invoke_result_t<WithFn>, T>)
  T& get_or_insert_with(WithFn f) & noexcept {
    if (t_.state() == None) t_.construct_from_none(f());
    return t_.val_;
  }

  /// Returns a new Option containing whatever was inside the current Option.
  ///
  /// If this Option contains #None then it is left unchanged and returns an
  /// Option containing #None. If this Option contains #Some with a value, the
  /// value is moved into the returned Option and this Option will contain #None
  /// afterward.
  Option take() & noexcept {
    if (t_.state() == Some)
      return Option(t_.take_and_set_none());
    else
      return Option::none();
  }

  /// Maps the Option's value through a function.
  ///
  /// Consumes the Option, passing the value through the map function, and
  /// returning an `Option<R>` where `R` is the return type of the map function.
  ///
  /// Returns an `Option<R>` in state #None if the current Option is in state
  /// #None.
  template <class MapFn, int&..., class R = std::invoke_result_t<MapFn, T&&>>
    requires(!std::is_void_v<R>)
  Option<R> map(MapFn m) && noexcept {
    if (t_.state() == Some) {
      return Option<R>(m(t_.take_and_set_none()));
    } else {
      return Option<R>::none();
    }
  }

  /// Maps the Option's value through a function, or returns a default value.
  ///
  /// Consumes the Option, passing the value through the map function, and
  /// returning an `Option<R>` where `R` is the return type of the map function.
  ///
  /// Returns an `Option<R>` with the `default_result` as its value if the
  /// current Option's state is #None.
  template <class MapFn, class D, int&...,
            class R = std::invoke_result_t<MapFn, T&&>>
    requires(!std::is_void_v<R> && std::is_same_v<D, R>)
  Option<R> map_or(D default_result, MapFn m) && noexcept {
    if (t_.state() == Some) {
      return Option<R>(m(t_.take_and_set_none()));
    } else {
      return Option<R>(static_cast<R&&>(default_result));
    }
  }

  /// Maps the Option's value through a function, or returns a default value
  /// constructed from the default function.
  ///
  /// Consumes the Option, passing the value through the map function, and
  /// returning an `Option<R>` where `R` is the return type of the map function.
  ///
  /// Returns an `Option<R>` with the result of calling `default_fn` as its
  /// value if the current Option's state is #None.
  template <class DefaultFn, class MapFn, int&...,
            class D = std::invoke_result_t<DefaultFn>,
            class R = std::invoke_result_t<MapFn, T&&>>
    requires(!std::is_void_v<R> && std::is_same_v<D, R>)
  Option<R> map_or_else(DefaultFn default_fn, MapFn m) && noexcept {
    if (t_.state() == Some) {
      return Option<R>(m(t_.take_and_set_none()));
    } else {
      return Option<R>(default_fn());
    }
  }

  /// Consumes the Option and applies a predicate function to the value
  /// contained in the Option. Returns a new Option with the same value if the
  /// predicate returns true, otherwise returns an Option with its state set to
  /// #None.
  ///
  /// The predicate function must take `const T&` and return `bool`.
  template <class Predicate>
    requires(std::is_same_v<std::invoke_result_t<Predicate, const T&>, bool>)
  Option<T> filter(Predicate p) && noexcept {
    if (t_.state() == Some) {
      if (p(const_cast<const T&>(t_.val_))) {
        return Option(t_.take_and_set_none());
      } else {
        // The state has to become None, and we must destroy the inner T.
        t_.set_none();
        return Option::none();
      }
    } else {
      return Option::none();
    }
  }

  /// Consumes this Option and returns an Option with #None if this Option holds
  /// #None, otherwise returns the given `opt`.
  template <class U>
  Option<U> and_opt(Option<U> opt) && noexcept {
    if (t_.state() == Some) {
      t_.set_none();
      return opt;
    } else {
      return Option<U>::none();
    }
  }

  /// Consumes this Option and returns an Option with #None if this Option holds
  /// #None, otherwise calls `f` with the contained value and returns an Option
  /// with the result.
  ///
  /// Some languages call this operation flatmap.
  template <
      class AndFn, int&..., class R = std::invoke_result_t<AndFn, T&&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  Option<InnerR> and_then(AndFn f) && noexcept {
    if (t_.state() == Some)
      return f(t_.take_and_set_none());
    else
      return Option<InnerR>::none();
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the given `opt`.
  Option<T> or_opt(Option<T> opt) && noexcept {
    if (t_.state() == Some)
      return Option(t_.take_and_set_none());
    else
      return opt;
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the Option returned by `f`.
  template <class ElseFn, int&..., class R = std::invoke_result_t<ElseFn>>
    requires(std::is_same_v<R, Option<T>>)
  Option<T> or_else(ElseFn f) && noexcept {
    if (t_.state() == Some)
      return Option(t_.take_and_set_none());
    else
      return static_cast<ElseFn&&>(f)();
  }

  /// Consumes this Option and returns an Option, holding the value from either
  /// this Option `opt`, if exactly one of them holds a value, otherwise returns
  /// an Option that holds #None.
  Option<T> xor_opt(Option<T> opt) && noexcept {
    if (t_.state() == Some) {
      // If `this` holds Some, we change `this` to hold None. If `opt` is None,
      // we return what this was holding, otherwise we return None.
      if (opt.t_.state() == None) {
        return Option(t_.take_and_set_none());
      } else {
        t_.set_none();
        return Option::none();
      }
    } else {
      // If `this` holds None, we need to do nothing to `this`. If `opt` is Some
      // we would return its value, and if `opt` is None we should return None.
      return opt;
    }
  }

  /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some(v)` to
  /// `Ok(v)` and `None` to `Err(e)`.
  ///
  /// Arguments passed to #ok_or are eagerly evaluated; if you are passing the
  /// result of a function call, it is recommended to use ok_or_else, which is
  /// lazily evaluated.
  template <class E, int&..., class Result = ::sus::result::Result<T, E>>
  inline Result ok_or(E e) && noexcept {
    if (t_.state() == Some)
      return Result::with(t_.take_and_set_none());
    else
      return Result::with_err(static_cast<E&&>(e));
  }

  /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some(v)` to
  /// `Ok(v)` and `None` to `Err(f())`.
  template <class ElseFn, int&..., class E = std::invoke_result_t<ElseFn>,
            class Result = ::sus::result::Result<T, E>>
  inline Result ok_or_else(ElseFn f) && noexcept {
    if (t_.state() == Some)
      return Result::with(t_.take_and_set_none());
    else
      return Result::with_err(static_cast<ElseFn&&>(f)());
  }

  /// Zips self with another Option.
  ///
  /// If self is `Some(s)` and other is `Some(o)`, this method returns `Some((s,
  /// o))`. Otherwise, `None` is returned.
  template <class U, int&..., class Tuple = ::sus::tuple::Tuple<T, U>>
  inline Option<Tuple> zip(Option<U> o) && noexcept {
    if (o.t_.state() == None) {
      if (t_.state() == Some) t_.set_none();
      return Option<Tuple>::none();
    } else if (t_.state() == None) {
      return Option<Tuple>::none();
    } else {
      return Option<Tuple>::some(Tuple::with(
          t_.take_and_set_none(), static_cast<Option<U>&&>(o).unwrap()));
    }
  }

  /// Transposes an #Option of a #Result into a #Result of an #Option.
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
  inline Result transpose() && noexcept {
    if (t_.state() == None) {
      return Result::with(Option<OkType>::none());
    } else {
      if (t_.val_.is_ok()) {
        return Result::with(Option<OkType>::some(
            t_.take_and_set_none().unwrap_unchecked(unsafe_fn)));
      } else {
        return Result::with_err(
            t_.take_and_set_none().unwrap_err_unchecked(unsafe_fn));
      }
    }
  }

  /// Replaces whatever the Option is currently holding with #Some value `t` and
  /// returns an Option holding what was there previously.
  Option replace(T t) & noexcept {
    if (t_.state() == None) {
      t_.construct_from_none(static_cast<T&&>(t));
      return Option::none();
    } else {
      return Option(t_.replace_some(static_cast<T&&>(t)));
    }
  }

  /// Maps an `Option<Option<T>>` to an `Option<T>`.
  T flatten() && noexcept
    requires(::sus::option::__private::IsOptionType<T>::value)
  {
    if (t_.state() == Some)
      return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
    else
      return T::none();
  }

  /// Returns an Option<const T&> from this Option<T>, that either holds #None
  /// or a reference to the value in this Option.
  Option<const T&> as_ref() const& noexcept {
    if (t_.state() == None)
      return Option<const T&>::none();
    else
      return Option<const T&>(t_.val_);
  }
  Option<const T&> as_ref() && noexcept = delete;

  /// Returns an Option<T&> from this Option<T>, that either holds #None or a
  /// reference to the value in this Option.
  Option<T&> as_mut() & noexcept {
    if (t_.state() == None)
      return Option<T&>::none();
    else
      return Option<T&>(t_.val_);
  }

  constexpr Iterator<Once<const T&>> iter() const& noexcept {
    return Iterator<Once<const T&>>(as_ref());
  }
  Iterator<Once<const T&>> iter() const&& = delete;

  constexpr Iterator<Once<T&>> iter_mut() & noexcept {
    return Iterator<Once<T&>>(as_mut());
  }

  constexpr Iterator<Once<T>> into_iter() && noexcept {
    return Iterator<Once<T>>(take());
  }

 private:
  template <class U>
  friend class Option;

  /// Constructor for #None.
  constexpr explicit Option() = default;
  /// Constructor for #Some.
  constexpr explicit Option(const T& t) : t_(t) {}
  constexpr explicit Option(T&& t) : t_(static_cast<T&&>(t)) {}

  Storage<T> t_;

  sus_class_maybe_trivial_relocatable_types(unsafe_fn, T);
};

/// Implementation of Option for a reference type.
template <class T>
class Option<T&> final {
 public:
  /// Construct an Option that is holding the given value.
  static inline constexpr Option some(T& t) noexcept
    requires(std::is_const_v<T>)  // Require mref() for mutable references.
  {
    return Option(t);
  }

  /// Construct an Option that is holding the given value.
  static inline constexpr Option some(
      Mref<std::remove_const_t<T>> t) noexcept {
    return Option(t);
  }

  /// Construct an Option that is holding no value.
  static inline constexpr Option none() noexcept { return Option(); }

  /// Destructor for the Option.
  ///
  /// This is a no-op for references.
  constexpr ~Option() noexcept = default;

  // References can be trivially copied and moved, so we use the default
  // constructors and operators.
  constexpr Option(const Option& o) = default;
  constexpr Option(Option&& o) = default;
  constexpr Option& operator=(const Option& o) = default;
  constexpr Option& operator=(Option&& o) = default;

  /// Drop the current value from the Option, if there is one.
  ///
  /// Afterward the option will unconditionally be #None.
  void clear() & noexcept {
    if (t_.state() == Some) t_.set_none();
  }

  /// Returns whether the Option currently contains a value.
  ///
  /// If there is a value present, it can be extracted with <unwrap>() or
  /// <expect>().
  bool is_some() const noexcept { return t_.state() == Some; }
  /// Returns whether the Option is currently empty, containing no value.
  bool is_none() const noexcept { return t_.state() == None; }

  /// An operator which returns the state of the Option, either #Some or #None.
  ///
  /// This supports the use of an Option in a `switch()`, allowing it to act as
  /// a tagged union between "some value" and "no value".
  ///
  /// # Example
  ///
  /// ```cpp
  /// auto x = Option<int>::some(2);
  /// switch (x) {
  ///  case Some:
  ///   return sus::move(x).unwrap_unchecked(unsafe_fn);
  ///  case None:
  ///   return -1;
  /// }
  /// ```
  operator State() const& { return t_.state(); }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr sus_nonnull_fn T& expect(
      /* TODO: string view type */ sus_nonnull_arg const char*
          msg) && noexcept {
    if (!std::is_constant_evaluated())
      ::sus::check_with_message(t_.state() == Some, *msg);
    return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr sus_nonnull_fn const T& expect_ref(
      /* TODO: string view type */ sus_nonnull_arg const char* msg)
      const& noexcept {
    if (!std::is_constant_evaluated())
      ::sus::check_with_message(t_.state() == Some, *msg);
    return t_.val_.as_ref();
  }

  /// Returns a mutable reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_mut().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr sus_nonnull_fn T& expect_mut(
      /* TODO: string view type */ sus_nonnull_arg const char*
          msg) noexcept
    requires(!std::is_const_v<T>)
  {
    if (!std::is_constant_evaluated())
      ::sus::check_with_message(t_.state() == Some, *msg);
    return t_.val_.as_mut();
  }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr T& unwrap() && noexcept {
    if (!std::is_constant_evaluated()) ::sus::check(t_.state() == Some);
    return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
  }

  /// Returns the contained value inside the Option.
  ///
  /// # Safety
  ///
  /// It is Undefined Behaviour to call this function when the Option's state is
  /// `None`. The caller is responsible for ensuring the Option contains a value
  /// beforehand, and the safer <unwrap>() or <expect>() should almost always be
  /// preferred.
  constexpr inline T& unwrap_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept {
    return get_ref(t_.take_and_set_none());
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().unwrap()`.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr const T& unwrap_ref() const& noexcept {
    if (!std::is_constant_evaluated()) ::sus::check(t_.state() == Some);
    return t_.val_.as_ref();
  }

  /// Returns a mutable reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_mut().unwrap()`.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr T& unwrap_mut() noexcept
    requires(!std::is_const_v<T>)
  {
    if (!std::is_constant_evaluated()) ::sus::check(t_.state() == Some);
    return t_.val_.as_mut();
  }

  /// Returns the contained value inside the Option, if there is one. Otherwise,
  /// returns `default_result`.
  ///
  /// Note that if it is non-trivial to construct a `default_result`, that
  /// <unwrap_or_else>() should be used instead, as it will only construct the
  /// default value if required.
  ///
  /// Not constexpr because it's not possible
  T& unwrap_or(T& default_result) && noexcept {
    if (t_.state() == Some) {
      return get_ref(t_.take_and_set_none());
    } else
      return default_result;
  }

  /// Returns the contained value inside the Option, if there is one.
  /// Otherwise, returns the result of the given function.
  template <class Functor>
    requires(std::is_same_v<std::invoke_result_t<Functor>, T&>)
  T& unwrap_or_else(Functor f) && noexcept {
    if (t_.state() == Some)
      return get_ref(t_.take_and_set_none());
    else
      return f();
  }

  /// Stores the value `t` inside this Option, replacing any previous value, and
  /// returns a mutable reference to the new value.
  T& insert(T& t) noexcept {
    t_.set_some(StoragePointer(t));
    return t_.val_.as_mut();
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// stores `t` inside the Option and returns a mutable reference to the new
  /// value.
  T& get_or_insert(T& t) noexcept {
    if (t_.state() == None) t_.construct_from_none(StoragePointer(t));
    return t_.val_.as_mut();
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// constructs a `T` by calling `f`, stores it inside the Option and returns a
  /// mutable reference to the new value.
  ///
  /// This method differs from <unwrap_or_else>() in that it does not consume
  /// the Option, and instead it can not be called on rvalues.
  template <class WithFn>
    requires(std::is_same_v<std::invoke_result_t<WithFn>, T&>)
  T& get_or_insert_with(WithFn f) noexcept {
    if (t_.state() == None) t_.construct_from_none(StoragePointer(f()));
    return t_.val_.as_mut();
  }

  /// Returns a new Option containing whatever was inside the current Option.
  ///
  /// If this Option contains #None then it is left unchanged and returns an
  /// Option containing #None. If this Option contains #Some with a value, the
  /// value is moved into the returned Option and this Option will contain #None
  /// afterward.
  Option take() noexcept {
    if (t_.state() == Some)
      return Option(get_ref(t_.take_and_set_none()));
    else
      return Option::none();
  }

  /// Maps the Option's value through a function.
  ///
  /// Consumes the Option, passing the value through the map function, and
  /// returning an `Option<R>` where `R` is the return type of the map function.
  ///
  /// Returns an `Option<R>` in state #None if the current Option is in state
  /// #None.
  template <class MapFn, int&..., class R = std::invoke_result_t<MapFn, T&>>
    requires(!std::is_void_v<R>)
  Option<R> map(MapFn m) && noexcept {
    if (t_.state() == Some)
      return Option<R>::some(m(get_ref(t_.take_and_set_none())));
    else
      return Option<R>::none();
  }

  /// Maps the Option's value through a function, or returns a default value.
  ///
  /// Consumes the Option, passing the value through the map function, and
  /// returning an `Option<R>` where `R` is the return type of the map function.
  ///
  /// Returns an `Option<R>` with the `default_result` as its value if the
  /// current Option's state is #None.
  template <class MapFn, class D, int&...,
            class R = std::invoke_result_t<MapFn, T&>>
    requires(!std::is_void_v<R> && std::is_same_v<D, R>)
  Option<R> map_or(D default_result, MapFn m) && noexcept {
    if (t_.state() == Some)
      return Option<R>(m(get_ref(t_.take_and_set_none())));
    else
      return Option<R>(static_cast<R&&>(default_result));
  }

  /// Maps the Option's value through a function, or returns a default value
  /// constructed from the default function.
  ///
  /// Consumes the Option, passing the value through the map function, and
  /// returning an `Option<R>` where `R` is the return type of the map function.
  ///
  /// Returns an `Option<R>` with the result of calling `default_fn` as its
  /// value if the current Option's state is #None.
  template <class DefaultFn, class MapFn, int&...,
            class D = std::invoke_result_t<DefaultFn>,
            class R = std::invoke_result_t<MapFn, T&>>
    requires(!std::is_void_v<R> && std::is_same_v<D, R>)
  Option<R> map_or_else(DefaultFn default_fn, MapFn m) && noexcept {
    if (t_.state() == Some)
      return Option<R>(m(get_ref(t_.take_and_set_none())));
    else
      return Option<R>(default_fn());
  }

  /// Consumes the Option and applies a predicate function to the value
  /// contained in the Option. Returns a new Option with the same value if the
  /// predicate returns true, otherwise returns an Option with its state set to
  /// #None.
  ///
  /// The predicate function must take `const T&` and return `bool`.
  template <class Predicate>
    requires(std::is_same_v<std::invoke_result_t<Predicate, const T&>, bool>)
  Option<T&> filter(Predicate p) && noexcept {
    if (t_.state() == Some) {
      // The state must move to None.
      StoragePointer ptr = t_.take_and_set_none();
      if (p(const_cast<const T&>(ptr.as_ref())))
        return Option(get_ref(static_cast<decltype(ptr)&&>(ptr)));
      else
        return Option::none();
    } else {
      return Option::none();
    }
  }

  /// Consumes this Option and returns an Option with #None if this Option holds
  /// #None, otherwise returns the given `opt`.
  template <class U>
  Option<U> and_opt(Option<U> opt) && noexcept {
    if (t_.state() == Some) {
      t_.set_none();
      return opt;
    } else {
      return Option<U>::none();
    }
  }

  /// Consumes this Option and returns an Option with #None if this Option holds
  /// #None, otherwise calls `f` with the contained value and returns an Option
  /// with the result.
  ///
  /// Some languages call this operation flatmap.
  template <
      class AndFn, int&..., class R = std::invoke_result_t<AndFn, T&>,
      class InnerR = ::sus::option::__private::IsOptionType<R>::inner_type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  Option<InnerR> and_then(AndFn f) && noexcept {
    if (t_.state() == Some)
      return f(get_ref(t_.take_and_set_none()));
    else
      return Option<InnerR>::none();
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the given `opt`.
  Option<T&> or_opt(Option<T&> opt) && noexcept {
    if (t_.state() == Some)
      return Option(get_ref(t_.take_and_set_none()));
    else
      return opt;
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the Option returned by `f`.
  template <class ElseFn, int&..., class R = std::invoke_result_t<ElseFn>>
    requires(std::is_same_v<R, Option<T&>>)
  Option<T&> or_else(ElseFn f) && noexcept {
    if (t_.state() == Some)
      return Option(get_ref(t_.take_and_set_none()));
    else
      return f();
  }

  /// Consumes this Option and returns an Option, holding the value from either
  /// this Option `opt`, if exactly one of them holds a value, otherwise returns
  /// an Option that holds #None.
  Option<T&> xor_opt(Option<T&> opt) && noexcept {
    if (t_.state() == Some) {
      // If `this` holds Some, we change `this` to hold None. If `opt` is None,
      // we return what this was holding, otherwise we return None.
      auto nonnull = t_.take_and_set_none();
      if (opt.t_.state() == None)
        return Option(nonnull.as_mut());
      else
        return Option::none();
    } else {
      // If `this` holds None, we need to do nothing to `this`. If `opt` is Some
      // we would return its value, and if `opt` is None we should return None.
      return opt;
    }
  }

  /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some(v)` to
  /// `Ok(v)` and `None` to `Err(e)`.
  ///
  /// Arguments passed to #ok_or are eagerly evaluated; if you are passing the
  /// result of a function call, it is recommended to use ok_or_else, which is
  /// lazily evaluated.
  template <class E, int&..., class Result = ::sus::result::Result<T&, E>>
  inline Result ok_or(E e) && noexcept {
    if (t_.state() == Some)
      return Result::with(get_ref(t_.take_and_set_none()));
    else
      return Result::with_err(static_cast<E&&>(e));
  }

  /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some(v)` to
  /// `Ok(v)` and `None` to `Err(f())`.
  template <class ElseFn, int&..., class E = std::invoke_result_t<ElseFn>,
            class Result = ::sus::result::Result<T&, E>>
  constexpr inline Result ok_or_else(ElseFn f) && noexcept {
    if (t_.set_state(None) == Some)
      return Result::with(get_ref(t_.take_and_set_none()));
    else
      return Result::with_err(static_cast<ElseFn&&>(f)());
  }

  /// Zips self with another Option.
  ///
  /// If self is `Some(s)` and other is `Some(o)`, this method returns `Some((s,
  /// o))`. Otherwise, `None` is returned.
  template <class U, int&..., class Tuple = ::sus::tuple::Tuple<T&, U>>
  inline Option<Tuple> zip(Option<U> o) && noexcept {
    if (o.t_.state() == None) {
      t_.set_none();
      return Option<Tuple>::none();
    } else if (t_.state() == None) {
      return Option<Tuple>::none();
    } else {
      return Option<Tuple>::some(
          Tuple::with(get_ref(t_.take_and_set_none()),
                      static_cast<Option<U>&&>(o).unwrap()));
    }
  }

  /// Replaces whatever the Option is currently holding with #Some value `t` and
  /// returns an Option holding what was there previously.
  Option replace(T& t) noexcept {
    if (t_.state() == None) {
      t_.construct_from_none(StoragePointer(t));
      return Option::none();
    } else {
      return Option(
          ::sus::mem::replace(mref(t_.val_), StoragePointer(t)).as_mut());
    }
  }

  /// Maps an `Option<T&>` to an `Option<T>` by copying the referenced `T`.
  Option<std::remove_const_t<T>> copied() && noexcept
    requires(std::is_nothrow_copy_constructible_v<T>)
  {
    if (t_.state() == None)
      return Option<std::remove_const_t<T>>::none();
    else
      return Option<std::remove_const_t<T>>::some(t_.val_.as_ref());
  }

  /// Maps an `Option<Option<T>>` to an `Option<T>`.
  T flatten() && noexcept
    requires(::sus::option::__private::IsOptionType<T>::value)
  {
    if (t_.state() == Some)
      return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
    else
      return T::none();
  }

  /// Returns an Option<const T&> from this Option<T>, that either holds #None
  /// or a reference to the value in this Option.
  Option<const T&> as_ref() const& noexcept {
    if (t_.state() == None)
      return Option<const T&>::none();
    else
      return Option<const T&>(t_.val_.as_ref());
  }

  /// Returns an Option<T&> from this Option<T>, that either holds #None or a
  /// reference to the value in this Option.
  Option<T&> as_mut() noexcept {
    if (t_.state() == None)
      return Option<T&>::none();
    else
      return Option<T&>(t_.val_.as_mut());
  }

  Iterator<Once<const T&>> iter() const& noexcept {
    return Iterator<Once<const T&>>(as_ref());
  }

  Iterator<Once<T&>> iter_mut() noexcept
    requires(!std::is_const_v<T>)
  {
    return Iterator<Once<T&>>(as_mut());
  }

  Iterator<Once<T&>> into_iter() && noexcept {
    return Iterator<Once<T&>>(take());
  }

 private:
  template <class U>
  friend class Option;

  /// Constructor for #None.
  constexpr explicit Option() = default;
  /// Constructor for #Some.
  constexpr explicit Option(T& t) : t_(StoragePointer(t)) {}

  /// Gets a reference with the same const-ness as the `T` in `Option<T&>`.
  constexpr sus_always_inline static T& get_ref(
      StoragePointer<T>&& ptr) noexcept {
    if constexpr (std::is_const_v<T>)
      return ptr.as_ref();
    else
      return ptr.as_mut();
  }

  Storage<StoragePointer<T>> t_;

  sus_class_maybe_trivial_relocatable_types(unsafe_fn, T&);
};

/// sus::ops::Eq<Option<U>> trait.
template <class T, class U>
  requires(::sus::ops::Eq<T, U>)
constexpr inline bool operator==(const Option<T>& l,
                                 const Option<U>& r) noexcept {
  switch (l) {
    case Some: return r.is_some() && l.unwrap_ref() == r.unwrap_ref();
    case None: return r.is_none();
  }
  ::sus::unreachable_unchecked(unsafe_fn);
}

/// sus::ops::Ord<Option<U>> trait.
template <class T, class U>
  requires(::sus::ops::ExclusiveOrd<T, U>)
constexpr inline auto operator<=>(const Option<T>& l,
                                  const Option<U>& r) noexcept {
  switch (l) {
    case Some:
      if (r.is_some())
        return l.unwrap_ref() <=> r.unwrap_ref();
      else
        return std::strong_ordering::greater;
    case None:
      if (r.is_some())
        return std::strong_ordering::less;
      else
        return std::strong_ordering::equivalent;
  }
  ::sus::unreachable_unchecked(unsafe_fn);
}

/// sus::ops::WeakOrd<Option<U>> trait.
template <class T, class U>
  requires(::sus::ops::ExclusiveWeakOrd<T, U>)
constexpr inline auto operator<=>(const Option<T>& l,
                                  const Option<U>& r) noexcept {
  switch (l) {
    case Some:
      if (r.is_some())
        return l.unwrap_ref() <=> r.unwrap_ref();
      else
        return std::weak_ordering::greater;
    case None:
      if (r.is_some())
        return std::weak_ordering::less;
      else
        return std::weak_ordering::equivalent;
  }
  ::sus::unreachable_unchecked(unsafe_fn);
}

/// sus::ops::PartialOrd<Option<U>> trait.
template <class T, class U>
  requires(::sus::ops::ExclusivePartialOrd<T, U>)
constexpr inline auto operator<=>(const Option<T>& l,
                                  const Option<U>& r) noexcept {
  switch (l) {
    case Some:
      if (r.is_some())
        return l.unwrap_ref() <=> r.unwrap_ref();
      else
        return std::partial_ordering::greater;
    case None:
      if (r.is_some())
        return std::partial_ordering::less;
      else
        return std::partial_ordering::equivalent;
  }
  ::sus::unreachable_unchecked(unsafe_fn);
}

// Implicit for-ranged loop iteration via `Array::iter()`.
using sus::iter::__private::begin;
using sus::iter::__private::end;

}  // namespace sus::option

// Promote Option and its enum values into the `sus` namespace.
namespace sus {
using ::sus::option::None;
using ::sus::option::Option;
using ::sus::option::Some;
}  // namespace sus
