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

/// Evaluates and concatenates two macro expressions.
///
/// `x##y` will concatenate without evaluating.
#define _sus_eval_and_concat(x, y) _sus__eval_and_concat_impl(x, y)

#define _sus__eval_and_concat_impl(x, y) x##y
