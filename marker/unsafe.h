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

namespace sus::marker {

namespace __private {
enum class UnsafeFnMarkerConstructor { F };
}

struct UnsafeFnMarker {
  // Prevent `{}` or `{any values...}` from being used to construct an
  // `UnsafeFnMarker`, the marker should only be used as `unsafe_fn`.
  consteval UnsafeFnMarker(__private::UnsafeFnMarkerConstructor) {}
};

constexpr inline auto unsafe_fn =
    UnsafeFnMarker(::sus::marker::__private::UnsafeFnMarkerConstructor::F);

}  // namespace sus::marker
