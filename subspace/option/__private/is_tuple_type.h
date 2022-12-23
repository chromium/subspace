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

namespace sus::tuple {
template <class T, class... Ts>
class Tuple;
}

namespace sus::option::__private {

template <class U>
struct IsTupleOfSizeTwo final {
  static constexpr bool value = false;
  using first_type = void;
  using second_type = void;
};

template <class U, class V>
struct IsTupleOfSizeTwo<::sus::tuple::Tuple<U, V>> final {
  static constexpr bool value = true;
  using first_type = U;
  using second_type = V;
};

}  // namespace sus::option::__private
