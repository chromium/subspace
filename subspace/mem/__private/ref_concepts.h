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

namespace sus::mem::__private {

template <class T>
concept IsTrivialDtorOrRef =
    std::is_trivially_destructible_v<T> || std::is_reference_v<T>;

template <class T>
concept IsTrivialCopyCtorOrRef =
    std::is_trivially_copy_constructible_v<T> || std::is_reference_v<T>;

template <class T>
concept IsTrivialCopyAssignOrRef =
    std::is_trivially_copy_assignable_v<T> || std::is_reference_v<T>;

template <class T>
concept IsTrivialMoveCtorOrRef =
    std::is_trivially_move_constructible_v<T> || std::is_reference_v<T>;

template <class T>
concept IsTrivialMoveAssignOrRef =
    std::is_trivially_move_assignable_v<T> || std::is_reference_v<T>;

}  // namespace sus::mem::__private
