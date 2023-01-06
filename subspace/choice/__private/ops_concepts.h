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

#include "subspace/choice/__private/type_list.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"

namespace sus::choice_type::__private {

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct ChoiceIsEqHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct ChoiceIsEqHelper<ValueType1, TypeList<Types1...>, ValueType2,
                        TypeList<Types2...>> {
  static constexpr bool value = (::sus::ops::Eq<ValueType1, ValueType2> &&
                                 ... && ::sus::ops::Eq<Types1, Types2>);
};

// Out of line from the requires clause, and in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept ChoiceIsEq =
    ChoiceIsEqHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct ChoiceIsOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct ChoiceIsOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
                         TypeList<Types2...>> {
  static constexpr bool value = (::sus::ops::Ord<ValueType1, ValueType2> &&
                                 ... && ::sus::ops::Ord<Types1, Types2>);
};

// Out of line from the requires clause, and in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept ChoiceIsOrd =
    ChoiceIsOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct ChoiceIsWeakOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct ChoiceIsWeakOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
                             TypeList<Types2...>> {
  // clang-format off
  static constexpr bool value =
      ((!::sus::ops::Ord<ValueType1, ValueType2> || ... ||
        !::sus::ops::Ord<Types1, Types2>)
        &&
       (::sus::ops::WeakOrd<ValueType1, ValueType2> && ... &&
        ::sus::ops::WeakOrd<Types1, Types2>));
  // clang-format on
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept ChoiceIsWeakOrd =
    ChoiceIsWeakOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct ChoiceIsPartialOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct ChoiceIsPartialOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
                                TypeList<Types2...>> {
  // clang-format off
  static constexpr bool value =
      (!::sus::ops::WeakOrd<ValueType1, ValueType2> || ... ||
       !::sus::ops::WeakOrd<Types1, Types2>)
      &&
      (::sus::ops::PartialOrd<ValueType1, ValueType2> && ... &&
       ::sus::ops::PartialOrd<Types1, Types2>);
  // clang-format on
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept ChoiceIsPartialOrd =
    ChoiceIsPartialOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct ChoiceIsAnyOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct ChoiceIsAnyOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
                            TypeList<Types2...>> {
  // clang-format off
  static constexpr bool value =
      (::sus::ops::PartialOrd<ValueType1, ValueType2> && ... &&
       ::sus::ops::PartialOrd<Types1, Types2>);
  // clang-format on
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept ChoiceIsAnyOrd =
    ChoiceIsAnyOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

}  // namespace sus::choice_type::__private
