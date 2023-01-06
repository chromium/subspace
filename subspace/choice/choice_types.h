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

#include "subspace/choice/__private/storage.h"
#include "subspace/choice/__private/type_list.h"
#include "subspace/macros/for_each.h"
#include "subspace/macros/remove_parens.h"
#include "subspace/tuple/tuple.h"

/// Construct a set of associated value and types pairings. The type of the
/// values need have no relationship to the specified types.
///
/// The input takes the format: `(Value1, Type1A, Type1B), (Value2, Type2), ...`
/// The output is the sequence `TypeList<Tuple<Type1A, Type1B>, Tuple<Type2>,
/// ...>, Value1, Value2, ...`.///
/// Use `sus::macros::value_types::TypeAt<I, Types<...>>` to extract each tuple
/// type from the returned set of types.
///
/// The number of values that follow will always be the same as the number of
/// types in the set. This is the primary value of the `sus_choice_types()`
/// construct.
///
/// # Example
/// ```
/// template <class Types, auto FirstValue, auto... Values>
/// class Example {
///   using first_type = v<0, Types>;
///   static constexpr auto first_value = FirstValue;
/// };
///
/// using E = Example<sus_choice_types(('h', i32), ('w', f32))>;
/// // `E::first_value` will be `'h'` of type `char`.
/// // `E::first_type` will be `Tuple<i32>`.
/// ```
//
// clang-format off
#define sus_choice_types(...) \
    sus::choice_type::__private::TypeList< \
        sus_for_each(_sus__make_union_storage_type, sus_for_each_sep_comma, \
                     sus_for_each(_sus__value_types_types, sus_for_each_sep_comma, \
                                  __VA_ARGS__))>, \
    sus_for_each(_sus__value_types_value, sus_for_each_sep_comma, \
                 __VA_ARGS__)

// clang-format on

#define _sus__make_union_storage_type(types) \
  ::sus::choice_type::__private::MakeStorageType<sus_remove_parens(types)>::type

#define _sus__first(a, ...) a
#define _sus__second_plus(a, ...) __VA_ARGS__

#define _sus__value_types_types(x) \
  (sus_remove_parens_and_eval(_sus__second_plus, x))
#define _sus__value_types_value(x) sus_remove_parens_and_eval(_sus__first, x)
