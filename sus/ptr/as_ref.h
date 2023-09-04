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

#include "sus/macros/lifetimebound.h"
#include "sus/option/option.h"

namespace sus::ptr {

/// Returns an empty [`Option<T&>`]($sus::option::Option) if the `T*` pointer is
/// null, or else returns a reference to the value `T` wrapped in an
/// [`Option<T&>`]($sus::option::Option).
///
/// This is the [`pointer::as_ref`](
/// https://doc.rust-lang.org/stable/std/primitive.pointer.html#method.as_ref)
/// operation in Rust. However it is not unsafe in C++ as no lifetime is
/// created as part of the operation.
template <class T>
constexpr inline sus::Option<T&> as_ref(T* pointer sus_lifetimebound) noexcept {
  if (pointer != nullptr)
    return sus::Option<T&>(*pointer);
  else
    return sus::Option<T&>();
}

}  // namespace sus::ptr
