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
#include "mem/mref.h"
#include "mem/relocate.h"

namespace sus::mem {

template <class T>
  requires std::is_move_constructible_v<T> && std::is_default_constructible_v<T>
inline constexpr T take(Mref<T&> t_ref) noexcept {
  T& t = t_ref;
  T taken(static_cast<T&&>(t));
  t.~T();
  // TODO: Support classes with a `with_default()` constructor as well.
  new (&t) T();
  return taken;
}

// SAFETY: This does *not* re-construct the object pointed to by `t`. It must
// not be used (or destructed again) afterward.
template <class T>
  requires std::is_move_constructible_v<T>
inline constexpr T take_and_destruct(::sus::marker::UnsafeFnMarker,
                                     Mref<T&> t_ref) noexcept {
  T& t = t_ref;
  T taken(static_cast<T&&>(t));
  t.~T();
  return taken;
}

template <class T>
  requires std::is_move_constructible_v<T>
inline constexpr T take_and_destruct(::sus::marker::UnsafeFnMarker,
                                     T& t) noexcept {
  T taken(static_cast<T&&>(t));
  t.~T();
  return taken;
}

}  // namespace sus::mem
