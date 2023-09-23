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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include <type_traits>

#include "sus/cmp/eq.h"
#include "sus/cmp/ord.h"

namespace sus::cmp::__private {

template <class T, class U = T>
concept VoidOrEq =
    (std::is_void_v<T> && std::is_void_v<U>) || ::sus::cmp::Eq<T, U>;

template <class T, class U = T>
concept VoidOrOrd =
    (std::is_void_v<T> && std::is_void_v<U>) || ::sus::cmp::StrongOrd<T, U>;

template <class T, class U = T>
concept VoidOrWeakOrd =
    (std::is_void_v<T> && std::is_void_v<U>) || ::sus::cmp::Ord<T, U>;

template <class T, class U = T>
concept VoidOrPartialOrd =
    (std::is_void_v<T> && std::is_void_v<U>) || ::sus::cmp::PartialOrd<T, U>;

}  // namespace sus::cmp::__private
