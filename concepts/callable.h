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

namespace sus::concepts::callable {

template <class F>
concept FunctionPointer = std::is_pointer_v<decltype(+std::declval<F>())>;

template <class F>
concept CallableObject = (!FunctionPointer<F>) && requires { &F::operator(); };

template <class F>
concept Callable = FunctionPointer<F> || CallableObject<F>;

}  // namespace sus::concepts::callable
