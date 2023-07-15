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

#include "subspace/lib/__private/forward_decl.h"
#include "subspace/mem/move.h"

namespace sus::mem::__private {

template <class T>
struct NonNullMarker {
  inline constexpr NonNullMarker(T& t) noexcept : t_(t) {}

  template <class U>
  inline constexpr operator NonNull<U>() && noexcept {
    return ::sus::mem::NonNull<U>::with(t_);
  }

  inline constexpr NonNull<T> construct() && noexcept {
    return ::sus::move(*this);
  }

 private:
  T& t_;
};

}  // namespace sus::mem::__private
