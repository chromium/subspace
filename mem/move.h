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

#include "macros/always_inline.h"

namespace sus::mem {

template <class T>
concept Moveable = (!std::is_const_v<std::remove_reference_t<T>> &&
                    std::is_move_constructible_v<T>);

template <class T>
concept MoveableForAssign = (!std::is_const_v<std::remove_reference_t<T>> &&
                             std::is_move_assignable_v<T>);

template <class T>
  requires(!std::is_const_v<std::remove_reference_t<T>>)
sus_always_inline constexpr std::remove_reference_t<T>&& move(T&& t) noexcept {
  return static_cast<typename std::remove_reference_t<T>&&>(t);
}

}  // namespace sus::mem

namespace sus {
using ::sus::mem::move;
}
