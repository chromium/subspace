// Copyright 2023 Google LLC
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

#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"

namespace sus::ops::__private {

template <class T, class U>
concept VoidOrEq =
    (std::is_void_v<T> && std::is_void_v<U>) || ::sus::ops::Eq<T, U>;

template <class T, class U>
concept VoidOrOrd =
    (std::is_void_v<T> && std::is_void_v<U>) || ::sus::ops::Ord<T, U>;

template <class T, class U>
concept VoidOrWeakOrd =
    (std::is_void_v<T> && std::is_void_v<U>) || ::sus::ops::WeakOrd<T, U>;

template <class T, class U>
concept VoidOrPartialOrd =
    (std::is_void_v<T> && std::is_void_v<U>) || ::sus::ops::PartialOrd<T, U>;

}  // namespace sus::ops::__private
