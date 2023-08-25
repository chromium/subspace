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

#include "fmt/core.h"
#include "sus/assertions/debug_check.h"
#include "sus/choice/__private/all_values_are_unique.h"
#include "sus/choice/__private/index_of_value.h"
#include "sus/choice/__private/index_type.h"
#include "sus/choice/__private/marker.h"
#include "sus/choice/__private/ops_concepts.h"
#include "sus/choice/__private/pack_index.h"
#include "sus/choice/__private/storage.h"
#include "sus/choice/__private/type_list.h"
#include "sus/choice/choice_types.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/lifetimebound.h"
#include "sus/macros/no_unique_address.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/clone.h"
#include "sus/mem/copy.h"
#include "sus/mem/move.h"
#include "sus/mem/never_value.h"
#include "sus/mem/replace.h"
#include "sus/num/__private/intrinsics.h"
#include "sus/ops/eq.h"
#include "sus/ops/ord.h"
#include "sus/option/option.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"
#include "sus/tuple/tuple.h"

namespace sus::choice_type {

/// A helper concept that reports if the value in a `Choice` for the given `Tag`
/// is null. When true, the accessor and setter methods are not available.
template <class Choice, auto Tag>
concept ChoiceValueIsVoid = !requires(const Choice& c) {
  { c.template get<Tag>() };
};

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

  // The data size of a union is not known from sus::data_size_of(), but is the
  // max of all its members. We know all the member types so we can compute it
  // ourselves.
  static constexpr size_t union_data_size = []() -> size_t {
    auto find_max = [max = size_t{1u}](size_t a) mutable {
      if (a == 0 || max == 0)
        max = 0;
      else if (a > max)
        max = a;
      return max;
    };
    auto max = (find_max(::sus::data_size_of<Ts>()), ..., find_max(1));
    // If the result was 0 then a member of the union does not have a defined
    // data size.
    return max > 0u ? max : ::sus::size_of<Storage>();
  }();
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
                           union_data_size>;
  static_assert(!std::is_void_v<IndexType>,
                "A union can only hold a number of members representable in "
                "size_t (minus two).");
  static constexpr IndexType kNeverValue =
      ::sus::num::__private::unchecked_not(IndexType{0u});
  static constexpr IndexType kUseAfterMove =
      ::sus::num::__private::unchecked_sub(
          ::sus::num::__private::unchecked_not(IndexType{0u}), IndexType{1u});

  template <TagsType V>
  static constexpr auto index =
      // SAFETY: We know the index fits inside `IndexType` because it is the
      // index of a union member, and `IndexType` is chosen specifically to be
      // large enough to hold the index of all union members.
      static_cast<IndexType>(__private::get_index_for_value<V, Tags...>());

  template <TagsType V>
  using StorageTypeOfTag = __private::StorageTypeOfTag<index<V>, Ts...>;

  template <TagsType V>
  using AccessTypeOfTagConst =
      decltype(__private::find_choice_storage<index<V>>(
                   std::declval<const Storage&>())
                   .as());
  template <TagsType V>
  using AccessTypeOfTagMut =
      decltype(__private::find_choice_storage_mut<index<V>>(
                   std::declval<Storage&>())
                   .as_mut());

 public:
  using Tag = TagsType;

  /// The type associated with a tag. If multiple types are associated, the
  /// resulting type here is a Tuple of those types.
  ///
  /// The `Tag` must be valid or it will fail to compile.
  template <TagsType Tag>
  using TypeForTag = __private::PublicTypeForStorageType<StorageTypeOfTag<Tag>>;

  template <TagsType V, class U, int&...,
            __private::ValueIsNotVoid Arg = StorageTypeOfTag<V>>
    requires(std::constructible_from<Arg, U &&>)
  constexpr static Choice with(U&& values) noexcept {
    auto u = Choice(index<V>);
    u.storage_.activate_for_construct(index<V>);
    find_choice_storage_mut<index<V>>(u.storage_)
        .construct(::sus::forward<U>(values));
    return u;
  }

  template <TagsType V, int&...,
            __private::ValueIsVoid Arg = StorageTypeOfTag<V>>
  constexpr static Choice with() noexcept {
    return Choice(index<V>);
  }

  constexpr ~Choice()
    requires((std::is_trivially_destructible_v<TagsType> && ... &&
              std::is_trivially_destructible_v<Ts>))
  = default;
  constexpr ~Choice() noexcept
    requires(!(std::is_trivially_destructible_v<TagsType> && ... &&
               std::is_trivially_destructible_v<Ts>))
  {
    if (index_ != kUseAfterMove && index_ != kNeverValue)
      storage_.destroy(index_);
  }

  constexpr Choice(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             (std::is_trivially_move_constructible_v<TagsType> && ... &&
              std::is_trivially_move_constructible_v<Ts>))
  = default;
  Choice(Choice&& o)
    requires(!(... && ::sus::mem::Move<Ts>))
  = delete;

  constexpr Choice(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             !(std::is_trivially_move_constructible_v<TagsType> && ... &&
               std::is_trivially_move_constructible_v<Ts>))
      : index_(::sus::mem::replace(o.index_,
                                   // Attempt to catch use-after-move by setting
                                   // the tag to an unused value.
                                   kUseAfterMove)) {
    check(index_ != kUseAfterMove);
    storage_.move_construct(index_, ::sus::move(o.storage_));
  }

  Choice& operator=(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             (std::is_trivially_move_assignable_v<TagsType> && ... &&
              std::is_trivially_move_assignable_v<Ts>))
  = default;
  Choice& operator=(Choice&& o)
    requires(!(... && ::sus::mem::Move<Ts>))
  = delete;

  constexpr Choice& operator=(Choice&& o) noexcept
    requires((... && ::sus::mem::Move<Ts>) &&
             !(std::is_trivially_move_assignable_v<TagsType> && ... &&
               std::is_trivially_move_assignable_v<Ts>))
  {
    check(o.index_ != kUseAfterMove);
    if (index_ == o.index_) {
      storage_.move_assign(index_, ::sus::move(o.storage_));
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(index_);
      index_ = ::sus::move(o.index_);
      storage_.move_construct(index_, ::sus::move(o.storage_));
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

  constexpr Choice(const Choice& o) noexcept
    requires((... && ::sus::mem::Copy<Ts>) &&
             !(std::is_trivially_copy_constructible_v<TagsType> && ... &&
               std::is_trivially_copy_constructible_v<Ts>))
      : index_(o.index_) {
    check(o.index_ != kUseAfterMove);
    storage_.copy_construct(index_, o.storage_);
  }

  constexpr Choice& operator=(const Choice& o)
    requires((... && ::sus::mem::Copy<Ts>) &&
             (std::is_trivially_copy_assignable_v<TagsType> && ... &&
              std::is_trivially_copy_assignable_v<Ts>))
  = default;
  Choice& operator=(const Choice& o)
    requires(!(... && ::sus::mem::Copy<Ts>))
  = delete;

  constexpr Choice& operator=(const Choice& o) noexcept
    requires((... && ::sus::mem::Copy<Ts>) &&
             !(std::is_trivially_copy_assignable_v<TagsType> && ... &&
               std::is_trivially_copy_assignable_v<Ts>))
  {
    check(o.index_ != kUseAfterMove);
    if (index_ == o.index_) {
      storage_.copy_assign(index_, o.storage_);
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(index_);
      index_ = o.index_;
      storage_.copy_construct(index_, o.storage_);
    }
    return *this;
  }

  // sus::mem::Clone<Choice<Ts...>> trait.
  constexpr Choice clone() const& noexcept
    requires((... && ::sus::mem::Clone<Ts>) && !(... && ::sus::mem::Copy<Ts>))
  {
    check(index_ != kUseAfterMove);
    auto u = Choice(::sus::clone(index_));
    u.storage_.clone_construct(index_, storage_);
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
  ///   case Value1: return my_union.as<Value1>().stuff;
  ///   case Value2: return my_union.as<Value2>().andmore;
  ///   case Value3: return my_union.as<Value3>().stufftoo;
  /// }
  /// ```
  ///
  /// # Inspiration
  /// ```txt
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
  /// ```
  constexpr inline TagsType which() const& noexcept {
    check(index_ != kUseAfterMove);
    constexpr TagsType tags[] = {Tags...};
    return tags[size_t{index_}];
  }

  /// Returns a const reference to the value(s) inside the Choice.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the choice.
  ///
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a Tuple of references is returned to all values in the
  /// active member.
  ///
  /// # Panics
  /// The function will panic if the active member does not match the tag value
  /// passed as the template parameter.
  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  constexpr inline decltype(auto) as() const& noexcept {
    ::sus::check(index_ == index<V>);
    return __private::find_choice_storage<index<V>>(storage_).as();
  }
  // If the storage is a value type, it can't be accessed by reference in an
  // rvalue (temporary) Choice object.
  template <TagsType V>
    requires(!std::is_reference_v<StorageTypeOfTag<V>>)
  constexpr inline const StorageTypeOfTag<V>& as() && noexcept = delete;

  /// Returns a mutable reference to the value(s) inside the Choice.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the choice.
  ///
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a Tuple of references is returned to all values in the
  /// active member.
  ///
  /// # Panics
  /// The function will panic if the active member does not match the tag value
  /// passed as the template parameter.
  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  constexpr inline decltype(auto) as_mut() & noexcept {
    ::sus::check(index_ == index<V>);
    return __private::find_choice_storage_mut<index<V>>(storage_).as_mut();
  }

  /// Unwraps the Choice to move out the current value(s) inside the Choice.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the choice.
  ///
  /// If the active member has a single value, an rvalue reference to it is
  /// returned directly, otherwise a Tuple of rvalue references is returned to
  /// all values in the active member.
  ///
  /// # Panics
  /// The function will panic if the active member does not match the tag value
  /// passed as the template parameter.
  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  constexpr inline decltype(auto) into_inner() && noexcept {
    ::sus::check(index_ == index<V>);
    auto& s = __private::find_choice_storage_mut<index<V>>(storage_);
    return ::sus::move(s).into_inner();
  }

  /// Returns a const reference to the value(s) inside the Choice.
  ///
  /// If the template parameter does not match the active member in the Choice,
  /// the function returns `None`.
  ///
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a Tuple of references is returned to all values in the
  /// active member.
  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  constexpr inline Option<AccessTypeOfTagConst<V>> get() const& noexcept {
    if (index_ != index<V>) return ::sus::none();
    return ::sus::some(__private::find_choice_storage<index<V>>(storage_).as());
  }
  // If the storage is a value type, it can't be accessed by reference in an
  // rvalue (temporary) Choice object.
  template <TagsType V>
    requires(!std::is_reference_v<StorageTypeOfTag<V>>)
  constexpr inline Option<AccessTypeOfTagConst<V>> get() && noexcept = delete;

  /// Returns a mutable reference to the value(s) inside the Choice.
  ///
  /// If the template parameter does not match the active member in the Choice,
  /// the function returns `None`.
  ///
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a Tuple of references is returned to all values in the
  /// active member.
  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  constexpr inline Option<AccessTypeOfTagMut<V>> get_mut() & noexcept {
    if (index_ != index<V>) return ::sus::none();
    return ::sus::some(
        __private::find_choice_storage_mut<index<V>>(storage_).as_mut());
  }

  template <TagsType V, class U, int&..., class Arg = StorageTypeOfTag<V>>
    requires(std::convertible_to<U &&, Arg> &&
             __private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  constexpr void set(U&& values) & noexcept {
    if (index_ == index<V>) {
      __private::find_choice_storage_mut<index<V>>(storage_).assign(
          ::sus::move(values));
    } else {
      if (index_ != kUseAfterMove) storage_.destroy(index_);
      index_ = index<V>;
      storage_.activate_for_construct(index<V>);
      __private::find_choice_storage_mut<index<V>>(storage_).construct(
          ::sus::move(values));
    }
  }

  /// Returns a const reference to the value(s) inside the Choice.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the choice.
  ///
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a Tuple of references is returned to all values in the
  /// active member.
  ///
  /// # Safety
  /// If the active member does not match the tag value passed as the template
  /// parameter, Undefined Behaviour results.
  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  constexpr inline decltype(auto) get_unchecked(
      ::sus::marker::UnsafeFnMarker) const& noexcept {
    return __private::find_choice_storage<index<V>>(storage_).as();
  }
  // If the storage is a value type, it can't be accessed by reference in an
  // rvalue (temporary) Choice object.
  template <TagsType V>
    requires(!std::is_reference_v<StorageTypeOfTag<V>>)
  constexpr inline const StorageTypeOfTag<V>& get_unchecked(
      ::sus::marker::UnsafeFnMarker) && noexcept = delete;

  /// Returns a mutable reference to the value(s) inside the Choice.
  ///
  /// The function has a template parameter specifying the tag of the active
  /// member in the choice.
  ///
  /// If the active member has a single value, a reference to it is returned
  /// directly, otherwise a Tuple of references is returned to all values in the
  /// active member.
  ///
  /// # Safety
  /// If the active member does not match the tag value passed as the template
  /// parameter, Undefined Behaviour results.
  template <TagsType V>
    requires(__private::ValueIsNotVoid<StorageTypeOfTag<V>>)
  constexpr inline decltype(auto) get_unchecked_mut(
      ::sus::marker::UnsafeFnMarker) & noexcept {
    return __private::find_choice_storage_mut<index<V>>(storage_).as_mut();
  }

  template <TagsType V, int&...,
            __private::ValueIsVoid Arg = StorageTypeOfTag<V>>
  constexpr void set() & noexcept {
    if (index_ != index<V>) {
      if (index_ != kUseAfterMove) storage_.destroy(index_);
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
    return l.index_ == r.index_ && l.storage_.eq(l.index_, r.storage_);
  }

  template <class... Us, auto V, auto... Vs>
    requires(!__private::ChoiceIsEq<TagsType, __private::TypeList<Ts...>,
                                    decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr bool operator==(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept = delete;

  /// sus::ops::StrongOrd<Choice<Ts...>, Choice<Us...>> trait.
  ///
  /// #[doc.overloads=ord]
  template <class... Us, auto V, auto... Vs>
    requires(
        __private::ChoiceIsStrongOrd<TagsType, __private::TypeList<Ts...>,
                                     decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr std::strong_ordering operator<=>(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::strong_order(l.which(), r.which());
    if (value_order != std::strong_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.ord(l.index_, r.storage_);
    }
  }

  /// sus::ops::Ord<Choice<Ts...>, Choice<Us...>> trait.
  ///
  /// #[doc.overloads=weakord]
  template <class... Us, auto V, auto... Vs>
    requires(__private::ChoiceIsOrd<TagsType, __private::TypeList<Ts...>,
                                    decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr std::weak_ordering operator<=>(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::weak_order(l.which(), r.which());
    if (value_order != std::weak_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.weak_ord(l.index_, r.storage_);
    }
  }

  /// sus::ops::PartialOrd<Choice<Ts...>, Choice<Us...>> trait.
  ///
  /// #[doc.overloads=partialord]
  template <class... Us, auto V, auto... Vs>
    requires(
        __private::ChoiceIsPartialOrd<TagsType, __private::TypeList<Ts...>,
                                      decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr std::partial_ordering operator<=>(
      const Choice& l,
      const Choice<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.index_ != kUseAfterMove && r.index_ != kUseAfterMove);
    const auto value_order = std::partial_order(l.which(), r.which());
    if (value_order != std::partial_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.partial_ord(l.index_, r.storage_);
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

  // TODO: We don't use `[[sus_no_unique_address]]` here as the compiler
  // overwrites the `index_` when we move-construct into the Storage union.
  // Clang: https://github.com/llvm/llvm-project/issues/60711
  // GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108775
  Storage storage_;
  IndexType index_;

  // Declare that this type can always be trivially relocated for library
  // optimizations.
  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn, IndexType,
                                           Ts...);

  sus_class_never_value_field(::sus::marker::unsafe_fn, Choice, index_,
                              kNeverValue, kNeverValue);
  // For the NeverValueField.
  constexpr Choice(sus::mem::NeverValueConstructor) noexcept
      : index_(kNeverValue) {}
};

/// Used to construct a Choice with the tag and parameters as its values.
///
/// Calling make_union() produces a hint to make a Choice but does not actually
/// construct the Choice, as the full type of the Choice include all its member
/// types is not known here.
template <auto Tag, class... Ts>
[[nodiscard]] inline constexpr auto choice(
    Ts&&... vs sus_lifetimebound) noexcept {
  if constexpr (sizeof...(Ts) == 0) {
    return __private::ChoiceMarkerVoid<Tag>();
  } else if constexpr (sizeof...(Ts) == 1) {
    return __private::ChoiceMarker<Tag, Ts...>(::sus::forward<Ts>(vs)...);
  } else {
    return __private::ChoiceMarker<Tag, Ts...>(
        ::sus::tuple_type::Tuple<Ts&&...>(::sus::forward<Ts>(vs)...));
  }
}

}  // namespace sus::choice_type

// fmt support.
template <class... Ts, auto... Tags, class Char>
struct fmt::formatter<
    ::sus::choice_type::Choice<sus::choice_type::__private::TypeList<Ts...>,
                               Tags...>,
    Char> {
  using Choice =
      ::sus::choice_type::Choice<sus::choice_type::__private::TypeList<Ts...>,
                                 Tags...>;

  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(const Choice& choice, FormatContext& ctx) const {
    return find_choice<FormatContext, Tags...>(choice, ctx);
  }

 private:
  template <class FormatContext, auto Tag, auto... MoreTags>
    requires(sizeof...(MoreTags) > 0)
  static auto find_choice(const Choice& choice, FormatContext& ctx) {
    if (choice.which() == Tag) {
      return format_choice<Tag>(choice, ctx);
    } else {
      return find_choice<FormatContext, MoreTags...>(choice, ctx);
    }
  }

  template <class FormatContext, auto Tag>
  static auto find_choice(const Choice& choice, FormatContext& ctx) {
    sus_debug_check(choice.which() == Tag);
    return format_choice<Tag>(choice, ctx);
  }

  template <auto Tag, class FormatContext>
  static auto format_choice(const Choice& choice, FormatContext& ctx) {
    if constexpr (sus::choice_type::ChoiceValueIsVoid<Choice, Tag>) {
      auto out = fmt::format_to(ctx.out(), "Choice(");
      ctx.advance_to(out);
      using TagFormatter =
          ::sus::string::__private::AnyFormatter<decltype(Tag), Char>;
      out = TagFormatter().format(Tag, ctx);
      return fmt::format_to(ctx.out(), ")");
    } else {
      auto out = fmt::format_to(ctx.out(), "Choice(");
      ctx.advance_to(out);
      using TagFormatter =
          ::sus::string::__private::AnyFormatter<decltype(Tag), Char>;
      out = TagFormatter().format(Tag, ctx);
      out = fmt::format_to(ctx.out(), ", ");
      ctx.advance_to(out);
      using ValueFormatter = ::sus::string::__private::AnyFormatter<
          decltype(choice.template get_unchecked<Tag>(
              ::sus::marker::unsafe_fn)),
          Char>;
      // SAFETY: The Tag here is the active tag as `find_choice()` calls this
      // method only if `which() == Tag`.
      out = ValueFormatter().format(
          choice.template get_unchecked<Tag>(::sus::marker::unsafe_fn), ctx);
      return fmt::format_to(ctx.out(), ")");
    }
  }
};

// Stream support (written out manually due to use of template specialization).
namespace sus::choice_type {
template <class... Ts, auto... Tags,
          ::sus::string::__private::StreamCanReceiveString<char> StreamType>
inline StreamType& operator<<(
    StreamType& stream,
    const Choice<__private::TypeList<Ts...>, Tags...>& value) {
  return ::sus::string::__private::format_to_stream(stream,
                                                    fmt::to_string(value));
}
}  // namespace sus::choice_type

// Promote Choice into the `sus` namespace.
namespace sus {
using ::sus::choice_type::Choice;
using ::sus::choice_type::choice;
}  // namespace sus
