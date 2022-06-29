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

/// Remove parentheses around one or more arguments, if they are present.
///
/// It performs the following transformations.
/// x => x
/// (x) => x
/// (x, y) => x, y
///
/// Based on: https://stackoverflow.com/a/62984543
#define sus_remove_parens(x) _sus__remove_inner_rename(_sus__remove_inner x)

// Step 1: If the input had brackets, now it no longer does. The result will
// always be `_sus__remove_inner x` at the end.
#define _sus__remove_inner(...) _sus__remove_inner __VA_ARGS__
// Step 2: Now that `x` has no parentheses, expand `x` into all of its
// arguments, which we denote `x...`.
#define _sus__remove_inner_rename(...) _sus__remove_inner_rename_(__VA_ARGS__)
// Step 3: Concat to the start of the content, resulting in
// `_sus_remove_outer_sus__remove_inner x...`.
#define _sus__remove_inner_rename_(...) _sus__remove_outer##__VA_ARGS__
// Step 4: Remove the `_sus_remove_outer_sus__remove_inner`, leaving just
// `x...`.
#define _sus__remove_outer_sus__remove_inner
