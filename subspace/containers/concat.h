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

#include "subspace/mem/clone.h"
#include "subspace/mem/move.h"

namespace sus::num {
struct usize;
}

namespace sus::containers {

/// Types that support being flattened and concatenated together into a
/// container.
///
/// For example, `Slice` is `Concat`, which allows a `Slice<Slice<U>>` to be
/// concatenated into a `Slice<U>` of all the original elements. This concept
/// requires `T` to provide `concat_into(T::ConcatOutputType&)` that does the
/// concatenation.
///
/// TODO: String should satisfy Concat as well.
template <class T>
concept Concat = requires(const T& t, const ::sus::num::usize& cap) {
  // Concat between types of T must report their lengths.
  { t.len() } -> std::same_as<::sus::num::usize>;
  // The Concat between types of T produces this type.
  typename T::ConcatOutputType;
  // The output type can be constructed with a capacity.
  {
    T::ConcatOutputType::with_capacity(cap)
  } -> std::same_as<typename T::ConcatOutputType>;
  // The output type can be extended by each object of type T.
  requires requires(typename T::ConcatOutputType & container) {
    { t.concat_into(container) } -> std::same_as<void>;
  };
};

}  // namespace sus::containers
