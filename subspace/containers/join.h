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
/// container, with a separator between each item. This is similar to `Concat`
/// but with a separator.
///
/// For example, `Slice` is `Join<S>`, which allows a `Slice<Slice<U>>` to be
/// concatenated into a `Slice<U>` of all the original elements, with separators
/// `S` cloned and placed between them.  This concept requires `T` to provide
/// `join_into(T::JoinOutputType&)` and `join_sep_into(T::JoinOutputType&, const
/// Sep&)` that does the concatenation.
///
/// The `join_into()` method will be called with `None` for the first element
/// being joined, then with `Some(const Sep&)` for the remaining elements.
///
/// TODO: String should satisfy Join as well.
template <class T, class Sep>
concept Join = requires(const T& t, const ::sus::num::usize& cap) {
  // The separator must be Clone to be replicated between each join.
  requires ::sus::mem::Clone<Sep>;
  // Join between types of T must report their lengths.
  { t.len() } -> std::same_as<::sus::num::usize>;
  // The Join between types of T produces this type.
  typename T::JoinOutputType;
  // The output type can be constructed with a capacity.
  {
    T::JoinOutputType::with_capacity(cap)
  } -> std::same_as<typename T::JoinOutputType>;
  requires requires(typename T::JoinOutputType & container, const Sep& sep) {
    // The output type can be extended by each object of type T.
    { t.join_into(container) } -> std::same_as<void>;
    // The output type can be extended by the separator `Sep`. This method is
    // static, as it does not use a specific `T`.
    { T::join_sep_into(container, sep) } -> std::same_as<void>;
  };
};

}  // namespace sus::containers
