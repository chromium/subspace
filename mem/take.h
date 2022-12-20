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

#include "marker/unsafe.h"
#include "mem/addressof.h"
#include "mem/move.h"
#include "mem/mref.h"
#include "mem/relocate.h"

namespace sus::mem {

/// Requires that the type is `final` because calling take() on a base class
/// could lead to Undefined Behaviour, unless the object was accessed through
/// std::launder thereafter, as replacing a subclass with the base class would
/// change the underlying storage.
template <class T>
  requires(std::is_move_constructible_v<T> &&
           std::is_default_constructible_v<T> && std::is_final_v<T>)
inline constexpr T take(T& t) noexcept {
  auto taken = T(::sus::move(t));
  t.~T();
  // TODO: Support classes with a `with_default()` constructor as well.
  new (&t) T();
  return taken;
}

/// SAFETY: This does *not* re-construct the object pointed to by `t`. It must
/// not be used (or destructed again) afterward.
template <class T>
  requires std::is_move_constructible_v<T>
inline constexpr T take_and_destruct(::sus::marker::UnsafeFnMarker,
                                     T& t) noexcept {
  // NOTE: MSVC fails to compile if we use move() here, which is not really
  // explainable beyond it being a compiler bug. It claims there is use of an
  // uninitialized symbol if we static_cast though a method. So we manually
  // static_cast here.
  auto taken = T(static_cast<T&&>(t));
  t.~T();
  return taken;
}

}  // namespace sus::mem
