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

#include <stdint.h>

#include <type_traits>

#include "subspace/choice/__private/all_values_are_unique.h"
#include "subspace/choice/__private/index_of_value.h"
#include "subspace/choice/__private/index_type.h"
#include "subspace/choice/__private/marker.h"
#include "subspace/choice/__private/ops_concepts.h"
#include "subspace/choice/__private/pack_index.h"
#include "subspace/choice/__private/storage.h"
#include "subspace/choice/__private/type_list.h"
#include "subspace/choice/choice_types.h"
#include "subspace/macros/no_unique_address.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/clone.h"
#include "subspace/mem/copy.h"
#include "subspace/mem/move.h"
#include "subspace/mem/never_value.h"
#include "subspace/mem/replace.h"
#include "subspace/num/__private/intrinsics.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"
#include "subspace/tuple/tuple.h"

namespace sus::choice_type {

template <class TypeListOfMemberTypes, auto... Tags>
class Choice;

template <class... Ts, auto... Tags>
class Choice<__private::TypeList<Ts...>, Tags...> final {
  static_assert(sizeof...(Ts) > 0,
                "A Choice must have at least one value-type pairs.");
  static_assert(
      sizeof...(Ts) == sizeof...(Tags),
      "The number of types and values in the Choice don't match. Use "
      "`sus_choice_types()` to define the Choice's value-type pairings.");

  using Storage = __private::Storage<0, Ts...>;
  using TagsType = __private::PackFirst<decltype(Tags)...>;

  static_assert((... && std::same_as<TagsType, decltype(Tags)>),
                "All tag values must be the same type.");

  // As of GCC 13, it makes class-type template parameters be const
  // (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108169) which means they will
  // not be Copy, as you can't copy into a const type. So we have to strip the
  // const.
  // clang-format off
  template <class T>
  using GccBug = sus_gcc_bug_108169(std::remove_const_t <) T sus_gcc_bug_108169(>);
  // clang-format on

  // This is required in order for the implementation of `which()` to not be a
  // performance cliff of potentially-expensive `clone()`s.
  static_assert(::sus::mem::Copy<GccBug<TagsType>>,
                "The tag values must be Copy.");
  static_assert(::sus::ops::Eq<TagsType>,
                "The tag values must be Eq in order to find the storage from a "
                "tag value.");
  static_assert(
      __private::AllValuesAreUnique<Tags...>,
      "All tag values must be unique or some of them would be inaccessible.");

  // We add 2 to `sizeof...(Tags)` for the range of the index.
  //
  // The ~0 value (all bits are 1) is reserved for using the index as a
  // never-value field, so the index's type must be large enough to hold all
  // indices without using that bit sequence.
  //
  // The (~0 - 1) index (all bits are 1 except the lowest order bit) is
  // reserved to mark the type as moved-from. Any use of the type afterward will
  // panic due to the index being outside the range of acceptable values.
  using IndexType =
      __private::IndexType<sizeof...(Tags) + 2u, ::sus::size_of<Storage>(),
                           ::sus::data_size_of<Storage>()>;
  static_assert(!std::is_void_v<IndexType>,
                "A union can only hold a number of members representable in "
                "size_t (minus two).");
  static constexpr IndexType kNeverValue =
      ::sus::num::__private::unchecked_not(IndexType{0u});
  static constexpr IndexType kUseAfterMove =
      ::sus::num::__private::unchecked_sub(
          ::sus::num::__private::unchecked_not(IndexType{0u}), IndexType{1u});

  template <TagsType V>
  static constexpr IndexType get_index_for_value() noexcept {
    using Index = __private::IndexOfValue<V, Tags...>;
    static_assert(!std::is_void_v<Index>,
                  "The value V is not part of the Choice.");
    // SAFETY: We know `I` fits inside `IndexType` because `I` is the index of a
    // union member, and `IndexType` is chosen specifically to be large enough
    // to hold the index of all union members.
    return static_cast<IndexType>(Index::value);
  }

  template <TagsType V>
  static constexpr IndexType index = get_index_for_value<V>();

  template <TagsType V>
  using StorageTypeOfTag = __private::StorageTypeOfTag<size_t{index<V>}, Ts...>;

 public:
  using Tag = TagsType;

  // TODO: Can we construct Tuples of trivially constructible things (or some
  // set of things) without placement new and thus make this constexpr?
  template <TagsType V, class U, int&...,
            __private::ValueIsNotVoid Arg = StorageTypeOfTag<V>>
    requires(std::convertible_to<U &&, Arg>)
  static Choice with(U&& values) noexcept {
    auto u = Choice(index<V>);
    find_choice_storage_mut<index<V>>(u.storage_)
        .construct(::sus::forward<U>(values));
    return u;
  }

  template <TagsType V, int&...,
            __private::ValueIsVoid Arg = StorageTypeOfTag<V>>
  static Choice with() noexcept {
    return Choice(index<V>);
  }

  constexpr ~Choice()
    requires((std::is_trivially_destructible_v<TagsType> && ... &&
              std::is_trivially_destructible_v<Ts>))
  = default;
  ~Choice() noexcept
    requires(!(std::is_trivially_destructible_v<TagsType> && ... &&
               std::is_trivially_destructible_v<Ts>))
  {
    if (index_ != kUseAfterMove && index_ != kNeverValue)
      storage_.destroy(size_t{index_});
  }

  constexpr Choice(Choice&& o)
    requires((... && ::sus::mem::Move<Ts>) &&
             (std::is_trivially_move_constructible_v<TagsType> && ... &&
              std::is_trivially_move_constructible_v<Ts>))
  = default;
  Choice(Choice&& o)
    requires(!(... && ::sus::mem::Move<Ts>))
  = delete;

  Choice(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             !(std::is_trivially_move_constructible_v<TagsType> && ... &&
               std::is_trivially_move_constructible_v<Ts>))
      : index_(::sus::mem::replace(::sus::mref(o.index_),
                                   // Attempt to catch use-after-move by setting
                                   // the tag to an unused value.
                                   kUseAfterMove)) {
    check(index_ != kUseAfterMove);
    storage_.move_construct(size_t{index_}, ::sus::move(o.storage_));
  }

  constexpr Choice& operator=(Choice&& o)
    requires((... && ::sus::mem::Move<Ts>) &&
             (std::is_trivially_move_assignable_v<TagsType> && ... &&
              std::is_trivially_move_assignable_v<Ts>))
  = default;
  Choice& operator=(Choice&& o)
    requires(!(... && ::sus::mem::Move<Ts>))
  = delete;

  Choice& operator=(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             !(std::is_trivially_move_assignable_v<TagsType> && ... &&
               std::is_trivially_move_assignable_v<Ts>))
  {
    check(o.index_ != kUseAfterMove);
    if (index_ == o.index_) {
      storage_.move_assign(size_t{index_}, ::sus::move(o.storage_));
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(size_t{index_});
      index_ = ::sus::move(o.index_);
      storage_.move_construct(size_t{index_}, ::sus::move(o.storage_));
    }
    o.index_ = kUseAfterMove;
    return *this;
  }

  constexpr Choice(const Choice& o)
    requires((... && ::sus::mem::Copy<Ts>) &&
             (std::is_trivially_copy_constructible_v<TagsType> && ... &&
              std::is_trivially_copy_constructible_v<Ts>))
  = default;
  Choice(const Choice& o)
    requires(!(... && ::sus::mem::Copy<Ts>))
  = delete;

  Choice(const Choice& o) noexcept
    requires((... && ::sus::mem::Copy<Ts>) &&
             !(std::is_trivially_copy_constructible_v<TagsType> && ... &&
               std::is_trivially_copy_constructible_v<Ts>))
      : index_(o.index_) {
    check(o.index_ != kUseAfterMove);
    storage_.copy_construct(size_t{index_}, o.storage_);
  }

  constexpr Choice& operator=(const Choice& o)
    requires((... && ::sus::mem::Copy<Ts>) &&
             (std::is_trivially_copy_assignable_v<TagsType> && ... &&
              std::is_trivially_copy_assignable_v<Ts>))
  = default;
  Choice& operator=(const Choice& o)
    requires(!(... && ::sus::mem::Copy<Ts>))
  = delete;

  Choice& operator=(const Choice& o) noexcept
    requires((... && ::sus::mem::Copy<Ts>) &&
             !(std::is_trivially_copy_assignable_v<TagsType> && ... &&
               std::is_trivially_copy_assignable_v<Ts>))
  {
    check(o.index_ != kUseAfterMove);
    if (index_ == o.index_) {
      storage_.copy_assign(size_t{index_}, o.storage_);
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(size_t{index_});
      index_ = o.index_;
      storage_.copy_construct(size_t{index_}, o.storage_);
    }
    return *this;
  }

  // sus::mem::Clone<Choice<Ts...>> trait.
  constexpr Choice clone() const& noexcept
    requires((... && ::sus::mem::Clone<Ts>) && !(... && ::sus::mem::Copy<Ts>))
  {
    check(index_ != kUseAfterMove);
    auto u = Choice(::sus::clone(index_));
    u.storage_.clone_construct(size_t{index_}, storage_);
    return u;
  }

  /// Support for using Choice in a `switch()` statment.
  constexpr inline operator TagsType() const& noexcept { return which(); }

  /// Returns which is the active member of the Choice.
  ///
  /// Typically to access the data in the Choice, a switch statement would be
  /// used, so as to call the getter or setter methods with the right value
  /// specified as a template parameter. When used in a switch statement, the
  /// which() method can be omitted.
  ///
  /// # Example
  /// ```
  /// switch (my_union) {
  ///   case Value1: return my_union.get_ref<Value1>().stuff;
  ///   case Value2: return my_union.get_ref<Value2>().andmore;
  ///   case Value3: return my_union.get_ref<Value3>().stufftoo;
  /// }
  /// ```
  ///
  /// # Inspiration
  ///
  ///                       ████████
  ///                   ████▓▓░░▓▓██
  ///                 ██▓▓▓▓▓▓▓▓██
  ///               ██▓▓▓▓░░▓▓██
  ///             ██░░▓▓▓▓▓▓██
  ///           ██▓▓▓▓▓▓▓▓▓▓██
  ///           ██▓▓▓▓░░▓▓▓▓██
  ///   ████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓████████
  /// ██▓▓░░▓▓▓▓▓▓░░▓▓▓▓▓▓▓▓▓▓░░▓▓▓▓▓▓██
  /// ██████████████████▓▓██████████████
  ///         ██      ██      ██
  ///         ██  ██  ██      ██
  ///         ██  ██  ████      ██
  ///         ██        ██      ██
  ///         ██▒▒      ██      ██
  ///         ██▒▒        ██      ██
  ///           ████████████████████
  ///                   ██  ██
  ///                 ██  ██▓▓██
  ///                 ▓▓  ██▓▓▓▓██
  ///               ██  ████░░▓▓▓▓██            ▓▓▓▓▓▓▓▓▓▓▓▓
  ///             ██  ██  ██▓▓▓▓░░▓▓██        ▓▓░░░░░░░░░░░░
  ///             ██  ██    ████▓▓▓▓▓▓██      ▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  /// ▓▓▓▓      ██  ██    ██▓▓▓▓▓▓░░▓▓██    ▓▓▓▓░░░░░░░░░░░░
  /// ▓▓▓▓▓▓▓▓▓▓██████▓▓▓▓██▓▓░░▓▓▓▓██▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  ///                     ██████████        ▓▓▓▓░░░░░░░░░░░░
  ///                       ██  ██            ▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  ///                         ██  ██          ▓▓░░░░░░░░░░░░
  ///                           ██  ██          ▓▓▓▓▓▓▓▓▓▓▒▒
  ///                             ████
  constexpr inline TagsType which() const& noexcept {
    check(index_ != kUseAfterMove);
    constexpr TagsType tags[] = {Tags...};
    return tags[size_t{index_}];
  }

  // clang-format off
  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  inline decltype(auto) get_ref() const& noexcept{
    ::sus::check(index_ == index<V>);
    return __private::find_choice_storage<index<V>>(storage_).get_ref();
  }

  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  inline decltype(auto) get_mut() &noexcept {
    ::sus::check(index_ == index<V>);
    return __private::find_choice_storage_mut<index<V>>(storage_).get_mut();
  }

  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  inline decltype(auto) into_inner() && noexcept{
    ::sus::check(index_ == index<V>);
    auto& s = __private::find_choice_storage_mut<index<V>>(storage_);
    return ::sus::move(s).into_inner();
  }
  // clang-format on

  template <TagsType V, class U, int&..., class Arg = StorageTypeOfTag<V>>
    requires(std::convertible_to<U &&, Arg> &&
             __private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  void set(U&& values) & noexcept {
    if (index_ == index<V>) {
      __private::find_choice_storage_mut<index<V>>(storage_).assign(
          ::sus::move(values));
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(size_t{index_});
      index_ = index<V>;
      __private::find_choice_storage_mut<index<V>>(storage_).construct(
          ::sus::move(values));
    }
  }

  template <TagsType V, int&...,
            __private::ValueIsVoid Arg = StorageTypeOfTag<V>>
  void set() & noexcept {
    if (index_ != index<V>) {
      if (index_ != kUseAfterMove) storage_.destroy(size_t{index_});
      index_ = index<V>;
    }
  }

  /// sus::ops::Eq<Choice<Ts...>, Choice<Us...>> trait.
  template <class... Us, auto V, auto... Vs>
    requires(__private::ChoiceIsEq<TagsType, __private::TypeList<Ts...>,
                                   decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr bool operator==(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    return l.index_ == r.index_ && l.storage_.eq(size_t{l.index_}, r.storage_);
  }

  template <class... Us, auto V, auto... Vs>
    requires(!__private::ChoiceIsEq<TagsType, __private::TypeList<Ts...>,
                                    decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr bool operator==(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept = delete;

  /// sus::ops::Ord<Choice<Ts...>, Choice<Us...>> trait.
  ///
  /// #[doc.overloads=ord]
  template <class... Us, auto V, auto... Vs>
    requires(__private::ChoiceIsOrd<TagsType, __private::TypeList<Ts...>,
                                    decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::strong_order(l.which(), r.which());
    if (value_order != std::strong_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.ord(size_t{l.index_}, r.storage_);
    }
  }

  /// sus::ops::WeakOrd<Choice<Ts...>, Choice<Us...>> trait.
  ///
  /// #[doc.overloads=weakord]
  template <class... Us, auto V, auto... Vs>
    requires(
        __private::ChoiceIsWeakOrd<TagsType, __private::TypeList<Ts...>,
                                   decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::weak_order(l.which(), r.which());
    if (value_order != std::weak_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.weak_ord(size_t{l.index_}, r.storage_);
    }
  }

  /// sus::ops::PartialOrd<Choice<Ts...>, Choice<Us...>> trait.
  ///
  /// #[doc.overloads=partialord]
  template <class... Us, auto V, auto... Vs>
    requires(
        __private::ChoiceIsPartialOrd<TagsType, __private::TypeList<Ts...>,
                                      decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::partial_order(l.which(), r.which());
    if (value_order != std::partial_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.partial_ord(size_t{l.index_}, r.storage_);
    }
  }

  template <class... RhsTs, auto RhsTag, auto... RhsTags>
    requires(!__private::ChoiceIsAnyOrd<TagsType, __private::TypeList<Ts...>,
                                        decltype(RhsTag),
                                        __private::TypeList<RhsTs...>>)
  friend inline constexpr auto operator<=>(
      const Choice<__private::TypeList<Ts...>, Tags...>& l,
      const Choice<__private::TypeList<RhsTs...>, RhsTag, RhsTags...>& r) =
      delete;

 private:
  constexpr inline Choice(IndexType i) noexcept : index_(i) {}

  [[sus_no_unique_address]] Storage storage_;
  IndexType index_;

  sus_class_never_value_field(::sus::marker::unsafe_fn, Choice, index_,
                              kNeverValue, kNeverValue);
  constexpr Choice() = default;  // For the NeverValueField.
};

/// Used to construct a Choice with the tag and parameters as its values.
///
/// Calling make_union() produces a hint to make a Choice but does not actually
/// construct the Choice, as the full type of the Choice include all its member
/// types is not known here.
template <auto Tag, class... Ts>
[[nodiscard]] inline constexpr auto choice(
    Ts&&... vs sus_if_clang([[clang::lifetimebound]])) noexcept {
  if constexpr (sizeof...(Ts) == 0) {
    return __private::ChoiceMarkerVoid<Tag>();
  } else if constexpr (sizeof...(Ts) == 1) {
    return __private::ChoiceMarker<Tag, Ts...>(::sus::forward<Ts>(vs)...);
  } else {
    return __private::ChoiceMarker<Tag, Ts...>(
        ::sus::tuple_type::Tuple<Ts&&...>::with(::sus::forward<Ts>(vs)...));
  }
}

}  // namespace sus::choice_type

// Promote Choice into the `sus` namespace.
namespace sus {
using ::sus::choice_type::Choice;
using ::sus::choice_type::choice;
}  // namespace sus
