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

// TODO: Pair/tuple integration: zip().

#pragma once

#include <type_traits>

#include "assertions/check.h"
#include "assertions/nonnull.h"
#include "assertions/unreachable.h"
#include "construct/make_default.h"
#include "marker/unsafe.h"
#include "mem/mref.h"
#include "mem/replace.h"
#include "mem/take.h"
#include "num/num_concepts.h"
#include "option/__private/is_option_type.h"
#include "result/__private/is_result_type.h"

namespace sus::iter {
template <class Item>
class Once;
template <class I>
class Iterator;
}  // namespace sus::iter

namespace sus::result {
template <class T, class E>
class Result;
}

namespace sus::option {

using sus::iter::Iterator;
using sus::iter::Once;

template <class T>
class Option;

/// The representation of an Option's state, which can either be #None to
/// represent it has no value, or #Some for when it is holding a value.
enum class State : bool {
  /// The Option is not holding any value.
  None,
  /// The Option is holding a value.
  Some,
};
using State::None;
using State::Some;

namespace __private {

// TODO: Determine if we can put the State into the storage of `T`. Probably
// though a user-defined trait for `T`?
//
// TODO: If the compiler provided an extension to get the offset of a reference
// or non-null-annotated pointer inside a type, we could use that to determine a
// place to "store" the liveness bit inside `T`. When we destroy `T`, we'd write
// a `null` to that location, and when `T` is constructed, we know it will write
// a non-`null` there. This is a generalization of what we have done for the
// `T&` type. Something like `__offset_of_nonnull_field(T)`, which would be
// possible to determine at compile time for a fully-defined type `T`.
template <class T>
struct Storage final {
  constexpr ~Storage()
    requires(std::is_trivially_destructible_v<T>)
  = default;
  constexpr ~Storage()
    requires(!std::is_trivially_destructible_v<T>)
  {}

  constexpr Storage(const Storage&)
    requires(std::is_trivially_copy_constructible_v<T>)
  = default;
  constexpr Storage& operator=(const Storage&)
    requires(std::is_trivially_copy_assignable_v<T>)
  = default;
  constexpr Storage(Storage&&)
    requires(std::is_trivially_move_constructible_v<T>)
  = default;
  constexpr Storage& operator=(Storage&&)
    requires(std::is_trivially_move_assignable_v<T>)
  = default;

  constexpr Storage(State s) : state_(s) {}
  constexpr Storage(const std::remove_cvref_t<T>& t) : val_(t), state_(Some) {}
  constexpr Storage(std::remove_cvref_t<T>& t) : val_(t), state_(Some) {}
  constexpr Storage(std::remove_cvref_t<T>&& t)
      : val_(static_cast<T&&>(t)), state_(Some) {}

  union {
    T val_;
  };
  State state_ = None;

  constexpr inline State set_state(State s) {
    return ::sus::mem::replace(mref(state_), s);
  }
  constexpr inline State state() const { return state_; }
};

template <class T>
struct Storage<T&> final {
  constexpr ~Storage() = default;

  constexpr Storage(const Storage&) = default;
  constexpr Storage& operator=(const Storage&) = default;
  constexpr Storage(Storage&&) = default;
  constexpr Storage& operator=(Storage&&) = default;

  constexpr Storage(State) {}
  constexpr Storage(T& t) : ptr_(&t) {}

  T* ptr_ = nullptr;

  // The state is derived from the pointer.
  constexpr inline State state() const { return ptr_ ? Some : None; }
};

}  // namespace __private

/// A type which either holds #Some value of type T, or #None.
template <class T>
class Option;

/// Implementation of Option for a value type (non-reference).
template <class T>
class Option final {
 public:
  /// Construct an Option that is holding the given value.
  static inline constexpr Option some(const T& t) noexcept
    requires(std::is_copy_constructible_v<T>)
  {
    return Option(t);
  }

  /// Construct an Option that is holding the given value.
  template <class U>
  static inline constexpr Option some(Mref<U&> t) noexcept
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
  constexpr inline ~Option() noexcept
    requires(!std::is_trivially_destructible_v<T>)
  {
    if (t_.state() == Some) t_.val_.~T();
  }

  /// If T can be trivially copy-constructed, Option<T> can also be trivially
  /// copy-constructed.
  constexpr Option(const Option& o)
    requires(std::is_trivially_copy_constructible_v<T>)
  = default;

  constexpr Option(const Option& o) noexcept
    requires(!std::is_trivially_copy_constructible_v<T> &&
             std::is_copy_constructible_v<T>)
  : t_(o.t_.state()) {
    // If this could be done in a `constexpr` way, methods that receive an
    // Option could also be constexpr.
    if (t_.state() == Some) new (&t_.val_) T(const_cast<const T&>(o.t_.val_));
  }

  constexpr Option(const Option& o)
    requires(!std::is_copy_constructible_v<T>)
  = delete;

  /// If T can be trivially copy-constructed, Option<T> can also be trivially
  /// move-constructed.
  constexpr Option(Option&& o)
    requires(std::is_trivially_move_constructible_v<T>)
  = default;

  Option(Option&& o) noexcept
    requires(!std::is_trivially_move_constructible_v<T> &&
             std::is_move_constructible_v<T>)
  : t_(o.t_.set_state(None)) {
    // If this could be done in a `constexpr` way, methods that receive an
    // Option could also be constexpr.
    if (t_.state() == Some) new (&t_.val_) T(static_cast<T&&>(o.t_.val_));
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
    if (t_.set_state(o.t_.state()) == Some) {
      ::sus::mem::replace_and_discard(mref(t_.val_),
                                      const_cast<const T&>(o.t_.val_));
    } else {
      t_.val_.~T();
    }
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
    if (t_.set_state(o.t_.set_state(None)) == Some) {
      ::sus::mem::replace_and_discard(mref(t_.val_),
                                      static_cast<T&&>(o.t_.val_));
    } else {
      t_.val_.~T();
    }
    return *this;
  }

  constexpr Option& operator=(Option&& o)
    requires(!std::is_move_assignable_v<T>)
  = delete;

  /// Drop the current value from the Option, if there is one.
  ///
  /// Afterward the option will unconditionally be #None.
  constexpr void clear() & noexcept {
    if (t_.set_state(None) == Some) t_.val_.~T();
  }

  /// Returns whether the Option currently contains a value.
  ///
  /// If there is a value present, it can be extracted with <unwrap>() or
  /// <expect>().
  constexpr bool is_some() const noexcept { return t_.state() == Some; }
  /// Returns whether the Option is currently empty, containing no value.
  constexpr bool is_none() const noexcept { return t_.state() == None; }

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
  constexpr operator State() const& { return t_.state(); }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr T expect(/* TODO: string view type */ const char* msg) && noexcept
      sus_assertions_nonnull {
    ::sus::check_with_message(is_some(), *msg);
    return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr const T& expect_ref(
      /* TODO: string view type */ const char* msg) const& noexcept
      sus_assertions_nonnull {
    ::sus::check_with_message(is_some(), *msg);
    return t_.val_;
  }
  const T& expect_ref() && noexcept = delete;

  /// Returns a mutable reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_mut().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr T& expect_mut(
      /* TODO: string view type */ const char* msg) & noexcept
      sus_assertions_nonnull {
    ::sus::check_with_message(is_some(), *msg);
    return t_.val_;
  }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr T unwrap() && noexcept {
    ::sus::check(is_some());
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
    t_.set_state(None);
    return static_cast<T&&>(t_.val_);
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().unwrap()`.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr const T& unwrap_ref() const& noexcept {
    ::sus::check(is_some());
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
    ::sus::check(is_some());
    return t_.val_;
  }

  /// Returns the contained value inside the Option, if there is one. Otherwise,
  /// returns `default_result`.
  ///
  /// Note that if it is non-trivial to construct a `default_result`, that
  /// <unwrap_or_else>() should be used instead, as it will only construct the
  /// default value if required.
  constexpr T unwrap_or(T default_result) && noexcept {
    if (t_.set_state(None) == Some)
      return ::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_));
    else
      return default_result;
  }

  /// Returns the contained value inside the Option, if there is one.
  /// Otherwise, returns the result of the given function.
  template <class Functor>
    requires(std::is_same_v<std::invoke_result_t<Functor>, T>)
  constexpr T unwrap_or_else(Functor f) && noexcept {
    if (t_.set_state(None) == Some)
      return ::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_));
    else
      return f();
  }

  /// Returns the contained value inside the Option, if there is one.
  /// Otherwise, constructs a default value for the type and returns that.
  ///
  /// The Option's contained type `T` must be #MakeDefault, and will be
  /// constructed through that trait.
  constexpr T unwrap_or_default() && noexcept
    requires(::sus::construct::MakeDefault<T>)
  {
    if (t_.set_state(None) == Some)
      return ::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_));
    else
      return ::sus::construct::make_default<T>();
  }

  /// Stores the value `t` inside this Option, replacing any previous value, and
  /// returns a mutable reference to the new value.
  constexpr T& insert(T t) & noexcept {
    if (t_.set_state(Some) == None)
      new (&t_.val_) T(static_cast<T&&>(t));
    else
      ::sus::mem::replace_and_discard(mref(t_.val_), static_cast<T&&>(t));
    return t_.val_;
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// stores `t` inside the Option and returns a mutable reference to the new
  /// value.
  ///
  /// If it is non-trivial to construct `T`, the <get_or_insert_with>() method
  /// would be preferable, as it only constructs a `T` if needed.
  constexpr T& get_or_insert(T t) & noexcept {
    if (t_.set_state(Some) == None) new (&t_.val_) T(static_cast<T&&>(t));
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
  constexpr T& get_or_insert_default() & noexcept
    requires(::sus::construct::MakeDefault<T>)
  {
    if (t_.set_state(Some) == None)
      new (&t_.val_) T(::sus::construct::make_default<T>());
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
  constexpr T& get_or_insert_with(WithFn f) & noexcept {
    if (t_.set_state(Some) == None) new (&t_.val_) T(f());
    return t_.val_;
  }

  /// Returns a new Option containing whatever was inside the current Option.
  ///
  /// If this Option contains #None then it is left unchanged and returns an
  /// Option containing #None. If this Option contains #Some with a value, the
  /// value is moved into the returned Option and this Option will contain #None
  /// afterward.
  constexpr Option take() & noexcept {
    if (t_.set_state(None) == Some)
      return Option(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_)));
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
  constexpr Option<R> map(MapFn m) && noexcept {
    if (t_.set_state(None) == Some) {
      return Option<R>(
          m(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_))));
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
  constexpr Option<R> map_or(D default_result, MapFn m) && noexcept {
    if (t_.set_state(None) == Some) {
      return Option<R>(
          m(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_))));
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
  constexpr Option<R> map_or_else(DefaultFn default_fn, MapFn m) && noexcept {
    if (t_.set_state(None) == Some) {
      return Option<R>(
          m(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_))));
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
  constexpr Option<T> filter(Predicate p) && noexcept {
    if (t_.set_state(None) == Some) {
      if (p(const_cast<const T&>(t_.val_))) {
        return Option(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_)));
      } else {
        // The state has become None, so we must destroy the inner T.
        t_.val_.~T();
        return Option::none();
      }
    } else {
      return Option::none();
    }
  }

  /// Consumes this Option and returns an Option with #None if this Option holds
  /// #None, otherwise returns the given `opt`.
  template <class U>
  constexpr Option<U> and_opt(Option<U> opt) && noexcept {
    if (t_.set_state(None) == Some) {
      t_.val_.~T();
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
  constexpr Option<InnerR> and_then(AndFn f) && noexcept {
    if (t_.set_state(None) == Some)
      return f(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_)));
    else
      return Option<InnerR>::none();
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the given `opt`.
  constexpr Option<T> or_opt(Option<T> opt) && noexcept {
    if (t_.set_state(None) == Some)
      return Option(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_)));
    else
      return opt;
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the Option returned by `f`.
  template <class ElseFn, int&..., class R = std::invoke_result_t<ElseFn>>
    requires(std::is_same_v<R, Option<T>>)
  constexpr Option<T> or_else(ElseFn f) && noexcept {
    if (t_.set_state(None) == Some)
      return Option(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_)));
    else
      return static_cast<ElseFn&&>(f)();
  }

  /// Consumes this Option and returns an Option, holding the value from either
  /// this Option `opt`, if exactly one of them holds a value, otherwise returns
  /// an Option that holds #None.
  constexpr Option<T> xor_opt(Option<T> opt) && noexcept {
    if (t_.state() == Some) {
      // If `this` holds Some, we change `this` to hold None. If `opt` is None,
      // we return what this was holding, otherwise we return None.
      t_.set_state(None);
      if (opt.t_.state() == None) {
        return Option(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_)));
      } else {
        t_.val_.~T();
      }
      return Option::none();
    } else {
      // If `this` holds None, we need to do nothing to `this`. If `opt` is Some
      // we would return its value, and if `opt` is None we should return None.
      return opt;
    }
  }

  template <class E, int&..., class Result = ::sus::result::Result<T, E>>
  constexpr inline Result ok_or(E e) && noexcept {
    if (t_.set_state(None) == Some)
      return Result::with(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_)));
    else
      return Result::with_err(static_cast<E&&>(e));
  }

  template <class ElseFn, int&..., class E = std::invoke_result_t<ElseFn>,
            class Result = ::sus::result::Result<T, E>>
  constexpr inline Result ok_or_else(ElseFn f) && noexcept {
    if (t_.set_state(None) == Some)
      return Result::with(::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_)));
    else
      return Result::with_err(static_cast<ElseFn&&>(f)());
  }

  template <int&...,
            class OkType =
                typename ::sus::result::__private::IsResultType<T>::ok_type,
            class ErrType =
                typename ::sus::result::__private::IsResultType<T>::err_type,
            class Result = ::sus::result::Result<Option<OkType>, ErrType>>
    requires(::sus::result::__private::IsResultType<T>::value)
  constexpr inline Result transpose() && noexcept {
    if (t_.set_state(None) == None) {
      return Result::with(Option<OkType>::none());
    } else {
      if (t_.val_.is_ok()) {
        return Result::with(Option<OkType>::some(
            ::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_))
                .unwrap_unchecked(unsafe_fn)));
      } else {
        return Result::with_err(
            ::sus::mem::take_and_destruct(unsafe_fn, mref(t_.val_))
                .unwrap_err_unchecked(unsafe_fn));
      }
    }
  }

  /// Replaces whatever the Option is currently holding with #Some value `t` and
  /// returns an Option holding what was there previously.
  constexpr Option replace(T t) & noexcept {
    if (t_.set_state(Some) == None) {
      new (&t_.val_) T(static_cast<T&&>(t));
      return Option::none();
    } else {
      return Option(::sus::mem::replace(mref(t_.val_), static_cast<T&&>(t)));
    }
  }

  /// Maps an `Option<Option<T>>` to an `Option<T>`.
  constexpr T flatten() && noexcept
    requires(::sus::option::__private::IsOptionType<T>::value)
  {
    if (t_.state() == Some)
      return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
    else
      return T::none();
  }

  /// Returns an Option<const T&> from this Option<T>, that either holds #None
  /// or a reference to the value in this Option.
  constexpr Option<const T&> as_ref() const& noexcept {
    if (t_.state() == None)
      return Option<const T&>::none();
    else
      return Option<const T&>(t_.val_);
  }
  Option<const T&> as_ref() && noexcept = delete;

  /// Returns an Option<T&> from this Option<T>, that either holds #None or a
  /// reference to the value in this Option.
  constexpr Option<T&> as_mut() & noexcept {
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

  // TODO: Consider adding a for-loop adaptor, with a macro?
  // - begin() && { return into_iter().begin(); }

 private:
  template <class U>
  friend class Option;

  /// Constructor for #None.
  constexpr explicit Option() : t_(None) {}
  /// Constructor for #Some.
  constexpr explicit Option(const T& t) : t_(t) {}
  constexpr explicit Option(T&& t) : t_(static_cast<T&&>(t)) {}

  ::sus::option::__private::Storage<T> t_;

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
      Mref<std::remove_const_t<T>&> t) noexcept {
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
  constexpr void clear() & noexcept { t_.ptr_ = nullptr; }

  /// Returns whether the Option currently contains a value.
  ///
  /// If there is a value present, it can be extracted with <unwrap>() or
  /// <expect>().
  constexpr bool is_some() const noexcept { return t_.state() == Some; }
  /// Returns whether the Option is currently empty, containing no value.
  constexpr bool is_none() const noexcept { return t_.state() == None; }

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
  constexpr operator State() const& { return t_.state(); }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr T& expect(/* TODO: string view type */ const char* msg) && noexcept
      sus_assertions_nonnull {
    ::sus::check_with_message(is_some(), *msg);
    return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr const T& expect_ref(
      /* TODO: string view type */ const char* msg) const& noexcept
      sus_assertions_nonnull {
    ::sus::check_with_message(is_some(), *msg);
    return *t_.ptr_;
  }
  const T& expect_ref() && noexcept = delete;

  /// Returns a mutable reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_mut().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr T& expect_mut(
      /* TODO: string view type */ const char* msg) & noexcept
      sus_assertions_nonnull {
    ::sus::check_with_message(is_some(), *msg);
    return *t_.ptr_;
  }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr T& unwrap() && noexcept {
    ::sus::check(is_some());
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
    return *::sus::mem::replace_ptr(mref(t_.ptr_), nullptr);
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().unwrap()`.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr const T& unwrap_ref() const& noexcept {
    ::sus::check(is_some());
    return *t_.ptr_;
  }
  const T& unwrap_ref() && noexcept = delete;

  /// Returns a mutable reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_mut().unwrap()`.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr T& unwrap_mut() & noexcept {
    ::sus::check(is_some());
    return *t_.ptr_;
  }

  /// Returns the contained value inside the Option, if there is one. Otherwise,
  /// returns `default_result`.
  ///
  /// Note that if it is non-trivial to construct a `default_result`, that
  /// <unwrap_or_else>() should be used instead, as it will only construct the
  /// default value if required.
  constexpr T& unwrap_or(T& default_result) && noexcept {
    if (t_.state() == Some)
      return *::sus::mem::replace_ptr(mref(t_.ptr_), nullptr);
    else
      return default_result;
  }

  /// Returns the contained value inside the Option, if there is one.
  /// Otherwise, returns the result of the given function.
  template <class Functor>
    requires(std::is_same_v<std::invoke_result_t<Functor>, T&>)
  constexpr T& unwrap_or_else(Functor f) && noexcept {
    if (t_.state() == Some)
      return *::sus::mem::replace_ptr(mref(t_.ptr_), nullptr);
    else
      return f();
  }

  /// Stores the value `t` inside this Option, replacing any previous value, and
  /// returns a mutable reference to the new value.
  T& insert(T& t) & noexcept {
    t_.ptr_ = &t;
    return *t_.ptr_;
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// stores `t` inside the Option and returns a mutable reference to the new
  /// value.
  T& get_or_insert(T& t) & noexcept {
    if (t_.state() == None) t_.ptr_ = &t;
    return *t_.ptr_;
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// constructs a `T` by calling `f`, stores it inside the Option and returns a
  /// mutable reference to the new value.
  ///
  /// This method differs from <unwrap_or_else>() in that it does not consume
  /// the Option, and instead it can not be called on rvalues.
  template <class WithFn>
    requires(std::is_same_v<std::invoke_result_t<WithFn>, T&>)
  T& get_or_insert_with(WithFn f) & noexcept {
    if (t_.state() == None) t_.ptr_ = &f();
    return *t_.ptr_;
  }

  /// Returns a new Option containing whatever was inside the current Option.
  ///
  /// If this Option contains #None then it is left unchanged and returns an
  /// Option containing #None. If this Option contains #Some with a value, the
  /// value is moved into the returned Option and this Option will contain #None
  /// afterward.
  constexpr Option take() & noexcept {
    if (t_.state() == Some)
      return Option(*::sus::mem::replace_ptr(mref(t_.ptr_), nullptr));
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
  constexpr Option<R> map(MapFn m) && noexcept {
    if (t_.state() == Some)
      return Option<R>::some(
          m(*::sus::mem::replace_ptr(mref(t_.ptr_), nullptr)));
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
  constexpr Option<R> map_or(D default_result, MapFn m) && noexcept {
    if (t_.state() == Some)
      return Option<R>(m(*::sus::mem::replace_ptr(mref(t_.ptr_), nullptr)));
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
  constexpr Option<R> map_or_else(DefaultFn default_fn, MapFn m) && noexcept {
    if (t_.state() == Some)
      return Option<R>(m(*::sus::mem::replace_ptr(mref(t_.ptr_), nullptr)));
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
  constexpr Option<T&> filter(Predicate p) && noexcept {
    if (t_.state() == Some) {
      // The state must move to None.
      auto* ptr = ::sus::mem::replace_ptr(mref(t_.ptr_), nullptr);
      if (p(const_cast<const T&>(*ptr)))
        return Option(*ptr);
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
      t_.ptr_ = nullptr;
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
  constexpr Option<InnerR> and_then(AndFn f) && noexcept {
    if (t_.state() == Some)
      return f(*::sus::mem::replace_ptr(mref(t_.ptr_), nullptr));
    else
      return Option<InnerR>::none();
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the given `opt`.
  Option<T&> or_opt(Option<T&> opt) && noexcept {
    if (t_.state() == Some)
      return Option(*::sus::mem::replace_ptr(mref(t_.ptr_), nullptr));
    else
      return opt;
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the Option returned by `f`.
  template <class ElseFn, int&..., class R = std::invoke_result_t<ElseFn>>
    requires(std::is_same_v<R, Option<T&>>)
  constexpr Option<T&> or_else(ElseFn f) && noexcept {
    if (t_.state() == Some)
      return Option(*::sus::mem::replace_ptr(mref(t_.ptr_), nullptr));
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
      auto* ptr = ::sus::mem::replace_ptr(mref(t_.ptr_), nullptr);
      if (opt.t_.state() == None)
        return Option(*ptr);
      else
        return Option::none();
    } else {
      // If `this` holds None, we need to do nothing to `this`. If `opt` is Some
      // we would return its value, and if `opt` is None we should return None.
      return opt;
    }
  }

  /// Replaces whatever the Option is currently holding with #Some value `t` and
  /// returns an Option holding what was there previously.
  Option replace(T& t) & noexcept {
    if (t_.state() == None) {
      t_.ptr_ = &t;
      return Option::none();
    } else {
      return Option(*::sus::mem::replace_ptr(mref(t_.ptr_), &t));
    }
  }

  /// Maps an `Option<T&>` to an `Option<T>` by copying the referenced `T`.
  constexpr Option<T> copied() && noexcept
    requires(std::is_nothrow_copy_constructible_v<T>)
  {
    if (t_.state() == None)
      return Option<T>::none();
    else
      return Option<T>(const_cast<const T&>(*t_.ptr_));
  }

  /// Maps an `Option<Option<T>>` to an `Option<T>`.
  constexpr T flatten() && noexcept
    requires(::sus::option::__private::IsOptionType<T>::value)
  {
    if (t_.state() == Some)
      return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
    else
      return T::none();
  }

  /// Returns an Option<const T&> from this Option<T>, that either holds #None
  /// or a reference to the value in this Option.
  constexpr Option<const T&> as_ref() const& noexcept {
    if (t_.state() == None)
      return Option<const T&>::none();
    else
      return Option<const T&>(*t_.ptr_);
  }
  Option<const T&> as_ref() && noexcept = delete;

  /// Returns an Option<T&> from this Option<T>, that either holds #None or a
  /// reference to the value in this Option.
  Option<T&> as_mut() & noexcept {
    if (t_.state() == None)
      return Option<T&>::none();
    else
      return Option<T&>(*t_.ptr_);
  }

  Iterator<Once<const T&>> iter() const& noexcept {
    return Iterator<Once<const T&>>(as_ref());
  }
  Iterator<Once<const T&>> iter() const&& = delete;

  Iterator<Once<T&>> iter_mut() & noexcept {
    return Iterator<Once<T&>>(as_mut());
  }

  Iterator<Once<T&>> into_iter() && noexcept {
    return Iterator<Once<T&>>(take());
  }

  // TODO: Consider adding a for-loop adaptor, with a macro?
  // - begin() && { return into_iter().begin(); }

 private:
  template <class U>
  friend class Option;

  /// Constructor for #None.
  constexpr explicit Option() : t_(None) {}
  /// Constructor for #Some.
  constexpr explicit Option(T& t) : t_(t) {}

  ::sus::option::__private::Storage<T&> t_;

  sus_class_maybe_trivial_relocatable_types(unsafe_fn, T&);
};

/// sus::num::Eq<Option<U>> trait.
template <class T, class U>
  requires(::sus::num::Eq<T, U>)
constexpr inline bool operator==(const Option<T>& l,
                                 const Option<U>& r) noexcept {
  switch (l) {
    case Some: return r.is_some() && l.unwrap_ref() == r.unwrap_ref();
    case None: return r.is_none();
  }
  ::sus::unreachable_unchecked(unsafe_fn);
}

/// sus::num::Ord<Option<U>> trait.
template <class T, class U>
  requires(::sus::num::ExclusiveOrd<T, U>)
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

/// sus::num::WeakOrd<Option<U>> trait.
template <class T, class U>
  requires(::sus::num::ExclusiveWeakOrd<T, U>)
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

/// sus::num::PartialOrd<Option<U>> trait.
template <class T, class U>
  requires(::sus::num::ExclusivePartialOrd<T, U>)
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

}  // namespace sus::option

// Promote Option and its enum values into the `sus` namespace.
namespace sus {
using ::sus::option::None;
using ::sus::option::Option;
using ::sus::option::Some;
}  // namespace sus
