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

#include "assertions/builtin.h"

namespace sus::mem::__private {

// Tests if the type T can be relocated with memcpy(). Checking for trivially
// movable and destructible is not sufficient, as this also honors the
// [[trivial_abi]] clang attribute, as they are now considered "trivially
// relocatable" in https://reviews.llvm.org/D114732.
template <class T>
struct relocate_one_by_memcpy
    : public std::integral_constant<bool,
#if __has_extension(trivially_relocatable)
                                    __is_trivially_relocatable(T)
#else
                                    std::is_trivially_move_constructible_v<T> &&
                                        std::is_trivially_destructible_v<T>
#endif
                                    > {
};

template <class T>
inline constexpr bool relocate_one_by_memcpy_v =
    relocate_one_by_memcpy<T>::value;

// Tests if an array of type T can be relocated with memcpy(). Checking for
// trivially movable and destructible is not sufficient, as this also honors the
// [[trivial_abi]] clang attribute, as they are now considered "trivially
// relocatable" in https://reviews.llvm.org/D114732.
//
// Volatile types are excluded, since if we have a range of volatile Foo, then
// the user is probably expecting us to follow the abstract machine and copy the
// Foo objects one by one, instead of byte-by-byte (possible tearing). See:
// https://reviews.llvm.org/D61761?id=198907#inline-548830
template <class T>
struct relocate_array_by_memcpy
    : public std::integral_constant<bool,
#if __has_extension(trivially_relocatable)
                                    __is_trivially_relocatable(T) &&
                                        !std::is_volatile_v<T>
#else
                                    std::is_trivially_move_constructible_v<T> &&
                                        std::is_trivially_destructible_v<T> &&
                                        !std::is_volatile_v<T>
#endif
                                    > {
};

template <class T>
inline constexpr bool relocate_array_by_memcpy_v =
    relocate_array_by_memcpy<T>::value;

} // namespace sus::mem::__private
