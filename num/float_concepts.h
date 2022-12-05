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

namespace sus::num {

struct f32;
struct f64;

template <class T>
concept Float =
    std::same_as<f32, std::decay_t<T>> || std::same_as<f64, std::decay_t<T>>;

template <class T>
concept PrimitiveFloat = std::same_as<float, T> || std::same_as<double, T> ||
                         std::same_as<long double, T>;

}  // namespace sus::num
