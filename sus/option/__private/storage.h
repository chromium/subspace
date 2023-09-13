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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include <memory>

#include "sus/macros/inline.h"
#include "sus/macros/nonnull.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/addressof.h"
#include "sus/mem/move.h"
#include "sus/mem/never_value.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/mem/take.h"
#include "sus/option/state.h"

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
  constexpr Storage(const std::remove_cvref_t<T>& t) : state_(Some), val_(t) {}
  constexpr Storage(std::remove_cvref_t<T>& t) : state_(Some), val_(t) {}
  constexpr Storage(std::remove_cvref_t<T>&& t)
      : state_(Some), val_(::sus::move(t)) {}

  sus_pure constexpr const T& val() const { return val_; }
  sus_pure constexpr T& val_mut() { return val_; }

  sus_pure constexpr inline State state() const noexcept { return state_; }

  constexpr inline void construct_from_none(const T& t) noexcept
    requires(::sus::mem::Copy<T>)
  {
    std::construct_at(&val_, t);
    // Avoid setting to `Some` during the execution of the ctor.
    state_ = Some;
  }
  constexpr inline void construct_from_none(T&& t) noexcept {
    std::construct_at(&val_, ::sus::move(t));
    // Avoid setting to `Some` during the execution of the ctor.
    state_ = Some;
  }

  constexpr inline void set_some(const T& t) noexcept
    requires(::sus::mem::Copy<T>)
  {
    if (state_ == None)
      construct_from_none(t);
    else
      val_ = t;
    // Avoid setting to `Some` during the execution of the ctor.
    state_ = Some;
  }
  constexpr inline void set_some(T&& t) noexcept {
    if (state_ == None)
      construct_from_none(::sus::move(t));
    else
      val_ = ::sus::move(t);
    // Avoid setting to `Some` during the execution of the ctor.
    state_ = Some;
  }

  [[nodiscard]] constexpr inline T replace_some(T&& t) noexcept {
    return ::sus::mem::replace(val_, ::sus::move(t));
  }

  [[nodiscard]] constexpr inline T take_and_set_none() noexcept {
    // Set to `None` before the execution of the dtor.
    state_ = None;
    if constexpr (::sus::mem::Move<T>) {
      return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn, val_);
    } else {
      return ::sus::mem::take_copy_and_destruct(::sus::marker::unsafe_fn, val_);
    }
  }

  constexpr inline void set_none() noexcept {
    // Set to `None` before the execution of the dtor.
    state_ = None;
    val_.~T();
  }

  constexpr inline void destroy() noexcept { val_.~T(); }

 private:
  // The `state_` comes first because davidben@ found that this has a profound
  // impact on binary size in Chromium ARM64. This matches `std::optional`'s
  // order, but is the inverse of `absl::optional`'s. Since the `state_` is
  // always read, but the `val_` is only read sometimes, the more frequent one
  // is at offset 0. We don't know the details about _why_ it ends up smaller
  // though.
  State state_ = None;
  union {
    T val_;
  };

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

  constexpr Storage() : access_() {}
  constexpr Storage(const T& t) : access_(t) {}
  constexpr Storage(T&& t) : access_(::sus::move(t)) {}

  sus_pure constexpr const T& val() const { return access_.as_inner(); };
  sus_pure constexpr T& val_mut() { return access_.as_inner_mut(); };

  sus_pure constexpr inline State state() const noexcept {
    return access_.is_constructed() ? Some : None;
  }

  constexpr inline void construct_from_none(const T& t) noexcept
    requires(::sus::mem::Copy<T>)
  {
    access_.set_destroy_value(::sus::marker::unsafe_fn);
    access_.~NeverValueAccess();
    std::construct_at(&access_, t);
  }
  constexpr inline void construct_from_none(T&& t) noexcept {
    access_.set_destroy_value(::sus::marker::unsafe_fn);
    access_.~NeverValueAccess();
    std::construct_at(&access_, ::sus::move(t));
  }

  constexpr inline void set_some(const T& t) noexcept
    requires(::sus::mem::Copy<T>)
  {
    if (state() == None)
      construct_from_none(t);
    else
      access_.as_inner_mut() = t;
  }
  constexpr inline void set_some(T&& t) noexcept {
    if (state() == None)
      construct_from_none(::sus::move(t));
    else
      access_.as_inner_mut() = ::sus::move(t);
  }

  [[nodiscard]] constexpr inline T replace_some(T&& t) noexcept {
    return ::sus::mem::replace(access_.as_inner_mut(), ::sus::move(t));
  }

  [[nodiscard]] constexpr inline T take_and_set_none() noexcept {
    auto t = T(::sus::move(access_.as_inner_mut()));
    access_.~NeverValueAccess();
    std::construct_at(&access_);
    return t;
  }

  constexpr inline void set_none() noexcept {
    access_.~NeverValueAccess();
    std::construct_at(&access_);
  }

  constexpr inline void destroy() noexcept {
    if (!access_.is_constructed())
      access_.set_destroy_value(::sus::marker::unsafe_fn);
    access_.~NeverValueAccess();
  }

 private:
  using NeverValueAccess = ::sus::mem::__private::NeverValueAccess<T>;

  union {
    NeverValueAccess access_;
  };

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(access_));
};

template <class T>
struct StoragePointer {};

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
  // The pointer is never set to null in public usage.
  sus_class_never_value_field(::sus::marker::unsafe_fn, StoragePointer, ptr_,
                              nullptr, nullptr);
  // For the NeverValueField.
  explicit constexpr StoragePointer(::sus::mem::NeverValueConstructor) noexcept
      : ptr_(nullptr) {}
};

static_assert(std::is_trivially_copy_constructible_v<StoragePointer<int&>>);
static_assert(std::is_trivially_copy_assignable_v<StoragePointer<int&>>);
static_assert(std::is_trivially_move_constructible_v<StoragePointer<int&>>);
static_assert(std::is_trivially_move_assignable_v<StoragePointer<int&>>);

// This must be true in order for StoragePointer to be useful with the
// never-value field optimization.
// clang-format off
static_assert(::sus::mem::NeverValueField<StoragePointer<int&>>);
// clang-format on

}  // namespace sus::option::__private
