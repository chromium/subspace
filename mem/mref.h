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

namespace sus::mem {

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
class Mref;

template <class T>
  requires(!std::is_reference_v<T>)
struct Mref<T> final {
  static_assert(
      std::is_reference_v<T>,
      "The type parameter for Mref<T> must be a mutable reference: Mref<T&>");
};

template <class T>
struct Mref<const T&> final {
  static_assert(
      std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>,
      "The type parameter for Mref<T> must be a mutable reference: Mref<T&>");
};

/// Pass a variable to a function as a mutable reference.
template <class T>
constexpr inline Mref<T&> mref(T& t) {
  using Construct = Mref<T&>::Construct;
  return Mref<T&>(Construct(t));
}

template <class T>
constexpr inline Mref<T&> mref(const T& t) = delete;

/// An Mref can be passed along as an Mref.
template <class T>
constexpr inline Mref<T> mref(Mref<T>& t) {
  using Construct = Mref<T&>::Construct;
  return Mref<T>(Construct(t.inner()));
}

template <class T>
  requires(!std::is_array_v<T>)
struct Mref<T&> final {
  constexpr Mref(Mref&&) noexcept = default;
  constexpr Mref& operator=(Mref&&) noexcept = default;

  // Prevent constructing an Mref argument without writing mref().
  consteval Mref(T& t) = delete;
  // Prevent passing an Mref argument along without writing mref() again.
  Mref(Mref&) = delete;

  // Act like a T&. It can convert to a T&.
  constexpr inline operator T&() & noexcept { return t_; }
  // Act like a T&. It can be assigned a const T&.
  constexpr inline T& operator=(const T& t) noexcept {
    t_ = t;
    return *this;
  }
  // Act like a T&. It can be assigned a T&&.
  constexpr inline T& operator=(T&& t) noexcept {
    t_ = static_cast<T&&>(t);
    return *this;
  }

  // mref() should only be used to construct Mref, not T&.
  constexpr inline operator T&() && = delete;

  /// Get access to the inner type without doing an explicit type conversion to
  /// `T&`.
  constexpr inline T& inner() & { return t_; }

 private:
  struct Construct final {
    T& reference;
  };

  friend constexpr Mref<T&> mref<>(T&);
  friend constexpr Mref<T&> mref<>(Mref&);

  constexpr inline Mref(Construct c) noexcept : t_(c.reference) {}

  T& t_;
};

}  // namespace sus::mem

// Promote Mref into the `sus` namespace.
namespace sus {
using ::sus::mem::Mref;
}  // namespace sus

// Promote mref() into the top level namespace.
// TODO: Provide an option to do this or not.
using ::sus::mem::mref;
