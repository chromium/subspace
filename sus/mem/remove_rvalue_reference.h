// Copyright 2023 Google LLC
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

namespace sus::mem {

namespace __private {
template <class T>
struct remove_rvalue_reference_helper {
  using type = T;
};

template <class T>
struct remove_rvalue_reference_helper<T&&> {
  using type = T;
};

}  // namespace __private

/// Performs reference collapsing, removing rvalue references but leaving lvalue
/// references untouched.
template <class T>
using remove_rvalue_reference = __private::remove_rvalue_reference_helper<T>::type;

}  // namespace sus::mem
