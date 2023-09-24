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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include <compare>
#include <type_traits>

namespace sus::choice_type::__private {

struct Nothing {
  friend constexpr bool operator==(const Nothing&, const Nothing&) noexcept {
    return true;
  }
  friend constexpr std::strong_ordering operator<=>(const Nothing&, const Nothing&) noexcept {
    return std::strong_ordering::equivalent;
  }
};

template <class StorageType>
concept StorageIsVoid = std::same_as<Nothing, StorageType>;

/// Maps the storage type back to the public type, which means it maps `Nothing`
/// back to `void`.
template <class StorageType>
using PublicTypeForStorageType =
    std::conditional_t<StorageIsVoid<StorageType>, void, StorageType>;

}  // namespace sus::choice_type::__private
