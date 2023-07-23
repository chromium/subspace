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

#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/relocate.h"

namespace sus::test {

struct DefaultConstructible final {
  int i = 2;
};

struct TriviallyDefaultConstructible final {
  int i;
};

struct NotDefaultConstructible final {
  int i;
  constexpr NotDefaultConstructible(int i) : i(i) {}
};

struct TriviallyCopyable final {
  constexpr TriviallyCopyable(const TriviallyCopyable&) = default;
  constexpr TriviallyCopyable& operator=(const TriviallyCopyable&) = default;
  constexpr ~TriviallyCopyable() = default;
  constexpr TriviallyCopyable(int i) : i(i) {}
  int i;
};

struct TriviallyMoveableAndRelocatable final {
  constexpr TriviallyMoveableAndRelocatable(TriviallyMoveableAndRelocatable&&) =
      default;
  constexpr TriviallyMoveableAndRelocatable& operator=(
      TriviallyMoveableAndRelocatable&&) = default;
  constexpr ~TriviallyMoveableAndRelocatable() = default;
  constexpr TriviallyMoveableAndRelocatable(int i) : i(i) {}
  int i;
};

struct TriviallyCopyableNotDestructible final {
  constexpr TriviallyCopyableNotDestructible(
      const TriviallyCopyableNotDestructible&) = default;
  constexpr TriviallyCopyableNotDestructible& operator=(
      const TriviallyCopyableNotDestructible&) = default;
  constexpr ~TriviallyCopyableNotDestructible() {}
  constexpr TriviallyCopyableNotDestructible(int i) : i(i) {}
  int i;
};

struct TriviallyMoveableNotDestructible final {
  constexpr TriviallyMoveableNotDestructible(
      TriviallyMoveableNotDestructible&&) = default;
  constexpr TriviallyMoveableNotDestructible& operator=(
      TriviallyMoveableNotDestructible&&) = default;
  constexpr ~TriviallyMoveableNotDestructible(){};
  constexpr TriviallyMoveableNotDestructible(int i) : i(i) {}
  int i;
};

struct NotTriviallyRelocatableCopyableOrMoveable final {
  constexpr NotTriviallyRelocatableCopyableOrMoveable(
      NotTriviallyRelocatableCopyableOrMoveable&& o) noexcept
      : i(o.i) {}
  constexpr void operator=(
      NotTriviallyRelocatableCopyableOrMoveable&& o) noexcept {
    i = o.i;
  }
  constexpr NotTriviallyRelocatableCopyableOrMoveable(
      const NotTriviallyRelocatableCopyableOrMoveable& o) noexcept
      : i(o.i) {}
  constexpr void operator=(
      const NotTriviallyRelocatableCopyableOrMoveable& o) noexcept {
    i = o.i;
  }
  constexpr ~NotTriviallyRelocatableCopyableOrMoveable() {}
  constexpr NotTriviallyRelocatableCopyableOrMoveable(int i) : i(i) {}
  int i;
};

struct [[sus_trivial_abi]] TrivialAbiRelocatable final {
  constexpr TrivialAbiRelocatable(TrivialAbiRelocatable&&) noexcept = default;
  constexpr TrivialAbiRelocatable& operator=(TrivialAbiRelocatable&&) noexcept =
      default;
  constexpr ~TrivialAbiRelocatable() {}
  constexpr TrivialAbiRelocatable(int i) : i(i) {}
  int i;

  sus_class_trivially_relocatable_unchecked(::sus::marker::unsafe_fn);
};

}  // namespace sus::test
