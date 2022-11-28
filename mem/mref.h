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

#include <stddef.h>  // Replace with usize?

#include <type_traits>

#include "assertions/check.h"
#include "macros/__private/compiler_bugs.h"
#include "marker/unsafe.h"
#include "mem/move.h"
#include "mem/relocate.h"

namespace sus::mem {

template <class T>
  requires(!std::is_reference_v<T> && !std::is_array_v<T>)
struct Mref;

/// Pass a variable to a function as a mutable reference.
template <class T>
constexpr inline Mref<T> mref(T& t) noexcept {
  return Mref<T>(Mref<T>::kConstruct, t);
}

template <class T>
constexpr inline Mref<T> mref(const T& t) = delete;

/// A mutable reference receiver.
///
/// Mref should only be used as a function parameter. It receives a mutable
/// (lvalue) reference, and requires the caller to pass it explicitly with
/// mref().
///
/// This ensures that passing a variable as mutable is visible at the callsite.
/// It generates the same code as a bare reference:
/// https://godbolt.org/z/9xPaqhvfq
///
/// # Example
///
/// ```
/// // Without Mref:
/// void receive_ref(int& i) { i++; }
///
/// // With Mref:
/// void receive_ref(Mref<int> i) { i++; }
///
/// int i;
/// receive_ref(mref(i));   // Explicitly pass lvalue ref.
/// ```
template <class T>
  requires(!std::is_reference_v<T> && !std::is_array_v<T>)
struct [[sus_trivial_abi]] Mref final {
  /// Mref can be trivially moved, so this is the move constructor.
  constexpr Mref(Mref&&) noexcept = default;
  /// Mref can be trivially moved, but is only meant for a function argument, so
  /// no need for assignment.
  constexpr Mref& operator=(Mref&&) noexcept = delete;

  /// Prevent constructing an Mref argument without writing mref().
  Mref(T& t) = delete;
  /// Prevent passing an Mref argument along without writing mref() again.
  Mref(Mref&) = delete;

  /// Returns the reference held by the Mref.
  constexpr inline T& inner() & noexcept { return t_; }

  /// Act like a T&. It can convert to a T&.
  constexpr inline operator T&() & noexcept { return t_; }
  /// Act like a T&. It can be assigned a new T.
  constexpr inline Mref& operator=(const T& t) noexcept
    requires(std::is_copy_assignable_v<T>)
  {
    t_ = t;
    return *this;
  }
  /// Act like a T&. It can be assigned a new T.
  constexpr inline Mref& operator=(T&& t) noexcept
    requires(std::is_move_assignable_v<T>)
  {
    t_ = ::sus::move(t);
    return *this;
  }

 private:
  friend constexpr Mref<T> mref<>(T&) noexcept;

  enum Construct { kConstruct };
  constexpr inline Mref(Construct, T& reference) noexcept : t_(reference) {}

  T& t_;

  sus_class_assert_trivial_relocatable_types(unsafe_fn, decltype(t_));
};

}  // namespace sus::mem

// Promote Mref into the `sus` namespace.
namespace sus {
using ::sus::mem::Mref;
}  // namespace sus

// Promote mref() into the top level namespace.
// TODO: Provide an option to do this or not.
using ::sus::mem::mref;
