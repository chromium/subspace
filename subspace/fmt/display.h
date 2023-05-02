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

#include <concepts>

#include "subspace/fmt/result.h"

namespace sus::fmt {

class Formatter;

template <class T>
concept Display = requires(const T& t, ::sus::fmt::Formatter& f) {
  { t.fmt(f) } -> std::same_as<::sus::fmt::Result>;
};

}  // namespace sus::fmt
