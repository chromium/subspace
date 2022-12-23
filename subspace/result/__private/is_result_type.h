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

namespace sus::result {
template <class T, class E>
class Result;
}

namespace sus::result::__private {

template <class U>
struct IsResultType final : std::false_type {
  using ok_type = void;
  using err_type = void;
};

template <class U, class V>
struct IsResultType<Result<U, V>> final : std::true_type {
  using ok_type = U;
  using err_type = V;
};

}  // namespace sus::result::__private
