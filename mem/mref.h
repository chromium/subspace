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
class Mref {
 public:
  Mref(Mref&&) noexcept = default;
  Mref& operator=(Mref&&) noexcept = default;

  // Prevent passing an Mref argument along without writing mref() again.
  Mref(Mref&) = delete;

  // Act like a T&. It can convert to a T&.
  operator T&() & noexcept { return t_; }
  // Act like a T&. It can be assigned a const T&.
  T& operator=(const T& t) noexcept {
    t_ = t;
    return *this;
  }
  // Act like a T&. It can be assigned a T&&.
  T& operator=(T&& t) noexcept {
    t_ = static_cast<T&&>(t);
    return *this;
  }

 private:
  template <class U>
  friend Mref<U> mref(U&);
  template <class U>
  friend Mref<U> mref(Mref<U>&);

  Mref(T& t) noexcept : t_(t) {}

  T& t_;
};

/// Wrap an lvalue variable with mref() when passing it as a function argument
/// to pass it as a mutable reference.
template <class T>
inline Mref<T> mref(T& t) {
  return Mref<T>(t);
}

/// An Mref can be passed along as an Mref.
template <class T>
inline Mref<T> mref(Mref<T>& t) {
  return Mref<T>(static_cast<T&>(t));
}

}  // namespace sus::mem

// Promote Mref into the `sus` namespace.
namespace sus {
using ::sus::mem::Mref;
}  // namespace sus

// Promote mref() into the top level namespace.
// TODO: Provide an option to do this or not.
using ::sus::mem::mref;
