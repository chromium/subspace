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

/// _sus_for_each() will apply `macro` to each argument in the variadic argument
/// list, putting the output of `sep()` between each one.
///
/// The `sep` should be one of _sus_for_each_sep_XYZ() macros, or a function
/// macro that returns a separator.
#define _sus_for_each(macro, sep, ...) \
  __VA_OPT__(                         \
      _sus__for_each_expand(_sus__for_each_helper(macro, sep, __VA_ARGS__)))

#define _sus_for_each_sep_comma() ,
#define _sus_for_each_sep_none()

// Private helpers.

#define _sus__for_each_helper(macro, sep, a1, ...) \
  macro(a1) __VA_OPT__(sep()) __VA_OPT__(          \
      _sus__for_each_again _sus__for_each_parens(macro, sep, __VA_ARGS__))
#define _sus__for_each_parens ()
#define _sus__for_each_again() _sus__for_each_helper

#define _sus__for_each_expand(...)               \
  _sus__for_each_expand1(_sus__for_each_expand1( \
      _sus__for_each_expand1(_sus__for_each_expand1(__VA_ARGS__))))
#define _sus__for_each_expand1(...) __VA_ARGS__

// TODO: Provide a different for_each version that can handle lots of args if
// needed.
/*
#define _sus__for_each_expand(...)               \
  _sus__for_each_expand4(_sus__for_each_expand4( \
      _sus__for_each_expand4(_sus__for_each_expand4(__VA_ARGS__))))
#define _sus__for_each_expand4(...)              \
  _sus__for_each_expand3(_sus__for_each_expand3( \
      _sus__for_each_expand3(_sus__for_each_expand3(__VA_ARGS__))))
#define _sus__for_each_expand3(...)              \
  _sus__for_each_expand2(_sus__for_each_expand2( \
      _sus__for_each_expand2(_sus__for_each_expand2(__VA_ARGS__))))
#define _sus__for_each_expand2(...)              \
  _sus__for_each_expand1(_sus__for_each_expand1( \
      _sus__for_each_expand1(_sus__for_each_expand1(__VA_ARGS__))))
#define _sus__for_each_expand1(...) __VA_ARGS__
*/
