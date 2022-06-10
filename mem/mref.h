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
template <class T, size_t N = 0>
class Mref;

/// Pass a variable to a function as a mutable reference.
template <class T>
constexpr inline Mref<T&, 0> mref(T& t) {
  return Mref<T&>(t);
}

/// Pass a variable to a function as a mutable reference.
template <class T, size_t N>
constexpr inline Mref<T, N> mref(T (&t)[N]) {
  return Mref<T, N>(t);
}

template <class T>
constexpr inline Mref<T> mref(const T& t) = delete;
template <class T, size_t N>
constexpr inline Mref<T, N> mref(const T (&t)[N]) = delete;

/// An Mref can be passed along as an Mref.
template <class T, size_t N>
constexpr inline Mref<T, N> mref(Mref<T, N>& t) {
  return Mref<T, N>(t.inner());
}

template <class T, size_t N>
struct Mref {
 private:
  template <class U>
  using RefType = std::remove_extent_t<U> (&)[N];
  template <class U>
  using RvalueRefType = std::remove_extent_t<U> (&&)[N];

 public:
  constexpr Mref(Mref&&) noexcept = default;
  constexpr Mref& operator=(Mref&&) noexcept = default;

  // Prevent passing an Mref argument along without writing mref() again.
  Mref(Mref&) = delete;

  // Act like a T&. It can convert to a T&.
  constexpr operator RefType<T>() & noexcept { return t_; }
  // Act like a T&. Arrays are not assignable.
  constexpr RefType<T> operator=(const RefType<T> t) = delete;
  // Act like a T&. Arrays are not assignable.
  constexpr T& operator=(RvalueRefType<T> t) = delete;
  // Act like a T&. Arrays can access by operator[].
  constexpr const T& operator[](size_t i) const& {
    ::sus::check(i < N);
    return t_[i];
  }
  constexpr const T& operator[](size_t i) && = delete;
  // Act like a T&. Arrays can access by operator[].
  constexpr T& operator[](size_t i) & {
    ::sus::check(i < N);
    return t_[i];
  }

  // mref() should only be used to construct Mref, not T&.
  constexpr operator RefType<T>() && = delete;

  constexpr RefType<T> inner() & { return t_; }

 private:
  friend constexpr Mref<T, N> mref<>(RefType<T>);
  friend constexpr Mref<T, N> mref<>(Mref<T, N>&);

  // TODO: Errors are confusing when you do it wrong:
  //
  //   error: ‘constexpr sus::mem::Mref<T>::Mref(T&) [with T =
  //   {anonymous}::MoveOnly]’ is private within this context
  //
  // Could we fix it with an intermediate type. mref() returns MrefPass, and
  // Mref is constructed form MrefPass. Then we could delete Mref(T&) instead
  // of making it private. Does it codegen the same though?
  constexpr Mref(RefType<T> t) noexcept : t_(t) {}

  RefType<T> t_;
};

template <class T>
struct Mref<T&, 0> {
  constexpr Mref(Mref&&) noexcept = default;
  constexpr Mref& operator=(Mref&&) noexcept = default;

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

  constexpr inline T& inner() & { return t_; }

 private:
  friend constexpr Mref<T&> mref<>(T&);
  friend constexpr Mref mref<>(Mref&);

  // TODO: Errors are confusing when you do it wrong:
  //
  //   error: ‘constexpr sus::mem::Mref<T>::Mref(T&) [with T =
  //   {anonymous}::MoveOnly]’ is private within this context
  //
  // Could we fix it with an intermediate type. mref() returns MrefPass, and
  // Mref is constructed form MrefPass. Then we could delete Mref(T&) instead
  // of making it private. Does it codegen the same though?
  constexpr inline Mref(T& t) noexcept : t_(t) {}

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
