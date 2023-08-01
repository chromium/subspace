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

#include "sus/mem/move.h"

namespace sus::ops {

/// Specializations of this struct are used to implement the Try concept.
template <class T>
struct TryImpl;

/// A concept for types that can indicate success and failure.
///
/// The Try concept is implemented by specializing `TryImpl` for a type `T`
/// with:
/// * A type `Output` that is the unwrapped success value type.
/// * A member `static bool is_success(const T& t)` that reports if the
///   given `t` is a success or failure value.
/// * A member `static Output to_output(T&& t)` that unwraps a successful T to
///   its success value.
template <class T>
concept Try =
    requires {
      // The Try type is not a reference, conversions require concrete types.
      requires !std::is_reference_v<T>;
      // The Output type can be converted to/from the Try type.
      typename TryImpl<T>::Output;
      // The Output type is also not a reference.
      requires !std::is_reference_v<typename TryImpl<T>::Output>;
      // The Output type must not be void, as that interfers with type
      // conversions and complicates things greatly.
      requires !std::is_void_v<typename TryImpl<T>::Output>;
    }   //
    &&  //
    requires(const T& t, T&& tt, TryImpl<T>::Output output) {
      // is_success() reports if the Try type is in a success state.
      { TryImpl<T>::is_success(t) } -> std::same_as<bool>;
      // to_output() unwraps from the Try type to its success type.
      {
        TryImpl<T>::to_output(::sus::move(tt))
      } -> std::same_as<typename TryImpl<T>::Output>;
      // from_output() converts a success type to the Try type.
      { TryImpl<T>::from_output(::sus::move(output)) } -> std::same_as<T>;
    };

}  // namespace sus::ops
