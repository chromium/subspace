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

#include "subspace/macros/always_inline.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/addressof.h"
#include "subspace/mem/move.h"
#include "subspace/mem/mref.h"
#include "subspace/mem/never_value.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/replace.h"
#include "subspace/mem/take.h"
#include "subspace/option/state.h"

namespace sus::option::__private {

using State::None;
using State::Some;

template <class T, bool = sus::mem::NeverValueField<T>>
struct Storage;

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

  // Make the Storage have the same non-trivial copy/move characteristics, for
  // Option to query. But these aren't used by Option, since it copies/moves the
  // stored value itself.
  constexpr Storage(const Storage&)
    requires(!std::is_trivially_copy_constructible_v<T>)
  {
    sus::unreachable();
  }
  constexpr Storage& operator=(const Storage&)
    requires(!std::is_trivially_copy_assignable_v<T>)
  {
    sus::unreachable();
  }
  constexpr Storage(Storage&&)
    requires(!std::is_trivially_move_constructible_v<T>)
  {
    sus::unreachable();
  }
  constexpr Storage& operator=(Storage&&)
    requires(!std::is_trivially_move_assignable_v<T>)
  {
    sus::unreachable();
  }

  constexpr Storage() {}
  constexpr Storage(const std::remove_cvref_t<T>& t) : val_(t), state_(Some) {}
  constexpr Storage(std::remove_cvref_t<T>& t) : val_(t), state_(Some) {}
  constexpr Storage(std::remove_cvref_t<T>&& t)
      : val_(::sus::move(t)), state_(Some) {}

  union {
    T val_;
  };
  State state_ = None;

  [[nodiscard]] constexpr inline State state() const noexcept { return state_; }

  constexpr inline void construct_from_none(const T& t) noexcept {
    new (&val_) T(t);
    state_ = Some;
  }
  constexpr inline void construct_from_none(T&& t) noexcept {
    new (&val_) T(::sus::move(t));
    state_ = Some;
  }

  constexpr inline void set_some(T&& t) noexcept {
    if (state_ == None)
      construct_from_none(::sus::move(t));
    else
      ::sus::mem::replace_and_discard(mref(val_), ::sus::move(t));
    state_ = Some;
  }

  [[nodiscard]] constexpr inline T replace_some(T&& t) noexcept {
    return ::sus::mem::replace(mref(val_), ::sus::move(t));
  }

  [[nodiscard]] constexpr inline T take_and_set_none() noexcept {
    state_ = None;
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn, mref(val_));
  }

  constexpr inline void set_none() noexcept {
    state_ = None;
    val_.~T();
  }

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(val_));
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

  // Make the Storage have the same non-trivial copy/move characteristics, for
  // Option to query. But these aren't used by Option, since it copies/moves the
  // stored value itself.
  constexpr Storage(const Storage&)
    requires(!std::is_trivially_copy_constructible_v<T>)
  {
    sus::unreachable();
  }
  constexpr Storage& operator=(const Storage&)
    requires(!std::is_trivially_copy_assignable_v<T>)
  {
    sus::unreachable();
  }
  constexpr Storage(Storage&&)
    requires(!std::is_trivially_move_constructible_v<T>)
  {
    sus::unreachable();
  }
  constexpr Storage& operator=(Storage&&)
    requires(!std::is_trivially_move_assignable_v<T>)
  {
    sus::unreachable();
  }

  constexpr Storage() : overlay_() {
    ::sus::mem::never_value_access<T>::set_never_value(::sus::marker::unsafe_fn,
                                                       overlay_);
  }
  constexpr Storage(const T& t) : val_(t) {}
  constexpr Storage(T&& t) : val_(::sus::move(t)) {}

  using Overlay = typename ::sus::mem::never_value_access<T>::OverlayType;

  union {
    Overlay overlay_;
    T val_;
  };
  // If both `bytes_` and `val_` are standard layout, and the same size, then we
  // can access the memory of one through the other in a well-defined way:
  // https://en.cppreference.com/w/cpp/language/union
  static_assert(std::is_standard_layout_v<Overlay>);
  static_assert(std::is_standard_layout_v<T>);

  // Not constexpr because in a constant-evalation context, the compiler will
  // produce an error if the Option is #Some, since we're reading the union's
  // inactive field in that case. When using the never-value field optimization,
  // it's not possible to query the state of the Option, without already knowing
  // the correct state and thus the correct union field to read, given the
  // current limitations of constexpr in C++20.
  [[nodiscard]] inline State state() const noexcept {
    return ::sus::mem::never_value_access<T>::is_constructed(
               ::sus::marker::unsafe_fn, overlay_)
               ? Some
               : None;
  }

  inline void construct_from_none(const T& t) noexcept { new (&val_) T(t); }
  inline void construct_from_none(T&& t) noexcept {
    new (&val_) T(::sus::move(t));
  }

  constexpr inline void set_some(T&& t) noexcept {
    if (state() == None)
      construct_from_none(::sus::move(t));
    else
      ::sus::mem::replace_and_discard(mref(val_), ::sus::move(t));
  }

  [[nodiscard]] constexpr inline T replace_some(T&& t) noexcept {
    return ::sus::mem::replace(mref(val_), ::sus::move(t));
  }

  [[nodiscard]] constexpr inline T take_and_set_none() noexcept {
    T t = ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn, mref(val_));
    // Make the overlay_ field active.
    overlay_ = Overlay();
    ::sus::mem::never_value_access<T>::set_never_value(::sus::marker::unsafe_fn,
                                                       overlay_);
    return t;
  }

  constexpr inline void set_none() noexcept {
    val_.~T();
    // Make the overlay_ field active.
    overlay_ = Overlay();
    ::sus::mem::never_value_access<T>::set_never_value(::sus::marker::unsafe_fn,
                                                       overlay_);
  }

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(val_));
};

template <class T>
struct StoragePointer;

template <class T>
struct [[sus_trivial_abi]] StoragePointer<T&> {
  explicit constexpr sus_always_inline StoragePointer(T& ref) noexcept
      : ptr_(::sus::mem::addressof(ref)) {}

  inline constexpr operator const T&() const { return *ptr_; }
  inline constexpr operator T&() { return *ptr_; }

 private:
  T* ptr_;

  // Pointers are trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ptr_));
  // The pointer is never set to null.
  sus_class_never_value_field(::sus::marker::unsafe_fn, StoragePointer, ptr_,
                              nullptr);
};

// This must be true in order for StoragePointer to be useful with the
// never-value field optimization.
// clang-format off
static_assert(::sus::mem::NeverValueField<StoragePointer<int&>>);
// clang-format on

}  // namespace sus::option::__private
