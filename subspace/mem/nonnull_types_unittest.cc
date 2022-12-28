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

#include "construct/default.h"
#include "mem/nonnull.h"
#include "mem/relocate.h"
#include "test/behaviour_types.h"

using sus::construct::Default;
using sus::mem::NonNull;
using sus::mem::relocate_array_by_memcpy;
using sus::mem::relocate_one_by_memcpy;

namespace default_constructible {
using T = NonNull<sus::test::DefaultConstructible>;
using From = T;
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T>);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_assignable_v<T>);
static_assert(std::is_trivially_destructible_v<T>);
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_copy_assignable_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_move_assignable_v<T>);
static_assert(std::is_nothrow_swappable_v<T>);
static_assert(std::is_constructible_v<T, From&&>);
static_assert(std::is_assignable_v<T, From&&>);
static_assert(std::is_constructible_v<T, const From&>);
static_assert(std::is_assignable_v<T, const From&>);
static_assert(std::is_constructible_v<T, From>);
static_assert(std::is_trivially_constructible_v<T, From>);
static_assert(std::is_assignable_v<T, From>);
static_assert(std::is_nothrow_destructible_v<T>);
static_assert(!Default<T>);
static_assert(relocate_one_by_memcpy<T>);
static_assert(relocate_array_by_memcpy<T>);
}  // namespace default_constructible

namespace not_default_constructible {
using T = NonNull<sus::test::NotDefaultConstructible>;
using From = T;
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T>);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_assignable_v<T>);
static_assert(std::is_trivially_destructible_v<T>);
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_copy_assignable_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_move_assignable_v<T>);
static_assert(std::is_nothrow_swappable_v<T>);
static_assert(std::is_constructible_v<T, From&&>);
static_assert(std::is_assignable_v<T, From&&>);
static_assert(std::is_constructible_v<T, const From&>);
static_assert(std::is_assignable_v<T, const From&>);
static_assert(std::is_constructible_v<T, From>);
static_assert(std::is_trivially_constructible_v<T, From>);
static_assert(std::is_assignable_v<T, From>);
static_assert(std::is_nothrow_destructible_v<T>);
static_assert(!Default<T>);
static_assert(relocate_one_by_memcpy<T>);
static_assert(relocate_array_by_memcpy<T>);
}  // namespace not_default_constructible

namespace trivially_copyable {
using T = NonNull<sus::test::TriviallyCopyable>;
using From = T;
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T>);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_assignable_v<T>);
static_assert(std::is_trivially_destructible_v<T>);
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_copy_assignable_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_move_assignable_v<T>);
static_assert(std::is_nothrow_swappable_v<T>);
static_assert(std::is_constructible_v<T, From&&>);
static_assert(std::is_assignable_v<T, From&&>);
static_assert(std::is_constructible_v<T, const From&>);
static_assert(std::is_assignable_v<T, const From&>);
static_assert(std::is_constructible_v<T, From>);
static_assert(std::is_assignable_v<T, From>);
static_assert(std::is_nothrow_destructible_v<T>);
static_assert(!Default<T>);
static_assert(relocate_one_by_memcpy<T>);
static_assert(relocate_array_by_memcpy<T>);
}  // namespace trivially_copyable

namespace trivially_moveable_and_relocatable {
using T = NonNull<sus::test::TriviallyMoveableAndRelocatable>;
using From = T;
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T>);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_assignable_v<T>);
static_assert(std::is_trivially_destructible_v<T>);
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_copy_assignable_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_move_assignable_v<T>);
static_assert(std::is_nothrow_swappable_v<T>);
static_assert(std::is_constructible_v<T, From&&>);
static_assert(std::is_assignable_v<T, From&&>);
static_assert(std::is_constructible_v<T, const From&>);
static_assert(std::is_assignable_v<T, const From&>);
static_assert(std::is_constructible_v<T, From>);
static_assert(std::is_trivially_constructible_v<T, From>);
static_assert(std::is_assignable_v<T, From>);
static_assert(std::is_nothrow_destructible_v<T>);
static_assert(!Default<T>);
static_assert(relocate_one_by_memcpy<T>);
static_assert(relocate_array_by_memcpy<T>);
}  // namespace trivially_moveable_and_relocatable

namespace trivially_copyable_not_destructible {
using T = NonNull<sus::test::TriviallyCopyableNotDestructible>;
using From = T;
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T>);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_assignable_v<T>);
static_assert(std::is_trivially_destructible_v<T>);
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_copy_assignable_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_move_assignable_v<T>);
static_assert(std::is_nothrow_swappable_v<T>);
static_assert(std::is_constructible_v<T, From&&>);
static_assert(std::is_assignable_v<T, From&&>);
static_assert(std::is_constructible_v<T, const From&>);
static_assert(std::is_assignable_v<T, const From&>);
static_assert(std::is_constructible_v<T, From>);
static_assert(std::is_trivially_constructible_v<T, From>);
static_assert(std::is_assignable_v<T, From>);
static_assert(std::is_nothrow_destructible_v<T>);
static_assert(!Default<T>);
static_assert(relocate_one_by_memcpy<T>);
static_assert(relocate_array_by_memcpy<T>);
}  // namespace trivially_copyable_not_destructible

namespace trivially_moveable_not_destructible {
using T = NonNull<sus::test::TriviallyMoveableNotDestructible>;
using From = T;
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T>);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_assignable_v<T>);
static_assert(std::is_trivially_destructible_v<T>);
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_copy_assignable_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_move_assignable_v<T>);
static_assert(std::is_nothrow_swappable_v<T>);
static_assert(std::is_constructible_v<T, From&&>);
static_assert(std::is_assignable_v<T, From&&>);
static_assert(std::is_constructible_v<T, const From&>);
static_assert(std::is_assignable_v<T, const From&>);
static_assert(std::is_constructible_v<T, From>);
static_assert(std::is_trivially_constructible_v<T, From>);
static_assert(std::is_assignable_v<T, From>);
static_assert(std::is_nothrow_destructible_v<T>);
static_assert(!Default<T>);
static_assert(relocate_one_by_memcpy<T>);
static_assert(relocate_array_by_memcpy<T>);
}  // namespace trivially_moveable_not_destructible

namespace not_trivially_relocatable_copyable_or_moveable {
using T = NonNull<sus::test::NotTriviallyRelocatableCopyableOrMoveable>;
using From = T;
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T>);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_assignable_v<T>);
static_assert(std::is_trivially_destructible_v<T>);
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_copy_assignable_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_move_assignable_v<T>);
static_assert(std::is_nothrow_swappable_v<T>);
static_assert(std::is_constructible_v<T, From&&>);
static_assert(std::is_assignable_v<T, From&&>);
static_assert(std::is_constructible_v<T, const From&>);
static_assert(std::is_assignable_v<T, const From&>);
static_assert(std::is_constructible_v<T, From>);
static_assert(std::is_trivially_constructible_v<T, From>);
static_assert(std::is_assignable_v<T, From>);
static_assert(std::is_nothrow_destructible_v<T>);
static_assert(!Default<T>);
static_assert(relocate_one_by_memcpy<T>);
static_assert(relocate_array_by_memcpy<T>);
}  // namespace not_trivially_relocatable_copyable_or_moveable

namespace trivial_abi_relocatable {
using T = NonNull<sus::test::TrivialAbiRelocatable>;
using From = T;
static_assert(!std::is_trivial_v<T>);
static_assert(!std::is_aggregate_v<T>);
static_assert(std::is_standard_layout_v<T>);
static_assert(!std::is_trivially_default_constructible_v<T>);
static_assert(std::is_trivially_copy_constructible_v<T>);
static_assert(std::is_trivially_copy_assignable_v<T>);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_assignable_v<T>);
static_assert(std::is_trivially_destructible_v<T>);
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_copy_assignable_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_move_assignable_v<T>);
static_assert(std::is_nothrow_swappable_v<T>);
static_assert(std::is_constructible_v<T, From&&>);
static_assert(std::is_assignable_v<T, From&&>);
static_assert(std::is_constructible_v<T, const From&>);
static_assert(std::is_assignable_v<T, const From&>);
static_assert(std::is_constructible_v<T, From>);
static_assert(std::is_trivially_constructible_v<T, From>);
static_assert(std::is_assignable_v<T, From>);
static_assert(std::is_nothrow_destructible_v<T>);
static_assert(!Default<T>);
static_assert(relocate_one_by_memcpy<T>);
static_assert(relocate_array_by_memcpy<T>);
}  // namespace trivial_abi_relocatable
