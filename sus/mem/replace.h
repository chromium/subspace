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

#include <concepts>
#include <type_traits>

#include "sus/marker/unsafe.h"
#include "sus/mem/addressof.h"
#include "sus/mem/copy.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::mem {

// It would be nice to have an array overload of replace() but functions can't
// return arrays.

/// Replace the `dest` with `src`, and return the old value in `dest`.
///
/// This is equivalent to:
/// ```
/// T old = sus::move(dest);
/// dest = sus::forward<T>(src);
/// return old;
/// ```
///
/// This operation is known as `std::exchange()` in the stdlib.
template <class T, std::convertible_to<T> U>
  requires(!std::is_array_v<T> &&  //
           ::sus::mem::Move<T> &&  //
           !std::is_const_v<T> &&  //
           (!std::same_as<T, U> || ::sus::mem::Copy<T>))
[[nodiscard]] inline constexpr T replace(T& dest, const U& src) noexcept {
  auto old = T(::sus::move(dest));
  const T& typed_src = src;  // Possibly converts from U to T.
  dest = typed_src;
  return old;
}

template <class T, std::convertible_to<T> U>
  requires(!std::is_array_v<T> &&  //
           ::sus::mem::Move<T> &&  //
           !std::is_const_v<T>)
[[nodiscard]] inline constexpr T replace(T& dest, U&& src) noexcept
  requires(::sus::mem::IsMoveRef<decltype(src)>)
{
  auto old = T(::sus::move(dest));
  T&& typed_src = ::sus::move(src);  // Possibly converts from U to T.
  dest = ::sus::move(typed_src);
  return old;
}

}  // namespace sus::mem
