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

#include <cstddef>
#include <type_traits>

#include "subspace/macros/compiler.h"
#include "subspace/macros/no_unique_address.h"

namespace sus::mem::__private {

constexpr inline size_t min(size_t a, size_t b) { return a < b ? a : b; }

template <class T, size_t bytes>
struct NoUnique {
  [[sus_no_unique_address]] T x;
  char c[bytes];
};

template <class T, size_t bytes>
struct sus_if_msvc(__declspec(empty_bases)) Subclass : T {
  char c[bytes];
};

// A final class can't be inherited from, so we don't need to check for that
// method of injecting objects into the type's padding.
template <class T, size_t bytes>
constexpr inline size_t data_size_finder_final_class =
    sizeof(T) < sizeof(NoUnique<T, bytes>)
        ? sizeof(T) - bytes + 1
        : data_size_finder_final_class<T, bytes + 1>;

template <class T>
constexpr inline size_t
    data_size_finder_final_class<T, alignof(std::max_align_t)> = 0u;

// An inheritable class can have its padding used by a subclass.
template <class T, size_t bytes>
constexpr inline size_t data_size_finder_inheritable_class =
    sizeof(T) < min(sizeof(NoUnique<T, bytes>), sizeof(Subclass<T, bytes>))
        ? sizeof(T) - bytes + 1
        : data_size_finder_inheritable_class<T, bytes + 1>;

template <class T>
constexpr inline size_t
    data_size_finder_inheritable_class<T, alignof(std::max_align_t)> = 0u;

// We don't know the types inside a union, so we're unable to tell what is
// padding. Such types should not be memcpy'd unless the types inside are
// checked for their data sizes explicitly and they all have the same data size.
template <class T, size_t bytes>
constexpr inline size_t data_size_finder_union = 0u;

/// Finds the first `bytes` where it causes the size of either NoUnique or
/// Subclass to grow, which indicates it has filled more than the number of
/// padding bytes. The remaining bytes can contain data for the type `T`.
//
// clang-format off
template <class T, size_t bytes = 1u,
          bool is_class = std::is_class_v<T>,
          bool is_final = std::is_final_v<T>,
          bool is_union = std::is_union_v<T>>
constexpr inline size_t data_size_finder = sizeof(T);
// clang-format on

// For types that can have padding (aka record types), we have to check
// different cases.
template <class T, size_t bytes>
constexpr inline size_t data_size_finder<T, bytes, true, true, false> =
    data_size_finder_final_class<T, bytes>;
template <class T, size_t bytes>
constexpr inline size_t data_size_finder<T, bytes, true, false, false> =
    data_size_finder_inheritable_class<T, bytes>;
template <class T, size_t bytes>
constexpr inline size_t data_size_finder<T, bytes, false, false, true> =
    data_size_finder_union<T, bytes>;

}  // namespace sus::mem::__private
