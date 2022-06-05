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

#include <type_traits>

#include "assertions/check.h"
#include "marker/unsafe.h"
#include "mem/replace.h"
#include "mem/take.h"
#include "traits/make_default.h"

namespace sus::option {

template <class T>
class Option;

namespace __private {

template <class U>
struct IsOptionType : std::false_type {
  using type = void;
};

template <class U>
struct IsOptionType<Option<U>> : std::true_type {
  using type = U;
};

}  // namespace __private

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

/// A type which either holds #Some value of type T, or #None.
template <class T>
class Option {
 public:
  /// Construct an Option that is holding the given value.
  static inline constexpr Option some(T t) noexcept {
    return Option(static_cast<T&&>(t));
  }
  /// Construct an Option that is holding no value.
  static inline constexpr Option none() noexcept { return Option(); }

  /// Construct an Option with the default value for the type it contains.
  ///
  /// The Option's contained type `T` must be #MakeDefault, and will be
  /// constructed through that trait.
  template <class U = T>
    requires(::sus::traits::MakeDefault<U>::has_trait && std::is_same_v<U, T>)
  static inline constexpr Option<T> with_default() noexcept {
    return Option<T>::some(::sus::traits::MakeDefault<T>::make_default());
  }

  /// Destructor for the Option.
  ///
  /// Destroys the value contained within the option, if there is one.
  constexpr inline ~Option() noexcept {
    if constexpr (!std::is_reference_v<T>)
      if (state_ == Some) t_.val_.~T();
  }

  Option(Option&& o) noexcept : state_(o.state_) {
    // If this could be done in a `constexpr` way, methods that receive an
    // Option could also be constexpr.
    if constexpr (!std::is_reference_v<T>) {
      if (state_ == Some) new (&t_.val_) T(static_cast<T&&>(o.t_.val_));
    } else {
      if (state_ == Some) t_.ptr_ = o.t_.ptr_;
    }
  }
  Option& operator=(Option&& o) noexcept {
    if constexpr (!std::is_reference_v<T>) {
      if (state_ == Some) t_.val_.~T();
      if (o.state_ == Some) new (&t_.val_) T(static_cast<T&&>(o.t_.val_));
    } else {
      if (o.state_ == Some) t_.ptr_ = o.t_.ptr_;
    }
    state_ = o.state_;
    return *this;
  }

  /// Drop the current value from the Option, if there is one.
  ///
  /// Afterward the option will unconditionally be #None.
  constexpr void clear() & noexcept {
    if constexpr (!std::is_reference_v<T>)
      if (::sus::mem::replace(state_, None) == Some) t_.val_.~T();
  }

  /// Returns whether the Option currently contains a value.
  ///
  /// If there is a value present, it can be extracted with <unwrap>() or
  /// <expect>().
  constexpr bool is_some() const noexcept { return state_ == Some; }
  /// Returns whether the Option is currently empty, containing no value.
  constexpr bool is_none() const noexcept { return state_ == None; }

  /// An operator which returns the state of the Option, either #Some or #None.
  ///
  /// This supports the use of an Option in a `switch()`, allowing it to act as
  /// a tagged union between "some value" and "no value".
  ///
  /// # Example
  ///
  /// ```cpp
  /// auto x = Option<int>::some();
  /// switch (x) {
  ///  case Some:
  ///   return static_cast<decltype(x)&&>(x).unwrap_unchecked(unsafe_fn);
  ///  case None:
  ///   return -1;
  /// }
  /// ```
  operator State() { return state_; }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr T expect(/* TODO: string view type */ const char* msg) && noexcept
      [[nonnull]] {
    ::sus::check_with_message(state_ == Some, *msg);
    return static_cast<Option&&>(*this).unwrap_unchecked(unsafe_fn);
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr const std::remove_reference_t<T>& expect_ref(
      /* TODO: string view type */ const char* msg) const& noexcept
      [[nonnull]] {
    ::sus::check_with_message(state_ == Some, *msg);
    if constexpr (!std::is_reference_v<T>)
      return t_.val_;
    else
      return const_cast<const T>(*t_.ptr_);
  }
  std::remove_reference_t<T>& expect_ref() && noexcept = delete;

  /// Returns a mutable reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_mut().expect()`.
  ///
  /// The function will panic with the given message if the Option's state is
  /// currently `None`.
  constexpr std::remove_reference_t<T>& expect_mut(
      /* TODO: string view type */ const char* msg) & noexcept [[nonnull]] {
    ::sus::check_with_message(state_ == Some, *msg);
    if constexpr (!std::is_reference_v<T>)
      return t_.val_;
    else
      return *t_.ptr_;
  }

  /// Returns the contained value inside the Option.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr T unwrap() && noexcept {
    ::sus::check(state_ == Some);
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
    state_ = None;
    if constexpr (!std::is_reference_v<T>)
      return static_cast<T&&>(t_.val_);
    else
      return *t_.ptr_;
  }

  /// Returns a const reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_ref().unwrap()`.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr const std::remove_reference_t<T>& unwrap_ref() const& noexcept {
    ::sus::check(state_ == Some);
    if constexpr (!std::is_reference_v<T>)
      return t_.val_;
    else
      return const_cast<const T>(*t_.ptr_);
  }
  const std::remove_reference_t<T>& unwrap_ref() && noexcept = delete;

  /// Returns a mutable reference to the contained value inside the Option.
  ///
  /// This is a shortcut for `option.as_mut().unwrap()`.
  ///
  /// The function will panic without a message if the Option's state is
  /// currently `None`.
  constexpr std::remove_reference_t<T>& unwrap_mut() & noexcept {
    ::sus::check(state_ == Some);
    if constexpr (!std::is_reference_v<T>)
      return t_.val_;
    else
      return *t_.ptr_;
  }

  /// Returns the contained value inside the Option, if there is one. Otherwise,
  /// returns `default_result`.
  ///
  /// Note that if it is non-trivial to construct a `default_result`, that
  /// <unwrap_or_else>() should be used instead, as it will only construct the
  /// default value if required.
  constexpr T unwrap_or(T default_result) && noexcept {
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>)
        return ::sus::mem::take_and_destruct(unsafe_fn, t_.val_);
      else
        return *t_.ptr_;
    } else {
      return default_result;
    }
  }
  /// Returns the contained value inside the Option, if there is one. Otherwise,
  /// returns the result of the given function.
  template <class Functor>
    requires(std::is_same_v<std::invoke_result_t<Functor>, T>)
  constexpr T unwrap_or_else(Functor f) && noexcept {
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>)
        return ::sus::mem::take_and_destruct(unsafe_fn, t_.val_);
      else
        return *t_.ptr_;
    } else {
      return f();
    }
  }
  /// Returns the contained value inside the Option, if there is one. Otherwise,
  /// constructs a default value for the type and returns that.
  ///
  /// The Option's contained type `T` must be #MakeDefault, and will be
  /// constructed through that trait.
  template <class U = T>
    requires(::sus::traits::MakeDefault<U>::has_trait && std::is_same_v<U, T>)
  constexpr T unwrap_or_default() && noexcept {
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>)
        return ::sus::mem::take_and_destruct(unsafe_fn, t_.val_);
      else
        return *t_.ptr_;
    } else {
      return ::sus::traits::MakeDefault<T>::make_default();
    }
  }

  /// Stores the value `t` inside this Option, replacing any previous value, and
  /// returns a mutable reference to the new value.
  T& insert(T t) & noexcept {
    if constexpr (!std::is_reference_v<T>) {
      if (::sus::mem::replace(state_, Some) == None)
        new (&t_.val_) T(static_cast<T&&>(t));
      else
        t_.val_ = static_cast<T&&>(t);
      return t_.val_;
    } else {
      state_ = Some;
      t_.ptr_ = &t;
      return *t_.ptr_;
    }
  }

  /// If the Option holds a value, returns a mutable reference to it. Otherwise,
  /// stores `t` inside the Option and returns a mutable reference to the new
  /// value.
  ///
  /// If it is non-trivial to construct `T`, the <get_or_insert_with>() method
  /// would be preferable, as it only constructs a `T` if needed.
  T& get_or_insert(T t) & noexcept {
    if constexpr (!std::is_reference_v<T>) {
      if (::sus::mem::replace(state_, Some) == None)
        new (&t_.val_) T(static_cast<T&&>(t));
      return t_.val_;
    } else {
      state_ = Some;
      t_.ptr_ = &t;
      return *t_.ptr_;
    }
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
  template <class U = T>
    requires(::sus::traits::MakeDefault<U>::has_trait && std::is_same_v<U, T>)
  T& get_or_insert_default() & noexcept {
    static_assert(!std::is_reference_v<T>, "");  // We require MakeDefault<T>.
    if (::sus::mem::replace(state_, Some) == None)
      new (&t_.val_) T(::sus::traits::MakeDefault<T>::make_default());
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
    if constexpr (!std::is_reference_v<T>) {
      if (::sus::mem::replace(state_, Some) == None) new (&t_.val_) T(f());
      return t_.val_;
    } else {
      state_ = Some;
      t_.ptr_ = &f();
      return *t_.ptr_;
    }
  }

  /// Returns a new Option containing whatever was inside the current Option.
  ///
  /// If this Option contains #None then it is left unchanged and returns an
  /// Option containing #None. If this Option contains #Some with a value, the
  /// value is moved into the returned Option and this Option will contain
  /// #None afterward.
  constexpr Option take() & noexcept {
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>)
        return Option::some(::sus::mem::take_and_destruct(unsafe_fn, t_.val_));
      else
        return Option::some(*t_.ptr_);
    } else {
      return Option::none();
    }
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
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>) {
        return Option<R>::some(
            m(::sus::mem::take_and_destruct(unsafe_fn, t_.val_)));
      } else {
        return Option<R>::some(m(*t_.ptr_));
      }
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
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>) {
        return Option<R>::some(
            m(::sus::mem::take_and_destruct(unsafe_fn, t_.val_)));
      } else {
        return Option<R>::some(m(*t_.ptr_));
      }
    } else {
      return Option<R>::some(static_cast<R&&>(default_result));
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
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>)
        return Option<R>::some(
            m(::sus::mem::take_and_destruct(unsafe_fn, t_.val_)));
      else
        return Option<R>::some(m(*t_.ptr_));
    } else {
      return Option<R>::some(default_fn());
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
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>) {
        if (p(const_cast<const T&>(t_.val_))) {
          return Option::some(
              ::sus::mem::take_and_destruct(unsafe_fn, t_.val_));
        } else {
          // state_ has become None, so we must not keep `t_` alive.
          t_.val_.~T();
          return Option::none();
        }
      } else {
        return p(const_cast<const T>(*t_.ptr_)) ? Option::some(*t_.ptr_)
                                                : Option::none();
      }
    } else {
      return Option::none();
    }
  }

  /// Consumes this Option and returns an Option with #None if this Option holds
  /// #None, otherwise returns the given `opt`.
  template <class U>
  Option<U> and_opt(Option<U> opt) && noexcept {
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>) t_.val_.~T();
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
  template <class AndFn, int&..., class R = std::invoke_result_t<AndFn, T&&>,
            class InnerR = ::sus::option::__private::IsOptionType<R>::type>
    requires(::sus::option::__private::IsOptionType<R>::value)
  constexpr Option<InnerR> and_then(AndFn f) && noexcept {
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>) {
        return f(::sus::mem::take_and_destruct(unsafe_fn, t_.val_));
      } else {
        return f(*t_.ptr_);
      }
    } else {
      return Option<InnerR>::none();
    }
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the given `opt`.
  Option<T> or_opt(Option<T> opt) && noexcept {
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>) {
        return Option::some(::sus::mem::take_and_destruct(unsafe_fn, t_.val_));
      } else {
        return Option::some(*t_.ptr_);
      }
    } else {
      return opt;
    }
  }

  /// Consumes and returns an Option with the same value if this Option contains
  /// a value, otherwise returns the Option returned by `f`.
  template <class ElseFn, int&..., class R = std::invoke_result_t<ElseFn>>
    requires(std::is_same_v<R, Option<T>>)
  constexpr Option<T> or_else(ElseFn f) && noexcept {
    if (::sus::mem::replace(state_, None) == Some) {
      if constexpr (!std::is_reference_v<T>) {
        return Option::some(::sus::mem::take_and_destruct(unsafe_fn, t_.val_));
      } else {
        return Option::some(*t_.ptr_);
      }
    } else {
      return f();
    }
  }

  /// Consumes this Option and returns an Option, holding the value from either
  /// this Option `opt`, if exactly one of them holds a value, otherwise returns
  /// an Option that holds #None.
  Option<T> xor_opt(Option<T> opt) && noexcept {
    State lhs = ::sus::mem::replace(state_, None);
    State rhs = opt.state_;
    if (lhs != None) {
      if constexpr (!std::is_reference_v<T>) {
        Option<T> self =
            Option::some(::sus::mem::take_and_destruct(unsafe_fn, t_.val_));
        if (rhs == None) return self;
      } else {
        if (rhs == None) return Option::some(*t_.ptr_);
      }
      return Option::none();
    } else if (rhs != None) {
      return opt;
    } else {
      return Option::none();
    }
  }

  /// Returns an Option<const T&> from this Option<T>, that either holds #None
  /// or a reference to the value in this Option.
  constexpr Option<const std::remove_reference_t<T>&> as_ref() const& noexcept {
    if (state_ == None) {
      return Option<const std::remove_reference_t<T>&>::none();
    } else {
      if constexpr (!std::is_reference_v<T>)
        return Option<const std::remove_reference_t<T>&>::some(t_.val_);
      else
        return Option<const std::remove_reference_t<T>&>::some(
            const_cast<const T>(*t_.ptr_));
    }
  }
  Option<const std::remove_reference_t<T>&> as_ref() && noexcept = delete;

  /// Returns an Option<T&> from this Option<T>, that either holds #None or a
  /// reference to the value in this Option.
  Option<std::remove_reference_t<T>&> as_mut() & noexcept {
    if (state_ == None) {
      return Option<std::remove_reference_t<T>&>::none();
    } else {
      if constexpr (!std::is_reference_v<T>)
        return Option<std::remove_reference_t<T>&>::some(t_.val_);
      else
        return Option<std::remove_reference_t<T>&>::some(*t_.ptr_);
    }
  }

 private:
  /// Constructor for #None.
  constexpr explicit Option() : state_(None) {}
  /// Constructor for #Some.
  constexpr explicit Option(std::remove_reference_t<T>& t)
      : state_(Some), t_(&t) {}
  /// Constructor for #Some.
  constexpr explicit Option(std::remove_reference_t<T>&& t)
      : state_(Some), t_(static_cast<T&&>(t)) {}

  template <class U>
  struct Storage {
    U val_;
  };
  template <class U>
  struct Storage<U&> {
    U* ptr_;
  };

  // TODO: determine if we can put the tag into `T` from its type (e.g. sizeof
  // < alignment?), and then do so?
  union {
    Storage<T> t_;
  };

  State state_;
};

}  // namespace sus::option

// Promote Option and its enum values into the `sus` namespace.
namespace sus {
using ::sus::option::None;
using ::sus::option::Option;
using ::sus::option::Some;
}  // namespace sus
