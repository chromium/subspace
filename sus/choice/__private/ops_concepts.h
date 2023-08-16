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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/choice/__private/type_list.h"
#include "sus/ops/eq.h"
#include "sus/ops/ord.h"

namespace sus::choice_type::__private {

template <class TagType1, class Types1, class TagType2, class Types2>
struct ChoiceIsEqHelper;

template <class TagType1, class... Types1, class TagType2, class... Types2>
struct ChoiceIsEqHelper<TagType1, TypeList<Types1...>, TagType2,
                        TypeList<Types2...>> {
  static constexpr bool value = (::sus::ops::Eq<TagType1, TagType2> && ... &&
                                 ::sus::ops::Eq<Types1, Types2>);
};

// Out of line from the requires clause, and in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class TagType1, class Types1, class TagType2, class Types2>
concept ChoiceIsEq =
    ChoiceIsEqHelper<TagType1, Types1, TagType2, Types2>::value;

template <class TagType1, class Types1, class TagType2, class Types2>
struct ChoiceIsStrongOrdHelper;

template <class TagType1, class... Types1, class TagType2, class... Types2>
struct ChoiceIsStrongOrdHelper<TagType1, TypeList<Types1...>, TagType2,
                         TypeList<Types2...>> {
  static constexpr bool value = (::sus::ops::StrongOrd<TagType1, TagType2> && ... &&
                                 ::sus::ops::StrongOrd<Types1, Types2>);
};

// Out of line from the requires clause, and in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class TagType1, class Types1, class TagType2, class Types2>
concept ChoiceIsStrongOrd =
    ChoiceIsStrongOrdHelper<TagType1, Types1, TagType2, Types2>::value;

template <class TagType1, class Types1, class TagType2, class Types2>
struct ChoiceIsOrdHelper;

template <class TagType1, class... Types1, class TagType2, class... Types2>
struct ChoiceIsOrdHelper<TagType1, TypeList<Types1...>, TagType2,
                             TypeList<Types2...>> {
  // clang-format off
  static constexpr bool value =
      ((!::sus::ops::StrongOrd<TagType1, TagType2> || ... ||
        !::sus::ops::StrongOrd<Types1, Types2>)
        &&
       (::sus::ops::Ord<TagType1, TagType2> && ... &&
        ::sus::ops::Ord<Types1, Types2>));
  // clang-format on
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class TagType1, class Types1, class TagType2, class Types2>
concept ChoiceIsOrd =
    ChoiceIsOrdHelper<TagType1, Types1, TagType2, Types2>::value;

template <class TagType1, class Types1, class TagType2, class Types2>
struct ChoiceIsPartialOrdHelper;

template <class TagType1, class... Types1, class TagType2, class... Types2>
struct ChoiceIsPartialOrdHelper<TagType1, TypeList<Types1...>, TagType2,
                                TypeList<Types2...>> {
  // clang-format off
  static constexpr bool value =
      (!::sus::ops::Ord<TagType1, TagType2> || ... ||
       !::sus::ops::Ord<Types1, Types2>)
      &&
      (::sus::ops::PartialOrd<TagType1, TagType2> && ... &&
       ::sus::ops::PartialOrd<Types1, Types2>);
  // clang-format on
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class TagType1, class Types1, class TagType2, class Types2>
concept ChoiceIsPartialOrd =
    ChoiceIsPartialOrdHelper<TagType1, Types1, TagType2, Types2>::value;

template <class TagType1, class Types1, class TagType2, class Types2>
struct ChoiceIsAnyOrdHelper;

template <class TagType1, class... Types1, class TagType2, class... Types2>
struct ChoiceIsAnyOrdHelper<TagType1, TypeList<Types1...>, TagType2,
                            TypeList<Types2...>> {
  // clang-format off
  static constexpr bool value =
      (::sus::ops::PartialOrd<TagType1, TagType2> && ... &&
       ::sus::ops::PartialOrd<Types1, Types2>);
  // clang-format on
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class TagType1, class Types1, class TagType2, class Types2>
concept ChoiceIsAnyOrd =
    ChoiceIsAnyOrdHelper<TagType1, Types1, TagType2, Types2>::value;

}  // namespace sus::choice_type::__private
