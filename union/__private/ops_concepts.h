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

#include "ops/eq.h"
#include "ops/ord.h"
#include "union/__private/type_list.h"

namespace sus::union_type::__private {

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsEqHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsEqHelper<ValueType1, TypeList<Types1...>, ValueType2,
                       TypeList<Types2...>> {
  static constexpr bool value = (::sus::ops::Eq<ValueType1, ValueType2> &&
                                 ... && ::sus::ops::Eq<Types1, Types2>);
};

// Out of line from the requires clause, and in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept UnionIsEq =
    UnionIsEqHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
                        TypeList<Types2...>> {
  static constexpr bool value = (::sus::ops::Ord<ValueType1, ValueType2> &&
                                 ... && ::sus::ops::Ord<Types1, Types2>);
};

// Out of line from the requires clause, and in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept UnionIsOrd =
    UnionIsOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsWeakOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsWeakOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
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
concept UnionIsWeakOrd =
    UnionIsWeakOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsPartialOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsPartialOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
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
concept UnionIsPartialOrd =
    UnionIsPartialOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsAnyOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsAnyOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
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
concept UnionIsAnyOrd =
    UnionIsAnyOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

}  // namespace sus::union_type::__private
