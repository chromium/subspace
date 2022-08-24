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

#include "mem/layout.h"
#include "mem/replace.h"
#include "mem/take.h"
#include "option/state.h"

namespace sus::option::__private {

using State::None;
using State::Some;

template <class T,
          bool HasNonNullField = sus::mem::layout::nonzero_field<T>::has_field>
struct Storage;

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
struct Storage<T, false> final {
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

  constexpr Storage() {}
  constexpr Storage(const std::remove_cvref_t<T>& t) : val_(t), state_(Some) {}
  constexpr Storage(std::remove_cvref_t<T>& t) : val_(t), state_(Some) {}
  constexpr Storage(std::remove_cvref_t<T>&& t)
      : val_(static_cast<T&&>(t)), state_(Some) {}

  union {
    T val_;
  };
  State state_ = None;

  [[nodiscard]] constexpr inline State state() const { return state_; }

  constexpr inline void construct_from_none(T&& t) noexcept {
    new (&val_) T(static_cast<T&&>(t));
    state_ = Some;
  }

  [[nodiscard]] constexpr inline void set_some(T&& t) noexcept {
    if (state_ == None)
      construct_from_none(static_cast<T&&>(t));
    else
      ::sus::mem::replace_and_discard(mref(val_), static_cast<T&&>(t));
    state_ = Some;
  }

  [[nodiscard]] constexpr inline T replace_some(T&& t) noexcept {
    return ::sus::mem::replace(mref(val_), static_cast<T&&>(t));
  }

  [[nodiscard]] constexpr inline T take_and_set_none() noexcept {
    state_ = None;
    return ::sus::mem::take_and_destruct(unsafe_fn, val_);
  }

  constexpr inline void set_none() noexcept {
    state_ = None;
    val_.~T();
  }
};

template <class T>
struct Storage<T, true> final {
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

  constexpr Storage() {
    ::sus::mem::layout::nonzero_field<T>::set_zero(unsafe_fn, &val_);
  }
  constexpr Storage(const T& t) : val_(t) {}
  constexpr Storage(T&& t) : val_(static_cast<T&&>(t)) {}

  union {
    T val_;
  };

  [[nodiscard]] constexpr inline State state() const noexcept {
    return ::sus::mem::layout::nonzero_field<T>::is_non_zero(unsafe_fn, &val_)
               ? Some
               : None;
  }

  inline void construct_from_none(T&& t) noexcept {
    new (&val_) T(static_cast<T&&>(t));
  }

  [[nodiscard]] constexpr inline T take_and_set_none() noexcept {
    T t = take_and_destruct(unsafe_fn, mref(val_));
    ::sus::mem::layout::nonzero_field<T>::set_zero(unsafe_fn, &val_);
    return t;
  }

  constexpr inline void set_none() noexcept {
    val_.~T();
    ::sus::mem::layout::nonzero_field<T>::set_zero(unsafe_fn, &val_);
  }
};

}  // namespace sus::option::__private
