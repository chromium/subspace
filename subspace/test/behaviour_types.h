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
  TriviallyCopyable(const TriviallyCopyable&) = default;
  TriviallyCopyable& operator=(const TriviallyCopyable&) = default;
  ~TriviallyCopyable() = default;
  TriviallyCopyable(int i) : i(i) {}
  int i;
};

struct TriviallyMoveableAndRelocatable final {
  TriviallyMoveableAndRelocatable(TriviallyMoveableAndRelocatable&&) = default;
  TriviallyMoveableAndRelocatable& operator=(
      TriviallyMoveableAndRelocatable&&) = default;
  ~TriviallyMoveableAndRelocatable() = default;
  TriviallyMoveableAndRelocatable(int i) : i(i) {}
  int i;
};

struct TriviallyCopyableNotDestructible final {
  TriviallyCopyableNotDestructible(const TriviallyCopyableNotDestructible&) =
      default;
  TriviallyCopyableNotDestructible& operator=(
      const TriviallyCopyableNotDestructible&) = default;
  TriviallyCopyableNotDestructible(TriviallyCopyableNotDestructible&&) = delete;
  TriviallyCopyableNotDestructible& operator=(
      TriviallyCopyableNotDestructible&&) = delete;
  ~TriviallyCopyableNotDestructible() {}
  TriviallyCopyableNotDestructible(int i) : i(i) {}
  int i;
};

struct TriviallyMoveableNotDestructible final {
  TriviallyMoveableNotDestructible(TriviallyMoveableNotDestructible&&) =
      default;
  TriviallyMoveableNotDestructible& operator=(
      TriviallyMoveableNotDestructible&&) = default;
  ~TriviallyMoveableNotDestructible(){};
  TriviallyMoveableNotDestructible(int i) : i(i) {}
  int i;
};

struct NotTriviallyRelocatableCopyableOrMoveable final {
  NotTriviallyRelocatableCopyableOrMoveable(
      NotTriviallyRelocatableCopyableOrMoveable&& o) noexcept
      : i(o.i) {}
  void operator=(NotTriviallyRelocatableCopyableOrMoveable&& o) noexcept {
    i = o.i;
  }
  NotTriviallyRelocatableCopyableOrMoveable(
      const NotTriviallyRelocatableCopyableOrMoveable& o) noexcept
      : i(o.i) {}
  void operator=(const NotTriviallyRelocatableCopyableOrMoveable& o) noexcept {
    i = o.i;
  }
  ~NotTriviallyRelocatableCopyableOrMoveable() {}
  NotTriviallyRelocatableCopyableOrMoveable(int i) : i(i) {}
  int i;
};

struct [[sus_trivial_abi]] TrivialAbiRelocatable final {
  sus_class_trivially_relocatable_unchecked(::sus::marker::unsafe_fn);
  TrivialAbiRelocatable(TrivialAbiRelocatable&&) noexcept = default;
  TrivialAbiRelocatable& operator=(TrivialAbiRelocatable&&) noexcept = default;
  ~TrivialAbiRelocatable() {}
  TrivialAbiRelocatable(int i) : i(i) {}
  int i;
};

}  // namespace sus::test
